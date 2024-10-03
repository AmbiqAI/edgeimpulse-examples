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
/* ============================================================================ */
/* ULPBench: Platform-specific functions (Template)                             */
/* ============================================================================ */

//=============================================================================
// Platform-specific includes
#include "BenchmarkConfig.h"
#include "Platform.h"
#include "TesTPI.h"

//=============================================================================
// $$$ Adjust this value by trial and error so that pltSpin takes about
// $$$  1/8 second. Or in other words, pltStatusPinFlash should do 2 LED
// $$$  flashes in no more than 1/2 second.
#define PLT_FLASH_COUNT 150000

//=============================================================================
// $$$ board/chip includes:
#include "board.h"

// $$$ external to set up MCU hardware
void hardware_setup_part1( void );
void hardware_setup_part2( void );

//=============================================================================

//*****************************************************************************
//
// Platform Status Pin Set
//
//*****************************************************************************
void pltStatusPinSet(void)
{
    //
    // Set indicator pin.
    //
    AM_REG(GPIO,WTSA) = 0x800;
}

//*****************************************************************************
//
// Platform Status Pin Clear
//
//*****************************************************************************
void pltStatusPinClear(void)
{
    //
    // Clear indicator pin.
    //
    AM_REG(GPIO,WTCA) = 0x800;
}


//=============================================================================
// Platform Print String - for debugging only
void pltStringPrint(char * text)
{

}

//=============================================================================
// Platform Flash Spin - See note above for PLT_FLASH_COUNT
void pltFlashSpin(void)
{
    volatile ulp_u4 i;        // use volatile to avoid optimizing away
    for (i = 0; i < PLT_FLASH_COUNT; i++);

}

//=============================================================================
// Platform IO Pin Flash - See note above for PLT_FLASH_COUNT
void pltStatusPinFlash(ulp_u1 numFlashes)
{
    while (numFlashes--) {
        PLT_STATUS_PIN_SET;
        pltFlashSpin();         // spin while pin = 1
        PLT_STATUS_PIN_CLEAR;
        pltFlashSpin();         // spin while pin = 0
    }
}

//=============================================================================
// Platform Sleep
void pltSleep( void )
{
    //
    // Enter deep sleep.
    //
    AM_REG(SYSCTRL, SCR) = AM_REG_SYSCTRL_SCR_SLEEPDEEP_M;

    //
    // WFI
    //
    AM_ASM_WFI;
}

//=============================================================================
// Main function. Initialize and call Primary Thread's main function.
int main( void )
{
    VoidFunctionPtr threadMainFunction;

    // $$$ Call code to set up Platform hardware, such as:
    //   Platform initializations.
    //   Unused board circuits turned off.
    //   Unused MCU peripherals turned off.
    //   Set StatusPin to output mode.
    //   Select clock source/mode when benchmark is executing.
    //   Select sleep mode when benchmark is not executing.
    //   Start 1-second timer interrupt.
    hardware_setup_part1();

    // $$$ If desired, do a single flash of the StatusPin to indicate MCU is alive.
    pltStatusPinFlash(1);

    //
    // Initialize TES
    //
    threadMainFunction = tesInitialize();

    //   Select clock source/mode when benchmark is executing.
    //   Select sleep mode when benchmark is not executing.
    //   Start 1-second timer interrupt.
    hardware_setup_part2();

    threadMainFunction();       // run the Primary Thread's Main function, no return

    return 0;
}

#ifdef TES_MULTIPLE_THREAD_STARTS
//=============================================================================
// Platform Initialize a Thread for first execution at its main function.
//
// It creates a dummy stack frame at the top of the stack space.
//
// Note: TES contest switching does NOT happen inside exception handler
//       As a result, need to save PC.
//    Memory
//            |/////////////|
//            |-------------| <- End of stack
//            |             | <- (might have a padding word here due to double word stack alignement)
//            |   R4-R11    | 8 words, not initialized
//            |   PC        | 1 word
//            |   CONTROL   | Required for Cortex-M4 with FPU (for FPCA bit)
//            |-------------| <- SP value
//            |(stack space)|
//            |             |
//            :             :
//            :             :
//            |             | <- PltStackCell * stack
//            |-------------|
//
// Note: Due to AAPCS requirement, the SP value need to be double word aligned.

void pltThreadInit(TesThread * thread, PltStackCell * stack, PltStackSize stackSize, void * main)
{
    // $$$ Code to initialize thread details. Not needed if only Primary Thread.
    PltStackCell * sp = &stack[stackSize - PLT_NUM_SAVE_REGS]; // stack runs backwards
    // PLT_NUM_SAVE_REGS equal 10, and is defined in PlatformConfig.h
    sp  = (PltStackCell *) (((uint32_t) sp) & ~(0x4UL));  // force sp to be double word aligned (AAPCS)
    // setup thread starting address
    *(sp+1) = (PltStackCell)main;               // set up for a "return" to the "main"
    // setup CONTROL register in stack frame which include FPCA bit in context switch
    *(sp)   = (PltStackCell) 0;                 // Initial CONTROL value
    thread->stack = stack;                      // stack area
    thread->stackSize = stackSize;              // stack size
    thread->sp = sp;        // set sp at last stacked item

}

//=============================================================================
// The code for Platform Swap Stacks
// Note: For ULPbench-Core profile, stack switching is not required.

// Note: pltThreadSwapStacks is implemened in PlatformStackSwap.c
//void pltThreadSwapStacks( PltStackCell ** addrNewSP, PltStackCell ** addrOldSP )
//{
//    // $$$ Code to switch stacks from one Thread to another.
//    #error pltThreadSwapStacks not implemented
//}

//=============================================================================
// Example of extra threads, not required for ULPbench-Core Profile
//void SecondThread(void)
//{
//  while(1){
//    TesEvent * event = tesEventWait();
//    }
//}


#endif // TES_MULTIPLE_THREAD_STARTS

