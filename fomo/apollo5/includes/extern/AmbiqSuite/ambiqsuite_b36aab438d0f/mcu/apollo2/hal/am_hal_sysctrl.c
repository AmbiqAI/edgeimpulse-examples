//*****************************************************************************
//
//  am_hal_sysctrl.c
//! @file
//!
//! @brief Functions for interfacing with the M4F system control registers
//!
//! @addtogroup sysctrl2 System Control (SYSCTRL)
//! @ingroup apollo2hal
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
//  Local macro constants
//
//*****************************************************************************
//
// Define ZX workaround values.
// These values are defined by the factory.
//
#define COREZXVALUE         0x07
#define MEMZXVALUE          0x07

//
// Define values for g_ui32CoreBuck, which indicates which timer carries
// the signal for the CORE Buck, and which also implies that the other timer
// carries the signal for the MEM buck.
//
#define COREBUCK_TIMERA     1       // Core buck signal comes in on timer A
#define COREBUCK_TIMERB     2       // Core buck signal comes in on timer B

//
// Define the bit values for static function g_buckZX_chk;
//
#define CHKBUCKZX_BUCKS     0x01    // The bucks are enabled
#define CHKBUCKZX_REV       0x02    // This chip rev needs the workaround
#define CHKBUCKZX_TIMER     0x04    // A valid timer has been allocated
#define CHKBUCKZX_DEVEN     0x08    // Devices are powered up and enabled

#define TIMER_OFFSET        (AM_REG_CTIMER_TMR1_O - AM_REG_CTIMER_TMR0_O)

//*****************************************************************************
//
//  Prototypes
//
//*****************************************************************************
static void am_hal_sysctrl_buckA_ctimer_isr(void);
static void am_hal_sysctrl_buckB_ctimer_isr(void);

//*****************************************************************************
//
//  Globals
//
//*****************************************************************************
static volatile uint32_t g_ui32BuckTimer = 0;
static volatile uint32_t g_ui32BuckInputs = 0;
static volatile bool     g_bBuckRestoreComplete = false;
static volatile bool     g_bBuckTimed = false;
static          uint32_t g_ui32SaveCoreBuckZX, g_ui32SaveMemBuckZX;
static          uint32_t g_buckZX_chk = 0;
static volatile uint32_t g_ui32CoreBuck;

//*****************************************************************************
//
// Determine if we need to do the zero cross workaround on this device.
// Four criteria are used.  All four must be true.
//  1. Are the bucks enabled?
//  2. Is the chip rev appropriate for the workaround?
//  3. Has a timer been allocated to do the workaround?
//  4. Are certain peripherals powered up?
//
// Saves the bitmask to the global g_buckZX_chk.
// Bitmask bits are defined as: CHKBUCKZX_BUCKS, CHKBUCKZX_REV, CHKBUCKZX_TIMER.
//
// Returns true if all criteria are met, false otherwise.
// g_buckZX_chk can be probed to determine which criteria passed or failed.
//
//*****************************************************************************
static bool
buckZX_chk(void)
{
    uint32_t ui32SupplySrc;

    //
    // Is this chip rev appropriate to do the workaround?
    //
    g_buckZX_chk = AM_BFM(MCUCTRL, CHIPREV, REVMAJ) == AM_REG_MCUCTRL_CHIPREV_REVMAJ_B ?
                   CHKBUCKZX_REV : 0x0;

    //
    // Has a timer been configured to handle the workaround?
    //
    g_buckZX_chk |= ( g_ui32BuckTimer - 1 ) <= BUCK_TIMER_MAX ?
                    CHKBUCKZX_TIMER : 0x0;

    //
    // Are either or both of the bucks actually enabled?
    //
    ui32SupplySrc = AM_REG(PWRCTRL, SUPPLYSRC);

    g_buckZX_chk |= (ui32SupplySrc &
                    (AM_REG_PWRCTRL_SUPPLYSRC_COREBUCKEN_M  |
                     AM_REG_PWRCTRL_SUPPLYSRC_MEMBUCKEN_M) ) ?
                     CHKBUCKZX_BUCKS : 0x0;

    //
    // Finally, if any peripheral (other than ADC-only) is already powered up,
    //  we don't need to do the ZX workaround because in this case the bucks
    //  remain in active mode.
    // If ONLY the ADC is powered up (and no other peripherals), a case occurs
    //  which is complex to handle.  However it can also be completely handled
    //  via the ZX workaround, so that case is also checked at this point.
    // For more information on both issues see erratum ERR010 and ERR019.
//##### INTERNAL BEGIN #####
    // FUTURE FEATURE.
    // If ONLY the ADC is powered up, that is a much more complex case which
    // will eventually be handled separately. For now, it will be handled via
    // the ZX workaround.
    // Also for this initial implementation, we don't need to worry about
    // timer configurations.  We'll trigger the ZX workaround based solely
    // on device enables.
//##### INTERNAL END #####
    //
    ui32SupplySrc = AM_REG(PWRCTRL, DEVICEEN);

    g_buckZX_chk |= ( ui32SupplySrc &
            (AM_REG_PWRCTRL_DEVICEEN_PWRPDM_M       |
             AM_REG_PWRCTRL_DEVICEEN_PWRUART1_M     |
             AM_REG_PWRCTRL_DEVICEEN_PWRUART0_M     |
             AM_REG_PWRCTRL_DEVICEEN_IO_MASTER5_M   |
             AM_REG_PWRCTRL_DEVICEEN_IO_MASTER4_M   |
             AM_REG_PWRCTRL_DEVICEEN_IO_MASTER3_M   |
             AM_REG_PWRCTRL_DEVICEEN_IO_MASTER2_M   |
             AM_REG_PWRCTRL_DEVICEEN_IO_MASTER1_M   |
             AM_REG_PWRCTRL_DEVICEEN_IO_MASTER0_M   |
             AM_REG_PWRCTRL_DEVICEEN_IO_SLAVE_M) )      ?
                0x0 : CHKBUCKZX_DEVEN;

    //
    // If all 4 criteria were met, we're good to do the workaround.
    //
    return ( g_buckZX_chk ==
             (CHKBUCKZX_BUCKS | CHKBUCKZX_REV |
              CHKBUCKZX_TIMER | CHKBUCKZX_DEVEN) ) ? true : false;
}

