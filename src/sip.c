/* SIPbot - An opensource VoIP answering machine
 * Copyright (C) 2014-2015 Alain (Carpikes) Carlucci
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
 * @file sip.c
 * @brief SIP and Call Management functions
 */

#include "sip.h"
#include "sdp.h"

static int              port = 5060;
static int              register_id = NULL;
static struct eXosip_t* ctx = NULL;
static time_t           reg_timer = 0;


/**
 * This function frees osip2 used memory
 */
void sip_exit(void) {
    sip_reg_delete();
    eXosip_quit(ctx);
    free(ctx);
}
/**
 * This function handles oSIP messages
 */
void sip_update(void) {
    sdp_message_t* sdp_packet;
    sdp_connection_t* sdp_audio_conn;
    sdp_media_t* sdp_audio_media;

    eXosip_event_t* evt;
    int ret;
    time_t cur_time = time(NULL);

    if(reg_timer > 0 && cur_time - reg_timer > REG_TIMEOUT) {
        log_debug("SIP_UPDATE", "Updating registration");
        sip_reg_update();
        reg_timer = 0; /* prevent multiple sip_reg_update() calls */
    }

    evt = eXosip_event_wait(ctx, 0, 50);
    eXosip_lock(ctx);
    eXosip_automatic_action(ctx);
    eXosip_unlock(ctx);

    if(evt == NULL)
        return;

    switch(evt->type) {
        case EXOSIP_REGISTRATION_FAILURE:
            log_debug("SIP_UPDATE", "Reg fail");
            break;
        case EXOSIP_REGISTRATION_SUCCESS:
            reg_timer = time(NULL);
            log_debug("SIP_UPDATE", "Client registered");
            break;
        case EXOSIP_CALL_INVITE:
            log_debug("SIP_UPDATE", "Received CALL_INVITE from %s", 
                    evt->request->from->displayname);
            sdp_packet = eXosip_get_remote_sdp(ctx, evt->did);

            if(sdp_packet == NULL) {
                log_err("SIP_UPDATE", 
                        "I've received a CALL_INVITE without SDP infos!");
                break;
            }

            sdp_audio_conn = eXosip_get_audio_connection(sdp_packet);
            sdp_audio_media = eXosip_get_audio_media(sdp_packet);

            if(sdp_audio_conn != NULL && sdp_audio_media != NULL
                    && strlen(sdp_audio_conn->c_addr) > 0) {

                ret = call_new(evt->request->from->displayname,
                        sdp_audio_conn->c_addr,
                        atoi(sdp_audio_media->m_port),
                        evt->cid,
                        evt->tid,
                        evt->did);

                if(ret) {
                    eXosip_lock (ctx);
                    eXosip_call_send_answer (ctx, evt->tid, 180, NULL); 
                    eXosip_unlock (ctx);
                }
            } 

            sdp_message_free(sdp_packet);
            break;
        case EXOSIP_CALL_ACK:
            log_debug("SIP_UPDATE", 
                    "Call %d got CALL_ACK, starting stream...", evt->cid);
            ret = call_set_status(evt->cid, CALL_ACTIVE);
            if(!ret)
                sip_terminate_call(evt->cid, evt->did);
            break;
        case EXOSIP_CALL_CLOSED:
            log_debug("SIP_UPDATE", "Call %d closed", evt->cid);
            call_set_status(evt->cid, CALL_CLOSED);
            break;
        default:
            log_debug("SIP_UPDATE", 
                    "Got unknown event %d. Ignoring.", evt->type);
            break;
    }

    eXosip_event_free(evt);
}

/**
 * This function initializes libeXosip2
 */
int sip_init(void) {
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
        log_err("SIP_INIT", "Could not listen. Another client is opened?");
        return -1;
    }

    eXosip_set_user_agent(ctx, "julietta");

    val = 3600;
    eXosip_set_option(ctx, EXOSIP_OPT_UDP_KEEP_ALIVE, (void*)&val);
    return 1;
}

/**
 * This function tries to register this client with VoIP provider
 *
 * @param account SIP account
 * @param host SIP host
 * @param login SIP username
 * @param passwd SIP password
 * @return A number >=0 if this packet was built and sent
 */
int sip_register(char* account, char* host, char* login, char* passwd) {
    osip_message_t* reg = NULL;
    int i;

    eXosip_lock(ctx);

    register_id = eXosip_register_build_initial_register(
            ctx, account, host, NULL, REG_TIMEOUT, &reg);

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

/**
 * This function updates client registration with VoIP provider
 *
 * @return 1 if there are no errors
 */
int sip_reg_update() {
    osip_message_t* reg = NULL;
    int i;
    if(register_id != 0) {
        eXosip_lock(ctx);
        i = eXosip_register_build_register(ctx, register_id, REG_TIMEOUT, &reg);
        if(i<0) {
            eXosip_unlock(ctx);
            return 0;
        }

        eXosip_register_send_register(ctx, register_id, reg);
        eXosip_unlock(ctx);
    }
    return 1;
}
/**
 * Cancels a provider registration
 */
int sip_reg_delete() {
    osip_message_t *reg = NULL;
    int i;

    if(register_id != 0) {
        eXosip_lock (ctx);
        log_debug("SIP_REG_DELETE", "Unregistering");
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

void recv_tev_cb(RtpSession *session, int type, unsigned long user_data) {
    char buf[32];
    call_t *call = (call_t *)(user_data);

    memset(buf, 0, sizeof(buf));
    sprintf(buf, "DTMF %d\n", type);
    write(call->exec.wfd, buf, strlen(buf));

    log_debug("DTMF", "Pressed %d", type);
}

/**
 * This function is called when SIPbot answers a call
 *
 * @param call call info
 * @return 1 if call is answered successfully
 */
int sip_answer_call(call_t* call) {
    int retval=0,i;
    char localip[128] = { 0 };
    osip_message_t* answer = NULL;
    int jittcomp = 50;
    bool_t adapt = TRUE;

    eXosip_guess_localip(ctx, AF_INET, localip, 127);

    eXosip_lock (ctx);
    i = eXosip_call_build_answer (ctx, call->tid, 200, &answer);
    if (i != 0)
        eXosip_call_send_answer (ctx, call->tid, 400, NULL);
    else
    {
        call->r_session = rtp_session_new(RTP_SESSION_SENDRECV);
        rtp_session_set_scheduling_mode(call->r_session, 1);
        rtp_session_set_blocking_mode(call->r_session, 0);
        rtp_session_set_payload_type(call->r_session, 0); 
        rtp_session_enable_adaptive_jitter_compensation(call->r_session,adapt);
        rtp_session_set_jitter_compensation(call->r_session,jittcomp);

        rtp_session_set_local_addr(call->r_session, localip, 10500, 0);
        rtp_session_set_remote_addr(call->r_session, call->ip, call->port);

        rtp_session_signal_connect(call->r_session, "telephone-event", (RtpCallback) recv_tev_cb, (void *) call);

        i = sdp_complete_200ok (ctx, call->did, answer, localip, 10500);
        if (i != 0)
        {
            osip_message_free (answer);
            eXosip_call_send_answer (ctx, call->tid, 415, NULL);
        }
        else {
            eXosip_call_send_answer (ctx, call->tid, 200, answer);
            retval = 1;
        }
    }
    eXosip_unlock (ctx);
    return retval;
}

/**
 * Terminate a SIP call 
 *
 * @param cid eXosip Call ID
 * @param did eXosip Dialog ID
 */
void sip_terminate_call(int cid, int did) {
    eXosip_call_terminate(ctx, cid, did);
}
