//*****************************************************************************
//
//! @file dsi_speed_test_cases.c
//!
//! @brief DSI bit rate test cases.
//! Need to connect RM67162 to DSI interface, you will see a golden image on
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
#include "golden_340_rgba565.h"

//*****************************************************************************
//
// Macro definitions
//
//*****************************************************************************
#define FB_RESX 340
#define FB_RESY 340
#define LIMITED_TIME    (7.2f)

//*****************************************************************************
//
//! @brief test DBI2DSI interface performance.
//!
//! @param ui8LanesNum - DSI data lanes number.
//! @param ui8DbiWidth - DBI interface width.
//! @param ui32FreqTrim - DSI frequency trim value.
//! @param eFormat - MIPI configuration bits data format.
//!
//! THis function can caculate DSI bit rate(Mbps) and transfer time per frame(fps).
//!
//! @return bPass.
//
//*****************************************************************************
bool
test_MIPI_DSI(uint8_t ui8LanesNum, uint8_t ui8DbiWidth, uint32_t ui32FreqTrim, pixel_format_t eFormat)
{
    bool bPass = false;
    float fStartTime, fStopTime, fMbps,fTransferTime;
    uint16_t ui16MinX, ui16MinY;
    nemadc_initial_config_t sDCConfig;
    am_devices_dc_dsi_raydium_config_t sDisplayPanelConfig;
    uint32_t ui32MipiCfg;
    size_t framebuffersize=0;

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
    layer0.format        = NEMADC_RGB565;
    layer0.stride        = layer0.resx * 2;
    layer0.startx        = 0;
    layer0.starty        = 0;
    layer0.sizex         = layer0.resx;
    layer0.sizey         = layer0.resy;
    layer0.alpha         = 0xff;
    layer0.flipx_en      = 0;
    layer0.flipy_en      = 0;
    layer0.baseaddr_virt = tsi_malloc(layer0.resy*layer0.stride);
    layer0.baseaddr_phys = (unsigned)(layer0.baseaddr_virt);
    framebuffersize = sizeof(golden_340_rgba565);
    memcpy((char*)layer0.baseaddr_virt, golden_340_rgba565,framebuffersize);
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

    fTransferTime = (fStopTime - fStartTime) * 1000.f;      //! convert time unit from second to millisecond
    fMbps = 8 * framebuffersize / fTransferTime / 1000.f;   //! convert to Megabits per second
    nemadc_layer_disable(0);
    tsi_free(layer0.baseaddr_virt);

    if (fTransferTime <= LIMITED_TIME)
    {
        bPass = true;
    }
    am_util_stdio_printf("DSI transfer time %.1f ms is %s the maximum tolerance %.1f ms,in addition,its bit rate is %.1f Mbps.\n", \
                        fTransferTime,bPass == true ? "below" :"exceeding",LIMITED_TIME,fMbps);
    return bPass;
}

//*****************************************************************************
//
//! @brief Test DSI bit rate and transfer time.
//!
//! @return bTestPass.
//
//*****************************************************************************
bool
dsi_speed_test(void)
{
    bool bTestPass = false;
    uint8_t ui8Lanes, ui8Width;
    uint32_t ui32Trim,ui32Status;
    pixel_format_t eColor;
    //
    // Timer init
    //
    am_hal_timer_config_t sTimerConfig;
    ui32Status = am_hal_timer_default_config_set(&sTimerConfig);
    if (AM_HAL_STATUS_SUCCESS != ui32Status)
    {
        am_util_stdio_printf("Failed to initialize a timer configuration structure with default values!\n");
    }
    sTimerConfig.eInputClock = AM_HAL_TIMER_CLOCK_HFRC_DIV16;
    sTimerConfig.eFunction = AM_HAL_TIMER_FN_EDGE;
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

    ui8Lanes = 1; //!< 1 lane
    ui8Width = 16; //!< 16 bit DBI bus
    //
    // Maximum total bit rate is 500Mbps(DDR) of 24-bit data format(RGB888)/320Mbps(DDR) of 16-bit data format(RGB565), reference RM67162/RM69330 datasheet.
    //
    eColor = FMT_RGB565;
    if(eColor == FMT_RGB888)
    {
        am_util_stdio_printf("24-bit data format.\n");
        ui32Trim = AM_HAL_DSI_FREQ_TRIM_X20; //!< 12MHz * 20 = 240MHz, 480Mbps(DDR)
    }
    else if(eColor == FMT_RGB565)
    {
        am_util_stdio_printf("16-bit data format.\n");
        ui32Trim = AM_HAL_DSI_FREQ_TRIM_X13; //!< 12MHz * 13 = 156MHz, 312Mbps(DDR)
    }
    else
    {
        am_util_stdio_printf("invalid bit data format.\n");
        return bTestPass;
    }

    bTestPass = test_MIPI_DSI(ui8Lanes, ui8Width, ui32Trim, eColor);
    am_util_stdio_printf("%d-Lane, %d-bit DBI width, Frequence trim value - 0x%02X.\nDSI throughput test %s.\n", \
                            ui8Lanes, ui8Width, ui32Trim,bTestPass == true ? "Passed" : "Failed");
    TEST_ASSERT_TRUE(bTestPass);
    return bTestPass;
} // dsi_speed_test()

