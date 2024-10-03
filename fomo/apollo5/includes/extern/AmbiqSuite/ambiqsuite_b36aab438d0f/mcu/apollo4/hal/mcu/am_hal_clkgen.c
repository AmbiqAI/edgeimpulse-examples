// ****************************************************************************
//
//  am_hal_clkgen.c
//! @file
//!
//! @brief Functions for interfacing with the CLKGEN.
//!
//! @addtogroup clkgen4 Clock Generator (CLKGEN)
//! @ingroup apollo4hal
//! @{
//
// ****************************************************************************

// ****************************************************************************
//
// ${copyright}
//
// This is part of revision ${version} of the AmbiqSuite Development Package.
//
// ****************************************************************************

#include <stdint.h>
#include <stdbool.h>
#include "am_mcu_apollo.h"


// ****************************************************************************
//
//  am_hal_clkgen_control()
//      Apply various specific commands/controls on the CLKGEN module.
//
// ****************************************************************************
uint32_t
am_hal_clkgen_control(am_hal_clkgen_control_e eControl, void *pArgs)
{
    uint32_t ui32Regval;

    switch ( eControl )
    {
        case AM_HAL_CLKGEN_CONTROL_LFRC_START:
            CLKGEN->OCTRL_b.STOPRC = CLKGEN_OCTRL_STOPRC_EN;
            break;

        case AM_HAL_CLKGEN_CONTROL_XTAL_START:
            CLKGEN->OCTRL_b.STOPXT = CLKGEN_OCTRL_STOPXT_EN;
            break;

        case AM_HAL_CLKGEN_CONTROL_LFRC_STOP:
            CLKGEN->OCTRL_b.STOPRC = CLKGEN_OCTRL_STOPRC_STOP;
            break;

        case AM_HAL_CLKGEN_CONTROL_XTAL_STOP:
// #### INTERNAL BEGIN ####
            // TODO - Fixme.  What is Errata #?
            // Software Workaround to guarantee proper function of HFADJ.
            //if (APOLLO3_B0)//?
            //{
              //MCUCTRL->XTALCTRL_b.XTALICOMPTRIM = 1;
            //}
// #### INTERNAL END ####
            CLKGEN->OCTRL_b.STOPXT = CLKGEN_OCTRL_STOPXT_STOP;
            break;

        case AM_HAL_CLKGEN_CONTROL_RTC_SEL_LFRC:
            CLKGEN->OCTRL_b.OSEL = CLKGEN_OCTRL_OSEL_RTC_LFRC;
            break;

        case AM_HAL_CLKGEN_CONTROL_RTC_SEL_XTAL:
            CLKGEN->OCTRL_b.OSEL = CLKGEN_OCTRL_OSEL_RTC_XT;
            break;

        case AM_HAL_CLKGEN_CONTROL_HFADJ_ENABLE:
// #### INTERNAL BEGIN ####
            // TODO - Fixme.  What is Errata #?
            // Software Workaround to guarantee proper function of HFADJ.
            //if (APOLLO3_B0) //?
            //{
              //MCUCTRL->XTALCTRL_b.XTALICOMPTRIM = 3;
              //am_hal_flash_delay(FLASH_CYCLES_US(1000));
            //}
// #### INTERNAL END ####
            if ( pArgs == 0 )
            {
                ui32Regval =
                    _VAL2FLD(CLKGEN_HFADJ_HFADJGAIN, CLKGEN_HFADJ_HFADJGAIN_Gain_of_1_in_32) |   /* Slowest attack possible */
                    _VAL2FLD(CLKGEN_HFADJ_HFWARMUP, CLKGEN_HFADJ_HFWARMUP_1SEC)             |   /* Default value */
                    _VAL2FLD(CLKGEN_HFADJ_HFXTADJ, 0xB70)                                   |   /* Default value */
                    _VAL2FLD(CLKGEN_HFADJ_HFADJCK, CLKGEN_HFADJ_HFADJCK_4SEC)               |   /* Default value */
                    _VAL2FLD(CLKGEN_HFADJ_HFADJEN, CLKGEN_HFADJ_HFADJEN_EN);
            }
            else
            {
                ui32Regval = *(uint32_t*)pArgs;
            }

            //
            // Make sure the ENABLE bit is set.
            //
            ui32Regval |= _VAL2FLD(CLKGEN_HFADJ_HFADJEN, CLKGEN_HFADJ_HFADJEN_EN);
            CLKGEN->HFADJ = ui32Regval;
            break;

        case AM_HAL_CLKGEN_CONTROL_HFADJ_DISABLE:
            CLKGEN->HFADJ_b.HFADJEN = 0;
            break;

// #### INTERNAL BEGIN ####
#if 0   // FIXME, the XTAL24MCTRL register was removed from the Falcon design
        // on 12/31/19. Any replacement is currently unknown.
        case AM_HAL_CLKGEN_CONTROL_XTAL24M_ENABLE:
            CLKGEN->XTAL24MCTRL_b.XTAL24MEN = 1;
            break;

        case AM_HAL_CLKGEN_CONTROL_XTAL24M_DISABLE:
            CLKGEN->XTAL24MCTRL_b.XTAL24MEN = 0;
            break;

        case AM_HAL_CLKGEN_CONTROL_XTAL24MDS_0:
            CLKGEN->XTAL24MCTRL_b.XTAL24MDS = 0;
            break;

        case AM_HAL_CLKGEN_CONTROL_XTAL24MDS_1:
            CLKGEN->XTAL24MCTRL_b.XTAL24MDS = 1;
            break;

        case AM_HAL_CLKGEN_CONTROL_XTAL24MDS_2:
            CLKGEN->XTAL24MCTRL_b.XTAL24MDS = 2;
            break;

        case AM_HAL_CLKGEN_CONTROL_XTAL24MDS_3:
            CLKGEN->XTAL24MCTRL_b.XTAL24MDS = 3;
            break;

        case AM_HAL_CLKGEN_CONTROL_XTAL24MDS_4:
            CLKGEN->XTAL24MCTRL_b.XTAL24MDS = 4;
            break;

        case AM_HAL_CLKGEN_CONTROL_XTAL24MDS_5:
            CLKGEN->XTAL24MCTRL_b.XTAL24MDS = 5;
            break;

        case AM_HAL_CLKGEN_CONTROL_XTAL24MDS_6:
            CLKGEN->XTAL24MCTRL_b.XTAL24MDS = 6;
            break;

        case AM_HAL_CLKGEN_CONTROL_XTAL24MDS_7:
            CLKGEN->XTAL24MCTRL_b.XTAL24MDS = 7;
            break;
#endif
// #### INTERNAL END ####
        case AM_HAL_CLKGEN_CONTROL_DCCLK_ENABLE:
            CLKGEN->DISPCLKCTRL_b.DCCLKEN = 1;
            break;

        case AM_HAL_CLKGEN_CONTROL_DCCLK_DISABLE:
            CLKGEN->DISPCLKCTRL_b.DCCLKEN = 0;
            break;

        case AM_HAL_CLKGEN_CONTROL_DISPCLKSEL_OFF:
            CLKGEN->DISPCLKCTRL_b.DISPCLKSEL = 0;
            break;

        case AM_HAL_CLKGEN_CONTROL_DISPCLKSEL_HFRC48:
            CLKGEN->DISPCLKCTRL_b.DISPCLKSEL = 1;
            break;

        case AM_HAL_CLKGEN_CONTROL_DISPCLKSEL_HFRC96:
            CLKGEN->DISPCLKCTRL_b.DISPCLKSEL = 2;
            break;

        case AM_HAL_CLKGEN_CONTROL_DISPCLKSEL_DPHYPLL:
            CLKGEN->DISPCLKCTRL_b.DISPCLKSEL = 3;
            break;

        case AM_HAL_CLKGEN_CONTROL_PLLCLK_ENABLE:
            CLKGEN->DISPCLKCTRL_b.PLLCLKEN = 1;
            break;

        case AM_HAL_CLKGEN_CONTROL_PLLCLK_DISABLE:
            CLKGEN->DISPCLKCTRL_b.PLLCLKEN = 0;
            break;

        case AM_HAL_CLKGEN_CONTROL_PLLCLKSEL_OFF:
            CLKGEN->DISPCLKCTRL_b.PLLCLKSEL = 0;
            break;

        case AM_HAL_CLKGEN_CONTROL_PLLCLKSEL_HFRC12:
            CLKGEN->DISPCLKCTRL_b.PLLCLKSEL = 1;
            break;

        case AM_HAL_CLKGEN_CONTROL_PLLCLKSEL_HFRC24:
            CLKGEN->DISPCLKCTRL_b.PLLCLKSEL = 2;
            break;

        case AM_HAL_CLKGEN_CONTROL_PLLCLKSEL_HFXT:
            CLKGEN->DISPCLKCTRL_b.PLLCLKSEL = 3;
            break;

        default:
            return AM_HAL_STATUS_INVALID_ARG;
    }

    //
    // Return success status.
    //
    return AM_HAL_STATUS_SUCCESS;

} // am_hal_clkgen_control()

