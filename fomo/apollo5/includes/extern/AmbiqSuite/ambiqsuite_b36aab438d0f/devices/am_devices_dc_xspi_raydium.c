//*****************************************************************************
//
//! @file am_devices_dc_xspi_raydium.c
//!
//! @brief Generic Raydium display driver with DC xSPI interfaces. This
//! driver supports the display panels with driver IC rm67162, rm69330 or
//! rm69090, etc.
//!
//! @addtogroup dc_xspi Raydium DC xSPI Driver
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

#include "am_devices_dc_xspi_raydium.h"
#include "am_util_delay.h"
#include "am_bsp.h"

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
am_devices_dc_xspi_raydium_hardware_reset(void)
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
am_devices_dc_xspi_raydium_flip(uint8_t ui8FlipXY)
{
    uint8_t ui8CmdBuffer[4];
    ui8CmdBuffer[0] = ui8FlipXY;
    nemadc_mipi_cmd_write(MIPI_set_address_mode, ui8CmdBuffer, 1, true, false);
}

//*****************************************************************************
//
// Initialize raydium's IC RM67162, RM69090 or RM69330 with DC
//        SPI4/DSPI/QSPI interface.
//
//*****************************************************************************
uint32_t
am_devices_dc_xspi_raydium_init(am_devices_dc_xspi_raydium_config_t *psDisplayPanelConfig)
{
    uint8_t ui8CmdBuffer[4];
    uint32_t ui32Cfg = 0;
    ui32Cfg = nemadc_reg_read(NEMADC_REG_DBIB_CFG);
    //
    // Enable/disable tearing
    //
    if ( psDisplayPanelConfig->bTEEnable )
    {
        ui8CmdBuffer[0] = 0x02;
        nemadc_mipi_cmd_write(MIPI_set_tear_on, ui8CmdBuffer, 1, true, false);
    }
    else
    {
        nemadc_mipi_cmd_write(MIPI_set_tear_off, NULL, 0, true, false);
    }
    DELAY(10);

    const int MIPI_set_dspi_mode = 0xc4;
    if (((ui32Cfg & MIPICFG_DSPI) != 0) &&
        ((ui32Cfg & MIPICFG_SPI4) != 0) &&
        ((ui32Cfg & (MIPICFG_SPI3 | MIPICFG_QSPI | MIPICFG_DSPI_SPIX )) == 0))
    { // DSPI
        ui8CmdBuffer[0] = 0xA1; //enable DSPI 1P1T 2-Wire
    }
    else
    {
        ui8CmdBuffer[0] = 0x80;
    }
    nemadc_mipi_cmd_write(MIPI_set_dspi_mode, ui8CmdBuffer, 1, true, false);
    DELAY(10);

    // Set MIPI Panel Pixel Format
    ui8CmdBuffer[0] = (uint8_t)(psDisplayPanelConfig->ui32PixelFormat & 0x3f);
    nemadc_mipi_cmd_write(MIPI_set_pixel_format, ui8CmdBuffer, 1, true, false);
    DELAY(10);

#ifdef AM_BSP_DISP_FLIP
    ui8CmdBuffer[0] = AM_BSP_DISP_FLIP;
#else
    //
    //need to flip display when drive IC is rm67162
    //
    if ( psDisplayPanelConfig->bFlip )
    {
        ui8CmdBuffer[0] = 0x02;
    }
    else
    {
        ui8CmdBuffer[0] = 0x00;
    }
#endif
    //
    // Flip the image
    //
    am_devices_dc_xspi_raydium_flip(ui8CmdBuffer[0]);
    DELAY(10);

    const int MIPI_set_wr_display_ctrl = 0x53;
    ui8CmdBuffer[0] = 0x20;
    nemadc_mipi_cmd_write(MIPI_set_wr_display_ctrl, ui8CmdBuffer, 1, true, false);
    DELAY(10);

    const int MIPI_set_display_brightness = 0x51;
    ui8CmdBuffer[0] = 0xff;      // write display brightness
    nemadc_mipi_cmd_write(MIPI_set_display_brightness, ui8CmdBuffer, 1, true, false);
    DELAY(10);

    // Enable MIPI Panel
    nemadc_mipi_cmd_write(MIPI_exit_sleep_mode, NULL, 0, true, false);
    DELAY(130);

    nemadc_mipi_cmd_write(MIPI_set_display_on, NULL, 0, true, false);
    DELAY(200);

    am_devices_dc_xspi_raydium_set_region(psDisplayPanelConfig->ui16ResX,
                                          psDisplayPanelConfig->ui16ResY,
                                          psDisplayPanelConfig->ui16MinX,
                                          psDisplayPanelConfig->ui16MinY);

    return AM_DEVICES_DISPLAY_STATUS_SUCCESS;
}

