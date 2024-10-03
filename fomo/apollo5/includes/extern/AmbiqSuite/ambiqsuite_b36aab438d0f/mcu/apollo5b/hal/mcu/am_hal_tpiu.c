//*****************************************************************************
//
//! @file am_hal_tpiu.c
//!
//! @brief Support functions for the Arm TPIU module
//!
//! Provides support functions for configuring the Arm TPIU module
//!
//! @addtogroup tpiu4 TPIU - Trace Port Interface Unit
//! @ingroup apollo5b_hal
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
static bool    g_bTpiu_DebugEnabled = false;
static uint8_t g_ui8TpiuEnCnt       = 0;


//*****************************************************************************
//
// Local functions
//
//*****************************************************************************
static uint32_t
tpiu_flush(void)
{
    uint32_t ui32Status;

    //
    // Generate a flush.
    // Section 2.10 Programming the TPIU-M for trace capture:
    // "Arm recommends that debug tools perform a flush by writing TPIU_FFCR.FOnMan = 1
    //  whenever the TPIU-M is reprogrammed."
    // "The TPIU clears the bit to 0 when the flush completes."
    //
    TPI->FFCR |= _VAL2FLD(TPI_FFCR_FOnMan, 1);
    ui32Status = am_hal_delay_us_status_change(100000,
                                               (uint32_t)&TPI->FFCR,
                                               TPI_FFCR_FOnMan_Msk,
                                               0);
    return ui32Status;
} // tpiu_flush()


//*****************************************************************************
//
// TPIU configuration function
//
//*****************************************************************************
uint32_t
am_hal_tpiu_config(uint32_t ui32DbgTpiuClksel,  // MCUCTRL_DBGCTRL_DBGTPIUCLKSEL_HFRC_48MHz
                   uint32_t ui32FFCR,           // 0: Disable continuous formatting (EnFCont)
                   uint32_t ui32CSPSR,          // TPI_CSPSR_CWIDTH_1BIT
                   uint32_t ui32PinProtocol,    // TPI_SPPR_TXMODE_UART
                   uint32_t ui32SWOscaler)
{
    uint32_t ui32Status;

    //
    // Enable the TPIU clock source in MCU control.
    //
    MCUCTRL->DBGCTRL_b.DBGTPIUCLKSEL = ui32DbgTpiuClksel;

    //
    // Perform general debug/tracing configuration.
    //
    if ( g_bTpiu_DebugEnabled == false )
    {
        am_hal_debug_enable();
        g_bTpiu_DebugEnabled = true;
    }

    //
    // Perform a TPIU flush before reconfiguring.
    //
    ui32Status = tpiu_flush();
    if ( ui32Status != AM_HAL_STATUS_SUCCESS )
    {
        return ui32Status;
    }

    //
    // TPIU formatter & flush control register.
    // Disable continuous formatting (EnFCont bit1).
    //
    TPI->FFCR = ui32FFCR;

    //
    // Set the Current Parallel Port Size (note - only 1 bit can be set).
    //
    TPI->CSPSR = ui32CSPSR;


    //
    // Set the scaler value.
    //
    TPI->ACPR = _VAL2FLD(TPI_ACPR_SWOSCALER, ui32SWOscaler);

    //
    // Set the Pin Protocol.
    //
    TPI->SPPR = _VAL2FLD( TPI_SPPR_TXMODE, ui32PinProtocol);

    return AM_HAL_STATUS_SUCCESS;
} // am_hal_tpiu_config()


//*****************************************************************************
//
// Enable the TPIU
//
// This function enables the Arm TPIU by setting the TPIU clock source.
//
// Arm v8-M Architecture Reference Manual, DDI0553B.y section B14.4 states
// "The TPIU is not directly affected by DEMCR.TRCENA being set to 0."
//
//*****************************************************************************
uint32_t
am_hal_tpiu_enable(uint32_t ui32DeprecatedItmBaud)
{
    //
    // Bump the counter on each enable call.
    //
    g_ui8TpiuEnCnt++;

    //
    // The deprecated ItmBaud argument is an ITM-specific parameter and is only
    // maintained here for backward compatibility. On entry, it should typically
    // be set to 0. Otherwise, pass it along to the ITM HAL.
    //
    if ( (ui32DeprecatedItmBaud != AM_HAL_TPIU_BAUD_DEFAULT) &&
         (ui32DeprecatedItmBaud != 0) )
    {
        am_hal_itm_parameters_set(ui32DeprecatedItmBaud);
    }

    if ( g_bTpiu_DebugEnabled == false )
    {
        //
        // Perform general debug/tracing configuration.
        //
        am_hal_debug_enable();
        g_bTpiu_DebugEnabled = true;
    }

    return AM_HAL_STATUS_SUCCESS;

} // am_hal_tpiu_enable()

//*****************************************************************************
//
// Disable the TPIU
//
// This function enables the Arm TPIU by disabling the TPIU clock source.
//
//*****************************************************************************
uint32_t
am_hal_tpiu_disable(void)
{
    uint32_t ui32Status;

    if ( --g_ui8TpiuEnCnt > 0 )
    {
        //
        // TPIU needs to remain enabled for now.
        //
        return AM_HAL_STATUS_IN_USE;
    }

    //
    // Invalidate any previous TPIU configuration
    //
    g_bTpiu_DebugEnabled = false;

    //
    // Perform a TPIU flush before reconfiguring.
    //
    ui32Status = tpiu_flush();
    if ( ui32Status != AM_HAL_STATUS_SUCCESS )
    {
        return ui32Status;
    }

    //
    // Shut down the general debug/tracing configuration.
    //
    ui32Status = am_hal_debug_disable();
    if ( ui32Status == AM_HAL_STATUS_IN_USE )
    {
        ui32Status = AM_HAL_STATUS_SUCCESS;
    }

    return ui32Status;

} // am_hal_tpiu_disable()

//*****************************************************************************
//
// End Doxygen group.
//! @}
//
//*****************************************************************************
