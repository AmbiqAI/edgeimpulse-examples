//*****************************************************************************
//
//! @file am_hal_itm.c
//!
//! @brief Functions for operating the instrumentation trace macrocell
//!
//! @addtogroup itm4 ITM - Instrumentation Trace Macrocell
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

#define ITM_LAR_KEYVAL  0xC5ACCE55

//*****************************************************************************
//
// Global Variables
//
//*****************************************************************************
bool gbItmEnable = false;
bool gbSWOOverride = false;

// Structure to keep track of SWO pad workaround
typedef struct
{
    uint8_t  pad;  // SWO pad option
    uint8_t  func; // FuncSel for SWO
    bool     bOverride; // Has it been overridden?
    uint32_t origVal; // Original value of the GPIO Config on this pad before override
} swo_pin_info_t;

// List of all possible SWO pads on Apollo5a
swo_pin_info_t gSwoPinInfo[] =
    {
        {3,  AM_HAL_PIN_3_SWO, 0, 0},
        {22, AM_HAL_PIN_22_SWO, 0, 0},
        {23, AM_HAL_PIN_23_SWO, 0, 0},
        {24, AM_HAL_PIN_24_SWO, 0, 0},
        {28, AM_HAL_PIN_28_SWO, 0, 0},
        {34, AM_HAL_PIN_34_SWO, 0, 0},
        {35, AM_HAL_PIN_35_SWO, 0, 0},
        {41, AM_HAL_PIN_41_SWO, 0, 0},
        {44, AM_HAL_PIN_44_SWO, 0, 0},
        {56, AM_HAL_PIN_56_SWO, 0, 0},
        {57, AM_HAL_PIN_57_SWO, 0, 0},
        {64, AM_HAL_PIN_64_SWO, 0, 0},
        {65, AM_HAL_PIN_65_SWO, 0, 0},
        {66, AM_HAL_PIN_66_SWO, 0, 0},
        {67, AM_HAL_PIN_67_SWO, 0, 0},
        {68, AM_HAL_PIN_68_SWO, 0, 0},
        {69, AM_HAL_PIN_69_SWO, 0, 0},
        {79, AM_HAL_PIN_79_SWO, 0, 0},
    };


// #### INTERNAL BEGIN ####
// TODO: setup with config structure.
// #### INTERNAL END ####
//*****************************************************************************
//
// Enables the ITM
//
//*****************************************************************************
uint32_t
am_hal_itm_enable(void)
{
    uint32_t ui32Ret;

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
    // To be able to access ITM registers, set the Trace Enable bit
    // in the Debug Exception and Monitor Control Register (DEMCR).
    //
    DCB->DEMCR |= DCB_DEMCR_TRCENA_Msk;

    //
    // Wait for the changes to take effect with a 10us timeout.
    // #### INTERNAL BEGIN ####
    // NOTE: 10us is arbitrary, but should be more than enough time.
    // #### INTERNAL END ####
    //
    ui32Ret = am_hal_delay_us_status_change(10,
                                            (uint32_t)&DCB->DEMCR,
                                            DCB_DEMCR_TRCENA_Msk,
                                            DCB_DEMCR_TRCENA_Msk );
    if ( ui32Ret != AM_HAL_STATUS_SUCCESS )
    {
        return ui32Ret;
    }

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
    gbItmEnable = true;
    return AM_HAL_STATUS_SUCCESS;

} // am_hal_itm_enable()

