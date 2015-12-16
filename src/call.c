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
#include "cmds.h"
#include "config.h"

#define IN_BUFSIZE  8192
#define RTP_SENDBUF 512
#define RTP_RECVBUF 160

/**
 * Active calls
 */
static call_t* call_list = NULL;

extern int current_calls;

/**
 * Command table
 *
 * Each command is 4 characters long! 
 */
cmd_t commands[] = {
    {"PLAY", cmd_play},
    {"APND", cmd_append},
    {"STOP", cmd_stop},
    {"KILL", cmd_kill},
    {NULL, NULL},
};

/**
 * Read a stderr line from the called program
 *
 * @param call The call
 */
int parse_error(call_t* call) {
    char buf[128];
    int n, len;

    n = read(call->exec.efd, buf, 128);
    if(n > 0) {
        len = strlen(buf);
        if(len > 0) {
            buf[len-1] = 0;
            len--;
        }

        log_debug("PARSE", "ERROR: %s\n", buf);
        return 1;
    }
    return 0;
}

/**
 * Parse a command
 *
 * @param call The call
 * @param line Line to parse
 */
void parse_command(call_t* call, char *line) {
    cmd_t *cur = commands;
    char cmd[5];
    char *args = NULL;
    size_t len;

    len = strlen(line);

    /* Due to semplicity, each cmd is 4 char long */
    if(len >= 4) {
        memcpy(cmd, line, 4);
        cmd[4] = 0;

        if(len > 5) {
            if(line[4] != ' ')
                return;
            args = (char *) malloc(len - 5 + 1);
            memcpy(args, line + 5, len - 5);
            args[len-5] = 0;
        } 

        while(cur->cmd != NULL) {
            if(!strcmp(cmd, cur->cmd)) {
                log_debug("COMMAND", "Triggering %s (%s)", cmd, (args?args:"<NULL>"));
                
                cur->func(call, args);
                break;
            }

            cur++;
        }

        if(cur->cmd == NULL)
            log_debug("COMMAND", "Unknown input: %s\n", line);

        if(args)
            free(args);
    }
}

/**
 * Read data from the called program and parse each line
 *
 * @param call The call
 */
int parse_input(call_t* call) {
    char buf[IN_BUFSIZE];
    char line[IN_BUFSIZE];
    int i, j, n, len;

    memset(buf, 0, sizeof(buf));
    memset(line, 0, sizeof(line));
    n = read(call->exec.rfd, buf, IN_BUFSIZE);
    if(n > 0) {
        len = strlen(buf);

        if(call->exec.temp) {
            memcpy(line, call->exec.temp, strlen(call->exec.temp));
            free(call->exec.temp);
            call->exec.temp = NULL;
        }

        for(i=0,j=strlen(line);i<len && j<IN_BUFSIZE;i++) {
            if(buf[i] == '\n') {
                parse_command(call, line);
                j = 0;
                memset(line, 0, sizeof(line));
            }
            else if(buf[i]>=' ')
                line[j++] = buf[i];
        }

        if(j>0 && j<IN_BUFSIZE) {
            call->exec.temp = (char *) calloc(strlen(line),1);
            if(!call->exec.temp) {
                log_err("PARSE", "Not enough memory");
                exit(-1);
            }
            memcpy(call->exec.temp, line, strlen(line));
        }
        return 1;
    }
    return 0;
}

/**
 * This function is called when socket is ready to transmit
 * a new audio frame. 
 *
 * @param call The call
 */
void call_transmit_data(call_t* call) {
    wavlist_t *el;
    char send_buf[RTP_SENDBUF];
    int i;

    memset(send_buf, 0xFF, RTP_SENDBUF);

    /**
     * Send stream
     */
    while((el = call->exec.list)) {
        i = waveread(el->wav, send_buf, RTP_SENDBUF);
        if(i>=0)
            break;

        /* TODO: change these lines with wavelist_remove_head(..)*/
        call->exec.list = el->next; 
        if(el->wav)
            waveclose(el->wav);
        free(el);
        if(call->exec.list == NULL) 
            write(call->exec.wfd, "FNSH\n",5);
        
    }
    
    rtp_session_send_with_ts(call->r_session, (uint8_t*) send_buf, 
            RTP_SENDBUF, call->send_ts);
    call->send_ts += RTP_SENDBUF;
}

/**
 * This functions is called when socket is ready to receive
 * a new audio frame.
 *
 * @param call The call
 */
