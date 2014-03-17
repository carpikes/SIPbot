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

#ifndef SIP_H
#define SIP_H

#include "common.h"

enum e_calls{
    CALL_RINGING, 
    CALL_ACTIVE,
    CALL_CLOSED
};

struct call {
    int cid, tid, did; /* did is used as call identifier*/

    char* caller;
    char* ip;
    int port;
    int status;
    time_t ringing_timer;

    RtpSession* r_session;
    FILE* song;
    int user_ts;

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
