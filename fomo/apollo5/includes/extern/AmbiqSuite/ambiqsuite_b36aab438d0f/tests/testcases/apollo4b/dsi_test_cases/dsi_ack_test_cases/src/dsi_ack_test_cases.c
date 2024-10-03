//*****************************************************************************
//
//! @file dsi_ack_test_cases.c
//!
//! @brief DSI ACK test cases.
//! Need to connect RM67162 to DSI interface.
//! You will see a windmill image if DSI works.
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
#include "windmill_200x200_rgba565.h"

//*****************************************************************************
//
// Macro definitions
//
//*****************************************************************************
#define FB_RESX 200
#define FB_RESY 200
#define NUM_OF_LANE_OPTIONS  1
#define NUM_OF_WIDTH_OPTIONS 1
#define NUM_OF_TRIM_OPTIONS  6
#define NUM_OF_COLOR_MODES   2

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
    uint16_t ui16MinX, ui16MinY;
    nemadc_initial_config_t sDCConfig;
    am_devices_dc_dsi_raydium_config_t sDisplayPanelConfig;
    uint32_t ui32MipiCfg;
    bool bPass = true;

    switch ( eFormat )
    {
        case FMT_RGB888:
            if ( ui8DbiWidth == 16 )
            {
                ui32MipiCfg = MIPICFG_16RGB888_OPT0;
            }
            if ( ui8DbiWidth ==  8 )
            {
                ui32MipiCfg = MIPICFG_8RGB888_OPT0;
            }
            break;

        case FMT_RGB565:
            if ( ui8DbiWidth == 16 )
            {
                ui32MipiCfg = MIPICFG_16RGB565_OPT0;
            }
            if ( ui8DbiWidth ==  8 )
            {
                ui32MipiCfg = MIPICFG_8RGB565_OPT0;
            }
            break;

        default:
            //
            // invalid color component index
            //
            bPass = false;
            return bPass;
    }
    //
    // Initialize DSI
    //
    if ( am_hal_dsi_para_config(ui8LanesNum, ui8DbiWidth, ui32FreqTrim, false) != 0 )
    {
        bPass = false;
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
    sDisplayPanelConfig.ui32PixelFormat = sDCConfig.ui32PixelFormat;
    nemadc_configure(&sDCConfig);
    am_devices_dc_dsi_raydium_init(&sDisplayPanelConfig );
    //
    // send layer 0 to display via NemaDC and DSI
    //
    nemadc_layer_t layer0;
    layer0.resx          = FB_RESX;
    layer0.resy          = FB_RESY;
    layer0.buscfg        = 0;
    layer0.blendmode     = NEMADC_BL_SRC;
    if (eFormat == FMT_RGB888)
    {
        layer0.format        = NEMADC_RGBA8888;
        layer0.stride        = layer0.resx * 4;
    }
    else if (eFormat == FMT_RGB565)
    {
        layer0.format        = NEMADC_RGB565;
        layer0.stride        = layer0.resx * 2;
    }
    else
    {
        //
        // invalid color component index
        //
        bPass = false;
        return bPass;
    }
    layer0.startx        = 0;
    layer0.starty        = 0;
    layer0.sizex         = layer0.resx;
    layer0.sizey         = layer0.resy;
    layer0.alpha         = 0xff;
    layer0.flipx_en      = 0;
    layer0.flipy_en      = 0;
    layer0.baseaddr_virt = tsi_malloc(layer0.resy*layer0.stride);
    layer0.baseaddr_phys = (unsigned)tsi_virt2phys(layer0.baseaddr_virt);
    if (eFormat == FMT_RGB888)
    {
        memcpy((char*)layer0.baseaddr_virt, windmill_200x200_rgba8888, sizeof(windmill_200x200_rgba8888));
    }
    else if (eFormat == FMT_RGB565)
    {
        memcpy((char*)layer0.baseaddr_virt, windmill_200x200_rgba565, sizeof(windmill_200x200_rgba565));
    }
    else
    {
        //
        // invalid color component index
        //
        bPass = false;
        return bPass;
    }
    //
    // Program NemaDC Layer0
    //
    nemadc_set_layer(0, &layer0); // This function includes layer enable.
    nemadc_transfer_frame_prepare(false);
    //
    //It's necessary to launch frame manually when TE is disabled.
    //
    nemadc_transfer_frame_launch();
    nemadc_wait_vsync();
    nemadc_layer_disable(0);
    tsi_free(layer0.baseaddr_virt);
    return bPass;
}

