//*****************************************************************************
//
//! @file am_hal_pdm.c
//!
//! @brief HAL implementation for the PDM module.
//!
//! @addtogroup pdm PDM - Pulse Density Modulation
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
//! @name PDM magic number for handle verification
//! @{
//
//*****************************************************************************
#define AM_HAL_MAGIC_PDM                0xF956E2

#define AM_HAL_PDM_HANDLE_VALID(h)                                            \
    ((h) &&                                                                   \
    (((am_hal_handle_prefix_t *)(h))->s.bInit) &&                             \
    (((am_hal_handle_prefix_t *)(h))->s.magic == AM_HAL_MAGIC_PDM))
//! @}

//*****************************************************************************
//
//! Convenience macro for passing errors
//
//*****************************************************************************
#define RETURN_ON_ERROR(x)                                                    \
    if ((x) != AM_HAL_STATUS_SUCCESS)                                         \
    {                                                                         \
        return (x);                                                           \
    };

//*****************************************************************************
//
//! Abbreviation for validating handles and returning errors
//
//*****************************************************************************
#ifndef AM_HAL_DISABLE_API_VALIDATION

#define AM_HAL_PDM_HANDLE_CHECK(h)                                            \
    if (!AM_HAL_PDM_HANDLE_VALID(h))                                          \
    {                                                                         \
        return AM_HAL_STATUS_INVALID_HANDLE;                                  \
    }

#else

#define AM_HAL_PDM_HANDLE_CHECK(h)

#endif // AM_HAL_DISABLE_API_VALIDATION

//*****************************************************************************
//
//! @def USE_PDM_TWO_STAGE_DMA
//! @brief Enable the two-stage DMA request pipeline feature.
//!
//! This feature is added to ease SW ISR latency requirement to reload next
//! DMA request when the current DMA request is completed.
//!
//! To disable the 2-stage DMA feature, undefine this macro. The user needs to
//! ensure that the response time of the DMACPL ISR is less than the time it
//! takes for the PDM FIFO (typically with a size of 16 words) to become filled,
//! or else data loss may occur.
//!
//! To enable the 2-stage DMA feature, define ths macro. The user has a longer
//! response time for the DMACPL ISR, which is approximately equal to the time
//! it takes to fill the entire DMA buffer.
//
//*****************************************************************************
// #### INTERNAL BEGIN ####
// Please refer to the following wiki link for detailed information.
// https://ambiqmicro.atlassian.net/wiki/spaces/C/pages/3551854611/Two-Stage+DMA+Request+Pipeline+Feature
// #### INTERNAL END ####
#ifndef AM_HAL_DISABLE_PDM_TWO_STAGE_DMA
#define USE_PDM_TWO_STAGE_DMA
#else
#undef  USE_PDM_TWO_STAGE_DMA
#endif

#if defined(AM_PART_APOLLO5B) && defined(USE_PDM_TWO_STAGE_DMA)
#define CLEAR_DMACPL_SAFELY
#endif

// #### INTERNAL BEGIN ####
// Assuming Lowest Possible PDM_CLK that a customer will set is 2.048MHz,
// The time needed for 2 clock cycle is: 2/2.048MHz = 0.975625us. delay
// 2us to gain some margin.
// #### INTERNAL END ####
#define LL_MUX_SWITCH_DELAY_US      (2)
#define FORCE_HFRC_ON               (*(volatile uint32_t*)0x400200C0 |=  1)
#define RELEASE_HFRC                (*(volatile uint32_t*)0x400200C0 &= ~1)
#define XTHS_IS_ENABLED             ((MCUCTRL->XTALHSCTRL & MCUCTRL_XTALHSCTRL_XTALHSPDNB_Msk) == 1)

//*****************************************************************************
//
// Set the PDM CLKGEN mux.
//
// Disable CLKEN to avoid clock glitches.
//
//*****************************************************************************
#define set_clkgen_mux(x)           do {                                      \
    uint32_t clken_38f98 = PDMn(ui32Module)->CTRL_b.CLKEN;                    \
    PDMn(ui32Module)->CTRL_b.CLKEN  = 0;                                      \
    PDMn(ui32Module)->CTRL_b.CLKSEL = x;                                      \
    PDMn(ui32Module)->CTRL_b.CLKEN  = clken_38f98;                            \
} while(0)

//*****************************************************************************
//
// Extract the mux selection from a given clock.
//
//*****************************************************************************
#define extract_ll_mux(x)     ((x & AM_HAL_PDM_CLK_LL_MUX_MSK)     >> AM_HAL_PDM_CLK_LL_MUX_POS)
#define extract_clkgen_mux(x) ((x & AM_HAL_PDM_CLK_CLKGEN_MUX_MSK) >> AM_HAL_PDM_CLK_CLKGEN_MUX_POS)


//*****************************************************************************
//
//! Is Register State Valid
//
//*****************************************************************************
typedef struct
{
    bool bValid;
}
am_hal_pdm_register_state_t;

//*****************************************************************************
//
//! Structure for handling PDM HAL state information.
//
//*****************************************************************************
typedef struct
{
    am_hal_handle_prefix_t prefix;
    am_hal_pdm_register_state_t sRegState;
    uint32_t ui32Module;

    //
    //! DMA transaction Tranfer Control Buffer.
    //
    uint32_t            ui32BufferPing;
    uint32_t            ui32BufferPong;
    uint32_t            ui32BufferPtr;
    uint32_t            ui32BufferSizeBytes;

    //
    //! Selected Clock Source
    //
    am_hal_pdm_clkspd_e eClkSource;

    #ifdef CLEAR_DMACPL_SAFELY
    bool                bNeedtoClearDmacpl;
    #endif
}
am_hal_pdm_state_t;

