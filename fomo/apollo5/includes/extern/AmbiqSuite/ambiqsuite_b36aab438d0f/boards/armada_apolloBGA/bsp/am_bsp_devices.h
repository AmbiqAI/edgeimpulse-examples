//*****************************************************************************
//
//! @file am_bsp_devices.h
//!
//! @brief External device support.
//
//*****************************************************************************

//*****************************************************************************
//
// ${copyright}
//
// This is part of revision ${version} of the AmbiqSuite Development Package.
//
//*****************************************************************************
#ifndef AM_BSP_DEVICES_H
#define AM_BSP_DEVICES_H

#ifdef __cplusplus
extern "C"
{
#endif
//*****************************************************************************
//
// External variable definitions
//
//*****************************************************************************
extern am_devices_adxl362_t am_bsp_devices_sADXL362;
extern am_devices_spiflash_t am_bsp_devices_sSPIFlash;
// #### INTERNAL BEGIN ####
extern am_devices_l3gd20h_t am_bsp_devices_sL3GD20H;
// #### INTERNAL END ####

//*****************************************************************************
//
// External function definitions
//
//*****************************************************************************

#ifdef __cplusplus
}
#endif

#endif // AM_BSP_DEVICES_H

