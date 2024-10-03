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


// Tes.h    mbutler    Jan 2014
//
// TES declarations for Benchmarks.
//=============================================================================

#ifndef _TES_H_
#define _TES_H_

//=============================================================================
#include "BenchmarkConfig.h"
#include "Platform.h"

//=============================================================================
// Note: a maximum of 256 time slots
typedef ulp_u1 TesTimeSlot;

//=============================================================================
// struct for TesThread data
typedef struct TesThread {
    PltStackCell * sp;      // stack pointer
    PltStackCell * stack;   // stack base
    PltStackSize stackSize; // stack size
} TesThread;

//=============================================================================
// a List Element
typedef struct TesListElement {
    struct TesListElement * next;   // first field is pointer to next element
} TesListElement;

#define TES_LIST_FREE  ( (void*)0 )     // if element is not on any list
#define TES_LIST_END   ( (void*)1 )     // if element is last in the list

//=============================================================================
// a Queue Header
typedef volatile struct TesQueueHeader {
    TesListElement * first;             // ptr to first element in queue
    TesListElement * last;              // ptr to last element in queue
} TesQueueHeader;

//=============================================================================
// struct for TesEvent data
typedef struct TesEvent {
    struct TesEvent * next;     // pointer to next event in list
    ulp_u1 type;                // event type
    ulp_u1 param;               // user-supplied event parameter
    TesThread * thread;         // thread to awaken on this event
} TesEvent;

//=============================================================================
// Event type values
enum TES_EVENT_TYPES {
    TES_THREAD_START = 0,       // Special Thread (re)start Event
    TES_TIMER_SLOT = 1,         // Slot Timer Event
    TES_TIMER_PERIOD = 2        // Period Timer Event
};

//=============================================================================
// struct for TesPeriodTimer, just an Event with an extra data field
typedef struct TesPeriodTimer {
    TesEvent event;         // a subclass of Event
    ulp_u2 numPeriods;      // number of periods to wait
} TesPeriodTimer;

//=============================================================================
// typedefs for a function with no parameters and no return value
typedef void (VoidFunction)(void);

typedef void (*VoidFunctionPtr)(void);

//=============================================================================
enum TES_ERRORS {
    TES_ERROR_OK = 0,                   // no error
    TES_ERROR_YIELD = 1,                // an Event is/was waiting
    TES_ERROR_EVENT_ON_LIST = 2,        // event expected to be free
    TES_ERROR_EVENT_NOT_ON_LIST = 3,    // event expected to be on a list
    TES_ERROR_NO_TIME_SLOT = 4          // exceeded NUM_TIME_SLOTS
};

#define TES_NEXT_SLOT ( (TesTimeSlot)0xFF ) // special value for "next slot"


//=============================================================================
// API from TES, see .c files for descriptions

void * tesBytesCopy(void * RESTRICT dest, const void * RESTRICT src, ulp_u2 len);

ulp_u2 tesStringLength(const ulp_u1 *s);

const ulp_u1 *tesStringSearch(const ulp_u1 *hay, const ulp_u1 *needle);

void tesListInsertAfter( TesListElement * prev, TesListElement * element );

void tesQueueAppend( TesQueueHeader * queueHeader, TesListElement * element );

void tesQueueSetEmpty( TesQueueHeader * queueHeader );

ulp_u2 tesPeriodSlotGetNext( TesTimeSlot * nextSlot );

ulp_u1 tesSlotTimerSet(TesEvent * timer, ulp_u1 slot, ulp_u1 param);

ulp_u1 tesPeriodTimerSet(TesPeriodTimer * timer, ulp_u2 numPeriods, ulp_u1 param);

TesEvent * tesEventWait( void );

ulp_u1 tesEventIsReady( void );

ulp_u1 tesEventThreadRun( TesEvent * event, ulp_u1 param, TesThread * thread );

#ifdef TES_MULTIPLE_THREAD_STARTS
ulp_u1 tesThreadStart(TesThread * thread, PltStackCell * stack, PltStackSize stackSize, VoidFunctionPtr main, TesEvent * event);
#endif // TES_MULTIPLE_THREAD_STARTS

#endif /* _TES_H_ */
