//*****************************************************************************
//
//! @file am_hal_clkmgr_private.h
//!
//! @brief Internal api definition for internal clock manager functions
//
//*****************************************************************************

//*****************************************************************************
//
// ${copyright}
//
// This is part of revision ${version} of the AmbiqSuite Development Package.
//
//*****************************************************************************
//! @cond CLKMGR_PRIVATE_FUNC
#ifndef AM_HAL_CLKMGR_PRIVATE_H
#define AM_HAL_CLKMGR_PRIVATE_H

#ifdef __cplusplus
extern "C"
{
#endif

#include "am_hal_clkmgr.h"

//*****************************************************************************
//
//! @brief Handle Call to Legacy CLKGEN AM_HAL_CLKGEN_CONTROL_HFADJ_ENABLE
//!        This function will convert and call HFRC clock configuration.
//!
//! @note This API is inteded for use by HAL only. Do not call this API from
//!       Application/BSP.
//!
//! @param pArgs - Argument passed to the AM_HAL_CLKGEN_CONTROL_HFADJ_ENABLE
//!                for custom HFADJ configuration
//!
//! @return status - Status for the operation, as defined in am_hal_status_e
//
//*****************************************************************************
extern uint32_t am_hal_clkmgr_private_clkgen_hfadj_apply(void* pArgs);

//*****************************************************************************
//
//! @brief Handle Call to Legacy CLKGEN AM_HAL_CLKGEN_CONTROL_HFADJ_DISABLE
//!        This function will convert and call HFRC clock configuration.
//!
//! @note This API is inteded for use by HAL only. Do not call this API from
//!       Application/BSP.
//!
//! @return status - Status for the operation, as defined in am_hal_status_e
//
//*****************************************************************************
extern uint32_t am_hal_clkmgr_private_clkgen_hfadj_disable();

//*****************************************************************************
//
//! @brief Handle Call to Legacy CLKGEN AM_HAL_CLKGEN_CONTROL_HF2ADJ_ENABLE
//!        This function will convert and call HFRC2 clock configuration.
//!
//! @note This API is inteded for use by HAL only. Do not call this API from
//!       Application/BSP.
//!
//! @return status - Status for the operation, as defined in am_hal_status_e
//
//*****************************************************************************
extern uint32_t am_hal_clkmgr_private_clkgen_hf2adj_apply();

//*****************************************************************************
//
//! @brief Handle Call to Legacy CLKGEN AM_HAL_CLKGEN_CONTROL_HF2ADJ_DISABLE
//!        This function will convert and call HFRC2 clock configuration.
//!
//! @note This API is inteded for use by HAL only. Do not call this API from
//!       Application/BSP.
//!
//! @return status - Status for the operation, as defined in am_hal_status_e
//
//*****************************************************************************
extern uint32_t am_hal_clkmgr_private_clkgen_hf2adj_disable();

#ifdef __cplusplus
}
#endif

#endif //AM_HAL_CLKMGR_PRIVATE_H
//! @endcond CLKMGR_PRIVATE_FUNC