//*****************************************************************************
//
//! Structure for handling PDM register state information for power up/down
//
//*****************************************************************************
am_hal_pdm_state_t g_PDMhandles[AM_REG_PDM_NUM_MODULES];


static am_hal_clkmgr_clock_id_e
pdm_clksrc_get(am_hal_pdm_clkspd_e clksel)
{
    switch (clksel)
    {
        case AM_HAL_PDM_CLK_HFRC2_31MHZ:
            return AM_HAL_CLKMGR_CLK_ID_HFRC2;

        case AM_HAL_PDM_CLK_HFRC_24MHZ:
        case AM_HAL_PDM_CLK_HFRC_DED:
            return AM_HAL_CLKMGR_CLK_ID_HFRC;

        case AM_HAL_PDM_CLK_HFXTAL:
        case AM_HAL_PDM_CLK_HFXTAL_DIV2:
        case AM_HAL_PDM_CLK_HFXTAL_DIV4:
            return AM_HAL_CLKMGR_CLK_ID_XTAL_HS;

        case AM_HAL_PDM_CLK_PLL:
            return AM_HAL_CLKMGR_CLK_ID_SYSPLL;

        case AM_HAL_PDM_CLK_EXTREF:
            return AM_HAL_CLKMGR_CLK_ID_EXTREF_CLK;

        default:
            return AM_HAL_CLKMGR_CLK_ID_MAX;
    }
}

//*****************************************************************************
//
// Set PDM LLMux with Mux Switcing Delay
//
//*****************************************************************************
static inline void
pdm_set_llmux_with_delay(uint32_t ui32Module, uint32_t targetLLMux)
{
    (void) ui32Module;

    am_hal_syspll_mux_select_e eLLMuxSel = AM_HAL_SYSPLL_LLMUX_PDM_SRC_CLKGEN;
    switch (targetLLMux)
    {
        case MCUCTRL_PLLMUXCTL_PDMPLLCLKSEL_PLL:
            eLLMuxSel = AM_HAL_SYSPLL_LLMUX_PDM_SRC_PLL;
            break;

        case MCUCTRL_PLLMUXCTL_PDMPLLCLKSEL_HFRC:
            eLLMuxSel = AM_HAL_SYSPLL_LLMUX_PDM_SRC_HFRC;
            break;

        case MCUCTRL_PLLMUXCTL_PDMPLLCLKSEL_CLKGEN:
            eLLMuxSel = AM_HAL_SYSPLL_LLMUX_PDM_SRC_CLKGEN;
            break;
    }
    am_hal_syspll_mux_select(eLLMuxSel, LL_MUX_SWITCH_DELAY_US);
}

//*****************************************************************************
//
// Set PDM clock by selecting the LL mux and the CLKGEN mux.
//
//*****************************************************************************
static void
pdm_clock_set(uint32_t ui32Module, am_hal_pdm_clkspd_e targetClk)
{
    //
    // Extract mux selctions from target clock.
    //
    uint32_t targetLLMux = extract_ll_mux(targetClk);
    uint32_t targetClkgen = extract_clkgen_mux(targetClk);

    //
    // Get the current LL mux status and the CLKGEN mux status.
    // Reading the LL mux status may be inaccurate if the previous LL mux switch failed.
    //
    uint32_t LLMuxStatus = MCUCTRL->PLLMUXCTL_b.PDMPLLCLKSEL;
    uint32_t clkgenStatus = PDMn(ui32Module)->CTRL_b.CLKSEL;

    //
    // The LL mux doesn't require switching.
    //
    if (targetLLMux == LLMuxStatus)
    {
        // Only change CLKGEN mux.
        if ((targetLLMux == MCUCTRL_PLLMUXCTL_PDMPLLCLKSEL_CLKGEN) && (clkgenStatus != targetClkgen))
        {
            set_clkgen_mux(targetClkgen);
        }

        return;
    }
    //
    // The LL mux requires switching, both clocks (current clock and target clock) to the LL mux input must remain active.
    //
    else
    {
        //
        // Set CLKEN = 1, which is required by CLKGEN switching in or out, so keep the restored value here.
        //
        uint32_t restoreCLKEN = PDMn(ui32Module)->CTRL_b.CLKEN;
        PDMn(ui32Module)->CTRL_b.CLKEN = 1;

        //
        // CLKGEN -> PLL/HFRC_DED, put it the first 'if' statement as it's the most common case.
        //
        if (LLMuxStatus == MCUCTRL_PLLMUXCTL_PDMPLLCLKSEL_CLKGEN)
        {
            if (clkgenStatus == PDM0_CTRL_CLKSEL_OFF)
            {
                set_clkgen_mux(PDM0_CTRL_CLKSEL_HFRC_24MHz);
            }
            pdm_set_llmux_with_delay(ui32Module, targetLLMux);
            set_clkgen_mux(PDM0_CTRL_CLKSEL_OFF);
        }
        //
        // PLL -> CLKGEN/HFRC_DED
        //
        else if (LLMuxStatus == MCUCTRL_PLLMUXCTL_PDMPLLCLKSEL_PLL)
        {
            if (targetLLMux == MCUCTRL_PLLMUXCTL_PDMPLLCLKSEL_CLKGEN)
            {
                if (clkgenStatus == PDM0_CTRL_CLKSEL_OFF)
                {
                    set_clkgen_mux(PDM0_CTRL_CLKSEL_HFRC_24MHz);
                }
                // Switch the LL mux first, then switch the CLKGEN mux.
                pdm_set_llmux_with_delay(ui32Module, MCUCTRL_PLLMUXCTL_PDMPLLCLKSEL_CLKGEN);
                set_clkgen_mux(targetClkgen);
            }
            else if (targetLLMux == MCUCTRL_PLLMUXCTL_PDMPLLCLKSEL_HFRC)
            {
                pdm_set_llmux_with_delay(ui32Module, MCUCTRL_PLLMUXCTL_PDMPLLCLKSEL_HFRC);
            }
        }
        //
        // HFRC_DED -> CLKGEN/PLL
        //
        else if (LLMuxStatus == MCUCTRL_PLLMUXCTL_PDMPLLCLKSEL_HFRC)
        {
            FORCE_HFRC_ON;
            if (targetLLMux == MCUCTRL_PLLMUXCTL_PDMPLLCLKSEL_CLKGEN)
            {
                if (clkgenStatus == PDM0_CTRL_CLKSEL_OFF)
                {
                    set_clkgen_mux(PDM0_CTRL_CLKSEL_HFRC_24MHz);
                }
                // Switch the LL mux first, then switch the CLKGEN mux.
                pdm_set_llmux_with_delay(ui32Module, MCUCTRL_PLLMUXCTL_PDMPLLCLKSEL_CLKGEN);
                set_clkgen_mux(targetClkgen);
            }
            else if (targetLLMux == MCUCTRL_PLLMUXCTL_PDMPLLCLKSEL_PLL)
            {
                pdm_set_llmux_with_delay(ui32Module, MCUCTRL_PLLMUXCTL_PDMPLLCLKSEL_PLL);
            }
            RELEASE_HFRC;
        }

        //
        // Restore CLKEN.
        //
        PDMn(ui32Module)->CTRL_b.CLKEN = restoreCLKEN;

        return;
    }
}

