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
/* LCD defines and functions                                                    */
/* ============================================================================ */

#define SEG_a   0x01
#define SEG_b   0x02
#define SEG_c   0x04
#define SEG_d   0x08
#define SEG_e   0x10
#define SEG_f   0x20
#define SEG_g   0x40
#define SEG_h   0x80

static const ulp_u1 lcd_7segment_ascii_table[] =
{
    0,
    0,            /* exclamation */
    (ulp_u1) (SEG_a|SEG_f),
    0,            /* hash */
    (ulp_u1) (SEG_a|SEG_c|SEG_d|SEG_f|SEG_g),
    0,            /* percent */
    0,            /* ampersand */
    (ulp_u1) (SEG_a),
    (ulp_u1) (SEG_a|SEG_d|SEG_e|SEG_f),
    (ulp_u1) (SEG_a|SEG_b|SEG_c|SEG_d),
    0,            /* asterisk */
    0,            /* plus */
    0,            /* comma */
    (ulp_u1) (SEG_g),
    0,            /* full stop */
    (ulp_u1) (SEG_b|SEG_c),
    (ulp_u1) (SEG_a|SEG_b|SEG_c|SEG_d|SEG_e|SEG_f),
    (ulp_u1) (SEG_b|SEG_c),
    (ulp_u1) (SEG_a|SEG_b|SEG_d|SEG_e|SEG_g),
    (ulp_u1) (SEG_a|SEG_b|SEG_c|SEG_d|SEG_g),
    (ulp_u1) (SEG_b|SEG_c|SEG_f|SEG_g),
    (ulp_u1) (SEG_a|SEG_c|SEG_d|SEG_f|SEG_g),
    (ulp_u1) (SEG_a|SEG_c|SEG_d|SEG_e|SEG_f|SEG_g),
    (ulp_u1) (SEG_a|SEG_b|SEG_c),
    (ulp_u1) (SEG_a|SEG_b|SEG_c|SEG_d|SEG_e|SEG_f|SEG_g),
    (ulp_u1) (SEG_a|SEG_b|SEG_c|SEG_d|SEG_f|SEG_g),
    0,            /* colon */
    0,            /* semi-colon */
    0,            /* less than */
    (ulp_u1) (SEG_d|SEG_g),
    0,            /* greater than */
    (ulp_u1) (SEG_a|SEG_b|SEG_e|SEG_g),
    0,            /* at sign */
    (ulp_u1) (SEG_a|SEG_b|SEG_c|SEG_e|SEG_f|SEG_g),
    (ulp_u1) (SEG_c|SEG_d|SEG_e|SEG_f|SEG_g),
    (ulp_u1) (SEG_a|SEG_d|SEG_e|SEG_f),
    (ulp_u1) (SEG_b|SEG_c|SEG_d|SEG_e|SEG_g),
    (ulp_u1) (SEG_a|SEG_d|SEG_e|SEG_f|SEG_g),
    (ulp_u1) (SEG_a|SEG_e|SEG_f|SEG_g),
    (ulp_u1) (SEG_a|SEG_c|SEG_d|SEG_e|SEG_f),
    (ulp_u1) (SEG_b|SEG_c|SEG_e|SEG_f|SEG_g),
    (ulp_u1) (SEG_b|SEG_c),
    (ulp_u1) (SEG_a|SEG_b|SEG_c|SEG_d|SEG_e),
    (ulp_u1) (SEG_c|SEG_d|SEG_e|SEG_f|SEG_g),
    (ulp_u1) (SEG_d|SEG_e|SEG_f),
    (ulp_u1) (SEG_b|SEG_c|SEG_d|SEG_e|SEG_f),
    (ulp_u1) (SEG_b|SEG_c|SEG_d|SEG_e|SEG_f),
    (ulp_u1) (SEG_a|SEG_b|SEG_c|SEG_d|SEG_e|SEG_f),
    (ulp_u1) (SEG_a|SEG_b|SEG_e|SEG_f|SEG_g),
    (ulp_u1) (SEG_a|SEG_b|SEG_c|SEG_f|SEG_g),
    (ulp_u1) (SEG_a|SEG_b|SEG_c|SEG_e|SEG_f|SEG_g),
    (ulp_u1) (SEG_a|SEG_c|SEG_d|SEG_f|SEG_g),
    (ulp_u1) (SEG_d|SEG_e|SEG_f|SEG_g),
    (ulp_u1) (SEG_c|SEG_d|SEG_e),
    (ulp_u1) (SEG_b|SEG_c|SEG_d|SEG_e|SEG_f),
    (ulp_u1) (SEG_b|SEG_c|SEG_d|SEG_e|SEG_f),
    (ulp_u1) (SEG_b|SEG_c|SEG_e|SEG_f|SEG_g),
    (ulp_u1) (SEG_b|SEG_c|SEG_d|SEG_f|SEG_g),
    (ulp_u1) (SEG_a|SEG_b|SEG_d|SEG_e|SEG_g),
    (ulp_u1) (SEG_a|SEG_d|SEG_e|SEG_f),
    (ulp_u1) (SEG_e|SEG_f),
    (ulp_u1) (SEG_a|SEG_b|SEG_c|SEG_d),
    (ulp_u1) (SEG_a),
    (ulp_u1) (SEG_d),
    (ulp_u1) (SEG_b),
    (ulp_u1) (SEG_a|SEG_b|SEG_c|SEG_d|SEG_e|SEG_g),
    (ulp_u1) (SEG_c|SEG_d|SEG_e|SEG_f|SEG_g),
    (ulp_u1) (SEG_d|SEG_e|SEG_g),
    (ulp_u1) (SEG_b|SEG_c|SEG_d|SEG_e|SEG_g),
    (ulp_u1) (SEG_a|SEG_b|SEG_d|SEG_e|SEG_f|SEG_g),
    (ulp_u1) (SEG_a|SEG_e|SEG_f|SEG_g),
    (ulp_u1) (SEG_a|SEG_b|SEG_c|SEG_d|SEG_f|SEG_g),
    (ulp_u1) (SEG_c|SEG_e|SEG_f|SEG_g),
    (ulp_u1) (SEG_c),
    (ulp_u1) (SEG_a|SEG_b|SEG_c|SEG_d|SEG_e),
    (ulp_u1) (SEG_c|SEG_d|SEG_e|SEG_f|SEG_g),
    (ulp_u1) (SEG_b|SEG_c),
    (ulp_u1) (SEG_c|SEG_e|SEG_g),
    (ulp_u1) (SEG_c|SEG_e|SEG_g),
    (ulp_u1) (SEG_c|SEG_d|SEG_e|SEG_g),
    (ulp_u1) (SEG_a|SEG_b|SEG_e|SEG_f|SEG_g),
    (ulp_u1) (SEG_a|SEG_b|SEG_c|SEG_f|SEG_g),
    (ulp_u1) (SEG_e|SEG_g),
    (ulp_u1) (SEG_a|SEG_c|SEG_d|SEG_f|SEG_g),
    (ulp_u1) (SEG_d|SEG_e|SEG_f|SEG_g),
    (ulp_u1) (SEG_c|SEG_d|SEG_e),
    (ulp_u1) (SEG_c|SEG_d|SEG_e),
    (ulp_u1) (SEG_c|SEG_d|SEG_e),
    (ulp_u1) (SEG_b|SEG_c|SEG_e|SEG_f|SEG_g),
    (ulp_u1) (SEG_b|SEG_c|SEG_d|SEG_f|SEG_g),
    (ulp_u1) (SEG_a|SEG_b|SEG_d|SEG_e|SEG_g),
    (ulp_u1) (SEG_a|SEG_d|SEG_e|SEG_f),
    (ulp_u1) (SEG_e|SEG_f),
    (ulp_u1) (SEG_a|SEG_b|SEG_c|SEG_d),
    0,
    0 
};
/* ============================================================================ */
/* Function: bin2LCD
	Write values to an array representing a 32b unsigned values on an LCD display.

	Parameters:
	
	lcd		- Array that will hold the output.
	size	- Number of array bytes to write.
	bin		- Value to represent.
	
	Returns:
	
	Nothing. The lcd array holds the output.
	
	Description:
	
	This function will write values to the buffer that will represent LCD characters.
	The values written are based on the value of the bin parameter, and actual value to write
	is picked from the lcd_7segment_ascii_table.
	
*/
void bin2LCD(ulp_u1 *lcd, unsigned int size, ulp_u4 bin)
{
    unsigned int i;
    
    for(i=0;i<size;i++){
      lcd[i] = lcd_7segment_ascii_table[(bin & 0x7F) + 0x20];
      bin = (bin>>4);
    }
}
