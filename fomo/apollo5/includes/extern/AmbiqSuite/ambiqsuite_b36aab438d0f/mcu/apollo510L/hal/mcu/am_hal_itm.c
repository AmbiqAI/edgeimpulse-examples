//*****************************************************************************
//
//! @file am_hal_itm.c
//!
//! @brief Functions for operating the instrumentation trace macrocell
//!
//! @addtogroup itm4 ITM - Instrumentation Trace Macrocell
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
static uint32_t g_ui32ItmBaud = 0;

//*****************************************************************************
//
// Sets certain ITM parameters if something other than defaults are required.
//
//*****************************************************************************
uint32_t
am_hal_itm_parameters_set(uint32_t ui32ItmBaud)
{
    g_ui32ItmBaud = ui32ItmBaud;

    return AM_HAL_STATUS_SUCCESS;

} // am_hal_itm_parameters_set()

//*****************************************************************************
//
// Enable the ITM
//
//*****************************************************************************
uint32_t
am_hal_itm_enable(void)
{
    uint32_t ui32SWOscaler;
    uint32_t ui32Status = AM_HAL_STATUS_SUCCESS;

    //
    // Perform general debug/tracing configuration.
    //
    am_hal_debug_enable();

    //
    // Use the default baud rate if none was provided.
    //
    if ( g_ui32ItmBaud == 0 )
    {
        //
        // Set the default
        //
        g_ui32ItmBaud = AM_HAL_TPIU_BAUD_DEFAULT;
    }

    //
    // Compute SWOscaler so that the TPIU can be configured.
    //
// #### INTERNAL BEGIN ####
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
// #### INTERNAL END ####
    ui32SWOscaler = ( (AM_HAL_CLKGEN_FREQ_MAX_HZ / 2) /
                       g_ui32ItmBaud ) - 1;
// #### INTERNAL BEGIN ####
#endif // APOLLO5_FPGA
// #### INTERNAL END ####

    am_hal_tpiu_config(MCUCTRL_DBGCTRL_DBGTPIUCLKSEL_HFRC_48MHz,
                       0,                                   // FFCR = Disable continuous formatting (EnFCont)
                       TPI_CSPSR_CWIDTH_1BIT,               // CSPSR = TPI_CSPSR_CWIDTH_1BIT
                       TPI_SPPR_TXMODE_UART,                // PinProtocol = TPI_SPPR_TXMODE_UART
                       ui32SWOscaler);

    //
    // Write the key to the ITM Lock Access Register to unlock the ITM TCR.
    //
    ITM->LAR = ITM_LAR_KEYVAL;

    //
    // Set the enable bits in the ITM Trace Privilege Register and the
    // ITM Trace Enable Register to enable trace data output.
    //
    ITM->TPR = 0x0000000F;
    ITM->TER = 0xFFFFFFFF;

    //
    // Write the fields in the ITM Trace Control Register.
    //
    ITM->TCR =
        _VAL2FLD(ITM_TCR_TRACEBUSID, 0x15)      |
        _VAL2FLD(ITM_TCR_GTSFREQ, 1)            |
        _VAL2FLD(ITM_TCR_TSPRESCALE, 1)         |
        _VAL2FLD(ITM_TCR_STALLENA, 0)           |
        _VAL2FLD(ITM_TCR_SWOENA, 1)             |
        _VAL2FLD(ITM_TCR_DWTENA, 0)             |
        _VAL2FLD(ITM_TCR_SYNCENA, 0)            |
        _VAL2FLD(ITM_TCR_TSENA, 0)              |
        _VAL2FLD(ITM_TCR_ITMENA, 1);
// #### INTERNAL BEGIN ####
#if MCU_VALIDATION
    //
    // To promote better stability over temperature, enable the HFRC Adjust.
    //
    // Important: After this point, the XTAL and HFADJ will remain enabled,
    // even after am_hal_itm_disable() is called, for those applications that
    // repeatedly enable and disable the ITM.
    //
    if ( CLKGEN->OCTRL_b.STOPXT == 1 )
    {
        //
        // Enable the XTAL.
        //
        am_hal_clkgen_osc_start(AM_HAL_CLKGEN_OSC_XT);
    }

    if ( CLKGEN->HFADJ_b.HFADJEN == 0 )
    {
        am_hal_clkgen_hfrc_adjust_enable(AM_REG_CLKGEN_HFADJ_HFWARMUP_2SEC,
                                         AM_REG_CLKGEN_HFADJ_HFADJCK_4SEC);
    }
#endif // MCU_VALIDATION
// #### INTERNAL END ####

    return ui32Status;

} // am_hal_itm_enable()

