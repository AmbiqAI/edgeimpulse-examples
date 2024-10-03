//*****************************************************************************
//
//! @file am_hal_sysctrl.c
//!
//! @brief Functions for interfacing with the M4F system control registers
//!
//! @addtogroup sysctrl3p SYSCTRL - System Control
//! @ingroup apollo3p_hal
//! @{
//
//*****************************************************************************

//*****************************************************************************
//
// ${copyright}
//
// This is part of revision ${version} of the AmbiqSuite Development Package.
//
//*****************************************************************************

#include <stdint.h>
#include <stdbool.h>
#include "am_mcu_apollo.h"

//*****************************************************************************
//
// Globals
//
//*****************************************************************************
//
//! g_am_hal_sysctrl_sleep_count is a running total of the number of times the
//! MCU has gone to sleep. (Wraps around at uint32_t max)
//
uint32_t g_am_hal_sysctrl_sleep_count = 0;

#ifdef AM_HAL_SYSCTRL_DEEPSLEEP_WA
//
// Execute this deepsleep function in SRAM in order to implement a power
// saving algorithm.
// Function returns 0 if the power saving was successful, 1 otherwise.
// #### INTERNAL BEGIN ####
// A2SD-1538. Requires that the FLASHCACHEPGCTRL register be enabled.
// A companion SBL patch is available to enable that register in CV space.
//
// Following is the assembly code with comments.
//      push {r3-r5,lr}         // Save required regs and LR
//      push {r0}               // Save the calling parameter for later
//      movw r2, #0x0178        // Address for MCUCTRL.FLASHCACHEPGCTRL (undocumented register)
//      movt r2, #0x4002        // "
//      ldr  r3, [r2, #0]       // Get the current value of FLASHCACHEPGCTRL
//      nop
//      // Set FLASHCACHEPGCTRL=0x00066590 (a magic value given us by
//      // design/validation for the lowest deepsleep power).
//      movw   r1, #0x6590      // Set FLASHCACHEPGCTRL for min deepsleep current
//      movt   r1, #0x0006      //  "
//      str    r1, [r2, #0]     //  "
//      movs   r4, #0           // Get function default return value of 0
//      ldr    r5, [r2, #0]     // Read FLASHCACHEPGCTRL
//      cmp    r5, r1           // Is it 0x00066590?
//      it     ne
//      movne  r4, #1           // N: Get return value of 1
//      dsb                     // DSB (prepare for sleep using the usual sleep sequence)
//      movs   r0, #0x0000      // Execute the am_hal_sysctrl_bus_write_flush() macro
//      nop
//      movt   r0, #0x5FFF      //  "
//      ldr    r1, [r0, #0]     //  "
//      wfi                     // WFI
//      isb                     // On wake execute ISB
//      str      r3, [r2,#0]    // Restore FLASHCACHEPGCTRL
//      // The workaround calls for a 5us delay before returning to flash execution
//      pop      {r0}           // Get arg for bootrom_delay_cycles()
//      movw    r1, #0x009d     // Call bootrom_delay_cycles() helper function
//      movt    r1, #0x0800     //  "
//      blx     r1              //  "
//      mov     r0, r4          // Get return value
//      pop     {r3-r5,pc}      // Restore and return
// #### INTERNAL END ####
//
uint32_t SRAM_sleep[20] =
{
    0xB401B538,         // B538         push {r3-r5,lr}
                        // B401         push {r0}
    0x1278F240,         // F2401278     movw r2, #0x0178
    0x0202F2C4,         // F2C40202     movt r2, #0x4002
    0xBF006813,         // 6813         ldr  r3, [r2, #0]
                        // BF00         nop
    0x51B0F646,         // F64651B0     movw   r1, #0x6DB0
    0x0106F2C0,         // F2C00106     movt   r1, #0x0006
    0x24006011,         // 6011         str    r1, [r2, #0]
                        // 2400         movs   r4, #0
    0x428D6815,         // 6815         ldr    r5, [r2, #0]
                        // 428D         cmp    r5, r1
    0x2401BF18,         // BF18         it     ne
                        // 2401         movne  r4, #1
    0x8F4FF3BF,         // F3BF8F4F     dsb
    0xBF002000,         // 2000         movs   r0, #0x0000
                        // BF00         nop
    0x70FFF6C5,         // F6C570FF     movt   r0, #0x5FFF
    0xBF306801,         // 6801         ldr    r1, [r0, #0]
                        // BF30         wfi
    0x8F6FF3BF,         // F3BF8F6F     isb
    0xBC016013,         // 6013         str      r3, [r2,#0]
                        // BC01         pop      {r0}
    0x019dF240,         // F240019D     movw    r1, #0x009d
    0x0100F6C0,         // F6C00100     movt    r1, #0x0800
    0x46204788,         // 4788         blx     r1
                        // 4620         mov     r0, r4
    0xBF00BD38,         // BD38         pop     {r3-r5,pc}
                        // BF00         nop
    0xBF00BF00
};

