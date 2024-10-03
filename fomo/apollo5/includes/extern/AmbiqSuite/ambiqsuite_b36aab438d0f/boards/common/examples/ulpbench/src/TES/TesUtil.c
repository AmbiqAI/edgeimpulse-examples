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


// TesUtil.c    mbutler    Jan 2014
//
// TES utility functions
//=============================================================================
#include "Platform.h"

/* Function: tesBytesCopy
    Copy n bytes from src to dest.
      dest  -   Destination array
      src   -   Source array
      len   -   Number of bytes to copy
    Returns:
      dest
*/

void * tesBytesCopy(void * RESTRICT dest, const void * RESTRICT src, ulp_u2 len) {
     ulp_u2 i;
     ulp_u1 * RESTRICT d = dest;
     const ulp_u1 * RESTRICT s = src;
     for (i = 0; i < len; i++) {
         d[i] = s[i];
     }
     return d;
}

/* Function: tesStringSearch
    Search for a pattern of bytes in a string
      hay       - String to search in
      needle    - String pattern to search for
    Returns:
      Null if not found, otherwise pointer to the first byte of the pattern in hay
*/
const ulp_u1 *tesStringSearch(const ulp_u1 *hay, const ulp_u1 *needle) {
    if (*needle == 0) {
        /* special case: empty pattern string */
        return (ulp_u1 *)hay;
    }
    while (*hay) {
        ulp_u1 i = 0;
        while (1) {
            if (needle[i] == 0) {
                return (ulp_u1 *)hay;
            }
            if (hay[i] != needle[i]) {
                break;
            }
            i++;
        }
        hay++;
    }
    return 0;   // not found
}

/* Function: tesStringLength
    Count bytes until a 0 byte is encountered.
      s - string
    Returns:
      Number of bytes in the array until terminating 0 byte.
*/
ulp_u2 tesStringLength(const ulp_u1 *s) {
     const ulp_u1 *start = s;
     while (*s) {
         s++;
     }
     return (ulp_u2)(s - start);
}
