//*****************************************************************************
//
//  am_int_dispatcher.S
//! @file
//!
//! @brief Ambiq Hal Interrupt Dispatcher
//!
//! @addtogroup dsp4
//! @ingroup apollo4hal
//! @{

//*****************************************************************************

//*****************************************************************************
//
// ${copyright}
//
// This is part of revision ${version} of the AmbiqSuite Development Package.
//
//*****************************************************************************

#include <xtensa/coreasm.h>
#include "xtos-internal.h"

    .text
    .align  4
    .global am_hal_dsp_dispatcher
am_hal_dsp_dispatcher:
// todo: do we need 16 here
    entry a1, 16;
CHECK_NEXT_LINKED_LIST:
    // if a2(next int handler in the linked-list) is zero, return.
    BEQZ.W15  a2, No_MORE_FUNCS_FOR_DISPATCH
//todo: for debugging, the a2 range can be checked here. a2 should point to IntHandlerStruct.
MORE_HANDLER_IN_THE_LIST:
    // loading the following:
    // a9  = void     *pHandler;
    // a10 = void     *pHandlerCtxt;
    // a3  = void     *pIRQStatusRegAdd;
    // a4  = uint32_t  IRQStatusRegVal;
    // a5  = void     *dNextIntHandler;
    // a6, a7, and a8 are loaded and reserved for future use

    AET_L32X4_AR_IP   a9,a10,a3,a4,a2,4*4
    AET_L32X4_AR_IP   a5, a6,a7,a8,a2,4*4

    // NOP: The processor add one stall here before continue
    // load the interrupt status register from the external memory(pIRQStatusRegAdd) into a3
        // clear a2
    { L32I       a3, a3, 0 ;  MOVI a2, 0 ;    }
    // BNONE: branches if all of the bits specified by the mask in address register at are clear
    // in address register as (that is, if none of them are set). The test is performed by taking
    // the bitwise logical and of as with at and testing if the result is zero.
        // Copy the address of the next linked-list to a2 ( a5+0 )
    { BNONE.W15   a3, a4, CHECK_NEXT_LINKED_LIST ; ADD  a2, a2, a5; }
    // call the function
    CALLX8        a9
    // any more handler in the list
    BNEZ.N  a2, MORE_HANDLER_IN_THE_LIST
No_MORE_FUNCS_FOR_DISPATCH:
    // return from dispatcher
    RETW.N



// extern void am_hal_delay_us(uint32_t ui32us);
// delay_overhead_cycle : the call function(7) + return(7) + function asm(12)
// Note: The maximum allowed number for delay_overhead_cycle is ULP_FREQ.
#define delay_overhead_cycle  (29)

#define DSP_FREQ_BIT_MASK 0x0000018
// REG_PWRCTRL_DSP0PERFREQ register definition
// Bits Name            RW      Flags       Description
// 31:5 RSVD            RO      Reserved    RESERVED
// 4:3  DSP0PERFSTATUS  RO                  DSP0 Performance mode request
//                                             ULP  =   0x0 -   DSP0 is in ULP mode (freq=48MHz)
//                                             LP   =   0x1 -   DSP0 is in LP mode (freq=192MHz)
//                                             HP   =   0x2 -   DSP0 is in HP mode (freq=384MHz)
// 2:2  DSP0PERFACK     RO                  Indicates the DSP0 performance status indicated in STATUS register is valid.
// 1:0  DSP0PERFREQ     RW                  DSP0 Performance mode request
//                                             ULP  =   0x0 -   DSP0 to be run in ULP mode (freq=48MHz)
//                                             LP   =   0x1 -   DSP0 to be run in LP mode (freq=192MHz)
//                                             HP   =   0x2 -   DSP0 to be run in HP mode (freq=384MHz)
#define REG_PWRCTRL_DSP0PERFREQ   0x40021054
#define REG_PWRCTRL_DSP1PERFREQ   0x40021074
#define REG_PWRCTRL_DSP_PERFREQ   REG_PWRCTRL_DSP0PERFREQ

#define BIT_ULP   3
#define BIT_LP    3
#define BIT_HP    4
#define ULP_FREQ  48
#define LP_FREQ   192
#define HP_FREQ   384

    .global FREQ_DETECTED1, FREQ_DETECTED2, FREQ_DETECTED3

    .text
    .literal_position   // tells the assembler/linker to place literals here
    .align  4
regDspFreqAdd: .word   REG_PWRCTRL_DSP_PERFREQ
extractSign:   .word   0x7FFFFFFF
    .align  4

    .global am_hal_delay_us
am_hal_delay_us:
// todo: do we need 16 here
//
    entry a1, 16;
// a2 : the delay in usec
    L32R a4, regDspFreqAdd;  // a4 = REG_PWRCTRL_DSP_PERFREQ

// if delay == 0 return immediately
    BEQZ  a2, _return_from_am_hal_delay_us
// Read the current value of CCOUNT
    RSR  a3, CCOUNT;         // a3 = CCOUNT
