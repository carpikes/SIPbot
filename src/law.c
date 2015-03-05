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
 * @file law.c
 * @brief muLaw Encoder
 */

#include "law.h"
#include "filter.h"
/**
 * Open a wave file.
 *
 * @param file_name Filename to open
 * @return wavfile_t File pointer.
 */ 
wavfile_t* waveopen(char* file_name) {
    wavechunk_t chunk = {0};
    riffhdr_t riff = {0};
    wavfile_t* wav = NULL;
   
    wav = (wavfile_t*) calloc(1, sizeof(wavfile_t));
    if(!wav) 
        return NULL;

    if(!(wav->file = fopen(file_name, "rb"))) {
        free(wav);
        log_err("WAVEOPEN", "Cannot find: %s", file_name);
        return NULL;
    }

    if( fread(&riff, sizeof(riffhdr_t), 1, wav->file) != 1)
        goto fail;

    if(memcmp(&riff.name, "RIFF", 4) || memcmp(&riff.type, "WAVE", 4))
        goto fail;

    for(;;) {
        if(fread(&chunk, sizeof(wavechunk_t), 1, wav->file) != 1)
            goto fail;

        if(!memcmp(&chunk.name, "fmt ", 4)) {
            if(fread(&wav->header, sizeof(wavefmt_t), 1, wav->file) != 1) {
                log_err("WAVEOPEN", "Cannot read header");
                goto fail;
            }

            while(chunk.size>sizeof(wavefmt_t)) {
                fgetc(wav->file);
                chunk.size--;
            }

            /* LETTURA HEADER */
            if( wav->header.bits_per_sample > 32 || 
                wav->header.bits_per_sample == 0 ) {

                log_err("WAVEOPEN", "%d bits per sample? Something is wrong",
                        wav->header.bits_per_sample);
                goto fail;
            }

        }
        else if(!memcmp(&chunk.name, "data", 4)) {
            if(wav->header.num_chans>0)
                return wav;

            log_err("WAVEOPEN", "This WAVE file seems have 0 audio channels");
            goto fail;
        }
        else /* skip others */
            fseek(wav->file, chunk.size+((chunk.size%2)?1:0), SEEK_CUR);
        
    }

fail:
    fclose(wav->file);
    free(wav);
    return NULL;
}

/**
 *  Close a recently opened Wave File
 *
 *  @param wavfile File to close
 */
void waveclose(wavfile_t* wavfile) {
    if(wavfile != NULL) {
        fclose(wavfile->file);
        free(wavfile);
    }
}

/**
 * Read some bytes from Wave File and covnvert them in muLaw format
 *
 * @param wavfile Wave File pointer
 * @param output Output buffer
 * @param len Size of the output buffer
 */
int waveread(wavfile_t* wavfile, char* output, int len) {
    int i,n;
    int samples_num = (len * wavfile->header.sample_rate/8000);
    int bps = wavfile->header.bits_per_sample/8;
    char *piece, *buf;
    int16_t *f_in, *f_out, *f_dsp;
    
    buf = (char*) calloc(samples_num, bps);
    piece = (char*) calloc(1, bps);
    f_in = (int16_t*) calloc(samples_num, 2);
    f_out = (int16_t*) calloc(samples_num, 2);
    f_dsp = (int16_t*) calloc(len, 2);

    if(!buf || !piece || !f_in || !f_out || !f_dsp) {
        log_err("WAVEREAD", "Cannot allocate memory");
        exit(-1);
    }

    /**
     * Read data from file
     */
    n = fread(buf, bps, samples_num, wavfile->file);
    if(n<=0) {
        i = -1;
        goto fail;
    }

    /**
     * Convert data to int16
     */
    for(i=0;i<samples_num;i++) {

        /* if audio file is stereo */
        if(wavfile->header.num_chans == 2) {
            /* TODO: implement stereo mixingg. */
            log_err("WAVEREAD", "Function not implemented yet.");
            i = -1;
            goto fail;
        }


        memcpy(piece, &buf[i*bps], bps);
        switch(wavfile->header.bits_per_sample) {
            case 8: 
                f_in[i] = (int16_t) ((*(int8_t *)piece) - 0x80) << 8; 
                break;
            case 16: 
                f_in[i] = *(int16_t *)piece; 
                break;
            case 32: 
                f_in[i] = ((*(int32_t *)piece) >> 16); 
                break;
            default: 
                log_err("WAVEREAD", "Unsupported wave (%d bits)",
                        wavfile->header.bits_per_sample);
                i = -1;
                goto fail;
        }
    }

    /** 
     * Bandwidth filtering.
     *
     * High pass filter (antialiasing) + Low pass filter
     * (low frequencies may create some noise in the phone)
     */ 

    highpass(f_in, f_out, n, 0.99451509); /* e^(-2.2/400hz) */
    memcpy(f_in,f_out,n*2);
    lowpass(f_in, f_out, n, 0.99924166); /* e^(-2.2/2900hz) */

    /* Downsample audio to 8000Hz */
    downsample(f_in, f_dsp, n, wavfile->header.sample_rate, 8000);

    for(i=0;i<len;++i)
        output[i] = linear2ulaw(f_dsp[i]);

fail:
    free(buf);
    free(piece);
    free(f_in);
    free(f_out);
    free(f_dsp);
    return i;
}


/*
** This routine converts from linear to ulaw.
**
** Craig Reese: IDA/Supercomputing Research Center
** Joe Campbell: Department of Defense
** 29 September 1989
**
** References:
** 1) CCITT Recommendation G.711  (very difficult to follow)
** 2) "A New Digital Technique for Implementation of Any
**     Continuous PCM Companding Law," Villeret, Michel,
**     et al. 1973 IEEE Int. Conf. on Communications, Vol 1,
**     1973, pg. 11.12-11.17
** 3) MIL-STD-188-113,"Interoperability and Performance Standards
**     for Analog-to_Digital Conversion Techniques,"
**     17 February 1987
**
** Input: Signed 16 bit linear sample
** Output: 8 bit ulaw sample
*/


#define BIAS 0x84   /* define the add-in bias for 16 bit samples */
#define CLIP 32635

unsigned char linear2ulaw(int16_t sample){
  static int exp_lut[256] = {0,0,1,1,2,2,2,2,3,3,3,3,3,3,3,3,
                             4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,
                             5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,
                             5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,
                             6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,
                             6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,
                             6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,
                             6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,
                             7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,
                             7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,
                             7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,
                             7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,
                             7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,
                             7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,
                             7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,
                             7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7};
  int sign, exponent, mantissa;
  unsigned char ulawbyte;

  /* Get the sample into sign-magnitude. */
  sign = (sample >> 8) & 0x80;		/* set aside the sign */
  if(sign != 0) sample = -sample;		/* get magnitude */
  if(sample > CLIP) sample = CLIP;		/* clip the magnitude */

  /* Convert from 16 bit linear to ulaw. */
  sample = sample + BIAS;
  exponent = exp_lut[( sample >> 7 ) & 0xFF];
  mantissa = (sample >> (exponent + 3)) & 0x0F;
  ulawbyte = ~(sign | (exponent << 4) | mantissa);

  return(ulawbyte);
}

