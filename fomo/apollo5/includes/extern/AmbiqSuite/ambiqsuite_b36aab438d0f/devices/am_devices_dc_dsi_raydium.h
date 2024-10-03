//*****************************************************************************
//
//! @file am_devices_dc_dsi_raydium.h
//!
//! @brief Generic raydium DSI driver.
//!
//! @addtogroup dc_dsi Raydium DC DSI Driver
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

#ifndef AM_DEVICES_DC_DSI_RAYDIUM_H
#define AM_DEVICES_DC_DSI_RAYDIUM_H

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

typedef am_devices_dc_config_t am_devices_dc_dsi_raydium_config_t;

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
extern void am_devices_dc_dsi_raydium_hardware_reset(void);

//*****************************************************************************
//
//! @brief Set the flip along with the x/y-axis or both the x and y-axis.
//!
//! @param ui8FlipXY            - how to flip the display
//!
//! @note Register bits function for Driver IC RM69330 or CO5300.
//!       Bitfield 0 - Reserved                 (please don't set this bit)
//!       Bitfield 1 - Flip along with y-axis   (reserved for CO5300)
//!       Bitfield 2 - Reserved
//!       Bitfield 3 - RGB or BGR order
//!       Bitfield 4 - Flip along with x-axis   (reserved for CO5300)
//!       Bitfield 5 - Flip along y = x         (reserved for CO5300)
//!       Bitfield 6 - Flip along with y-axis
//!       Bitfield 7 - Flip along with x-axis
//! The x-axis will keep no flip when setting bitfields 4 and 7 same time. In the
//! same way, the same is true for the y-axis. Please be careful some of the flips
//! could cause the tear effect.
//!
//! @return None.
//
//*****************************************************************************
extern void am_devices_dc_dsi_raydium_flip(uint8_t ui8FlipXY);

//*****************************************************************************
//
//! @brief Initialize raydium's IC RM67162, RM69090 or RM69330 with DSI interface.
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
extern uint32_t am_devices_dc_dsi_raydium_init(am_devices_dc_dsi_raydium_config_t *psDisplayPanelConfig);

//*****************************************************************************
//
//! @brief Set scanline
//!
//! @param ui16ScanLine - scanline index
//! @param ui16ResY     - Y resolution of display panel
//!
//! @note This function is used to set scanline index. It will return error when given
//! scanline is greater than or equal to display maximum resolution.
//!
//! @return status.
//
//*****************************************************************************
extern uint32_t am_devices_dc_dsi_set_scanline(uint16_t ui16ScanLine, uint16_t ui16ResY);

//*****************************************************************************
//
//! @brief Set scanline with recommended parameter
//!
//! @param TETimesPerFrame    - How many TE interrupts during transfering one frame
//! @param ui16ResY           - Y resolution of display panel
//!
//! @note This function is used to set scanline with recommended parameter, it's valid
//! when ui8TEIntTimesPerFrame equals to 1 or 2.
//!
//! @return status.
//
//*****************************************************************************
extern uint32_t am_devices_dc_dsi_set_scanline_with_recommended_parameter(uint8_t TETimesPerFrame,
                                                                          uint16_t ui16ResY);

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
extern uint32_t am_devices_dc_dsi_raydium_set_region(uint16_t ui16ResX,
                                                     uint16_t ui16ResY,
                                                     uint16_t ui16MinX,
                                                     uint16_t ui16MinY);

// #### INTERNAL BEGIN ####
//*****************************************************************************
//
//! @brief Adjust TE frequency of display panel.
//!
//! This function adjusts TE frequency by setting MIPI_set_lfrctr4 register.
//
//*****************************************************************************
extern void am_devices_dc_dsi_raydium_adjust_te_frequency(uint8_t ui8Para);

// #### INTERNAL END ####

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