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
 * @file filter.h
 * @brief Audio Filters
 */

#ifndef FILTER_H
#define FILTER_H

#include "common.h"

void lowpass(int16_t *in, int16_t *out, int n, float a); 
void highpass(int16_t *in, int16_t *out, int n, float a); 
void downsample(int16_t *in, int16_t *out, int in_size, int in_rate, int out_rate); 
#endif
