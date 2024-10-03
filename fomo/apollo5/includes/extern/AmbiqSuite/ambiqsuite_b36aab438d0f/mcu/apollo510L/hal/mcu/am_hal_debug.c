//*****************************************************************************
//
//! @file am_hal_debug.c
//!
//! @brief Functions for general debug operations.
//!
//! @addtogroup debug
//! @ingroup apollo510L_hal
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
// Global Variables
//
//*****************************************************************************
static uint8_t g_ui8DebugEnableCount    = 0;
static uint8_t g_ui8PowerCount          = 0;
static uint8_t g_ui8TRCENAcount         = 0;
static uint8_t g_ui8PwrStDbgOnEntry     = 0;


//*****************************************************************************
//
//  am_hal_debug_enable()
//  Perform steps necessary for general enabling for debug.
//
//*****************************************************************************
uint32_t
am_hal_debug_enable(void)
{
    uint32_t ui32Ret;

    AM_CRITICAL_BEGIN

    //
    // Bump the counter on each enable call.
    //
    g_ui8DebugEnableCount++;

    //
    // Check if the debug power domain needs to be powered up.
    //
    am_hal_debug_power(true);

    //
    // Set DEMCR.TRCENA (see notes in the function)
    //
    ui32Ret = am_hal_debug_trace_enable();

    //
    // Enable the TPIU clock source in MCU control.
    // TPIU clock is required for any tracing capability.
    //
    if ( MCUCTRL->DBGCTRL_b.DBGTPIUCLKSEL == MCUCTRL_DBGCTRL_DBGTPIUCLKSEL_OFF )
    {
        //
        // Set a default clock rate.
        //
        MCUCTRL->DBGCTRL_b.DBGTPIUCLKSEL = MCUCTRL_DBGCTRL_DBGTPIUCLKSEL_HFRC_96MHz;
    }

    //
    // Set TPIU TRACEENABLE in MCUCTRL.
    //
    MCUCTRL->DBGCTRL_b.DBGTPIUTRACEENABLE = MCUCTRL_DBGCTRL_DBGTPIUTRACEENABLE_EN;

    AM_CRITICAL_END

    return ui32Ret;

} // am_hal_debug_enable()

//*****************************************************************************
//
//  am_hal_debug_disable()
//  Perform steps necessary to disable from debug.
//
//*****************************************************************************
uint32_t
am_hal_debug_disable(void)
{
    uint32_t ui32Ret = AM_HAL_STATUS_SUCCESS;

    AM_CRITICAL_BEGIN

    if ( --g_ui8DebugEnableCount > 0 )
    {
        //
        // Debug to remain enabled for now.
        //
        ui32Ret = AM_HAL_STATUS_IN_USE;
    }
    else
    {
        am_hal_debug_trace_disable();

        //
        // Disable the CM55 TPIU clock source in MCU control.
        //
        MCUCTRL->DBGCTRL_b.DBGTPIUTRACEENABLE = MCUCTRL_DBGCTRL_DBGTPIUTRACEENABLE_DIS;
        MCUCTRL->DBGCTRL_b.DBGTPIUCLKSEL      = MCUCTRL_DBGCTRL_DBGTPIUCLKSEL_OFF;

        //
        // Check if the debug power domain needs to be powered down.
        //
        ui32Ret = am_hal_debug_power(false);
    }

    AM_CRITICAL_END

    return ui32Ret;

} // am_hal_debug_disable()