// ****************************************************************************
//
//  am_hal_clkgen_status_get()
//  This function returns the current value of various CLKGEN statuses.
//
// ****************************************************************************
uint32_t
am_hal_clkgen_status_get(am_hal_clkgen_status_t *psStatus)
{
    uint32_t ui32Status;

    if ( psStatus == NULL )
    {
        return AM_HAL_STATUS_INVALID_ARG;
    }

    // FIXME TODO: The frequency is not static and should be determined at runtime.
    psStatus->ui32SysclkFreq = AM_HAL_CLKGEN_FREQ_MAX_HZ;

    ui32Status = CLKGEN->STATUS;

    psStatus->eRTCOSC =
        _FLD2VAL(CLKGEN_STATUS_OMODE, ui32Status)   ?
            AM_HAL_CLKGEN_STATUS_RTCOSC_LFRC        :
            AM_HAL_CLKGEN_STATUS_RTCOSC_XTAL;

    psStatus->bXtalFailure =
        _FLD2VAL(CLKGEN_STATUS_OSCF, ui32Status);

    return AM_HAL_STATUS_SUCCESS;

} // am_hal_clkgen_status_get()

// ****************************************************************************
//
//  am_hal_clkgen_clkout_enable()
//  This function is used to select and enable CLKOUT.
//
// ****************************************************************************
uint32_t
am_hal_clkgen_clkout_enable(bool bEnable, am_hal_clkgen_clkout_e eClkSelect)
{
    if ( !bEnable )
    {
        CLKGEN->CLKOUT_b.CKEN = 0;
    }

    //
    // Do a basic validation of the eClkSelect parameter.
    // Not every value in the range is valid, but at least this simple check
    //  provides a reasonable chance that the parameter is valid.
    //
    if ( eClkSelect <= (am_hal_clkgen_clkout_e)AM_HAL_CLKGEN_CLKOUT_MAX )
    {
        //
        // Are we actually changing the frequency?
        //
        if ( CLKGEN->CLKOUT_b.CKSEL != eClkSelect )
        {
            //
            // Disable before changing the clock
            //
            CLKGEN->CLKOUT_b.CKEN = 0;

            //
            // Set the new clock select
            //
            CLKGEN->CLKOUT_b.CKSEL = eClkSelect;
        }

        //
        // Enable/disable as requested.
        //
        CLKGEN->CLKOUT_b.CKEN = bEnable;
    }
    else
    {
        return AM_HAL_STATUS_INVALID_ARG;
    }

    //
    // Return success status.
    //
    return AM_HAL_STATUS_SUCCESS;

} // am_hal_clkgen_clkout_enable()

