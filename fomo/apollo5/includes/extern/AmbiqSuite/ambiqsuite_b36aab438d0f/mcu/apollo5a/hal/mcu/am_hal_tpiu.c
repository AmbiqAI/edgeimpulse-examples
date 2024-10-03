//*****************************************************************************
//
//! @file am_hal_tpiu.c
//!
//! @brief Support functions for the ARM TPIU module
//!
//! Provides support functions for configuring the ARM TPIU module
//!
//! @addtogroup tpiu4 TPIU - Trace Port Interface Unit
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

#define TPI_CSPSR_CWIDTH_1BIT      1


// ##### INTERNAL BEGIN #####
// None of our examples appear to be using these functions, but we don't want to
// throw them away just yet.
#if 0
//*****************************************************************************
//
// Enable the clock to the TPIU module.
//
// This function enables the clock to the TPIU module.
//
//*****************************************************************************
void
am_hal_tpiu_clock_enable(void)
{
    //
    // Enable the TPIU clock
    //
    MCUCTRL->DBGCTRL_b.CM4TPIUENABLE = MCUCTRL_DBGCTRL_CM4TPIUENABLE_EN;
} // am_hal_tpiu_clock_enable()

//*****************************************************************************
//
// Disable the clock to the TPIU module.
//
// This function disables the clock to the TPIU modules
//
//*****************************************************************************
void
am_hal_tpiu_clock_disable(void)
{
    //
    // Disable the TPIU clock
    //
    MCUCTRL->DBGCTRL_b.CM4TPIUENABLE = MCUCTRL_DBGCTRL_CM4TPIUENABLE_DIS;
} // am_hal_tpiu_clock_disable()

//*****************************************************************************
//
// Set the output port width of the TPIU
//
// This function uses the TPIU_CSPSR register to set the desired output port
// width of the TPIU.
//
//*****************************************************************************
void
am_hal_tpiu_port_width_set(uint32_t ui32PortWidth)
{
    TPI->CSPSR = 1 << (ui32PortWidth - 1);
} // am_hal_tpiu_port_width_set()

//*****************************************************************************
//
// Read the supported_output port width of the TPIU
//
// This function uses the \e TPIU_SSPSR register to set the supported output
// port widths of the TPIU.
//
//*****************************************************************************
uint32_t
am_hal_tpiu_supported_port_width_get(void)
{
    uint32_t i, ui32WidthValue;

    //
    // Read the supported width register.
    //
    ui32WidthValue = TPI->SSPSR;

    //
    // The register value is encoded in a one-hot format, so the position of
    // the single set bit determines the actual width of the port.
    //
    for (i = 1; i < 32; i++)
    {
        //
        // Check each bit for a '1'. When we find it, our current loop index
        // will be equal to the port width.
        //
        if (ui32WidthValue == (0x1 << (i - 1)))
        {
            return i;
        }
    }

    //
    // We should never get here, but if we do, just return the smallest
    // possible value for a supported trace port width.
    //
    return 1;
} // am_hal_tpiu_supported_port_width_get()

//*****************************************************************************
//
// Read the output port width of the TPIU
//
// This function uses the \e TPIU_CSPSR register to set the desired output
// port width of the TPIU.
//
//*****************************************************************************
uint32_t
am_hal_tpiu_port_width_get(void)
{
    uint32_t ui32Temp;
    uint32_t ui32Width;

    ui32Width = 1;
    ui32Temp = TPI->CSPSR;

    while ( !(ui32Temp & 1) )
    {
        ui32Temp = ui32Temp >> 1;
        ui32Width++;

        if (ui32Width > 32)
        {
            ui32Width = 0;
            break;
        }
    }

    //
    // Current width of the TPIU output port.
    //
    return ui32Width;
} // am_hal_tpiu_port_width_get()

//*****************************************************************************
//
// Configure the TPIU based on the values in the configuration struct.
//
// This function reads the provided configuration structure, and sets the
// relevant TPIU registers to achieve the desired configuration.
//
//*****************************************************************************
void
am_hal_tpiu_configure(am_hal_tpiu_config_t *psConfig)
{
    //
    // Set the clock freq in the MCUCTRL register.
    //
    MCUCTRL->DBGCTRL_b.DBGCLKSEL = psConfig->eTraceClkIn;

    //
    // Set the desired protocol.
    //
    TPI->SPPR = psConfig->ui32PinProtocol;

    //
    // Set the parallel port width. This may be redundant if the user has
    // selected a serial protocol, but we'll set it anyway.
    //
    TPI->CSPSR = (1 << (psConfig->ui32ParallelPortSize - 1));

    //
    // Set the clock prescaler.
    //
    TPI->ACPR = psConfig->ui32ClockPrescaler;
} // am_hal_tpiu_configure()

#endif
// ##### INTERNAL END #####

