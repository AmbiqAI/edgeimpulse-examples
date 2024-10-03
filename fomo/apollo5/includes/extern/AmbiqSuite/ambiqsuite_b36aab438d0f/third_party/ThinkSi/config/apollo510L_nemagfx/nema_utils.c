/*******************************************************************************
 * Copyright (c) 2022 Think Silicon S.A.
 *
   Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this header file and/or associated documentation files to use, copy,
 * modify, merge, publish, distribute, sublicense, and/or sell copies of the
 * Materials, and to permit persons to whom the Materials are furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Materials.
 *
 * MODIFICATIONS TO THIS FILE MAY MEAN IT NO LONGER ACCURATELY REFLECTS
 * NEMAGFX API. THE UNMODIFIED, NORMATIVE VERSIONS OF THINK-SILICON NEMAGFX
 * SPECIFICATIONS AND HEADER INFORMATION ARE LOCATED AT:
 *   https://think-silicon.com/products/software/nemagfx-api
 *
 *  The software is provided 'as is', without warranty of any kind, express or
 *  implied, including but not limited to the warranties of merchantability,
 *  fitness for a particular purpose and noninfringement. In no event shall
 *  Think Silicon S.A. be liable for any claim, damages or other liability, whether
 *  in an action of contract, tort or otherwise, arising from, out of or in
 *  connection with the software or the use or other dealings in the software.
 ******************************************************************************/

#include <string.h>
#ifndef BAREMETAL
#include "FreeRTOS.h"
#include "task.h"
#endif
#include "nema_utils.h"
#include "nema_blender.h"
#include "nema_cmdlist.h"

#include "am_util_stdio.h"
#include "am_mcu_apollo.h"
#include "nema_regs.h"

#define NEMA_MAX_SINGLE_TRANSFER            0x200

#ifdef APOLLO5_FPGA
#define STIME_FREQUENCY										(APOLLO5_FPGA * 1000000 / 16.0f)
#else
#define STIME_FREQUENCY										(6000000.0f)
#endif

//returns time in seconds
float nema_get_time(void)
{
#ifdef BAREMETAL
    static uint32_t count_last = 0;
    static float last_sec = 0.f;
    float full_scale_sec,timer_offset;
    uint32_t ui32TimerCount;

    //If the count_last and last_sec both equals zero, this is the first time 
    //we enter this function, we will clear and start the STIMER.
    //Note: it is safe to use equal comparaison between last_sec and 0.f at this scenario.
    if((count_last == 0) && (last_sec == 0.f))
    {
        am_hal_stimer_config(AM_HAL_STIMER_CFG_CLEAR);
        am_hal_stimer_config(AM_HAL_STIMER_HFRC_6MHZ| AM_HAL_STIMER_CFG_RUN);
    }

    //
    // use stimer
    //
    ui32TimerCount = am_hal_stimer_counter_get();
    full_scale_sec = (float)0xFFFFFFFF / STIME_FREQUENCY;
    timer_offset = (float)ui32TimerCount / STIME_FREQUENCY;
    if (count_last > ui32TimerCount)
    {
        // overflow
        last_sec += full_scale_sec;
    }
    count_last = ui32TimerCount;
    return last_sec + timer_offset;

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
        am_util_stdio_printf("\nfps: %.02f\n", 100.f/(stop_time-start_time));
        start_time = stop_time;
    }
}

#if 0
//
// Maximum num is 0x7FFF, return NULL if larger than 0x7FFF
//
static
void *nema_gpu_memcpy ( void * destination, const void * source, size_t num )
{
    img_obj_t sFB;
    img_obj_t sObjSourceRGB332;
    nema_cmdlist_t *psCLLast;
    static nema_cmdlist_t sCL;
    static bool bFlag = false;

    if (num > NEMA_MAX_SINGLE_TRANSFER)
    {
        return NULL;
    }

    if (nema_reg_read(NEMA_STATUS) != 0U)
    {
        return NULL;
    }

    psCLLast = nema_cl_get_bound();

    if (bFlag == false)
    {
        bFlag = true;
        sCL = nema_cl_create();
    }
    else
    {
        nema_cl_rewind(&sCL);
    }

    nema_cl_bind(&sCL);

    sFB.bo.base_phys = (uintptr_t)destination;
    sFB.bo.base_virt = (void*)sFB.bo.base_phys;
    sFB.bo.fd = 0;
    sFB.bo.size = num;
    sFB.w = sFB.bo.size;
    sFB.h = 1;
    sFB.stride = sFB.w;
    sFB.color = 0;
    sFB.format = NEMA_RGB332;
    sFB.sampling_mode = 0;
    nema_bind_dst_tex(sFB.bo.base_phys, sFB.w, sFB.h, sFB.format, sFB.stride);

    sObjSourceRGB332.bo.base_virt = (void*)source;
    sObjSourceRGB332.bo.base_phys = (uintptr_t)sObjSourceRGB332.bo.base_virt;
    sObjSourceRGB332.bo.fd = 0;
    sObjSourceRGB332.bo.size = sFB.bo.size;
    sObjSourceRGB332.w = sFB.bo.size;
    sObjSourceRGB332.h = sFB.h;
    sObjSourceRGB332.stride = sFB.stride;
    sObjSourceRGB332.format = sFB.format;
    nema_bind_src_tex(sObjSourceRGB332.bo.base_phys,
                      sObjSourceRGB332.w,
                      sObjSourceRGB332.h,
                      sObjSourceRGB332.format,
                      sObjSourceRGB332.stride,
                      NEMA_FILTER_PS);
    nema_set_clip(0, 0, sFB.bo.size, sFB.h);
    nema_set_blend_blit(NEMA_BL_SRC);
    nema_blit(0, 0);

    nema_cl_submit(&sCL);
    nema_cl_wait(&sCL);

    nema_cl_unbind();

    if (psCLLast != NULL)
    {
        nema_cl_bind(psCLLast);
    }

    return destination;
}
#endif

void *nema_memcpy ( void * destination, const void * source, size_t num )
{
#if 0
    uint32_t ui32MaxLoops;
    uint32_t ui32LastNum;
    uint32_t ui32CurrentLoop;
    uint8_t *pui8CurrentDestination = (uint8_t*)destination;
    uint8_t *pui8CurrentSource = (uint8_t*)source;

    ui32MaxLoops = num / NEMA_MAX_SINGLE_TRANSFER;
    ui32LastNum = num % NEMA_MAX_SINGLE_TRANSFER;

    for (ui32CurrentLoop = 0; ui32CurrentLoop < ui32MaxLoops; ui32CurrentLoop++)
    {
        nema_gpu_memcpy(pui8CurrentDestination, pui8CurrentSource, NEMA_MAX_SINGLE_TRANSFER);
        pui8CurrentDestination += NEMA_MAX_SINGLE_TRANSFER;
        pui8CurrentSource += NEMA_MAX_SINGLE_TRANSFER;
    }

    if (ui32LastNum > 0)
    {
        nema_gpu_memcpy(pui8CurrentDestination, pui8CurrentSource, ui32LastNum);
    }
#else
    am_hal_cachectrl_range_t sCopyRange;

    // Memory copy
    memcpy(destination, source, num);

    // Cache flush
    sCopyRange.ui32StartAddr = (uint32_t)destination;
    sCopyRange.ui32Size = num;
    am_hal_cachectrl_dcache_clean(&sCopyRange);
#endif

    return destination;
}

