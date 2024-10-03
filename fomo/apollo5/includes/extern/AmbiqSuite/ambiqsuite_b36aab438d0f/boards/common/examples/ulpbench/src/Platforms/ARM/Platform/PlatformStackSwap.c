/**************************************************************************//**
 * @file     PlatformStackSwap.c
 * @brief    ULPbench-Core TES task scheduler stack switching code
 * @version  V0.00
 * @date     21. June 2014
 *
 * @note     Ported by Joseph Yiu, ARM
 *
 ******************************************************************************/

/*******************************************************************************
*
* Purpose : Platform-specific stack switching code for TES.
*           By default the ULPbench-Core only have one thread so this is never
*           used. So currently the stack switching code is implemented as a
*           blank function (see end).
*           If multiple thread is needed, set the TES_MULTIPLE_THREAD_STARTS macro.
*
*           TES is a task scheduler in ULPbench-core, the source code is 
*           contributed by Renesas.
*******************************************************************************/

#include "TesTPI.h"


#ifdef TES_MULTIPLE_THREAD_STARTS
/* -------------------------------------------------- */
/* 
  Note: Context switching can happen only when a thread
        execute tesEventWait.

  tesEventWait -> tesEventDispatchReady -> tesThreadRun -> pltThreadSwapStacks
	
  According to AAPCS, R0-R3, R12, APSR of the thread should already be saved by caller,
  so we need to save R4-R11, LR

  The stack frame look like this (also see pltThreadInit() in Platform.c):

            |/////////////|
            |-------------| <- End of stack
            |             | <- (might have a padding word here due to double word stack alignement)
            |   R4-R11    | 8 words, not initialized
            |   PC        | 1 word
            |   CONTROL   | Required for Cortex-M4 with FPU (for FPCA bit)
            |-------------| <- SP value
            |(stack space)|
  
  This function has two input parameters:
  r0 = addrNewSP, r1 = addrOldSP
  
  The following Macros are defined in CMSIS-Core header: __CORTEX_M, __FPU_USED

*/
/* -------------------------------------------------- */
#if (__CORTEX_M == 0)
/* Cortex-M0 or Cortex-M0+ 
   Note: saving of CONTROL is optional for ARMv6-M
   Push total of 10 registers (even number) to keep stack frame double word aligned
*/
#if   defined ( __CC_ARM )
  /* ARM C Compiler or Keil MDK-ARM */
  __asm void pltThreadSwapStacks(PltStackCell ** addrNewSP, PltStackCell ** addrOldSP)
  { 
     PUSH {R4-R7}
     MOV  R2, R8
     MOV  R3, R9
     MOV  R4, R10
     MOV  R5, R11
     MRS  R6, CONTROL ; optional
     PUSH {R2-R5}
     PUSH {R6, LR}
     MOV  R2, SP
     STR  R2, [R1]
     
     LDR  R2, [R0]
     MOV  SP, R2
     POP  {R6, R7}
     MOV  LR, R7
     MSR  CONTROL, R6 ; optional
     POP  {R2-R5}
     MOV  R8, R2
     MOV  R9, R3
     MOV  R10, R4
     MOV  R11, R5
     POP  {R4-R7}
     BX   LR  
  }
#elif defined ( __ICCARM__ )
  /* IAR C Compiler */
  void pltThreadSwapStacks(PltStackCell ** addrNewSP, PltStackCell ** addrOldSP)
  { 
     __asm volatile ("push {r4-r7}\n"
                   "mov  r2, r8\n"
                   "mov  r3, r9\n"
                   "mov  r4, r10\n"
                   "mov  r5, r11\n"
                   "mrs  r6, control\n"
                   "push {r2-r5}\n"
                   "push {r6, lr}\n"
                   "str  r2, [r1]\n"
                   "mov  sp, r2\n"
                        
                   "ldr  r2, [r0]\n"
                   "mov  sp, r2\n"
                   "pop  {r6, r7}\n"
                   "mov  lr, r7\n"
                   "msr  control, r6\n"
                   "pop  {r2-r5}\n"
                   "mov  r8, r2\n"
                   "mov  r9, r3\n"
                   "mov  r10, r4\n"
                   "mov  r11, r5\n"
                   "pop  {r4-r7}\n"
                   "bx   lr"
                   );
  }
#elif defined ( __GNUC__ )
   /* gcc */
  void pltThreadSwapStacks(PltStackCell ** addrNewSP, PltStackCell ** addrOldSP) __attribute__ ( ( naked ) );

  void pltThreadSwapStacks(PltStackCell ** addrNewSP, PltStackCell ** addrOldSP)
  { 
   __asm __volatile__ ("push {r4-r7}\n"
                   "mov  r2, r8\n"
                   "mov  r3, r9\n"
                   "mov  r4, r10\n"
                   "mov  r5, r11\n"
                   "mrs  r6, control\n"
                   "push {r2-r5}\n"
                   "push {r6, lr}\n"
                   "str  r2, [r1]\n"
                   "mov  sp, r2\n"
                       
                   "ldr  r2, [r0]\n"
                   "mov  sp, r2\n"
                   "pop  {r6, r7}\n"
                   "mov  lr, r7\n"
                   "msr  control, r6\n"
                   "pop  {r2-r5}\n"
                   "mov  r8, r2\n"
                   "mov  r9, r3\n"
                   "mov  r10, r4\n"
                   "mov  r11, r5\n"
                   "pop  {r4-r7}\n"
                   "bx   lr");
		   
    }