//*****************************************************************************
//
// am_hal_debug_power()
// Set or disable power for the debug domain.
//
// #### INTERNAL BEGIN ####
// CAB-114 Power-up of PD_DBG does not work on GLS SPRINT17.
// FBRD-156 Auto wakeup of debug power domain does not seem to work on Rev A Silicon
// These issues require software to make sure the debug domain is powered up.
// #### INTERNAL END ####
//*****************************************************************************
uint32_t
am_hal_debug_power(bool bPowerup)
{
    uint32_t ui32Ret = AM_HAL_STATUS_SUCCESS;

    AM_CRITICAL_BEGIN

    if ( bPowerup )
    {
        //
        // Bump the count
        //
        g_ui8PowerCount++;

        //
        // Make sure the debug domain is powered up.
        //
        if ( g_ui8PwrStDbgOnEntry == 0x0 )
        {
            //
            //  0x1 = Debug power was initially off.
            //  0x3 = Debug power was initially on.
            //  All other values are invalid and would generally indicate that
            //  this function was never called.
            //
            g_ui8PwrStDbgOnEntry = 0x1;

            if ( PWRCTRL->DEVPWRSTATUS_b.PWRSTDBG )
            {
                //
                // Already powered up, set the flag.
                //
                g_ui8PwrStDbgOnEntry |= 0x2;
            }
            else
            {
                //
                // Power up the debug domain.
                //
                PWRCTRL->DEVPWREN_b.PWRENDBG = PWRCTRL_DEVPWREN_PWRENDBG_EN;
            }
        }
    }
    else
    {
        if ( --g_ui8PowerCount > 0 )
        {
            //
            // Debug to remain enabled for now.
            //
            ui32Ret = AM_HAL_STATUS_IN_USE;
        }
        else
        {
            if ( g_ui8PwrStDbgOnEntry == 0x1 )
            {
                PWRCTRL->DEVPWREN_b.PWRENDBG = PWRCTRL_DEVPWREN_PWRENDBG_DIS;
            }
            g_ui8PwrStDbgOnEntry = 0x0;
        }
    }

    AM_CRITICAL_END

    return ui32Ret;

} // am_hal_debug_power()

//*****************************************************************************
//
// am_hal_debug_trace_enable()
// Enable debug tracing.
//
//*****************************************************************************
uint32_t
am_hal_debug_trace_enable(void)
{
    uint32_t ui32Ret;

    AM_CRITICAL_BEGIN

    //
    // DWT, PMU, and ITM all require that the TRCENA bit be set in the Debug
    // Exception and Monitor Control Register (DEMCR).
    //
    // Further, ETM may not be completely excluded.
    // Per section D1.2.37 DEMCR of Arm v8-M Architecture Reference Manual,
    // DDI0553B.y: "Arm recommends that this bit is set to 1 when using an
    // ETM even if any implemented DWT, PMU, and ITM are not being used."
    //
    // Conversely, section B14.3 states "The ETM is not directly affected by
    // DEMCR.TRCENA being set to 0."
    //
    g_ui8TRCENAcount++;

    DCB->DEMCR |= DCB_DEMCR_TRCENA_Msk;

    //
    // Wait for the changes to take effect with a 10us timeout.
    //
    ui32Ret = am_hal_delay_us_status_change(10,
                                            (uint32_t)&DCB->DEMCR,
                                            DCB_DEMCR_TRCENA_Msk,
                                            DCB_DEMCR_TRCENA_Msk );

    AM_CRITICAL_END

    return ui32Ret;

} // am_hal_debug_trace_enable()

//*****************************************************************************
//
// am_hal_debug_trace_disable()
// Disable debug tracing.
//
//*****************************************************************************
uint32_t
am_hal_debug_trace_disable(void)
{
    uint32_t ui32Ret = AM_HAL_STATUS_SUCCESS;

    AM_CRITICAL_BEGIN

    //
    // Reset the TRCENA bit in the DEMCR register.
    // Note that all operations from DWT, PMU, and ITM should be discontinued
    // and flushed before calling this function.
    //
    if ( --g_ui8TRCENAcount > 0 )
    {
        ui32Ret = AM_HAL_STATUS_IN_USE;
    }
    else
    {
        DCB->DEMCR &= ~DCB_DEMCR_TRCENA_Msk;

        //
        // Wait for the changes to take effect with a 10us timeout.
        //
        ui32Ret = am_hal_delay_us_status_change(10,
                                                (uint32_t)&DCB->DEMCR,
                                                DCB_DEMCR_TRCENA_Msk,
                                                0 );
    }

    AM_CRITICAL_END

    return ui32Ret;

} // am_hal_trace_disable()


//*****************************************************************************
//
// End Doxygen group.
//! @}
//
//*****************************************************************************
