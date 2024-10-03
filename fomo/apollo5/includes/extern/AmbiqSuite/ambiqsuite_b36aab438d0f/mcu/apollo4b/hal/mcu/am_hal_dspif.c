//*****************************************************************************
//
//! @file am_hal_dspif.c
//!
//! @brief Functions for DSP Interface
//!
//! @addtogroup dspif_4b DPSIF - DSP Interface
//! @ingroup apollo4b_hal
//! @{

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

// #### INTERNAL BEGIN ####
//#define AM_HAL_DSPIF_DSP_PCM_RESET_DELAY_IOCLKS       8  // 24MHz clocks. TODO - FIXME: What is proper value?
// #### INTERNAL END ####
#define AM_HAL_DSPIF_DSP_PCM_RESET_DELAY_IOCLKS       8

// internal function - to be called from within am_hal_dspif_init
static uint32_t am_hal_dspif_interrupt_init(uint8_t coreId, void *pInterruptConfig)
{
    // This function is responsible for initializing the interrupt matrix & wake source for the DSP
    return AM_HAL_STATUS_INVALID_OPERATION;
}

uint32_t am_hal_dspif_reset(uint8_t coreId)
{
    // DSP0RSTOR - High then low
    return AM_HAL_STATUS_INVALID_OPERATION;
}

// TODO: Is there a need to allow parallel initialization of both the DSPs to save time?
uint32_t am_hal_dspif_init(uint8_t coreId, void *pInterruptConfig)
{
    // Code Relocation if needed
    // Set up Reset Vector Registers - DSP0STATVECSEL, DSP0RESETVEC
    // The reset vector information should come as part of the metadata information (along with the code relocation info)
    am_hal_dspif_interrupt_init(coreId, pInterruptConfig);
    // Reset Sequencing
    am_hal_dspif_reset(coreId);
    // Should also initiate the IPC handshake between the two cores
    // TODO: Should return when the IPC between the two cores is fully initialized
    return AM_HAL_STATUS_INVALID_OPERATION;
}

uint32_t am_hal_dspif_iDmaTrig_config(uint8_t coreId, am_hal_dspif_iDmaTrig_cfg_e eTrig)
{
    // DSP0IDMATRIG
    return AM_HAL_STATUS_INVALID_OPERATION;
}

uint32_t am_hal_dspif_iDmaTrig_auto_config(uint8_t coreId, uint32_t srcMask)
{
    // DSP0IDMAXTRIGSRC
    return AM_HAL_STATUS_INVALID_OPERATION;
}

uint32_t am_hal_dspif_iDmaTrig_pulse(uint8_t coreId)
{
    // DSP0IDMATRIGPULSE
    return AM_HAL_STATUS_INVALID_OPERATION;
}

uint32_t am_hal_dspif_iDmaTrig_get(uint8_t coreId, bool *pbOutTrig)
{
    // DSP0IDMATRIGSTAT
    return AM_HAL_STATUS_INVALID_OPERATION;
}

uint32_t am_hal_dspif_pause(uint8_t coreId, bool bPauseResume)
{
    // DSP0RUNSTALL
    return AM_HAL_STATUS_INVALID_OPERATION;
}
// ****************************************************************************
//
//  am_hal_dspif_dsp_reset_mode()
//  Select the DSP reset mode.
//
// ****************************************************************************
uint32_t am_hal_dspif_dsp_reset_mode(am_hal_dsp_select_e eDSP,
                                     bool bResetOverride)
{
    //
    // Set the DSP reset mode.
    //
    switch ( eDSP )
    {
        case AM_HAL_DSP0:
            if (bResetOverride)
            {
                PWRCTRL->DSP0PWRCTRL_b.DSP0PCMRSTOR = PWRCTRL_DSP0PWRCTRL_DSP0PCMRSTOR_EN;
            }
            else
            {
                PWRCTRL->DSP0PWRCTRL_b.DSP0PCMRSTOR = PWRCTRL_DSP0PWRCTRL_DSP0PCMRSTOR_DIS;
            }
            PWRCTRL->DSP0PWRCTRL_b.DSP0PCMRSTDLY = AM_HAL_DSPIF_DSP_PCM_RESET_DELAY_IOCLKS;
            break;
        case AM_HAL_DSP1:
            if (bResetOverride)
            {
                PWRCTRL->DSP1PWRCTRL_b.DSP1PCMRSTOR = PWRCTRL_DSP1PWRCTRL_DSP1PCMRSTOR_EN;
            }
            else
            {
                PWRCTRL->DSP1PWRCTRL_b.DSP1PCMRSTOR = PWRCTRL_DSP1PWRCTRL_DSP1PCMRSTOR_DIS;
            }
            PWRCTRL->DSP1PWRCTRL_b.DSP1PCMRSTDLY = AM_HAL_DSPIF_DSP_PCM_RESET_DELAY_IOCLKS;
            break;
    }

    return AM_HAL_STATUS_SUCCESS;
}

//*****************************************************************************
//
// End Doxygen group.
//! @}
//
//*****************************************************************************
