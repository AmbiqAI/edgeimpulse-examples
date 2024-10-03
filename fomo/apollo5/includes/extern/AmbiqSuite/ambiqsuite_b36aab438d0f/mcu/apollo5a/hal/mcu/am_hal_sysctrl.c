//*****************************************************************************
//
//! @file am_hal_sysctrl.c
//!
//! @brief Functions for interfacing with the M4F system control registers
//!
//! @addtogroup sysctrl5 SYSCTRL - System Control
//! @ingroup apollo5a_hal
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
//  Globals
//
//*****************************************************************************
//
// Extern some variables needed in this module.
//
extern uint32_t g_ui32origSimobuckVDDStrim;

// #### INTERNAL BEGIN ####
#if 0
#if AM_HAL_PWRCTL_SET_CORELDO_MEMLDO_IN_PARALLEL
extern void coreldo_enable(void);
extern void coreldo_disable(void);
extern void memldo_enable(void);
extern void memldo_disable(void);
#endif
extern void simobuck_ldos_force_active(void);
extern void simobuck_ldos_force_disable(void);
#endif
// #### INTERNAL END ####

extern void buck_ldo_update_override(bool bEnable);

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
    bool bBuckIntoLPinDS = false, bSimobuckAct = false, bIsHPMode = false;
    bool bMSPI0Enabled = false, bMSPI1Enabled = false, bMSPI2Enabled = false, bMSPI3Enabled = false;
    bool bGFXEnabled = false, bDISPEnabled = false;
    bool bCSP = ((MCUCTRL->CHIPPN & MCUCTRL_CHIPPN_PARTNUM_PKG_M) == (0x3 << 6));
    bool bInfo1OTP = MCUCTRL->SHADOWVALID_b.INFO1SELOTP;
    uint32_t trimRev = bInfo1OTP ? AM_REGVAL(AM_REG_OTP_INFO1_TRIM_REV_ADDR) : AM_REGVAL(AM_REG_INFO1_TRIM_REV_ADDR);

    //
    // Disable interrupts and save the previous interrupt state.
    //
    AM_CRITICAL_BEGIN

    //
    // Get current mode.
    //
    bSimobuckAct = ( PWRCTRL->VRSTATUS_b.SIMOBUCKST == PWRCTRL_VRSTATUS_SIMOBUCKST_ACT );

    //
    // If the user selected DEEPSLEEP and the TPIU is off, attempt to enter
    // DEEP SLEEP.
    // CPU cannot go to deepsleep if either OTP or ROM is still powered on
    // #### INTERNAL BEGIN ####
    // CAB-926
    // #### INTERNAL END ####
    //
    if ( (bSleepDeep == AM_HAL_SYSCTRL_SLEEP_DEEP) &&
         (!PWRCTRL->MEMPWRSTATUS_b.PWRSTOTP && !PWRCTRL->MEMPWRSTATUS_b.PWRSTROM) &&
         (MCUCTRL->DBGCTRL_b.DBGTPIUENABLE == MCUCTRL_DBGCTRL_DBGTPIUENABLE_DIS) )
    {

// #### INTERNAL BEGIN ####
        //
        // This is a quick workaround code for CYSV-139.
        // 1. If all MSPI off - go to deepslepp
        // 2. If any MSPI is ON, and if both GPU and DISP is off - just do normal sleep, otherwise deepsleep
        //
// #### INTERNAL END ####

        //
        // Checking whether MSPI, GFX and DISP are enabled when entering sleep mode
        //
        am_hal_pwrctrl_periph_enabled(AM_HAL_PWRCTRL_PERIPH_MSPI0, &bMSPI0Enabled);
        am_hal_pwrctrl_periph_enabled(AM_HAL_PWRCTRL_PERIPH_MSPI1, &bMSPI1Enabled);
        am_hal_pwrctrl_periph_enabled(AM_HAL_PWRCTRL_PERIPH_MSPI2, &bMSPI2Enabled);
        am_hal_pwrctrl_periph_enabled(AM_HAL_PWRCTRL_PERIPH_MSPI3, &bMSPI3Enabled);
        am_hal_pwrctrl_periph_enabled(AM_HAL_PWRCTRL_PERIPH_GFX, &bGFXEnabled);
        am_hal_pwrctrl_periph_enabled(AM_HAL_PWRCTRL_PERIPH_DISP, &bDISPEnabled);

        //
        // If any MSPI is ON, and if both GPU and DISP is off - just do normal sleep, otherwise deepsleep
        //
        if ((bMSPI0Enabled || bMSPI1Enabled || bMSPI2Enabled || bMSPI3Enabled) && (!bGFXEnabled) && (!bDISPEnabled))
        {
            //
            // Prepare the core for normal sleep (write 0 to the DEEPSLEEP bit).
            //
            SCB->SCR &= ~_VAL2FLD(SCB_SCR_SLEEPDEEP, 1);
        }
        else
        {
            //
            // If in HP mode, need to switch back to LP mode before entering deepsleep.
            //
            if (PWRCTRL->MCUPERFREQ_b.MCUPERFSTATUS == AM_HAL_PWRCTRL_MCU_MODE_HIGH_PERFORMANCE)
            {
                am_hal_pwrctrl_mcu_mode_select(AM_HAL_PWRCTRL_MCU_MODE_LOW_POWER);
                bIsHPMode = true;
            }

            //
            // Check if SIMOBUCK needs to stay in Active mode in DeepSleep
            // For CSP parts with TRIM_REV 8 we need to keep SIMOBUCK in Active mode because of defective CSP RDL
            //
            if ( bSimobuckAct )
            {
                //
                // Check if SIMOBUCK would go into LP mode in DeepSleep
                //
                if ( !(PWRCTRL->AUDSSPWRSTATUS &
                        (PWRCTRL_AUDSSPWRSTATUS_PWRSTAUDADC_Msk |
                        PWRCTRL_AUDSSPWRSTATUS_PWRSTI2S1_Msk   |
                        PWRCTRL_AUDSSPWRSTATUS_PWRSTI2S0_Msk   |
                        PWRCTRL_AUDSSPWRSTATUS_PWRSTPDM0_Msk   |
                        PWRCTRL_AUDSSPWRSTATUS_PWRSTAUDPB_Msk  |
                        PWRCTRL_AUDSSPWRSTATUS_PWRSTAUDREC_Msk)) &&
                    !(PWRCTRL->DEVPWRSTATUS &
                        (PWRCTRL_DEVPWRSTATUS_PWRSTDBG_Msk      |
                        PWRCTRL_DEVPWRSTATUS_PWRSTUSBPHY_Msk   |
                        PWRCTRL_DEVPWRSTATUS_PWRSTUSB_Msk      |
                        PWRCTRL_DEVPWRSTATUS_PWRSTSDIO1_Msk    |
                        PWRCTRL_DEVPWRSTATUS_PWRSTSDIO0_Msk    |
                        PWRCTRL_DEVPWRSTATUS_PWRSTCRYPTO_Msk   |
                        PWRCTRL_DEVPWRSTATUS_PWRSTDISPPHY_Msk  |
                        PWRCTRL_DEVPWRSTATUS_PWRSTDISP_Msk     |
                        PWRCTRL_DEVPWRSTATUS_PWRSTGFX_Msk      |
                        PWRCTRL_DEVPWRSTATUS_PWRSTMSPI3_Msk    |
                        PWRCTRL_DEVPWRSTATUS_PWRSTMSPI2_Msk    |
                        PWRCTRL_DEVPWRSTATUS_PWRSTMSPI1_Msk    |
                        PWRCTRL_DEVPWRSTATUS_PWRSTMSPI0_Msk    |
                        PWRCTRL_DEVPWRSTATUS_PWRSTADC_Msk      |
                        PWRCTRL_DEVPWRSTATUS_PWRSTUART3_Msk    |
                        PWRCTRL_DEVPWRSTATUS_PWRSTUART2_Msk    |
                        PWRCTRL_DEVPWRSTATUS_PWRSTUART1_Msk    |
                        PWRCTRL_DEVPWRSTATUS_PWRSTUART0_Msk    |
                        PWRCTRL_DEVPWRSTATUS_PWRSTIOM7_Msk     |
                        PWRCTRL_DEVPWRSTATUS_PWRSTIOM6_Msk     |
                        PWRCTRL_DEVPWRSTATUS_PWRSTIOM5_Msk     |
                        PWRCTRL_DEVPWRSTATUS_PWRSTIOM4_Msk     |
                        PWRCTRL_DEVPWRSTATUS_PWRSTIOM3_Msk     |
                        PWRCTRL_DEVPWRSTATUS_PWRSTIOM2_Msk     |
                        PWRCTRL_DEVPWRSTATUS_PWRSTIOM1_Msk     |
                        PWRCTRL_DEVPWRSTATUS_PWRSTIOM0_Msk     |
                        PWRCTRL_DEVPWRSTATUS_PWRSTIOS_Msk))      &&
                    !(PWRCTRL->MEMPWRSTATUS &
                        (PWRCTRL_MEMPWRSTATUS_PWRSTROM_Msk      |
                        PWRCTRL_MEMPWRSTATUS_PWRSTOTP_Msk))       )
                {
                    // CSP TrimRev 8 has an RDL issue which requires Buck to stay in Active mode during DeepSleep
                    if (!(bCSP && (trimRev == 8)))
                    {
                        bBuckIntoLPinDS = true;

                        //
                        // Remove overrides to allow buck to go in LP mode
                        //
                        buck_ldo_update_override(false);
                    }
                }
            }
            am_hal_itm_handle_deepsleep_enter();
            //
            // Prepare the core for deepsleep (write 1 to the DEEPSLEEP bit).
            //
            SCB->SCR |= _VAL2FLD(SCB_SCR_SLEEPDEEP, 1);
        }
    }
    else
    {
        //
        // Prepare the core for normal sleep (write 0 to the DEEPSLEEP bit).
        //
        SCB->SCR &= ~_VAL2FLD(SCB_SCR_SLEEPDEEP, 1);
    }

    //
    // Before executing WFI, flush APB writes.
    //
    am_hal_sysctrl_sysbus_write_flush();

    //
    // Execute the sleep instruction.
    //
    __WFI();

    //
    // Upon wake, execute the Instruction Sync Barrier instruction.
    //
    __ISB();

    am_hal_itm_handle_deepsleep_exit();
    if ( bBuckIntoLPinDS )
    {
        //
        // Re-enable overrides
        //
        buck_ldo_update_override(true);
    }

    //
    // Switch back to HP mode after entering deepsleep, if it was originally in HP mode.
    //
    if (bIsHPMode == true)
    {
        am_hal_pwrctrl_mcu_mode_select(AM_HAL_PWRCTRL_MCU_MODE_HIGH_PERFORMANCE);
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
//*****************************************************************************
void
am_hal_sysctrl_fpu_enable(void)
{
    //
    //! Enable the EPU. See section 6.3 of the CM55 TRM.

    //
    SCB->CPACR |= (0xF << 20);
    __DSB();
    __ISB();
}

//*****************************************************************************
//
// Disable the floating point module.
//
//*****************************************************************************
void
am_hal_sysctrl_fpu_disable(void)
{
    //
    // Disable access to the FPU in both privileged and user modes.
    // NOTE: Write 0s to all reserved fields in this register.
    //
    SCB->CPACR |= ~(0xF << 20);
    __DSB();
    __ISB();
}

//*****************************************************************************
//
// Enable stacking of FPU registers on exception entry.
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
