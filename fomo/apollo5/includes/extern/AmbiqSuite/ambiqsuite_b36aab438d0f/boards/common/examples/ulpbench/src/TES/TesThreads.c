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


// TesThreads.c    mbutler    Jan 2013
//
// TES Thread-related data and functions
//=============================================================================

//=============================================================================
#include "TesTPI.h"

//=============================================================================
// Thread global data structures

TesThread * tesActiveThread;            // the currently active Thread

TesThread tesPrimaryThread;             // data struct for primary Thread

VoidFunction TES_PRIMARY_THREAD_MAIN;   // declare the primary thread's main function

//=============================================================================
// If this configuration contains Multiple Threads to be started by TES, then
// set up pointers and a function to start them.

#ifdef TES_MULTIPLE_THREAD_STARTS

// declare the Thread start functions
VoidFunction TES_MULTIPLE_THREAD_STARTS;

// Zero-terminated array of Thread start functions
const VoidFunctionPtr tesMultipleThreadStarts[] = { TES_MULTIPLE_THREAD_STARTS, 0 };

// TES function to start multiple threads by calling their Start functions
void tesThreadMultipleStart( void )
{
    // call each Thread's "start" function
    const VoidFunctionPtr *threadStart = tesMultipleThreadStarts;

    while (*threadStart != 0) {
        (*threadStart)();   // call it
        threadStart++;      // next item
    }
}

//=============================================================================
// Call this to start a new Thread
// event is a temporary Event used only between the call to start and main.
// It should be a global variable, rather than a local (stack-based) variable.
ulp_u1 tesThreadStart(TesThread * thread, PltStackCell * stack, PltStackSize stackSize, VoidFunctionPtr main, TesEvent * event)
{
    pltThreadInit(thread, stack, stackSize, (void *) main);
    return tesEventThreadRun(event, 0/*param*/, thread);
}
#endif  // TES_MULTIPLE_THREAD_STARTS
//=============================================================================
// Error in TES task scheduler
void tesFatalError(void)
{
    // You can set break point here during software development
    for(;;);    // Fatal error, just spin !!
}
//=============================================================================
// Run a Thread. The triggering Event is at the front of the tesEventReadyQueue
void tesThreadRun( TesThread * thread )
{
    if (thread != tesActiveThread) {
#ifdef TES_MULTIPLE_THREAD_STARTS
        // if not the same as activeThread, then swap stacks
        TesThread * prevThread = tesActiveThread;
        tesActiveThread = thread;  // new active thread
        pltThreadSwapStacks(&thread->sp, &prevThread->sp);
#else
        tesFatalError();    // no multiple threads
#endif  // TES_MULTIPLE_THREAD_STARTS
    }
}

//=============================================================================
// Initialize TES for the Main and other Threads
VoidFunctionPtr tesInitialize( void ) {
    tesTimerQueuesInit();

    tesEventQueuesInit();

#ifdef TES_MULTIPLE_THREAD_STARTS
    // prepare to run multiple threads (if any)
    tesThreadMultipleStart();
#endif  // TES_MULTIPLE_THREAD_STARTS

    // initialize main thread
    tesActiveThread = &tesPrimaryThread;

    return TES_PRIMARY_THREAD_MAIN;    // return main function
}
