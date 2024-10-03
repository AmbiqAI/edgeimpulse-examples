//*****************************************************************************
//
//! @file mpu_config.h
//!
//! @brief Configure the ARMv8.1 MPU
//!
//
//*****************************************************************************

//*****************************************************************************
//
// ${copyright}
//
// This is part of revision ${version} of the AmbiqSuite Development Package.
//
//*****************************************************************************

#include "am_mcu_apollo.h"

//
// Global variables
//
extern am_hal_mpu_region_config_t sMPUCfgTest1[2];
extern am_hal_mpu_region_config_t sMPUCfgTest2[2];
extern am_hal_mpu_attr_t sMPUAttr[3];

//
// Configure MPU
//
extern int32_t mpu_config (am_hal_mpu_region_config_t *psMPURegionConfig, uint32_t ui32CfgCnt);
