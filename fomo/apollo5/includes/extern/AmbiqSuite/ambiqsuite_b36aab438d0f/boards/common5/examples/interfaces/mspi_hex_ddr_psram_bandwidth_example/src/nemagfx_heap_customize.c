//*****************************************************************************
//
//! @file nemagfx_heap_customize.c
//!
//! @brief customize the graphics heap used in NemaSDK.
//!
//*****************************************************************************

//*****************************************************************************
//
// ${copyright}
//
// This is part of revision ${version} of the AmbiqSuite Development Package.
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
