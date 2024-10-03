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
// CoreProfileConfig.h    mbutler    Jan 2014
//
// Configuration for CoreProfile benchmark
//=============================================================================

#ifndef _CORE_PROFILE_CONFIG_H_
#define _CORE_PROFILE_CONFIG_H_

//=============================================================================
// $$$ Define the name of this benchmark's Primary Thread main function.
#define TES_PRIMARY_THREAD_MAIN     CoreProfileMain

// $$$ Define the number of slots in each period for this benchmark.
#define TES_NUM_TIME_SLOTS          1

// $$$ If this benchmark has Multiple Threads, define their start functions.
// $$$ Else leave out this define.
// #define TES_MULTIPLE_THREAD_STARTS

#endif /* _CORE_PROFILE_CONFIG_H_ */
