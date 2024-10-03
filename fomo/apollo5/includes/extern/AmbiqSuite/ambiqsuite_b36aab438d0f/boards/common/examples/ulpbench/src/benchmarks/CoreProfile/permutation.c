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
   
/* Function: ip
	A permutation function

	Parameters:
	
	i - a value
	
	Returns:
	
	A value based on the i parameter.
	
	Description:
	
	The function modifies a value using simple 8b math.
	
*/
ulp_u1 ip(ulp_u1 i)
{
	return (ulp_u1)57 - (ulp_u1)((i & (ulp_u1)0x07) << (ulp_u1)(ulp_u1)3) + (ulp_u1)((i >> (ulp_u1)2) & (ulp_u1)0x06) - (ulp_u1)((i >> (ulp_u1)5) & (ulp_u1)0x01);
}


/* Function: getBit
	Returns the value of the bit 'index' in the bit table 'table'.

	Parameters:

	table		- Table containing values to use
	index		- Index of value to use and of bit to use from that value
	size_mask	- Mask to use on the index to avoid overflow
	
	Returns:
	
	An 8b value that may have 1 of the bits set depending on input.
	
	Description:
	
	This function is used to select a bit from a value in the input table parameter
	
*/
ulp_u1 getBit(ulp_u1 *table, ulp_u1 index, ulp_u1 size_mask)
{
	return (table[index & size_mask] & (0x80 >> (index & 0x07)));
}

/* Function: putBit
	Sets or clears the bit 'index' in the bit table 'table'
	
	Parameters:
	
	table		- Table containing values to modify
	index		- Index of value to modify and of bit to modify in that value
	value		- Value to decide if a bit should be set or unset
	size_mask	- Mask to use on the index to avoid overflow
	
	Returns:
	
	Nothing. A bit is set or unset in table based on the input values.
	
	Description:
	
	Set a particular bit. The value to modify in table is based on size_mask lower bits of the index.
	The bit to modify is based on the 3 lower bits of the index.
	The bit is set if the value parameter is not 0, unset otherwise.
	
*/
void putBit(ulp_u1 *table, ulp_u1 index, ulp_u1 value, ulp_u1 size_mask)
{
	unsigned char *p = &table[index & size_mask];
	unsigned char m = 0x80 >> (index & 0x07);

	*p = (value!=0) ? (m | *p) : (~m & *p);
}

/* Function: permute
	Changes bits in an output array based on an input array and a permutation function.

	Parameters:
	
	out 		- output array
	in 			- input array
	func		- permutation function to use
	permutes	- number of times to repeat the permutation
	size_mask	- mask for size of the input array allowable for permutes.
	
	Returns:
	
	Nothing. The out array is modified.
	
	Description:
	
	This function will modify bits in the output array based on bits permuted from the input array. 
	
*/
void permute(
ulp_u1 *out,
ulp_u1 *in,
ulp_u1 (*func)(ulp_u1),
ulp_u1 permutes,
ulp_u1 size_mask
)
{
	do
	{
		permutes--;
		putBit(out, permutes, getBit(in, func(permutes),size_mask),size_mask);
	}
	while (permutes);
}

