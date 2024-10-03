//*****************************************************************************
//
//! @file nemadc_partial_update_effect.c
//!
//! @brief partial update effect example.
//!
//! This example demonstrates how to implement partial update effect with DC.
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
#include "nemadc_partial_update_effect.h"
#include "pinwheel_400x400_rgba565.h"
#include "umbrella_400x400_rgba565.h"

//
// It has the restriction that the layer size is a multiple of 4 for Apollo5.
//
#define FB_RESX             400
#define FB_RESY             400
#define SMALLFB_STRIPES     4
#define CENTER_ALIGN_X      ((g_sDispCfg.ui16ResX - FB_RESX) >> 2 << 1)
#define CENTER_ALIGN_Y      ((g_sDispCfg.ui16ResY - FB_RESY) >> 2 << 1)

//
// Implement partial update through write continue
//
#define DC_WRITE_CONTINUE

//
//Intialize small buffer if defined macro SMALLFB
//
//#define SMALLFB

#ifdef DC_WRITE_CONTINUE
#ifndef SMALLFB
#define SMALLFB
#endif
#endif

//*****************************************************************************
//
//! @brief continue transfer frame
//!
//! @param bAutoLaunch     - automative launch
//!
//! @note This function is used to continue transfer frame
//!
//! @return status.
//
//*****************************************************************************
void
nemadc_transfer_frame_continue(bool bAutoLaunch)
{
    uint32_t ui32Cfg = nemadc_reg_read(NEMADC_REG_DBIB_CFG);
    uint32_t ui32MemWrCmd = MIPI_write_memory_continue;

    if (((ui32Cfg & MIPICFG_DBI_EN) != 0) &&
        ((ui32Cfg & (MIPICFG_SPI3 | MIPICFG_SPI4 | MIPICFG_DSPI | MIPICFG_QSPI | MIPICFG_DSPI_SPIX )) == 0))
    {
        nemadc_reg_write(NEMADC_REG_GPIO, nemadc_reg_read(NEMADC_REG_GPIO) & (~0x1));
        //
        // disable clock gating
        //
        nemadc_reg_write(NEMADC_REG_CLKCTRL_CG, 0xFFFFFFF5U);

        while ((nemadc_reg_read(NEMADC_REG_STATUS) & DC_STATUS_dbi_busy) != 0);
        //
        // Set data/commands command type
        //
        nemadc_dsi_ct((uint32_t)0, // Unused parameter
                      (uint32_t)0, // Unused parameter
                      NemaDC_dcs_datacmd);

        nemadc_MIPI_CFG_out(ui32Cfg | MIPICFG_SPI_HOLD);
        //
        // Send DCS write_memory_start command
        //
        nemadc_MIPI_out(MIPI_DBIB_CMD | ui32MemWrCmd);
    }
    else if ((ui32Cfg & (MIPICFG_SPI3 | MIPICFG_SPI4 | MIPICFG_DSPI | MIPICFG_QSPI)) != 0)
    {
        //
        // disable clock gates
        //
        nemadc_reg_write(NEMADC_REG_CLKCTRL_CG, NemaDC_clkctrl_cg_clk_en);
        if ( (ui32Cfg & MIPICFG_QSPI) != 0 )
        {
            //
            // QSPI interface
            //
            nemadc_MIPI_CFG_out( ui32Cfg | MIPICFG_SPI_HOLD);
            nemadc_MIPI_out( MIPI_DBIB_CMD | MIPI_MASK_QSPI | CMD1_DATA4);
            nemadc_MIPI_out( MIPI_DBIB_CMD | MIPI_MASK_QSPI | MIPI_CMD24 |
                            (ui32MemWrCmd << CMD_OFFSET));
        }
        else if ( (ui32Cfg & MIPICFG_DSPI) != 0 )
        {
            //
            // DSPI interface
            //
            nemadc_MIPI_CFG_out( ui32Cfg & (~MIPICFG_DSPI));
            // Start MIPI Panel Memory Write
            nemadc_MIPI_out(MIPI_DBIB_CMD | ui32MemWrCmd);
            nemadc_MIPI_CFG_out(((ui32Cfg & (~MIPICFG_SPI4)) | MIPICFG_SPI3) | MIPICFG_SPIDC_DQSPI | MIPICFG_SPI_HOLD);
        }
        else
        {
            //
            // SPI4 interface
            //
            nemadc_MIPI_CFG_out( ui32Cfg | MIPICFG_SPI_HOLD);
            // Start MIPI Panel Memory Write
            nemadc_MIPI_out(MIPI_DBIB_CMD | ui32MemWrCmd);
        }
    }
    //
    // turn DC TE interrupt
    //
    if (ui32Cfg & MIPICFG_DIS_TE)
    {
        nemadc_reg_write(NEMADC_REG_INTERRUPT, 1 << 3);
    }
}
//*****************************************************************************
//
//! @brief Set display region
//!
//! @param ui16ResX     - resolution x
//! @param ui16ResY     - resolution y
//! @param ui16MinX     - vertex of x
//! @param ui16MinY     - vertex of y
//!
//! @note This function is used to set region for partial update.
//!
//! @return status.
//
//*****************************************************************************
uint32_t
set_display_region(uint16_t ui16ResX,
                   uint16_t ui16ResY,
                   uint16_t ui16MinX,
                   uint16_t ui16MinY)
{
    //
    // panel offset
    //
    ui16MinX += g_sDispCfg.ui16Offset;
    nemadc_reg_write(NEMADC_REG_CLKCTRL_CG, NemaDC_clkctrl_cg_clk_en);
    switch (g_sDispCfg.eInterface)
    {
        case DISP_IF_SPI4:
        case DISP_IF_DSPI:
        case DISP_IF_QSPI:
            am_devices_dc_xspi_raydium_set_region(ui16ResX,
                                                  ui16ResY,
                                                  ui16MinX,
                                                  ui16MinY);
            nemadc_timing(ui16ResX, 1, 1, 1,
                          ui16ResY, 1, 1, 1);
            break;
        case DISP_IF_DSI:
            am_devices_dc_dsi_raydium_set_region(ui16ResX,
                                                 ui16ResY,
                                                 ui16MinX,
                                                 ui16MinY);
            nemadc_timing(ui16ResX, 1, 10, 1,
                          ui16ResY, 1, 1, 1);
            break;
        default:
            return 1;
    }
    return 0;
}

