//*****************************************************************************
//
//! @file nemadc_4layer.c
//!
//! @brief NemaDC 4-Layer Example.
//!
//! @addtogroup graphics_examples Graphics Examples
//!
//! @defgroup nemadc_4layer NemaDC 4-Layer Example
//! @ingroup graphics_examples
//! @{
//!
//! Purpose: This example demonstrates DC 4 layers overlay with global alpha blending.
//!   * Layer0 - Red image
//!   * Layer1 - Green image
//!   * Layer2 - Blue image
//!   * Layer3 - Yellow image
//! Global alpha value can be changed in layer*.alpha. Blendmode can be changed
//! in layer*.blendmode.
//!
//! This example can work at two different SPI interfaces. When defined ENABLE_SPI4
//! in preprocessor defined symbols, this example drives panel through SPI4 interface.
//! When defined ENABLE_QSPI in preprocessor defined symbols, this example drives
//! panel through QSPI interface.
//
//*****************************************************************************

//*****************************************************************************
//
// ${copyright}
//
// This is part of revision ${version} of the AmbiqSuite Development Package.
//
//*****************************************************************************

// -----------------------------------------------------------------------------
// Copyright (c) 2019 Think Silicon S.A.
// Think Silicon S.A. Confidential Proprietary
// -----------------------------------------------------------------------------
//     All Rights reserved - Unpublished -rights reserved under
//         the Copyright laws of the European Union
//
//  This file includes the Confidential information of Think Silicon S.A.
//  The receiver of this Confidential Information shall not disclose
//  it to any third party and shall protect its confidentiality by
//  using the same degree of care, but not less than a reasonable
//  degree of care, as the receiver uses to protect receiver's own
//  Confidential Information. The entire notice must be reproduced on all
//  authorised copies and copies may only be made to the extent permitted
//  by a licensing agreement from Think Silicon S.A..
//
//  The software is provided 'as is', without warranty of any kind, express or
//  implied, including but not limited to the warranties of merchantability,
//  fitness for a particular purpose and noninfringement. In no event shall
//  Think Silicon S.A. be liable for any claim, damages or other liability, whether
//  in an action of contract, tort or otherwise, arising from, out of or in
//  connection with the software or the use or other dealings in the software.
//
//
//                    Think Silicon S.A.
//                    http://www.think-silicon.com
//                    Patras Science Park
//                    Rion Achaias 26504
//                    Greece
// -----------------------------------------------------------------------------
#include "nemadc_4layer.h"

#define FB_RESX 200
#define FB_RESY 200

typedef volatile struct __FB32
{
    uint32_t ui32Pixels[FB_RESY][FB_RESX];
}
FB32Type_t;

