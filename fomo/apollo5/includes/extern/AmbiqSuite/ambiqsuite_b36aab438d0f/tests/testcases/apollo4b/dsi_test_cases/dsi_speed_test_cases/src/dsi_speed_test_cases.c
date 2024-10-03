//*****************************************************************************
//
//! @file dsi_speed_test_cases.c
//!
//! @brief DSI bit rate test cases.
//! Need to connect RM67162 to DSI interface, you will see a windmill image on
//! display if DSI works.
//! This test case can be considered to be passed only if both SWO log and image
//! showed on display are correct.
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
#include "windmill_200x200_rgba8888.h"

//*****************************************************************************
//
// Macro definitions
//
//*****************************************************************************
#define FB_RESX 200
#define FB_RESY 200

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
    bool bPass = false;
    float fStartTime, fStopTime, fbps;
    uint16_t ui16MinX, ui16MinY;
    nemadc_initial_config_t sDCConfig;
    am_devices_dc_dsi_raydium_config_t sDisplayPanelConfig;
    uint32_t ui32MipiCfg;

    switch (eFormat)
    {
        case FMT_RGB888:
            if (ui8DbiWidth == 16)
            {
                ui32MipiCfg = MIPICFG_16RGB888_OPT0;
            }
            if (ui8DbiWidth ==  8)
            {
                ui32MipiCfg = MIPICFG_8RGB888_OPT0;
            }
            break;

        case FMT_RGB565:
            if (ui8DbiWidth == 16)
            {
                ui32MipiCfg = MIPICFG_16RGB565_OPT0;
            }
            if (ui8DbiWidth ==  8)
            {
                ui32MipiCfg = MIPICFG_8RGB565_OPT0;
            }
            break;

        default:
            //
            // invalid color component index
            //
            return bPass;
    }
    //
    // Initialize DSI
    //
    if (am_hal_dsi_para_config(ui8LanesNum, ui8DbiWidth, ui32FreqTrim, false) != 0)
    {
        return bPass;
    }
    //
    // Set the display region to center
    //
    if (FB_RESX < g_sDispCfg.ui16ResX)
    {
        sDisplayPanelConfig.ui16ResX = FB_RESX;
    }
    else
    {
        sDisplayPanelConfig.ui16ResX = g_sDispCfg.ui16ResX;
    }
    ui16MinX = (g_sDispCfg.ui16ResX - sDisplayPanelConfig.ui16ResX) >> 1;
    ui16MinX = (ui16MinX >> 1) << 1;

    if (FB_RESY < g_sDispCfg.ui16ResY)
    {
        sDisplayPanelConfig.ui16ResY = FB_RESY;
    }
    else
    {
        sDisplayPanelConfig.ui16ResY = g_sDispCfg.ui16ResY;
    }
    ui16MinY = (g_sDispCfg.ui16ResY - sDisplayPanelConfig.ui16ResY) >> 1;
    ui16MinY = (ui16MinY >> 1) << 1;

    g_sDispCfg.eTEType = DISP_TE_DISABLE;
    sDCConfig.ui16ResX = sDisplayPanelConfig.ui16ResX;
    sDCConfig.ui16ResY = sDisplayPanelConfig.ui16ResY;
    sDCConfig.bTEEnable = (g_sDispCfg.eTEType == DISP_TE_DC);
    sDisplayPanelConfig.ui16MinX = ui16MinX + g_sDispCfg.ui16Offset;
    sDisplayPanelConfig.ui16MinY = ui16MinY;
    sDisplayPanelConfig.bTEEnable = (g_sDispCfg.eTEType != DISP_TE_DISABLE);
    sDisplayPanelConfig.bFlip = g_sDispCfg.bFlip;

    am_bsp_disp_pins_enable();

    am_devices_dc_dsi_raydium_hardware_reset();
    sDCConfig.eInterface = DISP_INTERFACE_DBIDSI;
    sDCConfig.ui32PixelFormat = ui32MipiCfg;
    sDisplayPanelConfig.ui32PixelFormat = ui32MipiCfg;
    nemadc_configure(&sDCConfig);

    if (AM_DEVICES_DISPLAY_STATUS_SUCCESS != am_devices_dc_dsi_raydium_init(&sDisplayPanelConfig))
    {
        return false;
    }
    //
    // send layer 0 to display via NemaDC and DSI
    //
    nemadc_layer_t layer0 = {0};
    layer0.resx          = FB_RESX;
    layer0.resy          = FB_RESY;
    layer0.buscfg        = 0;
    layer0.blendmode     = NEMADC_BL_SRC;
    layer0.format        = NEMADC_RGBA8888;
    layer0.stride        = layer0.resx * 4;
    layer0.startx        = 0;
    layer0.starty        = 0;
    layer0.sizex         = layer0.resx;
    layer0.sizey         = layer0.resy;
    layer0.alpha         = 0xff;
    layer0.flipx_en      = 0;
    layer0.flipy_en      = 0;
    layer0.baseaddr_virt = tsi_malloc(layer0.resy*layer0.stride);
    layer0.baseaddr_phys = (unsigned)tsi_virt2phys(layer0.baseaddr_virt);
    memcpy((char*)layer0.baseaddr_virt, windmill_200x200_rgba8888, sizeof(windmill_200x200_rgba8888));
    //
    // Program NemaDC Layer0
    //
    nemadc_set_layer(0, &layer0);
    fStartTime = nema_get_time();

    nemadc_transfer_frame_prepare(false);
    //
    //It's necessary to launch frame manually when TE is disabled.
    //
    nemadc_transfer_frame_launch();
    nemadc_wait_vsync();

    fStopTime = nema_get_time();
    fbps = (FB_RESX * FB_RESY * 24) / (fStopTime - fStartTime);
    nemadc_layer_disable(0);
    tsi_free(layer0.baseaddr_virt);
    if (fbps >= 480000000.0f * 0.9f)
    {
        am_util_stdio_printf("DSI throughput is correct, its bit rate is %.1f Mbps, please also check the image displayed on panel.\n", fbps / 1000000.f);
        return bPass;
    }
    else
    {
        am_util_stdio_printf("DSI throughput is too low, its bit rate is %.1f Mbps.\n", fbps / 1000000.f);
        bPass = false;
        return bPass;
    }
}

