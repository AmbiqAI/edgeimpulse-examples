//*****************************************************************************
//
//! @file am_util_time.c
//!
//! @brief Functions useful for RTC, calendar, time, etc. computations.
//!
//! @addtogroup time Time - RTC Time Computations
//! @ingroup utils
//! @{
//
//*****************************************************************************

//*****************************************************************************
//
// ${copyright}
//
// This is part of revision ${version} of the AmbiqSuite Development Package.
//
//*****************************************************************************
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

//*****************************************************************************
//
// Macro definitions
//
//*****************************************************************************
#define MAX_BLOCK_NUMBER   (1024UL)

#define SIMPLE_MALLOC_ASSERT(expr)                  \
    do                                              \
    {                                               \
        if ( !(expr) )                              \
        {                                           \
            while(1);                               \
        }                                           \
    } while(0)

//*****************************************************************************
//
// Global Variables
//
//*****************************************************************************
typedef struct
{
    uint32_t total_block;
    uint32_t block_size;
    uint32_t start_address;
    uint32_t end_address;
    uint32_t empty_block;
    uint32_t used_block;
    uint16_t map[MAX_BLOCK_NUMBER];
}simple_malloc_t;

static simple_malloc_t simple_malloc_core =
{
    .total_block = 0,
    .block_size = 0,
    .start_address = 0,
    .empty_block = 0,
    .used_block = 0,
};

//*****************************************************************************
//
//! @brief Stub of a function to show our naming convention
//!
//! The parameter list was left out completely here, since there are no
//! parameters.
//!
//! @return None.
//
//*****************************************************************************
void ssram_heap_init(uint32_t start_addr, uint32_t heap_size_in_byte,
                     uint32_t block_size_in_byte)
{
    if ( start_addr == 0 )
    {
        return;
    }

    if ( heap_size_in_byte == 0 )
    {
        return;
    }

    if ( block_size_in_byte == 0 )
    {
        block_size_in_byte = 4 * 1024;
    }

    uint32_t block_number = heap_size_in_byte / block_size_in_byte;

    if ( block_number > MAX_BLOCK_NUMBER )
    {
        return;
    }

    simple_malloc_core.total_block = block_number;
    simple_malloc_core.block_size = block_size_in_byte;
    simple_malloc_core.start_address = start_addr;
    simple_malloc_core.end_address = start_addr + block_number * block_size_in_byte;
    simple_malloc_core.empty_block = simple_malloc_core.total_block;
    simple_malloc_core.used_block = 0;

    memset(simple_malloc_core.map, 0xff, MAX_BLOCK_NUMBER*sizeof(simple_malloc_core.map[0]));
}

//*****************************************************************************
//
//! @brief Stub of a function to show our naming convention
//!
//! The parameter list was left out completely here, since there are no
//! parameters.
//!
//! @return None.
//
//*****************************************************************************
void* ssram_heap_malloc(uint32_t size_in_byte)
{
    if ( simple_malloc_core.block_size == 0 || simple_malloc_core.empty_block == 0 )
    {
        return NULL;
    }

    uint32_t needed_block = size_in_byte / simple_malloc_core.block_size + ((size_in_byte % simple_malloc_core.block_size) ? 1 : 0);

    if ( needed_block > simple_malloc_core.empty_block )
    {
        return NULL;
    }

    uint32_t continue_empty_block_cnt = 0;
    uint32_t continue_empty_block_start = MAX_BLOCK_NUMBER;
    uint32_t current_block = 0;

    while(current_block < simple_malloc_core.total_block)
    {
        if ( simple_malloc_core.map[current_block] == 0xffff )
        {
            continue_empty_block_cnt ++;
            if ( continue_empty_block_start == MAX_BLOCK_NUMBER )
            {
                continue_empty_block_start = current_block;
            }

            if ( continue_empty_block_cnt == needed_block )
            {
                break;
            }
        }
        else
        {
            continue_empty_block_cnt = 0;
            continue_empty_block_start = MAX_BLOCK_NUMBER;
        }

        current_block ++;
    }

    if ( current_block == simple_malloc_core.total_block )
    {
        return NULL;
    }
    else
    {
        uint32_t continue_empty_block_end = continue_empty_block_start + continue_empty_block_cnt;
        for ( uint32_t i = continue_empty_block_start; i < continue_empty_block_end; i++ )
        {
            simple_malloc_core.map[i] = continue_empty_block_start;
        }

        simple_malloc_core.empty_block -= continue_empty_block_cnt;
        simple_malloc_core.used_block += continue_empty_block_cnt;

        return (void*)(simple_malloc_core.start_address + continue_empty_block_start * simple_malloc_core.block_size);
    }
}

//*****************************************************************************
//
//! @brief Stub of a function to show our naming convention
//!
//! The parameter list was left out completely here, since there are no
//! parameters.
//!
//! @return None.
//
//*****************************************************************************
void ssram_heap_free(void* ptr)
{
    if ( ((uint32_t)ptr < simple_malloc_core.start_address) || ((uint32_t)ptr >= simple_malloc_core.end_address ) )
    {
        return;
    }

    uint32_t address = (uint32_t)ptr - simple_malloc_core.start_address;

    if ( address % simple_malloc_core.block_size )
    {
        SIMPLE_MALLOC_ASSERT(false);
        return;
    }

    uint32_t start_block = address / simple_malloc_core.block_size;

    SIMPLE_MALLOC_ASSERT(simple_malloc_core.map[start_block] == start_block);

    for ( uint32_t i = start_block; i < simple_malloc_core.total_block; i++ )
    {
        if ( simple_malloc_core.map[i] == start_block )
        {
            simple_malloc_core.empty_block ++;
            simple_malloc_core.used_block --;
            simple_malloc_core.map[i] = 0xffff;
        }
        else
        {
            break;
        }
    }
}

void * ssram_heap_realloc(void * ptr, size_t size)
{
    uint32_t address = (uint32_t)ptr - simple_malloc_core.start_address;

    if ( address % simple_malloc_core.block_size )
    {
        SIMPLE_MALLOC_ASSERT(false);
        return NULL;
    }

    uint32_t start_block = address / simple_malloc_core.block_size;

    SIMPLE_MALLOC_ASSERT(simple_malloc_core.map[start_block] == start_block);

    uint32_t current_size = 0;

    for ( uint32_t i = start_block; i < simple_malloc_core.total_block; i++ )
    {
        if ( simple_malloc_core.map[i] == start_block )
        {
            current_size += simple_malloc_core.block_size;
        }
        else
        {
            break;
        }
    }

    if ( current_size >= size )
    {
        return ptr;
    }
    else
    {
        ssram_heap_free(ptr);
        return ssram_heap_malloc(size);
    }
}
