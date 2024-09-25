//*****************************************************************************
//
//! @file nemagfx_heap_customize.c
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

//*****************************************************************************
//
// A simple demo to customize the graphics heap in nema_hal.c.
// We use tsi_malloc here, but you are free to use any heap management schema,
// just the heap malloc and free APIs are required.
// Note: The heap should be initialized before nema_init();
//
//*****************************************************************************
int graphic_heap_init(void* heap_start, uint32_t heap_size)
{
    return tsi_malloc_init_pool_aligned(0,
                                (void *)heap_start,
                                (uintptr_t)heap_start,
                                heap_size,
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

extern bool graphic_heap_region_need_flush(uint32_t region_start, uint32_t region_size);

void graphic_heap_flush(nema_buffer_t *buffer)
{
    bool need_flush = true;
    need_flush = graphic_heap_region_need_flush(buffer->base_phys, buffer->size);

    if ( need_flush )
    {
        am_hal_cachectrl_range_t Range;
        Range.ui32Size = buffer->size;
        Range.ui32StartAddr = buffer->base_phys;
        am_hal_cachectrl_dcache_clean(&Range);
    }
}
