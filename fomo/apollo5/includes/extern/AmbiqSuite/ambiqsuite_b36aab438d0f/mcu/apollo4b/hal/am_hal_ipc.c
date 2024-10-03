//*****************************************************************************
//
//!  @file am_hal_ipc.c
//!
//!  @brief Functions for Interprocessor Communication Services
//!
//!  @addtogroup ipc_4b IPC - Interprocessor Communication Services
//!  @ingroup apollo4b_hal
//!  @{

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
#include "am_hal_ipc.h"
#include "am_hal_shmem.h"

//*****************************************************************************
//
// Macro definitions
//
//*****************************************************************************
// The index of MailBox registered with respect to REG_DSP_CPUMBINTA
#define MBINTA_INDEX        0
#define MBINTB_INDEX        1
#define MBINTSTAT_INDEX     2
#define CPUMBDATA_INDEX     3
#define DSP0MBDATA_INDEX    4
#define DSP1MBDATA_INDEX    5

#define MUTEX_OFFSET            (&DSP->MUTEX1 - &DSP->MUTEX0)

#define DSP_MUTEX0_MUTEX_CLEAR  DSP_MUTEX0_MUTEX0_CLEAR
#define DSP_MUTEX0_MUTEX_SET    DSP_MUTEX0_MUTEX0_SET

// #### INTERNAL BEGIN ####
// [FIXME] CAJ: The CPUMBINT register model has changed. Now there are separate
// registers for setting and clearing the interrupt bits. The change below will
// get the HAL to compile, but we need to re-work the functions that use these
// macros to use the new register model.
//
// #define  MBINT_SET     DSP_CPUMBINT_CPUMBINT_SET
// #define  MBINT_CLR     DSP_CPUMBINT_CPUMBINT_CLR
//
// #### INTERNAL END ####
#define  MBINT_SET     0x01
#define  MBINT_CLR     0x02

//
//! The NSAU instruction returns the bit index of the first MSB
//
#define NSAU_RETURNS_ALL_BITS_ZERO             32               // < NSAU assembly instruction return value when all bits are zero

#define NEXT_LEADING_ZEROS(_out, _in)       _out = am_count_num_leading_zeros(_in); _in ^= ( 1 << (_out) )

//*****************************************************************************
//
//! @name Queue full/empty/used-word status
//! @{
//
//*****************************************************************************
#define IS_QUEUE_FULL(_wr, _rd, _size)      ( _wr == (_size - 1)) ? (_rd == 0) : ( (_wr + 1) == _rd )
#define IS_QUEUE_EMPTY(_wr, _rd)            ( _wr == _rd )
#define QUEUE_USED_WORDS(_wr, _rd, _size)   ((_wr) >= (_rd) ) ? (_wr - (_rd)) : (_size - (_rd) + _wr )
//! @}

//*****************************************************************************
//
// Global Variables
//
//*****************************************************************************

//
//  the main structure for all IPC variables, including Queues, MASK,
//
am_hal_ipc_t sIpcHal;

//
//  The Queue buffer
//
uint32_t ipcQueue0Buffer[IPC_QUEUE0_SIZE]; // the Rx Queue buffer in internal memory. This buffer is receiving msg from ARM.
uint32_t ipcQueue1Buffer[IPC_QUEUE1_SIZE]; // the Rx Queue buffer in internal memory. This buffer is receiving msg from DSP1.
uint32_t ipcQueue2Buffer[IPC_QUEUE2_SIZE]; // the Rx Queue buffer in internal memory. This buffer is receiving msg from DSP2.

//
// The IPC Queue is designed based on single input/single output queue.
// Short Message Queue
//