#endif

#endif
/* -------------------------------------------------- */
#if ((__CORTEX_M == 3) || ((__CORTEX_M == 4) && (__FPU_USED == 0)))
/* Cortex-M3 or Cortex-M4 without FPU
   Note: saving of CONTROL is optional if without FPU
   Push total of 10 registers (even number) to keep stack frame double word aligned
*/
#if   defined ( __CC_ARM )
  /* ARM C Compiler or Keil MDK-ARM */
  __asm void pltThreadSwapStacks(PltStackCell ** addrNewSP, PltStackCell ** addrOldSP)
  {
     PUSH {R4-R11}
     MRS  R2, CONTROL ; optional - push total of 10 registers to keep stack frame double word aligned
     PUSH {R2, LR}
     STR  SP, [R1]
     
     LDR  SP, [R0]
     POP  {R2, R3}
     MSR  CONTROL, R2 ; optional
     POP  {R4-R11}
     BX   R3
  }
#elif defined ( __ICCARM__ )
  /* IAR C Compiler */
  void pltThreadSwapStacks(PltStackCell ** addrNewSP, PltStackCell ** addrOldSP)
  {
   __asm volatile ("push {r4-r11}\n"
                   "mrs  r2, control\n"
                   "push {r2, lr}\n"
                   "str  sp, [r1]\n"
     
                   "ldr  sp, [r0]\n"
                   "pop  {r2, r3}\n"
                   "msr  control, r2\n"
                   "pop  {r4-r11}\n"
                   "bx   r3");

  }
#elif defined ( __GNUC__ )
  /* gcc */
  void pltThreadSwapStacks(PltStackCell ** addrNewSP, PltStackCell ** addrOldSP) __attribute__ ( ( naked ) );

  void pltThreadSwapStacks(PltStackCell ** addrNewSP, PltStackCell ** addrOldSP)
  {
   __asm __volatile__ ("push {r4-r11}\n"
                   "mrs  r2, control\n"
                   "push {r2, lr}\n"
                   "str  sp, [r1]\n"
     
                   "ldr  sp, [r0]\n"
                   "pop  {r2, r3}\n"
                   "msr  control, r2\n"
                   "pop  {r4-r11}\n"
                   "bx   r3");
  }
#endif
#endif


/* -------------------------------------------------- */
#if ((__CORTEX_M == 4) && (__FPU_USED == 1))
/* Cortex-M4 with FPU 
   Note: Need to save CONTROL because of FPCA bit.
*/

#if   defined ( __CC_ARM )
  /* ARM C Compiler or Keil MDK-ARM */
  __asm void pltThreadSwapStacks(PltStackCell ** addrNewSP, PltStackCell ** addrOldSP)
  {
     PUSH     {R4-R11}
     MRS      R2, CONTROL  ; FPCA bit need to be saved
     TST      R2, #0x4
     IT       NE
     VPUSHNE  {S16-S31}    ; Save FPU registers if FPCA is 1     
     PUSH     {R2, LR}
     STR      SP, [R1]
     
     LDR      SP, [R0]
     POP      {R2, R3}
     MOV      LR, R3
     MSR      CONTROL, R2  ; FPCA bit need to be saved
     TST      R2, #0x4
     IT       NE
     VPOPNE   {S16-S31}    ; Restore FPU registers if FPCA is 1
     POP      {R4-R11}
     BX       LR  
  }
#elif defined ( __ICCARM__ )
  /* IAR C Compiler */
  void pltThreadSwapStacks(PltStackCell ** addrNewSP, PltStackCell ** addrOldSP)
  {
   __asm volatile ("push     {r4-r11}\n"
                   "mrs      s2, control\n"
                   "tst      s2, #0x4\n"
                   "it       ne\n"
                   "vpushne  {s16-s31}\n"
                   "push     {r2, lr}\n"
                   "str      sp, [r1]\n"
     
                   "ldr      sp, [r0]\n"
                   "pop      {r2, r3}\n"
                   "mov      lr, r3\n"
                   "msr      control, r2\n"
                   "tst      r2, #0x4\n"
                   "it       ne\n"
                   "vpopne   {s16-s31}\n"
                   "pop      {r4-r11}\n"
                   "bx       lr" );
  }
#elif defined ( __GNUC__ )
  /* gcc */
  void pltThreadSwapStacks(PltStackCell ** addrNewSP, PltStackCell ** addrOldSP) __attribute__ ( ( naked ) );

  void pltThreadSwapStacks(PltStackCell ** addrNewSP, PltStackCell ** addrOldSP)
  {

   __asm __volatile__ ("push     {r4-r11}\n"
                   "mrs      s2, control\n"
                   "tst      s2, #0x4\n"
                   "it       ne\n"
                   "vpushne  {s16-s31}\n"
                   "push     {r2, lr}\n"
                   "str      sp, [r1]\n"
     
                   "ldr      sp, [r0]\n"
                   "pop      {r2, r3}\n"
                   "mov      lr, r3\n"
                   "msr      control, r2\n"
                   "tst      r2, #0x4\n"
                   "it       ne\n"
                   "vpopne   {s16-s31}\n"
                   "pop      {r4-r11}\n"
                   "bx       lr" );
  }

#endif
#endif
/* -------------------------------------------------- */
#else

// Only one thread is used, do not require any stack switching code
void pltThreadSwapStacks(PltStackCell ** addrNewSP, PltStackCell ** addrOldSP)
{
  return;
}
#endif