//*****************************************************************************
//
// am_hal_pdm_initialize
//
//*****************************************************************************
uint32_t
am_hal_pdm_initialize(uint32_t ui32Module, void **ppHandle)
{
    //
    // Check that the request module is in range.
    //
    if (ui32Module >= AM_REG_PDM_NUM_MODULES)
    {
        return AM_HAL_STATUS_OUT_OF_RANGE;
    }

    //
    // Check for valid arguements.
    //
    if (!ppHandle)
    {
        return AM_HAL_STATUS_INVALID_ARG;
    }

    //
    // Check if the handle is unallocated.
    //
    if (g_PDMhandles[ui32Module].prefix.s.bInit)
    {
        return AM_HAL_STATUS_INVALID_OPERATION;
    }

    //
    // Initialize the handle.
    //
    g_PDMhandles[ui32Module].prefix.s.bInit = true;
    g_PDMhandles[ui32Module].prefix.s.magic = AM_HAL_MAGIC_PDM;
    g_PDMhandles[ui32Module].ui32Module = ui32Module;
    g_PDMhandles[ui32Module].sRegState.bValid = false;
    #ifdef CLEAR_DMACPL_SAFELY
    g_PDMhandles[ui32Module].bNeedtoClearDmacpl = false;
    #endif

    //
    // Return the handle.
    //
    *ppHandle = (void *)&g_PDMhandles[ui32Module];

    //
    // Return the status.
    //
    return AM_HAL_STATUS_SUCCESS;
}

//*****************************************************************************
//
// De-Initialization function.
//
//*****************************************************************************
uint32_t
am_hal_pdm_deinitialize(void *pHandle)
{
    //
    // Check the handle.
    //
    AM_HAL_PDM_HANDLE_CHECK(pHandle);

    am_hal_pdm_state_t *pState = (am_hal_pdm_state_t *)pHandle;

    //
    // Reset the handle.
    //
    pState->prefix.s.bInit = false;
    pState->prefix.s.magic = 0;
    pState->ui32Module = 0;

    //
    // Return the status.
    //
    return AM_HAL_STATUS_SUCCESS;
}