//*****************************************************************************
//
// get the information of the peer queue from the shared memory
//
//*****************************************************************************
uint32_t
am_hal_ipc_update_queue_from_shared_mem(uint32_t fromCore)
{
    am_hal_shortMsgQ_descr_t *remoteWrQueue = sHalSharedMem->sIpcSharedState.ipcInfo[fromCore][am_hal_get_core_id()].pWriteDescr;
    am_hal_shortMsgQ_descr_t *remoteRdQueue = sHalSharedMem->sIpcSharedState.ipcInfo[fromCore][am_hal_get_core_id()].pReadDescr;

    if ( remoteWrQueue == NULL)
    {
        return AM_HAL_IPC_WRQ_NOT_CREATED_YET;
    }
    if ( remoteRdQueue == NULL)
    {
        return AM_HAL_IPC_RDQ_NOT_CREATED_YET;
    }

    am_hal_ipc_mmap((uint32_t)remoteWrQueue, fromCore, am_hal_get_core_id(), (uint32_t *)remoteWrQueue);
    am_hal_ipc_mmap((uint32_t)remoteRdQueue, fromCore, am_hal_get_core_id(), (uint32_t *)remoteRdQueue);

// update the TX copy
    sIpcHal.sIpcTxQueue[fromCore].pPeerIndex = &remoteRdQueue->ui32WrIndex;
    // update the remote write address
    *sIpcHal.sIpcTxQueue[fromCore].pPeerIndex = sIpcHal.sIpcTxQueue[fromCore].ui32WrIndex;
    sIpcHal.sIpcTxQueue[fromCore].ui32RdIndex = remoteRdQueue->ui32RdIndex;
    am_hal_ipc_mmap((uint32_t) remoteRdQueue->ui32QueueBuffer, fromCore, am_hal_get_core_id(), (uint32_t *)&sIpcHal.sIpcTxQueue[fromCore].ui32QueueBuffer);
    sIpcHal.sIpcTxQueue[fromCore].ui32QueueSize = remoteRdQueue->ui32QueueSize;

//update the Rx copy
    sIpcHal.sIpcRxQueue[fromCore].pPeerIndex  = &remoteRdQueue->ui32RdIndex;
    // update peer with the current ReadIndex
    *sIpcHal.sIpcRxQueue[fromCore].pPeerIndex = sIpcHal.sIpcRxQueue[fromCore].ui32RdIndex;

// update the latest event Mask bit for the core from shared memory.
    sIpcHal.eventMask[fromCore] = sHalSharedMem->sIpcSharedState.eventMask[fromCore];
    return AM_HAL_STATUS_SUCCESS;
}

// Each queue has two descriptor. The Queue body is on the receive side but there are two descriptors(containing Rd & Wr index & ...)
// on each side. These two descriptors are identical and they are being updated after each write and read to the queue.
// Example : Sending a message from Host to Queue
// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------
// |   Host Descriptor :      |             Host ACTION                              |  Remote Descriptor :                     |     Remote Action
// |                          |  Write the message contents to the remote memory     |    ui32QueueBuffer[WriteIndex] = VALUE   |
// |         WriteIndex++     |  Host updates its local write index.                 |                                          |
// |                          |  Host updates the remote copy of write index         |       WriteIndex++                       |
// |                          |  Host sends an event to remote                       |                                          |
// |                          |                                                      |    ui32QueueBuffer[ReadIndex]            |   Remote reads VALUE from the Queue
// |                          |                                                      |       ReadIndex++                        |   Remote updates the local copy of ReadIndex
// |         ReadIndex++      |                                                      |                                          |   Remote updates the Host copy of ReadIndex
// |--------------------------------------------------------------------------------------------------------------------------------------------------------------------------

//*****************************************************************************
//
// write the information of the queue to the shared memory
//
//*****************************************************************************
uint32_t
am_hal_ipc_update_shared_mem_with_queue_data(uint32_t core)
{
    // Update the shared memory with the address of the local queue.
    // After executing this function, the remote side can rebuild the connection between the read/write
    // Descriptors of the same queue and then read or write to the queue
    sHalSharedMem->sIpcSharedState.ipcInfo[am_hal_get_core_id()][core ].pWriteDescr = &sIpcHal.sIpcTxQueue[core ];
    sHalSharedMem->sIpcSharedState.ipcInfo[am_hal_get_core_id()][core ].pReadDescr  = &sIpcHal.sIpcRxQueue[core ];
    // update shared memory with the latest event mask bit
    sHalSharedMem->sIpcSharedState.eventMask[am_hal_get_core_id()] = sIpcHal.eventMask[am_hal_get_core_id()];
    return AM_HAL_STATUS_SUCCESS;
}

//*****************************************************************************
//
// write the information of the queue to the shared memory
//
//*****************************************************************************
uint32_t
am_hal_ipc_local_queue_init(uint32_t core)
{
    // reset and initialized the receive Queue(on the host side) for the 'core'.
    // Also the connection between this Queue and its descriptor on the remote side is reset.
    // after this function, the queue needs to be connected to its peer on the remote side.
    memset((void*) &sIpcHal.sIpcRxQueue[core], 0, sizeof(sIpcHal.sIpcRxQueue[core]));
    memset((void*) &sIpcHal.sIpcTxQueue[core], 0, sizeof(sIpcHal.sIpcTxQueue[core]));
    if ( core == ARM_CORE_ID )
    {
        memset( ipcQueue0Buffer, 0, sizeof(ipcQueue0Buffer));
        sIpcHal.sIpcRxQueue[ ARM_CORE_ID].ui32QueueBuffer = ipcQueue0Buffer;
        sIpcHal.sIpcRxQueue[ ARM_CORE_ID].ui32QueueSize   = sizeof(ipcQueue0Buffer) / sizeof(ipcQueue0Buffer[0]);
    }
    else if ( core == DSP1_CORE_ID )
    {
        memset( ipcQueue1Buffer, 0, sizeof(ipcQueue1Buffer));
        sIpcHal.sIpcRxQueue[DSP1_CORE_ID].ui32QueueBuffer = ipcQueue1Buffer;
        sIpcHal.sIpcRxQueue[DSP1_CORE_ID].ui32QueueSize   = sizeof(ipcQueue1Buffer) / sizeof(ipcQueue1Buffer[0]);
    }
    else if ( core == DSP2_CORE_ID )
    {
        memset( ipcQueue2Buffer, 0, sizeof(ipcQueue2Buffer));
        sIpcHal.sIpcRxQueue[DSP2_CORE_ID].ui32QueueBuffer = ipcQueue2Buffer;
        sIpcHal.sIpcRxQueue[DSP2_CORE_ID].ui32QueueSize   = sizeof(ipcQueue2Buffer) / sizeof(ipcQueue2Buffer[0]);
    }
    return AM_HAL_STATUS_SUCCESS;
}

