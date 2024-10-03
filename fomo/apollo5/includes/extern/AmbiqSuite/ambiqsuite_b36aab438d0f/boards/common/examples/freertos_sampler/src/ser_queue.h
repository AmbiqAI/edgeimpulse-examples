
//*****************************************************************************
//
//! @file app_queue.h
//!
//! @brief Queue interface to application task from everything but IOS.
//!
//! This queue collects data and responses from any other task except the IOS.
//! IOS responses are collected in the app_ios_queue.
//!
//! The queue element here is essentially the union of any data structure that
//! is to be carried in an element of this queue. Sources of queue elements
//! include:
//! Sensor Fusion Task
//! IOM task
//! Audio tasks
//! HRM task
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

#ifndef SER_QUEUE_H
#define SER_QUEUE_H

#ifdef __cplusplus
extern "C" {
#endif


//*****************************************************************************
//
// Macro definitions
//
//*****************************************************************************
#define SER_QUEUE_NUM_ELEMENTS   16

//*****************************************************************************
//
// Typedefs
//
//*****************************************************************************

typedef enum
{
    SERQueueSF_RTOS_EVENT = 0, // from RTOS tick timer
    SERQueueSF_STUDENT_EVENT,  // from student developed task.
} SERQueueType_t;

// QUEUE entry for the serial queue
typedef struct
{
    SERQueueType_t SerQueType;
    uint32_t       key_press;
    union
    {
        uint32_t RTOS_event;
        uint32_t student_value;
    } u;
}SERQueueElement_t;


//*****************************************************************************
//
// Global Variables
//
//*****************************************************************************

extern QueueHandle_t g_SERQueue;



//*****************************************************************************
//
// Interface Functions
//
//*****************************************************************************
bool SERQueue_create(void);
bool SERQueue_send(SERQueueElement_t *Element); // blocking, but it can fail
bool SERQueue_read(SERQueueElement_t *Element); // blocking, but it can fail
bool SERQueue_sendFromISR(SERQueueElement_t *Element);
bool SERQueue_is_full(void); // non-blocking
bool SERQueue_is_empty(void); // non-blocking


#ifdef __cplusplus
}
#endif

#endif // SER_QUEUE_H