//*****************************************************************************
//
//! @brief Implement partial update effect.
//!
//! @return None.
//
//*****************************************************************************
void
nemadc_partial_update_effect(void)
{
    //
    // send layer 0 to display via NemaDC and DSI
    //
    nemadc_layer_t sLayer0 = {0};
    sLayer0.resx = FB_RESX;
#ifdef SMALLFB
    sLayer0.resy = FB_RESY / SMALLFB_STRIPES;
#else
    sLayer0.resy = FB_RESY;
#endif
    sLayer0.buscfg = 0;
    sLayer0.blendmode = NEMADC_BL_SRC;
    sLayer0.startx = 0;
    sLayer0.starty = 0;
    sLayer0.sizex = sLayer0.resx;
    sLayer0.sizey = sLayer0.resy;
    sLayer0.alpha = 0xff;
    sLayer0.flipx_en = 0;
    sLayer0.flipy_en = 0;
    sLayer0.format = NEMADC_RGB565;
    sLayer0.stride = sLayer0.resx * 2;
    sLayer0.baseaddr_virt = tsi_malloc(sLayer0.resy * sLayer0.stride);
    sLayer0.baseaddr_phys = (unsigned)(sLayer0.baseaddr_virt);
    nema_memcpy((char*)sLayer0.baseaddr_virt, pinwheel_400x400_rgba565, sLayer0.resy * sLayer0.stride);

    nemadc_layer_t sLayer1 = {0};
    sLayer1.resx = FB_RESX;
#ifdef SMALLFB
    sLayer1.resy = FB_RESY / SMALLFB_STRIPES;
#else
    sLayer1.resy = FB_RESY;
#endif
    sLayer1.buscfg = 0;
    sLayer1.blendmode = NEMADC_BL_SIMPLE;
    sLayer1.startx = 0;
    sLayer1.starty = 0;
    sLayer1.sizex = sLayer1.resx;
    sLayer1.sizey = sLayer1.resy;
    sLayer1.alpha = 0xff;
    sLayer1.flipx_en = 0;
    sLayer1.flipy_en = 0;
    sLayer1.format = NEMADC_RGB565;
    sLayer1.stride = sLayer1.resx * 2;
    sLayer1.baseaddr_virt = tsi_malloc(sLayer1.resy * sLayer1.stride);
    sLayer1.baseaddr_phys = (unsigned)(sLayer1.baseaddr_virt);
    nema_memcpy((char*)sLayer1.baseaddr_virt, umbrella_400x400_rgba565, sLayer1.resy * sLayer1.stride);

#ifdef DC_WRITE_CONTINUE
    switch (g_sDispCfg.eInterface)
    {
        case DISP_IF_SPI4:
        case DISP_IF_DSPI:
        case DISP_IF_QSPI:
            nemadc_reg_write(NEMADC_REG_CLKCTRL_CG, NemaDC_clkctrl_cg_clk_en);
            nemadc_timing(FB_RESX, 1, 1, 1,
                          FB_RESY / SMALLFB_STRIPES, 1, 1, 1);
            break;
        case DISP_IF_DSI:
            nemadc_reg_write(NEMADC_REG_CLKCTRL_CG, NemaDC_clkctrl_cg_clk_en);
            nemadc_timing(FB_RESX, 1, 10, 1,
                          FB_RESY / SMALLFB_STRIPES, 1, 1, 1);
            break;
        default:
            break;
    }
#endif
    while (1)
    {

        for ( uint8_t ui8layer = 0; ui8layer < 2; ui8layer ++ )
        {
            for (int32_t stripe = 0; stripe < SMALLFB_STRIPES; stripe ++)
            {
#ifndef DC_WRITE_CONTINUE
                //
                // Reposition the display area if necessary
                //
                set_display_region(FB_RESX,
                                   FB_RESY / SMALLFB_STRIPES,
                                   CENTER_ALIGN_X,
                                   CENTER_ALIGN_Y + FB_RESY / SMALLFB_STRIPES * stripe);
#endif //DC_WRITE_CONTINUE
                if ( ui8layer == 1 )
                {
#ifdef SMALLFB
                    nema_memcpy((char*)sLayer1.baseaddr_virt,
                                umbrella_400x400_rgba565 + sLayer1.resy * sLayer1.stride * stripe,
                                sLayer1.resy * sLayer1.stride);
#else
                    sLayer1.starty = -(FB_RESY / SMALLFB_STRIPES * stripe);
                    sLayer1.sizey = sLayer1.resy = FB_RESY / SMALLFB_STRIPES * (stripe + 1);
#endif //SMALLFB
                    nemadc_layer_disable(0);
                    nemadc_set_layer(1, &sLayer1);
                }
                else if ( ui8layer == 0 )
                {
#ifdef SMALLFB
                    nema_memcpy((char*)sLayer0.baseaddr_virt,
                                pinwheel_400x400_rgba565 + sLayer0.resy * sLayer0.stride * stripe,
                                sLayer0.resy * sLayer0.stride);
#else
                    sLayer0.starty = -(FB_RESY / SMALLFB_STRIPES * stripe);
                    sLayer0.sizey = sLayer0.resy = FB_RESY / SMALLFB_STRIPES * (stripe + 1);
#endif //SMALLFB
                    nemadc_set_layer(0, &sLayer0);
                    nemadc_layer_disable(1);
                }
#ifndef DC_WRITE_CONTINUE
                nemadc_transfer_frame_prepare(false);
#else
                if ( stripe == 0 )
                {
                    nemadc_transfer_frame_prepare(false);
                }
                else
                {
                    nemadc_transfer_frame_continue(false);
                }
#endif
                //
                // start frame transfer with DPI disabled
                //
                nemadc_transfer_frame_launch();
                nemadc_wait_vsync();
            }
            am_util_delay_ms(200);
        }
    }
}


