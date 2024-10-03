//*****************************************************************************
//
//! @file graphics_power_control.h
//!
//! @brief Power ON/OFF GPU,DC,DSI(DPHY).
//
//*****************************************************************************

//*****************************************************************************
//
// ${copyright}
//
// This is part of revision ${version} of the AmbiqSuite Development Package.
//
//*****************************************************************************

#ifndef GRAPHICS_POWER_CONTROL_H
#define GRAPHICS_POWER_CONTROL_H

#include "am_devices_dc_xspi_raydium.h"
#include "am_devices_dc_dsi_raydium.h"

extern int32_t display_power_down(void);
extern int32_t display_power_up(uint32_t ui32PixelFormat, int i32ResX, int i32ResY);

extern int32_t dc_power_down(void);
extern int32_t dc_power_up(uint32_t ui32PixelFormat, int i32ResX, int i32ResY);

extern int32_t gpu_power_up(void);
extern int32_t gpu_power_down(void);

#endif // GRAPHICS_POWER_CONTROL_H
