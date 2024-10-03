// ****************************************************************************
//
//! @file am_hal_clkgen.c
//!
//! @brief Functions for interfacing with the CLKGEN.
//!
//! @addtogroup clkgen4 CLKGEN - Clock Generator
//! @ingroup apollo5b_hal
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
#include "../am_hal_clkmgr_private.h"

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
            CLKGEN->OCTRL_b.OSEL = CLKGEN_OCTRL_OSEL_LFRC_512Hz;
            break;

        case AM_HAL_CLKGEN_CONTROL_RTC_SEL_XTAL:
            CLKGEN->OCTRL_b.OSEL = CLKGEN_OCTRL_OSEL_XT_512Hz;
            break;

        case AM_HAL_CLKGEN_CONTROL_HFADJ_ENABLE:
            return am_hal_clkmgr_private_clkgen_hfadj_apply(pArgs);
            break;

        case AM_HAL_CLKGEN_CONTROL_HFADJ_DISABLE:
            return am_hal_clkmgr_private_clkgen_hfadj_disable();
            break;

        case AM_HAL_CLKGEN_CONTROL_HF2ADJ_ENABLE:
            return am_hal_clkmgr_private_clkgen_hf2adj_apply();
            break;

        case AM_HAL_CLKGEN_CONTROL_HF2ADJ_DISABLE:
            return am_hal_clkmgr_private_clkgen_hf2adj_disable();
            break;

        case AM_HAL_CLKGEN_CONTROL_HFRC2_ON_REQ:
            AM_CRITICAL_BEGIN
            //
            // If this is the first request, turn HFRC2 on.
            //
            if (g_ui32HFRC2ReqCnt == 0)
            {
                // #### INTERNAL BEGIN ####
                // MCUCTRL->HFRC2_b.HFRC2PWD = 0;
                // MCUCTRL->HFRC2_b.HFRC2SWE = 1;
                // #### INTERNAL END ####
                AM_REGVAL(0x400200c4) &= ~(1 << 5);
                AM_REGVAL(0x400200c4) |=  (1 << 0);
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
                // #### INTERNAL BEGIN ####
                // MCUCTRL->HFRC2_b.HFRC2PWD = 1;
                // #### INTERNAL END ####
                AM_REGVAL(0x400200c4) |=  (1 << 5);
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
            CLKGEN->CLKCTRL_b.DISPCTRLCLKEN = CLKGEN_CLKCTRL_DISPCTRLCLKEN_ENABLE;
            if ( CLKGEN->DISPCLKCTRL_b.DISPCLKSEL > CLKGEN_DISPCLKCTRL_DISPCLKSEL_DPHYPLL )
            {
                return AM_HAL_STATUS_INVALID_ARG;
            }
            else
            {
                CLKGEN->DISPCLKCTRL_b.DCCLKEN = 1;
                return am_hal_clkmgr_clock_request(AM_HAL_CLKMGR_CLK_ID_HFRC, AM_HAL_CLKMGR_USER_ID_DC);
            }
            break;

        case AM_HAL_CLKGEN_CONTROL_DCCLK_DISABLE:
            CLKGEN->DISPCLKCTRL_b.DCCLKEN = 0;
            CLKGEN->CLKCTRL_b.DISPCTRLCLKEN = CLKGEN_CLKCTRL_DISPCTRLCLKEN_DISABLE;
            return am_hal_clkmgr_clock_release(AM_HAL_CLKMGR_CLK_ID_HFRC, AM_HAL_CLKMGR_USER_ID_DC);
            break;

        case AM_HAL_CLKGEN_CONTROL_DBICLKSEL_DBIB_CLK:
            CLKGEN->DISPCLKCTRL_b.DBICLKSEL = CLKGEN_DISPCLKCTRL_DBICLKSEL_DBIB_CLK;
            break;

        case AM_HAL_CLKGEN_CONTROL_DBICLKSEL_FORMAT_CLK:
            CLKGEN->DISPCLKCTRL_b.DBICLKSEL = CLKGEN_DISPCLKCTRL_DBICLKSEL_FORMAT_CLK;
            break;

        case AM_HAL_CLKGEN_CONTROL_DBICLKSEL_PLL_CLK:
            CLKGEN->DISPCLKCTRL_b.DBICLKSEL = CLKGEN_DISPCLKCTRL_DBICLKSEL_PLL_CLK;
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

// #### INTERNAL BEGIN ####
#if 0 // CAYNSWS-1744 Remove CPUHPFREQSEL
        case AM_HAL_CLKGEN_MISC_CPUHPFREQSEL_HP192M:
            CLKGEN->MISC_b.CPUHPFREQSEL = CLKGEN_MISC_CPUHPFREQSEL_HP192MHz;
            break;
        case AM_HAL_CLKGEN_MISC_CPUHPFREQSEL_HP125M:
            CLKGEN->MISC_b.CPUHPFREQSEL = CLKGEN_MISC_CPUHPFREQSEL_HP125MHz;
            break;
        case AM_HAL_CLKGEN_MISC_CPUHPFREQSEL_HP250M:
            CLKGEN->MISC_b.CPUHPFREQSEL = CLKGEN_MISC_CPUHPFREQSEL_HP250MHz;
            break;
#endif // CAYNSWS-1744
// #### INTERNAL END ####
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

static am_hal_clkmgr_clock_id_e
am_hal_clkgen_clksrc_get(uint32_t clk)
{
    switch(clk)
    {
#ifdef AM_HAL_CLKMGR_MANAGE_XTAL_LS
        // XTAL
        case AM_HAL_CLKGEN_CLKOUT_XTAL_32768:
        case AM_HAL_CLKGEN_CLKOUT_XTAL_16384:
        case AM_HAL_CLKGEN_CLKOUT_XTAL_8192:
        case AM_HAL_CLKGEN_CLKOUT_XTAL_4096:
        case AM_HAL_CLKGEN_CLKOUT_XTAL_2048:
        case AM_HAL_CLKGEN_CLKOUT_XTAL_1024:
        case AM_HAL_CLKGEN_CLKOUT_XTAL_128:
        case AM_HAL_CLKGEN_CLKOUT_XTAL_4:
        case AM_HAL_CLKGEN_CLKOUT_XTAL_0_5:
        case AM_HAL_CLKGEN_CLKOUT_XTAL_0_015:
        // Not Autoenabled ("NE")
        case AM_HAL_CLKGEN_CLKOUT_XTALNE_32768:
        case AM_HAL_CLKGEN_CLKOUT_XTALNE_2048:
            return AM_HAL_CLKMGR_CLK_ID_XTAL_LS;
#endif
        // LFRC
        case AM_HAL_CLKGEN_CLKOUT_LFRC:
        case AM_HAL_CLKGEN_CLKOUT_LFRC_512:
        case AM_HAL_CLKGEN_CLKOUT_LFRC_32:
        case AM_HAL_CLKGEN_CLKOUT_LFRC_2:
        case AM_HAL_CLKGEN_CLKOUT_LFRC_0_03:
        case AM_HAL_CLKGEN_CLKOUT_LFRC_0_0010:
        // Uncalibrated LFRC
        case AM_HAL_CLKGEN_CLKOUT_ULFRC_64:
        case AM_HAL_CLKGEN_CLKOUT_ULFRC_8:
        case AM_HAL_CLKGEN_CLKOUT_ULFRC_1:
        case AM_HAL_CLKGEN_CLKOUT_ULFRC_0_25:
        case AM_HAL_CLKGEN_CLKOUT_ULFRC_0_0009:
        case AM_HAL_CLKGEN_CLKOUT_LFRCNE:
        case AM_HAL_CLKGEN_CLKOUT_LFRCNE_32:
            return AM_HAL_CLKMGR_CLK_ID_LFRC;
        // HFRC2
        case AM_HAL_CLKGEN_CLKOUT_HFRC2_250M:
        case AM_HAL_CLKGEN_CLKOUT_HFRC2_125M:
        case AM_HAL_CLKGEN_CLKOUT_HFRC2_31M:
        case AM_HAL_CLKGEN_CLKOUT_HFRC2_16M:
        case AM_HAL_CLKGEN_CLKOUT_HFRC2_8M:
        case AM_HAL_CLKGEN_CLKOUT_HFRC2_4M:
            return AM_HAL_CLKMGR_CLK_ID_HFRC2;
        default:
            return AM_HAL_CLKMGR_CLK_ID_MAX;
    }
}

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
        am_hal_clkmgr_clock_release(am_hal_clkgen_clksrc_get(CLKGEN->CLKOUT_b.CKSEL), AM_HAL_CLKMGR_USER_ID_CLKOUT);
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
            if ( bEnable )
            {
                am_hal_clkmgr_clock_release(am_hal_clkgen_clksrc_get(CLKGEN->CLKOUT_b.CKSEL), AM_HAL_CLKMGR_USER_ID_CLKOUT);
            }
            //
            // Set the new clock select
            //
            CLKGEN->CLKOUT_b.CKSEL = eClkSelect;
            if ( bEnable )
            {
                if ( (eClkSelect != AM_HAL_CLKGEN_CLKOUT_RTC_1HZ) && (eClkSelect != AM_HAL_CLKGEN_CLKOUT_CG_100) )
                {
                    am_hal_clkmgr_clock_request(am_hal_clkgen_clksrc_get(eClkSelect), AM_HAL_CLKMGR_USER_ID_CLKOUT);
                }
            }
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

uint32_t
am_hal_clkgen_hfrc2adj_ratio_calculate(uint32_t ui32RefFreq, uint32_t ui32TargetFreq, am_hal_clkgen_hfrc2adj_refclk_div_e eXtalDiv, uint32_t *pui32AdjRatio)
{
    //
    // Calculate target frequency ratio to reference frequency
    //
    float fRatio = (float)ui32TargetFreq;
    fRatio /= (ui32RefFreq / (1 << (uint8_t)eXtalDiv));

    //
    // Convert to register representation
    //
    *pui32AdjRatio = (uint32_t)(fRatio * (1 << 15));

    return AM_HAL_STATUS_SUCCESS;
}

uint32_t
am_hal_clkgen_hfrcadj_target_calculate(uint32_t ui32RefFreq, uint32_t ui32TargetFrequency, uint32_t *pui32AdjTarget)
{
    *pui32AdjTarget = (ui32TargetFrequency / ui32RefFreq);

    return AM_HAL_STATUS_SUCCESS;
}

//! @cond CLKGEN_PRIVATE_FUNC
// ****************************************************************************
//
//  am_hal_clkgen_private_hfrc_force_on()
//  This private function is used to enable/disable of force-on for HFRC
//  oscillator block.
//  Note: This API is inteded for use by HAL only. Do not call this API from
//  Application/BSP.
//
// ****************************************************************************
uint32_t am_hal_clkgen_private_hfrc_force_on(bool bForceOn)
{
    CLKGEN->MISC_b.FRCHFRC = bForceOn ? CLKGEN_MISC_FRCHFRC_FRC: CLKGEN_MISC_FRCHFRC_NOFRC;
    return AM_HAL_STATUS_SUCCESS;
}

// ****************************************************************************
//
//  am_hal_clkgen_private_hfadj_apply()
//  This private function is used to configure and enable HFADJ.
//  Note: This API is inteded for use by HAL only. Do not call this API from
//  Application/BSP.
//
// ****************************************************************************
uint32_t am_hal_clkgen_private_hfadj_apply(uint32_t ui32RegVal)
{
    //
    // Make sure the ENABLE bit is set, and set config into register
    //
    ui32RegVal |= _VAL2FLD(CLKGEN_HFADJ_HFADJEN, CLKGEN_HFADJ_HFADJEN_EN);
    CLKGEN->HFADJ = ui32RegVal;
    return AM_HAL_STATUS_SUCCESS;
}

// ****************************************************************************
//
//  am_hal_clkgen_private_hfadj_disable()
//  This private function is used to disable HFADJ.
//  Note: This API is inteded for use by HAL only. Do not call this API from
//  Application/BSP.
//
// ****************************************************************************
uint32_t am_hal_clkgen_private_hfadj_disable()
{
    CLKGEN->HFADJ_b.HFADJEN = CLKGEN_HFADJ_HFADJEN_DIS;
    return AM_HAL_STATUS_SUCCESS;
}

// ****************************************************************************
//
//  am_hal_clkgen_private_hfrc2_force_on()
//  This private function is used to enable/disable of force-on for HFRC2
//  oscillator block.
//  Note: This API is inteded for use by HAL only. Do not call this API from
//  Application/BSP.
//
// ****************************************************************************
uint32_t am_hal_clkgen_private_hfrc2_force_on(bool bForceOn)
{
    if (bForceOn)
    {
        if (CLKGEN->MISC_b.FRCHFRC2 != CLKGEN_MISC_FRCHFRC2_FRC)
        {
            CLKGEN->MISC_b.FRCHFRC2 = CLKGEN_MISC_FRCHFRC2_FRC;
            return am_hal_delay_us_status_check(100,
                                (uint32_t)&CLKGEN->CLOCKENSTAT,
                                CLKGEN_CLOCKENSTAT_HFRC2READY_Msk,
                                CLKGEN_CLOCKENSTAT_HFRC2READY_Msk,
                                true);
        }
    }
    else
    {

        CLKGEN->MISC_b.FRCHFRC2 = CLKGEN_MISC_FRCHFRC2_NOFRC;
    }

    return AM_HAL_STATUS_SUCCESS;
}

// ****************************************************************************
//
//  am_hal_clkgen_private_hf2adj_apply()
//  This private function is used to configure and enable HFADJ2.
//  Note: This API is inteded for use by HAL only. Do not call this API from
//  Application/BSP.
//
// ****************************************************************************
uint32_t am_hal_clkgen_private_hf2adj_apply(am_hal_clkgen_hfrc2adj_t* psHFRC2Adj)
{
    //
    // Check pointer validity
    //
    if (psHFRC2Adj == NULL)
    {
        return AM_HAL_STATUS_INVALID_ARG;
    }

    //
    // Set HF2ADJ configuration into CLKGEN registers
    //
    CLKGEN->HF2ADJ1_b.HF2ADJTRIMEN       = CLKGEN_HF2ADJ1_HF2ADJTRIMEN_TRIM_EN7;
    CLKGEN->HF2ADJ0_b.HF2ADJXTHSMUXSEL   = (uint32_t)psHFRC2Adj->eRefClkSel;
    CLKGEN->HF2ADJ2_b.HF2ADJXTALDIVRATIO = (uint32_t)psHFRC2Adj->eRefClkDiv;
    CLKGEN->HF2ADJ2_b.HF2ADJRATIO        = psHFRC2Adj->ui32AdjRatio;
    CLKGEN->HF2ADJ0_b.HF2ADJEN           = CLKGEN_HF2ADJ0_HF2ADJEN_EN;

    return AM_HAL_STATUS_SUCCESS;
}

// ****************************************************************************
//
//  am_hal_clkgen_private_hf2adj_disable()
//  This private function is used to disable HFADJ2.
//  Note: This API is inteded for use by HAL only. Do not call this API from
//  Application/BSP.
//
// ****************************************************************************
uint32_t am_hal_clkgen_private_hf2adj_disable()
{
    CLKGEN->HF2ADJ0_b.HF2ADJEN = CLKGEN_HF2ADJ0_HF2ADJEN_DIS;
    return AM_HAL_STATUS_SUCCESS;
}
//! @endcond CLKGEN_PRIVATE_FUNC

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
