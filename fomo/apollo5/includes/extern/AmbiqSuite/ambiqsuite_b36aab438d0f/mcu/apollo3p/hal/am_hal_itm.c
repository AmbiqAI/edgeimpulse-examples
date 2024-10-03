//*****************************************************************************
//
//! @file am_hal_itm.c
//!
//! @brief Functions for Operating the Instrumentation Trace Macrocell
//!
//! @addtogroup itm3p ITM - Instrumentation Trace Macrocell
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
// Global Variables
//
//*****************************************************************************

// #### INTERNAL BEGIN ####
// TODO: setup with config structure.
// #### INTERNAL END ####
//*****************************************************************************
//
// @brief Enables the ITM
//
// This function enables the ARM ITM by setting the TRCENA bit in the DEMCR
// register.
//
//*****************************************************************************
void
am_hal_itm_enable(void)
{
    //
    // To be able to access ITM registers, set the Trace Enable bit
    // in the Debug Exception and Monitor Control Register (DEMCR).
    //
    CoreDebug->DEMCR |= _VAL2FLD(CoreDebug_DEMCR_TRCENA, 1);
    while ( !(CoreDebug->DEMCR & _VAL2FLD(CoreDebug_DEMCR_TRCENA, 1)) );

    //
    // Write the key to the ITM Lock Access register to unlock the ITM_TCR.
    //
    ITM->LAR = ITM_LAR_KEYVAL;

    //
    // Set the enable bits in the ITM trace enable register, and the ITM
    // control registers to enable trace data output.
    //
    ITM->TPR = 0x0000000F;
    ITM->TER = 0xFFFFFFFF;

    //
    // Write to the ITM control and status register.
    //
    ITM->TCR =
        _VAL2FLD(ITM_TCR_TraceBusID, 0x15)      |
        _VAL2FLD(ITM_TCR_GTSFREQ, 1)            |
        _VAL2FLD(ITM_TCR_TSPrescale, 1)         |
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


}

//*****************************************************************************
//
// @brief Disables the ITM
//
// This function completely disables the ARM ITM by resetting the TRCENA bit
// in the DEMCR register.
//
//*****************************************************************************
void
am_hal_itm_disable(void)
{

    if ( MCUCTRL->TPIUCTRL == 0 )
    {
        //
        // This is a disable without enable, which could be the case with some
        // earlier versions of SBL. To avoid a hang, ITM (particularly TPIU
        // clock) must first be enabled.
        //
        am_hal_itm_enable();
    }

    //
    // Make sure the ITM/TPIU is not busy.
    //
    am_hal_itm_not_busy();

    //
    // Make sure the ITM_TCR is unlocked.
    //
    ITM->LAR = ITM_LAR_KEYVAL;

    //
    // Disable the ITM.
    // #### INTERNAL BEGIN ####
    // Thoughts:
    // - This weird sequence is probably necessary because even if the last few
    //   characters have gotten through the ITM, the chars may not actually
    //   be output yet (depending on baud rate, etc).  So if we disable TRCENA
    //   and/or cut off the TPIU clock too soon, corruption may occur, not only
    //   to the characters but to the ITM and/or TPIU registers.
    // - We probably need to make sure the characters are actually output before
    //   cutting them off.  This may require a delay or other means (the amount
    //   of delay itself would be dependent on baud rate).
    // - Whatever is ultimately done should go for Apollo, Apollo2, & Apollo3.
    // #### INTERNAL END ####
    //
    for (int ix = 0; ix < 100; ix++)
    {
        ITM->TCR &= ~_VAL2FLD(ITM_TCR_ITMENA, 1);
        while ( ITM->TCR  & (_VAL2FLD(ITM_TCR_ITMENA, 1)  |  _VAL2FLD(ITM_TCR_BUSY, 1)) );
    }

    //
    // Reset the TRCENA bit in the DEMCR register, which should disable the ITM
    // for operation.
    //
    CoreDebug->DEMCR &= ~_VAL2FLD(CoreDebug_DEMCR_TRCENA, 1);
    while ( CoreDebug->DEMCR & _VAL2FLD(CoreDebug_DEMCR_TRCENA, 1) );

    //
    // Disable the TPIU clock source in MCU control.
    //
    MCUCTRL->TPIUCTRL =
        _VAL2FLD(MCUCTRL_TPIUCTRL_CLKSEL, MCUCTRL_TPIUCTRL_CLKSEL_LOWPWR) |
        _VAL2FLD(MCUCTRL_TPIUCTRL_ENABLE, MCUCTRL_TPIUCTRL_ENABLE_DIS);
    while (MCUCTRL->TPIUCTRL);

}

