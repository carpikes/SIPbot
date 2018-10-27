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
 * @file test_filter.c
 * @brief Bandwidth filters test file
 */

#include "../src/common.h"
#include "../src/filter.h"

#include <pulse/error.h>
#include <pulse/simple.h>

#define BB 32000

int main(int argc, char *argv[])
{
    FILE *fp;
    int16_t buf[BB], out[(BB * 8000 / 44100)];
    int error;
    static const pa_sample_spec ss = {
        .format = PA_SAMPLE_S16LE, .rate = 8000, .channels = 1};

    pa_simple *s = NULL;
    if (!(s = pa_simple_new(NULL, argv[0], PA_STREAM_PLAYBACK, NULL, "playback",
                            &ss, NULL, NULL, &error)))
    {
        fprintf(stderr, __FILE__ ": pa_simple_new() failed: %s\n",
                pa_strerror(error));
        goto finish;
    }

    fp = fopen("out.wav", "rb");
    if (!fp)
        exit(-1);

    fread(buf, 1, 46, fp);
    fread(buf, 2, BB, fp);
    while (!feof(fp))
    {
        downsample(buf, out, BB, 44100, 8000);
        if (pa_simple_write(s, out, (size_t)(BB * 8000 / 44100) * 2, &error) <
            0)
        {
            fprintf(stderr, __FILE__ ": pa_simple_write() failed: %s\n",
                    pa_strerror(error));
            goto finish;
        }

        fread(buf, 2, BB, fp);
    }

    if (pa_simple_drain(s, &error) < 0)
    {
        fprintf(stderr, __FILE__ ": pa_simple_drain() failed: %s\n",
                pa_strerror(error));
        goto finish;
    }

finish:
    if (s)
        pa_simple_free(s);
    return 0;
}
