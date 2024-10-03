// ****************************************************************************
//
//! @file am_hal_clkgen.c
//!
//! @brief Functions for interfacing with the CLKGEN.
//!
//! @addtogroup clkgen4 CLKGEN - Clock Generator
//! @ingroup apollo5a_hal
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


#define TRIM_WR_ENABLE                         \
        AM_REGVAL(0x400401fc) = 0x2EF1543E;    \
        AM_REGVAL(0x400400f8) = 0xDF328C18;    \
        AM_REGVAL(0x40040144) = 0x8299B572;    \
        AM_REGVAL(0x40040278) = 0xA07DB9C8;

#define TRIM_WR_DISABLE                        \
        AM_REGVAL(0x400401fc) = 0;             \
        AM_REGVAL(0x400400f8) = 0;             \
        AM_REGVAL(0x40040144) = 0;             \
        AM_REGVAL(0x40040278) = 0;

uint32_t g_ui32HFRC2ReqCnt = 0;

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
// #### INTERNAL BEGIN ####
#if 0   // FAL-778 RevB0 removes OCTRL.STOPRC, OCTRL.STOPXT, and STATUS (RTCOSC, OSCF)
        case AM_HAL_CLKGEN_CONTROL_LFRC_START:
            CLKGEN->OCTRL_b.STOPRC = CLKGEN_OCTRL_STOPRC_EN;
            break;

        case AM_HAL_CLKGEN_CONTROL_XTAL_START:
            CLKGEN->OCTRL_b.STOPXT = CLKGEN_OCTRL_STOPXT_EN;
            break;
#endif
// #### INTERNAL END ####

// #### INTERNAL BEGIN ####
#if 0   // FAL-778 STOPRC and STOPXT removed for RevB
        case AM_HAL_CLKGEN_CONTROL_LFRC_STOP:
            CLKGEN->OCTRL_b.STOPRC = CLKGEN_OCTRL_STOPRC_STOP;
            break;

        case AM_HAL_CLKGEN_CONTROL_XTAL_STOP:
            // TODO - Fixme.  What is Errata #?
            // Software Workaround to guarantee proper function of HFADJ.
            //if (APOLLO3_B0)//?
            //{
              //MCUCTRL->XTALCTRL_b.XTALICOMPTRIM = 1;
            //}
            CLKGEN->OCTRL_b.STOPXT = CLKGEN_OCTRL_STOPXT_STOP;
            break;
