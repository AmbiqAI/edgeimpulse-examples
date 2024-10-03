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


// TesTPI.h    mbutler    Jan 2014
//
// TES Programming Interface (for TES and Platform layers)
//=============================================================================

#ifndef _TES_TPI_H_
#define _TES_TPI_H_

//=============================================================================
#include "Tes.h"
#include "PlatformTPI.h"

//=============================================================================
extern TesThread * tesActiveThread;

//=============================================================================
// see .c files for descriptions

ulp_u1 tesEventPrepare( TesEvent * event, ulp_u1 type, ulp_u1 param, TesThread * thread );

void tesEventProcessReady( void );

void tesTimerQueuesInit(void);

void tesTimerInterrupt( void );

void tesEventToReady( TesEvent * event );

void tesEventQueueToReady( TesQueueHeader * queueHeader );

void tesThreadRun( TesThread * thread );

VoidFunctionPtr tesInitialize( void );

void tesEventQueuesInit(void);

TesEvent * tesEventRemoveReady( void );

#ifdef TES_MULTIPLE_THREAD_STARTS
void tesThreadMultipleStart( void );
#endif // TES_MULTIPLE_THREAD_STARTS

#endif /* _TES_TPI_H_ */
