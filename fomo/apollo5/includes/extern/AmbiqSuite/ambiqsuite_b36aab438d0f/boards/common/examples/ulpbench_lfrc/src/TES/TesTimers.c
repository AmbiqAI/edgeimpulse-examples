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


// TesTimers.c    mbutler    Jan 2013
//
// TES Timer-related data and functions
//=============================================================================

//=============================================================================/*
#include "TesTPI.h"

//=============================================================================
// Timer global data structures

TesQueueHeader tesSlotTimerQueues[TES_NUM_TIME_SLOTS]; // Events waiting on each time slot

TesPeriodTimer * tesPeriodTimerList;    // Events to awake when numPeriods = 0

ulp_u2 tesNextPeriod = 0;               // next Period value, simple up counter

TesTimeSlot tesNextSlot = 0;            // next TimeSlot value, up counter

//=============================================================================
// Initialize all Timer lists to empty
void tesTimerQueuesInit(void)
{
    ulp_u1 i;
    for (i = 0; i < TES_NUM_TIME_SLOTS; i++) {
        tesQueueSetEmpty(&tesSlotTimerQueues[i]);
    }
    tesPeriodTimerList = (TesPeriodTimer*)TES_LIST_END;
}

//=============================================================================
// Add a SlotTimer to the list
ulp_u1 tesSlotTimerSet(TesEvent * timer, TesTimeSlot slot, ulp_u1 param)
{
    ulp_u1 error;

    if (slot == TES_NEXT_SLOT) {
        slot = tesNextSlot;
    } else if (slot >= TES_NUM_TIME_SLOTS) {
        return TES_ERROR_NO_TIME_SLOT;  // no such time slot
    }
    error = tesEventPrepare(timer, TES_TIMER_SLOT, param, 0/*thread*/);
    if (!error) {
        tesQueueAppend(&tesSlotTimerQueues[slot], (TesListElement*)timer);
    }
    return error;
}

//=============================================================================
// Add a PeriodTimer to the list (sorted in order of wait time)
ulp_u1 tesPeriodTimerSet(TesPeriodTimer * timer, ulp_u2 numPeriods, ulp_u1 param)
{
    ulp_u1 error = tesEventPrepare(&timer->event, TES_TIMER_PERIOD, param, 0/*thread*/);

    if (!error) {
        // find prev Timer by numPeriods to wait
        TesPeriodTimer * prev;

        PLT_INTERRUPTS_OFF;     // synchronize with interrupt handlers
        prev = (TesPeriodTimer*)&tesPeriodTimerList;
        for (;;) {
            TesPeriodTimer * next = (TesPeriodTimer*)prev->event.next;
            if (next == (TesPeriodTimer*)TES_LIST_END) {
                break;      // insert here (after element at prev)
            }
            if (next->numPeriods > numPeriods) {
                next->numPeriods -= numPeriods; // decrease next timer's wait time
                break;      // insert here (after element at prev)
            }
            numPeriods -= next->numPeriods; // decrease by previous wait time
            prev = next;    // move to next item
        }

        // insert timer after prev with adjusted numPeriods
        timer->numPeriods = numPeriods;
        tesListInsertAfter((TesListElement*)prev, (TesListElement*)&timer->event);
        PLT_INTERRUPTS_ON;
    }

    return error;
}

//=============================================================================
// A Timer Interrupt has occurred, update systemTime and check for Timers
// Should only be called from interrupt handler environment
void tesTimerInterrupt( void )
{
    TesQueueHeader * queueHeader;

    if (tesNextSlot == 0) {
        // all periodTimers with numPeriods = 0 are ready
        for (;;) {
            TesPeriodTimer * timer;
            ulp_u2 numPeriods;
            if ( (timer = tesPeriodTimerList) == (TesPeriodTimer*)TES_LIST_END) {
                 break;     // no entries on PeriodTimer list
            }
            if ( (numPeriods = timer->numPeriods) != 0) {
                timer->numPeriods = numPeriods - 1;   // decrement wait time
                break;      // next entry is not ready yet
            }
            // else this PeriodTimer is ready
            tesPeriodTimerList = (TesPeriodTimer*)timer->event.next;
            tesEventToReady(&timer->event);
        }
    }

    queueHeader = &tesSlotTimerQueues[tesNextSlot];
    if (queueHeader->first != TES_LIST_END) {
        tesEventQueueToReady(queueHeader);      // all Timers for this slot are ready
    }

    // compute next slot and period
    if (++tesNextSlot >= TES_NUM_TIME_SLOTS) {
        tesNextSlot = 0;    // roll over slot number
        ++tesNextPeriod;
    }
}

//=============================================================================
// Get the values for the period and slot as of the next tick
ulp_u2 tesPeriodSlotGetNext( TesTimeSlot * slot )
{
    ulp_u2 period;

    PLT_INTERRUPTS_OFF;     // synchronize with interrupt handlers
    *slot = tesNextSlot;
    period = tesNextPeriod;
    PLT_INTERRUPTS_ON;

    return period;
}
