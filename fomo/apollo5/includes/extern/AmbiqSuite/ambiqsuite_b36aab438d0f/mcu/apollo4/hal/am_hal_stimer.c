//*****************************************************************************
//
//  am_hal_stimer.c
//! @file
//!
//! @brief Functions for interfacing with the system timer (STIMER).
//!
//! @addtogroup stimer4 System Timer (STIMER)
//! @ingroup apollo4hal
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

static bool bStimerConfigured = false;

//*****************************************************************************
//
//! @brief Set up the stimer.
//!
//! @param ui32STimerConfig is the value to load into the configuration reg.
//!
//! This function should be used to perform the initial set-up of the
//! stimer.
//!
//! @return The 32-bit current config of the STimer Config register
//
//*****************************************************************************
uint32_t
am_hal_stimer_config(uint32_t ui32STimerConfig)
{
    uint32_t ui32CurrVal;

    //
    // Read the current config
    //
    ui32CurrVal = STIMER->STCFG;

    //
    // Write our configuration value.
    //
    STIMER->STCFG = ui32STimerConfig;

    //
    // Indication that the STIMER has been configured.
    bStimerConfigured = true;

    return ui32CurrVal;
}

//*****************************************************************************
//
//! @brief Check if the STIMER is running.
//!
//! This function should be used to perform the initial set-up of the
//! stimer.
//!
//! @return true = STIMER running;  false = STIMER needs to be configured.
//
//*****************************************************************************
bool
am_hal_stimer_is_running(void)
{
  //
  // Check the STIMER has been configured and is currently counting
  //
  return (bStimerConfigured &&
      (STIMER_STCFG_CLKSEL_NOCLK != STIMER->STCFG_b.CLKSEL) &&
      (STIMER_STCFG_FREEZE_THAW == STIMER->STCFG_b.FREEZE) &&
      (STIMER_STCFG_CLEAR_RUN == STIMER->STCFG_b.CLEAR));
}
//*****************************************************************************
//
//! @brief Reset the current stimer block to power-up state.
//!
//! @return The 32-bit value from the STimer counter register.
//
//*****************************************************************************
void
am_hal_stimer_reset_config(void)
{
    STIMER->STCFG       = _VAL2FLD(STIMER_STCFG_FREEZE, 1);
    STIMER->SCAPCTRL0   = _VAL2FLD(STIMER_SCAPCTRL0_STSEL0, 0x7F);
    STIMER->SCAPCTRL1   = _VAL2FLD(STIMER_SCAPCTRL1_STSEL1, 0x7F);
    STIMER->SCAPCTRL2   = _VAL2FLD(STIMER_SCAPCTRL2_STSEL2, 0x7F);
    STIMER->SCAPCTRL3   = _VAL2FLD(STIMER_SCAPCTRL3_STSEL3, 0x7F);
    STIMER->SCMPR0      = 0;
    STIMER->SCMPR1      = 0;
    STIMER->SCMPR2      = 0;
    STIMER->SCMPR3      = 0;
    STIMER->SCMPR4      = 0;
    STIMER->SCMPR5      = 0;
    STIMER->SCMPR6      = 0;
    STIMER->SCMPR7      = 0;
    STIMER->SCAPT0      = 0;
    STIMER->SCAPT1      = 0;
    STIMER->SCAPT2      = 0;
    STIMER->SCAPT3      = 0;
    STIMER->SNVR0       = 0;
    STIMER->SNVR1       = 0;
    STIMER->SNVR2       = 0;
    STIMER->SNVR3       = 0;
    STIMER->STMINTEN    = 0;
    STIMER->STMINTSTAT   = 0;
    STIMER->STMINTCLR    = 0xFFF;

}

//*****************************************************************************
//
//! @brief Get the current stimer value.
//!
//! This function can be used to read, uninvasively, the value in the stimer.
//!
//! @return The 32-bit value from the STimer counter register.
//
//*****************************************************************************
uint32_t
am_hal_stimer_counter_get(void)
{
    return STIMER->STTMR;
}

