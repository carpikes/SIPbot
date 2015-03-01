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
 * @file spawn.c
 * @brief Launch a program with bidirectional pipes
 */

#include "spawn.h"

pid_t spawn(const char *cmd, spawn_t* out) {
    int wpipe[2], rpipe[2], epipe[2];
    pid_t pid;

    if (pipe(wpipe) || pipe(rpipe) || pipe(epipe))
        return -1;

    if ((pid = fork()) < 0)
        return pid;

    if (!pid) {
        close(rpipe[0]);
        close(wpipe[1]);
        close(epipe[0]);

        dup2(rpipe[1], 1);
        dup2(wpipe[0], 0);
        dup2(epipe[1], 2);

        execl("/bin/bash", "bash", "-c", cmd, NULL);
        perror("execl");
        exit(1);
    }

    close(rpipe[1]);
    close(wpipe[0]);
    close(epipe[1]);

    out->rfd = rpipe[0];
    out->wfd = wpipe[1];
    out->efd = epipe[0];
    out->wav = NULL;

    return pid;
}

void sclose(spawn_t *cmd) {
    close(cmd->rfd);
    close(cmd->wfd);
    close(cmd->efd);
    
    if(cmd->wav != NULL)
        waveclose(cmd->wav);
} 
