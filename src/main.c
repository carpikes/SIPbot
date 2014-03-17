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

#include "common.h"
#include "sip.h"

static volatile int stop_event = 0;

char my_ip[64]={0};

int get_my_ip(char *buf, int max_len);

void signal_handler(int s);
void signal_init();
void signal_halt();

int main (int argc, char **argv) {
	
    printf("Starting up..\n");

	ortp_init();
	ortp_scheduler_init();
    ortp_set_log_level_mask(ORTP_MESSAGE|ORTP_WARNING|ORTP_ERROR);

    if(sip_init() == -1) {
        fprintf(stderr, "[ERROR] SIP_INIT returned -1\n");
        exit(1);
    }
	
	if(sip_register(REG_INFO, REG_HOST, REG_USER, REG_PASS) == -1) {
		fprintf(stderr, "[ERROR] SIP_REGISTER returned -1\n");
		exit(1);
	}

    signal_init();	
    printf("[DEBUG] Listening...\n");
    while(!stop_event) {
        call_update();
        sip_update(my_ip);
    }

    printf("[DEBUG] Exiting...\n");
    call_freeall();
    sip_exit();
	ortp_exit();

    signal_halt();
    return 0;
}

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
        printf("Invalid host\n");
		return 0;
    }

	h_addr.s_addr = *((unsigned long *) host->h_addr);

	params.sin_port = htons(80);
	params.sin_addr = h_addr;
	params.sin_family = AF_INET;

	s = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if(!s) {
        printf("Cannot create socket.\n");
		return 0;
    }

	if( connect(s, (struct sockaddr *) &params, sizeof(params)) < 0) { 
        printf("Cannot connect\n");
        close(s);
		return 0;
    }

	send(s, send_buf, strlen(send_buf), 0);

	n = recv(s, recv_buf, 8192-1, 0);
	close(s);
	if(n==-1) {
        printf("Recv error\n");
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