//*****************************************************************************
//
// Set the buck zero cross settings to the values given.
//
// ui32Flags, one or more of the following:
//  SETBUCKZX_USE_PROVIDED_SETTINGS - Use the values provided in the parameters
//                                    to set the trim value(s).
//  SETBUCKZX_USE_SAVED_SETTINGS    - Use the values that were previously saved
//                                    to set the trim value(s).
//  SETBUCKZX_SAVE_CURR_SETTINGS    - Save the current trim values before
//                                    setting the new ones.
//  SETBUCKZX_RESTORE_CORE_ONLY     - Restore the Core trim and save the current
//                                    value of the core buck trim iff
//                                    SETBUCKZX_SAVE_CURR_SETTINGS is set.
//  SETBUCKZX_RESTORE_MEM_ONLY      - Restore the Mem trim and save the current
//                                    value of the mem buck trim iff
//                                    SETBUCKZX_SAVE_CURR_SETTINGS is set.
//  SETBUCKZX_RESTORE_BOTH          - Restore both buck trims and save the
//                                    current value of both iff
//                                    SETBUCKZX_SAVE_CURR_SETTINGS is set.
//
//*****************************************************************************
#define SETBUCKZX_USE_PROVIDED_SETTINGS 0x01
#define SETBUCKZX_USE_SAVED_SETTINGS    0x02
#define SETBUCKZX_SAVE_CURR_SETTINGS    0x04
#define SETBUCKZX_RESTORE_CORE_ONLY     0x10
#define SETBUCKZX_RESTORE_MEM_ONLY      0x20
#define SETBUCKZX_RESTORE_BOTH          ( SETBUCKZX_RESTORE_CORE_ONLY |     \
                                          SETBUCKZX_RESTORE_MEM_ONLY )
static void
setBuckZX(uint32_t ui32CoreBuckZX, uint32_t ui32MemBuckZX, uint32_t ui32Flags)
{
    uint32_t ui32SaveCore, ui32SaveMem, ui32NewCore, ui32NewMem;
    bool bDoRestore = false;

    //
    // Begin critical section.
    //
    AM_CRITICAL_BEGIN

    //
    // Get the current zero cross trim values.
    //
    ui32SaveCore = AM_BFR(MCUCTRL, BUCK3, COREBUCKZXTRIM);
    ui32SaveMem  = AM_BFR(MCUCTRL, BUCK3, MEMBUCKZXTRIM);

    //
    // Determine which values will be restored.
    //
    if ( ui32Flags & SETBUCKZX_USE_SAVED_SETTINGS )
    {
        //
        // Use saved settings
        //
        ui32NewCore = g_ui32SaveCoreBuckZX;
        ui32NewMem  = g_ui32SaveMemBuckZX;
        bDoRestore = true;
    }
    else if ( ui32Flags & SETBUCKZX_USE_PROVIDED_SETTINGS )
    {
        //
        // Use settings provided in the call parameters
        //
        ui32NewCore = ui32CoreBuckZX;
        ui32NewMem  = ui32MemBuckZX;
        bDoRestore = true;
    }

    //
    // Restore the buck Core and Mem trim registers.
    //
    if ( bDoRestore )
    {
        if ( ui32Flags & SETBUCKZX_RESTORE_CORE_ONLY )
        {
            AM_BFW(MCUCTRL, BUCK3, COREBUCKZXTRIM, ui32NewCore);
        }

        if ( ui32Flags & SETBUCKZX_RESTORE_MEM_ONLY )
        {
            AM_BFW(MCUCTRL, BUCK3, MEMBUCKZXTRIM,  ui32NewMem);
        }
    }

    if ( ui32Flags & SETBUCKZX_SAVE_CURR_SETTINGS )
    {
        //
        // Save off the zero cross values as read on entry to the function.
        //
        if ( ui32Flags & SETBUCKZX_RESTORE_CORE_ONLY )
        {
            g_ui32SaveCoreBuckZX = ui32SaveCore;
        }

        if ( ui32Flags & SETBUCKZX_RESTORE_MEM_ONLY )
        {
            g_ui32SaveMemBuckZX  = ui32SaveMem;
        }
    }

    //
    // Done with critical section.
    //
    AM_CRITICAL_END

} // setBuckZX()