//*****************************************************************************
//
// Power control function.
//
//*****************************************************************************
uint32_t
am_hal_pdm_power_control(void *pHandle,
                         am_hal_sysctrl_power_state_e ePowerState,
                         bool bRetainState)
{
    //
    // Check the handle.
    //
    AM_HAL_PDM_HANDLE_CHECK(pHandle);

    am_hal_pdm_state_t *pState = (am_hal_pdm_state_t *) pHandle;
    uint32_t ui32Module = pState->ui32Module;

    am_hal_pwrctrl_periph_e ePDMPowerModule =
        (am_hal_pwrctrl_periph_e)(AM_HAL_PWRCTRL_PERIPH_PDM0 + ui32Module);

    //
    // Decode the requested power state and update PDM operation accordingly.
    //
    switch (ePowerState)
    {
        //
        // Turn on the PDM.
        //
        case AM_HAL_SYSCTRL_WAKE:
            //
            // Make sure we don't try to restore an invalid state.
            //
            if (bRetainState && !pState->sRegState.bValid)
            {
                return AM_HAL_STATUS_INVALID_OPERATION;
            }
// #### INTERNAL BEGIN ####
#ifdef HFRC2_ON_WA
            //
            // HFRC2 ON request.
            //
            am_hal_clkgen_control(AM_HAL_CLKGEN_CONTROL_HFRC2_ON_REQ, false);
#endif // HFRC2_ON_WA
// #### INTERNAL END ####
            //
            // Enable power control.
            //
            am_hal_pwrctrl_periph_enable(ePDMPowerModule);

            if (bRetainState)
            {
                //
                // Restore PDM registers
                //
                AM_CRITICAL_BEGIN;

                pState->sRegState.bValid = false;

                AM_CRITICAL_END;
            }
            break;

        //
        // Turn off the PDM.
        //
        case AM_HAL_SYSCTRL_NORMALSLEEP:
        case AM_HAL_SYSCTRL_DEEPSLEEP:
            if (bRetainState)
            {
                AM_CRITICAL_BEGIN;

                pState->sRegState.bValid = true;

                AM_CRITICAL_END;
            }

            //
            // Disable power control.
            //
            am_hal_pwrctrl_periph_disable(ePDMPowerModule);
// #### INTERNAL BEGIN ####
#ifdef HFRC2_OFF_WA
            //
            // HFRC2 OFF request.
            //
            am_hal_clkgen_control(AM_HAL_CLKGEN_CONTROL_HFRC2_OFF_REQ, false);
#endif // HFRC2_ON_WA
// #### INTERNAL END ####
            break;

        default:
            return AM_HAL_STATUS_INVALID_ARG;
    }

    //
    // Return the status.
    //
    return AM_HAL_STATUS_SUCCESS;
}

//*****************************************************************************
//
// Configure the PDM.
//
//*****************************************************************************
uint32_t
am_hal_pdm_configure(void *pHandle, am_hal_pdm_config_t *psConfig)
{
    //
    // Check the handle.
    //
    AM_HAL_PDM_HANDLE_CHECK(pHandle);

    am_hal_pdm_state_t *pState = (am_hal_pdm_state_t *) pHandle;
    uint32_t ui32Module = pState->ui32Module;

    //
    // Apply the config structure settings to the CORECFG0 register.
    //
    PDMn(ui32Module)->CORECFG0_b.LRSWAP = psConfig->bLRSwap;
    PDMn(ui32Module)->CORECFG0_b.SOFTMUTE = psConfig->bSoftMute;
    // Set number of PDMA_CKO cycles during gain setting changes or soft mute
    PDMn(ui32Module)->CORECFG0_b.SCYCLES = psConfig->ui32GainChangeDelay;

    PDMn(ui32Module)->CORECFG0_b.HPGAIN = psConfig->ui32HighPassCutoff;
    PDMn(ui32Module)->CORECFG0_b.ADCHPD = psConfig->bHighPassEnable;
    // PDMA_CKO frequency divisor. Fpdma_cko = Fmclk_l/(MCLKDIV+1)
    PDMn(ui32Module)->CORECFG0_b.MCLKDIV = psConfig->ePDMAClkOutDivder;
    PDMn(ui32Module)->CORECFG0_b.SINCRATE = psConfig->ui32DecimationRate;

    PDMn(ui32Module)->CORECFG0_b.PGAL = psConfig->eLeftGain;
    PDMn(ui32Module)->CORECFG0_b.PGAR = psConfig->eRightGain;

    //
    // Program the "CORECFG1_b" registers.
    //
    PDMn(ui32Module)->CORECFG1_b.PCMCHSET = psConfig->ePCMChannels;
    // Divide down ratio for generating internal master MCLKQ.
    PDMn(ui32Module)->CORECFG1_b.DIVMCLKQ = psConfig->eClkDivider;
    // PDMA_CKO clock phase delay in terms of PDMCLK period to internal sampler
    PDMn(ui32Module)->CORECFG1_b.CKODLY = psConfig->bPDMSampleDelay;
    // Fine grain step size for smooth PGA or Softmute attenuation transition
    PDMn(ui32Module)->CORECFG1_b.SELSTEP = psConfig->eStepSize;

    //
    // Set the PDM Control register.
    //
    PDMn(ui32Module)->CTRL_b.CLKEN = 0;
    PDMn(ui32Module)->CTRL_b.PCMPACK = psConfig->bDataPacking;
    PDMn(ui32Module)->CTRL_b.CLKGATEMODE = 1;

    //
    // Save clock selection.
    //
    pState->eClkSource = psConfig->ePDMClkSpeed;

    PDMn(ui32Module)->CTRL_b.RSTB = 0;

    am_hal_delay_us(100);

    PDMn(ui32Module)->CTRL_b.RSTB = 1;

    return AM_HAL_STATUS_SUCCESS;
}