// #### INTERNAL BEGIN ####
#if 0   // FIXME, the XTAL24MCTRL register was removed from the Falcon design
        // on 12/31/19. Any replacement is currently unknown.
// ****************************************************************************
//
//  am_hal_clkgen_xtal24mctrl_enable()
//  This function is used to enable XTAL 24MHz and select drive strength.
//
// ****************************************************************************
/*uint32_t
am_hal_clkgen_xtal24mctrl_enable(bool bEnable, am_hal_clkgen_xtal24mctrl_e eDSSelect)
{
    if ( !bEnable )
    {
        CLKGEN->XTAL24MCTRL_b.XTAL24MEN = 0;
    }

    //
    // Do a basic validation of the eDSSelect parameter.
    // Not every value in the range is valid, but at least this simple check
    // provides a reasonable chance that the parameter is valid.
    //
    if ( eDSSelect <= (am_hal_clkgen_xtal24mctrl_e)AM_HAL_CLKGEN_XTAL24MCTRL_XTAL24MDS_7 )
    {
        //
        // Are we actually changing the drive strength?
        //
        if ( CLKGEN->XTAL24MCTRL_b.XTAL24MDS != eDSSelect )
        {
            //
            // Disable before changing the drive strength
            //
            CLKGEN->XTAL24MCTRL_b.XTAL24MEN = 0;

            //
            // Set the new drive strength
            //
            CLKGEN->XTAL24MCTRL_b.XTAL24MDS = eDSSelect;
        }

        //
        // Enable/disable as requested.
        //
        CLKGEN->XTAL24MCTRL_b.XTAL24MEN = bEnable;
    }
    else
    {
        return AM_HAL_STATUS_INVALID_ARG;
    }

    //
    // Return success status.
    //
    return AM_HAL_STATUS_SUCCESS;

} // am_hal_clkgen_xtal24mctrl_enable()*/
#endif //
// #### INTERNAL END ####