//*****************************************************************************
//
// try locking the mutex
//
//*****************************************************************************
uint32_t
am_hal_ipc_mutex_trylock(uint32_t mutexId)
{
    volatile uint32_t *pMutex;

#ifndef AM_HAL_DISABLE_API_VALIDATION
    if (mutexId >= AM_HAL_IPC_MUTEX_NUM)
    {
        return AM_HAL_STATUS_OUT_OF_RANGE;
    }
#endif
    pMutex = &DSP->MUTEX0 + mutexId*MUTEX_OFFSET;

    *pMutex = DSP_MUTEX0_MUTEX_SET;

    return ((*pMutex == (1 << am_hal_get_core_id())) ? AM_HAL_STATUS_SUCCESS : AM_HAL_STATUS_FAIL);
}

//*****************************************************************************
//
// lock the mutex
//
//*****************************************************************************
uint32_t
am_hal_ipc_mutex_lock(uint32_t mutexId, uint32_t timeout)
{
    volatile uint32_t *pMutex;
    uint32_t value;

#ifndef AM_HAL_DISABLE_API_VALIDATION
    if (mutexId >= AM_HAL_IPC_MUTEX_NUM)
    {
        return AM_HAL_STATUS_OUT_OF_RANGE;
    }
#endif
    pMutex = &DSP->MUTEX0 + mutexId*MUTEX_OFFSET;
    do
    {
        *pMutex = DSP_MUTEX0_MUTEX_SET;
        value = *pMutex;
        if (timeout != AM_HAL_IPC_MUTEX_WAIT_FOREVER)
        {
            if (!timeout--)
            {
                break;
            }
        }
    } while ((value != (1 << am_hal_get_core_id())));
    if (value == (1 << am_hal_get_core_id()))
    {
        return AM_HAL_STATUS_SUCCESS;
    }
    else
    {
        return AM_HAL_STATUS_TIMEOUT;
    }
}

//*****************************************************************************
//
// unlock the mutex
//
//*****************************************************************************
uint32_t
am_hal_ipc_mutex_unlock(uint32_t mutexId)
{
    volatile uint32_t *pMutex;
    uint32_t value;

#ifndef AM_HAL_DISABLE_API_VALIDATION
    if (mutexId >= AM_HAL_IPC_MUTEX_NUM)
    {
        return AM_HAL_STATUS_OUT_OF_RANGE;
    }
#endif
    pMutex = &DSP->MUTEX0 + mutexId*MUTEX_OFFSET;

    value = *pMutex;
    if (value == (1 << am_hal_get_core_id()))
    {
        *pMutex = DSP_MUTEX0_MUTEX_CLEAR;
        return AM_HAL_STATUS_SUCCESS;
    }
    else
    {
        return AM_HAL_STATUS_INVALID_OPERATION;
    }
}

//*****************************************************************************
//
// get the mutex state
//
//*****************************************************************************

uint32_t
am_hal_ipc_mutex_get_state(uint32_t mutexId, am_hal_ipc_mutex_state_e *pState)
{
    volatile uint32_t *pMutex;

#ifndef AM_HAL_DISABLE_API_VALIDATION
    if (mutexId >= AM_HAL_IPC_MUTEX_NUM)
    {
        return AM_HAL_STATUS_OUT_OF_RANGE;
    }
#endif
    pMutex = &DSP->MUTEX0 + mutexId*MUTEX_OFFSET;

    *pState = (am_hal_ipc_mutex_state_e)*pMutex;
    return AM_HAL_STATUS_SUCCESS;
}