//*****************************************************************************
//
//! @brief Test DSI bit rate.
//!
//! @return bTestPass.
//
//*****************************************************************************
bool
dsi_speed_test(void)
{
    bool bTestPass = true;
    bool bRet = true;
    uint8_t ui8Lanes;
    uint8_t ui8Width;
    uint32_t ui32Trim;
    pixel_format_t eColor;
    //
    // Timer init
    //
    am_hal_timer_config_t sTimerConfig;
    uint32_t ui32Status;
    ui32Status = am_hal_timer_default_config_set(&sTimerConfig);
    if (AM_HAL_STATUS_SUCCESS != ui32Status)
    {
        am_util_stdio_printf("Failed to initialize a timer configuration structure with default values!\n");
    }
    sTimerConfig.eInputClock = AM_HAL_TIMER_CLOCK_HFRC_DIV16;
    sTimerConfig.eFunction = AM_HAL_TIMER_FN_UPCOUNT;
    ui32Status = am_hal_timer_config(0, &sTimerConfig);
    if (AM_HAL_STATUS_SUCCESS != ui32Status)
    {
        am_util_stdio_printf("Failed to configure a timer!\n");
    }
    ui32Status = am_hal_timer_start(0);
    if (AM_HAL_STATUS_SUCCESS != ui32Status)
    {
        am_util_stdio_printf("Failed to start a timer!\n");
    }
    //
    // Assign a fixed value to display type.
    //
    ui8Lanes = 1; //!< 1 lane
    ui8Width = 16; //!< 16 bit DBI bus
    ui32Trim = 0x0A; //!< 240 MHz
    eColor = FMT_RGB888;
    bRet = test_MIPI_DSI(ui8Lanes, ui8Width, ui32Trim, eColor);
    if(bRet == false)
    {
        bTestPass = false;
        am_util_stdio_printf("%d-Lane, %d-bit DBI width, Frequence trim value - 0x%02X, RGBA8888 DSI throughput test failed.\n", ui8Lanes, ui8Width, ui32Trim);
    }
    else
    {
        am_util_stdio_printf("%d-Lane, %d-bit DBI width, Frequence trim value - 0x%02X, RGBA8888 DSI throughput test passed.\n", ui8Lanes, ui8Width, ui32Trim);
    }

    TEST_ASSERT_TRUE(bTestPass);

    return bTestPass;
} // dsi_speed_test()
