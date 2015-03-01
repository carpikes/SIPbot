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
 * @file cmds.h
 * @brief Commands
 */
#ifndef CMDS_H
#define CMDS_H

#include "common.h"
#include "call.h"

typedef struct {
    /** @brief Command string */
    const char * cmd;

    /** @brief Function to call */
    void (*func)(call_t *call, char *arg);
} cmd_t;

/* Command prototypes */

void cmd_play(call_t *call, char *arg);
void cmd_stop(call_t *call, char *arg);
void cmd_kill(call_t *call, char *arg);

#endif