//*****************************************************************************
//
//! @brief Clear the stimer counter.
//!
//! This function clears the STimer Counter and leaves the stimer running.
//!
//! @return None.
//
//*****************************************************************************
void
am_hal_stimer_counter_clear(void)
{
    //
    // Set the clear bit
    //
    STIMER->STCFG |= STIMER_STCFG_CLEAR_Msk;

    //
    // Reset the clear bit
    //
    STIMER->STCFG &= ~STIMER_STCFG_CLEAR_Msk;
}

//*****************************************************************************
//
//! @brief Set the compare value.
//!
//! @param ui32CmprInstance is the compare register instance number (0-7).
//! @param ui32Delta is the value to add to the STimer counter and load into
//!        the comparator register.
//!
//! NOTE: There is no way to set an absolute value into a comparator register.
//!       Only deltas added to the STimer counter can be written to the compare
//!       registers.
//!
//! @return None.
//
//*****************************************************************************
void
am_hal_stimer_compare_delta_set(uint32_t ui32CmprInstance, uint32_t ui32Delta)
{
#ifndef AM_HAL_DISABLE_API_VALIDATION
    if ( ui32CmprInstance > 7 )
    {
        return;
    }
#endif

    //
    // Set the delta
    //
    AM_REGVAL(AM_REG_STIMER_COMPARE(0, ui32CmprInstance)) = ui32Delta;

}

//*****************************************************************************
//
//! @brief Get the current stimer compare register value.
//!
//! @param ui32CmprInstance is the compare register instance number (0-7).
//!
//! This function can be used to read the value in an stimer compare register.
//!
//!
//! @return None.
//
//*****************************************************************************
uint32_t
am_hal_stimer_compare_get(uint32_t ui32CmprInstance)
{
#ifndef AM_HAL_DISABLE_API_VALIDATION
    if ( ui32CmprInstance > 7 )
    {
        return 0;
    }
#endif

    return AM_REGVAL(AM_REG_STIMER_COMPARE(0, ui32CmprInstance));
}

//*****************************************************************************
//
//! @brief Start capturing data with the specified capture register.
//!
//! @param ui32CaptureNum is the Capture Register Number to read (0-3).
//! @param ui32GPIONumber is the pin number.
//! @param bPolarity: false (0) = Capture on low to high transition.
//!                   true  (1) = Capture on high to low transition.
//!
//! Use this function to start capturing.
//!
//! @return None.
//
//*****************************************************************************
void
am_hal_stimer_capture_start(uint32_t ui32CaptureNum,
                            uint32_t ui32GPIONumber,
                            bool bPolarity)
{
#ifndef AM_HAL_DISABLE_API_VALIDATION
    if ( ui32GPIONumber > (AM_HAL_GPIO_MAX_PADS-1) )
    {
        return;
    }
#endif

    //
    // Set the polarity and pin selection in the GPIO block.
    //
    switch (ui32CaptureNum)
    {
         case 0:
            STIMER->SCAPCTRL0_b.STPOL0 = bPolarity;
            STIMER->SCAPCTRL0_b.STSEL0 = ui32GPIONumber;
            STIMER->SCAPCTRL0_b.CAPTURE0 = STIMER_SCAPCTRL0_CAPTURE0_ENABLE;
            break;
         case 1:
            STIMER->SCAPCTRL1_b.STPOL1 = bPolarity;
            STIMER->SCAPCTRL1_b.STSEL1 = ui32GPIONumber;
            STIMER->SCAPCTRL1_b.CAPTURE1 = STIMER_SCAPCTRL1_CAPTURE1_ENABLE;
            break;
         case 2:
            STIMER->SCAPCTRL2_b.STPOL2 = bPolarity;
            STIMER->SCAPCTRL2_b.STSEL2 = ui32GPIONumber;
            STIMER->SCAPCTRL2_b.CAPTURE2 = STIMER_SCAPCTRL2_CAPTURE2_ENABLE;
            break;
         case 3:
            STIMER->SCAPCTRL3_b.STPOL3 = bPolarity;
            STIMER->SCAPCTRL3_b.STSEL3 = ui32GPIONumber;
            STIMER->SCAPCTRL3_b.CAPTURE3 = STIMER_SCAPCTRL3_CAPTURE3_ENABLE;
            break;
         default:
            return;     // error concealment.
    }

    //
    // Set TIMER Global Enable for GPIO inputs.
    //
    TIMER->GLOBEN_b.ENABLEALLINPUTS = 1;

}