//*****************************************************************************
//
//! @brief Place the core into sleep or deepsleep.
//!
//! @param bSleepDeep - False for Normal or True Deep sleep.
//!
//! This function puts the MCU to sleep or deepsleep depending on bSleepDeep.
//!
//! Valid values for bSleepDeep are:
//!
//!     AM_HAL_SYSCTRL_SLEEP_NORMAL
//!     AM_HAL_SYSCTRL_SLEEP_DEEP
//!
//! @return None.
//
//*****************************************************************************
void
am_hal_sysctrl_sleep(bool bSleepDeep)
{
    uint32_t ui32Critical;
    bool bBuckZX_chk;
    volatile uint32_t ui32BuckTimer;

    //
    // Disable interrupts and save the previous interrupt state.
    //
    ui32Critical = am_hal_interrupt_master_disable();

    //
    // If the user selected DEEPSLEEP and the TPIU is off, attempt to enter
    // DEEP SLEEP.
    //
    if ((bSleepDeep == AM_HAL_SYSCTRL_SLEEP_DEEP) &&
        (AM_BFM(MCUCTRL, TPIUCTRL, ENABLE) == AM_REG_MCUCTRL_TPIUCTRL_ENABLE_DIS))
    {
//##### INTERNAL BEGIN #####
        //
        // Note - this sequence is now strictly post RevA silicon.
        // The isRevA power sequence was removed on 6/22/17.
        // Note - For Apollo2 >= B0, the bucks should not be DISABLED or
        //        ENABLED, the hardware handles that.
        //
//##### INTERNAL END #####
        //
        // Prepare the core for deepsleep (write 1 to the DEEPSLEEP bit).
        //
        AM_BFW(SYSCTRL, SCR, SLEEPDEEP, 1);

        //
        // Check if special buck handling is needed
        //
        bBuckZX_chk = buckZX_chk();

        if ( bBuckZX_chk )
        {
            ui32BuckTimer = g_ui32BuckTimer - 1;

            am_hal_ctimer_int_disable((AM_HAL_CTIMER_INT_TIMERA0C0 |
                                       AM_HAL_CTIMER_INT_TIMERB0C0 ) <<
                                       (ui32BuckTimer * 2));

            //
            // Before going to sleep, clear the buck timers.
            // This will also handle the case where we're going back to
            // sleep before the buck sequence has even completed.
            //
            am_hal_ctimer_clear(ui32BuckTimer, AM_HAL_CTIMER_BOTH);

            //
            // Enable the interrupts for timers A and B
            //
            am_hal_ctimer_int_enable( (AM_HAL_CTIMER_INT_TIMERA0C0 |
                                       AM_HAL_CTIMER_INT_TIMERB0C0 ) <<
                                       (ui32BuckTimer * 2) );

// #### INTERNAL BEGIN ####
            //
            // Normally we would simply set the new trims here, let the Apollo2
            // handle the buck to LDO transition when going to sleep, and awake
            // with the new trims. However, that cannot safely be done without
            // making sure we set the trims between pulses, or without first
            // powering down the bucks. So instead we'll power down the bucks
            // now, and after waking we'll set the new trims and then power the
            // bucks back up.
            //
// #### INTERNAL END ####
            //
            // Disable bucks before going to sleep.
            //
            am_hal_pwrctrl_bucks_disable();
        }
// #### INTERNAL BEGIN ####
// POSSIBLE FUTURE FEATURE
        else if ( g_bBuckTimed )
        {
            //
            // Handle the buck restore in a timed fashion.
            // After the required period of time, the user will call
            // am_hal_sysctrl_buck_restore() to restore the settings.
            //
            setBuckZX(COREZXVALUE, MEMZXVALUE,
                      SETBUCKZX_USE_PROVIDED_SETTINGS   |
                      SETBUCKZX_RESTORE_BOTH );
        }
// #### INTERNAL END ####

        //
        // Before executing WFI, flush any buffered core and peripheral writes.
        //
        AM_ASM_DSB

        //
        // Execute the sleep instruction.
        //
        AM_ASM_WFI;

        //
        // Upon wake, execute the Instruction Sync Barrier instruction.
        //
        AM_ASM_ISB

        //
        // Return from sleep
        //
        if ( bBuckZX_chk )
        {
            //
            // Adjust the core and mem trims
            //
            setBuckZX(COREZXVALUE, MEMZXVALUE,
                      SETBUCKZX_USE_PROVIDED_SETTINGS   |
                      SETBUCKZX_RESTORE_BOTH );

            //
            // Delay for 2us before enabling bucks.
            //
            am_hal_flash_delay( FLASH_CYCLES_US(2) );

            //
            // Turn on the bucks
            //
            am_hal_pwrctrl_bucks_enable();

            //
            // Get the actual timer number
            //
            ui32BuckTimer = g_ui32BuckTimer - 1;

            //
            // Initialize the complete flag
            //
            g_bBuckRestoreComplete = false;

            //
            // Initialize the input flags
            //
            g_ui32BuckInputs = 0;

// #### INTERNAL BEGIN ####
            //
            // Especially at cold temperatures, the Apollo2 bucks start up in a
            // somewhat unpredicable state where multiple TOn pulses can, and
            // often do, occur. To avoid problems with detecting the pulses
            // under those conditions, we'll give it a little time to stabilize
            // before we start the timer that will detect them.
            //
            // As of 7/20/17, a 5us delay is included as that was used in the
            // testing. However, a much shorter, or even no, delay may be
            // adequate.  On 7/20/17, a delay of 1, am_hal_flash_delay(1), was
            // briefly validated at -20C and ran just fine for at least 2 hours.
            //
// #### INTERNAL END ####
            //
            // Delay for 5us to make sure we're receiving clean buck signals.
            //
            am_hal_flash_delay( FLASH_CYCLES_US(5) );

            //
            // Start timers (set the enable bit, clear the clear bit)
            //
            am_hal_ctimer_start(ui32BuckTimer, AM_HAL_CTIMER_BOTH);
        }
        else
        {
            //
            // Since we're not doing anything, we're done, so set the done flag.
            //
            g_bBuckRestoreComplete = true;
        }
    }
    else
    {
        //
        // Prepare the core for normal sleep (write 0 to the DEEPSLEEP bit).
        //
        AM_BFW(SYSCTRL, SCR, SLEEPDEEP, 0);

        //
        // Before executing WFI, flush any buffered core and peripheral writes.
        //
        AM_ASM_DSB

        //
        // Go to sleep.
        //
        AM_ASM_WFI;

        //
        // Upon wake, execute the Instruction Sync Barrier instruction.
        //
        AM_ASM_ISB
    }

    //
    // Restore the interrupt state.
    //
    am_hal_interrupt_master_set(ui32Critical);

} // am_hal_sysctrl_sleep()

