//*****************************************************************************
//
//! @file am_hal_ipc.h
//!
//! @brief Functions for Interprocessor Communication Services
//!
//! @addtogroup ipc_4b IPC - Interprocessor Communication Services
//! @ingroup apollo4b_hal
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

#define ARM_CORE_ID   0   //!< The ARM core ID
#define DSP1_CORE_ID  1   //!< The first DSP core ID
#define DSP2_CORE_ID  2   //!< The second DSP core ID

#define AM_HAL_NUM_CORES      3   //! The number of cores. This is used to define number of Queues and event mask  and more

#define AM_HAL_IPC_MUTEX_NUM        4 //! Currently only 4 Mutex are available for generic usage outside of the HAL
#define AM_HAL_IPC_MUTEX_WAIT_FOREVER   0xFFFFFFFF
#define AM_HAL_IPC_MUTEX_WAIT_NONE      0

#define MAX_NUM_IPC_EVENT_HANDLER     32   //! Maximum IPC event handler that can be register(e.g. 32 means one per event ). No other number is supported.

#define IPC_QUEUE0_SIZE   64
#define IPC_QUEUE1_SIZE   64
#define IPC_QUEUE2_SIZE   64

//! The reserve events for Queue management and handling.
//! The reserved event for Queue management. When a queue is created/destroyed, this event raised by the other core.
#define SHORTMSG_MANAGEDQUEUE_MAILBOX_EVENT_FOR_ARM     26
#define SHORTMSG_MANAGEDQUEUE_MAILBOX_EVENT_FOR_DSP1    ( SHORTMSG_MANAGEDQUEUE_MAILBOX_EVENT_FOR_ARM + 1 )
#define SHORTMSG_MANAGEDQUEUE_MAILBOX_EVENT_FOR_DSP2    ( SHORTMSG_MANAGEDQUEUE_MAILBOX_EVENT_FOR_DSP1 + 1 )
//! The reserved event for Queue handling. When a message received by the queue, this event is raised by the sender.
#define SHORTMSG_QUEUE_MAILBOX_EVENT_FOR_ARM     (SHORTMSG_MANAGEDQUEUE_MAILBOX_EVENT_FOR_DSP2 + 1)
#define SHORTMSG_QUEUE_MAILBOX_EVENT_FOR_DSP1    ( SHORTMSG_QUEUE_MAILBOX_EVENT_FOR_ARM + 1 )
#define SHORTMSG_QUEUE_MAILBOX_EVENT_FOR_DSP2    ( SHORTMSG_QUEUE_MAILBOX_EVENT_FOR_DSP1 + 1 )

//! Returns the event ID for Queue Management events. Queue management event includes initialization and termination of the queues.
#define SHORTMSG_MANAGEQUEUE_EVENT_NO(_coreID)   ( _coreID + SHORTMSG_MANAGEDQUEUE_MAILBOX_EVENT_FOR_ARM ) // event 26 and 27 and 28 are for ARM, DSP1 and DSP2 queues
//! Returns the event ID for queue events. Queue event includes sending and receiving messages.
#define SHORTMSG_QUEUE_EVENT_NO(_coreID)   ( _coreID + SHORTMSG_QUEUE_MAILBOX_EVENT_FOR_ARM ) // event 29 and 30 and 31 are for ARM, DSP1 and DSP2 queues

#define     AM_HAL_IPC_MEM_NOT_INIT          2   //todo: this should be placed in proper enum
#define     AM_HAL_IPC_RDQ_NOT_CREATED_YET   3   //todo: this should be placed in proper enum
#define     AM_HAL_IPC_WRQ_NOT_CREATED_YET   4   //todo: this should be placed in proper enum

//*****************************************************************************
//
// Structs and enums definitions
//
//*****************************************************************************
//
//! @brief State of the IPC Mutex
//
typedef enum
{
    AM_HAL_IPC_MUTEX_STATE_FREE =   0x0,
    AM_HAL_IPC_MUTEX_STATE_CPU  =   0x1,
    AM_HAL_IPC_MUTEX_STATE_DSP0 =   0x2,
    AM_HAL_IPC_MUTEX_STATE_DSP1 =   0x4,
} am_hal_ipc_mutex_state_e;