/* FIXME!
// ****************************************************************************
//
//  am_hal_clkgen_dcclk_enable()
//  This function is used to enable dcclk and select clock source.
//
// ****************************************************************************
uint32_t
am_hal_clkgen_dcclk_enable(bool bEnable, CLKGEN_DISPCLKCTRL_DISPCLKSEL_Enum eDispclkSelect)
{
    if ( !bEnable )
    {
        CLKGEN->DISPCLKCTRL_b.DCCLKEN = 0;
    }

    //
    // Do a basic validation of the eDispclkSelect parameter.
    // Not every value in the range is valid, but at least this simple check
    // provides a reasonable chance that the parameter is valid.
    //
    if ( eDispclkSelect <= (CLKGEN_DISPCLKCTRL_DISPCLKSEL_Enum)CLKGEN_DISPCLKCTRL_DISPCLKSEL_DPHYPLL )
    {
        //
        // Are we actually changing the clock source?
        //
        if ( CLKGEN->DISPCLKCTRL_b.DISPCLKSEL != eDispclkSelect )
        {
            //
            // Disable before changing the clock source
            //
            CLKGEN->DISPCLKCTRL_b.DCCLKEN = 0;

            //
            // Set the new clock source
            //
            CLKGEN->DISPCLKCTRL_b.DISPCLKSEL = eDispclkSelect;
        }

        //
        // Enable/disable as requested.
        //
        CLKGEN->DISPCLKCTRL_b.DCCLKEN = bEnable;
    }
    else
    {
        return AM_HAL_STATUS_INVALID_ARG;
    }

    //
    // Return success status.
    //
    return AM_HAL_STATUS_SUCCESS;

} // am_hal_clkgen_dcclk_enable()
*/


/* FIXME!
// ****************************************************************************
//
//  am_hal_clkgen_pllclk_enable()
//  This function is used to enable pll clock and select pll reference clock.
//
// ****************************************************************************
uint32_t
am_hal_clkgen_pllclk_enable(bool bEnable, CLKGEN_DISPCLKCTRL_PLLCLKSEL_Enum ePllclkSelect)
{
    if ( !bEnable )
    {
        CLKGEN->DISPCLKCTRL_b.PLLCLKEN = 0;
    }

    //
    // Do a basic validation of the ePllclkSelect parameter.
    // Not every value in the range is valid, but at least this simple check
    // provides a reasonable chance that the parameter is valid.
    //
    if ( ePllclkSelect <= (CLKGEN_DISPCLKCTRL_PLLCLKSEL_Enum)CLKGEN_DISPCLKCTRL_PLLCLKSEL_HFXT )
    {
        //
        // Are we actually changing the reference clock?
        //
        if ( CLKGEN->DISPCLKCTRL_b.PLLCLKSEL != ePllclkSelect )
        {
            //
            // Disable before changing the reference clock
            //
            CLKGEN->DISPCLKCTRL_b.PLLCLKEN = 0;

            //
            // Set the new reference clock
            //
            CLKGEN->DISPCLKCTRL_b.PLLCLKSEL = ePllclkSelect;
        }

        //
        // Enable/disable as requested.
        //
        CLKGEN->DISPCLKCTRL_b.PLLCLKEN = bEnable;
    }
    else
    {
        return AM_HAL_STATUS_INVALID_ARG;
    }

    //
    // Return success status.
    //
    return AM_HAL_STATUS_SUCCESS;

} // am_hal_clkgen_pllclk_enable()
*/

