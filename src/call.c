/* SIPbot - An opensource VoIP answering machine
 * Copyright (C) 2014 Alain (Carpikes) Carlucci
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

/**
 * @file call.c
 * @brief Call manager
 */
#include "call.h"
#include "sip.h"

/**
 * \brief Active calls
 */
static call_t* call_list = NULL;

#define RTP_BUFSIZE 512

/**
 * This functions is called when socket is ready to transmit
 * a new audio frame. 
 *
 * @param call The call
 */
void call_transmit_data(call_t* call) {
    char send_buf[RTP_BUFSIZE];
    int i;
    
    /**
     * Send stream
     */
    i = waveread(call->song, send_buf, RTP_BUFSIZE);
    
    if(i>0) {
        rtp_session_send_with_ts(call->r_session, (uint8_t*) send_buf, 
                                 i, call->send_ts);
        call->send_ts += RTP_BUFSIZE;
    } else {
        log_debug("CALL_STREAM", "Song finished");
        call->status=CALL_CLOSED;
    }

}

/**
 * This functions is called when socket is ready to receive
 * a new audio frame.
 *
 * @param call The call
 */
void call_receive_data(call_t* call) {
    char *recv_buf = NULL;
    int i = 0;
    int recv_bufsize = 0;
    int must_recv_more = 0;
 
    do {
        recv_bufsize += RTP_BUFSIZE;
        recv_buf = (char *) realloc(recv_buf, recv_bufsize);
        if(!recv_buf) {
            log_err("CALL_STREAM", "Cannot allocate memory");
            exit(-1);
        }

        /* Receive a piece of data */
        i += rtp_session_recv_with_ts(call->r_session, (uint8_t*) recv_buf, 
                RTP_BUFSIZE, call->recv_ts, &must_recv_more);

    }while(i>0 && must_recv_more && recv_bufsize < RTP_BUFSIZE * 20);

    call->recv_ts+=RTP_BUFSIZE;

    /* TODO: here you can use recv_buf for your own purpose */

    free(recv_buf);
}

/**
 * This function closes the call and frees used memory.
 *
 * @param call call data to free
 * @return This function returns the call next to the deleted one
 */
call_t* call_free(call_t* call) {
    call_t* next = NULL;
    if(call->caller)
        free(call->caller);
    if(call->ip)
        free(call->ip);

    if(call->r_session != NULL)
        rtp_session_destroy(call->r_session);

    sip_terminate_call(call->cid, call->did);
    if(call->song != NULL)
        waveclose(call->song);

    next = call->next;
    free(call);

    return next;
}

/**
 * This function is called when SIPbot is closing
 * It frees and closes all calls.
 */
void call_freeall(void) {
    while(call_list != NULL)
       call_list = call_free(call_list);
}

/**
 * Cycles through all calls and update their status
 */
void call_update(void) {
    SessionSet *send_set, *recv_set;
    int num_socks=0, select_ret, update_list = 1;
    time_t cur_time = time(NULL);
    call_t* call = call_list;
    call_t* call_prec = NULL;

    send_set = session_set_new();
    recv_set = session_set_new();
    while(call != NULL) {
        update_list = 1;
        switch(call->status) {
            case CALL_RINGING:
                if(call->ringing_timer > 0 && 
                   cur_time-call->ringing_timer > 2) {
                    /* Ringing timeout: answer the call */
                    sip_answer_call(call);
                    call->ringing_timer = -1;
                }
                break;
            case CALL_ACTIVE:
                session_set_set(send_set, call->r_session);
                session_set_set(recv_set, call->r_session);
                num_socks++;
                break;
            case CALL_CLOSED:
                log_debug("CALL_CLOSED", "Freeing memory");
               
                if(call_prec == NULL)
                    call_list = call->next;
                else 
                    call_prec->next = call->next;
               
                call = call_free(call); 

                /* call_list is already incremented */
                update_list = 0;
                break;
        }

        if(update_list) {
            call_prec = call;
            call = call->next;
        }
    }

    if(num_socks > 0) {
        select_ret = session_set_select(recv_set, send_set, NULL);

        if(select_ret > 0) {
            call = call_list;

            while(call != NULL) {
                if(call->status == CALL_ACTIVE) {
                   if(session_set_is_set(recv_set, call->r_session))
                       call_receive_data(call);
                   if(session_set_is_set(send_set, call->r_session))
                       call_transmit_data(call);
                }

                call = call->next;
            }
        }
    }

    session_set_destroy(recv_set);
    session_set_destroy(send_set);
}

/**
 * Handle a new call
 *
 * @param display_name caller's phone number
 * @param rtp_addr RTP server ip address
 * @param rtp_port RTP server port
 * @param cid eXosip Call ID
 * @param tid eXosip Transaction ID 
 * @param did eXosip Dialog ID
 */
int call_new(const char* display_name, const char* rtp_addr, 
             int rtp_port, int cid, int tid, int did) {

    call_t *call = NULL;
    call = (call_t*) calloc(1, sizeof(call_t));
    if(call == NULL) {
        log_err("SIP_UPDATE", "Out of memory. Exiting.");
        exit(-1);
    }

    call->caller = strdup(display_name);
    call->ip = strdup(rtp_addr);

    call->port = rtp_port;
    call->cid = cid;
    call->tid = tid;
    call->did = did;
    call->status = CALL_RINGING;
    call->ringing_timer = time(NULL);
    call->r_session = NULL;
    call->next = NULL;

    call->send_ts = rand();
    call->recv_ts = 0;

    call->song = waveopen(WAVFILE);

    if(call->song == NULL) {
        log_debug("SIP_UPDATE", "Cannot open " WAVFILE);
        call_free(call);
        return 0;
    }

    if(call_list == NULL)
        call_list = call;
    else {
        call_t* head = call_list;
        while(head->next!=NULL) 
            head=head->next;
        head->next=call;
    }

    return 1;
}

/**
 * Change a call status
 *
 * @param cid eXosip Conversation ID
 * @param status New status
 */
int call_set_status(int cid, int status) {
    call_t *call = call_list;
    while(call != NULL) {
        if(call->cid == cid) {
            call->status = status;
            break;
        }
        call = call->next;
    }
    if(call != NULL)
        return 1;

    return 0;
}