//*****************************************************************************
//
// Disables the ITM
//
//*****************************************************************************
uint32_t
am_hal_itm_disable(void)
{
    uint32_t ui32Ret;

// #### INTERNAL BEGIN ####
    #warning "TODO - Fix me!!! This workaround should go away in RevB."
// #### INTERNAL END ####
    gbItmEnable = false;
    if (!PWRCTRL->DEVPWRSTATUS_b.PWRSTDBG)
    {
        //
        // Workaround to avoid hanging during ITM access.
        // Power up DBG domain explicitly.
        //
        PWRCTRL->DEVPWREN_b.PWRENDBG = 1;
    }
    //
    // Before disabling, make sure all printing has completed,
    // and also that the ITM is not busy.
    //
    if ( !(am_hal_itm_print_not_busy() && am_hal_itm_not_busy()) )
    {
        return AM_HAL_STATUS_TIMEOUT;
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
    ui32Ret = am_hal_delay_us_status_change(1000,
                                            (uint32_t)&ITM->TCR,
                                            (ITM_TCR_ITMENA_Msk & ITM_TCR_BUSY_Msk),
                                            0 );
    if ( ui32Ret != AM_HAL_STATUS_SUCCESS )
    {
        return ui32Ret;     // AM_HAL_STATUS_TIMEOUT
    }

    //
    // Reset the TRCENA bit in the DEMCR register, which should disable the ITM
    // for operation.
    //
    DCB->DEMCR &= ~DCB_DEMCR_TRCENA_Msk;

    //
    // Wait for the changes to take effect with a 10us timeout.
    // #### INTERNAL BEGIN ####
    // NOTE: 10us is arbitrary, but should be more than enough time.
    // #### INTERNAL END ####
    //
    ui32Ret = am_hal_delay_us_status_change(10,
                                            (uint32_t)&DCB->DEMCR,
                                            DCB_DEMCR_TRCENA_Msk,
                                            0 );
    if ( ui32Ret != AM_HAL_STATUS_SUCCESS )
    {
        return ui32Ret;     // AM_HAL_STATUS_TIMEOUT
    }

    return AM_HAL_STATUS_SUCCESS;

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
// #### INTERNAL BEGIN ####
    #warning "TODO - Fix me!!! This workaround should go away in RevB."
// #### INTERNAL END ####
    if (!PWRCTRL->DEVPWRSTATUS_b.PWRSTDBG)
    {
        //
        // Workaround to avoid hanging during ITM access.
        // Power up DBG domain explicitly.
        //
        PWRCTRL->DEVPWREN_b.PWRENDBG = 1;
    }
    //
    // Make sure the ITM is not busy.
    //
    uint32_t ui32Ret;
    ui32Ret = am_hal_delay_us_status_change(1000,
                                            (uint32_t)&ITM->TCR,
                                            ITM_TCR_BUSY_Msk,
                                            0 );
    //
    // Short wait for any data to flush out.
    //
    am_hal_delay_us(50);

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
// #### INTERNAL BEGIN ####
    #warning "TODO - Fix me!!! This workaround should go away in RevB."
// #### INTERNAL END ####
    if (!PWRCTRL->DEVPWRSTATUS_b.PWRSTDBG)
    {
        //
        // Workaround to avoid hanging during ITM access.
        // Power up DBG domain explicitly.
        //
        PWRCTRL->DEVPWREN_b.PWRENDBG = 1;
    }
    ITM->TPR |= (0x00000001 << (ui8portNum>>3));
} // am_hal_itm_trace_port_enable()

//*****************************************************************************
//
// Disable tracing on the given ITM stimulus port.
//
//*****************************************************************************
void
am_hal_itm_trace_port_disable(uint8_t ui8portNum)
{
// #### INTERNAL BEGIN ####
    #warning "TODO - Fix me!!! This workaround should go away in RevB."
// #### INTERNAL END ####
    if (!PWRCTRL->DEVPWRSTATUS_b.PWRSTDBG)
    {
        //
        // Workaround to avoid hanging during ITM access.
        // Power up DBG domain explicitly.
        //
        PWRCTRL->DEVPWREN_b.PWRENDBG = 1;
    }
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
// #### INTERNAL BEGIN ####
    #warning "TODO - Fix me!!! This workaround should go away in RevB."
// #### INTERNAL END ####
    if (!PWRCTRL->DEVPWRSTATUS_b.PWRSTDBG)
    {
        //
        // Workaround to avoid hanging during ITM access.
        // Power up DBG domain explicitly.
        //
        PWRCTRL->DEVPWREN_b.PWRENDBG = 1;
    }
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
// DeepSleep entry workaround for active SWO pads
//
//*****************************************************************************
void
am_hal_itm_handle_deepsleep_enter(void)
{
    uint32_t i;
    if (gbItmEnable)
    {
        //
        // TODO: Do we need to flush ITM?
// #### INTERNAL BEGIN ####
        // CAYNSWS-1950
// #### INTERNAL END ####
        // SWO goes low during Core DeepSleep, which could confuse the debugger
        // Need to workaround by reconfiguring to GPIO mode and set to high
        //
        // Set the key to enable GPIO configuration.
        //
        GPIO->PADKEY = GPIO_PADKEY_PADKEY_Key;
        for (i = 0; i < sizeof(gSwoPinInfo) / sizeof(swo_pin_info_t); i++)
        {
            gSwoPinInfo[i].origVal = AM_REGVAL(&GPIO->PINCFG0 + gSwoPinInfo[i].pad);
            if ((gSwoPinInfo[i].origVal & 0xf) == gSwoPinInfo[i].func)
            {
                gbSWOOverride = true;
                gSwoPinInfo[i].bOverride = true;
                am_hal_gpio_output_set(gSwoPinInfo[i].pad);
                AM_REGVAL(&GPIO->PINCFG0 + gSwoPinInfo[i].pad) = am_hal_gpio_pincfg_output.GP.cfg;
            }
        }
        //
        // Lock GPIO configuration.
        //
        GPIO->PADKEY = 0;

    }
}

//*****************************************************************************
//
// DeepSleep exit workaround for active SWO pads
//
//*****************************************************************************
void
am_hal_itm_handle_deepsleep_exit(void)
{
    uint32_t i;
    if (gbSWOOverride)
    {
        //
        // Set the key to enable GPIO configuration.
        //
        GPIO->PADKEY = GPIO_PADKEY_PADKEY_Key;
        for (i = 0; i < sizeof(gSwoPinInfo) / sizeof(swo_pin_info_t); i++)
        {
            if (gSwoPinInfo[i].bOverride == true)
            {
                gSwoPinInfo[i].bOverride = false;
                AM_REGVAL(&GPIO->PINCFG0 + gSwoPinInfo[i].pad) = gSwoPinInfo[i].origVal;
            }
        }
        //
        // Lock GPIO configuration.
        //
        GPIO->PADKEY = 0;
        gbSWOOverride = false;
    }
}

//*****************************************************************************
//
// End Doxygen group.
//! @}
//
//*****************************************************************************
