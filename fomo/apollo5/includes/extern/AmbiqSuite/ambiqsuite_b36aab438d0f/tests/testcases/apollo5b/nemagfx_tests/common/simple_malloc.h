//*****************************************************************************
//
//! @file coding_standard.h
//!
//! @brief Brief description of the header. No need to get fancy here.
//
//*****************************************************************************

//*****************************************************************************
//
// ${copyright}
//
// This is part of revision ${version} of the AmbiqSuite Development Package.
//
//*****************************************************************************
#ifndef LV_SIMPLE_MALLOC_H
#define LV_SIMPLE_MALLOC_H

//
// Put additional includes here if necessary.
//
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C"
{
#endif

//*****************************************************************************
//
// External function definitions.
//
//*****************************************************************************
extern void psram_heap_init(uint32_t start_addr, uint32_t heap_size_in_byte, 
               uint32_t block_size_in_byte);
extern void* psram_heap_malloc(uint32_t size_in_byte);
extern void psram_heap_free(void* ptr);
extern void * psram_heap_realloc(void * ptr, size_t size);

extern void ssram_heap_init(uint32_t start_addr, uint32_t heap_size_in_byte, 
               uint32_t block_size_in_byte);
extern void* ssram_heap_malloc(uint32_t size_in_byte);
extern void ssram_heap_free(void* ptr);
extern void * ssram_heap_realloc(void * ptr, size_t size);

extern void tcm_heap_init(uint32_t start_addr, uint32_t heap_size_in_byte, 
               uint32_t block_size_in_byte);
extern void* tcm_heap_malloc(uint32_t size_in_byte);
extern void tcm_heap_free(void* ptr);
extern void * tcm_heap_realloc(void * ptr, size_t size);

#ifdef __cplusplus
}
#endif

#endif // CODING_STANDARD_H