//*****************************************************************************
//
// called when a queue is destroyed by the peer
//
//*****************************************************************************
static void
am_hal_ipc_shortmsg_termination_event(uint32_t fromCore)
{
    // based on the descriptor values in shared memory, this function is called.
    sHalSharedMem->sIpcSharedState.ipcInfo[am_hal_get_core_id()][fromCore].pWriteDescr = NULL;
    sHalSharedMem->sIpcSharedState.ipcInfo[am_hal_get_core_id()][fromCore].pReadDescr  = NULL;
    memset( &sIpcHal.sIpcRxQueue[fromCore], 0, sizeof(sIpcHal.sIpcRxQueue[fromCore]));
    memset( &sIpcHal.sIpcTxQueue[fromCore], 0, sizeof(sIpcHal.sIpcTxQueue[fromCore]));
}

//*****************************************************************************
//
// the management init event service
//
//*****************************************************************************

static void
am_hal_ipc_shortmsg_init_event(uint32_t fromCore)
{
    // based on the descriptor values in shared memory, this function is called.
    am_hal_ipc_update_queue_from_shared_mem(fromCore);
}

//*****************************************************************************
//
// the management event service
//
//*****************************************************************************

void
am_hal_ipc_shortmsg_init_service(void * par)
{
    uint32_t fromCore = (uint32_t)par;
    // note: It is important that init event is followed by termination event before
    // a new init event happens. Because only the termination event clears all descriptors to zero
    // and init event only update the descriptor from the remote side.
    if (sHalSharedMem->sIpcSharedState.ipcInfo[fromCore][am_hal_get_core_id()].pReadDescr == NULL)
    {
        // when the queue does not exists, the termination process should shutdown the link
        am_hal_ipc_shortmsg_termination_event(fromCore);
    }
    else
    {
        am_hal_ipc_shortmsg_init_event(fromCore);
    }
    // Clear the incoming event
    am_hal_ipc_int_clear(SHORTMSG_MANAGEQUEUE_EVENT_NO( fromCore));

}

//*****************************************************************************
//
// terminate the local queues and send a management event to the other side
//
//*****************************************************************************
uint32_t
am_hal_ipc_term(void)
{
    int core;
    // The event handler is cleared so no more event is accepted.
    memset( (void*)&sIpcHal.sIpcEvenrHandler, 0, sizeof(sIpcHal.sIpcEvenrHandler));
    // clear event mask for the current core
    // update shared memory with new event Mask(clear shared mem)
    // Send queue management event to the other active cores
    sHalSharedMem->sIpcSharedState.eventMask[am_hal_get_core_id()] = sIpcHal.eventMask[am_hal_get_core_id()] = 0;

    for ( core = 0; core < AM_HAL_NUM_CORES; core++ )
    {
        //clear the descriptor inside the shared mem to zero.
        sHalSharedMem->sIpcSharedState.ipcInfo[am_hal_get_core_id()][core].pWriteDescr = NULL;
        sHalSharedMem->sIpcSharedState.ipcInfo[am_hal_get_core_id()][core].pReadDescr  = NULL;
        memset( &sIpcHal.sIpcRxQueue[core], 0, sizeof(sIpcHal.sIpcRxQueue[core]));
        memset( &sIpcHal.sIpcTxQueue[core], 0, sizeof(sIpcHal.sIpcTxQueue[core]));
        // send queue management event to the other core.
        // When the core== am_hal_get_core_id(), then am_hal_ipc_event_send will not send any event
        am_hal_ipc_event_send( core, SHORTMSG_MANAGEQUEUE_EVENT_NO(core) );
    }
    return AM_HAL_STATUS_SUCCESS;
}

//*****************************************************************************
//
// Map the memory - for IPC messages
//
//*****************************************************************************
uint32_t
am_hal_ipc_mmap(uint32_t addr, uint8_t fromCore, uint8_t toCore, uint32_t *pMappedAddr)
{
    // TODO
    // Mem-Map required only for DSP TCM
    // DSP2 internal memory has a secondary address(SECONDARY_MEM_ADD) from other cores view.
    // Also DSP2 sees the TCM of DSP1 at the secondary address
    // From DSP1 to DSP2 TCM( SECONDARY_MEM_ADD )
    // FROM DSP2 to DSP1 TCM( SECONDARY_MEM_ADD )
    // FROM ARM to DSP2 TCM( SECONDARY_MEM_ADD )
    //
    if ( ( ( fromCore == DSP2_CORE_ID ) && ( toCore == DSP1_CORE_ID ) ) ||
         ( ( fromCore == DSP1_CORE_ID ) && ( toCore == DSP2_CORE_ID ) ) ||
         ( ( fromCore == ARM_CORE_ID  ) && ( toCore == DSP2_CORE_ID ) )  )
    {
        // if addr is in range of internal memory, change the address.
        *pMappedAddr = CONVERT_SECONDARY_TO_PRIMARY_ADDRESS(addr);
    }
    else
    {
        *pMappedAddr = addr;
    }
    return AM_HAL_STATUS_SUCCESS;
}

