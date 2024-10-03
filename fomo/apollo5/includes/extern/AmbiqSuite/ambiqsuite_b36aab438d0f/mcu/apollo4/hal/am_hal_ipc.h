//*****************************************************************************
//
//  am_hal_ipc.h
//! @file
//!
//! @brief Functions for IPC Services
//!
//! @addtogroup ipc IPC
//! @ingroup apollo4hal
//! @{

//*****************************************************************************

//*****************************************************************************
//
// ${copyright}
//
// This is part of revision ${version} of the AmbiqSuite Development Package.
//
//*****************************************************************************

#ifndef AM_HAL_IPC_H
#define AM_HAL_IPC_H

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
// Macro definitions
//
//*****************************************************************************

#define  ARM_CORE_ID  0   //!< The ARM core ID
#define DSP1_CORE_ID  1   //!< The first DSP core ID
#define DSP2_CORE_ID  2   //!< The second DSP core ID

#define AM_HAL_NUM_CORES      3   //!< The number of cores. This is used to define number of Queues and event mask  and more

#define AM_HAL_IPC_MUTEX_NUM        4 //!< Currently only 4 Mutex are available for generic usage outside of the HAL
#define AM_HAL_IPC_MUTEX_WAIT_FOREVER   0xFFFFFFFF
#define AM_HAL_IPC_MUTEX_WAIT_NONE      0


#define MAX_NUM_IPC_EVENT_HANDLER     32   //!< Maximum IPC event handler that can be register(e.g. 32 means one per event ). No other number is supported.

#define IPC_QUEUE0_SIZE   64
#define IPC_QUEUE1_SIZE   64
#define IPC_QUEUE2_SIZE   64


// The reserve events for Queue management and handling.
// The reserved event for Queue management. When a queue is created/destroyed, this event raised by the other core.
#define SHORTMSG_MANAGEDQUEUE_MAILBOX_EVENT_FOR_ARM     26
#define SHORTMSG_MANAGEDQUEUE_MAILBOX_EVENT_FOR_DSP1    ( SHORTMSG_MANAGEDQUEUE_MAILBOX_EVENT_FOR_ARM + 1 )
#define SHORTMSG_MANAGEDQUEUE_MAILBOX_EVENT_FOR_DSP2    ( SHORTMSG_MANAGEDQUEUE_MAILBOX_EVENT_FOR_DSP1 + 1 )
// The reserved event for Queue handling. When a message received by the queue, this event is raised by the sender.
#define SHORTMSG_QUEUE_MAILBOX_EVENT_FOR_ARM     (SHORTMSG_MANAGEDQUEUE_MAILBOX_EVENT_FOR_DSP2 + 1)
#define SHORTMSG_QUEUE_MAILBOX_EVENT_FOR_DSP1    ( SHORTMSG_QUEUE_MAILBOX_EVENT_FOR_ARM + 1 )
#define SHORTMSG_QUEUE_MAILBOX_EVENT_FOR_DSP2    ( SHORTMSG_QUEUE_MAILBOX_EVENT_FOR_DSP1 + 1 )

//Returns the event ID for Queue Management events. Queue management event includes initialization and termination of the queues.
#define SHORTMSG_MANAGEQUEUE_EVENT_NO(_coreID)   ( _coreID + SHORTMSG_MANAGEDQUEUE_MAILBOX_EVENT_FOR_ARM ) // event 26 and 27 and 28 are for ARM, DSP1 and DSP2 queues
//Returns the event ID for queue events. Queue event includes sending and receiving messages.
#define SHORTMSG_QUEUE_EVENT_NO(_coreID)   ( _coreID + SHORTMSG_QUEUE_MAILBOX_EVENT_FOR_ARM ) // event 29 and 30 and 31 are for ARM, DSP1 and DSP2 queues

#define     AM_HAL_IPC_MEM_NOT_INIT          2   //todo: this should be placed in proper enum
#define     AM_HAL_IPC_RDQ_NOT_CREATED_YET   3   //todo: this should be placed in proper enum
#define     AM_HAL_IPC_WRQ_NOT_CREATED_YET   4   //todo: this should be placed in proper enum
//*****************************************************************************
//
// Structs and enums definitions
//
//*****************************************************************************
typedef enum
{
    AM_HAL_IPC_MUTEX_STATE_FREE =   0x0,    // Mutex is free
    AM_HAL_IPC_MUTEX_STATE_CPU  =   0x1,    // CPU Owns Mutex
    AM_HAL_IPC_MUTEX_STATE_DSP0 =   0x2,    // DSP0 Owns Mutex
    AM_HAL_IPC_MUTEX_STATE_DSP1 =   0x4,    // DSP1 Owns Mutex
} am_hal_ipc_mutex_state_e;

typedef struct
{
    uint32_t maxSize;
    uint32_t numEntries;
} am_hal_ipc_shortmsgQ_state_t;

typedef enum
{
    AM_HAL_IPC_CORE_STATUS_NOTCONNECTED = 0,
    AM_HAL_IPC_CORE_STATUS_CONNECTED,
} am_hal_ipc_core_status_e;