//*****************************************************************************
//
// Enable the PDM.
//
//*****************************************************************************
uint32_t
am_hal_pdm_enable(void *pHandle)
{
    //
    // Check the handle.
    //
    AM_HAL_PDM_HANDLE_CHECK(pHandle);

    uint32_t ui32Status;
    am_hal_pdm_state_t *pState = (am_hal_pdm_state_t *) pHandle;
    uint32_t ui32Module = pState->ui32Module;
    am_hal_clkmgr_user_id_e userID = AM_HAL_CLKMGR_USER_ID_PDM0;
    am_hal_clkmgr_clock_id_e clockID = pdm_clksrc_get(pState->eClkSource);

    //
    // Reset PDM module
    //
    PDMn(ui32Module)->CTRL_b.RSTB = 0;

    am_hal_delay_us(100);

    PDMn(ui32Module)->CTRL_b.RSTB = 1;

    //
    // Request HFRC for DMA operations.
    // 
    ui32Status = am_hal_clkmgr_clock_request(AM_HAL_CLKMGR_CLK_ID_HFRC, userID);
    if (AM_HAL_STATUS_SUCCESS != ui32Status)
    {
        return ui32Status;
    }

    //
    // Request PDM clock source.
    //
    ui32Status = am_hal_clkmgr_clock_request(clockID, userID);
    if (AM_HAL_STATUS_SUCCESS != ui32Status)
    {
        am_hal_clkmgr_clock_release(AM_HAL_CLKMGR_CLK_ID_HFRC, userID);
        return ui32Status;
    }

    pdm_clock_set(ui32Module, pState->eClkSource);

    //
    // Enable PDM clock
    //
    PDMn(ui32Module)->CTRL_b.CLKEN = 1;

    //
    // Enable PDM Module
    //
    PDMn(ui32Module)->CTRL_b.EN = 1;

    return AM_HAL_STATUS_SUCCESS;
}

//*****************************************************************************
//
// Reset the PDM.
//
//*****************************************************************************
uint32_t
am_hal_pdm_reset(void *pHandle)
{
    //
    // Check the handle.
    //
    AM_HAL_PDM_HANDLE_CHECK(pHandle);

    am_hal_pdm_state_t *pState = (am_hal_pdm_state_t *) pHandle;
    uint32_t ui32Module = pState->ui32Module;

    PDMn(ui32Module)->CTRL_b.RSTB = 1;

    return AM_HAL_STATUS_SUCCESS;
}

//*****************************************************************************
//
// Disable the PDM.
//
//*****************************************************************************
uint32_t
am_hal_pdm_disable(void *pHandle)
{
    //
    // Check the handle.
    //
    AM_HAL_PDM_HANDLE_CHECK(pHandle);

    am_hal_pdm_state_t *pState = (am_hal_pdm_state_t *) pHandle;
    uint32_t ui32Module = pState->ui32Module;
    am_hal_clkmgr_user_id_e userID = AM_HAL_CLKMGR_USER_ID_PDM0;
    am_hal_clkmgr_clock_id_e clockID = pdm_clksrc_get(pState->eClkSource);

    //
    // Disable PDM
    //
    PDMn(ui32Module)->CTRL_b.EN    = 0;
    PDMn(ui32Module)->CTRL_b.CLKEN = 0;

    //
    // Set PDM clock source to CLK_OFF and release HFRC and this clock.
    //
    pdm_clock_set(ui32Module, AM_HAL_PDM_CLK_OFF);
    am_hal_clkmgr_clock_release(AM_HAL_CLKMGR_CLK_ID_HFRC, userID);
    am_hal_clkmgr_clock_release(clockID, userID);

    return AM_HAL_STATUS_SUCCESS;
}

//*****************************************************************************
//
// Given the total number of bytes in a DMA transaction, find a reasonable
// threshold setting.
//
//*****************************************************************************
static uint32_t
find_dma_threshold(uint32_t ui32TotalCount)
{
    //
    // Start with a threshold value of 24, and search downward for values that
    // fit our criteria.
    //
    uint32_t ui32Threshold;
    uint32_t ui32Minimum = AM_HAL_PDM_DMA_THRESHOLD_MIN;

    for (ui32Threshold = 24; ui32Threshold >= ui32Minimum; ui32Threshold -= 4)
    {
        //
        // With our loop parameters, we've already guaranteed that the
        // threshold will be no higher than 24, and that it will be divisible
        // by 4. The only remaining requirement is that ui32TotalCount must
        // also be divisible by the threshold.
        //
        if ((ui32TotalCount % ui32Threshold) == 0)
        {
            break;
        }
    }

    //
    // If we found an appropriate value, we'll return it here. Otherwise, we
    // will return zero.
    //
    if (ui32Threshold < ui32Minimum)
    {
        ui32Threshold = 0;
    }

    return ui32Threshold;
}
// #### INTERNAL BEGIN ####
//*****************************************************************************
//
// TEST USE ONLY!
//
//*****************************************************************************
uint32_t
am_hal_pdm_find_dma_threshold(uint32_t ui32TotalCount)
{
    return find_dma_threshold(ui32TotalCount);
}
// #### INTERNAL END ####