//*****************************************************************************
//
// Disable the ITM
//
//*****************************************************************************
uint32_t
am_hal_itm_disable(void)
{
    uint32_t ui32Status = AM_HAL_STATUS_SUCCESS;

    //
    // Before disabling, make sure all printing has completed,
    // and also that the ITM is not busy.
    //
    // ARMv8-M Software must ensure that all trace is output and flushed
    // before clearing TRCENA.
    //
    if ( !(am_hal_itm_print_not_busy() && am_hal_itm_not_busy()) )
    {
        ui32Status = AM_HAL_STATUS_TIMEOUT;
        return ui32Status;
    }

    //
    // Unlock the ITM_TCR.
    //
    ITM->LAR = ITM_LAR_KEYVAL;

    //
    // Disable SWO and the ITM.
    //
    ITM->TCR &= ~ITM_TCR_SWOENA_Msk;
    ITM->TCR &= ~ITM_TCR_ITMENA_Msk;

    //
    // Wait for the changes to take effect with a 1ms timeout.
    // #### INTERNAL BEGIN ####
    // NOTE: 1ms is arbitrary, but should be more than enough time.
    // #### INTERNAL END ####
    //
    ui32Status = am_hal_delay_us_status_change(1000,
                                               (uint32_t)&ITM->TCR,
                                               (ITM_TCR_ITMENA_Msk & ITM_TCR_BUSY_Msk),
                                               0 );
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

} // am_hal_itm_disable()

//*****************************************************************************
//
// Checks if itm is busy and provides a delay to flush the fifo
// Return: true  = ITM not busy and no timeout occurred.
//         false = Timeout occurred.
//
//*****************************************************************************
bool
am_hal_itm_not_busy(void)
{
    //
    // Make sure the ITM is not busy.
    //
    uint32_t ui32Ret;
    ui32Ret = am_hal_delay_us_status_change(1000,
                                            (uint32_t)&ITM->TCR,
                                            ITM_TCR_BUSY_Msk,
                                            0 );

    return (ui32Ret == AM_HAL_STATUS_SUCCESS) ? true : false;

} // am_hal_itm_not_busy()

//*****************************************************************************
//
// Enables tracing on a given set of ITM ports
//
//*****************************************************************************
void
am_hal_itm_trace_port_enable(uint8_t ui8portNum)
{
    ITM->TPR |= (0x00000001 << (ui8portNum >> 3));

} // am_hal_itm_trace_port_enable()

//*****************************************************************************
//
// Disable tracing on the given ITM stimulus port.
//
//*****************************************************************************
void
am_hal_itm_trace_port_disable(uint8_t ui8portNum)
{
    ITM->TPR &= ~(0x00000001 << (ui8portNum >> 3));

} // am_hal_itm_trace_port_disable()

//*****************************************************************************
//
// Poll the given ITM stimulus register until not busy.
// Returns true if not busy, false on timeout (not ready).
//
//*****************************************************************************
bool
am_hal_itm_stimulus_not_busy(uint32_t ui32StimReg)
{
    uint32_t ui32StimAddr = (uint32_t)&ITM->PORT[0] + (4 * ui32StimReg);

    //
    // Waiting until port is available for writing, non-zero means ready to
    // accept additional characters.
    // Note that this doesn't necessarily mean the the port has emptied, and
    // there doesn't seem to be a way to determine that, so unfortunately this
    // function is a bit of a misnomer.
    //
    // M55 ITM stimulus registers on read:
    //  Bit1 DISABLED:  0=Stimulus port and ITM are enabled.
    //                  1=Stimulus port or ITM are disabled.
    //  Bit0 FIFOREADY: 0=Stimulus port cannot accept data,
    //                  1=Stimulus port can accept at least one word.
    //
    // Since this could likely occur while characters are still being output,
    // a hefty timeout period needs to be applied.
    //
    uint32_t ui32Ret;

    ui32Ret = am_hal_delay_us_status_change(1000,
                                            ui32StimAddr,
                                            ITM_STIM_DISABLED_Msk | ITM_STIM_FIFOREADY_Msk,
                                            ITM_STIM_FIFOREADY_Msk );

    return (ui32Ret == AM_HAL_STATUS_SUCCESS) ? true : false;
} // am_hal_itm_stimulus_not_busy()