// move a2 to aed0
    L32I.N a5, a4, 0 ;       // read the value of REG_PWRCTRL_DSP_PERFREQ from memory
//TODO: FOR TEST
//    MOVI   a5, 0x00;//18
// END FOR TEST
    // check bit 3:4 of REG_PWRCTRL_DSP_PERFREQ: 0=ULP, 1=LP, 2=HP
    MOVI   a6, HP_FREQ;
    // Branch if Bit Set
    BBSI   a5, BIT_HP, FREQ_DETECTED1  ; // frequency is 386
    MOVI   a6, LP_FREQ;
    BBSI   a5, BIT_LP, FREQ_DETECTED2  ; // frequency is 192
    MOVI   a6, ULP_FREQ;
    BBCI   a5, BIT_ULP, FREQ_DETECTED3 ; // frequency is 48
FREQ_DETECTED1:  // a6 has the frequency
    // the followig NOPs make sure all the branches takes the same number of clocks
    NOP;
    NOP;
FREQ_DETECTED2:  // a6 has the frequency
    NOP;
    NOP;
FREQ_DETECTED3:  // a6 has the frequency

// there is no unsigned 32*32 mul in HIFI5. The only one exists has 72bit result.
// to do unsigned 32 * 32 bit the following code written:
    // AED0= [a6, a2]
    //  Move two AR register into AE_DR
    { AE_MOVDA32  aed0, a6 ;    AE_MOVDA32  aed1, a2 ;}

    // AED0 = AED0.H * AED0L
    {MOVI a5, 0 ; MOVI a2, delay_overhead_cycle ; AE_MUL32U.ll  aed0, aed0, aed1 ;} // Signed 32-bit by Unsigned 32-bit multiply with 64-bit result
    // aed0 has the number of cycles.

//  The sudo code for the while loop
//  aed0 = aed0 - cycleAdjustment;
//  while(aed >0){
//      a4= CCOUNT;
//      if (a4 > a3) aed1 = a4-a3;
//      if (a4 < a3) aed1 = 0xffffffff- a3 + a4;
//      a3=a4;
//      aed0 = aed0 - aed1;
//      if (aed0 <0) return;
//  }

// Load the delay_overhead_cycle to AED1
    AE_MOVDA32X2  aed1, a5, a2 ;
    L32R a7, extractSign ;   // a7 = 0x7FFFFFFF
    MOVI a2, 1 ;  // return value from the function
// AED0 = AED0 -AED1
    AE_SUB64  aed0, aed0, aed1;
    AND  a3 , a3, a7 ;           // make sure bit 32(sign) is always zero

// While loop starts here
DELAY_LOOP:
    RSR  a4, CCOUNT;             // a4 = CCOUNT
    AND  a4 , a4, a7 ;           // make sure bit 32(sign) is always zero
DELAY_LOOP1:
//  SUB  a6, a4, a3 ;            // a6   = a4 - a3
//  AE_MOVDA32x2  aed1, a5, a6 ; // aed1 = a4 - a3
    BGEU a3, a4, WRAPAROUND_FOR_CCOUNT
//  if (a4 > a3) aed1 = a4-a3;
    SUB  a6, a4, a3 ;            // a6   = a4 - a3
    AE_MOVDA32x2  aed1, a5, a6 ; // aed1 = a4 - a3
    AE_SUB64  aed0, aed0, aed1 ; // aed0 = aed0 - aed1
    AE_MOVAD8 a6 , aed0, 7 ;     // a2   = aed0[63...56]
    MOV       a3, a4 ;           // a3   = a4
    RSR       a4, CCOUNT ;       // a4 = CCOUNT
// if aed0[63...56] == 0xff, then the number is negative
    BNEZ      a6, _return_from_am_hal_delay_us
    AND  a4 , a4, a7 ;           // make sure bit 32(sign) is always zero
    J         DELAY_LOOP1

WRAPAROUND_FOR_CCOUNT:
//      if (a4 < a3) a2 = 0xffffffff- a3 + a4;
    { AE_MOVDA32x2  aed1, a5, a7 ; AE_MOVDA32x2  aed2, a5, a3 ; NOP; NOP;} // aed1 = 0x000000007fffffff ; aed2 = a3
    { AE_SUB64  aed1, aed1, aed2 ; AE_MOVDA32x2  aed2, a5, a4 ; NOP; NOP;}// aed1 = 0x7fffffff- a3 ; aed2 = a4
    AE_ADD64  aed1, aed1, aed2 ; // aed1 = 0xffffffff- a3 + a4
    AE_SUB64  aed0, aed0, aed1 ; // aed0 = aed0 - aed1
    AE_MOVAD8 a6 , aed0, 7     ; // a2   = aed0[63...56]
// if aed0[63...56] == 0xff, then the number is negative
    MOV       a3, a4 ;           // a3   = a4
    RSR  a4, CCOUNT;             // a4 = CCOUNT
    AND  a4 , a4, a7 ;           // make sure bit 32(sign) is always zero
    BEQZ      a6, DELAY_LOOP1
_return_from_am_hal_delay_us:
    // return from dispatcher
    RETW
