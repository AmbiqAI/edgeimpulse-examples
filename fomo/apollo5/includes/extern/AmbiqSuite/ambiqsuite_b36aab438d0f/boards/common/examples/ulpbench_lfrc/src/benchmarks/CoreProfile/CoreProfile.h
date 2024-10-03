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
// CoreProfile.h    sschauer    Jan 2014
//	ULPBench CoreProfile, version 1.1.X
//
// Header file for CoreProfile benchmark
//=============================================================================

#ifndef _CORE_PROFILE_H_
#define _CORE_PROFILE_H_

#include <stdlib.h>
#include "BenchmarkConfig.h"
#include "Tes.h"

/* Special-purpose CoreProfile defines */
#ifndef DEBUG
#define DEBUG 0
#endif

#ifndef GEN_REF
#define GEN_REF 0
#endif

#ifndef CALLGRIND_RUN
#define CALLGRIND_RUN 0
#endif

#ifndef DO_MICA
#define DO_MICA 0
#endif

/* Kernels and associated functions */
void permute(ulp_u1 *out,ulp_u1 *in,ulp_u1 (*func)(ulp_u1),ulp_u1 permutes,ulp_u1 size_mask);

const ulp_u1 *ulp_8bstr(const ulp_u1 *hay, const ulp_u1 *needle);

void bin2LCD(ulp_u1 *lcd, unsigned int size, ulp_u4 bin);

ulp_u1 bubblesort(ulp_u1 * ptr, int n, unsigned int resultindex);

extern ulp_u1 ip(ulp_u1 i);

ulp_u2 math_int16(ulp_u2 x, ulp_u2 y, ulp_u2 offset, ulp_u2 gain);

ulp_u1 math_int8(ulp_u1 x, ulp_u1 y, ulp_u1 offset, ulp_u1 gain);

void math_int_init(void);

void reset_state(void);

unsigned int state_machine(unsigned int new_state);

#endif /* _CORE_PROFILE_H_ */
