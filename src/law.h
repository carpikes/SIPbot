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
 * @file law.h
 * @brief muLaw Encoder
 */

#ifndef LAW_H
#define LAW_H

#include "common.h"
#include "log.h"
#include "filter.h"

#pragma pack(2)

typedef struct {
    uint32_t name;
    uint32_t remaining;
    uint32_t type;   
} riffhdr_t;

typedef struct {
    uint32_t name;
    uint32_t size;
} wavechunk_t;

typedef struct {
    uint16_t format;
    uint16_t num_chans;
    uint32_t sample_rate;
    uint32_t bytes_per_sec;
    uint16_t block_align;
    uint16_t bits_per_sample;
} wavefmt_t;

typedef struct wavfile {
    FILE*       file;
    wavefmt_t   header;
} wavfile_t;


#pragma pack() 

wavfile_t*  waveopen(char* file_name);
void        waveclose(wavfile_t* wavfile);
int         waveread(wavfile_t* wavfile, char* output, int len);

uint8_t     linear2ulaw(int16_t sample);

#endif