//*****************************************************************************
//
//! @brief Enable the floating point module.
//!
//! Call this function to enable the ARM hardware floating point module.
//!
//! @return None.
//
//*****************************************************************************
void
am_hal_sysctrl_fpu_enable(void)
{
    //
    // Enable access to the FPU in both privileged and user modes.
    // NOTE: Write 0s to all reserved fields in this register.
    //
    AM_REG(SYSCTRL, CPACR) = (AM_REG_SYSCTRL_CPACR_CP11(0x3) |
                             AM_REG_SYSCTRL_CPACR_CP10(0x3));

} // am_hal_sysctrl_fpu_enable()

//*****************************************************************************
//
//! @brief Disable the floating point module.
//!
//! Call this function to disable the ARM hardware floating point module.
//!
//! @return None.
//
//*****************************************************************************
void
am_hal_sysctrl_fpu_disable(void)
{
    //
    // Disable access to the FPU in both privileged and user modes.
    // NOTE: Write 0s to all reserved fields in this register.
    //
    AM_REG(SYSCTRL, CPACR) = 0x00000000                     &
                          ~(AM_REG_SYSCTRL_CPACR_CP11(0x3) |
                            AM_REG_SYSCTRL_CPACR_CP10(0x3));
} // am_hal_sysctrl_fpu_disable()

//*****************************************************************************
//
//! @brief Enable stacking of FPU registers on exception entry.
//!
//! @param bLazy - Set to "true" to enable "lazy stacking".
//!
//! This function allows the core to save floating-point information to the
//! stack on exception entry. Setting the bLazy option enables "lazy stacking"
//! for interrupt handlers.  Normally, mixing floating-point code and interrupt
//! driven routines causes increased interrupt latency, because the core must
//! save extra information to the stack upon exception entry. With the lazy
//! stacking option enabled, the core will skip the saving of floating-point
//! registers when possible, reducing average interrupt latency.
//!
//! @note At reset of the Cortex M4, the ASPEN and LSPEN bits are set to 1,
//! enabling Lazy mode by default. Therefore this function will generally
//! only have an affect when setting for full-context save (or when switching
//! from full-context to lazy mode).
//!
//! @note See also:
//! infocenter.arm.com/help/index.jsp?topic=/com.arm.doc.dai0298a/DAFGGBJD.html
//!
//! @note Three valid FPU context saving modes are possible.
//! 1. Lazy           ASPEN=1 LSPEN=1 am_hal_sysctrl_fpu_stacking_enable(true)
//!                                   and default.
//! 2. Full-context   ASPEN=1 LSPEN=0 am_hal_sysctrl_fpu_stacking_enable(false)
//! 3. No FPU state   ASPEN=0 LSPEN=0 am_hal_sysctrl_fpu_stacking_disable()
//! 4. Invalid        ASPEN=0 LSPEN=1
//!
//! @return None.
//
//*****************************************************************************
void
am_hal_sysctrl_fpu_stacking_enable(bool bLazy)
{
#define SYSCTRL_FPCCR_LAZY  (AM_REG_SYSCTRL_FPCCR_ASPEN_M | AM_REG_SYSCTRL_FPCCR_LSPEN_M)

    uint32_t ui32fpccr;

    //
    // Set the requested FPU stacking mode in ISRs.
    //
    AM_CRITICAL_BEGIN
    ui32fpccr  = AM_REG(SYSCTRL, FPCCR);
    ui32fpccr &= ~(SYSCTRL_FPCCR_LAZY);
    ui32fpccr |= (bLazy ? SYSCTRL_FPCCR_LAZY : AM_REG_SYSCTRL_FPCCR_ASPEN_M);
    AM_REG(SYSCTRL, FPCCR) = ui32fpccr;
    AM_CRITICAL_END

} // am_hal_sysctrl_fpu_stacking_enable()

