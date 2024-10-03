//*****************************************************************************
//
//! @file am_bsp_devices.c
//!
//! @brief External device support.
//!
//! Structures and functions to support communication with external devices.
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
#include "am_devices.h"
#include "am_bsp_devices.h"

//*****************************************************************************
//
// Accelerometer
//
//*****************************************************************************
am_devices_adxl362_t am_bsp_devices_sADXL362 =
{
    .ui32IOMModule = 1,
    .ui32ChipSelect = 3,
};

//*****************************************************************************
//
// SPI Flash
//
//*****************************************************************************
am_devices_spiflash_t am_bsp_devices_sSPIFlash =
{
    .ui32IOMModule = 1,
    .ui32ChipSelect = 6,
};

// #### INTERNAL BEGIN ####
//*****************************************************************************
//
// Gyro
//
//*****************************************************************************
am_devices_l3gd20h_t am_bsp_devices_sL3GD20H =
{
    .ui32IOMModule = 1,
    .ui32ChipSelect = 2,
};
// #### INTERNAL END ####
