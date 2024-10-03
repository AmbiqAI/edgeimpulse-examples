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

extern int32_t display_power_down(void);
extern int32_t display_power_up(void);

extern bool dc_power_down(void);
extern bool dc_power_up(void);

extern int32_t gpu_power_up(void);
extern int32_t gpu_power_down(void);

#endif // GRAPHICS_POWER_CONTROL_H
