//*****************************************************************************
//
//! @file am_devices_dc_dsi_raydium.c
//!
//! @brief Generic Raydium display driver with DSI interfaces. This
//! driver supports the display panels with driver IC rm67162, rm69330 or
//! rm69090, etc.
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

#include "am_devices_dc_dsi_raydium.h"
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
am_devices_dc_dsi_raydium_hardware_reset(void)
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
// Flip the image along with x or y or x/y-axis
//
//*****************************************************************************
void
am_devices_dc_dsi_raydium_flip(uint8_t ui8FlipXY)
{
    uint8_t ui8CmdBuf[4];
    ui8CmdBuf[0] = ui8FlipXY;
    nemadc_mipi_cmd_write(MIPI_set_address_mode, ui8CmdBuf, 1, true, false);
}

//*****************************************************************************
//
// Initialize raydium's IC RM67162, RM69090 or RM69330 with DSI interface.
//
//*****************************************************************************
uint32_t
am_devices_dc_dsi_raydium_init(am_devices_dc_dsi_raydium_config_t *psDisplayPanelConfig)
{
    uint8_t ui8CmdBuf[4];
    const int MIPI_set_cmd_page = 0xFE;

    //
    // Sending brightness command.
    //
    const int MIPI_set_display_brightness = 0x51;
    ui8CmdBuf[0] = MIPI_set_display_brightness;
    ui8CmdBuf[1] = 0xff;
    nemadc_mipi_cmd_write(0, ui8CmdBuf, 2, false, false);
    DELAY(10);

    //
    // Set MIPI Panel Pixel Format
    //
    ui8CmdBuf[0] = (uint8_t)(psDisplayPanelConfig->ui32PixelFormat & 0x3f);
    nemadc_mipi_cmd_write(MIPI_set_pixel_format, ui8CmdBuf, 1, true, false);
    DELAY(10);

    //
    // When using RM69330 DSI, set N565 reg to 1 to solve color issue caused by endianess.
    //
    const int MIPI_set_hsifopctr = 0x0A;
    ui8CmdBuf[0] = MIPI_set_cmd_page;
    ui8CmdBuf[1] = 0x01; // MCS
    nemadc_mipi_cmd_write(0, ui8CmdBuf, 2, false, false);
    DELAY(10);
    ui8CmdBuf[0] = MIPI_set_hsifopctr;
    ui8CmdBuf[1] = 0xF8; // set N565 to 1
    nemadc_mipi_cmd_write(0, ui8CmdBuf, 2, false, false);
    DELAY(10);
    ui8CmdBuf[0] = MIPI_set_cmd_page;
    ui8CmdBuf[1] = 0x00; // UCS
    nemadc_mipi_cmd_write(0, ui8CmdBuf, 2, false, false);
    DELAY(10);
// #### INTERNAL BEGIN ####
#ifdef DSI_DEBUG //TODO
    // nemadc_dsi_ct((uint32_t)0,
    //               (uint32_t)0,
    //               NemaDC_dcs_datacmd);
    // nemadc_MIPI_out(NemaDC_DBI_cmd | NemaDC_ext_ctrl | NemaDC_DBI_read | MIPI_get_pixel_format);
    // am_util_delay_us(100);
    // DSI->DEVICEREADY_b.DISPLAYBUSPOSSESSEN = 1;
    // am_util_delay_us(1);
    // DSI->DEVICEREADY_b.DISPLAYBUSPOSSESSEN = 0;
    // am_util_delay_us(10);
    uint32_t ui32IntrStatus = DSI->INTRSTAT;
    // DSI->DEVICEREADY_b.DISPLAYBUSPOSSESSEN = 0;
    // DSI->INTRSTAT = 0x03FFFFFF;
    //nemadc_mipi_cmd_write(MIPI_soft_reset, NULL, 0, true, false);
    if ((ui32IntrStatus == 0x11100000) || (ui32IntrStatus == 0x10100000))
    {
        am_util_stdio_printf("\nACK from display is right, ACK is 0x%8X, please also check the image displayed on panel.\n", ui32IntrStatus);
    }
    else
    {
        am_util_stdio_printf("\nACK from display is wrong. ACK is 0x%8X.\n", ui32IntrStatus);
        return AM_DEVICES_DISPLAY_STATUS_ERROR;
    }
#endif
// #### INTERNAL END ####

#ifdef AM_BSP_DISP_FLIP
    ui8CmdBuf[0] = AM_BSP_DISP_FLIP;
#else
    //
    // Need to flip display when drive IC is RM67162
    //
    if ( psDisplayPanelConfig->bFlip )
    {
        ui8CmdBuf[0] = 0x02;
    }
    else
    {
        ui8CmdBuf[0] = 0x00;
    }
#endif
    //
    // Flip the image
    //
    am_devices_dc_dsi_raydium_flip(ui8CmdBuf[0]);
    DELAY(10);

    //
    // CMD2 password
    //
    if (g_sDispCfg.eIC == DISP_IC_CO5300)
    {
        ui8CmdBuf[0] = MIPI_set_cmd_page;
        ui8CmdBuf[1] = 0x20; // RFE 20
        nemadc_mipi_cmd_write(0, ui8CmdBuf, 2, false, false);

        ui8CmdBuf[0] = 0xF4;
        ui8CmdBuf[1] = 0x5A; // RF4 5A
        nemadc_mipi_cmd_write(0, ui8CmdBuf, 2, false, false);

        ui8CmdBuf[0] = 0xF5;
        ui8CmdBuf[1] = 0x59; // RF5 59
        nemadc_mipi_cmd_write(0, ui8CmdBuf, 2, false, false);

        ui8CmdBuf[0] = MIPI_set_cmd_page;
        ui8CmdBuf[1] = 0x80; // RFE 80
        nemadc_mipi_cmd_write(0, ui8CmdBuf, 2, false, false);

        ui8CmdBuf[0] = 0x00;
        ui8CmdBuf[1] = 0xF8; // R00 F8
        nemadc_mipi_cmd_write(0, ui8CmdBuf, 2, false, false);

        ui8CmdBuf[0] = 0xFE;
        ui8CmdBuf[1] = 0x00; // UCS
        nemadc_mipi_cmd_write(0, ui8CmdBuf, 2, false, false);
    }

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
    am_devices_dc_dsi_raydium_set_region(psDisplayPanelConfig->ui16ResX,
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
// Set scanline
//
//*****************************************************************************
uint32_t
am_devices_dc_dsi_set_scanline(uint16_t ui16ScanLine, uint16_t ui16ResY)
{
    uint8_t ui8CmdBuf[2];

    if ( ui16ScanLine > ui16ResY )
    {
        return AM_DEVICES_DISPLAY_STATUS_OUT_OF_RANGE;
    }

    ui8CmdBuf[0] = ui16ScanLine >> 8;
    ui8CmdBuf[1] = ui16ScanLine & 0x00FF;
    nemadc_mipi_cmd_write(MIPI_set_tear_scanline, ui8CmdBuf, 2, true, false);

    return AM_DEVICES_DISPLAY_STATUS_SUCCESS;
}

//*****************************************************************************
//
// Set scanline with recommended parameter
//
//*****************************************************************************
uint32_t
am_devices_dc_dsi_set_scanline_with_recommended_parameter(uint8_t TETimesPerFrame,
                                                          uint16_t ui16ResY)
{
    uint16_t ui16ScanLine = 0;
    //
    // setting scanline.
    //
    if ( TETimesPerFrame == 2 )
    {
        //
        // setting scanline to start line plus 10 lines when frame transfer time is longer than TE signals interval.
        //
        ui16ScanLine = 10;
    }
    else if ( TETimesPerFrame == 1 )
    {
        //
        // setting scanline to start line minus 10 lines when frame transfer time is shorter than TE signals interval.
        //
        ui16ScanLine = ui16ResY - 10;
    }
    else
    {
        return AM_DEVICES_DISPLAY_STATUS_INVALID_ARG;
    }
    return am_devices_dc_dsi_set_scanline(ui16ScanLine, ui16ResY);
}

//*****************************************************************************
//
// Set the region to be updated.
//
//*****************************************************************************
uint32_t
am_devices_dc_dsi_raydium_set_region(uint16_t ui16ResX,
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

    ui8CmdBuf[0] = (uint8_t)(ui16MinY >> 8U);
    ui8CmdBuf[1] = (uint8_t)(ui16MinY & 0xFFU);
    ui8CmdBuf[2] = (uint8_t)(ui16MaxY >> 8U);
    ui8CmdBuf[3] = (uint8_t)(ui16MaxY & 0xFFU);
    nemadc_mipi_cmd_write(MIPI_set_page_address, ui8CmdBuf, 4, true, false);

    return AM_DEVICES_DISPLAY_STATUS_SUCCESS;
}

// #### INTERNAL BEGIN ####
//*****************************************************************************
//
// Adjust TE frequency of display panel.
//
//*****************************************************************************
void
am_devices_dc_dsi_raydium_adjust_te_frequency(uint8_t ui8Para)
{
    uint8_t ui8CmdBuf[2];
    const int MIPI_set_cmd_page = 0xFE;
    const int MIPI_set_lfrctr4 = 0x28;
    //
    // Adjust TE frequency of display.
    //
    ui8CmdBuf[0] = MIPI_set_cmd_page;
    ui8CmdBuf[1] = 0x01; // MCS
    nemadc_mipi_cmd_write(0, ui8CmdBuf, 2, false, false);
    DELAY(10);
    ui8CmdBuf[0] = MIPI_set_lfrctr4;
    ui8CmdBuf[1] = ui8Para; // 0x65 - ~45fps, 0x70 - ~50fps
    nemadc_mipi_cmd_write(0, ui8CmdBuf, 2, false, false);
    DELAY(10);
    ui8CmdBuf[0] = MIPI_set_cmd_page;
    ui8CmdBuf[1] = 0x00; // UCS
    nemadc_mipi_cmd_write(0, ui8CmdBuf, 2, false, false);
    DELAY(10);
}
// #### INTERNAL END ####
// #### INTERNAL BEGIN ####
#if 0
//
// TODO: Remove functions below
//
//*****************************************************************************
//
//! @brief deprecated API
//
//****************************************************************************
void
dsi_send_frame_single_end(void)
{
}

//*****************************************************************************
//
//! @brief start sending single frame.(deprecated API)
//!
//! @param  ui32Mode    - mode
//!
//! This function sends frame to rm67162 or rm69330.
//
//****************************************************************************
void
dsi_send_frame_single_start(uint32_t ui32Mode)
{
    am_devices_dc_dsi_transfer_frame_prepare(NULL,NULL);
    if ( g_sDispCfg.eTEType == DISP_TE_DISABLE )
    {
        am_devices_dc_dsi_transfer_frame_launch();
    }
}

//*****************************************************************************
//
//! @brief start sending single frame.
//!
//! This function sends frame to rm67162.
//
//****************************************************************************
void
dsi_send_frame_single_start_none_block_te(void)
{
    uint32_t ui32Gpio;
    ui32Gpio = nemadc_reg_read(NEMADC_REG_GPIO);
    nemadc_reg_write(NEMADC_REG_GPIO, ui32Gpio & (~0x1)); // HS
    nemadc_reg_write(NEMADC_REG_CLKCTRL_CG, 0xFFFFFFF5U); // disable clock gating

    wait_NemaDC_dbi_idle();

    //
    // Set data/commands command type
    //
    nemadc_dsi_ct((uint32_t)0,
                  (uint32_t)0,
                   NemaDC_dcs_datacmd);

    //
    // Set scan-line (DCS) command
    //
    nemadc_MIPI_out(MIPI_DBIB_CMD | MIPI_write_memory_continue /* | NemaDC_ext_ctrl */ | NemaDC_sline_cmd);

    uint32_t ui32DbiCfg = nemadc_reg_read(NEMADC_REG_DBIB_CFG);
    nemadc_MIPI_CFG_out(ui32DbiCfg | MIPICFG_SPI_HOLD);

    //
    // Send DCS write_memory_start command
    //
    nemadc_MIPI_out(MIPI_DBIB_CMD | MIPI_write_memory_start /*| NemaDC_ext_ctrl*/);

    wait_NemaDC_dbi_idle();
    // Schedule an interrupt on next TE
    //
    nemadc_reg_write(NEMADC_REG_INTERRUPT, 1 << 3);
}

void
dsi_send_frame_single_start_none_block_vsync(uint32_t ui32Mode)
{
    nemadc_reg_write(NEMADC_REG_INTERRUPT, 1 << 4); //!< Enable frame end interrupt
    nemadc_set_mode(NEMADC_ONE_FRAME | ui32Mode);
}

//*****************************************************************************
//
//! @brief send single frame.(deprecated API)
//!
//! @param  ui32Mode    - mode
//!
//! This function sends frame to rm67162 or rm69330.
//
//****************************************************************************
void
dsi_send_frame_single(uint32_t ui32Mode)
{
    dsi_send_frame_single_start(ui32Mode);
    //
    // Wait for transfer to be completed
    //
    nemadc_wait_vsync();
}
#endif
// #### INTERNAL END ####

//*****************************************************************************
//
// End Doxygen group.
//! @}
//
//*****************************************************************************