//*****************************************************************************
//
// Register a function for an event and optionally send a management event to other cores
//
//*****************************************************************************
static uint32_t
am_hal_internal_ipc_event_register(uint32_t event, am_hal_ipc_event_callback_t pHandler, void *pCallbackCtxt, bool bNotify)
{
    // Populate the lookup table
    if ( event < MAX_NUM_IPC_EVENT_HANDLER )
    {
        // update local copy of the maskEvent
        sIpcHal.eventMask[am_hal_get_core_id()] |= (1 << event);
        // update shared memory copy of the maskEvent
        sHalSharedMem->sIpcSharedState.eventMask[am_hal_get_core_id()] = sIpcHal.eventMask[am_hal_get_core_id()];

        sIpcHal.sIpcEvenrHandler[event].pHandler = pHandler;
        sIpcHal.sIpcEvenrHandler[event].pCallbackCtxt = pCallbackCtxt;
        // todo: populate the shared memory with the new event to inform the other cores that the event is valid.
        // Jayesh: Is it just a valid bit? if it goes into shared memory, the sender always should check this bit before sending.
        // update shared memory bitmask. Send the notify event to all the other cores() if bNotify
        if ( am_hal_get_core_id() !=  ARM_CORE_ID )
        {
            // inform ARM of the new changes
            am_hal_ipc_event_send( ARM_CORE_ID, SHORTMSG_MANAGEQUEUE_EVENT_NO( ARM_CORE_ID) );
        }
        if ( am_hal_get_core_id() !=  DSP1_CORE_ID )
        {
            // inform DSP1 of the new changes
            am_hal_ipc_event_send( DSP1_CORE_ID, SHORTMSG_MANAGEQUEUE_EVENT_NO( DSP1_CORE_ID)  );
        }
        if ( am_hal_get_core_id() !=  DSP2_CORE_ID )
        {
            // inform DSP2 of the new changes
            am_hal_ipc_event_send( DSP2_CORE_ID, SHORTMSG_MANAGEQUEUE_EVENT_NO( DSP2_CORE_ID) );
        }
        return AM_HAL_STATUS_SUCCESS;
    }
    return AM_HAL_STATUS_INVALID_OPERATION;
}

//*****************************************************************************
//
// Register a function for an event and send a management event to other cores
//
//*****************************************************************************
uint32_t
am_hal_ipc_event_register(uint32_t event, am_hal_ipc_event_callback_t pHandler, void *pCallbackCtxt)
{
    return am_hal_internal_ipc_event_register(event, pHandler, pCallbackCtxt, true);
}

//*****************************************************************************
//
// Register a function for an event and send a management event to other cores
//
//*****************************************************************************

uint32_t *
am_hal_mail_box_address(uint8_t core)
{
    //todo: Removed and need to be added after register definition
/*    if ( core == ARM_CORE_ID )
    {
        return &DSP->CPUMBINTA;
    }
    if ( core == DSP1_CORE_ID )
    {
        return &DSP->DSP0MBINTA;
    }
    if ( core == DSP2_CORE_ID )
    {
        return &DSP->DSP1MBINTA;
    }
 */
    return NULL;
}

//*****************************************************************************
//
// send an event to other cores
//
//*****************************************************************************
uint32_t
am_hal_ipc_event_send(uint8_t toCore, uint32_t event)
{
    // Set the corresponding event in the IPC register
    uint32_t *mbAdd = am_hal_mail_box_address(toCore);
    if ( sIpcHal.eventMask[toCore] & (1 << event) )
    {
        // if a valid event, send it.
        if ( event<16)
        {
            mbAdd[MBINTA_INDEX] = (MBINT_SET) << (event*2);
//            return AM_HAL_STATUS_SUCCESS;
        }
        else if ( event<32)
        {
            mbAdd[MBINTB_INDEX] = (MBINT_SET) << (event*2);
//            return AM_HAL_STATUS_SUCCESS;
        }

#ifndef AM_HAL_DISABLE_API_VALIDATION
        else
        {
            return AM_HAL_STATUS_INVALID_OPERATION;
        }
#endif
    }
    // TODO: do we need an else here?!
    // no event is sent
    return AM_HAL_STATUS_SUCCESS;
}

//*****************************************************************************
//
// Return the event status bit.
//
//*****************************************************************************