//*****************************************************************************
//
// Starts a DMA transaction from the PDM directly to SRAM
//
//*****************************************************************************
uint32_t
am_hal_pdm_dma_start(void *pHandle, am_hal_pdm_transfer_t *pTransferCfg)
{
    //
    // Check the handle.
    //
    AM_HAL_PDM_HANDLE_CHECK(pHandle);

    am_hal_pdm_state_t *pState = (am_hal_pdm_state_t *) pHandle;
    uint32_t ui32Module = pState->ui32Module;

    //
    // Configure Pingpong Buffer
    //
    pState->ui32BufferPtr  = \
    pState->ui32BufferPing = pTransferCfg->ui32TargetAddr;
    pState->ui32BufferPong = pTransferCfg->ui32TargetAddrReverse;
    pState->ui32BufferSizeBytes = pTransferCfg->ui32TotalCount;

    //
    // Find an appropriate threshold size for this transfer.
    //
    uint32_t ui32Threshold = find_dma_threshold(pTransferCfg->ui32TotalCount);

    //
    // If we didn't find a threshold that will work, throw an error.
    //
    if (ui32Threshold == 0)
    {
        return AM_HAL_PDM_STATUS_BAD_TOTALCOUNT;
    }

    PDMn(ui32Module)->FIFOTHR = ui32Threshold;

    //
    // Reset DMA status.
    //
    PDMn(ui32Module)->DMASTAT = 0;
    #ifdef CLEAR_DMACPL_SAFELY
    pState->bNeedtoClearDmacpl = false;
    #endif

    //
    // Enable DMA
    //
    PDMn(ui32Module)->DMACFG = _VAL2FLD(PDM0_DMACFG_DMAEN, PDM0_DMACFG_DMAEN_EN);

    //
    // Configure DMA.
    //
    // If using 2-stage DMA, need to enable the NEXTDMAEN bit first. After writing
    // the address and size into the DMATARGADDRNEXT and DMATOTCOUNTNEXT registers
    // and enabling the DMAENNEXTCTRL bit, the hardware will automatically load the
    // contents of the DMATARGADDRNEXT and DMATOTCOUNTNEXT registers into the DMATARGADDR
    // and DMATOTCOUNT registers. Once the loading is complete, the DMAENNEXTCTRL
    // bit will be automatically cleared.
    // Note: automatic loading occurs only when the DMAEN bit is set to 1 and
    // the DMAENNEXTCTRL bit is not equal to 0.
    //
    #ifdef USE_PDM_TWO_STAGE_DMA
    PDMn(ui32Module)->DMACFG_b.NEXTDMAEN = 1;
    PDMn(ui32Module)->DMATARGADDRNEXT = pState->ui32BufferPing;
    PDMn(ui32Module)->DMATOTCOUNTNEXT = pState->ui32BufferSizeBytes;
    PDMn(ui32Module)->DMAENNEXTCTRL = 1;
    #else
    PDMn(ui32Module)->DMATARGADDR = pState->ui32BufferPing;
    PDMn(ui32Module)->DMATOTCOUNT = pState->ui32BufferSizeBytes;
    #endif

    //
    // Make sure the trigger is set for threshold.
    //
    PDMn(ui32Module)->DMATRIGEN_b.DTHR = 1;

    // #### INTERNAL BEGIN ####
    // Trigger DMA at FIFO 90 percent full. This signal is also used internally for AUTOHIP function
    PDMn(ui32Module)->DMATRIGEN_b.DTHR90 = 1; // DTHR90 = 0
    // #### INTERNAL END ####

    //
    // If a pong buffer is provided, and the USE_PDM_TWO_STAGE_DMA macro is defined,
    // configure the second DMA buffer immediately after the configuration of the
    // first DMA buffer is completed.
    //
    #ifdef USE_PDM_TWO_STAGE_DMA
    // "Pong = 0xFFFFFFFF" means that only the ping buffer works, otherwise, set the pong buffer to the next stage of DMA.
    if (pState->ui32BufferPong != 0xFFFFFFFF)
    {
        am_hal_delay_us(10);

        if (PDMn(ui32Module)->DMAENNEXTCTRL == 0)
        {
            pState->ui32BufferPtr = pState->ui32BufferPong;
            PDMn(ui32Module)->DMATARGADDRNEXT = pState->ui32BufferPong;
            PDMn(ui32Module)->DMATOTCOUNTNEXT = pState->ui32BufferSizeBytes;
            PDMn(ui32Module)->DMAENNEXTCTRL = 1;
        }
        else
        {
            return AM_HAL_STATUS_HW_ERR;
        }
    }
    #endif

    return AM_HAL_STATUS_SUCCESS;
}

//*****************************************************************************
//
// Stop DMA transfer.
//
//*****************************************************************************
uint32_t
am_hal_pdm_dma_stop(void *pHandle)
{
    //
    // Check the handle.
    //
    AM_HAL_PDM_HANDLE_CHECK(pHandle);

    am_hal_pdm_state_t *pState = (am_hal_pdm_state_t *) pHandle;
    uint32_t ui32Module = pState->ui32Module;

    //
    // Disable DMA.
    //
    PDMn(ui32Module)->DMACFG_b.DMAEN = 0;

    //
    // Clear TOTCOUNT register.
    //
    #ifdef USE_PDM_TWO_STAGE_DMA
    PDMn(ui32Module)->DMATOTCOUNTNEXT = 0;
    PDMn(ui32Module)->DMATOTCOUNT     = 0;
    #else
    PDMn(ui32Module)->DMATOTCOUNT     = 0;
    #endif

    //
    // Disable and clear FIFOTHR interrupt.
    //
    #ifdef CLEAR_DMACPL_SAFELY
    pState->bNeedtoClearDmacpl = false;
    PDMn(ui32Module)->INTEN &= ~(AM_HAL_PDM_INT_THR);
    PDMn(ui32Module)->INTCLR = AM_HAL_PDM_INT_THR;
    #endif

    return AM_HAL_STATUS_SUCCESS;
}

