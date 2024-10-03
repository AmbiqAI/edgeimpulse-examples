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
// BenchmarkConfig.h    mbutler    Jan 2014
//
// Select a Benchmark Configuration details.
//=============================================================================

#ifndef _BENCHMARK_CONFIG_H_
#define _BENCHMARK_CONFIG_H_

//==============================================================================
// $$$ Select which benchmark to run
// $$$ Set BENCHMARK_NUMBER to: 1...N, or 900+ for demo benchmarks.
#ifndef BENCHMARK_NUMBER
#define BENCHMARK_NUMBER 1
#endif


#if (BENCHMARK_NUMBER == 1)
  #include "CoreProfileConfig.h"    // the CoreProfile benchmark

#elif (BENCHMARK_NUMBER == 900)
  #include "HelloWorldConfig.h"     // the HelloWorld demo benchmark

#else
  #error "Invalid BENCHMARK_NUMBER selected !"
#endif

// When a low power mode with Core Domain off is used, this function 
// can be called to directly go into the workload function after the wake up
extern void TES_PRIMARY_THREAD_MAIN(void);

#endif /* _BENCHMARK_CONFIG_H_ */
