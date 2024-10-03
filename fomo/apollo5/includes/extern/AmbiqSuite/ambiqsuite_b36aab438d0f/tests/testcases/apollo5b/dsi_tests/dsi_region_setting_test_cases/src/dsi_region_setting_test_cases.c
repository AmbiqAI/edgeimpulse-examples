//*****************************************************************************
//
//! @file dsi_region_setting_test_cases.c
//!
//! @brief DSI bit rate test cases.
//! Need to connect RM67162 to DSI interface, you will see a static windmill image
//! on display without any tearing and noise if DSI works.
//! This test case can be considered to be passed only if image
//! showed on display is correct. Please ignore the swo log.
//
//*****************************************************************************

//*****************************************************************************
//
// ${copyright}
//
// This is part of revision ${version} of the AmbiqSuite Development Package.
//
//*****************************************************************************
#include "dsi_test_common.h"
#include "pinwheel_400x400_rgba565.h"
#include "umbrella_400x400_rgba565.h"

//*****************************************************************************
//
// Macro definitions
//
//*****************************************************************************
//
// It has the restriction that the layer size is multiple of 4 for Apollo5a.
//
#define FB_RESX 400
#define FB_RESY 400
#define SMALLFB_STRIPES     4
#define CENTER_ALIGN_X      ((g_sDispCfg.ui16ResX - FB_RESX) >> 2 << 1)
#define CENTER_ALIGN_Y      ((g_sDispCfg.ui16ResY - FB_RESY) >> 2 << 1)

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
static uint32_t
set_display_region(uint16_t ui16ResX,
                   uint16_t ui16ResY,
                   uint16_t ui16MinX,
                   uint16_t ui16MinY)
{
    //
    // panel offset
    //
    ui16MinX += g_sDispCfg.ui16Offset;

    am_devices_dc_dsi_raydium_set_region(ui16ResX,
                                         ui16ResY,
                                         ui16MinX,
                                         ui16MinY);
    nemadc_timing(ui16ResX, 1, 10, 1,
                  ui16ResY, 1, 1, 1);

    return 0;
}

//*****************************************************************************
//
//! @brief Test DBI2DSI interface.
//! @param uint8_t ui8LanesNum, uint8_t ui8DbiWidth, uint32_t ui32FreqTrim, pixel_format_enum eFormat
//! @return bPass.
//
//*****************************************************************************
bool
test_MIPI_DSI(uint8_t ui8LanesNum, uint8_t ui8DbiWidth, uint32_t ui32FreqTrim, pixel_format_t eFormat)
{
    uint16_t ui16Cnt = 1000;
    uint32_t ui32MipiCfg = MIPICFG_16RGB888_OPT0;
    uint16_t ui16MinX, ui16MinY;
    nemadc_initial_config_t sDCConfig;
    am_devices_dc_dsi_raydium_config_t sDisplayPanelConfig;

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

    am_bsp_disp_pins_enable();

    am_hal_dsi_para_config(ui8LanesNum, ui8DbiWidth, ui32FreqTrim, false);

// #### INTERNAL BEGIN ####
#ifdef APOLLO5_FPGA
    //
    // Initialize external DPHY board
    //
    if (dphy_init_with_para(DPHY_X8) != 0) 
    {
        return false;
    }
#endif
// #### INTERNAL END ####

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
            return false;
    }
    am_devices_dc_dsi_raydium_hardware_reset();
    sDCConfig.eInterface = DISP_INTERFACE_DBIDSI;
    sDCConfig.ui32PixelFormat = ui32MipiCfg;
    sDisplayPanelConfig.ui32PixelFormat = sDCConfig.ui32PixelFormat;
    nemadc_configure(&sDCConfig);
    am_devices_dc_dsi_raydium_init(&sDisplayPanelConfig);

    //
    // send layer 0 to display via NemaDC and DSI
    //
    nemadc_layer_t sLayer0 = {0};
    sLayer0.resx = FB_RESX;
    sLayer0.resy = FB_RESY / SMALLFB_STRIPES;
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
    sLayer1.resy = FB_RESY / SMALLFB_STRIPES;
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

    while (ui16Cnt--)
    {
        for(uint8_t ui8layer = 0;ui8layer < 2; ui8layer ++)
        {
            for (int32_t stripe = 0; stripe < SMALLFB_STRIPES; stripe ++)
            {
                //
                // Reposition the display area if necessary
                //
                set_display_region(FB_RESX,
                                   FB_RESY / SMALLFB_STRIPES,
                                   CENTER_ALIGN_X,
                                   CENTER_ALIGN_Y + FB_RESY / SMALLFB_STRIPES * stripe);
                if(ui8layer == 1)
                {
                    nema_memcpy((char*)sLayer1.baseaddr_virt, 
                                umbrella_400x400_rgba565 + sLayer1.resy * sLayer1.stride * stripe, 
                                sLayer1.resy * sLayer1.stride);

                    nemadc_layer_disable(0);
                    nemadc_set_layer(1, &sLayer1);
                }
                else if(ui8layer == 0)
                {
                    nema_memcpy((char*)sLayer0.baseaddr_virt, 
                                pinwheel_400x400_rgba565 + sLayer0.resy * sLayer0.stride * stripe, 
                                sLayer0.resy * sLayer0.stride);
                    
                    nemadc_set_layer(0, &sLayer0);
                    nemadc_layer_disable(1);
                }
                nemadc_transfer_frame_prepare(false);
                //
                // start frame transfer with DPI disabled
                //
                nemadc_transfer_frame_launch();

                uint32_t ui32usMaxDelay = 1000000; // 1 sec
                uint32_t ui32Status;
                //
                // bit 4 is frame end interrupt(JDI,DPI,DBI) or bit 5 is spi frame end interrupt(SPI4,DSPI,QSPI)
                //
                ui32Status = am_hal_delay_us_status_change(ui32usMaxDelay, (uint32_t)&DC->INTERRUPT, 3UL << 4, 0);

                if (ui32Status != AM_HAL_STATUS_SUCCESS)
                {
                    am_util_stdio_printf("\nDC missed frame end interrupt,Display work abnormal!\n");
                    return false;
                }
            }
        }
        if (ui16Cnt % 20 == 0)
        {
            am_util_stdio_printf(".");
        }
    }
    return true;
}

//*****************************************************************************
//
//! @brief Test display region setting with DSI.
//!
//! @return bTestPass.
//
//*****************************************************************************
bool
dsi_region_setting_test(void)
{
    bool bTestPass = true;
    bool bRet = true;
    uint8_t ui8Lanes;
    uint8_t ui8Width;
    uint32_t ui32Trim;
    pixel_format_t eColor;

    ui8Lanes = 1; //!< 1 lane
    ui8Width = 16; //!< 16 bit DBI bus
    ui32Trim = 0x0A; //!< 240 MHz
    eColor = FMT_RGB888;
    bRet = test_MIPI_DSI(ui8Lanes, ui8Width, ui32Trim, eColor);
    if(bRet == false)
    {
        bTestPass = false;
        am_util_stdio_printf("\n%d-Lane, %d-bit DBI width, Frequence trim value - 0x%02X, RGBA8888 DSI throughput test failed.\n", ui8Lanes, ui8Width, ui32Trim);
    }
    else
    {
        am_util_stdio_printf("\n%d-Lane, %d-bit DBI width, Frequence trim value - 0x%02X, RGBA8888 DSI throughput test passed.\n", ui8Lanes, ui8Width, ui32Trim);
    }

    TEST_ASSERT_TRUE(bTestPass);

    return bTestPass;
} // dsi_speed_test()