//
// If additional deepsleep power saving is desired, incurring the cost of
// a short delay on wake, the user can call am_hal_sysctrl_control() to
// enable the power savings. This variable holds that state.
//  Bit0: Workaround Enabled
//  Bit1: Good-to-go to use workaround
//  That is, only do the workaround if g_ui32DeepsleepMinPwr == 0x03.
//
#define DSLP_WORKAROUND_DIS     0x00
#define DSLP_WORKAROUND_EN      0x01
#define DSLP_WORKAROUND_GO      0x02
static uint32_t g_ui32DeepsleepMinPwr = DSLP_WORKAROUND_DIS;

//
// After sleep, this variable will be 0 if additional power saving
// was successful.
//
uint32_t g_ui32SleepReturnVal = 0;
#endif // AM_HAL_SYSCTRL_DEEPSLEEP_WA
// #### INTERNAL BEGIN ####
#define AM_HAL_DEEPSLEEP_MINPWR_DEBUG 0
#if AM_HAL_DEEPSLEEP_MINPWR_DEBUG
uint32_t g_SleepCnt = 0;
uint32_t g_WorkaroundSleepCnt = 0;
#endif // AM_HAL_DEEPSLEEP_MINPWR_DEBUG
// #### INTERNAL END ####


// ****************************************************************************
//
// Apply various specific commands/controls on the SYSCTRL module.
//
// ****************************************************************************
uint32_t
am_hal_sysctrl_control(am_hal_sysctrl_control_e eControl, void *pArgs)
{
#ifdef AM_HAL_SYSCTRL_DEEPSLEEP_WA
    switch ( eControl )
    {
        case AM_HAL_SYSCTRL_CONTROL_DEEPSLEEP_MINPWR_DIS:
            g_ui32DeepsleepMinPwr = DSLP_WORKAROUND_DIS;
            break;

        case AM_HAL_SYSCTRL_CONTROL_DEEPSLEEP_MINPWR_EN:
            //
            // Activate the workaround that allows minimum power consumption
            // during deepsleep with the cost of a short delay after wake.
            // Before enabling, make sure the trim patch that allows the
            // workaround to function has been applied.
            //
// #### INTERNAL BEGIN ####
            // For Apollo3p patch information see:
            // https://ambiqmicro.atlassian.net/wiki/spaces/AP/overview
// #### INTERNAL END ####
            g_ui32DeepsleepMinPwr = AM_REGVAL(PATCHVER2) & 0x00000001 ?
                                    DSLP_WORKAROUND_DIS : DSLP_WORKAROUND_EN;
            break;

        default:
            return AM_HAL_STATUS_INVALID_ARG;
    }

    //
    // Return success status.
    //
    return AM_HAL_STATUS_SUCCESS;
#else // AM_HAL_SYSCTRL_DEEPSLEEP_WA
    switch ( eControl )
    {
        case AM_HAL_SYSCTRL_CONTROL_DEEPSLEEP_MINPWR_DIS:
            return AM_HAL_STATUS_INVALID_OPERATION;

        case AM_HAL_SYSCTRL_CONTROL_DEEPSLEEP_MINPWR_EN:
            return AM_HAL_STATUS_INVALID_OPERATION;

        default:
            return AM_HAL_STATUS_INVALID_ARG;
    }
#endif // AM_HAL_SYSCTRL_DEEPSLEEP_WA
} // am_hal_sysctrl_control()