uint32_t
am_hal_ipc_int_status_get(uint32_t *pEventMask)
{
    // TODO
    // Scan the bitmap, and call respective handlers registered
    uint32_t *mbAdd = am_hal_mail_box_address( am_hal_get_core_id());
    *pEventMask &= mbAdd[MBINTSTAT_INDEX];

    return AM_HAL_STATUS_SUCCESS;
}

//*****************************************************************************
//
// Clear the event status bit.
//
//*****************************************************************************
uint32_t
am_hal_ipc_int_clear(uint32_t eventMask)
{
    uint32_t *mbAdd = am_hal_mail_box_address(am_hal_get_core_id());
    int8_t i;
    while (eventMask !=0)
    {
        NEXT_LEADING_ZEROS(i, eventMask);
        if ( i<16)
        {
            mbAdd[MBINTA_INDEX] = (MBINT_SET) << ((15-i)*2);
        }
        else if ( i<32)
        {
            mbAdd[MBINTB_INDEX] = (MBINT_SET) << ((31-i)*2);
        }
        else if ( i == NSAU_RETURNS_ALL_BITS_ZERO )
        {
            break;
        }
        // TODO: do we need an else here?!
    }
    return AM_HAL_STATUS_SUCCESS;
}

//*****************************************************************************
//
// call registered handler for each event
//
//*****************************************************************************
void
am_hal_ipc_int_service(uint32_t eventMask)
{
    // TODO
    // Scan the bitmap, and call respective handlers registered
    uint32_t i;
    while (eventMask !=0)
    {
        NEXT_LEADING_ZEROS(i, eventMask);

        if ( i < MAX_NUM_IPC_EVENT_HANDLER )
        {
            sIpcHal.sIpcEvenrHandler[31-i].pHandler(sIpcHal.sIpcEvenrHandler[31-i].pCallbackCtxt);
        }
    }
    return;
}

//*****************************************************************************
//
// send a 32bit message to the other core
//
//*****************************************************************************
uint32_t
am_hal_ipc_shortmsg_send(uint8_t toCore, uint32_t ui32Msg)
{
#ifndef AM_HAL_DISABLE_API_VALIDATION
    if (toCore >= AM_HAL_NUM_CORES)
    {
        return AM_HAL_STATUS_OUT_OF_RANGE;
    }
    if ( sIpcHal.sIpcTxQueue[toCore].ui32QueueSize  == 0 )
    {
        // the queue is not initialized yet
        return AM_HAL_STATUS_OUT_OF_RANGE;
    }
#endif
    //  the TXQueue is inside the peer(remote) core, and only the descriptor is inside the host core.
    // The host descriptor keeps a copy of the remote-buffer address (ui32QueueBuffer) and
    // remote-buffer size(ui32QueueSize) in the host core. The ui32WrIndex in the remote side is the exact
    // copy of the host side variable. The ui32RdIndex in the Host is the exact copy of the remote side.
    if ( IS_QUEUE_FULL(sIpcHal.sIpcTxQueue[toCore].ui32WrIndex, sIpcHal.sIpcTxQueue[toCore].ui32RdIndex, sIpcHal.sIpcTxQueue[toCore].ui32QueueSize ) )
    {
        // Queue is full
        return AM_HAL_STATUS_INVALID_OPERATION;
    }
    // update the remote queue buffer with the message.
    (sIpcHal.sIpcTxQueue[toCore].ui32QueueBuffer)[sIpcHal.sIpcTxQueue[toCore].ui32WrIndex++] = ui32Msg;
    // update the write pointer index
    if ( sIpcHal.sIpcTxQueue[toCore].ui32WrIndex >= sIpcHal.sIpcTxQueue[toCore].ui32QueueSize )
    {
        sIpcHal.sIpcTxQueue[toCore].ui32WrIndex = 0;
    }
    // update the peer write index.
    *sIpcHal.sIpcTxQueue[toCore].pPeerIndex = sIpcHal.sIpcTxQueue[toCore].ui32WrIndex;

    // flush the write
#if 0
    uint32_t *mbAdd = am_hal_mail_box_address(toCore);
#endif
    uint32_t event = SHORTMSG_QUEUE_MAILBOX_EVENT_FOR_ARM + am_hal_get_core_id();
    if ( sIpcHal.eventMask[ toCore ] & ( 1 << event ) )
    {
// #### INTERNAL BEGIN ####
#if 0  // Fixme.  Required for Apollo4?
        // if a valid event, send it.
        if ( event<16)
        {
            // as the event number is more than 26, this code never executed.
            am_hal_sysctrl_membarrier_write (&mbAdd[MBINTA_INDEX], (MBINT_SET) << (event*2));
        }
        else if ( event<32)
        {
            am_hal_sysctrl_membarrier_write (&mbAdd[MBINTB_INDEX], (MBINT_SET) << ((event-16)*2));
        }
#endif
// #### INTERNAL END ####
    }
    //removed  am_hal_ipc_event_send(toCore, SHORTMSG_QUEUE_MAILBOX_EVENT_FOR_ARM + am_hal_get_core_id() ) ;
    return AM_HAL_STATUS_SUCCESS;
}

