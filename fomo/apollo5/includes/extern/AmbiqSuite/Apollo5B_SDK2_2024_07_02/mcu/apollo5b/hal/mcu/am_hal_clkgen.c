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
// Copyright (c) 2024, Ambiq Micro, Inc.
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// 1. Redistributions of source code must retain the above copyright notice,
// this list of conditions and the following disclaimer.
//
// 2. Redistributions in binary form must reproduce the above copyright
// notice, this list of conditions and the following disclaimer in the
// documentation and/or other materials provided with the distribution.
//
// 3. Neither the name of the copyright holder nor the names of its
// contributors may be used to endorse or promote products derived from this
// software without specific prior written permission.
//
// Third party software included in this distribution is subject to the
// additional license terms as defined in the /docs/licenses directory.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
// ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
// LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
// CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
// SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
// INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
// CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
// ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
// POSSIBILITY OF SUCH DAMAGE.
//
// This is part of revision release_a5b_sdk2-748191cd0 of the AmbiqSuite Development Package.
//
// ****************************************************************************

#include <stdint.h>
#include <stdbool.h>
#include "am_mcu_apollo.h"

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


        case AM_HAL_CLKGEN_CONTROL_RTC_SEL_LFRC:
            CLKGEN->OCTRL_b.OSEL = CLKGEN_OCTRL_OSEL_LFRC_512Hz;
            break;

        case AM_HAL_CLKGEN_CONTROL_RTC_SEL_XTAL:
            CLKGEN->OCTRL_b.OSEL = CLKGEN_OCTRL_OSEL_XT_512Hz;
            break;

        case AM_HAL_CLKGEN_CONTROL_HFADJ_ENABLE:
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
                CLKGEN->HF2ADJ2_b.HF2ADJRATIO  = 0xC49BA;
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
                AM_REGVAL(0x400200c4) |=  (1 << 5);
            }
            AM_CRITICAL_END
            break;

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

    psStatus->ui32SysclkFreq = AM_HAL_CLKGEN_FREQ_MAX_HZ;

    psStatus->eRTCOSC = AM_HAL_CLKGEN_STATUS_RTCOSC_LFRC;
    psStatus->bXtalFailure = false;

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


//*****************************************************************************
//
// End Doxygen group.
//! @}
//
//*****************************************************************************