//*****************************************************************************
//
// Enables the TPIU
//
// This function enables the ARM TPIU by setting the TPIU registers and then
// enabling the TPIU clock source in MCU control register.
//
//*****************************************************************************
uint32_t
am_hal_tpiu_enable(uint32_t ui32SetItmBaud)
{
    uint32_t ui32SWOscaler;

// #### INTERNAL BEGIN ####
    #warning "TODO - Fix me!!! This workaround should go away in RevB."
// #### INTERNAL END ####
// #### INTERNAL BEGIN ####
    //
    // Comment out this if statement, for always enabling PWRENDBG after
    // powering up.
    //
    // if (!PWRCTRL->DEVPWRSTATUS_b.PWRSTDBG)
// #### INTERNAL END ####
    {
        //
        // Workaround to avoid hanging during initialization.
        // Power up DBG domain explicitly.
        //
        PWRCTRL->DEVPWREN_b.PWRENDBG = 1;
    }

    //
    // Enable the TPIU clock source in MCU control.
    // We have ui32SWOScaler set for 96/2/1 = 48MHz.
    // So for 1MHz SWO, set TPIU clock for 48MHz operation.
    //
    MCUCTRL->DBGCTRL_b.DBGCLKSEL     = MCUCTRL_DBGCTRL_DBGCLKSEL_HFRC48;
    MCUCTRL->DBGCTRL_b.DBGTPIUENABLE = MCUCTRL_DBGCTRL_DBGTPIUENABLE_EN;

    //
    // TPIU formatter & flush control register.
    //
    TPI->FFCR = 0;

    //
    // Set the Current Parallel Port Size (note - only 1 bit can be set).
    //
    TPI->CSPSR = TPI_CSPSR_CWIDTH_1BIT;

    //
    // Use some default assumptions to set the ITM frequency.
    //
    if ( (ui32SetItmBaud < AM_HAL_TPIU_BAUD_57600 )  ||
         (ui32SetItmBaud > AM_HAL_TPIU_BAUD_2M ) )
    {
        ui32SetItmBaud = AM_HAL_TPIU_BAUD_DEFAULT;
    }

#ifdef APOLLO5_FPGA
    //
    // Compute an appropriate SWO scaler based on the FPGA speed
    // specified by the APOLLO5_FPGA (e.g. 48, 24, 12, ...)
    //
    if ( g_ui32FPGAfreqMHz == 0 )
    {
        g_ui32FPGAfreqMHz = APOLLO5_FPGA;
    }

    ui32SWOscaler = (g_ui32FPGAfreqMHz / 2) - 1;
#else
    ui32SWOscaler = ( (AM_HAL_CLKGEN_FREQ_MAX_HZ / 2) /
                       ui32SetItmBaud ) - 1;
#endif // APOLLO5_FPGA

  //
  // Set the scaler value.
  //
// ##### INTERNAL BEGIN #####
// FIXME TODO: These should be defined in am_reg_m4.h
// ##### INTERNAL END #####
#define TPI_CSPSR_CWIDTH_1BIT      1
#define TPI_SPPR_TXMODE_UART       2
#define TPI_ITCTRL_MODE_NORMAL     0
#ifndef TPI_ACPR_SWOSCALER_Pos
#define TPI_ACPR_SWOSCALER_Pos              0U                                         /*!< TPI ACPR: SWOSCALER Position */
#define TPI_ACPR_SWOSCALER_Msk             (0xFFFFUL /*<< TPI_ACPR_SWOSCALER_Pos*/)    /*!< TPI ACPR: SWOSCALER Mask */
#endif
    TPI->ACPR = _VAL2FLD(TPI_ACPR_SWOSCALER, ui32SWOscaler);

    //
    // Set the Pin Protocol.
    //
    TPI->SPPR = _VAL2FLD( TPI_SPPR_TXMODE, TPI_SPPR_TXMODE_UART);  // NRZ

//  #### INTERNAL BEGIN ####
// ITCTRL register is not defined in core_cm55.h.
// It was, however, defined in core_m4.h. For more info, see:
// https://developer.arm.com/documentation/100166/0001/Trace-Port-Interface-Unit/TPIU-programmers-model/Integration-Mode-Control--TPIU-ITCTRL
// It was never used by Ambiq on M4 devices, so this write was unnecessary.
// It appears that Arm has removed visibility for it since it is only needed
// to activate a test mode used for redirecting trace data.
// "In integration data test mode, the trace output is disabled and data can
//  be read directly from each input port using the integration data registers."
    //
    // Make sure we are not in test mode (important for proper deep sleep
    // operation).
    //
    //TPI->ITCTRL = _VAL2FLD(TPI_ITCTRL_MODE, TPI_ITCTRL_MODE_NORMAL);
//  #### INTERNAL END ####
    //
    // Delay to allow the data to flush out.
    //
// ##### INTERNAL BEGIN #####
//  am_hal_delay_us(50);    // For Apollo4b, 50 is not quite enough, 60 seems to work (using binary_counter to test).
                            // The symptom of the failure is that occasionally after ITM/TPIU is disabled
                            // then re-enabled, the first character in the print string will not print.
// ##### INTERNAL END #####
    am_hal_delay_us(100);

    return AM_HAL_STATUS_SUCCESS;

} // am_hal_tpiu_enable()

//*****************************************************************************
//
// Disables the TPIU
//
// This function disables the ARM TPIU by disabling the TPIU clock source
// in MCU control register.
//
//*****************************************************************************
uint32_t
am_hal_tpiu_disable(void)
{
    //
    // Disable the CM4 TPIU clock source in MCU control.
    //
    MCUCTRL->DBGCTRL_b.DBGCLKSEL = MCUCTRL_DBGCTRL_DBGCLKSEL_LOWPWR;
    MCUCTRL->DBGCTRL_b.DBGTPIUENABLE = MCUCTRL_DBGCTRL_DBGTPIUENABLE_DIS;
// #### INTERNAL BEGIN ####
    #warning "TODO - Fix me!!! This workaround should go away in RevB."
// #### INTERNAL END ####
    if (PWRCTRL->DEVPWRSTATUS_b.PWRSTDBG)
    {
        //
        // Power down DBG domain explicitly.
        //
        PWRCTRL->DEVPWREN_b.PWRENDBG = 0;
    }

    return AM_HAL_STATUS_SUCCESS;

} // am_hal_tpiu_disable()

//*****************************************************************************
//
// End Doxygen group.
//! @}
//
//*****************************************************************************
