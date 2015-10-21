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
 * @file log.h
 * @brief Logging headers
 */
#ifndef LOG_H
#define LOG_H
#include "common.h"

#define L_DEBUG   2
#define L_INFO    1
#define L_ERROR   0

#define log_debug(a,...) log_write(L_DEBUG, a, __VA_ARGS__)
#define log_info(a,...) log_write(L_INFO, a, __VA_ARGS__)
#define log_err(a,...) log_write(L_ERROR, a, __VA_ARGS__)
void log_write(int level, const char* tag, const char* str, ...);
#endif
