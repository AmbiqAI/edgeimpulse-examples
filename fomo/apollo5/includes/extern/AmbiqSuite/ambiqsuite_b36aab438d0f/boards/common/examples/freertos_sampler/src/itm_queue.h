
//*****************************************************************************
//
//! @file itm_queue.h
//!
//! @brief Queue interface to ITM task for handling printf outputs.
//!
//! This queue collects data and responses from any other task that wants to
//! print via the ITM/SWO pin.
//!
//! The queue element here is essentially the union of any data structure that
//! is to be carried in an element of this queue. Sources of queue elements
//! include:
//! Application task
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

#ifndef ITM_QUEUE_H
#define ITM_QUEUE_H

#ifdef __cplusplus
extern "C" {
#endif


//*****************************************************************************
//
// Macro definitions
//
//*****************************************************************************
#define ITM_QUEUE_NUM_ELEMENTS   16

//*****************************************************************************
//
// Typedefs
//
//*****************************************************************************

typedef enum
{
  ITMQueue_ITM_REG = 0, // from RTOS tick timer
  ITMQueue_PRINT_IT,    // from student developed task.
} ITMQueueType_t;


// QUEUE entries for the
typedef struct
{
   uint32_t stim_register;
   uint32_t stim_value;
}ITMStimulusRegister_t;

// QUEUE entry for the ITM queue
typedef struct
{
   ITMQueueType_t ITMQueType;
   union
   {
       volatile uint32_t *pui32String;
       uint32_t RTOS_event;
       uint32_t student_value;
       ITMStimulusRegister_t s;
   }u;
}ITMQueueElement_t;


//*****************************************************************************
//
// Global Variables
//
//*****************************************************************************

extern QueueHandle_t g_ITMQueue;



//*****************************************************************************
//
// Interface Functions
//
//*****************************************************************************
bool ITMQueue_create(void);
bool ITMQueue_send(ITMQueueElement_t *Element); // blocking, but it can fail
bool ITMQueue_read(ITMQueueElement_t *Element); // blocking, but it can fail
bool ITMQueue_sendFromISR(ITMQueueElement_t *Element);
bool ITMQueue_is_full(void); // non-blocking
bool ITMQueue_is_empty(void); // non-blocking


#ifdef __cplusplus
}
#endif

#endif // ITM_QUEUE_H


