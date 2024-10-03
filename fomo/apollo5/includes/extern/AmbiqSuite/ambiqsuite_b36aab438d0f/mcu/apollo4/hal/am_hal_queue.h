//*****************************************************************************
//
//  am_hal_queue.h
//! @file
//!
//! @brief Functions for implementing a queue system.
//!
//! @addtogroup queue4
//! @ingroup apollo4hal
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
#ifndef AM_HAL_QUEUE_H
#define AM_HAL_QUEUE_H

//*****************************************************************************
//
//! @brief A data structure that will operate as a queue.
//!
//! This data structure holds information necessary for operating a thread-safe
//! queue. When declaring a structure of type am_hal_queue_t, you will also need
//! to provide some working memory for the queue to use. For more information on
//! setting up and using the am_hal_queue_t structure, please see the
//! documentation for am_hal_queue_init().
//
//*****************************************************************************
typedef struct
{
    volatile uint32_t ui32WriteIndex;
    volatile uint32_t ui32ReadIndex;
    volatile uint32_t ui32Length;
    uint32_t ui32Capacity;
    uint32_t ui32ItemSize;
    uint8_t *pui8Data;
}
am_hal_queue_t;

//*****************************************************************************
//
// Function-like macros.
//
//*****************************************************************************

//
// Returns true if the queue is empty.
//
#define am_hal_queue_empty(psQueue)                                           \
    ((psQueue)->ui32Length == 0)

//
// Returns true if the queue is full.
//
#define am_hal_queue_full(psQueue)                                            \
    ((psQueue)->ui32Length == (psQueue)->ui32Capacity)

//
// Returns the amount of space left in the queue (in bytes).
//
#define am_hal_queue_space_left(psQueue)                                      \
    ((psQueue)->ui32Capacity - (psQueue)->ui32Length)

//
// Returns the number of configured items that will fit in the queue.
//
#define am_hal_queue_slots_left(psQueue)                                      \
    (((psQueue)->ui32Capacity - (psQueue)->ui32Length)                        \
     / (psQueue)->ui32ItemSize)

//
// Returns the amount of data in the queue (in bytes).
//
#define am_hal_queue_data_left(psQueue)                                       \
    ((psQueue)->ui32Length)

//
// Returns the number of configured items left in the queue.
//
#define am_hal_queue_items_left(psQueue)                                      \
    ((psQueue)->ui32Length / (psQueue)->ui32ItemSize)

//
// Can be used as a pointer to the next item to be read from the queue.
//
#define am_hal_queue_peek(psQueue)                                            \
    ((void *) &((psQueue)->pui8Data[(psQueue)->ui32ReadIndex]))

//
// Can be used as a pointer to the next available slot in the queue memory.
//
#define am_hal_queue_next_slot(psQueue)                                       \
    ((void *) &((psQueue)->pui8Data[(psQueue)->ui32WriteIndex]))

//*****************************************************************************
//
// Use this to make sure you get the size parameters right.
//
//*****************************************************************************
#define am_hal_queue_from_array(queue, array)                                 \
    am_hal_queue_init((queue), (array), sizeof((array)[0]), sizeof(array))

#ifdef __cplusplus
extern "C"
{
#endif

//*****************************************************************************
//
// External function definitions.
//
//*****************************************************************************
extern void am_hal_queue_init(am_hal_queue_t *psQueue, void *pvData, uint32_t ui32ItemSize, uint32_t ui32ArraySize);
extern bool am_hal_queue_item_add(am_hal_queue_t *psQueue, const void *pvSource, uint32_t ui32NumItems);
extern bool am_hal_queue_item_get(am_hal_queue_t *psQueue, void *pvDest, uint32_t ui32NumItems);

#ifdef __cplusplus
}
#endif

#endif // AM_HAL_QUEUE_H

//*****************************************************************************
//
// End Doxygen group.
//! @}
//
//*****************************************************************************
