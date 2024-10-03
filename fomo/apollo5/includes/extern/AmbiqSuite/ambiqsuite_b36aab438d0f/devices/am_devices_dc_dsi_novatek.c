//*****************************************************************************
//
//! @file am_devices_dc_dsi_novatek.c
//!
//! @brief Generic Novatek display driver with DSI interfaces. This
//! driver supports the display panels with driver IC nt38350.
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

#include "am_devices_dc_dsi_novatek.h"
#include "am_util_delay.h"
#include "am_bsp.h"
#include "am_util.h"

#ifndef SIMULATION
#define DELAY am_util_delay_ms
#else
#define DELAY(...)
#endif

//*****************************************************************************
//
// Reset the display panel
//
//*****************************************************************************
void
am_devices_dc_dsi_novatek_hardware_reset(void)
{
    am_bsp_disp_reset_pins_set();
    DELAY(5);
    am_bsp_disp_reset_pins_clear();
    DELAY(20);
    am_bsp_disp_reset_pins_set();
    DELAY(150);
}

//*****************************************************************************
//
// Initialize novatek's IC NT38350 with DSI interface.
//
//*****************************************************************************
uint32_t
am_devices_dc_dsi_novatek_init(am_devices_dc_config_t *psDisplayPanelConfig)
{
    uint8_t ui8CmdBuf[4];
    //
    // CMD page selection
    //
    const int MIPI_set_cmd_page = 0xFF;
    ui8CmdBuf[0] = MIPI_set_cmd_page;
    //
    // 10h - CMD1 is selected(Basic Display Control Setting)
    // 20h - CMD2 page 0 is selected
    // 21h - CMD2 page 1 is selected
    // etc
    //
    ui8CmdBuf[1] = 0x10;
    nemadc_mipi_cmd_write(0, ui8CmdBuf, 2, false, false);
    DELAY(10);
    //
    // Reload CMD1
    //
    const int MIPI_reload_cmd1 = 0xFB;
    ui8CmdBuf[0] = MIPI_reload_cmd1;
    //
    // 00h - reload setting value from MTP or register default value to register
    // 01h - Don't reload MTP or register default value to register
    //
    ui8CmdBuf[1] = 0x01;
    nemadc_mipi_cmd_write(0, ui8CmdBuf, 2, false, false);
    DELAY(10);

    //
    // set control display
    //
    ui8CmdBuf[0] = 0x53;
    ui8CmdBuf[1] = 0x28;
    nemadc_mipi_cmd_write(0, ui8CmdBuf, 2, false, false);
    DELAY(10);

    //
    // Brightness control.
    //
    const int MIPI_set_display_brightness = 0x51;
    ui8CmdBuf[0] = MIPI_set_display_brightness;
    ui8CmdBuf[1] = 0xff;
    nemadc_mipi_cmd_write(0, ui8CmdBuf, 2, false, false);
    DELAY(10);

    //
    // OTP related command.
    //
    ui8CmdBuf[0] = 0x6F;
    ui8CmdBuf[1] = 0x82;
    nemadc_mipi_cmd_write(0, ui8CmdBuf, 2, false, false);
    DELAY(10);

    ui8CmdBuf[0] = 0x70;
    ui8CmdBuf[1] = 0x48;
    nemadc_mipi_cmd_write(0, ui8CmdBuf, 2, false, false);
    DELAY(10);

    ui8CmdBuf[0] = 0x71;
    ui8CmdBuf[1] = 0x05;
    nemadc_mipi_cmd_write(0, ui8CmdBuf, 2, false, false);
    DELAY(10);

    //
    // Set mipi lane
    //
    const int MIPI_set_mipi_lane = 0xBA;
    ui8CmdBuf[0] = MIPI_set_mipi_lane;
    //
    // 00h - 1 lane
    // 01h - 2 lanes
    //
    if (psDisplayPanelConfig->ui8Lanes == 1)
    {
        ui8CmdBuf[1] = 0x00;
    }
    else if (psDisplayPanelConfig->ui8Lanes == 2)
    {
        ui8CmdBuf[1] = 0x01;
    }
    else
    {
        return AM_DEVICES_DISPLAY_STATUS_INVALID_ARG;
    }
    nemadc_mipi_cmd_write(0, ui8CmdBuf, 2, false, false);
    DELAY(120);

    //
    // Set display mode
    //
    const int MIPI_set_display_mode = 0xBB;
    ui8CmdBuf[0] = MIPI_set_display_mode;
    ui8CmdBuf[1] = 0x10;
    nemadc_mipi_cmd_write(0, ui8CmdBuf, 2, false, false);
    DELAY(10);

    //
    // Set MIPI Panel Pixel Format
    //
    ui8CmdBuf[0] = MIPI_set_pixel_format;
    ui8CmdBuf[1] = (uint8_t)(psDisplayPanelConfig->ui32PixelFormat & 0x3f);
    ui8CmdBuf[2] = ui8CmdBuf[1];
    nemadc_mipi_cmd_write(0, ui8CmdBuf, 3, false, false);
    DELAY(10);

    //
    // Enable MIPI Panel
    //
    nemadc_mipi_cmd_write(MIPI_exit_sleep_mode, NULL, 0, true, false);
    DELAY(130);

    nemadc_mipi_cmd_write(MIPI_set_display_on, NULL, 0, true, false);
    DELAY(200);

    //
    // Set display panel region to be updated
    //
    am_devices_dc_dsi_novatek_set_region(psDisplayPanelConfig->ui16ResX,
                                         psDisplayPanelConfig->ui16ResY,
                                         psDisplayPanelConfig->ui16MinX,
                                         psDisplayPanelConfig->ui16MinY);

    DELAY(200);

    //
    // Enable/disable tearing
    //
    if ( psDisplayPanelConfig->bTEEnable )
    {
        ui8CmdBuf[0] = 0x02; // TE output active at refresh frame
        nemadc_mipi_cmd_write(MIPI_set_tear_on, ui8CmdBuf, 1, true, false);
    }
    else
    {
        nemadc_mipi_cmd_write(MIPI_set_tear_off, NULL, 0, true, false);
    }
    DELAY(10);

    return AM_DEVICES_DISPLAY_STATUS_SUCCESS;
}