//*****************************************************************************
//
// Place the core into sleep or deepsleep.
//
// This function puts the MCU to sleep or deepsleep depending on bSleepDeep.
//
// Valid values for bSleepDeep are:
//     AM_HAL_SYSCTRL_SLEEP_NORMAL
//     AM_HAL_SYSCTRL_SLEEP_DEEP
//
//*****************************************************************************
void
am_hal_sysctrl_sleep(bool bSleepDeep)
{
    bool bBurstModeSleep;
    am_hal_burst_mode_e eBurstMode;

    //
    // Disable interrupts and save the previous interrupt state.
    //
    AM_CRITICAL_BEGIN

    g_am_hal_sysctrl_sleep_count++;

    //
    // If Apollo3 Blue Plus rev 0 and in burst mode, must exit burst mode
    // before going to sleep.
    //
// #### INTERNAL BEGIN ####
#if 0
    // Currently no need to detect rev 0 of Apollo3 Blue Plus.
    if ( APOLLO3_C0  &&  (am_hal_burst_mode_status() == AM_HAL_BURST_MODE) )
#endif
// #### INTERNAL END ####
    if ( am_hal_burst_mode_status() == AM_HAL_BURST_MODE )
    {
        bBurstModeSleep = true;

        // #### INTERNAL BEGIN ####
        //
        // Corrects MSPI2 failure on first transaction after entering burst mode
        // AS-1284, HSP20-521, HSP20-856, HSP20-863, HSP20-871
        //
        //
        // if ( (am_hal_burst_mode_disable(&eBurstMode) != AM_HAL_STATUS_SUCCESS)  ||
        //
        // #### INTERNAL END ####
        if ( (am_hal_burst_mode_disable(&eBurstMode) != AM_HAL_STATUS_SUCCESS)  ||
             (eBurstMode != AM_HAL_NORMAL_MODE) )
        {
            // #### INTERNAL BEGIN ####
            // FIXME - Although a failure is highly unlikely, this error needs
            //         to be handled more cleanly!
            // #### INTERNAL END ####
            while(1);
        }
#if AM_HAL_BURST_LDO_WORKAROUND
        else
        {
            am_hal_pwrctrl_wa_vddf_restore();
        }
#endif
    }
    else
    {
        bBurstModeSleep = false;
    }

    //
    // If the user selected DEEPSLEEP and the TPIU is off, attempt to enter
    // DEEP SLEEP.
    //
    if ( (bSleepDeep == AM_HAL_SYSCTRL_SLEEP_DEEP)    &&
         (MCUCTRL->TPIUCTRL_b.ENABLE == MCUCTRL_TPIUCTRL_ENABLE_DIS) )
    {
        //
        // Retrieve the reset generator status bits
        // This gets reset on Deep Sleep, so we take a snapshot here
        //
        if ( !gAmHalResetStatus )
        {
            gAmHalResetStatus = RSTGEN->STAT;
        }

        //
        // do not boost core and mem voltages
        //

// #### INTERNAL BEGIN ####
#if 0
        //
        // save original SIMOBUCK1 value, it will be restored
        //
        uint32_t ui32Simobuck1Backup = MCUCTRL->SIMOBUCK1;

        //
        // increase VDDC by 9 counts
        //
        uint32_t ui32Vddc = _FLD2VAL( MCUCTRL_SIMOBUCK1_MEMACTIVETRIM, ui32Simobuck1Backup );
        ui32Vddc += 9;

        //
        // check for overflow and limit
        //
        ui32Vddc =  ui32Vddc > (MCUCTRL_SIMOBUCK1_MEMACTIVETRIM_Msk>>MCUCTRL_SIMOBUCK1_MEMACTIVETRIM_Pos) ?
                    (MCUCTRL_SIMOBUCK1_MEMACTIVETRIM_Msk>>MCUCTRL_SIMOBUCK1_MEMACTIVETRIM_Pos) :
                    ui32Vddc;

        ui32Vddc = _VAL2FLD(MCUCTRL_SIMOBUCK1_MEMACTIVETRIM, ui32Vddc );

        //
        // increase VDDF by 24 counts
        //
        uint32_t ui32Vddf = _FLD2VAL( MCUCTRL_SIMOBUCK1_COREACTIVETRIM, ui32Simobuck1Backup ) ;
        ui32Vddf += 24 ;

        //
        // check for overflow and limit
        //
        ui32Vddf    = ui32Vddf > (MCUCTRL_SIMOBUCK1_COREACTIVETRIM_Msk >> MCUCTRL_SIMOBUCK1_COREACTIVETRIM_Pos) ?
                      (MCUCTRL_SIMOBUCK1_COREACTIVETRIM_Msk >> MCUCTRL_SIMOBUCK1_COREACTIVETRIM_Pos) :
                      ui32Vddf;
        ui32Vddf = _VAL2FLD(MCUCTRL_SIMOBUCK1_COREACTIVETRIM, ui32Vddf );

        //
        // remove original values of vddc and vddf and replace with modified values
        //
        uint32_t  ui32VddVffMask = MCUCTRL_SIMOBUCK1_MEMACTIVETRIM_Msk | MCUCTRL_SIMOBUCK1_COREACTIVETRIM_Msk;
        uint32_t  ui32SimoBuck1Working =
                      (ui32Simobuck1Backup & ~ui32VddVffMask) | ui32Vddc | ui32Vddf;

        //
        // write updated vddc and vddf to SIMOBUCK1 and wait for 20 microseconds
        //
        MCUCTRL->SIMOBUCK1 = ui32SimoBuck1Working;

        //
        // 20 micosecond delay
        //
        am_hal_flash_delay(FLASH_CYCLES_US(20));

        //
        // just before sleep, restore SIMOBUCK1 to original value
        //
        MCUCTRL->SIMOBUCK1 = ui32Simobuck1Backup;
#endif
// #### INTERNAL END ####

        //
        // Prepare the core for deepsleep (write 1 to the DEEPSLEEP bit).
        //
        SCB->SCR |= _VAL2FLD(SCB_SCR_SLEEPDEEP, 1);
#ifdef AM_HAL_SYSCTRL_DEEPSLEEP_WA
        //
        // Set the workaround flag.
        //
        g_ui32DeepsleepMinPwr |= DSLP_WORKAROUND_GO;
#endif // AM_HAL_SYSCTRL_DEEPSLEEP_WA
    }
    else
    {
        //
        // Prepare the core for normal sleep (write 0 to the DEEPSLEEP bit).
        //
        SCB->SCR &= ~_VAL2FLD(SCB_SCR_SLEEPDEEP, 1);
#ifdef AM_HAL_SYSCTRL_DEEPSLEEP_WA
        //
        // Clear the workaround flag.
        //
        g_ui32DeepsleepMinPwr &= ~DSLP_WORKAROUND_GO;
#endif // AM_HAL_SYSCTRL_DEEPSLEEP_WA
    }

#ifdef AM_HAL_SYSCTRL_DEEPSLEEP_WA
    if ( g_ui32DeepsleepMinPwr == (DSLP_WORKAROUND_EN | DSLP_WORKAROUND_GO) )
    {
// #### INTERNAL BEGIN ####
#if AM_HAL_DEEPSLEEP_MINPWR_DEBUG
        g_WorkaroundSleepCnt++;
#endif // AM_HAL_DEEPSLEEP_MINPWR_DEBUG
// #### INTERNAL END ####
        //
        // Call the Apollo3p specific SRAM sleep routine that will minimize
        // deep sleep current consumption. On wake a short delay is incurred.
        //
        uint32_t SRAMCode = (uint32_t)SRAM_sleep | 0x1;
        uint32_t (*pFunc)(uint32_t) = (uint32_t (*)(uint32_t))SRAMCode;
        g_ui32SleepReturnVal = (*pFunc)(FLASH_CYCLES_US(5));
    }
    else
    {
// #### INTERNAL BEGIN ####
#if AM_HAL_DEEPSLEEP_MINPWR_DEBUG
        g_SleepCnt++;
#endif // AM_HAL_DEEPSLEEP_MINPWR_DEBUG
// #### INTERNAL END ####
#endif // AM_HAL_SYSCTRL_DEEPSLEEP_WA
        //
        // Before executing WFI, flush any buffered core and peripheral writes.
        //
        __DSB();
        am_hal_sysctrl_bus_write_flush();

        //
        // Execute the sleep instruction.
        //
        __WFI();

        //
        // Upon wake, execute the Instruction Sync Barrier instruction.
        //
        __ISB();
#ifdef AM_HAL_SYSCTRL_DEEPSLEEP_WA
    }
#endif // AM_HAL_SYSCTRL_DEEPSLEEP_WA

    //
    // Restore burst mode?
    //
    if ( bBurstModeSleep )
    {
        bBurstModeSleep = false;

        // #### INTERNAL BEGIN ####
        //
        // Corrects MSPI2 failure on first transaction after entering burst mode
        // AS-1284, HSP20-521, HSP20-856, HSP20-863, HSP20-871
        //
        // #### INTERNAL END ####
        if ( (am_hal_burst_mode_enable(&eBurstMode) != AM_HAL_STATUS_SUCCESS)  ||
             (eBurstMode != AM_HAL_BURST_MODE) )
        {
            // #### INTERNAL BEGIN ####
            // FIXME - Although a failure is highly unlikely, this error needs
            //         to be handled more cleanly!
            // #### INTERNAL END ####
            //while(1);
        }
    }

    //
    // Restore the interrupt state.
    //
    AM_CRITICAL_END
}