//*****************************************************************************
//
//! @brief Disable FPU register stacking on exception entry.
//!
//! This function disables all stacking of floating point registers for
//! interrupt handlers.  This mode should only be used when it is absolutely
//! known that no FPU instructions will be executed in an ISR.
//!
//! @return None.
//
//*****************************************************************************
void
am_hal_sysctrl_fpu_stacking_disable(void)
{
    //
    // Completely disable FPU context save on entry to ISRs.
    //
    AM_CRITICAL_BEGIN
    AM_REG(SYSCTRL, FPCCR) &= ~SYSCTRL_FPCCR_LAZY;
    AM_CRITICAL_END

} // am_hal_sysctrl_fpu_stacking_disable()

//*****************************************************************************
//
//! @brief Issue a system wide reset using the AIRCR bit in the M4 system ctrl.
//!
//! This function issues a system wide reset (Apollo POR level reset).
//!
//! @return None.
//
//*****************************************************************************
void
am_hal_sysctrl_aircr_reset(void)
{
    //
    // Set the system reset bit in the AIRCR register
    //
    AM_REG(SYSCTRL, AIRCR) = AM_REG_SYSCTRL_AIRCR_VECTKEY(0x5FA) |
                             AM_REG_SYSCTRL_AIRCR_SYSRESETREQ(1);

} // am_hal_sysctrl_aircr_reset()

