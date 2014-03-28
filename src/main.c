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
 * @file main.c
 * @brief Main
 */
#include "common.h"
#include "sip.h"
#include "call.h"

static volatile int stop_event = 0;

void signal_handler(int s);
void signal_init(void);
void signal_halt(void);

/**
 *  Entry point
 */
int main (int argc, char **argv) {

    log_debug("SIPBOT", "Starting up");

    srand(time(NULL));    
	ortp_init();
	ortp_scheduler_init();
    ortp_set_log_level_mask(ORTP_MESSAGE|ORTP_WARNING|ORTP_ERROR);

    if(sip_init() == -1) {
        log_err("MAIN", "SIP_INIT returned -1");
        exit(1);
    }
	
	if(sip_register(REG_INFO, REG_HOST, REG_USER, REG_PASS) == -1) {
		log_err("MAIN", "SIP_REGISTER returned -1");
		exit(1);
	}

    signal_init();	
    log_debug("MAIN", "Listening");
    while(!stop_event) {
        call_update();
        sip_update();
    }

    log_debug("MAIN", "Exiting");
    call_freeall();
    sip_exit();
	ortp_exit();

    signal_halt();
    return 0;
}

void signal_handler(int s) {
    stop_event = 1;
    signal(s, signal_handler);
}

void signal_init(void) {
    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);
    signal(SIGQUIT, signal_handler);
}

void signal_halt(void) {
    signal(SIGINT, 0);
    signal(SIGTERM, 0);
    signal(SIGQUIT, 0);
}

