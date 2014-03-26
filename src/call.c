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
 * This function handles a call and send a wave packet through the net
 *
 * @param call call data
 */
void call_stream(call_t* call) {
    struct timespec tv;
    char send_buf[RTP_BUFSIZE];
    char *recv_buf = NULL;

    int recv_bufsize = 0;
    int must_recv_more;
    int i;
    
    /**
     * Send stream
     */
    i = waveread(call->song, send_buf, RTP_BUFSIZE);
    
    if(i>0) {
        rtp_session_send_with_ts(call->r_session, (uint8_t*) send_buf, i, call->send_ts);
        call->send_ts += RTP_BUFSIZE;
    } else {
        log_debug("CALL_STREAM", "Song finished");
        call->status=CALL_CLOSED;
    }

    /**
     * Recieve stream
     */
    i=0;
    do {
        recv_bufsize += RTP_BUFSIZE;
        recv_buf = (char *) realloc(recv_buf, recv_bufsize);
        if(!recv_buf) {
            log_err("CALL_STREAM", "Cannot allocate memory");
            exit(-1);
        }
        i += rtp_session_recv_with_ts(call->r_session, (uint8_t*) recv_buf, RTP_BUFSIZE, 
                                     call->recv_ts, &must_recv_more);
    }while(i>0 && must_recv_more && recv_bufsize < RTP_BUFSIZE * 20);
    call->recv_ts+=RTP_BUFSIZE;

    /* Do something here with the received stream */

    free(recv_buf);

    /* TODO: multithreading */
    tv.tv_sec = 0;
    tv.tv_nsec = (long)(1000.0f*8000.0f/RTP_BUFSIZE);
    nanosleep(&tv, NULL);
}

/**
 * This function closes the call and frees used memory.
 *
 * @param call call data to free
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
void call_freeall() {
    while(call_list != NULL)
       call_list = call_free(call_list);
}

/**
 * Cycles through all calls and update their status
 */
void call_update() {
    int update_list = 1;
    time_t cur_time = time(NULL);
    call_t* call = call_list;
    call_t* call_prec = NULL;

    while(call != NULL) {
        update_list = 1;
        switch(call->status) {
            case CALL_RINGING:
                if(call->ringing_timer > 0 && cur_time - call->ringing_timer > 2) {
                    sip_answer_call(call);
                    call->ringing_timer = -1;
                }
                break;
            case CALL_ACTIVE: 
                call_stream(call);
                break;
            case CALL_CLOSED:
                log_debug("CALL_CLOSED", "Freeing memory");
               
                if(call_prec == NULL)
                    call_list = call->next;
                else 
                    call_prec->next = call->next;
               
                call = call_free(call); 

                update_list = 0;
                break;
        }
        if(update_list) {
            call_prec = call;
            call = call->next;
        }
    }
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

    /* FIXME: I think these values should be random.. */
    call->send_ts = 0;
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
