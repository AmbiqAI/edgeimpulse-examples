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
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
// #include <time.h>
#include <unistd.h>
#include "nema_utils.h"

//Root File System
#include "io_types.h"

//Warning:App must be built with "FS_TYPE=fat_fs", otherwise "include" is invalid
#include "ff.h"

#include "arc_timer.h"
#include "board.h"

#include "nema_utils.h"
// #ifndef PRINT_I
// #define PRINT_I DBG_print
// #endif

#ifndef TIMEOUT_S
#define TIMEOUT_S 3.0f
#endif

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
    FIL file;

    //Open file
    DBG_print("Open file %s\r\n", filename);
    int res = f_open(&file, filename, FA_READ | FA_OPEN_EXISTING);

    nema_buffer_t bo = {0};

    if (res == FR_OK) {
        if (length < 0) {
            length = f_size (&file);
        }

        if ( !buffer ) {
            bo = nema_buffer_create(length);
            buffer = nema_buffer_map(&bo);
        }

        if (buffer != NULL) {
            uint32_t cnt;
            res = f_read (&file, buffer, length, &cnt);
            if(res != 0 || ((int)cnt != length) ) {
                DBG_print("Error Read file %d\r\n", res);
            }
        } else {
            DBG_print("Cannot allocate buffer\n");
        }

        (void)f_close (&file);

        return bo;
    } else {
        DBG_print("Error Open File %d\r\n", res);
    }

    return bo;
}

float nema_get_time(void)
{
    float sec = -1;
    uint64_t timer_tick = 0;

    if ( timer_current(TIMER_RTC, &timer_tick) == 0) {
        sec = (timer_tick>>10)/(CLK_CPU/1024.f);
    }

    return sec;
}

// returns wall time in useconds
float nema_get_wall_time(void)
{
    return nema_get_time()*0.000001;
}

void nema_calculate_fps(void)
{
    static int   frame      = 0;

    ++frame;

    if ( frame%100 == 0 ) {
        static float start_time = 0.f;
        static float stop_time  = 0.f;

        stop_time = nema_get_time();
        DBG_print("fps: %.02f\n", 100.f/(stop_time-start_time));
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
