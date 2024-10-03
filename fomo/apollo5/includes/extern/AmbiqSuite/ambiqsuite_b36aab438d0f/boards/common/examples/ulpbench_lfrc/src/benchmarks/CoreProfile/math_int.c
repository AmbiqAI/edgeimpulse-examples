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

#include <stdint.h>
#include "CoreProfile.h"

RESTORE_ON_WAKEUP static ulp_s4 integrator_sum = 0;
/* ============================================================================ */
/* Function: math_int16
	16b math function

	Parameters:
	
	x,y,offset,gain - input values to the calculation
	
	Returns:
	
	Approximation of a filter.
	
	Description:
	
	This function performs a simple filter using 16b math.
    A routine to filter away the DC content from an AC mains waveform
    signal. It does this using a heavily damped integrator to estimate
    the DC level. The current DC level is then subtracted from the signal. 
	
*/
ulp_u2 math_int16(ulp_u2 x, ulp_u2 y, ulp_u2 offset, ulp_u2 gain)
{
   ulp_u2 result;
   
   /* Linear Approximation and Scale */
   result = ((ulp_u2)(x - offset + y - offset)>>(ulp_u2)1) * gain;
   
#if DEBUG>3
		printf("math16.res1,%x\n",result);
#endif     
   
   integrator_sum += ((((int32_t) result << 16) - integrator_sum) >> 14);
#if DEBUG>3
		printf("math16.sum,%x\n",integrator_sum);
#endif     
   result -= (ulp_u2)(integrator_sum >> 16);
#if DEBUG>3
		printf("math16.res2,%x\n",result);
#endif     
   return (result);
   
}

/* ============================================================================ */
/* Function: math_int8

	8b math function

	Parameters:
	
	x,y,offset,gain - input values to the calculation
	
	Returns:
	
	Linear approximation.
	
	Description:
	
	This function performs a simple approximation using 8b math.
	
*/
ulp_u1 math_int8(ulp_u1 x, ulp_u1 y, ulp_u1 offset, ulp_u1 gain)
{
   ulp_u1 result;
   
   /* Linear Approximation and Scale */
   result = ((ulp_u1)(x - offset + y - offset)>>(ulp_u1)1) * gain;
#if DEBUG>3
		printf("math8.res,%x\n",result);
#endif     
   return (result);
  
}

 void math_int_init(void) {
	integrator_sum=0;
}
