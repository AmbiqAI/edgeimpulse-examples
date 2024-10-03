/* TSI 2023.xmo */
/*******************************************************************************
 * Copyright (c) 2023 Think Silicon Single Member PC
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this header file and/or associated documentation files to use, copy,
 * modify, merge, publish, distribute, sublicense, and/or sell copies of the
 * Materials, and to permit persons to whom the Materials are furnished to do
 * so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Materials.
 *
 * MODIFICATIONS TO THIS FILE MAY MEAN IT NO LONGER ACCURATELY REFLECTS
 * NEMAGFX API. THE UNMODIFIED, NORMATIVE VERSIONS OF THINK-SILICON NEMAGFX
 * SPECIFICATIONS AND HEADER INFORMATION ARE LOCATED AT:
 *   https://think-silicon.com/products/software/nemagfx-api
 *
 *  The software is provided 'as is', without warranty of any kind, express or
 *  implied, including but not limited to the warranties of merchantability,
 *  fitness for a particular purpose and noninfringement. In no event shall
 *  Think Silicon Single Member PC be liable for any claim, damages or other
 *  liability, whether in an action of contract, tort or otherwise, arising
 *  from, out of or in connection with the software or the use or other dealings
 *  in the software.
 ******************************************************************************/

#include <ctype.h>
//#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
//#include <time.h>
#include <unistd.h>
#include "nema_utils.h"
#include "assist.h"

#ifndef TIMEOUT_US
#define TIMEOUT_US 500000.0f
#endif

// return time in seconds
float nema_get_time(void)
{
    float time = (float) get_assist_time();
    return time;
}

static unsigned long s[] = {123456789, 362436069};
unsigned int nema_rand(void)
{
    uint64_t x = s[0];
    uint64_t const y = s[1];
    s[0] = y;
    x ^= x << 23; // a
    s[1] = x ^ y ^ (x >> 17) ^ (y >> 26); // b, c

    return s[1] + y;
}

nema_buffer_t nema_load_file(const char *filename, int length, void *buffer)
{
    nema_buffer_t bo = {0};

    if ( buffer == NULL ) {
#ifdef NEMA_MEM_POOL_ASSETS
        bo = nema_buffer_create_pool(NEMA_MEM_POOL_ASSETS, length);
#else
        bo = nema_buffer_create(length);
#endif
        buffer = nema_buffer_map(&bo);
    }
    if (buffer != NULL) {
        fast_data_load(filename, length, (int)(char *)(buffer), 0);
    } else {
        aprint("Cannot allocate buffer\n");
    }

    return bo;
}

// returns clock time in useconds
float nema_get_clk_time(void)
{
    //TODO: Currently not supported
    aprint("nema_clk_time not supported\n");
    return 0;
}

void nema_calculate_fps(void)
{
    static int   frame      = 0;
    ++frame;

    if ( frame%100 == 0 ) {
        static float start_time = 0.f;
        static float stop_time  = 0.f;

        stop_time = nema_get_time();
        aprint("fps calculated:");
        float t = 100.f/(stop_time-start_time);
        nprint((int)t);
        start_time = stop_time;
    }
}


void *nema_memcpy ( void * destination, const void * source, size_t num )
{
#if 0
    return memcpy(destination, source, num);
#else
    uint32_t *dst32 = (uint32_t *)destination;
    const uint32_t *src32 = (const uint32_t *)source;
    size_t num32 = num&(~0x3U);

    size_t i = 0;

    //first copy 4bytes at a time
    for (; i < num32; i+=4U) {
        *dst32 = *src32;
        ++src32;
        ++dst32;
    }

    if (num32 != num) {
        //then finish off with the remains, if num is not multiple of 4
        uint8_t *dst8 = (uint8_t *)dst32;
        const uint8_t *src8 = (const uint8_t *)src32;

        for (; i < num; ++i) {
            *dst8 = *src8;
            ++src8;
            ++dst8;
        }
    }

    return destination;
#endif
}

int nema_save_file(const char* filename, int length, void * buffer)
{
    if ( !buffer ) {
        tsi_print("Cannot save - NULL buffer pointer\n");
        return -1;
    }
    if ( length <= 0 ) {
        tsi_print("Cannot save - invalide buffer length\n");
        return -1;
    }

    write_openfile_const(filename);

    uint8_t* buf = (uint8_t *)buffer;

    // In lattice uint8_t typecast from int
    // brings them in reverse order
    for(int i = 0; i < length; i+=4 )
    {
        write_file( (int)buf[i + 3] );
        write_file( (int)buf[i + 2] );
        write_file( (int)buf[i + 1] );
        write_file( (int)buf[i] );
    }

    write_closefile();
    return 0;
}
