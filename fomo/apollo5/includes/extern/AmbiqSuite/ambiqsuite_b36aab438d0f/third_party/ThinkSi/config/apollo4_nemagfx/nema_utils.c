// -----------------------------------------------------------------------------
// Copyright (c) 2019 Think Silicon S.A.
// Think Silicon S.A. Confidential Proprietary
// -----------------------------------------------------------------------------
//     All Rights reserved - Unpublished -rights reserved under
//         the Copyright laws of the European Union
//
//  This file includes the Confidential information of Think Silicon S.A.
//  The receiver of this Confidential Information shall not disclose
//  it to any third party and shall protect its confidentiality by
//  using the same degree of care, but not less than a reasonable
//  degree of care, as the receiver uses to protect receiver's own
//  Confidential Information. The entire notice must be reproduced on all
//  authorised copies and copies may only be made to the extent permitted
//  by a licensing agreement from Think Silicon S.A..
//
//  The software is provided 'as is', without warranty of any kind, express or
//  implied, including but not limited to the warranties of merchantability,
//  fitness for a particular purpose and noninfringement. In no event shall
//  Think Silicon S.A. be liable for any claim, damages or other liability, whether
//  in an action of contract, tort or otherwise, arising from, out of or in
//  connection with the software or the use or other dealings in the software.
//
//
//                    Think Silicon S.A.
//                    http://www.think-silicon.com
//                    Patras Science Park
//                    Rion Achaias 26504
//                    Greece
// -----------------------------------------------------------------------------

#include <string.h>
#ifndef BAREMETAL
#include "FreeRTOS.h"
#include "task.h"
#endif
#include "nema_utils.h"

#include "am_util_stdio.h"
#include "am_mcu_apollo.h"

//returns time in seconds
float nema_get_time(void)
{
#ifdef BAREMETAL
    uint32_t ui32TimerCount = am_hal_timer_read(0);
    static uint32_t count_last = 0;
    static float last_sec = 0;
    float full_scale_sec = ((float)0xFFFFFFFF/3000000.f);

    if (count_last > ui32TimerCount)
    {
        // overflow
       last_sec += full_scale_sec;
    }
    count_last = ui32TimerCount;
    //am_util_stdio_printf("time %d\n",ui32TimerCount);
    //return ((float)ui32TimerCount/ 750000.f);          // for 12MHz FPGA image
    //return ((float)ui32TimerCount/1500000.f);          // for 24MHz FPGA image
    return last_sec + ((float)ui32TimerCount/3000000.f);          // for 48MHz FPGA image

#else
    float sec;

    TickType_t ticks = xTaskGetTickCount();

    sec = (float)ticks/(float) configTICK_RATE_HZ;

    return sec;
#endif
}

static unsigned long s[] = {123456789, 362436069};
unsigned int nema_rand()
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
  return bo;
}


int nema_save_file(const char *filename, int length, void *buffer)
{
    return 0;
}


// returns wall time in useconds
float nema_get_wall_time(void)
{
    return nema_get_time();
}

void nema_calculate_fps(void)
{
    static int   frame      = 0;

    ++frame;

    if ( frame%100 == 0 ) {
        static float start_time = 0.f;
        static float stop_time  = 0.f;

        stop_time = nema_get_time();
        am_util_stdio_printf("fps: %.02f\n", 100.f/(stop_time-start_time));
        start_time = stop_time;
    }
}

void *nema_memcpy ( void * destination, const void * source, size_t num )
{
#if 1
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

