//*****************************************************************************
//
//! @file nemagfx_heap_customize.h
//!
//! @brief customize the graphics heap used in NemaSDK.
//!
//*****************************************************************************

//*****************************************************************************
//
// Copyright (c) 2024, Ambiq Micro, Inc.
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// 1. Redistributions of source code must retain the above copyright notice,
// this list of conditions and the following disclaimer.
//
// 2. Redistributions in binary form must reproduce the above copyright
// notice, this list of conditions and the following disclaimer in the
// documentation and/or other materials provided with the distribution.
//
// 3. Neither the name of the copyright holder nor the names of its
// contributors may be used to endorse or promote products derived from this
// software without specific prior written permission.
//
// Third party software included in this distribution is subject to the
// additional license terms as defined in the /docs/licenses directory.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
// ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
// LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
// CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
// SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
// INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
// CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
// ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
// POSSIBILITY OF SUCH DAMAGE.
//
// This is part of revision release_a5b_sdk2-748191cd0 of the AmbiqSuite Development Package.
//
//*****************************************************************************

//*****************************************************************************
//
// Global includes for this project.
//
//*****************************************************************************
#include "am_mcu_apollo.h"
#include "nema_hal.h"
#include "tsi_malloc.h"
#include "nemagfx_buffer_customize.h"
#include "nema_regs.h"

//*****************************************************************************
//
// A simple demo to customize the graphics heap and put it to non-cacheable region.
// We use tsi_malloc here, but you are free to use any heap management schema.
// Note: The heap should be initialized before nema_init().
//
//*****************************************************************************


#ifdef NEMA_USE_CUSTOM_MALLOC

//*****************************************************************************
//
// Macro define
//
//*****************************************************************************
#ifndef NON_CACHEABLE_CL_RB_HEAP
    //
    // Make the Command list and ring buffer heap non-cacheable to workaround 
    // an issue of Apollo5a which may result in SVG format image rendering abnormal.
    //
    #ifdef AM_PART_APOLLO5A
        #define NON_CACHEABLE_CL_RB_HEAP 1
    #else
        #define NON_CACHEABLE_CL_RB_HEAP 0
    #endif
#endif

//*****************************************************************************
//
// Customize graphics heap.
//
//*****************************************************************************

#define GRAPHIC_HEAP_SIZE 0x200000UL

AM_SHARED_RW uint8_t graphic_heap_ssram[GRAPHIC_HEAP_SIZE];

//*****************************************************************************
//
// Graphics heap flush callback.
//
//*****************************************************************************
graphic_heap_flush_cb_t g_graphic_heap_flush_cb = NULL;

//*****************************************************************************
//
// Setup MPU regionsm.
//
//*****************************************************************************

#if NON_CACHEABLE_CL_RB_HEAP==1

#define MPU_REGION_NUMBER 6
#define MPU_ATTRIBUTE_ENTRY 0

/*
 * Set the graphics heap to noncacheable,
 * the GPU command list buffers and global ring buffer will be allocated from this heap.
 */
void set_graphic_heap_noncacheable(void)
{

    am_hal_mpu_region_config_t sMPUCfg = 
    { 
        .ui32RegionNumber = MPU_REGION_NUMBER,
        .ui32BaseAddress = 0x20080000U,
        .eShareable = NON_SHARE,
        .eAccessPermission = RW_NONPRIV,
        .bExecuteNever = true,
        .ui32LimitAddress = 0,
        .ui32AttrIndex = MPU_ATTRIBUTE_ENTRY,
        .bEnable = true,
    };

    //MPU non-cacheable attribute setting.
    //OuterAttr and InnerAttr is set to 0b0100 to follow the requirement in ArmV8-m Architecture Reference Manual
    am_hal_mpu_attr_t sMPUAttr = 
    {   
        .ui8AttrIndex = MPU_ATTRIBUTE_ENTRY,
        .bNormalMem = true,
        .sOuterAttr = {
                        .bNonTransient = false, 
                        .bWriteBack = true, 
                        .bReadAllocate = false, 
                        .bWriteAllocate = false
                      },
        .sInnerAttr = {
                        .bNonTransient = false, 
                        .bWriteBack = true, 
                        .bReadAllocate = false, 
                        .bWriteAllocate = false
                      },
        .eDeviceAttr = 0,
    };

    //
    // Set up the attributes.
    //
    am_hal_mpu_attr_configure(&sMPUAttr, 1);
    //
    // Clear the MPU regions.
    //
    am_hal_mpu_region_clear();

    //
    // Set up the regions.
    //
    sMPUCfg.ui32BaseAddress = (uintptr_t)graphic_heap_ssram;
    sMPUCfg.ui32LimitAddress = (uintptr_t)graphic_heap_ssram + GRAPHIC_HEAP_SIZE - 1;
    am_hal_mpu_region_configure(&sMPUCfg, 1);

    //
    // Invalidate and clear DCACHE, this is required by CM55 TRF. 
    //
    am_hal_cachectrl_dcache_invalidate(NULL, true);

    //
    // MPU enable
    //
    am_hal_mpu_enable(true, true);
}

#endif

void graphic_heap_set_flush_cb(graphic_heap_flush_cb_t flush_cb)
{
    g_graphic_heap_flush_cb = flush_cb;
}

int graphic_heap_init(void)
{

    //
    // Set the memory region used for graphics buffer to non-cacheable.
    //
#if (NON_CACHEABLE_CL_RB_HEAP==1)
    set_graphic_heap_noncacheable();
#endif 

    return tsi_malloc_init_pool_aligned(0, 
                                (void *)graphic_heap_ssram, 
                                (uintptr_t)graphic_heap_ssram, 
                                GRAPHIC_HEAP_SIZE, 
                                1, 
                                32);   
}

void* graphic_heap_malloc(uint32_t pool, uint32_t size)
{
    return tsi_malloc(size);
}

void graphic_heap_free(void *ptr)
{
    tsi_free(ptr);
}  

void graphic_heap_flush(nema_buffer_t *buffer)
{
    bool need_flush = true;
    uintptr_t region_start = buffer->base_phys;

#if NON_CACHEABLE_CL_RB_HEAP==1
    if((region_start >= (uint32_t)graphic_heap_ssram) && 
       (region_start < (uint32_t)graphic_heap_ssram + GRAPHIC_HEAP_SIZE))
    {
        need_flush = false;
    }
    else
#endif
    {
        need_flush = true;
    }

    if(need_flush)
    {
        am_hal_cachectrl_range_t Range;
        Range.ui32Size = buffer->size;
        Range.ui32StartAddr = buffer->base_phys;
        am_hal_cachectrl_dcache_clean(&Range);
    }

    if(g_graphic_heap_flush_cb)
    {
        g_graphic_heap_flush_cb(buffer);
    }
}

#endif