//*****************************************************************************
//
//! @brief Stop capturing data with the specified capture register.
//!
//! @param ui32CaptureNum is the Capture Register Number to read.
//!
//! Use this function to stop capturing.
//!
//! @return None.
//
//*****************************************************************************
void
am_hal_stimer_capture_stop(uint32_t ui32CaptureNum)
{
    //
    // Disable it in the STIMER block.
    //
    switch (ui32CaptureNum)
    {
         case 0:
             STIMER->SCAPCTRL0_b.CAPTURE0 = STIMER_SCAPCTRL0_CAPTURE0_DISABLE;
            break;
         case 1:
             STIMER->SCAPCTRL1_b.CAPTURE1 = STIMER_SCAPCTRL1_CAPTURE1_DISABLE;
            break;
         case 2:
             STIMER->SCAPCTRL2_b.CAPTURE2 = STIMER_SCAPCTRL2_CAPTURE2_DISABLE;
            break;
         case 3:
             STIMER->SCAPCTRL3_b.CAPTURE3 = STIMER_SCAPCTRL3_CAPTURE3_DISABLE;
            break;
         default:
            return;     // error concealment.
    }

    //
    // Start a critical section.
    //
    AM_CRITICAL_BEGIN

    //
    // Set TIMER Global Disable for GPIO inputs if all are DISABLED.
    //
    if ((STIMER->SCAPCTRL0_b.CAPTURE0 == STIMER_SCAPCTRL0_CAPTURE0_DISABLE) &&
            (STIMER->SCAPCTRL1_b.CAPTURE1 == STIMER_SCAPCTRL1_CAPTURE1_DISABLE) &&
            (STIMER->SCAPCTRL2_b.CAPTURE2 == STIMER_SCAPCTRL2_CAPTURE2_DISABLE) &&
            (STIMER->SCAPCTRL3_b.CAPTURE3 == STIMER_SCAPCTRL3_CAPTURE3_DISABLE))
    {
        TIMER->GLOBEN_b.ENABLEALLINPUTS = 0;
    }

    AM_CRITICAL_END
}

//*****************************************************************************
//
//! @brief Get the current stimer nvram register value.
//!
//! @param ui32NvramNum is the NVRAM Register Number to read.
//! @param ui32NvramVal is the value to write to NVRAM.
//!
//! This function can be used to read the value in an stimer NVRAM register.
//!
//!
//! @return None.
//
//*****************************************************************************
void
am_hal_stimer_nvram_set(uint32_t ui32NvramNum, uint32_t ui32NvramVal)
{
#ifndef AM_HAL_DISABLE_API_VALIDATION
    if ( ui32NvramNum > 3 )
    {
        return;
    }
#endif

    AM_REGVAL(AM_REG_STIMER_NVRAM(0, ui32NvramNum)) = ui32NvramVal;
}

//*****************************************************************************
//
//! @brief Get the current stimer nvram register value.
//!
//! @param ui32NvramNum is the NVRAM Register Number to read.
//!
//! This function can be used to read the value in an stimer NVRAM register.
//!
//!
//! @return None.
//
//*****************************************************************************
uint32_t am_hal_stimer_nvram_get(uint32_t ui32NvramNum)
{
#ifndef AM_HAL_DISABLE_API_VALIDATION
    if ( ui32NvramNum > 3 )
    {
        return 0;
    }
#endif

    return AM_REGVAL(AM_REG_STIMER_NVRAM(0, ui32NvramNum));
}

//*****************************************************************************
//
//! @brief Get the current stimer capture register value.
//!
//! @param ui32CaptureNum is the Capture Register Number to read.
//!
//! This function can be used to read the value in an stimer capture register.
//!
//!
//! @return None.
//
//*****************************************************************************
uint32_t am_hal_stimer_capture_get(uint32_t ui32CaptureNum)
{
#ifndef AM_HAL_DISABLE_API_VALIDATION
    if ( ui32CaptureNum > 3 )
    {
        return 0;
    }
#endif

    return AM_REGVAL(AM_REG_STIMER_CAPTURE(0, ui32CaptureNum));
}