//*****************************************************************************
//
// Set scanline
//
//*****************************************************************************
uint32_t
am_devices_dc_xspi_set_scanline(uint16_t ui16ScanLine, uint16_t ui16ResY)
{
    uint8_t ui8CmdBuffer[2];

    if ( ui16ScanLine > ui16ResY )
    {
        return AM_DEVICES_DISPLAY_STATUS_OUT_OF_RANGE;
    }

    ui8CmdBuffer[0] = ui16ScanLine >> 8;
    ui8CmdBuffer[1] = ui16ScanLine & 0x00FF;
    nemadc_mipi_cmd_write(MIPI_set_tear_scanline, ui8CmdBuffer, 2, true, false);

    return AM_DEVICES_DISPLAY_STATUS_SUCCESS;
}

//*****************************************************************************
//
// Set scanline with recommended parameter
//
//*****************************************************************************
uint32_t
am_devices_dc_xspi_set_scanline_with_recommended_parameter(uint8_t TETimesPerFrame,
                                                           uint16_t ui16ResY)
{
    uint16_t ui16ScanLine = 0;
    //
    // setting scanline.
    //
    if ( TETimesPerFrame == 2 )
    {
        //
        // setting scanline equal to start line add 10 lines when data transfer time is longer than TE signals interval time.
        //
        ui16ScanLine = 10;
    }
    else if ( TETimesPerFrame == 1 )
    {
        //
        // setting scanline equal to start line minus 10 lines when data transfer time is less than TE signals interval time.
        //
        ui16ScanLine = ui16ResY - 10;
    }
    else
    {
        return AM_DEVICES_DISPLAY_STATUS_INVALID_ARG;
    }

    return am_devices_dc_xspi_set_scanline(ui16ScanLine, ui16ResY);
}

//*****************************************************************************
//
//Set the region to be updated.
//
//*****************************************************************************
uint32_t
am_devices_dc_xspi_raydium_set_region(uint16_t ui16ResX,
                                      uint16_t ui16ResY,
                                      uint16_t ui16MinX,
                                      uint16_t ui16MinY)
{
    uint8_t ui8CmdBuffer[4];
    uint16_t ui16MaxX, ui16MaxY;

    ui16MaxX = ui16MinX + ui16ResX - 1;
    ui16MaxY = ui16MinY + ui16ResY - 1;
    //
    // Set MIPI Panel region to be updated
    //
    ui8CmdBuffer[0] = (uint8_t)(ui16MinX >> 8U);
    ui8CmdBuffer[1] = (uint8_t)(ui16MinX  & 0xFFU);
    ui8CmdBuffer[2] = (uint8_t)(ui16MaxX >> 8U);
    ui8CmdBuffer[3] = (uint8_t)(ui16MaxX  & 0xFFU);
    nemadc_mipi_cmd_write(MIPI_set_column_address, ui8CmdBuffer, 4, true, false);

    ui8CmdBuffer[0] = (uint8_t)(ui16MinY >> 8U);
    ui8CmdBuffer[1] = (uint8_t)(ui16MinY  & 0xFFU);
    ui8CmdBuffer[2] = (uint8_t)(ui16MaxY >> 8U);
    ui8CmdBuffer[3] = (uint8_t)(ui16MaxY  & 0xFFU);
    nemadc_mipi_cmd_write(MIPI_set_page_address, ui8CmdBuffer, 4, true, false);

    return AM_DEVICES_DISPLAY_STATUS_SUCCESS;
}

// #### INTERNAL BEGIN ####
# if 0

 //
 // TODO: Remove functions below.
 //
//*****************************************************************************
//
//! @brief deprecated API
//!
//! @return None.
//
//****************************************************************************
void
nemadc_send_frame_single_end(void)
{
}

//*****************************************************************************
//
//! @brief start send single frame.(deprecated API)
//!
//! This function sends frame to raydium's display IC rm67162 or rm69330 via SPI4,
//! DSPI or QSPI interface.
//!
//! @return None.
//
//****************************************************************************
void
nemadc_send_frame_single_start(void)
{
    am_devices_dc_xspi_transfer_frame_prepare(NULL,NULL);
    if ( g_sDispCfg.eTEType == DISP_TE_DISABLE )
    {
        am_devices_dc_xspi_transfer_frame_launch();
    }
}

//*****************************************************************************
//
//! @brief send single frame.(deprecated API)
//!
//! This function sends frame to raydium's display IC rm67162 or rm69330 via SPI4,
//! DSPI or QSPI interface.this function will block CPU when baremental invoke function
//! nemadc_wait_vsync()
//!
//! @return None.
//
//****************************************************************************
void
nemadc_send_frame_single(void)
{
    nemadc_send_frame_single_start();
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