#endif
// #### INTERNAL END ####

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
                    _VAL2FLD(CLKGEN_HFADJ_HFXTADJ, 0x5B8)                                   |   /* Default value */
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
            CLKGEN->HFADJ_b.HFADJEN = CLKGEN_HFADJ_HFADJEN_DIS;
            break;

        case AM_HAL_CLKGEN_CONTROL_HF2ADJ_ENABLE:
            //
            // set HF2ADJ for 24.576MHz output
            //
            CLKGEN->HF2ADJ1_b.HF2ADJTRIMEN        = 7;
            CLKGEN->HF2ADJ2_b.HF2ADJXTALDIVRATIO  = 2;

            //
            // 32MHz XTALHS: default or *pArgs = 0.
            // 24MHz XTALHS: *pArgs is not zero.
            //
            if ( pArgs == 0 || *((uint32_t*)pArgs) == 0)
            {
                CLKGEN->HF2ADJ2_b.HF2ADJRATIO  = 0x189374;
            }
            else
            {
                CLKGEN->HF2ADJ2_b.HF2ADJRATIO  = 0x200000;
            }
            CLKGEN->HF2ADJ0_b.HF2ADJEN = CLKGEN_HF2ADJ0_HF2ADJEN_EN;
            break;

        case AM_HAL_CLKGEN_CONTROL_HF2ADJ_DISABLE:
            CLKGEN->HF2ADJ0_b.HF2ADJEN = CLKGEN_HF2ADJ0_HF2ADJEN_DIS;
            break;

        case AM_HAL_CLKGEN_CONTROL_HFRC2_ON_REQ:
            AM_CRITICAL_BEGIN
            //
            // If this is the first request, turn HFRC2 on.
            //
            if (g_ui32HFRC2ReqCnt == 0)
            {
                if ( (g_ui32TrimVer != 0xFFFFFFFF) && (g_ui32TrimVer > MIN_PCM_TRIM_VER) )
                {
                    // #### INTERNAL BEGIN ####
                    // MCUCTRL->HFRC2_b.HFRC2PWD = 0;
                    // MCUCTRL->HFRC2_b.HFRC2SWE = 1;
                    // #### INTERNAL END ####
                    AM_REGVAL(0x400200c4) &= ~(1 << 5);
                    AM_REGVAL(0x400200c4) |=  (1 << 0);
                }
                else
                {
                    TRIM_WR_ENABLE
                    // #### INTERNAL BEGIN ####
                    // MCUCTRL->HFRC2_b.HFRC2PWD = 0;
                    // MCUCTRL->HFRC2_b.HFRC2SWE = 1;
                    // #### INTERNAL END ####
                    AM_REGVAL(0x400400c4) &= ~(1 << 5);
                    AM_REGVAL(0x400400c4) |=  (1 << 0);
                    TRIM_WR_DISABLE
                }
                am_hal_delay_us(5);
            }

            //
            // Increase the request count.
            //
            g_ui32HFRC2ReqCnt++;
            AM_CRITICAL_END
            break;

        case AM_HAL_CLKGEN_CONTROL_HFRC2_OFF_REQ:
            AM_CRITICAL_BEGIN
            if (g_ui32HFRC2ReqCnt > 0)
            {
                //
                // Decrease the request count.
                //
                g_ui32HFRC2ReqCnt--;
            }

            if (g_ui32HFRC2ReqCnt == 0)
            {
                if ( (g_ui32TrimVer != 0xFFFFFFFF) && (g_ui32TrimVer > MIN_PCM_TRIM_VER) )
                {
                    // #### INTERNAL BEGIN ####
                    // MCUCTRL->HFRC2_b.HFRC2PWD = 1;
                    // #### INTERNAL END ####
                    AM_REGVAL(0x400200c4) |=  (1 << 5);
                    AM_REGVAL(0x400200c4) |=  (1 << 0);
                }
                else
                {
                    TRIM_WR_ENABLE
                    // #### INTERNAL BEGIN ####
                    // MCUCTRL->HFRC2_b.HFRC2PWD = 1;
                    // #### INTERNAL END ####
                    AM_REGVAL(0x400400c4) |=  (1 << 5);
                    AM_REGVAL(0x400400c4) |=  (1 << 0);
                    TRIM_WR_DISABLE
                }

            }
            AM_CRITICAL_END
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

        case AM_HAL_CLKGEN_CONTROL_DBICLKSEL_DBIB_CLK:
            CLKGEN->DISPCLKCTRL_b.DBICLKSEL = CLKGEN_DISPCLKCTRL_DBICLKSEL_DBIB_CLK;
            break;

        case AM_HAL_CLKGEN_CONTROL_DBICLKSEL_FORMAT_CLK:
            CLKGEN->DISPCLKCTRL_b.DBICLKSEL = CLKGEN_DISPCLKCTRL_DBICLKSEL_format_clk;
            break;

        case AM_HAL_CLKGEN_CONTROL_DBICLKSEL_PLL_CLK:
            CLKGEN->DISPCLKCTRL_b.DBICLKSEL = CLKGEN_DISPCLKCTRL_DBICLKSEL_pll_clk;
            break;

        case AM_HAL_CLKGEN_CONTROL_DBICLKDIV2EN_ENABLE:
            CLKGEN->DISPCLKCTRL_b.DBICLKDIV2EN = 1;
            break;

        case AM_HAL_CLKGEN_CONTROL_DBICLKDIV2EN_DISABLE:
            CLKGEN->DISPCLKCTRL_b.DBICLKDIV2EN = 0;
            break;

        case AM_HAL_CLKGEN_CONTROL_DISPCLKSEL_OFF:
            CLKGEN->DISPCLKCTRL_b.DISPCLKSEL = CLKGEN_DISPCLKCTRL_DISPCLKSEL_OFF;
            break;

        case AM_HAL_CLKGEN_CONTROL_DISPCLKSEL_HFRC12:
            CLKGEN->DISPCLKCTRL_b.DISPCLKSEL = CLKGEN_DISPCLKCTRL_DISPCLKSEL_HFRC12;
            break;

        case AM_HAL_CLKGEN_CONTROL_DISPCLKSEL_HFRC24:
            CLKGEN->DISPCLKCTRL_b.DISPCLKSEL = CLKGEN_DISPCLKCTRL_DISPCLKSEL_HFRC24;
            break;

        case AM_HAL_CLKGEN_CONTROL_DISPCLKSEL_HFRC48:
            CLKGEN->DISPCLKCTRL_b.DISPCLKSEL = CLKGEN_DISPCLKCTRL_DISPCLKSEL_HFRC48;
            break;

        case AM_HAL_CLKGEN_CONTROL_DISPCLKSEL_HFRC96:
            CLKGEN->DISPCLKCTRL_b.DISPCLKSEL = CLKGEN_DISPCLKCTRL_DISPCLKSEL_HFRC96;
            break;

        case AM_HAL_CLKGEN_CONTROL_DISPCLKSEL_HFRC192:
            CLKGEN->DISPCLKCTRL_b.DISPCLKSEL = CLKGEN_DISPCLKCTRL_DISPCLKSEL_HFRC192;
            break;

        case AM_HAL_CLKGEN_CONTROL_DISPCLKSEL_DPHYPLL:
            CLKGEN->DISPCLKCTRL_b.DISPCLKSEL = CLKGEN_DISPCLKCTRL_DISPCLKSEL_DPHYPLL;
            break;

        case AM_HAL_CLKGEN_CONTROL_PLLCLK_ENABLE:
            CLKGEN->DISPCLKCTRL_b.PLLCLKEN = 1;
            break;

        case AM_HAL_CLKGEN_CONTROL_PLLCLK_DISABLE:
            CLKGEN->DISPCLKCTRL_b.PLLCLKEN = 0;
            break;

        case AM_HAL_CLKGEN_CONTROL_PLLCLKSEL_OFF:
            CLKGEN->DISPCLKCTRL_b.PLLCLKSEL = CLKGEN_DISPCLKCTRL_PLLCLKSEL_OFF;
            break;

        case AM_HAL_CLKGEN_CONTROL_PLLCLKSEL_HFRC24:
            CLKGEN->DISPCLKCTRL_b.PLLCLKSEL = CLKGEN_DISPCLKCTRL_PLLCLKSEL_HFRC_24MHz;
            break;

        case AM_HAL_CLKGEN_CONTROL_PLLCLKSEL_HFRC12:
            CLKGEN->DISPCLKCTRL_b.PLLCLKSEL = CLKGEN_DISPCLKCTRL_PLLCLKSEL_HFRC_12MHz;
            break;

        case AM_HAL_CLKGEN_CONTROL_PLLCLKSEL_HFXT:
            CLKGEN->DISPCLKCTRL_b.PLLCLKSEL = CLKGEN_DISPCLKCTRL_PLLCLKSEL_XTAL_HS;
            break;

        case AM_HAL_CLKGEN_CONTROL_PLLCLKSEL_HFXT_DIV2:
            CLKGEN->DISPCLKCTRL_b.PLLCLKSEL = CLKGEN_DISPCLKCTRL_PLLCLKSEL_XTAL_HS_div2;
            break;

        case AM_HAL_CLKGEN_CONTROL_PLLCLKSEL_EXTREFCLK:
            CLKGEN->DISPCLKCTRL_b.PLLCLKSEL = CLKGEN_DISPCLKCTRL_PLLCLKSEL_EXTREFCLK;
            break;

        case AM_HAL_CLKGEN_CONTROL_PLLCLKSEL_EXTREFCLK_DIV2:
            CLKGEN->DISPCLKCTRL_b.PLLCLKSEL = CLKGEN_DISPCLKCTRL_PLLCLKSEL_EXTREFCLK_div2;
            break;

        case AM_HAL_CLKGEN_MISC_CPUHPFREQSEL_HP192M:
            CLKGEN->MISC_b.CPUHPFREQSEL = CLKGEN_MISC_CPUHPFREQSEL_HP192MHz;
            break;

        case AM_HAL_CLKGEN_MISC_CPUHPFREQSEL_HP125M:
            CLKGEN->MISC_b.CPUHPFREQSEL = CLKGEN_MISC_CPUHPFREQSEL_HP125MHz;
            break;

        case AM_HAL_CLKGEN_MISC_CPUHPFREQSEL_HP250M:
            CLKGEN->MISC_b.CPUHPFREQSEL = CLKGEN_MISC_CPUHPFREQSEL_HP250MHz;
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
    if ( psStatus == NULL )
    {
        return AM_HAL_STATUS_INVALID_ARG;
    }

