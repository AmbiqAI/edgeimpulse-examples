//*****************************************************************************
//
//! @file am_hal_ble_patch_b0.h
//!
//! @brief This is a Binary Patch for the BLE Core.
//!
//! @addtogroup BLE3ppatch BLE_Patch - BLE Patch
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

#ifndef AM_HAL_BLE_PATCH_B0_H
#define AM_HAL_BLE_PATCH_B0_H

#ifdef __cplusplus
extern "C"
{
#endif

//*****************************************************************************
//
//! Patch array pointer.
//
//*****************************************************************************
extern am_hal_ble_patch_t **am_hal_ble_default_patches_b0;
extern am_hal_ble_patch_t **am_hal_ble_default_copy_patches_b0;
extern const uint32_t am_hal_ble_num_default_patches_b0;

//*****************************************************************************
//
//! Pointers for specific patches.
//
//*****************************************************************************
extern am_hal_ble_patch_t am_ble_performance_patch_b0;
extern am_hal_ble_patch_t am_ble_nvds_patch_b0;

//*****************************************************************************
//
//! Default patch structure.
//
//*****************************************************************************
extern am_hal_ble_patch_t g_AMBLEDefaultPatchB0;

//*****************************************************************************
//
//! Macros for accessing specific NVDS parameters.
//
//*****************************************************************************
#define AM_HAL_BLE_NVDS_CLOCKDRIFT_OFFSET          30
#define AM_HAL_BLE_NVDS_SLEEPCLOCKDRIFT_OFFSET     35
#define AM_HAL_BLE_NVDS_CLOCKSOURCE_OFFSET         44
#define AM_HAL_BLE_NVDS_SLEEPENABLE_OFFSET         85
#define AM_HAL_BLE_NVDS_AGC_OFFSET                 125

#ifdef __cplusplus
}
#endif

#endif // AM_HAL_BLE_PATCH_B0_H

//*****************************************************************************
//
// End Doxygen group.
//! @}
//
//*****************************************************************************