//*****************************************************************************
//
//! @brief Enables the selected system timer interrupt.
//!
//! @param ui32Interrupt is the interrupt to be used.
//!
//! This function will enable the selected interrupts in the STIMER interrupt
//! enable register. In order to receive an interrupt from an stimer component,
//! you will need to enable the interrupt for that component in this main
//! register, as well as in the stimer configuration register (accessible though
//! am_hal_stimer_config()), and in the NVIC.
//!
//! ui32Interrupt should be the logical OR of one or more of the following
//! values:
//!
//!     AM_HAL_STIMER_INT_COMPAREA
//!     AM_HAL_STIMER_INT_COMPAREB
//!     AM_HAL_STIMER_INT_COMPAREC
//!     AM_HAL_STIMER_INT_COMPARED
//!     AM_HAL_STIMER_INT_COMPAREE
//!     AM_HAL_STIMER_INT_COMPAREF
//!     AM_HAL_STIMER_INT_COMPAREG
//!     AM_HAL_STIMER_INT_COMPAREH
//!
//!     AM_HAL_STIMER_INT_OVERFLOW
//!
//!     AM_HAL_STIMER_INT_CAPTUREA
//!     AM_HAL_STIMER_INT_CAPTUREB
//!     AM_HAL_STIMER_INT_CAPTUREC
//!     AM_HAL_STIMER_INT_CAPTURED
//!
//! @return None.
//
//*****************************************************************************
void
am_hal_stimer_int_enable(uint32_t ui32Interrupt)
{
    //
    // Enable the interrupt at the module level.
    //
    STIMER->STMINTEN |= ui32Interrupt;
}

//*****************************************************************************
//
//! @brief Return the enabled stimer interrupts.
//!
//! This function will return all enabled interrupts in the STIMER
//! interrupt enable register.
//!
//! @return return enabled interrupts. This will be a logical or of:
//!
//!     AM_HAL_STIMER_INT_COMPAREA
//!     AM_HAL_STIMER_INT_COMPAREB
//!     AM_HAL_STIMER_INT_COMPAREC
//!     AM_HAL_STIMER_INT_COMPARED
//!     AM_HAL_STIMER_INT_COMPAREE
//!     AM_HAL_STIMER_INT_COMPAREF
//!     AM_HAL_STIMER_INT_COMPAREG
//!     AM_HAL_STIMER_INT_COMPAREH
//!
//!     AM_HAL_STIMER_INT_OVERFLOW
//!
//!     AM_HAL_STIMER_INT_CAPTUREA
//!     AM_HAL_STIMER_INT_CAPTUREB
//!     AM_HAL_STIMER_INT_CAPTUREC
//!     AM_HAL_STIMER_INT_CAPTURED
//!
//! @return Return the enabled timer interrupts.
//
//*****************************************************************************
uint32_t
am_hal_stimer_int_enable_get(void)
{
    //
    // Return enabled interrupts.
    //
    return STIMER->STMINTEN;
}

//*****************************************************************************
//
//! @brief Disables the selected stimer interrupt.
//!
//! @param ui32Interrupt is the interrupt to be used.
//!
//! This function will disable the selected interrupts in the STIMER
//! interrupt register.
//!
//! ui32Interrupt should be the logical OR of one or more of the following
//! values:
//!
//!     AM_HAL_STIMER_INT_COMPAREA
//!     AM_HAL_STIMER_INT_COMPAREB
//!     AM_HAL_STIMER_INT_COMPAREC
//!     AM_HAL_STIMER_INT_COMPARED
//!     AM_HAL_STIMER_INT_COMPAREE
//!     AM_HAL_STIMER_INT_COMPAREF
//!     AM_HAL_STIMER_INT_COMPAREG
//!     AM_HAL_STIMER_INT_COMPAREH
//!
//!     AM_HAL_STIMER_INT_OVERFLOW
//!
//!     AM_HAL_STIMER_INT_CAPTUREA
//!     AM_HAL_STIMER_INT_CAPTUREB
//!     AM_HAL_STIMER_INT_CAPTUREC
//!     AM_HAL_STIMER_INT_CAPTURED
//!
//! @return None.
//
//*****************************************************************************
void
am_hal_stimer_int_disable(uint32_t ui32Interrupt)
{
    //
    // Disable the interrupt at the module level.
    //
    STIMER->STMINTEN &= ~ui32Interrupt;
}