typedef struct
{
    // a copy for the remote index of the peer queue. It can point to Rx or Tx index.
    uint32_t    *pPeerIndex;
    // WriteIndex always points to the next free location
    uint32_t    ui32WrIndex;
    // ReadIndex always points to the current unread location
    uint32_t    ui32RdIndex;
    // The Queue buffer address. For Rx Queue the buffer is located inside
    // the core and for Tx Queue the buffer is in the remote core.
    uint32_t    *ui32QueueBuffer;
    // The size of the queue buffer
    uint32_t    ui32QueueSize;
} am_hal_shortMsgQ_descr_t;

typedef void (*am_hal_ipc_event_callback_t)(void *pCallbackCtxt);
typedef struct
{
    // The event handler function for REG_DSP_CPUMBINT/REG_DSP_DSP0MBINT/REG_DSP_DSP1MBINT events.
    am_hal_ipc_event_callback_t pHandler;
    // The call back parameter for pHandler
    void *pCallbackCtxt;
} am_hal_ipc_event_handler_t;

// (Processor specific) initialization of IPC context
typedef void (*am_hal_ipc_shortmsgQ_callback_t)(uint8_t fromCore, void *pCtxt);
typedef struct
{
    // The Rx queue descriptor
    am_hal_shortMsgQ_descr_t    sIpcRxQueue[AM_HAL_NUM_CORES];
    // The Tx queue descriptor. The Tx queue descriptor is a copy of Rx Queue descriptor in the remote peer
    am_hal_shortMsgQ_descr_t    sIpcTxQueue[AM_HAL_NUM_CORES];
    // event handler
    am_hal_ipc_event_handler_t  sIpcEvenrHandler[MAX_NUM_IPC_EVENT_HANDLER];
    // When an event handler is added to 'sIpcEvenrHandler', the corresponding bit in 'eventMask' is set to ONE.
    uint32_t                    eventMask[AM_HAL_NUM_CORES]; // Possible Events bitmask for receiver
    // When a message is received inside any of the 3 Rx Queues, the pMsgQHandler is called. This function is
    // only specified during the IPC initialization process. The function accepts two inputs:
    // Input 1: The Rx queue number
    // Input 2: pCtxt
    am_hal_ipc_shortmsgQ_callback_t pMsgQHandler; // the place holder for user queue receive handler
    // The pCtxt can only be initialized during IPC initialization and is the second input for the pMsgQHandler.
void *pCtxt;   // the storage place for input parameter for the user queue handler
} am_hal_ipc_t;

//*****************************************************************************
//
// External variable definitions
//
//*****************************************************************************


//*****************************************************************************
//
// External function definitions.
//
//*****************************************************************************
uint32_t am_hal_ipc_mutex_trylock(uint32_t mutexId);

// timeout 0 makes this equivalent to trylock
// timeout implies number of tries
uint32_t am_hal_ipc_mutex_lock(uint32_t mutexId, uint32_t timeout);
uint32_t am_hal_ipc_mutex_unlock(uint32_t mutexId);
uint32_t am_hal_ipc_mutex_get_state(uint32_t mutexId, am_hal_ipc_mutex_state_e *pState);

uint32_t am_hal_ipc_init(am_hal_ipc_shortmsgQ_callback_t pMsgQHandler, void *pCtxt);
uint32_t am_hal_ipc_term(void);

// Map the memory - for IPC messages
//uint32_t am_hal_ipc_mmap(uint32_t addr, uint8_t fromCore, uint8_t toCore, uint32_t *pMappedAddr);
uint32_t am_hal_ipc_mmap(uint32_t addr, uint8_t fromCore, uint8_t toCore, uint32_t *pMappedAddr);

// Event notification
uint32_t am_hal_ipc_event_register(uint32_t event, am_hal_ipc_event_callback_t pHandler, void *pCallbackCtxt);
uint32_t am_hal_ipc_event_send(uint8_t toCore, uint32_t event);
//void am_hal_ipc_int_status_get(uint32_t eventMask);
uint32_t am_hal_ipc_int_clear(uint32_t eventMask);
// Event ISR Handler
void am_hal_ipc_int_service(uint32_t eventMask);
// Short Message Queue
uint32_t am_hal_ipc_shortmsg_send(uint8_t toCore, uint32_t ui32Msg);
uint32_t am_hal_ipc_shortmsg_get(uint8_t fromCore, uint32_t *pui32Msg);

uint32_t am_hal_ipc_shortmsgQ_get_state(uint8_t fromCore, uint8_t toCore, am_hal_ipc_shortmsgQ_state_t *pQState);

// Utility functions - subset of the get_state
bool am_hal_ipc_shortmsgQ_is_empty(uint8_t fromCore);
bool am_hal_ipc_shortmsgQ_is_full(uint8_t toCore);

uint32_t am_hal_ipc_get_status(uint8_t core, am_hal_ipc_core_status_e *pCoreStatus);

///TODO: The following should be checked for the final hardware
#define GLOBAL_SHARED_ADD    0x12345678    //todo: what is the shared memory location.
#define CONVERT_PRIMARY_TO_SECONDARY_ADDRESS(_a)  _a + 0x00000   //offset
#define CONVERT_SECONDARY_TO_PRIMARY_ADDRESS(_a)  _a - 0x000000  //offset


#ifdef __cplusplus
}
#endif

#endif // AM_HAL_IPC_H

//*****************************************************************************
//
// End Doxygen group.
//! @}
//
//*****************************************************************************