//*****************************************************************************
//
//! @brief Buck CTimer ISR initializer.
//!
//! @param ui32BuckTimerNumber - Timer number to be used for handling the buck.
//!                              Must be 0-3.
//!
//! If called with an invalid timer (that is, not 0 - 3, or greater than
//! BUCK_TIMER_MAX), then the workaround will not be enabled.
//!
//! Instead, the bucks will be initialized with a value that will avoid the
//! issues described in the Errata (ERR019).  However, this will cause a
//! less efficient energy usage condtion.
//!
//! @return 0.
//
//*****************************************************************************
uint32_t
am_hal_sysctrl_buck_ctimer_isr_init(uint32_t ui32BuckTimerNumber)
{
    uint32_t ui32RetVal = 0;

    //
    // Initialize the input flags
    //
    g_ui32BuckInputs = 0;

    //
    // Initialize operation complete flag
    //
    g_bBuckRestoreComplete = false;

    //
    // Initialize to assume there is no valid timer.
    //
    g_ui32BuckTimer = 0;

// #### INTERNAL BEGIN ####
// POSSIBLE FUTURE FEATURE!
    //
    // Initialize the Fixed Timed flag
    //
    g_bBuckTimed = false;
// #### INTERNAL END ####
    if ( ui32BuckTimerNumber > BUCK_TIMER_MAX )
    {
        if ( ( ui32BuckTimerNumber & 0xFFFF0000 ) ==
             AM_HAL_SYSCTRL_BUCK_CTIMER_ZX_CONSTANT )
        {
            //
            // The caller is asking for the hard option, which changes the
            //  settings to the more noise-immune, if less efficient, settings.
            // While we're at it, go ahead and save off the current settings.
            //
            if ( (ui32BuckTimerNumber & 0x0000FFFF) == 0 )
            {
                setBuckZX(COREZXVALUE, MEMZXVALUE,
                          SETBUCKZX_USE_PROVIDED_SETTINGS   |
                          SETBUCKZX_SAVE_CURR_SETTINGS      |
                          SETBUCKZX_RESTORE_BOTH );
            }
            else
            {
                uint32_t ui32Core, ui32Mem;

                //
                // Use the setting provided in the parameter.
                //
                ui32Core = (((ui32BuckTimerNumber & 0x001F) >> 0) - 1) & 0xF;
                ui32Mem  = (((ui32BuckTimerNumber & 0x1F00) >> 8) - 1) & 0xF;

                setBuckZX(ui32Core, ui32Mem,
                          SETBUCKZX_USE_PROVIDED_SETTINGS   |
                          SETBUCKZX_SAVE_CURR_SETTINGS      |
                          SETBUCKZX_RESTORE_BOTH );
            }
        }
// #### INTERNAL BEGIN ####
// POSSIBLE FUTURE FEATURE!
#if 0
        // TBD: Need to identify a method of timing that does not interfere with
        // an existing timer.
        // The best solution may be to require the user to handle the timing
        // After waking from sleep, the user would wait the required number
        // of microseconds before calling the function that handles the update
        // of the buck lower-power settings.
        // This function will therefore return a value containing the number
        // of microseconds required to wait before calling the function.

        else if ( ( ui32BuckTimerNumber & 0xFFFF0000 ) ==
                   AM_HAL_SYSCTRL_BUCK_CTIMER_ZX_TIMED )
        {
            //
            // Set the "Timed" flag
            //
            g_bBuckTimed = true;

            //
            // The user has asked for a timed ZX update.
            // The user will therefore be required to wait the following number
            //  of usecs, and then call am_hal_sysctrl_buck_restore().
            //
            ui32RetVal = 300;
        }
#endif // 0
// #### INTERNAL END ####
    }
    else
    {
        //
        // Save off the current trim settings (but don't change any settings).
        //
        setBuckZX(0, 0, SETBUCKZX_SAVE_CURR_SETTINGS | SETBUCKZX_RESTORE_BOTH);

        //
        // The timer number will be maintained as (n + 1).  Therefore, a value
        // of 0 saved in the global is an invalid timer.  1=timer0, 2=timer1...
        //
        g_ui32BuckTimer = ui32BuckTimerNumber + 1;

        //
        // Register the timer ISRs
        //
        am_hal_ctimer_int_register( AM_HAL_CTIMER_INT_TIMERA0C0 <<
                                    (ui32BuckTimerNumber * 2),
                                     am_hal_sysctrl_buckA_ctimer_isr );

        am_hal_ctimer_int_register( AM_HAL_CTIMER_INT_TIMERB0C0 <<
                                    (ui32BuckTimerNumber * 2),
                                     am_hal_sysctrl_buckB_ctimer_isr );

        //
        // Determine which timer input (A or B) is core buck and which is mem
        // buck based on the timer number.
        //  For CTIMER 0 & 1: Timer A is mem  buck, Timer B is core buck
        //  For CTIMER 2 & 3: Timer A is core buck, Timer B is mem  buck
        //
        if ( (ui32BuckTimerNumber == 0)  ||  (ui32BuckTimerNumber == 1) )
        {
            //
            // Indicate that TimerB is core buck.
            //
            g_ui32CoreBuck = COREBUCK_TIMERB;
        }
        else
        {
            //
            // Indicate that TimerA is core buck
            //
            g_ui32CoreBuck = COREBUCK_TIMERA;
        }

        //
        // Clear and configure the timers
        //
        am_hal_ctimer_clear(ui32BuckTimerNumber, AM_HAL_CTIMER_BOTH);

        //
        // Find the correct register to write based on the timer number.
        //
        volatile uint32_t *pui32CmprReg, *pui32ConfigReg;
        pui32CmprReg   = (uint32_t *)(AM_REG_CTIMERn(0) + AM_REG_CTIMER_CMPRA0_O +
                                      (ui32BuckTimerNumber * TIMER_OFFSET));
        pui32ConfigReg = (uint32_t *)(AM_REG_CTIMERn(0) + AM_REG_CTIMER_CTRL0_O +
                                      (ui32BuckTimerNumber * TIMER_OFFSET));
        //
        // Configure the timer.
        //
        AM_REGVAL(pui32ConfigReg) =
            AM_REG_CTIMER_CTRL0_TMRB0IE0(1)     |   // Interrupt on CMPR0 only
            AM_REG_CTIMER_CTRL0_TMRB0FN(4)      |   // Func 4=Continuous
            AM_REG_CTIMER_CTRL0_TMRB0CLK(0x10)  |   // Clock=BUCKB
            AM_REG_CTIMER_CTRL0_TMRA0IE0(1)     |   // Interrupt on CMPR0 only
            AM_REG_CTIMER_CTRL0_TMRA0FN(4)      |   // Func 4=Continuous
            AM_REG_CTIMER_CTRL0_TMRA0CLK(0x10);     // Clock=BUCKA

        #define     TIMER_BUCK_PULSES  10

        AM_REGVAL(pui32CmprReg) = (0 << 16) | TIMER_BUCK_PULSES;    // Timer A CMPR0
        pui32CmprReg++;
        AM_REGVAL(pui32CmprReg) = (0 << 16) | TIMER_BUCK_PULSES;    // Timer B CMPR0

        //
        // Enable the timer interrupt in the NVIC.
        //
        am_hal_interrupt_enable(AM_HAL_INTERRUPT_CTIMER);
    }

    return ui32RetVal;

} // am_hal_sysctrl_buck_ctimer_isr_init()

//*****************************************************************************
//
// Get buck update complete status.
//
//*****************************************************************************
bool
am_hal_sysctrl_buck_update_complete(void)
{
    return g_bBuckRestoreComplete;
} // am_hal_sysctrl_buck_update_complete()

