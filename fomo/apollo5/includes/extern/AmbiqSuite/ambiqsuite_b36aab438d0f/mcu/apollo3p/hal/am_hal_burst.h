//*****************************************************************************
//
//! @file am_hal_burst.h
//!
//! @brief Functions for Controlling Burst Mode Operation.
//!
//! @addtogroup burstmode3p Burst - Burst Mode Functionality
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
#ifndef AM_HAL_BURST_H
#define AM_HAL_BURST_H

#ifdef __cplusplus
extern "C"
{
#endif

//*****************************************************************************
//
//! @brief Option to Enable LDO Burst mode workaound
//!
//! When entering burst mode, the 96MHz clock is active prior to Burst Active
//! is enabled. This is not an issue on all chips.
//!
//! AM_HAL_BURST_LDO_WORKAROUND:
//!      0 = Use existing sequence when entering burst mode
//!      1 = Burst LDO into Active mode and delay before enabling 96MHz CLK
//!  Default: 1
//
// #### INTERNAL BEGIN ####
// Corrects MSPI2 failure on first transaction after entering burst mode
// AS-1284, HSP20-521, HSP20-856, HSP20-863, HSP20-871
// #### INTERNAL END ####
//*****************************************************************************
#define AM_HAL_BURST_LDO_WORKAROUND         1

//*****************************************************************************
//
// Burst Mode Status enums
//
//*****************************************************************************
//
//! Avail - the result of a feature availability interrogation.
//
typedef enum
{
    AM_HAL_BURST_AVAIL,
    AM_HAL_BURST_NOTAVAIL
} am_hal_burst_avail_e;

//
//! Mode - the result of a change request.
//
typedef enum
{
    AM_HAL_BURST_MODE,
    AM_HAL_NORMAL_MODE,
} am_hal_burst_mode_e;

//*****************************************************************************
//
//! @brief Burst mode initialization function
//!
//! @param peBurstAvail - Availibility of feature
//!
//! This function initializes the Apollo3 MCU for Burst Mode operation. It does
//! not set the MCU into Burst Mode.  This should be called once at system
//! initialization if Burst Mode is going to be used in the system.
//!
//! @return status of API call.
//
//*****************************************************************************
extern uint32_t am_hal_burst_mode_initialize(am_hal_burst_avail_e *peBurstAvail);

//*****************************************************************************
//
//! @brief Burst mode enable function
//!
//! @param peBurstStatus - resulting mode after call.
//!
//! This function enables the Apollo3 MCU into Burst Mode operation.
//!
//! @return status of API call.
//
//*****************************************************************************
extern uint32_t am_hal_burst_mode_enable(am_hal_burst_mode_e *peBurstStatus);

//*****************************************************************************
//
//! @brief Burst mode disable function
//!
//! @param peBurstStatus - resulting mode after call.
//!
//! This function disables the Apollo3 MCU from Burst Mode operation. It returns
//! the MCU to Normal Mode.
//!
//! @return status of API call.
//
//*****************************************************************************
extern uint32_t am_hal_burst_mode_disable(am_hal_burst_mode_e *peBurstStatus);

//*****************************************************************************
//
//! @brief Return current burst mode state
//!
//! Implemented as a macro, this function returns the current burst mode state.
//!     AM_HAL_BURST_MODE
//!     AM_HAL_NORMAL_MODE
//
//*****************************************************************************
#define am_hal_burst_mode_status()                                          \
    (CLKGEN->FREQCTRL_b.BURSTSTATUS ? AM_HAL_BURST_MODE : AM_HAL_NORMAL_MODE)


#if AM_HAL_BURST_LDO_WORKAROUND
#define AM_HAL_BURST_BUCK_VDDF_ADJ_CODE_30MV     5
#define AM_HAL_BURST_LDO_VDDF_ADJ_CODE_30MV      3

//*****************************************************************************
//
//! @brief Burst LDO to charge VDDS cap
//!
//! This function changes burst LDO power init to charge VDDS cap and put Burst
//! LDO into LP mode after chip power up
//
//*****************************************************************************
extern void am_hal_burst_ldo_charge(void);

//*****************************************************************************
//
//! @brief Enable Burst LDO workaround if Burst LDO Patch applied
//!
//! @param peBurstStatus - The memory power down enum.
//!
//! @return status - generic or interface specific status.
//
//*****************************************************************************
extern uint32_t am_hal_burst_mode_enable_w_manual_sequence(
                                        am_hal_burst_mode_e *peBurstStatus);

//*****************************************************************************
//
//! @brief Disable Burst LDO workaround if Burst LDO Patchapplied
//!
//! @param peBurstStatus - The memory power down enum.
//!
//! @return status - generic or interface specific status.
//
//*****************************************************************************
extern uint32_t am_hal_burst_mode_disable_w_manual_sequence(
                                        am_hal_burst_mode_e *peBurstStatus);

//*****************************************************************************
//
//! @brief Check if Burst LDO Patch is applied
//
//*****************************************************************************
extern bool am_hal_burst_ldo_patch_check(void);

#endif //AM_HAL_BURST_LDO_WORKAROUND

#ifdef __cplusplus
}
#endif

#endif // AM_HAL_BURST_H
//*****************************************************************************
//
// End Doxygen group.
//! @}
//
//*****************************************************************************