//*****************************************************************************
//
// the event handler for any incoming queue message
//
//*****************************************************************************

void
am_hal_ipc_shortmsg_service(void * par)
{
    uint8_t fromCore = (uint32_t)par;
    // The remote core sent a msg queue to this core.
    sIpcHal.pMsgQHandler(fromCore, sIpcHal.pCtxt );
}

//*****************************************************************************
//
// the event handler for any incoming queue message
//
//*****************************************************************************
uint32_t
am_hal_ipc_shortmsg_get_only_msg(uint8_t fromCore, uint32_t *pui32Msg)
{
#ifndef AM_HAL_DISABLE_API_VALIDATION
    if (fromCore >= AM_HAL_NUM_CORES)
    {
        return AM_HAL_STATUS_OUT_OF_RANGE;
    }
    if ( sIpcHal.sIpcRxQueue[fromCore].ui32QueueSize  == 0 )
    {
        // the queue is not initialized yet
        return AM_HAL_STATUS_OUT_OF_RANGE;
    }
    if ( !IS_QUEUE_EMPTY(sIpcHal.sIpcRxQueue[fromCore].ui32WrIndex, sIpcHal.sIpcRxQueue[fromCore].ui32RdIndex) )
    {
        *pui32Msg = sIpcHal.sIpcRxQueue[fromCore].ui32QueueBuffer[ sIpcHal.sIpcRxQueue[fromCore].ui32RdIndex];
    }
#endif
    return AM_HAL_STATUS_SUCCESS;
}

//*****************************************************************************
//
// the event handler for any incoming queue message
//
//*****************************************************************************

uint32_t
am_hal_ipc_shortmsg_get(uint8_t fromCore, uint32_t *pui32Msg)
{
    // Circular Buffer Receive
#ifndef AM_HAL_DISABLE_API_VALIDATION
    if (fromCore >= AM_HAL_NUM_CORES)
    {
        return AM_HAL_STATUS_OUT_OF_RANGE;
    }
    if ( sIpcHal.sIpcRxQueue[fromCore].ui32QueueSize  == 0 )
    {
        // the queue is not initialized yet
        return AM_HAL_STATUS_OUT_OF_RANGE;
    }
#endif

    if ( !IS_QUEUE_EMPTY(sIpcHal.sIpcRxQueue[fromCore].ui32WrIndex, sIpcHal.sIpcRxQueue[fromCore].ui32RdIndex) )
    {
        // if Rx queue is not empty
        // Read the message from the Queue and increment the read pointer.
        *pui32Msg = sIpcHal.sIpcRxQueue[fromCore].ui32QueueBuffer [ sIpcHal.sIpcRxQueue[fromCore].ui32RdIndex++];
        if ( sIpcHal.sIpcRxQueue[fromCore].ui32RdIndex >= sIpcHal.sIpcRxQueue[fromCore].ui32QueueSize )
        {
            sIpcHal.sIpcRxQueue[fromCore].ui32RdIndex = 0;
        }
        // after updating the local read index then
        // update the remote read index (inside the core send the message )
        *sIpcHal.sIpcRxQueue[fromCore].pPeerIndex = sIpcHal.sIpcRxQueue[fromCore].ui32RdIndex;
        // todo: do we need to flush?
    }
    return AM_HAL_STATUS_INVALID_OPERATION;
}

//*****************************************************************************
//
// Get the size of the queue between two cores.
//
//*****************************************************************************
uint32_t
am_hal_ipc_shortmsgQ_get_state(uint8_t fromCore, uint8_t toCore, am_hal_ipc_shortmsgQ_state_t *pQState)
{
    uint32_t wrIndex, rdIndex, add;
    am_hal_shortMsgQ_descr_t *desc = sHalSharedMem->sIpcSharedState.ipcInfo[fromCore][toCore].pWriteDescr;
    if ( desc == NULL )
    {
        pQState->maxSize  = 0;
        pQState->numEntries = 0;
        return AM_HAL_IPC_MEM_NOT_INIT;
    }
    am_hal_ipc_mmap((uint32_t) desc, am_hal_get_core_id(), fromCore, &add);
    desc = (am_hal_shortMsgQ_descr_t *)add;
    wrIndex = desc->ui32WrIndex;
    rdIndex = desc->ui32RdIndex;
    pQState->maxSize    = desc->ui32QueueSize;
    pQState->numEntries = QUEUE_USED_WORDS (wrIndex,  rdIndex, pQState->maxSize );
    // Circular Buffer state
    return AM_HAL_STATUS_INVALID_OPERATION;
}

