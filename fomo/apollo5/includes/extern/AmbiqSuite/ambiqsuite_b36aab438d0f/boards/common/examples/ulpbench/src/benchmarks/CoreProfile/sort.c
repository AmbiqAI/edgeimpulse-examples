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

#include "CoreProfile.h"
/* ============================================================================ */
/* Function: bubblesort
	Simple sort for 8b data

	Parameters:
	
	ptr			- Array to be sorted
	n 			- Size to sort
	resultindex	- Index of value to return
	
	Returns:
	
	Value at resultindex after sorting n bytes from the array
	
	Description:
	
	Implements a simple bubble sort for 8b data.
	
*/
ulp_u1 bubblesort(ulp_u1 * ptr, int n, unsigned int resultindex)
{
   unsigned int i, j;
   ulp_u1 temp;

   for (i = 1; i <= n-1; ++i)
      for ( j = 0; j < n-i; ++j)
         if ((ptr[j]!=0) && (ptr[j] > ptr[j+1]))
         {
             temp = ptr[j];
             ptr[j]   = ptr[j+1];
             ptr[j+1] = temp;
         }

   return ptr[resultindex];
}