//*****************************************************************************
//
// Flush the PDM FIFO
//
//*****************************************************************************
uint32_t
am_hal_pdm_fifo_flush(void *pHandle)
{
    //
    // Check the handle.
    //
    AM_HAL_PDM_HANDLE_CHECK(pHandle);

    am_hal_pdm_state_t *pState = (am_hal_pdm_state_t *) pHandle;
    uint32_t ui32Module = pState->ui32Module;

    PDMn(ui32Module)->FIFOFLUSH = 1;

    return AM_HAL_STATUS_SUCCESS;
}

//*****************************************************************************
//
// set the PDM FIFO Threshold value
//
//*****************************************************************************

uint32_t
am_hal_pdm_fifo_threshold_setup(void *pHandle, uint32_t value)
{
    //
    // Check the handle.
    //
    AM_HAL_PDM_HANDLE_CHECK(pHandle);

    am_hal_pdm_state_t *pState = (am_hal_pdm_state_t *) pHandle;
    uint32_t ui32Module = pState->ui32Module;

    PDMn(ui32Module)->FIFOTHR = value;

    return AM_HAL_STATUS_SUCCESS;
}

//*****************************************************************************
//
// PDM interrupt service routine
//
//*****************************************************************************
uint32_t am_hal_pdm_interrupt_service(void *pHandle, uint32_t ui32IntMask, am_hal_pdm_transfer_t *pTransferCfg)
{
    am_hal_pdm_state_t *pState = (am_hal_pdm_state_t *) pHandle;
    uint32_t ui32Module = pState->ui32Module;

    if (ui32IntMask & AM_HAL_PDM_INT_OVF)
    {
        // #### INTERNAL BEGIN ####
        //
        // Overflow handling is not clearing overflow status, need update.
        //
        // #### INTERNAL END ####
        am_hal_pdm_fifo_flush(pHandle);
    }

    if ((ui32IntMask & AM_HAL_PDM_INT_DCMP) && (pState->ui32BufferPong != 0xFFFFFFFF))
    {
        #ifdef USE_PDM_TWO_STAGE_DMA
        #ifdef CLEAR_DMACPL_SAFELY
        // assert(PDMn(ui32Module)->DMASTAT_b.DMACPL == 1);
        if (PDMn(ui32Module)->DMASTAT_b.DMATIP == 1)
        {
            //
            // If DMATIP is 1, it's safe to clear DMACPL.
            //
            PDMn(ui32Module)->DMASTAT_b.DMACPL = 0;
        }
        else
        {
            //
            // Otherwise, clear THR status and enable THR interrupt.
            // Clear DMACPL in the next THR ISR.
            //
            PDMn(ui32Module)->INTCLR_b.THR = 1;
            am_hal_pdm_interrupt_enable(pHandle, AM_HAL_PDM_INT_THR);
            pState->bNeedtoClearDmacpl = true;
            am_hal_sysctrl_sysbus_write_flush();
        }
        #else // !CLEAR_DMACPL_SAFELY
        PDMn(ui32Module)->DMASTAT_b.DMACPL = 0;
        #endif // !CLEAR_DMACPL_SAFELY

        if (PDMn(ui32Module)->DMAENNEXTCTRL == 0)
        {
            PDMn(ui32Module)->DMATARGADDRNEXT = pState->ui32BufferPtr = (pState->ui32BufferPtr == pState->ui32BufferPong) ? pState->ui32BufferPing : pState->ui32BufferPong;
            PDMn(ui32Module)->DMATOTCOUNTNEXT = pState->ui32BufferSizeBytes;
            PDMn(ui32Module)->DMAENNEXTCTRL = 1;
        }
        else
        {
            return AM_HAL_STATUS_HW_ERR;
        }
        #else // !USE_PDM_TWO_STAGE_DMA
        PDMn(ui32Module)->DMATARGADDR = pState->ui32BufferPtr = (pState->ui32BufferPtr == pState->ui32BufferPong) ? pState->ui32BufferPing : pState->ui32BufferPong;
        PDMn(ui32Module)->DMATOTCOUNT = pState->ui32BufferSizeBytes;
        #endif // !USE_PDM_TWO_STAGE_DMA
    }

    if (((ui32IntMask & AM_HAL_PDM_INT_THR) != 0) && ((ui32IntMask & AM_HAL_PDM_INT_DCMP) == 0))
    {
        #ifdef CLEAR_DMACPL_SAFELY
        if (pState->bNeedtoClearDmacpl)
        {
            // assert((ui32IntMask & AM_HAL_PDM_INT_DCMP) == 0);
            // assert((PDMn(ui32Module)->DMASTAT & (AM_HAL_PDM_DMASTAT_DCMP | AM_HAL_PDM_DMASTAT_DMATIP))
            //                                  == (AM_HAL_PDM_DMASTAT_DCMP | AM_HAL_PDM_DMASTAT_DMATIP));
            am_hal_pdm_interrupt_disable(pHandle, AM_HAL_PDM_INT_THR);
            PDMn(ui32Module)->DMASTAT_b.DMACPL = 0;
            pState->bNeedtoClearDmacpl = false;
        }
        #endif
    }

    return AM_HAL_STATUS_SUCCESS;
}

//*****************************************************************************
//
// Interrupt enable.
//
//*****************************************************************************
uint32_t
am_hal_pdm_interrupt_enable(void *pHandle, uint32_t ui32IntMask)
{
    //
    // Check the handle.
    //
    AM_HAL_PDM_HANDLE_CHECK(pHandle);

    am_hal_pdm_state_t *pState = (am_hal_pdm_state_t *) pHandle;
    uint32_t ui32Module = pState->ui32Module;

    PDMn(ui32Module)->INTEN |= ui32IntMask;

    return AM_HAL_STATUS_SUCCESS;
}