// #### INTERNAL BEGIN ####
// POSSIBLE FUTURE FEATURE
//*****************************************************************************
//
// Set the buck update complete status.
//
//*****************************************************************************
void
am_hal_sysctrl_buck_restore(void)
{
    //
    // Adjust the Core and Memory buck zero cross back
    // to its original value for optimal power.
    //
    setBuckZX(0, 0, SETBUCKZX_USE_SAVED_SETTINGS    |
                    SETBUCKZX_RESTORE_BOTH );

    //
    // Mark as complete.
    //
    g_bBuckRestoreComplete = true;
}
// #### INTERNAL END ####

//*****************************************************************************
//
// Perform a short delay.
//
//*****************************************************************************
static void
short_delay(volatile register uint32_t ui32iters)
{
    //
    // Note: for a 1us delay, we need 48 cycles at 48MHz.
    // When optimized, we would expect this function to be inlined by
    // the compiler and the loop itself to end up with about 6 instructions.
    // Therefore iters should be about 8 for 1us.
    //   6=0.52us, 10=0.76us, 13=0.9us, 15=1us, 20=1.36us
    //
    while ( ui32iters-- );
} // short_delay()

//*****************************************************************************
//
// Buck CTIMER ISR (for handling buck switching via TimerA).
//
// Note: This handler assumes that the interrupt is cleared in am_ctimer_isr().
//
//*****************************************************************************
static void
am_hal_sysctrl_buckA_ctimer_isr(void)
{
    uint32_t ui32CTaddr, ui32InitVal;

    //
    // Begin critical section.
    //
    AM_CRITICAL_BEGIN

    //
    // Disable Timer A interrupts
    //
    am_hal_ctimer_int_disable( AM_HAL_CTIMER_INT_TIMERA0C0 <<
                               ((g_ui32BuckTimer - 1) * 2));

    //
    // Determine which buck (core or mem) needs to be updated.
    //
    if ( g_ui32CoreBuck == COREBUCK_TIMERA )
    {
//##### INTERNAL BEGIN #####
#define ISHAN_GPIO_SIGNAL  0
#if ISHAN_GPIO_SIGNAL
        am_hal_gpio_out_bit_set(5);
#endif // ISHAN_GPIO_SIGNAL
//##### INTERNAL END #####
        //
        // Timer A buck signal is the CORE buck.
        // For the core buck, timing is critical as we cannot change the trims
        // during a buck event.
        // In order to assure optimal timing while the trims are changed, the
        // beginning of the next pulse must be determined by polling.
        //
        ui32CTaddr  = AM_REGADDRn(CTIMER, (g_ui32BuckTimer - 1), TMR0);
        ui32InitVal = AM_REGVAL(ui32CTaddr) & AM_HAL_CTIMER_TIMERA;

        //
        // We can safely assume that we're nowhere near a rollover since the
        // interrupt that got us here was based on a single digit TMR value.
        // Wait for the next buck pulse.
        //
        while ( (AM_REGVAL(ui32CTaddr) & AM_HAL_CTIMER_TIMERA) <= ui32InitVal ) {};

        //
        // The following delay ensures that we're far enough away from the
        // pulse to avoid any problems. It must be inside the critical section.
        //
//##### INTERNAL BEGIN #####
        // 2/7/20 update. A change in the algorithm to poll for the next core
        // pulse after entering the ISR necessitated further validation. During
        // that effort, it was determined by validation that a delay of 0.75us
        // after detecting the core pulse was sufficient before changing the ZX.
        // Further, no delay is needed for the memory pulse because of the ~2us
        // or so of overhead involved with getting into the ISR, plus the fact
        // that the mem pulses are significantly longer than the core pulses.
        // Therefore the algorithm now changes the MEM ZX immediately on entry.
        //
        // NOTE: A 2us delay was validated the week of 7/17/17 using the Huawei
        // board at -20C and even at -40C.  Hopefully a shorter delay time can be
        // validated in the near future (e.g. 1us seems to be enough delay, but has
        // not been validated nor approved by design).
        //
//##### INTERNAL END #####
        short_delay(10);    // 10 = about 750ns delay

        //
        // Restore the core buck.
        //
        setBuckZX(0, 0, SETBUCKZX_RESTORE_CORE_ONLY |
                        SETBUCKZX_USE_SAVED_SETTINGS );
//##### INTERNAL BEGIN #####
#if ISHAN_GPIO_SIGNAL
        am_hal_gpio_out_bit_clear(5);
#endif // ISHAN_GPIO_SIGNAL
//##### INTERNAL END #####
    }
    else
    {
        //
        // Timer A buck signal is the MEM buck.
        // Restore the mem buck.
        //
//##### INTERNAL BEGIN #####
#if ISHAN_GPIO_SIGNAL
        am_hal_gpio_out_bit_set(5);
#endif // ISHAN_GPIO_SIGNAL
//##### INTERNAL END #####
        setBuckZX(0, 0, SETBUCKZX_RESTORE_MEM_ONLY  |
                        SETBUCKZX_USE_SAVED_SETTINGS );
//##### INTERNAL BEGIN #####
#if ISHAN_GPIO_SIGNAL
        am_hal_gpio_out_bit_clear(5);
#endif // ISHAN_GPIO_SIGNAL
//##### INTERNAL END #####
    }

    g_ui32BuckInputs |= 0x1;

    if ( g_ui32BuckInputs == 0x3 )
    {
        g_bBuckRestoreComplete = true;
        g_ui32BuckInputs = 0;
    }

    //
    // End critical section.
    //
    AM_CRITICAL_END

} // am_hal_sysctrl_buckA_ctimer_isr

