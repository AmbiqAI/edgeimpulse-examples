//*****************************************************************************
//
//! @file buffer_pool.c
//!
//! @brief Buffer Pool Utility for cross thread communication.
//!
//! These functions provide a quick allocate and free mechanism for 256 byte
//! buffer pool arrays. These are thread safe (use ENTER/EXIT Critical
//! section brackets), but they use a very fast allocate/deallocate method.
//!
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

#include "am_mcu_apollo.h"
#include "am_bsp.h"
#include "am_util.h"

#include "FreeRTOS.h"
#include "task.h"
#include "portmacro.h"
#include "portable.h"
#include "semphr.h"
#include "event_groups.h"
#include "queue.h"

#include "rtos.h"

#include "freertos_sampler.h"

#include "buffer_pool.h"

//*****************************************************************************
// Macros
//*****************************************************************************
// BUFFER_SIZE must be a multiple of 4
#define  BUFFER_SIZE 256
// only 8 strings maximum and we start allocation from the MSB towards LSB
#define  BUFFER_NUMBER 8
#define  BUFFER_MASK 0x00FFFFFF

//*****************************************************************************
// Local variables implementing string storage.
//*****************************************************************************
volatile static uint32_t   buffer_pool_array[(BUFFER_NUMBER * BUFFER_SIZE) / 4];
volatile static uint32_t   buffer_pool_bitmap = BUFFER_MASK; // 2KBytes


//*****************************************************************************
//! @brief Get a 256 byte Buffer Storage Array from the Pool.
//!
//! @return pointer to string storage array or NULL if none are available.
//!
//*****************************************************************************
volatile uint32_t * buffer_pool_get(void)
{
    uint32_t ui32Clz;
    uint32_t ui32Temp;

    //
    // enter critical section bebore RMW.
    //
    taskENABLE_INTERRUPTS();

    ui32Temp = buffer_pool_bitmap | BUFFER_MASK;

    //
    // if no string storage available, then return NULL
    // bit == 0 means it is available.
    // ui32Temp == 0xFFFFFFFF means no buffers available.
    if ( ! (~ui32Temp) )
    {
        return NULL;
    }

    //
    // pick any available string
    //
#ifdef __IAR_SYSTEMS_ICC__
    ui32Clz = __CLZ(ui32Temp);
#else
    ui32Clz = __builtin_clz(ui32Temp);
#endif
    if ( ui32Clz > 7 )
    {
        ui32Clz = 0; // just use the first one.
    }

    //
    // Now mark this string as used in the bitmap.
    //
    buffer_pool_bitmap |= (1 << (31-ui32Clz));

    //
    // Set loose the hounds ..
    //
    taskDISABLE_INTERRUPTS();

    //
    // Now find the address of the string and return it.
    //
    return  &buffer_pool_array[BUFFER_SIZE * ui32Clz];
}


//*****************************************************************************
//! @brief Free a 256 byte Buffer Storage Array from the Pool.
//!
//! @param Buffer pointer to string storage array obtained from
//! buffer_pool_get().
//!
//*****************************************************************************
void buffer_pool_free(uint32_t * Buffer)
{
    uint32_t ui32Index;

    //
    // Compute the Index of this string
    //
    ui32Index = ((uint32_t)Buffer - (uint32_t)buffer_pool_array) / BUFFER_SIZE;
    if ( ui32Index  > BUFFER_NUMBER )
    {
        while (1);
    }

    //
    // Make it thread safe.
    //
    taskENABLE_INTERRUPTS();

    //
    // Now mark this string as free in the bitmap.
    //
    buffer_pool_bitmap &= ~(1 << (31-ui32Index));

    //
    // Set loose the hounds ..
    //
    taskDISABLE_INTERRUPTS();

}

