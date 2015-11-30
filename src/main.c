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
 * @file main.c
 * @brief Main
 */
#include "common.h"
#include "sip.h"
#include "call.h"
#include "config.h"

static volatile int stop_event = 0;

void signal_handler(int s);
void config_handler(int s);
void signal_init(void);
void signal_halt(void);
void show_usage(char *argv[]);

#define DEFAULT_CONFIG_FILE "sipbot.ini"

/**
 *  Entry point
 */
int main (int argc, char *argv[]) {
    const char *config_file = DEFAULT_CONFIG_FILE;
    int c;

    log_debug("SIPBOT", "Starting up");

    while ((c = getopt (argc, argv, "hc:")) != -1) {
        switch(c) {
            case 'h':
                show_usage(argv);
                return 0;
            case 'c':
                config_file = optarg;
                break;
        } 
    }

    srand(time(NULL));    
	ortp_init();
	ortp_scheduler_init();

    rtp_profile_set_payload(&av_profile, 101, &payload_type_telephone_event);
    ortp_set_log_level_mask(ORTP_MESSAGE|ORTP_WARNING|ORTP_ERROR);

    if(config_init(config_file) == -1) {
        log_err("MAIN", "Cannot read config. Exiting..");
        exit(1);
    }

    if(sip_init() == -1) {
        log_err("MAIN", "SIP_INIT returned -1");
        exit(1);
    }
	
	if(sip_register(
	        config_readstring(CONFIG_REGINFO), 
	        config_readstring(CONFIG_REGHOST), 
	        config_readstring(CONFIG_REGUSER), 
	        config_readstring(CONFIG_REGPASS)) == -1) {
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
	config_free();

    signal_halt();
    return 0;
}

void signal_handler(int s) {
    stop_event = 1;
    signal(s, signal_handler);
}

void config_handler(int s) {
    signal(s, SIG_IGN);
    config_reload(); 
    signal(s, config_handler);
}

void signal_init(void) {
    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);
    signal(SIGQUIT, signal_handler);
    signal(SIGHUP, config_handler);
}

void signal_halt(void) {
    signal(SIGINT, 0);
    signal(SIGTERM, 0);
    signal(SIGQUIT, 0);
    signal(SIGHUP, 0);
}

void show_usage(char *argv[]) {
    printf( "Usage: %s [options]\n"
            "  -c <config file>   use this configuration file\n"
            "  -h                 show this message\n", argv[0]);
}
