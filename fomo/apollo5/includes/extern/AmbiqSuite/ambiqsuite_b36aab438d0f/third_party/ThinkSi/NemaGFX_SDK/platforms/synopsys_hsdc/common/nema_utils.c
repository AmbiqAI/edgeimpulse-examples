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

#include "nema_sys_defs.h"
#include "board.h"

#include "nema_utils.h"
// #ifndef PRINT_I
// #define PRINT_I DBG_print
// #endif

#ifndef TIMEOUT_S
#define TIMEOUT_S 3.0f
#endif

#define TIMER_TICKS_PER_SECOND 20.f

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
    //Open file
    DBG_print("Opening file not supported (%s)\r\n", filename);
    nema_buffer_t bo = {0};

    return bo;
}

/**
* Timer Handler (for measurements)
*/
void nema_timer0_callback(void)
{
    return;
}

float nema_get_time(void)
{
    static int timer_init = 1;

    if (timer_init != 0) {
        timer_init = 0;

        Board_Timer_Config_t timer_config;
        /**
        * Init timer0 for 1 second
        */
        timer_config.timer_number  = 0;
        timer_config.timer_tick    = board_getCoreFreq()/TIMER_TICKS_PER_SECOND; //After x clk's a callback
        timer_config.timer_callback= nema_timer0_callback;                            //callback function
        board_timerStart(&timer_config);
        return 0.f;
    }

    uint32_t ticks = board_timerGet(0);
    float sec = (float)ticks/TIMER_TICKS_PER_SECOND;

    return sec;
}

void nema_calculate_fps(void)
{
    static int   frame      = 0;

    ++frame;

    if ( frame%100 == 0 ) {
        static float start_time = 0.f;
        static float stop_time  = 0.f;

        stop_time = nema_get_time();
        float fps = 100.f/(stop_time-start_time);
        int fps_i = (int) fps;
        float fps_diff = (fps-(float)fps_i)*1000.f;
        int fps_f = fps_diff;
        DBG_print("fps: %d.%d\n", fps_i, fps_f);
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
