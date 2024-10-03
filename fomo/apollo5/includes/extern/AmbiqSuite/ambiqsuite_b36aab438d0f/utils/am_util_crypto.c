//*****************************************************************************
//
//! @file am_util_crypto.c
//!
//! @brief Utility functions for the crypto module.
//!
//! A few utils to help with crypto functionality
//!
//! @addtogroup crypto Crypto
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

#include "am_mcu_apollo.h"
#include "am_util.h"
#include "am_util_crypto.h"

//*****************************************************************************
//
// Memory management.
//
//*****************************************************************************

//
// Simple buffer for variable-size calls to malloc. Memory allocated from this
// buffer cannot be freed.
//
uint8_t g_pui8CryptoMemPool[AM_UTIL_CRYPTO_VARIABLE_POOL_SIZE];
volatile uint32_t g_ui32CryptoMEMPoolIdx = 0;

//
//! A pool of fixed-size buffers. These buffers can be freed, and are used for
//! queue entries within the LibCrypto functions.
//
typedef struct
{
    uint8_t buffer[AM_UTIL_CRYPTO_FIXED_BUFFER_SIZE];
    bool    bUsed;
}
am_buffer_pool_t;

am_buffer_pool_t am_util_crypto_fixed_pool[AM_UTIL_CRYPTO_FIXED_BUFFER_COUNT];

//
//! @name Memory stats. Useful for debugging memory issues.
//! @{
//
uint32_t g_ui32CryptoMallocCount = 0;
uint32_t g_ui32CryptoFreeCount = 0;
uint32_t g_ui32CryptoMallocMax = 0;
uint32_t g_ui32BuffersUsed = 0;
uint32_t g_ui32MaxBuffersUsed = 0;
uint32_t g_ui32CryptoMallocStats[AM_UTIL_CRYPTO_VARIABLE_POOL_SIZE];
//! @}

//*****************************************************************************
//
// Malloc implemtation tailored to LibCrypto's needs.
//
//*****************************************************************************
void *
am_util_crypto_malloc(size_t size)
{
    void *retval = NULL;
    uint32_t i;

    AM_CRITICAL_BEGIN;

    //
    // We have two memory pools to choose from. If this buffer fits in one of
    // the fixed buffers, we should prefer using that.
    //
    if (size == AM_UTIL_CRYPTO_FIXED_BUFFER_SIZE)
    {
        //
        // Search for a free buffer in our fixed buffer pool.
        //
        for (i = 0; i < AM_UTIL_CRYPTO_FIXED_BUFFER_COUNT; i++)
        {
            //
            // If we can find one, mark it used, and return a pointer to it.
            //
            if (am_util_crypto_fixed_pool[i].bUsed == false)
            {
                am_util_crypto_fixed_pool[i].bUsed = true;
                retval = &am_util_crypto_fixed_pool[i].buffer;

                //
                // Track buffer usage stats.
                //
                g_ui32BuffersUsed++;

                if (g_ui32BuffersUsed > g_ui32MaxBuffersUsed)
                {
                    g_ui32MaxBuffersUsed = g_ui32BuffersUsed;
                }

                break;
            }
        }
    }
    else if (g_ui32CryptoMEMPoolIdx + size < AM_UTIL_CRYPTO_VARIABLE_POOL_SIZE)
    {
        //
        // For other memory allocation sizes, we'll use the other pool. This
        // pool implementation is very simple, and doesn't support free().
        //
        retval = &g_pui8CryptoMemPool[g_ui32CryptoMEMPoolIdx];
        g_ui32CryptoMEMPoolIdx += size;
    }

    //
    // For diagnostic purposes only...
    //
    if (size > g_ui32CryptoMallocMax)
    {
        g_ui32CryptoMallocMax = size;
    }

    AM_CRITICAL_END;

    //
    // Error reporting routine.
    //
    if (retval == NULL)
    {
        //
        // Malloc failed.
        //
        am_util_stdio_printf("Crypto Util: Out of memory.\n");
        am_util_crypto_print_malloc_stats();

        while (1);
    }
    else
    {
        g_ui32CryptoMallocStats[size]++;
        g_ui32CryptoMallocCount++;
    }

    return retval;
}

//*****************************************************************************
//
// Calloc implementation tailored to LibCrypto's needs.
//
//*****************************************************************************
void *
am_util_crypto_calloc(size_t num, size_t size)
{
    uint32_t i;
    size_t ui32ActualSize = num * size;
    void *ptr = am_util_crypto_malloc(ui32ActualSize);

    if (ptr != NULL)
    {
        for (i = 0; i < size; i++)
        {
            ((uint8_t *)ptr)[i] = 0;
        }
    }

    return ptr;
}

//*****************************************************************************
//
// Minimal implementation of Crypto Free.
//
// Note: This implementation of free can only free memory from the fixed-size
// buffer pool. It ignores all other requests.
//
//*****************************************************************************
void
am_util_crypto_free(void *ptr)
{
    am_buffer_pool_t *pBuffer;

    //
    // Check to see if this free operation is inside of our fixed pool.
    //
    if ((uint32_t) ptr >= (uint32_t) am_util_crypto_fixed_pool &&
        (uint32_t) ptr < (uint32_t) am_util_crypto_fixed_pool +
        sizeof(am_util_crypto_fixed_pool))
    {
        //
        // Free the fixed-size buffer.
        //
        pBuffer = ptr;
        pBuffer->bUsed = false;

        //
        // Track buffer usage.
        //
        g_ui32BuffersUsed--;
    }

    //
    // Track all calls to free.
    //
    g_ui32CryptoFreeCount++;

    return;
}

//*****************************************************************************
//
// Statistics function for malloc
//
//*****************************************************************************
void
am_util_crypto_print_malloc_stats(void)
{
    uint32_t i;

    am_util_stdio_printf("Malloc ptr:   %d\n", g_ui32CryptoMEMPoolIdx);
    am_util_stdio_printf("Malloc count: %d\n", g_ui32CryptoMallocCount);
    am_util_stdio_printf("Free count:   %d\n", g_ui32CryptoFreeCount);
    am_util_stdio_printf("Malloc max:   %d\n", g_ui32CryptoMallocMax);

    am_util_stdio_printf("Malloc stats:\n");

    for (i = 0; i < AM_UTIL_CRYPTO_VARIABLE_POOL_SIZE; i++)
    {
        if (g_ui32CryptoMallocStats[i])
        {
            am_util_stdio_printf("[%3d] - %4d\n", i, g_ui32CryptoMallocStats[i]);
        }
    }

    uint32_t total_used_buffers = 0;
    // am_util_stdio_printf("Used buffers:\n");

    for (i = 0; i < AM_UTIL_CRYPTO_FIXED_BUFFER_COUNT; i++)
    {
        if (am_util_crypto_fixed_pool[i].bUsed)
        {
            // am_util_stdio_printf("%d (0x%08X) - Used\n", i, &am_util_crypto_fixed_pool[i]);
            total_used_buffers++;
        }
    }

    am_util_stdio_printf("Total fixed buffers used: %d\n", total_used_buffers);
    am_util_stdio_printf("Max buffers used: %d\n", g_ui32MaxBuffersUsed);
}

//*****************************************************************************
//
// Crypto Exit
//
//*****************************************************************************
void
am_util_crypto_exit(uint32_t exitCode)
{
    while (1);
}

//*****************************************************************************
//
// Crypto Abort
//
//*****************************************************************************
void
am_util_crypto_abort(void)
{
    while (1);
}

//*****************************************************************************
//
// End Doxygen group.
//! @}
//
//*****************************************************************************

