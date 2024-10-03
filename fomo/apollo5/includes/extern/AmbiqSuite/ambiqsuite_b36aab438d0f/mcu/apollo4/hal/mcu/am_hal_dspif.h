//*****************************************************************************
//
//  am_hal_dspif.h
//! @file
//!
//! @brief Functions for DSP Interface
//!
//! @addtogroup dspif DSPIF
//! @ingroup apollo4hal
//! @{

//*****************************************************************************

//*****************************************************************************
//
// ${copyright}
//
// This is part of revision ${version} of the AmbiqSuite Development Package.
//
//*****************************************************************************

#ifndef AM_HAL_DSPIF_H
#define AM_HAL_DSPIF_H

#ifdef __cplusplus
extern "C"
{
#endif

// TODO: Is there a need to allow parallel initialization of both the DSPs to save time?
// TODO: Define structure for static interrupt & wake source configuration
uint32_t am_hal_dspif_init(uint8_t coreId, void *pInterruptConfig);

// If a run-time reset is needed - likely will only be called from within the am_hal_dspif_init
uint32_t am_hal_dspif_reset(uint8_t coreId);

typedef enum
{
    AM_HAL_DSPIF_IDMA_TRIG_OFF,
    AM_HAL_DSPIF_IDMA_TRIG_ON,
    AM_HAL_DSPIF_IDMA_TRIG_PULSE,
    AM_HAL_DSPIF_IDMA_TRIG_AUTO_PULSE,
} am_hal_dspif_iDmaTrig_cfg_e;

uint32_t am_hal_dspif_iDmaTrig_config(uint8_t coreId, am_hal_dspif_iDmaTrig_cfg_e eTrig);
uint32_t am_hal_dspif_iDmaTrig_auto_config(uint8_t coreId, uint32_t srcMask); // TODO - Need to define a mask corresponding to IDMAXTRIGSRC
uint32_t am_hal_dspif_iDmaTrig_pulse(uint8_t coreId);
uint32_t am_hal_dspif_iDmaTrig_get(uint8_t coreId, bool *pbOutTrig);

uint32_t am_hal_dspif_pause(uint8_t coreId, bool bPauseResume);
#if 0 // This should be configured as part of the interrupt config - as it maps to one of the final IRQ inputs
uint32_t am_hal_dspif_set_wakesource(uint8_t coreId, bool bPauseResume);
#endif

//*****************************************************************************
//
//! @brief Set the DSP reset mode.
//!
//! @param eDSP is the instance of the DSP 0 or 1.
//! @param bResetOverride - TRUE: DSP is held in reset; FALSE: HW handles reset.
//!
//! Select the reset mode for the DSP.
//!
//! @return Status code.
//
//*****************************************************************************
extern uint32_t am_hal_dspif_dsp_reset_mode(am_hal_dsp_select_e eDSP,
                                            bool bResetOverride);

#ifdef __cplusplus
}
#endif

#endif // AM_HAL_DSPIF_H

//*****************************************************************************
//
// End Doxygen group.
//! @}
//
//*****************************************************************************