//*****************************************************************************
//
// Writes a 32-bit value to the given ITM stimulus register.
//
//*****************************************************************************
void
am_hal_itm_stimulus_reg_word_write(uint32_t ui32StimReg, uint32_t ui32Value)
{
    uint32_t ui32StimAddr = (uint32_t)&ITM->PORT[0] + (4 * ui32StimReg);

    //
    // Busy waiting until it is available, non-zero means ready
    //
    while (!AM_REGVAL(ui32StimAddr));

    //
    // Write the register.
    //
    AM_REGVAL(ui32StimAddr) = ui32Value;
} // am_hal_itm_stimulus_reg_word_write()

//*****************************************************************************
//
// Writes a short to the given ITM stimulus register.
//
//*****************************************************************************
void
am_hal_itm_stimulus_reg_short_write(uint32_t ui32StimReg, uint16_t ui16Value)
{
    uint32_t ui32StimAddr = (uint32_t)&ITM->PORT[0] + (4 * ui32StimReg);

    //
    // Busy waiting until it is available non-zero means ready
    //
    while ( !AM_REGVAL(ui32StimAddr) );

    //
    // Write the register.
    //
    *((volatile uint16_t *) ui32StimAddr) = ui16Value;
} // am_hal_itm_stimulus_reg_short_write()

//*****************************************************************************
//
// Writes a byte to the given ITM stimulus register.
//
//*****************************************************************************
void
am_hal_itm_stimulus_reg_byte_write(uint32_t ui32StimReg, uint8_t ui8Value)
{
    uint32_t ui32StimAddr = (uint32_t)&ITM->PORT[0] + (4 * ui32StimReg);

    //
    // Busy waiting until it is available (non-zero means ready)
    //
    while (!AM_REGVAL(ui32StimAddr));

    //
    // Write the register.
    //
    *((volatile uint8_t *) ui32StimAddr) = ui8Value;
} // am_hal_itm_stimulus_reg_byte_write()

//*****************************************************************************
//
// Sends a Sync Packet.
//
//*****************************************************************************
void
am_hal_itm_sync_send(void)
{
    //
    // Write the register.
    //
    am_hal_itm_stimulus_reg_word_write(AM_HAL_ITM_SYNC_REG,
                                       AM_HAL_ITM_SYNC_VAL);
} // am_hal_itm_sync_send()

//*****************************************************************************
//
// Poll the print stimulus registers until not busy.
//
//*****************************************************************************
bool
am_hal_itm_print_not_busy(void)
{
    //
    // Poll stimulus register allocated for printing.
    //
    return am_hal_itm_stimulus_not_busy(0);

} // am_hal_itm_print_not_busy()

//*****************************************************************************
//
// Prints a char string out of the ITM.
//
//*****************************************************************************
void
am_hal_itm_print(char *pcString)
{
    uint32_t ui32Length = 0;

    //
    // Determine the length of the string.
    //
    while (*(pcString + ui32Length))
    {
        ui32Length++;
    }

    //
    // If there is no longer a word left, empty out the remaining characters.
    //
    while (ui32Length)
    {
        //
        // Print string out the ITM.
        //
        am_hal_itm_stimulus_reg_byte_write(0, (uint8_t)*pcString++);

        //
        // Subtract from length.
        //
        ui32Length--;
    }
} // am_hal_itm_print()

//*****************************************************************************
//
// End Doxygen group.
//! @}
//
//*****************************************************************************
