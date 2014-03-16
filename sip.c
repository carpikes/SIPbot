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

#include "sip.h"
#include "sdp.h"

static int              port = 5060;
static int              register_id = NULL;
static struct eXosip_t* ctx = NULL;
static time_t           reg_timer = 0;

call_t* call_list;

void sip_exit() {
	sip_reg_delete();
    eXosip_quit(ctx);
    free(ctx);
}

void call_stream(call_t* call) {
    char buf[500];
    int i;
    
    i = fread(buf, 1, 500, call->song);
    if(i>0) {
        rtp_session_send_with_ts(call->r_session, (uint8_t*) buf, i, call->user_ts);
        call->user_ts += i;
    } else {
        printf("[DEBUG] song finished\n");
        call->status=CALL_CLOSED;
    }
}

call_t* call_free(call_t* call) {
    call_t* next = NULL;
    if(call->caller)
        free(call->caller);
    if(call->ip)
        free(call->ip);

    if(call->r_session != NULL)
        rtp_session_destroy(call->r_session);

    eXosip_call_terminate(ctx, call->cid, call->did);
    fclose(call->song);

    next = call->next;
    free(call);

    return next;
}

void call_freeall() {
    while(call_list != NULL)
       call_list = call_free(call_list);
}

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
                printf("[CALL_CLOSED] Freeing memory\n");
               
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

void sip_update() {
    sdp_message_t* sdp_packet;
    sdp_connection_t* sdp_audio_conn;
    sdp_media_t* sdp_audio_media;

    eXosip_event_t* evt;
    call_t* call = NULL;
    time_t cur_time = time(NULL);

	if(reg_timer > 0 && cur_time - reg_timer > REG_TIMEOUT) {
		printf("[DEBUG] Updating registration\n");
		sip_reg_update();
	}

    evt = eXosip_event_wait(ctx, 0, 50);
    eXosip_lock(ctx);
    eXosip_automatic_action(ctx);
    eXosip_unlock(ctx);

    if(evt == NULL)
        return;

    switch(evt->type) {
        case EXOSIP_REGISTRATION_FAILURE:
            printf("[DEBUG] Reg fail\n");
            break;
        case EXOSIP_REGISTRATION_SUCCESS:
            reg_timer = time(NULL);
            printf("[DEBUG] Client registered\n");
            break;
        case EXOSIP_CALL_INVITE:
            printf("[CALL_INVITE] FROM %s\n", evt->request->from->displayname);
            sdp_packet = eXosip_get_remote_sdp(ctx, evt->did);
            if(sdp_packet == NULL) 
                break;
            
            sdp_audio_conn = eXosip_get_audio_connection(sdp_packet);
            sdp_audio_media = eXosip_get_audio_media(sdp_packet);

            if(sdp_audio_conn != NULL && sdp_audio_media != NULL
                 && strlen(sdp_audio_conn->c_addr)>0) {
                
                call = (call_t*) calloc(1, sizeof(call_t));
                if(call == NULL) {
                    fprintf(stderr, "[CALL_INVITE] Out of memory. Exiting.\n");
                    exit(-1);
                }

                call->caller = strdup(evt->request->from->displayname);
                call->ip = strdup(sdp_audio_conn->c_addr);

                call->port = atoi(sdp_audio_media->m_port);
                call->cid = evt->cid;
                call->tid = evt->tid;
                call->did = evt->did;
                call->status = CALL_RINGING;
                call->ringing_timer = time(NULL);
                call->r_session = NULL;
                call->next = NULL;

                /* prepare test song file */
                call->user_ts = 0;
                call->song = fopen(WAVFILE, "rb");

                if(call->song == NULL) {
                    fprintf(stderr, "[CALL_SONG] Cannot open file.\n");
                    exit(-1);
                }

                if(call_list == NULL)
                    call_list = call;
                else {
                    call_t* head = call_list;
                    while(head->next!=NULL) 
                        head=head->next;
                    head->next=call;
                }

                eXosip_lock (ctx);
                eXosip_call_send_answer (ctx, evt->tid, 180, NULL); 
                eXosip_unlock (ctx);
            } 
           
            sdp_message_free(sdp_packet);
            break;
        case EXOSIP_CALL_ACK:
            printf("[CALL_ACK] CID: %d\n", evt->cid);
            call = call_list;
            while(call != NULL) {
                if(call->cid == evt->cid) {
                    call->status = CALL_ACTIVE;
                }
                call = call->next;
            }
            break;
        case EXOSIP_CALL_CLOSED:
            printf("[CALL_CLOSED] CID: %d\n", evt->cid);
            call = call_list;
            while(call != NULL) {
                if(call->cid == evt->cid) {
                    call->status = CALL_CLOSED;
                }
                call = call->next;
            }
            break;
        default:
            printf("[DEBUG] Event %d\n", evt->type);
            break;
    }

    eXosip_event_free(evt);
}