// #### INTERNAL BEGIN ####
    // FIXME TODO: The frequency is not static and should be determined at runtime.
// #### INTERNAL END ####
    psStatus->ui32SysclkFreq = AM_HAL_CLKGEN_FREQ_MAX_HZ;

// #### INTERNAL BEGIN ####
#if 1   // FAL-778 RevB0 removes OCTRL.STOPRC, OCTRL.STOPXT, and STATUS (RTCOSC, OSCF)
// #### INTERNAL END ####
    psStatus->eRTCOSC = AM_HAL_CLKGEN_STATUS_RTCOSC_LFRC;
    psStatus->bXtalFailure = false;
// #### INTERNAL BEGIN ####
#else
    uint32_t ui32Status = CLKGEN->STATUS;

    psStatus->eRTCOSC =
        _FLD2VAL(CLKGEN_STATUS_OMODE, ui32Status)   ?
            AM_HAL_CLKGEN_STATUS_RTCOSC_LFRC        :
            AM_HAL_CLKGEN_STATUS_RTCOSC_XTAL;

    psStatus->bXtalFailure =
        _FLD2VAL(CLKGEN_STATUS_OSCF, ui32Status);
#endif
// #### INTERNAL END ####

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
    // provides a reasonable chance that the parameter is valid.
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
            CLKGEN->CLKOUT_b.CKEN = CLKGEN_CLKOUT_CKEN_DIS;

            //
            // Set the new clock select
            //
            CLKGEN->CLKOUT_b.CKSEL = eClkSelect;
        }

        //
        // Enable/disable as requested.
        //
        CLKGEN->CLKOUT_b.CKEN = bEnable ? CLKGEN_CLKOUT_CKEN_EN : CLKGEN_CLKOUT_CKEN_DIS;
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

#if 0 // FIXME!
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
#endif // 0

#if 0 // FIXME!
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
#endif // 0

#if 0
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
#endif
// #### INTERNAL END ####

//*****************************************************************************
//
// End Doxygen group.
//! @}
//
//*****************************************************************************