//*****************************************************************************
//
// Interrupt disable.
//
//*****************************************************************************
uint32_t
am_hal_pdm_interrupt_disable(void *pHandle, uint32_t ui32IntMask)
{
    //
    // Check the handle.
    //
    AM_HAL_PDM_HANDLE_CHECK(pHandle);

    am_hal_pdm_state_t *pState = (am_hal_pdm_state_t *) pHandle;
    uint32_t ui32Module = pState->ui32Module;

    PDMn(ui32Module)->INTEN &= ~ui32IntMask;

    return AM_HAL_STATUS_SUCCESS;
}

//*****************************************************************************
//
// Interrupt clear.
//
//*****************************************************************************
uint32_t
am_hal_pdm_interrupt_clear(void *pHandle, uint32_t ui32IntMask)
{
    //
    // Check the handle.
    //
    AM_HAL_PDM_HANDLE_CHECK(pHandle);

    am_hal_pdm_state_t *pState = (am_hal_pdm_state_t *) pHandle;
    uint32_t ui32Module = pState->ui32Module;

    PDMn(ui32Module)->INTCLR = ui32IntMask;
    *(volatile uint32_t*)(&PDMn(ui32Module)->INTSTAT);
    return AM_HAL_STATUS_SUCCESS;
}

//*****************************************************************************
//
// Returns the interrupt status.
//
//*****************************************************************************
uint32_t
am_hal_pdm_interrupt_status_get(void *pHandle, uint32_t *pui32Status, bool bEnabledOnly)
{
    //
    // Check the handle.
    //
    AM_HAL_PDM_HANDLE_CHECK(pHandle);

    am_hal_pdm_state_t *pState = (am_hal_pdm_state_t *) pHandle;
    uint32_t ui32Module = pState->ui32Module;

    //
    // If requested, only return the interrupts that are enabled.
    //
    if (bEnabledOnly)
    {
        *pui32Status = PDMn(ui32Module)->INTSTAT;
        *pui32Status &= PDMn(ui32Module)->INTEN;
    }
    else
    {
        *pui32Status = PDMn(ui32Module)->INTSTAT;
    }

    return AM_HAL_STATUS_SUCCESS;
}

//*****************************************************************************
//
// Get the DMA Bufffer.
//
//*****************************************************************************
uint32_t
am_hal_pdm_dma_get_buffer(void *pHandle)
{
    uint32_t ui32BufferPtr;
    am_hal_pdm_state_t *pState = (am_hal_pdm_state_t *) pHandle;

    //
    // "Pong = 0xFFFFFFFF" means that only the ping buffer works, just return the ping buffer.
    //
    if (pState->ui32BufferPong == 0xFFFFFFFF)
    {
        ui32BufferPtr = pState->ui32BufferPing;
    }
    else
    {
        #ifdef USE_PDM_TWO_STAGE_DMA
        ui32BufferPtr = pState->ui32BufferPtr;
        #else
        ui32BufferPtr = (pState->ui32BufferPtr == pState->ui32BufferPong) ? pState->ui32BufferPing : pState->ui32BufferPong;
        #endif
    }

    return ui32BufferPtr;
}

//*****************************************************************************
//
// Read the FIFO.
//
//*****************************************************************************
uint32_t
am_hal_pdm_fifo_data_read(void *pHandle)
{
    am_hal_pdm_state_t *pState = (am_hal_pdm_state_t *) pHandle;
    uint32_t ui32Module = pState->ui32Module;

    return PDMn(ui32Module)->FIFOREAD;
}

//*****************************************************************************
//
// Read the FIFOs.
//
//*****************************************************************************
uint32_t am_hal_pdm_fifo_data_reads(void *pHandle, uint8_t* buffer, uint32_t size)
{
    am_hal_pdm_state_t *pState = (am_hal_pdm_state_t *) pHandle;
    uint32_t ui32Module = pState->ui32Module;

    uint32_t fiforead;
    uint32_t buf_size = 0;

    for (uint32_t i = 0; i < size; i++)
    {
        fiforead = PDMn(ui32Module)->FIFOREAD; //left/right channel
        buffer[buf_size++] =  fiforead & 0xFF;
        buffer[buf_size++] = (fiforead & 0xFF00)>>8;
        buffer[buf_size++] = (fiforead & 0x00FF0000)>>16;
    }

    return 0;
}

//*****************************************************************************
//
// am_hal_pdm_fifo_count_get
//
//*****************************************************************************
uint32_t am_hal_pdm_fifo_count_get(void *pHandle)
{
    am_hal_pdm_state_t *pState = (am_hal_pdm_state_t *) pHandle;
    uint32_t ui32Module = pState->ui32Module;

    return PDMn(ui32Module)->FIFOCNT;
}

//*****************************************************************************
//
// am_hal_pdm_dma_state
//
//*****************************************************************************
uint32_t am_hal_pdm_dma_state(void *pHandle)
{
    am_hal_pdm_state_t *pState = (am_hal_pdm_state_t *) pHandle;
    uint32_t ui32Module = pState->ui32Module;

    return PDMn(ui32Module)->DMASTAT;
}

//*****************************************************************************
//
// End Doxygen group.
//! @}
//
//*****************************************************************************