//*****************************************************************************
//
// @brief Checks if itm is busy and provides a delay to flush the fifo
//
// This function disables the ARM ITM by resetting the TRCENA bit in the DEMCR
// register.
//
//*****************************************************************************
void
am_hal_itm_not_busy(void)
{
    //
    // Make sure the ITM/TPIU is not busy.
    //
    while (ITM->TCR & _VAL2FLD(ITM_TCR_BUSY, 1));

    //
    // wait for 50us for the data to flush out
    //
    am_hal_flash_delay(FLASH_CYCLES_US(50));
}

//*****************************************************************************
//
// @brief Enables tracing on a given set of ITM ports
//
// @param ui8portNum - Set ports to be enabled
//
// Enables tracing on the ports referred to by \e ui8portNum by writing the
// associated bit in the Trace Privilege Register in the ITM. The value for
// ui8portNum should be the logical OR one or more of the following values:
//
// \e ITM_PRIVMASK_0_7 - enable ports 0 through 7
// \e ITM_PRIVMASK_8_15 - enable ports 8 through 15
// \e ITM_PRIVMASK_16_23 - enable ports 16 through 23
// \e ITM_PRIVMASK_24_31 - enable ports 24 through 31
//
//*****************************************************************************
void
am_hal_itm_trace_port_enable(uint8_t ui8portNum)
{
    ITM->TPR |= (0x00000001 << (ui8portNum>>3));
}

//*****************************************************************************
//
// @brief Disable tracing on the given ITM stimulus port.
//
// @param ui8portNum
//
// Disables tracing on the ports referred to by \e ui8portNum by writing the
// associated bit in the Trace Privilege Register in the ITM. The value for
// ui8portNum should be the logical OR one or more of the following values:
//
// \e ITM_PRIVMASK_0_7 - disable ports 0 through 7
// \e ITM_PRIVMASK_8_15 - disable ports 8 through 15
// \e ITM_PRIVMASK_16_23 - disable ports 16 through 23
// \e ITM_PRIVMASK_24_31 - disable ports 24 through 31
//
//*****************************************************************************
void
am_hal_itm_trace_port_disable(uint8_t ui8portNum)
{
    ITM->TPR &= ~(0x00000001 << (ui8portNum >> 3));
}

//*****************************************************************************
//
// @brief Poll the given ITM stimulus register until not busy.
//
// @param ui32StimReg - stimulus register
//
// @return true if not busy, false if busy (timed out or other error).
//
//*****************************************************************************
bool
am_hal_itm_stimulus_not_busy(uint32_t ui32StimReg)
{
    uint32_t ui32StimAddr = (uint32_t)&ITM->PORT[0] + (4 * ui32StimReg);

    //
    // Busy waiting until it is available, non-zero means ready.
    //
    while ( !AM_REGVAL(ui32StimAddr) );

    return true;
}

//*****************************************************************************
//
// @brief Writes a 32-bit value to the given ITM stimulus register.
//
// @param ui32StimReg - stimulus register
// @param ui32Value - value to be written.
//
// Write a word to the desired stimulus register.
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
}

//*****************************************************************************
//
// @brief Writes a short to the given ITM stimulus register.
//
// @param ui32StimReg - stimulus register
// @param ui16Value - short to be written.
//
// Write a short to the desired stimulus register.
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
}

//*****************************************************************************
//
// @brief Writes a byte to the given ITM stimulus register.
//
// @param ui32StimReg - stimulus register
// @param ui8Value - byte to be written.
//
// Write a byte to the desired stimulus register.
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
}

//*****************************************************************************
//
// @brief Sends a Sync Packet.
//
// Sends a sync packet. This can be useful for external software should it
// become out of sync with the ITM stream.
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
}

//*****************************************************************************
//
// @brief Poll the print stimulus registers until not busy.
//
// @return true if not busy, false if busy (timed out or other error).
//
//*****************************************************************************
bool
am_hal_itm_print_not_busy(void)
{
    //
    // Poll stimulus register allocated for printing.
    //
    am_hal_itm_stimulus_not_busy(0);


    return true;
}

//*****************************************************************************
//
// @brief Prints a char string out of the ITM.
//
// @param pcString pointer to the character sting
//
// This function prints a sting out of the ITM.
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
}
//*****************************************************************************
//
// End Doxygen group.
//! @}
//
//*****************************************************************************
