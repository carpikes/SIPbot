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
 * @file test_wave.c
 * @brief WaveOpen and WaveRead test functions
 */

#include "../src/common.h"
#include "../src/filter.h"
#include "../src/law.h"

int main(int argc, char* argv[])
{
    wavfile_t* wav;
    char buf[500];
    printf(
        "******************\n"
        "*  PCM => muLAW  *\n"
        "******************\n");

    wav = waveopen("out.wav");
    if (!wav)
    {
        fprintf(stderr, "Cannot open out.wav\n");
        exit(-1);
    }

    while (1)
    {
        waveread(wav, buf, 500);
    }

    waveclose(wav);

    return 0;
}
