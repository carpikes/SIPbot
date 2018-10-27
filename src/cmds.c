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
 * @file cmds.c
 * @brief Commands
 */
#include "cmds.h"

void cmd_append(call_t *call, char *arg)
{
    wavlist_t *head;
    wavlist_t *el;

    el = (wavlist_t *)malloc(sizeof(wavlist_t));
    if (!el)
    {
        log_err("CMD_APPEND", "Not enough memory");
        exit(-1);
    }
    el->wav  = waveopen(arg);
    el->next = NULL;

    if (!el->wav)
    {
        log_err("CMD_APPEND", "Cannot find %s", arg);
        free(el);
        return;
    }

    head = call->exec.list;
    if (head)
    {
        while (head->next)
            head = head->next;

        head->next = el;
    }
    else
        call->exec.list = el;
}

void cmd_play(call_t *call, char *arg)
{
    cmd_stop(call, NULL);
    cmd_append(call, arg);
}

void cmd_stop(call_t *call, char *arg)
{
    wavlist_t *el;
    while ((el = call->exec.list))
    {
        call->exec.list = el->next;

        if (el->wav)
            waveclose(el->wav);
        free(el);
    }
}

void cmd_kill(call_t *call, char *arg)
{
    cmd_stop(call, NULL);
    call->status = CALL_CLOSED;
}
