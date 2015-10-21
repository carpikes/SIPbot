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
 * @file log.c
 * @brief Logging functions
 */
#include "log.h"

void log_write(int level, const char* tag, const char* str, ...) {
    va_list ap;
    FILE *file = stdout;
    time_t t;
    struct tm *tt;

    if( !str ) return;

    time(&t);
    tt = localtime(&t);
    if(LOG_LEVEL >= level) {
        if(level == 0)
            file = stderr;
        fprintf(file, "[%04d-%02d-%02d %02d:%02d:%02d] %s: ", 
                tt->tm_year + 1900, tt->tm_mon, tt->tm_mday, 
                tt->tm_hour, tt->tm_min, tt->tm_sec, tag);
        va_start(ap, str);
        vfprintf(file, str, ap );
        fprintf(file, "\n" );
        va_end(ap);
    }
}
