//*****************************************************************************
//
//! @file am_devices_dc_dsi_novatek.h
//!
//! @brief Generic Novatek DSI driver.
//!
//! @addtogroup dc_dsi Novatek DC DSI Driver
//! @ingroup devices
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

#ifndef AM_DEVICES_DC_DSI_NOVATEK_H
#define AM_DEVICES_DC_DSI_NOVATEK_H

#include "stdint.h"
#include "nema_dc.h"
#include "nema_dc_hal.h"
#include "nema_dc_mipi.h"
#include "nema_dc_regs.h"
#include "nema_dc_intern.h"
#include "nema_dc_dsi.h"
#include "nema_sys_defs.h"
#include "am_devices_display_types.h"

#ifdef __cplusplus
extern "C"
{
#endif

//*****************************************************************************
//
//! Global type definitions.
//
//*****************************************************************************

//*****************************************************************************
//
//! External function definitions.
//
//*****************************************************************************

//*****************************************************************************
//
//! @brief Reset the display panel
//!
//! This function resets the display panel by toggling reset pin.
//
//*****************************************************************************
extern void am_devices_dc_dsi_novatek_hardware_reset(void);

//*****************************************************************************
//
//! @brief Initialize Novatek's IC NT38350 with DSI interface.
//!
//! @param psDisplayPanelConfig     - Display panel configuration structure.
//!
//! @note This function configures display panel with the configurations in
//! sDisplayPanelConfig. It should be called after NemaDC and DSI are
//! initialized and configured.
//!
//! @return status.
//
//*****************************************************************************
extern uint32_t am_devices_dc_dsi_novatek_init(am_devices_dc_config_t *psDisplayPanelConfig);

//*****************************************************************************
//
//! @brief Set the region to be updated.
//!
//! @param ui16ResX    - X resolution of display region
//! @param ui16ResY    - Y resolution of display region
//! @param ui16MinX    - X-axis starting point of display region
//! @param ui16MinY    - Y-axis starting point of display region
//!
//! This function can be used to set display region of display driver IC.
//!
//! @return status.
//
//*****************************************************************************
extern uint32_t am_devices_dc_dsi_novatek_set_region(uint16_t ui16ResX,
                                                     uint16_t ui16ResY,
                                                     uint16_t ui16MinX,
                                                     uint16_t ui16MinY);

#ifdef __cplusplus
}
#endif

#endif // AM_DEVICES_DC_DSI_RAYDIUM_H

//*****************************************************************************
//
// End Doxygen group.
//! @}
//
//*****************************************************************************
