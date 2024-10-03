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

RESTORE_ON_WAKEUP static volatile unsigned int next_state = 0;   

/* Function: state_machine
	Simple state machine

	Parameters:
	
	new_state	-	input value
	
	Returns:
	
	A new state based on the current state and the input value.
	
	Description:
	
	Implement a simple state machine using a switch statement.
	
*/
unsigned int state_machine(unsigned int new_state)
{
  unsigned int state = 0;
   
   /* do some state machine processing */
   /* we have a state machine of 8 states (3 Bits) and wake through different states depending of the new and old state */
   state = next_state ^ (new_state & 0x01);
                              
   switch (state & 0x07){
     case 0:
       next_state = 1;
       break;
     case 1:
       next_state = 6;
       break;
     case 2:
       next_state = 3;
       break;
     case 3:
       next_state = 4;
       break;
     case 4:
       next_state = 5;
       break;
     case 5:
       next_state = 0;
       break;
     case 6:
       next_state = 7;
       break;
     case 7:
       next_state = 2;
       break;   
     default:
       next_state = 0;
       break;
   }
   return (next_state);
}

void reset_state(void) {
	next_state=0;
}
