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

static volatile int stop_event = 0;

char my_ip[64]={0};

int get_my_ip(char *buf, int max_len);

void signal_handler(int s);
void signal_init();
void signal_halt();

/**
 *  Entry point
 */
int main (int argc, char **argv) {

    log_debug("MAIN", "Starting up");

    
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
        sip_update(my_ip);
    }

    log_debug("MAIN", "Exiting");
    call_freeall();
    sip_exit();
	ortp_exit();

    signal_halt();
    return 0;
}

/**
 * Obtain your external IP Address
 * (Actually, a useless function)
 * 
 * @param buf Output buffer
 * @param max_len Output buffer length
 * @return If this function succeded, return value is 1. Otherwise it is 0
 */
int get_my_ip(char *buf, int max_len) {
    static char send_buf[] =   
        "GET / HTTP/1.1\r\n"
		"Host: checkip.dyndns.org\r\n"
        "Connection: close\r\n"
        "\r\n";
	
    int s, max, n;
    char recv_buf[8192], *p, *p2;
	struct hostent *host;
	struct in_addr h_addr;
	struct sockaddr_in params;

	host = gethostbyname("checkip.dyndns.org");
	if(host == NULL) {
        log_err("GETIP", "Invalid host");
		return 0;
    }

	h_addr.s_addr = *((unsigned long *) host->h_addr);

	params.sin_port = htons(80);
	params.sin_addr = h_addr;
	params.sin_family = AF_INET;

	s = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if(!s) {
        log_err("GETIP", "Cannot create socket");
		return 0;
    }

	if( connect(s, (struct sockaddr *) &params, sizeof(params)) < 0) { 
        log_err("GETIP","Cannot connect");
        close(s);
		return 0;
    }

	send(s, send_buf, strlen(send_buf), 0);

	n = recv(s, recv_buf, 8192-1, 0);
	close(s);
	if(n==-1) {
        log_err("GETIP", "Recv error");
		return 0;
    }

	memset(buf, 0, max_len);
	p = strstr(recv_buf, "Address: ");
	if(p != NULL) {
		p += strlen("Address: ");
		p2 = strstr(p, "<");
		if(p2 != NULL) {
			max = p2-p;
			if(max_len-1<max) 
                max = max_len - 1;
			strncpy(buf, p, max);
			return 1;
		}
	}
	return 0;
}

void signal_handler(int s) {
    stop_event = 1;
    signal(s, signal_handler);
}

void signal_init() {
    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);
    signal(SIGQUIT, signal_handler);
}

void signal_halt() {
    signal(SIGINT, 0);
    signal(SIGTERM, 0);
    signal(SIGQUIT, 0);
}

