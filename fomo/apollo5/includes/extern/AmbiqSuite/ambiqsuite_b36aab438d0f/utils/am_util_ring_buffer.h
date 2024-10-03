//*****************************************************************************
//
//! @file am_util_ring_buffer.h
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
#ifndef AM_UTIL_RING_BUFFER_H
#define AM_UTIL_RING_BUFFER_H

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C"
{
#endif
//*****************************************************************************
//
// External variable definitions
//
//*****************************************************************************
//
//! Ring Buffer Sturcture
//
typedef struct
{
    volatile uint8_t *pui8Data;
    volatile uint32_t ui32WriteIndex;
    volatile uint32_t ui32ReadIndex;
    volatile uint32_t ui32Length;
    volatile uint32_t ui32Capacity;
}
am_util_ring_buffer_t;

//*****************************************************************************
//
// Function-like macros.
//
//*****************************************************************************
#define am_util_ring_buffer_empty(psBuffer)                                   \
    ((psBuffer)->ui32Length == 0)

#define am_util_ring_buffer_full(psBuffer)                                    \
    ((psBuffer)->ui32Length == (psBuffer)->ui32Capacity)

#define am_util_ring_buffer_space_left(psBuffer)                              \
    ((psBuffer)->ui32Capacity - (psBuffer)->ui32Length)

#define am_util_ring_buffer_data_left(psBuffer)                               \
    ((psBuffer)->ui32Length)

//*****************************************************************************
//
// External function definitions
//
//*****************************************************************************
//*****************************************************************************
//
//! @brief Initializes a ring buffer structure.
//!
//! @param psBuffer is a pointer to the buffer structure to be initialized.
//! @param pvArray is a pointer to the array that the new ringbuffer will use
//! for storage space
//! @param ui32Bytes is the total number of bytes that the ringbuffer will be
//! allowed to use.
//!
//! This function should be called on a ring buffer structure before it is
//! used. If this function is called on a ring buffer that is already being
//! used, it will "erase" the buffer, effectively removing all of the buffer
//! contents from the perspective of the other ring buffer access functions.
//! The data will remain in memory, but it will be overwritten as the buffer is
//! used.
//!
//! @note This operation is not inherently thread-safe, so the caller must make
//! sure that it is appropriately guarded from interrupts and context switches.
//
//*****************************************************************************
void am_util_ring_buffer_init(am_util_ring_buffer_t *psBuffer,
                              void *pvArray, uint32_t ui32Bytes);

//*****************************************************************************
//
//! @brief Write a single byte to the ring buffer.
//!
//! @param psBuffer is the address of the ring buffer structure to be written.
//! @param pvSource is a pointer to the array that the new ringbuffer will use
//! for storage space
//! @param ui32Bytes is the total number of bytes that the ringbuffer will be
//! allowed to use.

//!
//! This function will write a single byte to the given ring buffer. Make sure
//! that the ring buffer is not already full when calling this function. If the
//! ring buffer is already full, this function will fail silently.
//!
//! @note This operation is not inherently thread-safe, so the caller must make
//! sure that it is appropriately guarded from interrupts and context switches.
//!
//! @return True if the data was written to the buffer. False for insufficient
//! space.
//
//*****************************************************************************
bool am_util_ring_buffer_write(am_util_ring_buffer_t *psBuffer,
                               void *pvSource, uint32_t ui32Bytes);

//*****************************************************************************
//
//! @brief Read a single byte from the ring buffer.
//!
//! @param psBuffer is the address of the ring buffer structure to be read.
//! @param pvDest is a pointer to the array that the new ringbuffer will use
//! for storage space
//! @param ui32Bytes is the total number of bytes that the ringbuffer will be
//! allowed to use.
//!
//! This function will write a single byte to the given ring buffer. Make sure
//! that the ring buffer is not already empty. If the ring buffer is empty,
//! this function will just return a NULL character.
//!
//! @note This operation is not inherently thread-safe, so the caller must make
//! sure that it is appropriately guarded from interrupts and context switches.
//!
//! @return The byte read from the buffer, or a NULL if the buffer was empty.
//
//*****************************************************************************
bool am_util_ring_buffer_read(am_util_ring_buffer_t *psBuffer,
                              void *pvDest, uint32_t ui32Bytes);

#ifdef __cplusplus
}
#endif

#endif // AM_UTIL_RING_BUFFER_H

//*****************************************************************************
//
// End Doxygen group.
//! @}
//
//*****************************************************************************