//*****************************************************************************
//
//! @brief Sets the selected stimer interrupt.
//!
//! @param ui32Interrupt is the interrupt to be used.
//!
//! This function will set the selected interrupts in the STIMER
//! interrupt register.
//!
//! ui32Interrupt should be the logical OR of one or more of the following
//! values:
//!
//!     AM_HAL_STIMER_INT_COMPAREA
//!     AM_HAL_STIMER_INT_COMPAREB
//!     AM_HAL_STIMER_INT_COMPAREC
//!     AM_HAL_STIMER_INT_COMPARED
//!     AM_HAL_STIMER_INT_COMPAREE
//!     AM_HAL_STIMER_INT_COMPAREF
//!     AM_HAL_STIMER_INT_COMPAREG
//!     AM_HAL_STIMER_INT_COMPAREH
//!
//!     AM_HAL_STIMER_INT_OVERFLOW
//!
//!     AM_HAL_STIMER_INT_CAPTUREA
//!     AM_HAL_STIMER_INT_CAPTUREB
//!     AM_HAL_STIMER_INT_CAPTUREC
//!     AM_HAL_STIMER_INT_CAPTURED
//!
//! @return None.
//
//*****************************************************************************
void
am_hal_stimer_int_set(uint32_t ui32Interrupt)
{
    //
    // Set the interrupts.
    //
    STIMER->STMINTSET = ui32Interrupt;
}

//*****************************************************************************
//
//! @brief Clears the selected stimer interrupt.
//!
//! @param ui32Interrupt is the interrupt to be used.
//!
//! This function will clear the selected interrupts in the STIMER
//! interrupt register.
//!
//! ui32Interrupt should be the logical OR of one or more of the following
//! values:
//!
//!     AM_HAL_STIMER_INT_COMPAREA
//!     AM_HAL_STIMER_INT_COMPAREB
//!     AM_HAL_STIMER_INT_COMPAREC
//!     AM_HAL_STIMER_INT_COMPARED
//!     AM_HAL_STIMER_INT_COMPAREE
//!     AM_HAL_STIMER_INT_COMPAREF
//!     AM_HAL_STIMER_INT_COMPAREG
//!     AM_HAL_STIMER_INT_COMPAREH
//!
//!     AM_HAL_STIMER_INT_OVERFLOW
//!
//!     AM_HAL_STIMER_INT_CAPTUREA
//!     AM_HAL_STIMER_INT_CAPTUREB
//!     AM_HAL_STIMER_INT_CAPTUREC
//!     AM_HAL_STIMER_INT_CAPTURED
//!
//! @return None.
//
//*****************************************************************************
void
am_hal_stimer_int_clear(uint32_t ui32Interrupt)
{
    //
    // Disable the interrupt at the module level.
    //
    STIMER->STMINTCLR = ui32Interrupt;
}


//*****************************************************************************
//
//! @brief Returns either the enabled or raw stimer interrupt status.
//!
//! This function will return the stimer interrupt status.
//!
//! @param bEnabledOnly if true returns the status of the enabled interrupts
//! only.
//!
//! The return value will be the logical OR of one or more of the following
//! values:
//!
//!
//! @return Returns the stimer interrupt status.
//
//*****************************************************************************
uint32_t
am_hal_stimer_int_status_get(bool bEnabledOnly)
{
    //
    // Return the desired status.
    //
    uint32_t ui32RetVal = STIMER->STMINTSTAT;

    if ( bEnabledOnly )
    {
        ui32RetVal &= STIMER->STMINTEN;
    }

    return ui32RetVal;
}

//*****************************************************************************
//
// End Doxygen group.
//! @}
//
//*****************************************************************************
