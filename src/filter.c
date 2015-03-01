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
 * @file filter.c
 * @brief Audio Filters
 */
#include "filter.h"
/* Thanks to Wikipedia for these formulas */

/**
 * Lowpass Filter
 *
 * @param in Audio data that should be filtered (input)
 * @param out Filtered audio data (output)
 * @param n Number of samples, audio data size
 * @param a e^(-2.2/F) where F is the cutoff frequency
 */
void lowpass(int16_t *in, int16_t *out, int n, float a) {
    int i;

    out[0]=in[0];
    for(i=1;i<n;i++)
        out[i] = out[i-1] + a*(in[i] - out[i-1]);
}

/**
 * Highpass Filter
 *
 * @param in Audio data that should be filtered (input)
 * @param out Filtered audio data (output)
 * @param n Number of samples, audio data size
 * @param a e^(-2.2/F) where F is the cutoff frequency
 */
void highpass(int16_t *in, int16_t *out, int n, float a) {
    int i;
    out[0]=in[0];

    for(i=1;i<n;i++)
        out[i] = a * (out[i-1] + in[i] - in[i-1]);
}

/**
 * Downsample audio stream
 *
 * @param in input buffer
 * @param out output buffer (downsampled audio)
 * @param in_size input buffer size
 * @param in_rate input rate (e.g. 44100)
 * @param out_rate output rate (8000)
 */
void downsample(int16_t *in, int16_t *out, 
    int in_size, int in_rate, int out_rate) {
    int i,j;
    int out_step = ceil(in_rate/out_rate);
    int out_size = in_size*(float)out_rate/in_rate;

    for(i=0;i<out_size;i++) {
        out[i]=0;
        for(j=0;j<out_step;j++) 
            out[i] += (float) 
                      in[(int)(i*((float)in_rate/out_rate)+j)] / out_step;
    }
}