//*****************************************************************************
//
// Buck CTIMER ISR (for handling buck switching via TimerB).
//
// Note: This handler assumes that the interrupt is cleared in am_ctimer_isr().
//
//*****************************************************************************
static void
am_hal_sysctrl_buckB_ctimer_isr(void)
{
    uint32_t ui32CTaddr, ui32InitVal;

    //
    // Begin critical section.
    //
    AM_CRITICAL_BEGIN

    //
    // Disable Timer B interrupts
    //
    am_hal_ctimer_int_disable( AM_HAL_CTIMER_INT_TIMERB0C0 <<
                               ((g_ui32BuckTimer - 1) * 2));

    //
    // Determine which buck (core or mem) needs to be updated.
    //
    if ( g_ui32CoreBuck == COREBUCK_TIMERB )
    {
//##### INTERNAL BEGIN #####
#if ISHAN_GPIO_SIGNAL
        am_hal_gpio_out_bit_set(5);
#endif // ISHAN_GPIO_SIGNAL
//##### INTERNAL END #####
        //
        // Timer B buck signal is the CORE buck.
        // For the core buck, timing is critical as we cannot change the trims
        // during a buck event.
        // In order to assure optimal timing while the trims are changed, the
        // beginning of the next pulse must be determined by polling.
        //
        ui32CTaddr  = AM_REGADDRn(CTIMER, (g_ui32BuckTimer - 1), TMR0);
        ui32InitVal = AM_REGVAL(ui32CTaddr) & AM_HAL_CTIMER_TIMERB;

        //
        // We can safely assume that we're nowhere near a rollover since the
        // interrupt that got us here was based on a single digit TMR value.
        // Wait for the next buck pulse.
        //
        while ( (AM_REGVAL(ui32CTaddr) & AM_HAL_CTIMER_TIMERB) <= ui32InitVal ) {};

        //
        // The following delay ensures that we're far enough away from the
        // pulse to avoid any problems. It must be inside the critical section.
        //
//##### INTERNAL BEGIN #####
        // 2/7/20 update. A change in the algorithm to poll for the next core
        // pulse after entering the ISR necessitated further validation. During
        // that effort, it was determined by validation that a delay of 0.75us
        // after detecting the core pulse was sufficient before changing the ZX.
        // Further, no delay is needed for the memory pulse because of the ~2us
        // or so of overhead involved with getting into the ISR, plus the fact
        // that the mem pulses are significantly longer than the core pulses.
        // Therefore the algorithm now changes the MEM ZX immediately on entry.
        //
        // NOTE: A 2us delay was validated the week of 7/17/17 using the Huawei
        // board at -20C and even at -40C.  Hopefully a shorter delay time can be
        // validated in the near future (e.g. 1us seems to be enough delay, but has
        // not been validated nor approved by design).
        //
//##### INTERNAL END #####
        short_delay(10);    // 10 = about 750ns delay

        //
        // Restore the core buck.
        //
        setBuckZX(0, 0, SETBUCKZX_RESTORE_CORE_ONLY |
                        SETBUCKZX_USE_SAVED_SETTINGS );
//##### INTERNAL BEGIN #####
#if ISHAN_GPIO_SIGNAL
        am_hal_gpio_out_bit_clear(5);
#endif // ISHAN_GPIO_SIGNAL
//##### INTERNAL END #####
    }
    else
    {
        //
        // Timer B buck signal is the MEM buck.
        // Restore the mem buck.
        //
//##### INTERNAL BEGIN #####
#if ISHAN_GPIO_SIGNAL
        am_hal_gpio_out_bit_set(5);
#endif // ISHAN_GPIO_SIGNAL
//##### INTERNAL END #####
        setBuckZX(0, 0, SETBUCKZX_RESTORE_MEM_ONLY  |
                        SETBUCKZX_USE_SAVED_SETTINGS );
//##### INTERNAL BEGIN #####
#if ISHAN_GPIO_SIGNAL
        am_hal_gpio_out_bit_clear(5);
#endif // ISHAN_GPIO_SIGNAL
//##### INTERNAL END #####
    }

    g_ui32BuckInputs |= 0x2;

    if ( g_ui32BuckInputs == 0x3 )
    {
        g_bBuckRestoreComplete = true;
        g_ui32BuckInputs = 0;
    }

    //
    // End critical section.
    //
    AM_CRITICAL_END

} // am_hal_sysctrl_buckB_ctimer_isr

//*****************************************************************************
//
// End Doxygen group.
//! @}
//
//*****************************************************************************
