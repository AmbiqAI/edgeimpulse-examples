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

#ifndef APP_QUEUE_H
#define APP_QUEUE_H

#ifdef __cplusplus
extern "C" {
#endif


//*****************************************************************************
//
// Macro definitions
//
//*****************************************************************************
#define APP_QUEUE_NUM_ELEMENTS   16

//*****************************************************************************
//
// Typedefs
//
//*****************************************************************************

typedef enum
{
  APPQueueSF_RTOS_EVENT = 0,    // from RTOS tick timer
  APPQueueSF_STUDENT_EVENT,     // from student developed task.
} APPQueueType_t;

// QUEUE entry for the APP queue
typedef struct
{
   APPQueueType_t AppQueType;
   uint32_t       key_press;
   union
   {
       uint32_t RTOS_event;
       uint32_t student_value;
   }u;
}APPQueueElement_t;


//*****************************************************************************
//
// Global Variables
//
//*****************************************************************************

extern QueueHandle_t g_APPQueue;



//*****************************************************************************
//
// Interface Functions
//
//*****************************************************************************
bool APPQueue_create(void);
bool APPQueue_send(APPQueueElement_t *Element); // blocking, but it can fail
bool APPQueue_read(APPQueueElement_t *Element); // blocking, but it can fail
bool APPQueue_sendFromISR(APPQueueElement_t *Element);
bool APPQueue_is_full(void); // non-blocking
bool APPQueue_is_empty(void); // non-blocking


#ifdef __cplusplus
}
#endif

#endif // APP_QUEUE_H


