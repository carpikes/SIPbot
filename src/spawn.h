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
 * @file spawn.h
 * @brief Launch a program with bidirectional pipes
 */

#ifndef SPAWN_H
#define SPAWN_H

#include "common.h"
#include "law.h"

typedef struct wavlist {
    wavfile_t *wav;
    struct wavlist *next;
} wavlist_t;

typedef struct {
    /** @brief File descriptors */
    int rfd, wfd, efd;

    /** @brief PID */
    int pid;

    /** @brief Audio files */
    wavlist_t *list;

    /** @bried Temp command */
    char *temp;
} spawn_t;

pid_t spawn(const char *cmd, spawn_t *out);
void sclose(spawn_t *cmd);

#endif