//*****************************************************************************
//
// Main function
//
//*****************************************************************************
int
main(void)
{
    //
    // panel's resolution.
    //
    uint32_t ui32MipiCfg = MIPICFG_16RGB888_OPT0;
    uint16_t ui16MinX, ui16MinY;
    nemadc_initial_config_t sDCConfig;
    am_devices_dc_xspi_raydium_config_t sDisplayPanelConfig;

    //
    // External power on
    //
    am_bsp_external_pwr_on();
    am_util_delay_ms(100);
    am_bsp_low_power_init();

    //
    // Initialize the printf interface for ITM/SWO output.
    //
    am_bsp_itm_printf_enable();
    //
    // Clear the terminal and print the banner.
    //
    am_util_stdio_terminal_clear();

    //
    // Global interrupt enable
    //
    am_hal_interrupt_master_enable();

    //
    // Print a banner.
    //
    am_util_stdio_printf("nemadc_partial_update_effect example.\n");

    am_bsp_disp_pins_enable();
    //
    // Set the display region to center
    //
    sDisplayPanelConfig.ui16ResX = FB_RESX;
    if (FB_RESX < g_sDispCfg.ui16ResX)
    {
        ui16MinX = (g_sDispCfg.ui16ResX - sDisplayPanelConfig.ui16ResX) >> 1;
        ui16MinX = (ui16MinX >> 1) << 1;
    }
    else
    {
        ui16MinX = 0;
    }

    sDisplayPanelConfig.ui16ResY = FB_RESY;
    if (FB_RESY < g_sDispCfg.ui16ResY)
    {
        ui16MinY = (g_sDispCfg.ui16ResY - sDisplayPanelConfig.ui16ResY) >> 1;
        ui16MinY = (ui16MinY >> 1) << 1;
    }
    else
    {
        ui16MinY = 0;
    }

    g_sDispCfg.eTEType = DISP_TE_DISABLE;
    sDCConfig.ui16ResX = sDisplayPanelConfig.ui16ResX;
    sDCConfig.ui16ResY = sDisplayPanelConfig.ui16ResY;
    sDCConfig.bTEEnable = (g_sDispCfg.eTEType == DISP_TE_DC);
    sDisplayPanelConfig.ui16MinX = ui16MinX + g_sDispCfg.ui16Offset;
    sDisplayPanelConfig.ui16MinY = ui16MinY;
    sDisplayPanelConfig.bTEEnable = (g_sDispCfg.eTEType != DISP_TE_DISABLE);
    sDisplayPanelConfig.bFlip = g_sDispCfg.bFlip;

    if (g_sDispCfg.eInterface == DISP_IF_DSI)
    {
        //
        // VDD18 control callback function
        //
        am_hal_dsi_register_external_vdd18_callback(am_bsp_external_vdd18_switch);
        //
        // Enable DSI power and configure DSI clock.
        //
        am_hal_dsi_init();
    }
    else
    {
        am_hal_clkgen_control(AM_HAL_CLKGEN_CONTROL_DISPCLKSEL_HFRC96, NULL);
        am_hal_clkgen_control(AM_HAL_CLKGEN_CONTROL_DCCLK_ENABLE, NULL);
    }

    am_hal_pwrctrl_periph_enable(AM_HAL_PWRCTRL_PERIPH_GFX);
    am_hal_pwrctrl_periph_enable(AM_HAL_PWRCTRL_PERIPH_DISP);

    nema_sys_init();
    //
    //Initialize NemaDC
    //
    if (nemadc_init() != 0)
    {
        return -2;
    }

    if (g_sDispCfg.eInterface == DISP_IF_DSI)
    {
        uint8_t ui8LanesNum = g_sDispCfg.ui8NumLanes;
        uint8_t ui8DbiWidth = g_sDispCfg.eDbiWidth;
        uint32_t ui32FreqTrim = g_sDispCfg.eDsiFreq;
        pixel_format_t eFormat = FMT_RGB888;
        if (am_hal_dsi_para_config(ui8LanesNum, ui8DbiWidth, ui32FreqTrim, false) != 0)
        {
            return -3;
        }
        switch (eFormat)
        {
            case FMT_RGB888:
                if (ui8DbiWidth == 16)
                {
                    ui32MipiCfg = MIPICFG_16RGB888_OPT0;
                }
                if (ui8DbiWidth == 8)
                {
                    ui32MipiCfg = MIPICFG_8RGB888_OPT0;
                }
                break;

            case FMT_RGB565:
                if (ui8DbiWidth == 16)
                {
                    ui32MipiCfg = MIPICFG_16RGB565_OPT0;
                }
                if (ui8DbiWidth == 8)
                {
                    ui32MipiCfg = MIPICFG_8RGB565_OPT0;
                }
                break;

            default:
                //
                // invalid color component index
                //
                return -3;
        }
    }
    //
    // Initialize the display
    //
    switch (g_sDispCfg.eInterface)
    {
        case DISP_IF_SPI4:
            am_devices_dc_xspi_raydium_hardware_reset();
            sDCConfig.eInterface = DISP_INTERFACE_SPI4;
            sDCConfig.ui32PixelFormat = MIPICFG_1RGB888_OPT0;
            sDisplayPanelConfig.ui32PixelFormat = sDCConfig.ui32PixelFormat;
            nemadc_configure(&sDCConfig);
            am_devices_dc_xspi_raydium_init(&sDisplayPanelConfig);
            break;
        case DISP_IF_DSPI:
            am_devices_dc_xspi_raydium_hardware_reset();
            sDCConfig.eInterface = DISP_INTERFACE_DSPI;
            sDCConfig.ui32PixelFormat = MIPICFG_2RGB888_OPT0;
            sDisplayPanelConfig.ui32PixelFormat = sDCConfig.ui32PixelFormat;
            nemadc_configure(&sDCConfig);
            am_devices_dc_xspi_raydium_init(&sDisplayPanelConfig);
            break;
        case DISP_IF_QSPI:
            am_devices_dc_xspi_raydium_hardware_reset();
            sDCConfig.eInterface = DISP_INTERFACE_QSPI;
            sDCConfig.ui32PixelFormat = MIPICFG_4RGB888_OPT0;
            sDisplayPanelConfig.ui32PixelFormat = sDCConfig.ui32PixelFormat;
            nemadc_configure(&sDCConfig);
            am_devices_dc_xspi_raydium_init(&sDisplayPanelConfig);
            break;
        case DISP_IF_DSI:
            am_devices_dc_dsi_raydium_hardware_reset();
            sDCConfig.eInterface = DISP_INTERFACE_DBIDSI;
            sDCConfig.ui32PixelFormat = ui32MipiCfg;
            sDisplayPanelConfig.ui32PixelFormat = sDCConfig.ui32PixelFormat;
            nemadc_configure(&sDCConfig);
            am_devices_dc_dsi_raydium_init(&sDisplayPanelConfig);
            break;
        default:
            break;
    }

#ifdef BURST_MODE
    //
    // Initialize for High Performance Mode
    //
    if (am_hal_pwrctrl_mcu_mode_select(AM_HAL_PWRCTRL_MCU_MODE_HIGH_PERFORMANCE) == AM_HAL_STATUS_SUCCESS)
    {
        am_util_stdio_printf("\nOperating in High Performance Mode\n");
    }
    else
    {
        am_util_stdio_printf("\nFailed to Initialize for High Performance Mode operation\n");
    }
#else
    am_util_stdio_printf("\nOperating in Normal Mode\n");
#endif

    nemadc_partial_update_effect();

    while (1)
    {
    }

}

