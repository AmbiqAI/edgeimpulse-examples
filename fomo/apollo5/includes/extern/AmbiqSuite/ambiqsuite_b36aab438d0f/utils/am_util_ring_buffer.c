//*****************************************************************************
//
//! @file am_util_ring_buffer.c
//!
//! @brief Some helper functions for implementing and managing a ring buffer.
//!
//! @addtogroup ring_buffer Ring Buffer
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
#include "am_util_ring_buffer.h"

//*****************************************************************************
//
// Initializes a ring buffer structure.
//
//*****************************************************************************
void
am_util_ring_buffer_init(am_util_ring_buffer_t *psBuffer, void *pvArray,
                         uint32_t ui32Bytes)
{
    psBuffer->ui32WriteIndex = 0;
    psBuffer->ui32ReadIndex = 0;
    psBuffer->ui32Length = 0;
    psBuffer->ui32Capacity = ui32Bytes;
    psBuffer->pui8Data = (uint8_t *)pvArray;
}

//*****************************************************************************
//
// Write a single byte to the ring buffer.
//
//*****************************************************************************
bool
am_util_ring_buffer_write(am_util_ring_buffer_t *psBuffer, void *pvSource,
                          uint32_t ui32Bytes)
{
    uint32_t i;
    uint8_t *pui8Source;

    pui8Source = (uint8_t *) pvSource;

    //
    // Check to make sure that the buffer isn't already full
    //
    if ( am_util_ring_buffer_space_left(psBuffer) >= ui32Bytes )
    {
        //
        // Loop over the bytes in the source array.
        //
        for ( i = 0; i < ui32Bytes; i++ )
        {
            //
            // Write the value to the buffer.
            //
            psBuffer->pui8Data[psBuffer->ui32WriteIndex] = pui8Source[i];

            //
            // Advance the write index, making sure to wrap if necessary.
            //
            psBuffer->ui32WriteIndex = ((psBuffer->ui32WriteIndex + 1) %
                                        psBuffer->ui32Capacity);
        }

        //
        // Update the length value appropriately.
        //
        psBuffer->ui32Length += ui32Bytes;

        //
        // Report a success.
        //
        return true;
    }
    else
    {
        //
        // The ring buffer can't fit the amount of data requested. Return a
        // failure.
        //
        return false;
    }
}

//*****************************************************************************
//
// Read a single byte from the ring buffer.
//
//*****************************************************************************
bool
am_util_ring_buffer_read(am_util_ring_buffer_t *psBuffer, void *pvDest,
                         uint32_t ui32Bytes)
{
    uint32_t i;
    uint8_t *pui8Dest;

    pui8Dest = (uint8_t *) pvDest;

    //
    // Check to make sure that the buffer isn't empty
    //
    if ( am_util_ring_buffer_data_left(psBuffer) >= ui32Bytes )
    {
        //
        // Loop over the bytes in the destination array.
        //
        for ( i = 0; i < ui32Bytes; i++ )
        {
            //
            // Grab the next value from the buffer.
            //
            pui8Dest[i] = psBuffer->pui8Data[psBuffer->ui32ReadIndex];

            //
            // Advance the read index, wrapping if needed.
            //
            psBuffer->ui32ReadIndex = ((psBuffer->ui32ReadIndex + 1) %
                                       psBuffer->ui32Capacity);
        }

        //
        // Adjust the length value to reflect the change.
        //
        psBuffer->ui32Length -= ui32Bytes;

        //
        // Report a success.
        //
        return true;
    }
    else
    {
        //
        // If the buffer didn't have enough data, just return a zero.
        //
        return false;
    }
}

//*****************************************************************************
//
// End Doxygen group.
//! @}
//
//*****************************************************************************

