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

#ifndef _PLATFORM_H_
#define _PLATFORM_H_

//=============================================================================
// Platform-specific includes, see note below.
#include <stdint.h>
#include <stdlib.h>
#include "board.h"

//=============================================================================
//{ Platform-specific configuration options
//=============================================================================
#ifdef TES_MULTIPLE_THREAD_STARTS

// For thread stack frame creation in pltThreadInit (Platform.c)
#define PLT_NUM_SAVE_REGS    10   // number of regs to save on switch (r4-r11, PC, CONTROL)

#endif /* TES_MULTIPLE_THREAD_STARTS */
//=============================================================================
//} End of Platform-specific configuration options
//=============================================================================

//=============================================================================
//{ Platform Types
//=============================================================================
typedef int8_t    ulp_s1;           // signed 1-byte value
typedef int16_t   ulp_s2;           // signed 2-byte value
typedef int32_t   ulp_s4;           // signed 4-byte value

typedef uint8_t   ulp_u1;           // unsigned 1-byte value
typedef uint16_t  ulp_u2;           // unsigned 2-byte value
typedef uint32_t  ulp_u4;           // unsigned 4-byte value

// $$$ What size is a stack "cell" on your Platform?
typedef ulp_u4 PltStackCell;      	// Example stack cell is 32 bits
typedef ulp_u4 PltStackSize;      	// Example stack size is 32-bit, 
                                  	//  value indicating size in bytes

//=============================================================================
// Platform-specific defines

// $$$ Define the following for turning the Status Pin off(0) and on(1):
#define PLT_STATUS_PIN_SET		AM_REG(GPIO,WTSA) = 0x800;
#define PLT_STATUS_PIN_CLEAR	AM_REG(GPIO,WTCA) = 0x800;

void pltStatusPinSet(void);
void pltStatusPinClear(void);

// $$$ Define the following for turning interrupts off and on:
#define PLT_INTERRUPTS_OFF      am_hal_interrupt_master_disable()

#define PLT_INTERRUPTS_ON       am_hal_interrupt_master_enable()

//=============================================================================
//} End of Platform-specific defines
//=============================================================================

//=============================================================================
// Platform-specific keywords:

// $$$ If your platform needs a "restrict" keyword, define it here.
// $$$ Note the default #define used below if not defined here.
#if defined ( __CC_ARM   )
// For ARM Compiler (DS-5, Keil MDK)
#define  RESTRICT __restrict
#elif defined ( __GNUC__ )
// For gcc
#define  RESTRICT __restrict__
#else

// Default definitions:
// (not needed on this platform)
// Note: If the Compiler does not support restrict keyword, define it to nothing
#define  RESTRICT

#endif

//==============
// $$$ If your platform needs this, define it here.
// $$$ Note the default #define used below if not defined here.
#ifndef RESTORE_ON_WAKEUP
// Default definitions:
// (not needed on this platform)
#define RESTORE_ON_WAKEUP
#endif

//==============
// $$$ If your platform needs large arrays to be declared "static" so
// $$$  that they won't take up local stack space, define it here as "1".
// $$$ Note the default #define used below if not defined here.

#ifndef STATIC_LONG_ARRAY
// Default definitions:
// (not needed on this platform)
#define STATIC_LONG_ARRAY 0
#endif

//=============================================================================
// Platform-specific functions, see .c files for descriptions
void pltStatusPinFlash(ulp_u1 flashes);

#endif /* _PLATFORM_H_ */
