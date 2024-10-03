//*****************************************************************************
//
//! @file am_hal_tpiu.c
//!
//! @brief Support functions for the ARM TPIU module
//!
//! Provides support functions for configuring the ARM TPIU module
//!
//! @addtogroup tpiu4_4b TPIU - Trace Port Interface Unit
//! @ingroup apollo4b_hal
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

// ##### INTERNAL BEGIN #####
// None of our examples appear to be using these functions, but we don't want to
// throw them away just yet.
#if 0
//*****************************************************************************
//
//! @brief Enable the clock to the TPIU module.
//!
//! This function enables the clock to the TPIU module.
//!
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
//! @brief Disable the clock to the TPIU module.
//!
//! This function disables the clock to the TPIU module.
//!
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
//! @brief Set the output port width of the TPIU
//!
//! @param ui32PortWidth - The desired port width (in bits)
//!
//! This function uses the TPIU_CSPSR register to set the desired output port
//! width of the TPIU.
//!
//
//*****************************************************************************
void
am_hal_tpiu_port_width_set(uint32_t ui32PortWidth)
{
    TPI->CSPSR = 1 << (ui32PortWidth - 1);
} // am_hal_tpiu_port_width_set()

//*****************************************************************************
//
//! @brief Read the supported_output port width of the TPIU
//!
//! This function uses the \e TPIU_SSPSR register to set the supported output
//! port widths of the TPIU.
//!
//! @return Current width of the TPIU output port
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
//! @brief Read the output port width of the TPIU
//!
//! This function uses the \e TPIU_CSPSR register to set the desired output
//! port width of the TPIU.
//!
//! @return Current width of the TPIU output port
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
//! @brief Configure the TPIU based on the values in the configuration struct.
//!
//! @param psConfig - pointer to an am_hal_tpiu_config_t structure containing
//! the desired configuration information.
//!
//! This function reads the provided configuration structure, and sets the
//! relevant TPIU registers to achieve the desired configuration.
//!
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
//! @brief Enables the TPIU
//!
//! This function enables the ARM TPIU by setting the TPIU registers and then
//! enabling the TPIU clock source in MCU control register.
//!
//! @param ui32SetItmBaud - structure for configuration.
//!     But for simplicity, ui32SetItmBaud can be set to one of the
//!      following, in which case all other structure members are ignored.
//!      In this case, the given BAUD rate is based on a div-by-8 HFRC clock.
//!         AM_HAL_TPIU_BAUD_57600
//!         AM_HAL_TPIU_BAUD_115200
//!         AM_HAL_TPIU_BAUD_230400
//!         AM_HAL_TPIU_BAUD_460800
//!         AM_HAL_TPIU_BAUD_500000
//!         AM_HAL_TPIU_BAUD_1M
//!
//
//*****************************************************************************
void
am_hal_tpiu_enable(uint32_t ui32SetItmBaud)
{
    uint32_t ui32SWOscaler;

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

#ifdef APOLLO4_FPGA
    //
    // Compute an appropriate SWO scaler based on the FPGA speed
    // specified by the APOLLO4_FPGA (e.g. 48, 24, 12, ...)
    //
    if ( g_ui32FPGAfreqMHz == 0 )
    {
        g_ui32FPGAfreqMHz = APOLLO4_FPGA;
    }

    ui32SWOscaler = (g_ui32FPGAfreqMHz / 2) - 1;
#else
    ui32SWOscaler = ( (AM_HAL_CLKGEN_FREQ_MAX_HZ / 4) /
                       ui32SetItmBaud ) - 1;
#endif // APOLLO4_FPGA

  //
  // Set the scaler value.
  //
// ##### INTERNAL BEGIN #####
// FIXME TODO: These should be defined in am_reg_m4.h
// ##### INTERNAL END #####
#define TPI_CSPSR_CWIDTH_1BIT      1
#define TPI_SPPR_TXMODE_UART       2
#define TPI_ITCTRL_Mode_NORMAL     0
#ifndef TPI_ACPR_SWOSCALER_Pos
#define TPI_ACPR_SWOSCALER_Pos              0U                                         /*!< TPI ACPR: SWOSCALER Position */
#define TPI_ACPR_SWOSCALER_Msk             (0xFFFFUL /*<< TPI_ACPR_SWOSCALER_Pos*/)    /*!< TPI ACPR: SWOSCALER Mask */
#endif
    TPI->ACPR = _VAL2FLD(TPI_ACPR_SWOSCALER, ui32SWOscaler);

    //
    // Set the Pin Protocol.
    //
    TPI->SPPR = _VAL2FLD( TPI_SPPR_TXMODE, TPI_SPPR_TXMODE_UART);  // NRZ

    //
    // Make sure we are not in test mode (important for proper deep sleep
    // operation).
    //
    TPI->ITCTRL = _VAL2FLD(TPI_ITCTRL_Mode, TPI_ITCTRL_Mode_NORMAL);

    //
    // Enable the TPIU clock source in MCU control.
    // We have the ui32SWOScaler set for 96/4/1 = 24MHz.
    // So for 1MHz SWO, set TPIU clock for 24MHz operation.
    //
    MCUCTRL->DBGCTRL_b.CM4CLKSEL     = MCUCTRL_DBGCTRL_CM4CLKSEL_HFRCDIV4;
    MCUCTRL->DBGCTRL_b.CM4TPIUENABLE = MCUCTRL_DBGCTRL_CM4TPIUENABLE_EN;

    //
    // Delay to allow the data to flush out.
    //
// ##### INTERNAL BEGIN #####
//  am_hal_delay_us(50);    // For Apollo4b, 50 is not quite enough, 60 seems to work (using binary_counter to test).
                            // The symptom of the failure is that occasionally after ITM/TPIU is disabled
                            // then re-enabled, the first character in the print string will not print.
// ##### INTERNAL END #####
    am_hal_delay_us(100);

} // am_hal_tpiu_enable()

//*****************************************************************************
//
//! @brief Disables the TPIU
//!
//! This function disables the ARM TPIU by disabling the TPIU clock source
//! in MCU control register.
//!
//
//*****************************************************************************
void
am_hal_tpiu_disable(void)
{
    //
    // Disable the CM4 TPIU clock source in MCU control.
    //
    MCUCTRL->DBGCTRL_b.CM4CLKSEL = MCUCTRL_DBGCTRL_CM4CLKSEL_LOWPWR;
    MCUCTRL->DBGCTRL_b.CM4TPIUENABLE = MCUCTRL_DBGCTRL_CM4TPIUENABLE_DIS;
} // am_hal_tpiu_disable()

//*****************************************************************************
//
// End Doxygen group.
//! @}
//
//*****************************************************************************