//*****************************************************************************
//
// Set the region to be updated.
//
//*****************************************************************************
uint32_t
am_devices_dc_dsi_novatek_set_region(uint16_t ui16ResX,
                                     uint16_t ui16ResY,
                                     uint16_t ui16MinX,
                                     uint16_t ui16MinY)
{
    uint8_t ui8CmdBuf[4];
    uint16_t ui16MaxX, ui16MaxY;

    ui16MaxX = ui16MinX + ui16ResX - 1;
    ui16MaxY = ui16MinY + ui16ResY - 1;

    //
    // Set MIPI Panel region to be updated
    //
    ui8CmdBuf[0] = (uint8_t)(ui16MinX >> 8U);
    ui8CmdBuf[1] = (uint8_t)(ui16MinX & 0xFFU);
    ui8CmdBuf[2] = (uint8_t)(ui16MaxX >> 8U);
    ui8CmdBuf[3] = (uint8_t)(ui16MaxX & 0xFFU);
    nemadc_mipi_cmd_write(MIPI_set_column_address, ui8CmdBuf, 4, true, false);
    DELAY(20);

    ui8CmdBuf[0] = (uint8_t)(ui16MinY >> 8U);
    ui8CmdBuf[1] = (uint8_t)(ui16MinY & 0xFFU);
    ui8CmdBuf[2] = (uint8_t)(ui16MaxY >> 8U);
    ui8CmdBuf[3] = (uint8_t)(ui16MaxY & 0xFFU);
    nemadc_mipi_cmd_write(MIPI_set_page_address, ui8CmdBuf, 4, true, false);

    return AM_DEVICES_DISPLAY_STATUS_SUCCESS;
}

//*****************************************************************************
//
// End Doxygen group.
//! @}
//
//*****************************************************************************
