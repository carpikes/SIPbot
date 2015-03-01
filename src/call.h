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
 * @file call.h
 * @brief Call manager
 */

#ifndef CALL_H
#define CALL_H

#include "common.h"
#include "log.h"
#include "law.h"
#include "spawn.h"

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

    /** @brief Executable handler */
    spawn_t exec;

    /** @brief RTP sent stream timestamp  */
    int send_ts;

    /** @brief RTP recv stream timestamp */
    int recv_ts;

    /** @brief Pointer to the next call */
    struct call* next;
}; 

typedef struct call call_t;


/* Functions prototypes */

void call_transmit_data(call_t* call);
void call_receive_data(call_t* call);

void call_update(void);

void call_freeall(void);
call_t* call_free(call_t* call);


int call_new(const char* display_name, const char* rtp_addr, 
        int rtp_port, int cid, int tid, int did);

int call_set_status(int cid, int status);

#endif