//
//! @brief State of the Short Message Queue for IPC
//
typedef struct
{
    uint32_t maxSize;
    uint32_t numEntries;
} am_hal_ipc_shortmsgQ_state_t;

//
//! @brief Connection Status of the IPC Core
//
typedef enum
{
    AM_HAL_IPC_CORE_STATUS_NOTCONNECTED = 0,
    AM_HAL_IPC_CORE_STATUS_CONNECTED,
} am_hal_ipc_core_status_e;

//
//! @brief Description of the Short Message Queue for IPC
//
typedef struct
{
    //! a copy for the remote index of the peer queue. It can point to Rx or Tx index.
    uint32_t    *pPeerIndex;
    //! WriteIndex always points to the next free location
    uint32_t    ui32WrIndex;
    //! ReadIndex always points to the current unread location
    uint32_t    ui32RdIndex;
    //! The Queue buffer address. For Rx Queue the buffer is located inside
    //! the core and for Tx Queue the buffer is in the remote core.
    uint32_t    *ui32QueueBuffer;
    //! The size of the queue buffer
    uint32_t    ui32QueueSize;
} am_hal_shortMsgQ_descr_t;

typedef void (*am_hal_ipc_event_callback_t)(void *pCallbackCtxt);
typedef struct
{
    //! The event handler function for REG_DSP_CPUMBINT/REG_DSP_DSP0MBINT/REG_DSP_DSP1MBINT events.
    am_hal_ipc_event_callback_t pHandler;
    //! The call back parameter for pHandler
    void *pCallbackCtxt;
} am_hal_ipc_event_handler_t;