//*****************************************************************************
//
// Enable the floating point module.
//
// Call this function to enable the ARM hardware floating point module.
//
//*****************************************************************************
void
am_hal_sysctrl_fpu_enable(void)
{
    //
    // Enable access to the FPU in both privileged and user modes.
    // NOTE: Write 0s to all reserved fields in this register.
    //
    SCB->CPACR = _VAL2FLD(SCB_CPACR_CP11, 0x3) |
                 _VAL2FLD(SCB_CPACR_CP10, 0x3);
}

//*****************************************************************************
//
// Disable the floating point module.
//
// Call this function to disable the ARM hardware floating point module.
//
//*****************************************************************************
void
am_hal_sysctrl_fpu_disable(void)
{
    //
    // Disable access to the FPU in both privileged and user modes.
    // NOTE: Write 0s to all reserved fields in this register.
    //
    SCB->CPACR = 0x00000000                         &
                 ~(_VAL2FLD(SCB_CPACR_CP11, 0x3) |
                   _VAL2FLD(SCB_CPACR_CP10, 0x3));
}

//*****************************************************************************
//
// Enable stacking of FPU registers on exception entry.
//
// @param bLazy - Set to "true" to enable "lazy stacking".
//
// This function allows the core to save floating-point information to the
// stack on exception entry. Setting the bLazy option enables "lazy stacking"
// for interrupt handlers.  Normally, mixing floating-point code and interrupt
// driven routines causes increased interrupt latency, because the core must
// save extra information to the stack upon exception entry. With the lazy
// stacking option enabled, the core will skip the saving of floating-point
// registers when possible, reducing average interrupt latency.
//
// @note At reset of the Cortex M4, the ASPEN and LSPEN bits are set to 1,
// enabling Lazy mode by default. Therefore this function will generally
// only have an affect when setting for full-context save (or when switching
// from full-context to lazy mode).
//
// @note See also:
// infocenter.arm.com/help/index.jsp?topic=/com.arm.doc.dai0298a/DAFGGBJD.html
//
// @note Three valid FPU context saving modes are possible.
// 1. Lazy           ASPEN=1 LSPEN=1 am_hal_sysctrl_fpu_stacking_enable(true)
//                                   and default.
// 2. Full-context   ASPEN=1 LSPEN=0 am_hal_sysctrl_fpu_stacking_enable(false)
// 3. No FPU state   ASPEN=0 LSPEN=0 am_hal_sysctrl_fpu_stacking_disable()
// 4. Invalid        ASPEN=0 LSPEN=1
//
//*****************************************************************************
void
am_hal_sysctrl_fpu_stacking_enable(bool bLazy)
{
    uint32_t ui32fpccr;

    //
    // Set the requested FPU stacking mode in ISRs.
    //
    AM_CRITICAL_BEGIN
#define SYSCTRL_FPCCR_LAZY  (FPU_FPCCR_ASPEN_Msk | FPU_FPCCR_LSPEN_Msk)
    ui32fpccr  = FPU->FPCCR;
    ui32fpccr &= ~SYSCTRL_FPCCR_LAZY;
    ui32fpccr |= (bLazy ? SYSCTRL_FPCCR_LAZY : FPU_FPCCR_ASPEN_Msk);
    FPU->FPCCR = ui32fpccr;
    AM_CRITICAL_END
}

//*****************************************************************************
//
// Disable FPU register stacking on exception entry.
//
// This function disables all stacking of floating point registers for
// interrupt handlers.  This mode should only be used when it is absolutely
// known that no FPU instructions will be executed in an ISR.
//
//*****************************************************************************
void
am_hal_sysctrl_fpu_stacking_disable(void)
{
    //
    // Completely disable FPU context save on entry to ISRs.
    //
    AM_CRITICAL_BEGIN
    FPU->FPCCR &= ~SYSCTRL_FPCCR_LAZY;
    AM_CRITICAL_END
}

//*****************************************************************************
//
// Issue a system wide reset using the AIRCR bit in the M4 system ctrl.
//
// This function issues a system wide reset (Apollo POR level reset).
//
//*****************************************************************************
void
am_hal_sysctrl_aircr_reset(void)
{
    //
    // Set the system reset bit in the AIRCR register
    //
    __NVIC_SystemReset();
}

//*****************************************************************************
//
// End Doxygen group.
//! @}
//
//*****************************************************************************