void call_receive_data(call_t* call) {
    uint8_t recv_buf[RTP_RECVBUF] = {0};
    int i;
    int must_recv_more = 0;

    do {
        /* Receive a piece of data */
        i = rtp_session_recv_with_ts(call->r_session, (uint8_t*) recv_buf, 
                RTP_RECVBUF, call->recv_ts, &must_recv_more);

        /* TODO: here you can use recv_buf for your own purpose */
    }while(must_recv_more && i>0);

    call->recv_ts += RTP_RECVBUF;
}

/**
 * This function closes the call and frees used memory.
 *
 * @param call call data to free
 * @return This function returns the call next to the deleted one
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
    sclose(&call->exec);

    next = call->next;
    free(call);

    --current_calls;

    return next;
}

/**
 * This function is called when SIPbot is closing
 * It frees and closes all calls.
 */
void call_freeall(void) {
    while(call_list != NULL)
        call_list = call_free(call_list);
}

/**
 * Cycles through all calls and update their status
 */
void call_update(void) {
    SessionSet *send_set, *recv_set;
    fd_set cmd_set;
    int cmd_max = 0;
    int num_socks=0, select_ret, update_list = 1;
    time_t cur_time = time(NULL);
    call_t* call = call_list;
    call_t* call_prec = NULL;

    send_set = session_set_new();
    recv_set = session_set_new();
    FD_ZERO(&cmd_set);
    while(call != NULL) {
        update_list = 1;
        switch(call->status) {
            case CALL_RINGING:
                if(call->ringing_timer > 0 && 
                        cur_time-call->ringing_timer > 2) {
                    /* Ringing timeout: answer the call */
                    sip_answer_call(call);
                    call->ringing_timer = -1;
                }
                break;
            case CALL_ACTIVE:
                session_set_set(send_set, call->r_session);
                session_set_set(recv_set, call->r_session);
                FD_SET(call->exec.rfd, &cmd_set);
                FD_SET(call->exec.efd, &cmd_set);
                cmd_max = (cmd_max > call->exec.rfd) ? cmd_max : call->exec.rfd;
                cmd_max = (cmd_max > call->exec.efd) ? cmd_max : call->exec.efd;
                num_socks++;
                break;
            case CALL_CLOSED:
                log_debug("CALL_CLOSED", "Freeing memory");

                if(call_prec == NULL)
                    call_list = call->next;
                else 
                    call_prec->next = call->next;

                call = call_free(call); 

                /* call_list is already incremented */
                update_list = 0;
                break;
        }

        if(update_list) {
            call_prec = call;
            call = call->next;
        }
    }

    if(num_socks > 0) {
        struct timeval tv;
        tv.tv_usec = 1000;
        tv.tv_sec = 0;
        select_ret = select(cmd_max, &cmd_set, NULL, NULL, &tv);

        if(select_ret > 0) {
            call = call_list;
            while(call != NULL) {
                if(call->status == CALL_ACTIVE) { 
                    if(FD_ISSET(call->exec.rfd, &cmd_set)) {
                        if(!parse_input(call)) {
                            log_debug("CALL_EXEC", "Program terminated");
                            call->status=CALL_CLOSED;
                        }
                    }

                    if(FD_ISSET(call->exec.efd, &cmd_set)) {
                        if(!parse_error(call)) {
                            log_debug("CALL_EXEC", "Program terminated");
                            call->status=CALL_CLOSED;
                        }
                    }
                }
            call = call->next;
            }
        }

        select_ret = session_set_select(recv_set, send_set, NULL);

        if(select_ret > 0) {
            call = call_list;

            while(call != NULL) {
                if(call->status == CALL_ACTIVE) {
                    if(session_set_is_set(recv_set, call->r_session))
                        call_receive_data(call);
                    if(session_set_is_set(send_set, call->r_session))
                        call_transmit_data(call);
                }

                call = call->next;
            }
        }
    }

    session_set_destroy(recv_set);
    session_set_destroy(send_set);
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
    char *tmp = NULL;
    const char *prog = config_readstring(CONFIG_PROGRAM);
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

    call->send_ts = rand();
    call->recv_ts = 0;

    if(spawn(prog, &call->exec) < 0) {
        log_debug("SIP_UPDATE", "Cannot open %s", prog);
        call_free(call);
        return 0;
    }

    tmp = (char *) calloc(strlen(call->caller) + 16, 1);
    sprintf(tmp, "CALL %s\n", call->caller);
    write(call->exec.wfd, tmp, strlen(tmp));
    free(tmp);

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