// (Processor specific) initialization of IPC context
typedef void (*am_hal_ipc_shortmsgQ_callback_t)(uint8_t fromCore, void *pCtxt);
typedef struct
{
    //! The Rx queue descriptor
    am_hal_shortMsgQ_descr_t    sIpcRxQueue[AM_HAL_NUM_CORES];

    //! The Tx queue descriptor. The Tx queue descriptor is a copy of Rx Queue descriptor in the remote peer
    am_hal_shortMsgQ_descr_t    sIpcTxQueue[AM_HAL_NUM_CORES];

    //! event handler
    am_hal_ipc_event_handler_t  sIpcEvenrHandler[MAX_NUM_IPC_EVENT_HANDLER];

    //! When an event handler is added to 'sIpcEvenrHandler', the corresponding bit in 'eventMask' is set to ONE.
    //! Possible Events bitmask for receiver
    uint32_t                    eventMask[AM_HAL_NUM_CORES];

    //! When a message is received inside any of the 3 Rx Queues, the pMsgQHandler is called. This function is
    //! only specified during the IPC initialization process. The function accepts two inputs:
    //! Input 1: The Rx queue number
    //! Input 2: pCtxt
    am_hal_ipc_shortmsgQ_callback_t pMsgQHandler; // the place holder for user queue receive handler

    //! The pCtxt can only be initialized during IPC initialization and is the second input for the pMsgQHandler.
    //! the storage place for input parameter for the user queue handler
    void *pCtxt;
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

///TODO: The following should be checked for the final hardware
#define GLOBAL_SHARED_ADD    0x12345678    //todo: what is the shared memory location.
#define CONVERT_PRIMARY_TO_SECONDARY_ADDRESS(_a)  _a + 0x00000   //offset
#define CONVERT_SECONDARY_TO_PRIMARY_ADDRESS(_a)  _a - 0x000000  //offset

//*****************************************************************************
//
//! @brief the management event service
//!
//! @param par pointer to return fromCore parameter
//
//*****************************************************************************
extern void am_hal_ipc_shortmsg_init_service(void * par);

//*****************************************************************************
//
//! @brief terminate the local queues and send a management event to the other side
//!
//! When this function is called, first clears the shared memory address of the queues.
//! Then clears the local copy of the queue descriptors to zero. At last, it sends a
//! queue management event to the remote side.
//! Important: After calling am_hal_ipc_term only am_hal_ipc_init function is allowed,
//! and no any other function from IPC Hal can be called.
//!
//! @return AM_HAL_STATUS_SUCCESS or applicable errors
//
//*****************************************************************************
extern uint32_t am_hal_ipc_term(void);

//*****************************************************************************
//
//! @brief Map the memory - for IPC messages
//!
//! @param fromCore
//! @param toCore
//! @param pMappedAddr
//!
//! @return AM_HAL_STATUS_SUCCESS or applicable errors
//
//*****************************************************************************
extern uint32_t am_hal_ipc_mmap(uint32_t addr, uint8_t fromCore,
                                                    uint8_t toCore,
                                                    uint32_t *pMappedAddr);

//*****************************************************************************
//
//! @brief Register a function for an event and send a management event to other cores
//!
//! The function registers a handler for an incoming event. Also update the maskEvent
//! in local and shared memory. And finally, send a management event to the other cores.
//!
//! @param event
//! @param pHandler
//! @param pCallbackCtxt
//!
//! @return AM_HAL_STATUS_SUCCESS or applicable errors
//
//*****************************************************************************
extern uint32_t am_hal_ipc_event_register(uint32_t event,
                                        am_hal_ipc_event_callback_t pHandler,
                                        void *pCallbackCtxt);

//*****************************************************************************
//
//! @brief Register a function for an event and send a management event to other cores
//!
//! The function registers a handler for an incoming event. Also update the maskEvent
//! in local and shared memory. And finally, send a management event to the other cores.
//!
//! @param core
//!
//! @return AM_HAL_STATUS_SUCCESS or applicable errors
//
//*****************************************************************************
extern uint32_t * am_hal_mail_box_address(uint8_t core);

//*****************************************************************************
//
//! @brief send an event to other cores
//!
//! If the maskEvent bit for the other core, accept the event, then send an event to
//! other core.
//!
//! @param toCore
//! @param event
//!
//! @return AM_HAL_STATUS_SUCCESS or applicable errors
//
//*****************************************************************************
extern uint32_t am_hal_ipc_event_send(uint8_t toCore, uint32_t event);

//*****************************************************************************
//
//! @brief get the event status bit
//!
//! Return the event status bit.
//!
//! @param pEventMask Interrupt Servvice Event Mask
//!
//! @return AM_HAL_STATUS_SUCCESS or applicable errors
//
//*****************************************************************************
extern uint32_t am_hal_ipc_int_status_get(uint32_t *pEventMask);

//*****************************************************************************
//
//! @brief clears the event status bit
//!
//! Clear the event status bit.
//!
//! @param eventMask Interrupt Servvice Event Mask
//!
//! @return status
//
//*****************************************************************************
extern uint32_t am_hal_ipc_int_clear(uint32_t eventMask);

//*****************************************************************************
//
//! @brief call registered handler for each event
//!
//! Any handler, registered for a specific event,is called by am_hal_ipc_int_service.
//! This function must be registered by Interrupt Hal for an IPC event interrupt.
//!
//! @param eventMask Interrupt Servvice Event Mask
//!
//
//*****************************************************************************
extern void am_hal_ipc_int_service(uint32_t eventMask);

//*****************************************************************************
//
//! @brief send a 32bit message to the other core
//!
//! The function write the msg directly to the remote-side queue and then increments
//! the local write index. After than it copies the local write index into remote
//! write index and finally send a queue event to the other core.
//!
//! parameters.
//!
//! @return status
//
//*****************************************************************************
extern uint32_t am_hal_ipc_shortmsg_send(uint8_t toCore, uint32_t ui32Msg);

//*****************************************************************************
//
//! @brief the event handler for any incoming queue message
//!
//! After a queue msg is written into the queue from the peer side, the queue event
//! is sent to the host side. The Hal interrupt calls the 'IPC event service'.
//! The 'IPC event service' calls the current function and then this function calls
//! user function.
//!
//! @param par pointer to return fromCore parameter
//!
//
//*****************************************************************************
extern void am_hal_ipc_shortmsg_service(void * par);

//*****************************************************************************
//
//! @brief the event handler for any incoming queue message
//!
//! The function only get the data from the message buffer without incrementing
//! the read pointer. This function is useful when a host send a message and
//! wait for the remote side to process the message. The host may assume, when
//! the message is read, the message processing is done, so there is no need to
//! keep the resource associated with that message. To make sure host can
//! safely rely on the Fifo-empty message to release the resource,
//! am_hal_ipc_shortmsg_get_only_msg only reads the message without
//! incrementing the index pointer. After reading the message with this
//! function, always am_hal_ipc_shortmsg_get should be executed to actually
//! receive the message and inform the Tx side(host) that the message is
//! successfully received.
//!
//! @param fromCore
//! @param pui32Msg
//!
//! @return status
//
//*****************************************************************************
extern uint32_t am_hal_ipc_shortmsg_get_only_msg(uint8_t fromCore,
                                                    uint32_t *pui32Msg);

//*****************************************************************************
//
//! @brief the event handler for any incoming queue message
//!
//! The  am_hal_ipc_shortmsg_get check the queue empty status. If the Rx queue
//! is not empty, the function reads the //! queue contents and then increment
//! rx-queue read pointer. After message completely removed from the rx queue,
//! the tx side(host) informed by incrementing the read pointer inside host by
//! one.
//!
//! @param fromCore
//! @param pui32Msg
//!
//! @return status
//
//*****************************************************************************
extern uint32_t am_hal_ipc_shortmsg_get(uint8_t fromCore, uint32_t *pui32Msg);

//*****************************************************************************
//
//! @brief Get the size of the queue between two cores.
//!
//! @param fromCore
//! @param toCore
//! @param pQState
//!
//! @return status
//
//*****************************************************************************
extern uint32_t am_hal_ipc_shortmsgQ_get_state(uint8_t fromCore,
                                    uint8_t toCore,
                                    am_hal_ipc_shortmsgQ_state_t *pQState);

//*****************************************************************************
//
//! @brief check if the Rx Queue is empty
//!
//! @param fromCore
//!
//! @return bool_status queue empty status
//
//*****************************************************************************
extern bool am_hal_ipc_shortmsgQ_is_empty(uint8_t fromCore);

//*****************************************************************************
//
//! @brief check if the Tx Queue is full
//!
//! @param toCore
//!
//! @return bool_status queue full status
//
//*****************************************************************************
extern bool am_hal_ipc_shortmsgQ_is_full(uint8_t toCore);

//*****************************************************************************
//
//! @brief get IPC Status
//!
//! @param core
//! @param pCoreStatus
//!
//! @return AM_HAL_STATUS_SUCCESS or applicable errors
//
//*****************************************************************************
extern uint32_t am_hal_ipc_get_status(uint8_t core,
                                        am_hal_ipc_core_status_e *pCoreStatus);

//*****************************************************************************
//
//! @brief initialization of IPC context
//!
//! The ipc init function is the first function( before any other IPC function)
//! which should be called. It initialize the IPC queue and events.
//!
//! @param pMsgQHandler
//! @param pCtxt
//!
//! @return status
//
//*****************************************************************************
extern uint32_t am_hal_ipc_init(am_hal_ipc_shortmsgQ_callback_t pMsgQHandler, void *pCtxt);

//*****************************************************************************
//
//! @brief IPC Mutex Trylock
//!
//! @param mutexId
//!
//! @return status
//
//*****************************************************************************
extern uint32_t am_hal_ipc_mutex_trylock(uint32_t mutexId);

//*****************************************************************************
//
//! @brief IPC Mutex Lock
//!
//! @param mutexId
//! @param timeout
//!
//! @return status
//
//*****************************************************************************
extern uint32_t am_hal_ipc_mutex_lock(uint32_t mutexId, uint32_t timeout);

//*****************************************************************************
//
//! @brief IPC Mutex Unlock
//!
//! @param mutexId
//! @param pCtx//!
//! @return status
//
//*****************************************************************************
extern uint32_t am_hal_ipc_mutex_unlock(uint32_t mutexId);

//*****************************************************************************
//
//! @brief IPC Mutex Get State
//!
//! @param mutexId
//! @param pState
//!
//! @return status
//
//*****************************************************************************
extern uint32_t am_hal_ipc_mutex_get_state(uint32_t mutexId, am_hal_ipc_mutex_state_e *pState);

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