// ****************************************************************************
//
//  am_hal_clkgen_interrupt_enable()
//  Enable selected CLKGEN Interrupts.
//
// ****************************************************************************
uint32_t am_hal_clkgen_interrupt_enable(am_hal_clkgen_interrupt_e ui32IntMask)
{
    if ( (ui32IntMask &
            (CLKGEN_INTRPTEN_OF_Msk         |
             CLKGEN_INTRPTEN_ACC_Msk        |
             CLKGEN_INTRPTEN_ACF_Msk)) == 0 )
    {
        return AM_HAL_STATUS_INVALID_ARG;
    }

    //
    // Set the interrupt enables according to the mask.
    //
    CLKGEN->INTRPTEN |= ui32IntMask;

    //
    // Return success status.
    //
    return AM_HAL_STATUS_SUCCESS;

} // am_hal_clkgen_interrupt_enable()

// ****************************************************************************
//
//  am_hal_clkgen_interrupt_disable(
//  Disable selected CLKGEN Interrupts.
//
// ****************************************************************************
uint32_t
am_hal_clkgen_interrupt_disable(am_hal_clkgen_interrupt_e ui32IntMask)
{
    if ( (ui32IntMask &
            (CLKGEN_INTRPTEN_OF_Msk         |
             CLKGEN_INTRPTEN_ACC_Msk        |
             CLKGEN_INTRPTEN_ACF_Msk)) == 0 )
    {
        return AM_HAL_STATUS_INVALID_ARG;
    }

    //
    // Disable the interrupts.
    //
    CLKGEN->INTRPTEN &= ~ui32IntMask;

    //
    // Return success status.
    //
    return AM_HAL_STATUS_SUCCESS;

} // am_hal_clkgen_interrupt_disable()

//*****************************************************************************
//
//  am_hal_clkgen_interrupt_clear()
//  IOM interrupt clear
//
//*****************************************************************************
uint32_t
am_hal_clkgen_interrupt_clear(am_hal_clkgen_interrupt_e ui32IntMask)
{
    if ( (ui32IntMask &
            (CLKGEN_INTRPTEN_OF_Msk         |
             CLKGEN_INTRPTEN_ACC_Msk        |
             CLKGEN_INTRPTEN_ACF_Msk)) == 0 )
    {
        return AM_HAL_STATUS_INVALID_ARG;
    }

    //
    // Clear the requested interrupts.
    //
    CLKGEN->INTRPTCLR = ui32IntMask;

    //
    // Return success status.
    //
    return AM_HAL_STATUS_SUCCESS;

} // am_hal_clkgen_interrupt_clear()

// ****************************************************************************
//
//  am_hal_clkgen_interrupt_status_get()
//  Return CLKGEN interrupts.
//
// ****************************************************************************
uint32_t
am_hal_clkgen_interrupt_status_get(bool bEnabledOnly,
                                   uint32_t *pui32IntStatus)
{
    uint32_t ui32IntStatus;

    if ( !pui32IntStatus )
    {
        return AM_HAL_STATUS_INVALID_ARG;
    }

    ui32IntStatus = CLKGEN->INTRPTSTAT;

    if ( bEnabledOnly )
    {
        ui32IntStatus &= CLKGEN->INTRPTEN;
    }

    *pui32IntStatus = ui32IntStatus;

    //
    // Return success status.
    //
    return AM_HAL_STATUS_SUCCESS;

} // am_hal_clkgen_interrupt_status_get)

// ****************************************************************************
//
// This function sets the CLKGEN interrupts.
//
// ****************************************************************************
uint32_t
am_hal_clkgen_interrupt_set(am_hal_clkgen_interrupt_e ui32IntMask)
{
    if ( (ui32IntMask &
            (CLKGEN_INTRPTEN_OF_Msk         |
             CLKGEN_INTRPTEN_ACC_Msk        |
             CLKGEN_INTRPTEN_ACF_Msk)) == 0 )
    {
        return AM_HAL_STATUS_INVALID_ARG;
    }

    //
    // Set the interrupt status.
    //
    CLKGEN->INTRPTSET = ui32IntMask;

    //
    // Return success status.
    //
    return AM_HAL_STATUS_SUCCESS;

} // am_hal_clkgen_interrupt_set()



//*****************************************************************************
//
// End Doxygen group.
//! @}
//
//*****************************************************************************
