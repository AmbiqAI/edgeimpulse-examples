//*****************************************************************************
//
//! @file am_alloc.h
//!
//! @brief Memory allocation handling .
//!
//*****************************************************************************

//*****************************************************************************
//
// ${copyright}
//
// This is part of revision ${version} of the AmbiqSuite Development Package.
//
//*****************************************************************************

#ifndef AM_ALLOC_H
#define AM_ALLOC_H

#ifdef __cplusplus
extern "C" {
#endif

// Do not include the full FreeRTOS header to avoid compilation dependencies:
#include <stddef.h>
void* pvPortMalloc(size_t size);
void  vPortFree(void*);

static inline void *am_allocate_memory(unsigned int size)
{
	return pvPortMalloc(size);
}

static inline void am_free_memory(void *ptr)
{
    vPortFree(ptr);
}

#ifdef __cplusplus
}
#endif

#endif /* AM_ALLOC_H */
