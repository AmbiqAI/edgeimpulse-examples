//*****************************************************************************
//
//! @file am_hal_burst.h
//!
//! @brief Functions for Controlling Burst Mode Operation.
//!
//! @addtogroup burstmode3 Burst - Burst Mode Functionality
//! @ingroup apollo3_hal
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