//*****************************************************************************
//
//! @brief Test DSI interface.
//!
//! @return bTestPass.
//
//*****************************************************************************
bool
dsi_ack_test(void)
{
    bool bTestPass = true;
    bool bRet = true;
    uint8_t ui8Lanes, ui8I, ui8J, ui8K;
    //
    // set sweeping range
    //
    uint8_t ui8MaxLanes = NUM_OF_LANE_OPTIONS;
    uint8_t ui8Width[2] = {16, 8}; // Add 16bit test when test Rev B
    uint32_t ui32Trim[6] = {0x06, 0x07, 0x08, 0x09, 0x0A, 0x05}; // Add more trim values when test Rev B
    pixel_format_t eColor = FMT_RGB888;

    for(ui8Lanes = 1; ui8Lanes <= ui8MaxLanes; ui8Lanes++)
    {
        for(ui8I = 0; ui8I < NUM_OF_WIDTH_OPTIONS; ui8I++)
        {
            for(ui8K = 0; ui8K < NUM_OF_COLOR_MODES; ui8K++)
            {
                if(ui8K == 0)
                {
                    eColor = FMT_RGB888;
                }
                else if(ui8K == 1)
                {
                    eColor = FMT_RGB565;
                }
//                else if(ui8K == 2) // Add FMT_RGB666 option when test Rev B
//                {
//                    eColor = FMT_RGB666;
//                }
                else
                {
                    //
                    // invalid color component index
                    //
                    bTestPass = false;
                    return bTestPass;
                }
                for(ui8J = 0; ui8J < NUM_OF_TRIM_OPTIONS; ui8J++)
                {
                    if((ui8K == 1) && ((ui32Trim[ui8J] & 0x0F) > 0x06))
                    {
                        am_util_stdio_printf("Higher than maximum bit rate which RM67162 supports, Skipped %d-Lane, %d-bit DBI width, Frequence trim value - 0x%02X, RGB565 DSI test.\n", ui8Lanes, ui8Width[ui8I], ui32Trim[ui8J]);
                    }
                    else
                    {
                        bRet = test_MIPI_DSI(ui8Lanes, ui8Width[ui8I], ui32Trim[ui8J], eColor);
                        if(bRet == false)
                        {
                            bTestPass = false;
                            if (eColor == FMT_RGB888) // Add RGB666 option when test Rev B
                            {
                                am_util_stdio_printf("%d-Lane, %d-bit DBI width, Frequence trim value - 0x%02X, RGBA8888 DSI test failed.\n", ui8Lanes, ui8Width[ui8I], ui32Trim[ui8J]);
                            }
                            if (eColor == FMT_RGB565)
                            {
                                am_util_stdio_printf("%d-Lane, %d-bit DBI width, Frequence trim value - 0x%02X, RGB565 DSI test failed.\n", ui8Lanes, ui8Width[ui8I], ui32Trim[ui8J]);
                            }
                        }
                        else
                        {
                            if (eColor == FMT_RGB888)
                            {
                                am_util_stdio_printf("%d-Lane, %d-bit DBI width, Frequence trim value - 0x%02X, RGBA8888 DSI test passed.\n", ui8Lanes, ui8Width[ui8I], ui32Trim[ui8J]);
                            }
                            if (eColor == FMT_RGB565)
                            {
                                am_util_stdio_printf("%d-Lane, %d-bit DBI width, Frequence trim value - 0x%02X, RGB565 DSI test passed.\n", ui8Lanes, ui8Width[ui8I], ui32Trim[ui8J]);
                            }
                        }
                        am_util_delay_ms(500);
                    }
                }
            }
        }
    }

    TEST_ASSERT_TRUE(bTestPass);

    return bTestPass;

} // dsi_ack_test()

