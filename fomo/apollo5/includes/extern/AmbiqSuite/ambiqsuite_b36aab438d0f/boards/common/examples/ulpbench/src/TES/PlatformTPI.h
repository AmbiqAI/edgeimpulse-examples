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
//=============================================================================
// PlatformTPI.h    
//
// Platform-specific declarations for TES Programming Interface
//=============================================================================

#ifndef _PLATFORM_TPI_H_
#define _PLATFORM_TPI_H_

//=============================================================================
// Platform-specific includes

#include "Platform.h"

//=============================================================================
// Platform-specific defines

//=============================================================================
// Platform-specific functions, see .c files for descriptions

void pltSleep( void );

#ifdef TES_MULTIPLE_THREAD_STARTS
void pltThreadSwapStacks( PltStackCell ** addrNewSP, PltStackCell ** addrOldSP );

void pltThreadInit( TesThread * thread, PltStackCell * stack, PltStackSize stackSize, void * main );
#endif // #if TES_MULTIPLE_THREAD_STARTS


#endif /* _PLATFORM_TPI_H_ */