//*****************************************************************************
//
// check if the Rx Queue is empty
//
//*****************************************************************************
bool
am_hal_ipc_shortmsgQ_is_empty(uint8_t fromCore)
{
    // Circular Buffer state
    return IS_QUEUE_EMPTY(sIpcHal.sIpcRxQueue[fromCore].ui32WrIndex, sIpcHal.sIpcRxQueue[fromCore].ui32RdIndex);
}
//*****************************************************************************
//
// check if the Tx Queue is full
//
//*****************************************************************************
bool
am_hal_ipc_shortmsgQ_is_full(uint8_t toCore)
{
    // Circular Buffer state
    return IS_QUEUE_FULL(sIpcHal.sIpcTxQueue[toCore].ui32WrIndex, sIpcHal.sIpcTxQueue[toCore].ui32RdIndex, sIpcHal.sIpcTxQueue[toCore].ui32QueueSize );
}

//??????????????
uint32_t
am_hal_ipc_get_status(uint8_t core, am_hal_ipc_core_status_e *pCoreStatus)
{
    // TODO
    // Return the status of the core, as visible from here
    return AM_HAL_STATUS_INVALID_OPERATION;
}

//*****************************************************************************
//
// initialization of IPC context
//
//  The ipc init function is the first function( before any other IPC function)
//  which should be called. It initialize the IPC queue and events.
//
//*****************************************************************************
uint32_t
am_hal_ipc_init(am_hal_ipc_shortmsgQ_callback_t pMsgQHandler, void *pCtxt)
{
    // clear IPC structure to zero:
    // 1- Clear Queue's descriptors
    // 2- Clear event handler
    memset( &sIpcHal, 0, sizeof(am_hal_ipc_t) );
    sIpcHal.pMsgQHandler = pMsgQHandler;
    sIpcHal.pCtxt = pCtxt;
    // Initialize event bit masks to zero.
    sHalSharedMem->sIpcSharedState.eventMask[ am_hal_get_core_id() ] = 0;

    // Register internal handler for msgQ events (ISR registration left to application - like other ISRs)

    // Initialize the local copy of the queue inside TCM
    am_hal_ipc_local_queue_init(ARM_CORE_ID );
    am_hal_ipc_local_queue_init(DSP1_CORE_ID);
    am_hal_ipc_local_queue_init(DSP2_CORE_ID);

    // Also initialize the shared memory data with queue data
    am_hal_ipc_update_shared_mem_with_queue_data(ARM_CORE_ID);
    am_hal_ipc_update_shared_mem_with_queue_data(DSP1_CORE_ID);
    am_hal_ipc_update_shared_mem_with_queue_data(DSP2_CORE_ID);

    am_hal_ipc_update_queue_from_shared_mem(ARM_CORE_ID);
    am_hal_ipc_update_queue_from_shared_mem(DSP1_CORE_ID);
    am_hal_ipc_update_queue_from_shared_mem(DSP2_CORE_ID);

// clear the whole interrupt handler
// register the    queue construction and termination handler
    am_hal_internal_ipc_event_register( SHORTMSG_MANAGEQUEUE_EVENT_NO( ARM_CORE_ID), am_hal_ipc_shortmsg_init_service, (void *)ARM_CORE_ID, false);
    am_hal_internal_ipc_event_register( SHORTMSG_MANAGEQUEUE_EVENT_NO(DSP1_CORE_ID), am_hal_ipc_shortmsg_init_service, (void *)DSP1_CORE_ID, false);
    am_hal_internal_ipc_event_register( SHORTMSG_MANAGEQUEUE_EVENT_NO(DSP2_CORE_ID), am_hal_ipc_shortmsg_init_service, (void *)DSP2_CORE_ID, false);

// register the receive queue event
    am_hal_internal_ipc_event_register( SHORTMSG_QUEUE_EVENT_NO(ARM_CORE_ID), am_hal_ipc_shortmsg_service, (void *)ARM_CORE_ID, false);
    am_hal_internal_ipc_event_register( SHORTMSG_QUEUE_EVENT_NO(DSP1_CORE_ID), am_hal_ipc_shortmsg_service, (void *)DSP1_CORE_ID, false);
    // send the queue management event to the other cores
    am_hal_internal_ipc_event_register( SHORTMSG_QUEUE_EVENT_NO(DSP2_CORE_ID), am_hal_ipc_shortmsg_service, (void *)DSP2_CORE_ID, true);

    return AM_HAL_STATUS_INVALID_OPERATION;
}

//*****************************************************************************
//
// End Doxygen group.
//!  @}
//
//*****************************************************************************
