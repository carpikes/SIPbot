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

void log_debug(const char* tag, const char* str, ...) {
    va_list ap;
    if( !str ) return;

    fprintf(stdout, "[DEBUG] %s: ", tag);
    va_start(ap, str);
    vfprintf(stdout, str, ap );
    fprintf(stdout, "\n" );
    va_end(ap);
}
void log_err(const char* tag, const char* str, ...) {
    va_list ap;
    if( !str ) return;

    fprintf(stderr, "[ERROR] %s: ", tag);
    va_start(ap, str);
    vfprintf(stderr, str, ap );
    fprintf(stderr, "\n" );
    va_end(ap);
}
