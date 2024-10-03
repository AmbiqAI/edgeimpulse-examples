//*****************************************************************************
//
//! @file am_hal_clkgen_private.h
//!
//! @brief Internal api definition for internal clock_gen functions
//
//*****************************************************************************

//*****************************************************************************
//
// ${copyright}
//
// This is part of revision ${version} of the AmbiqSuite Development Package.
//
//*****************************************************************************
//! @cond CLKGEN_PRIVATE_FUNC
#ifndef AM_HAL_CLKGEN_PRIVATE_H
#define AM_HAL_CLKGEN_PRIVATE_H

#ifdef __cplusplus
extern "C"
{
#endif

#include "am_hal_clkgen.h"

// ****************************************************************************
//
//! @brief Enable/Disable force-on for HFRC oscillator block.
//!
//! @note This API is inteded for use by HAL only. Do not call this API from
//!       Application/BSP.
//!
//! @param bForceOn - TRUE: force HFRC oscillator ON, FALSE: Hardware managed.
//!
//! @return status  - Execution status as defined in am_hal_status_e.
//
// ****************************************************************************
extern uint32_t am_hal_clkgen_private_hfrc_force_on(bool bForceOn);

// ****************************************************************************
//
//! @brief Configure and Enable HFADJ
//!
//! @note This API is inteded for use by HAL only. Do not call this API from
//!       Application/BSP.
//!
//! @param ui32RegVal - HFADJ configuration register value.
//!
//! @return status    - Execution status as defined in am_hal_status_e.
//
// ****************************************************************************
extern uint32_t am_hal_clkgen_private_hfadj_apply(uint32_t ui32RegVal);

// ****************************************************************************
//
//! @brief Disable HFADJ
//!
//! @note This API is inteded for use by HAL only. Do not call this API from
//!       Application/BSP.
//!
//! @return status    - Execution status as defined in am_hal_status_e.
//
// ****************************************************************************
extern uint32_t am_hal_clkgen_private_hfadj_disable();

// ****************************************************************************
//
//! @brief Enable/Disable force-on for HFRC2 oscillator block.
//!
//! @note This API is inteded for use by HAL only. Do not call this API from
//!       Application/BSP.
//!
//! @param bForceOn - TRUE: force HFRC2 oscillator ON, FALSE: Hardware managed.
//!
//! @return status  - Execution status as defined in am_hal_status_e.
//
// ****************************************************************************
extern uint32_t am_hal_clkgen_private_hfrc2_force_on(bool bForceOn);

// ****************************************************************************
//
//! @brief Configure and Enable HFADJ
//!
//! @note This API is inteded for use by HAL only. Do not call this API from
//!       Application/BSP.
//!
//! @param ui32RegVal - HFADJ configuration register value.
//!
//! @return status    - Execution status as defined in am_hal_status_e.
//
// ****************************************************************************
extern uint32_t am_hal_clkgen_private_hf2adj_apply(am_hal_clkgen_hfrc2adj_t* psHFRC2Adj);

// ****************************************************************************
//
//! @brief Disable HFADJ2
//!
//! @note This API is inteded for use by HAL only. Do not call this API from
//!       Application/BSP.
//!
//! @return status    - Execution status as defined in am_hal_status_e.
//
// ****************************************************************************
extern uint32_t am_hal_clkgen_private_hf2adj_disable();

#ifdef __cplusplus
}
#endif

#endif // AM_HAL_CLKGEN_PRIVATE_H
//! @endcond CLKGEN_PRIVATE_FUNC
