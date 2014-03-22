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
 * @file filter.c
 * @brief Audio Filters
 */

/* Thanks to Wikipedia for these formulas */

/**
 * Lowpass Filter
 *
 * @param in Audio data that should be filtered (input)
 * @param out Filtered audio data (output)
 * @param n Number of samples, audio data size
 * @param dt dt is 1/Sample Rate (e.g. 1/44100)
 * @param RC R*C constant, cut off frequency: 1/(2*PI*RC)
 */
void lowpass(int16_t *in, int16_t *out, int n, float dt, float RC) {
    int i;
    float a = dt/ (RC+dt);

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
 * @param dt dt is 1/Sample Rate (e.g. 1/44100)
 * @param RC R*C constant, cut off frequency: 1/(2*PI*RC)
 */
void highpass(int16_t *in, int16_t *out, int n, float dt, float RC) {
    int i;
    float a= dt/(RC+dt);
    out[0]=in[0];

    for(i=1;i<n;i++)
        out[i] = a * (out[i-1] + in[i] - in[i-1]);
}

