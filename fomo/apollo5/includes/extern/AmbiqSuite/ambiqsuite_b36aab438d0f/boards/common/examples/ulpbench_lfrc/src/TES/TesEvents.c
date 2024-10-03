/*********************************************************
(C) 2014 EEMBC(R) and ULPBench(TM).  All rights reserved.                            

EEMBC ULPBench Software is a product of EEMBC and is provided 
under the terms of the ULPBench License that is distributed 
with the official EEMBC ULPBench Software release.  
The Software is the proprietary intellectual property of EEMBC 
and its members and is protected under all applicable laws, 
including all applicable copyright laws.  
If you received this EEMBC ULPBench Software 
without the accompanying ULPBench License, 
you must discontinue use and download the official release 
from http://www.eembc.org/benchmark/ulp_sl.php.  
**********************************************************/


// TesEvents.c    mbutler    Jan 2013
//
// TES Event-related data and functions
//=============================================================================

//=============================================================================
#include "TesTPI.h"

//=============================================================================
// Event global data structures

TesQueueHeader tesEventReadyQueue;

//=============================================================================
// Verify Event is free, init Event values
// If thread == 0, then use current tesActiveThread
ulp_u1 tesEventPrepare( TesEvent * event, ulp_u1 type, ulp_u1 param, TesThread * thread )
{
    if (event->next != TES_LIST_FREE) {
        return TES_ERROR_EVENT_ON_LIST;
    }

    event->type = type;
    event->param = param;
    event->thread = (thread) ? thread : tesActiveThread;

    return TES_ERROR_OK;
}

//=============================================================================
// Dispatch queued ready Events to owning Thread.
// Sleep when no events are queued.
void tesEventDispatchReady( void )
{
    for (;;) {
        if (tesEventReadyQueue.first != TES_LIST_END) {
            TesEvent * event = (TesEvent*)tesEventReadyQueue.first;
            tesThreadRun(event->thread);
            return;
        }
        pltSleep();
    }
}

//=============================================================================
// Removes first ready event and returns it
TesEvent * tesEventRemoveReady( void )
{
    TesEvent * event = (TesEvent*)tesEventReadyQueue.first;

    PLT_INTERRUPTS_OFF;     // synchronize with interrupt handlers
    if ((event->next) != TES_LIST_END) {
        tesEventReadyQueue.first = (TesListElement*)event->next;
    } else {
        tesQueueSetEmpty(&tesEventReadyQueue);
    }
    PLT_INTERRUPTS_ON;

    event->next = TES_LIST_FREE;
    return event;
}

//=============================================================================
// Threads call this to wait for the next event
TesEvent * tesEventWait( void )
{
    tesEventDispatchReady();
    // returns here only when there is an event for this thread
    return tesEventRemoveReady();
}

//=============================================================================
// Initialize all Event lists (to empty)
void tesEventQueuesInit(void)
{
    tesQueueSetEmpty(&tesEventReadyQueue);
}

//=============================================================================
// Put the ready Event on the tesEventReadyQueue
// NOTE: Caller must ensure PLT_INTERRUPTS_OFF !
// Usually only called from interrupt handler environment
void tesEventToReady( TesEvent * event )
{
    tesQueueAppend(&tesEventReadyQueue, (TesListElement*)event);
}

//=============================================================================
// Move an entire list of ready Events to the tesEventReadyQueue
// NOTE: Caller must ensure PLT_INTERRUPTS_OFF !
// Should only be called from interrupt handler environment
void tesEventQueueToReady( TesQueueHeader * queueHeader )
{
    if (tesEventReadyQueue.first == TES_LIST_END) {
        tesEventReadyQueue.first = queueHeader->first;
        tesEventReadyQueue.last = queueHeader->last;
    } else {
        TesEvent * last = (TesEvent*)tesEventReadyQueue.last;
        last->next = (TesEvent*)queueHeader->first;
        tesEventReadyQueue.last = queueHeader->last;
    }
    tesQueueSetEmpty(queueHeader);
}

//=============================================================================
// Returns true if there is an Event waiting on the tesEventReadyQueue
ulp_u1 tesEventIsReady( void )
{
    return (ulp_u1)(tesEventReadyQueue.first != TES_LIST_END);
}

//=============================================================================
// Initialize and Event to wake the specified Thread and put it on the tesEventReadyQueue
// If thread == 0, use the current tesActiveThread
ulp_u1 tesEventThreadRun( TesEvent * event, ulp_u1 param, TesThread * thread )
{
    ulp_u1 error = tesEventPrepare(event, TES_THREAD_START, param, thread);
    if (!error) {
        PLT_INTERRUPTS_OFF;
        tesEventToReady(event);
        PLT_INTERRUPTS_ON;
    }
    return error;
}