int sip_init() {
    int i, val;

    ctx = eXosip_malloc();
    if(!ctx)
        return -1;

    i = eXosip_init(ctx);
    if(i)
        return -1;

    i = eXosip_listen_addr(ctx, IPPROTO_UDP, NULL, port, AF_INET, 0);
    if(i) {
        eXosip_quit(ctx);
        fprintf(stderr, "Could not init tr layer\n");
        return -1;
    }

    eXosip_set_user_agent(ctx, "julietta");

    val = 3600;
    eXosip_set_option(ctx, EXOSIP_OPT_UDP_KEEP_ALIVE, (void*)&val);
    return 1;
}

int sip_register(char* account, char* host, char* login, char* passwd) {
    osip_message_t* reg = NULL;
    int i;

    eXosip_lock(ctx);

    register_id = eXosip_register_build_initial_register(ctx, account, host, NULL, REG_TIMEOUT, &reg);

    if(register_id < 0) {
        eXosip_unlock(ctx);
        return -1;
    }

    eXosip_add_authentication_info(ctx, login, login, passwd, NULL, NULL);
    osip_message_set_supported(reg, "100rel");
    osip_message_set_supported(reg, "path");


    i = eXosip_register_send_register(ctx, register_id, reg);

    eXosip_unlock(ctx);
    return i;
}

int sip_reg_update() {
	osip_message_t* reg = NULL;
	int i;
	if(register_id != 0) {
		eXosip_lock(ctx);
		i = eXosip_register_build_register(ctx, register_id, REG_TIMEOUT, &reg);
		if(i<0) {
			eXosip_unlock(ctx);
			return -1;
		}

		eXosip_register_send_register(ctx, register_id, reg);
		eXosip_unlock(ctx);
	}
	return 0;
}

int sip_reg_delete() {
    osip_message_t *reg = NULL;
    int i;

    if(register_id != 0) {
        eXosip_lock (ctx);
		printf("[DEBUG] Unregistering..\n");
        i = eXosip_register_build_register (ctx, register_id, 0, &reg);
        if (i < 0) {
            eXosip_unlock (ctx);
            return -1;
        }
        eXosip_register_send_register (ctx, register_id, reg);
        eXosip_unlock (ctx);

        register_id = 0;
    }
    return 0;
}

int sip_answer_call(call_t* call) {
    char localip[128] = { 0 };
	int i;
	osip_message_t* answer = NULL;

    eXosip_guess_localip(ctx, AF_INET, localip, 127);

	eXosip_lock (ctx);
	i = eXosip_call_build_answer (ctx, call->tid, 200, &answer);
	if (i != 0)
		eXosip_call_send_answer (ctx, call->tid, 400, NULL);
	else
	{
		call->r_session = rtp_session_new(RTP_SESSION_SENDRECV);
		rtp_session_set_blocking_mode(call->r_session, 1);
		rtp_session_set_scheduling_mode(call->r_session, 1);
		rtp_session_set_connected_mode(call->r_session, 1);
		rtp_session_set_payload_type(call->r_session, 0); /* 0 = pcmu8000??*/

		rtp_session_set_local_addr(call->r_session, localip, 10500, 0);
		rtp_session_set_remote_addr(call->r_session, call->ip, call->port);

		i = sdp_complete_200ok (ctx, call->did, answer, localip, 10500);
		if (i != 0)
		{
			osip_message_free (answer);
			eXosip_call_send_answer (ctx, call->tid, 415, NULL);
		}
		else
			eXosip_call_send_answer (ctx, call->tid, 200, answer);
	}
	eXosip_unlock (ctx);
    return 1;
}

