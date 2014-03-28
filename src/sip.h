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
#include "call.h"

void sip_exit(void);
int sip_init(void);
void sip_update(void);
int sip_register(char* account, char* host, char* login, char* passwd);
int sip_reg_update(void);
int sip_reg_delete(void);
int sip_answer_call(call_t* call);
void sip_terminate_call(int cid, int did);

#endif
