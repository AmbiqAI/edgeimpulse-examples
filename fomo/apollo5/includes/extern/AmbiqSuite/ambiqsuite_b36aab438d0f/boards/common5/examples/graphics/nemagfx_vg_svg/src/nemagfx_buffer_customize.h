//*****************************************************************************
//
//! @file nemagfx_heap_customize.h
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

#ifndef NEMAGFX_HEAP_CUSTOMIZE_H
#define NEMAGFX_HEAP_CUSTOMIZE_H

#include "nema_hal.h"

//*****************************************************************************
//
// Macro definitions
//
//*****************************************************************************
#define NEMA_CUSTOM_MALLOC graphic_heap_malloc
#define NEMA_CUSTOM_FREE graphic_heap_free
#define NEMA_CUSTOM_FLUSH graphic_heap_flush

//*****************************************************************************
//
// External function definitions.
//
//*****************************************************************************
extern int graphic_heap_init(void* heap_start, uint32_t heap_size);
extern void* graphic_heap_malloc(uint32_t pool, uint32_t size);
extern void graphic_heap_free(void *ptr);
extern void graphic_heap_flush(nema_buffer_t *buffer);

#endif // NEMAGFX_HEAP_CUSTOMIZE_H
