//*****************************************************************************
//
//! @file dc_pwrctrl_test_cases.c
//!
//! @brief DC power control test cases.
//! Need to connect RM67162 to DC SPI4 interface.
//
//*****************************************************************************

//*****************************************************************************
//
// ${copyright}
//
// This is part of revision ${version} of the AmbiqSuite Development Package.
//
//*****************************************************************************
#include "gpu_dc_pwrctrl_test_common.h"
#include "oli_200x200_rgba.h"

//*****************************************************************************
//
// Macro definitions
//
//*****************************************************************************
#define FB_RESX 200
#define FB_RESY 200
#define MAX_CNT 100

//*****************************************************************************
//
//! @brief Test DC power control
//! @param pixel_format Panel pixel format
//!
//! @return bPass
//
//*****************************************************************************
bool
test_MIPI_SPI(int pixel_format )
{
    uint16_t ui16MinX, ui16MinY;
    nemadc_initial_config_t sDCConfig;
    am_devices_dc_xspi_raydium_config_t sDisplayPanelConfig;
    bool bPass = false;
    //
    // Power up DC
    //
    am_hal_pwrctrl_periph_enable(AM_HAL_PWRCTRL_PERIPH_DISP);
    //
    // Initialize NemaDC
    //
    if ( nemadc_init() != 0 )
    {
        bPass = false;
        am_util_stdio_printf("DC init failed!\n");
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

    am_devices_dc_xspi_raydium_hardware_reset();
    sDCConfig.eInterface = DISP_INTERFACE_SPI4;
    sDCConfig.ui32PixelFormat = MIPICFG_1RGB888_OPT0;
    sDisplayPanelConfig.ui32PixelFormat = sDCConfig.ui32PixelFormat;
    nemadc_configure(&sDCConfig);
    am_devices_dc_xspi_raydium_init(&sDisplayPanelConfig);
    //
    // send layer 0 to display via NemaDC
    //
    nemadc_layer_t layer0 = {0};
    layer0.resx          = FB_RESX;
    layer0.resy          = FB_RESY;
    layer0.buscfg        = 0;
    layer0.format        = NEMADC_RGBA8888;
    layer0.blendmode     = NEMADC_BL_SRC;
    layer0.stride        = layer0.resx*4;
    layer0.startx        = 0;
    layer0.starty        = 0;
    layer0.sizex         = layer0.resx;
    layer0.sizey         = layer0.resy;
    layer0.alpha         = 0xff;
    layer0.flipx_en      = 0;
    layer0.flipy_en      = 0;
    layer0.baseaddr_virt = (void *)0x10060438;//(void *)0x10061000; When FB start address was set to 0x10061000, tests passed on Rev B0.
    layer0.baseaddr_phys = (unsigned)(layer0.baseaddr_virt);

    memcpy((char*)layer0.baseaddr_virt, oli_200x200_rgba, sizeof(oli_200x200_rgba));
    //
    // Program NemaDC Layer0
    //
    nemadc_set_layer(0, &layer0); // This function includes layer enable.

    nemadc_transfer_frame_prepare(false);
    //
    //It's necessary to launch frame manually when TE is disabled.
    //
    nemadc_transfer_frame_launch();

    static uint32_t ui32usMaxDelay = 1000000; // 1 sec
    uint32_t ui32Status;
    ui32Status = am_hal_delay_us_status_change(ui32usMaxDelay, (uint32_t)&DC->INTERRUPT, 1UL << 4, 0);
    if (ui32Status != AM_HAL_STATUS_SUCCESS)
    {
        bPass = false;
        return bPass;
    }

    tsi_free(layer0.baseaddr_virt);
    //
    // Power down DC
    //
    am_hal_pwrctrl_periph_disable(AM_HAL_PWRCTRL_PERIPH_DISP);
    bPass = true;
    return bPass;
}

//*****************************************************************************
//
//! @brief NemaDC spi4 test case with power control
//!
//!
//!
//! @return bTestPass.
//
//*****************************************************************************
bool
dc_pwrctrl_test(void)
{
    bool bTestPass = true;
    uint32_t ui32Cnt = 0, ui32PassCnt = 0;

    g_sDispCfg.eInterface = DISP_IF_SPI4;

    if (g_sDispCfg.eInterface != DISP_IF_SPI4)
    {
        bTestPass = false;
        return bTestPass; // If the interface in BSP isn't set to SPI4, then return.
    }

    am_hal_clkgen_control(AM_HAL_CLKGEN_CONTROL_DISPCLKSEL_HFRC96, NULL);
    am_hal_clkgen_control(AM_HAL_CLKGEN_CONTROL_DCCLK_ENABLE, NULL);

    for (ui32Cnt = 0; ui32Cnt < MAX_CNT; ui32Cnt++)
    {
        if (true != test_MIPI_SPI(MIPICFG_1RGB888_OPT0))
        {
            bTestPass = false;
            break;
        }
        else
        {
            am_util_stdio_printf("DC power control test passed %d time(s), total number of times set is %d.\n", ++ui32PassCnt, MAX_CNT);
        }
        am_util_delay_ms(100);
    }

    TEST_ASSERT_TRUE(bTestPass);
    return bTestPass;
}
