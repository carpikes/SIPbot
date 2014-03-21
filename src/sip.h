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
 * @file sip.h
 * @brief call structure and sip.c definitions
 */

#ifndef SIP_H
#define SIP_H

#include "common.h"
#include "log.h"

enum e_calls{
    CALL_RINGING, 
    CALL_ACTIVE,
    CALL_CLOSED
};

struct call {

    /** @brief Call id */
    int cid;
    /** @brief Transaction id */
    int tid;
    /** @brief Dialog id */
    int did;

    /** @brief Caller's phone number */
    char* caller;
    
    /** @brief RTP server ip address */
    char* ip;

    /** @brief RTP server port */
    int port;

    /** @brief Call status */
    int status;

    /** @brief Ringing timer */
    time_t ringing_timer;

    /** @brief oRTP session data */
    RtpSession* r_session;

    /** @brief Song file handler */
    FILE* song;

    /** @brief RTP call packet count? */
    int user_ts;

    /** @brief Pointer to the next call */
    struct call* next;
}; 

typedef struct call call_t;



void sip_exit();
int sip_init();
void sip_update();
int sip_register(char*, char*, char*, char*);
int sip_reg_update();
int sip_reg_delete();
int sip_answer_call(call_t*);

void call_update();
void call_freeall();
call_t* call_free(call_t*);

#endif