//*****************************************************************************
//
//! @brief test Display Controller's four layers.
//!
//! This function demonstrate DC's four layers respectively.
//!
//! @return 0.
//
//*****************************************************************************
int32_t
test_nemadc_4layer()
{
    FB32Type_t *sFB320;
    FB32Type_t *sFB321;
    FB32Type_t *sFB322;
    FB32Type_t *sFB323;

    nemadc_layer_t sLayer0 = {0};
    sLayer0.resx = FB_RESX;
    sLayer0.resy = FB_RESY;
    sLayer0.buscfg = 0;
    sLayer0.format = NEMADC_RGBA8888;
    sLayer0.blendmode = NEMADC_BL_SRC;
    sLayer0.stride = sLayer0.resx * 4;
    sLayer0.startx = 0;
    sLayer0.starty = 0;
    sLayer0.sizex = sLayer0.resx;
    sLayer0.sizey = sLayer0.resy;
    sLayer0.alpha = 0xff;
    sLayer0.flipx_en      = 0;
    sLayer0.flipy_en      = 0;
    sLayer0.baseaddr_virt = tsi_malloc(sLayer0.resy * sLayer0.stride);
    sLayer0.baseaddr_phys = (unsigned)(sLayer0.baseaddr_virt);
    memset(sLayer0.baseaddr_virt, 0, sLayer0.resy * sLayer0.stride);
    sFB320 = (volatile FB32Type_t*) sLayer0.baseaddr_virt;

    nemadc_layer_t sLayer1 = {0};
    sLayer1.resx = FB_RESX;
    sLayer1.resy = FB_RESY;
    sLayer1.buscfg = 0;
    sLayer1.format = NEMADC_RGBA8888;
    sLayer1.blendmode = (NEMADC_BF_SRCGBLALPHA | (NEMADC_BF_INVSRCGBLALPHA << 4));
    sLayer1.stride = sLayer1.resx * 4;
    sLayer1.startx = 0;
    sLayer1.starty = 0;
    sLayer1.sizex = sLayer1.resx;
    sLayer1.sizey = sLayer1.resy;
    sLayer1.alpha = 0x80;
    sLayer1.flipx_en      = 0;
    sLayer1.flipy_en      = 0;
    sLayer1.baseaddr_virt = tsi_malloc(sLayer1.resy * sLayer1.stride);
    sLayer1.baseaddr_phys = (unsigned)(sLayer1.baseaddr_virt);
    memset(sLayer1.baseaddr_virt, 0, sLayer1.resy * sLayer1.stride);
    sFB321 = (volatile FB32Type_t*) sLayer1.baseaddr_virt;

    nemadc_layer_t sLayer2 = {0};
    sLayer2.resx = FB_RESX;
    sLayer2.resy = FB_RESY;
    sLayer2.buscfg = 0;
    sLayer2.format = NEMADC_RGBA8888;
    sLayer2.blendmode = (NEMADC_BF_SRCGBLALPHA | (NEMADC_BF_INVSRCGBLALPHA << 4));
    sLayer2.stride = sLayer2.resx * 4;
    sLayer2.startx = 0;
    sLayer2.starty = 0;
    sLayer2.sizex = sLayer2.resx;
    sLayer2.sizey = sLayer2.resy;
    sLayer2.alpha = 0x80;
    sLayer2.flipx_en      = 0;
    sLayer2.flipy_en      = 0;
    sLayer2.baseaddr_virt = tsi_malloc(sLayer2.resy * sLayer2.stride);
    sLayer2.baseaddr_phys = (unsigned)(sLayer2.baseaddr_virt);
    memset(sLayer2.baseaddr_virt, 0, sLayer2.resy * sLayer2.stride);
    sFB322 = (volatile FB32Type_t*) sLayer2.baseaddr_virt;

    nemadc_layer_t sLayer3 = {0};
    sLayer3.resx = FB_RESX;
    sLayer3.resy = FB_RESY;
    sLayer3.buscfg = 0;
    sLayer3.format = NEMADC_RGBA8888;
    sLayer3.blendmode = (NEMADC_BF_SRCGBLALPHA | (NEMADC_BF_INVSRCGBLALPHA << 4));
    sLayer3.stride = sLayer3.resx * 4;
    sLayer3.startx = 0;
    sLayer3.starty = 0;
    sLayer3.sizex = sLayer3.resx;
    sLayer3.sizey = sLayer3.resy;
    sLayer3.alpha = 0x80;
    sLayer3.flipx_en      = 0;
    sLayer3.flipy_en      = 0;
    sLayer3.baseaddr_virt = tsi_malloc(sLayer3.resy * sLayer3.stride);
    sLayer3.baseaddr_phys = (unsigned)(sLayer3.baseaddr_virt);
    memset(sLayer3.baseaddr_virt, 0, sLayer3.resy * sLayer3.stride);
    sFB323 = (volatile FB32Type_t*) sLayer3.baseaddr_virt;

    for (uint16_t j = 0; j < (FB_RESY - 60); j++)
    {
        for (uint16_t i = 0; i < (FB_RESX - 60); i++)
        {
            //! RED
            sFB320->ui32Pixels[j +  0][i +  0] = 0xFF0000FF;
            //! GREEN
            sFB321->ui32Pixels[j + 20][i + 20] = 0xFF00FF00;
            //! BLUE
            sFB322->ui32Pixels[j + 40][i + 40] = 0xFFFF0000;
            //! YELLOW
            sFB323->ui32Pixels[j + 60][i + 60] = 0xFF00FFFF;
        }
    }
    nemadc_set_layer(0, &sLayer0);
    nemadc_set_layer(1, &sLayer1);
    nemadc_set_layer(2, &sLayer2);
    nemadc_set_layer(3, &sLayer3);

    nemadc_transfer_frame_prepare(false);
    //
    //It's necessary to launch frame manually when TE is disabled.
    //
    nemadc_transfer_frame_launch();
    nemadc_wait_vsync();

    nemadc_layer_disable(0);
    nemadc_layer_disable(1);
    nemadc_layer_disable(2);
    nemadc_layer_disable(3);

    tsi_free(sLayer0.baseaddr_virt);
    tsi_free(sLayer1.baseaddr_virt);
    tsi_free(sLayer2.baseaddr_virt);
    tsi_free(sLayer3.baseaddr_virt);

    return 0;
}

//*****************************************************************************
//
// Main Function
//
//*****************************************************************************

int
main(void)
{
    //
    // panel's maxmum resolution.
    //
    uint32_t ui32MipiCfg = MIPICFG_16RGB888_OPT0;    //!< default config
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
    // Global interrupt enable
    //
    am_hal_interrupt_master_enable();

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
    //
    // If you want to change display configurations in example, please change them here as below.
    //
    // g_sDispCfg.ui8NumLanes = 1;
    // g_sDispCfg.eDbiWidth = AM_HAL_DSI_DBI_WIDTH_8;
    // g_sDispCfg.eDsiFreq = AM_HAL_DSI_FREQ_TRIM_X12;

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
            sDisplayPanelConfig.ui32PixelFormat = ui32MipiCfg;
            nemadc_configure(&sDCConfig);
            am_devices_dc_dsi_raydium_init(((am_devices_dc_dsi_raydium_config_t *) &sDisplayPanelConfig));
            break;
        default:
            ; //NOP
    }

    test_nemadc_4layer();

    while (1)
    {
    }
}

//*****************************************************************************
//
// End Doxygen group.
//! @}
//
//*****************************************************************************

