//*****************************************************************************
//
//! @file nemadc_4layers.c
//!
//! @brief NemaDC example.
//!
//! This example demonstrates DC 4 layers overlay with global alpha blending.
//!   * Layer 0 with red color,it format is RGBA8888.
//!   * Layer 1 with green color,it format is RGB24.
//!   * Layer 2 with blue color,it format is RGB565.
//!   * Layer 3 with pure Alpha,it format is A8.
//! Global alpha value will be valid when color feature NEMADC_FORCE_A or
//! NEMADC_MODULATE_A is enabled.
//!
//! For more information, please refer to NemaDC user manual.
//
//*****************************************************************************

//*****************************************************************************
//
// Copyright (c) 2024, Ambiq Micro, Inc.
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// 1. Redistributions of source code must retain the above copyright notice,
// this list of conditions and the following disclaimer.
//
// 2. Redistributions in binary form must reproduce the above copyright
// notice, this list of conditions and the following disclaimer in the
// documentation and/or other materials provided with the distribution.
//
// 3. Neither the name of the copyright holder nor the names of its
// contributors may be used to endorse or promote products derived from this
// software without specific prior written permission.
//
// Third party software included in this distribution is subject to the
// additional license terms as defined in the /docs/licenses directory.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
// ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
// LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
// CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
// SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
// INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
// CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
// ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
// POSSIBILITY OF SUCH DAMAGE.
//
// This is part of revision release_a5b_sdk2-748191cd0 of the AmbiqSuite Development Package.
//
//*****************************************************************************
#include "nemadc_4layers.h"

//
// It has the restriction that the layer size is a multiple of 4 for Apollo5.
//
#define FB_RESX         160
#define FB_RESY         160

#define LAYERS_NUMBER   4       // up to 4 layers
#define LAYERS_OFFSET   20      // offset of nearest two layers

//
// Display size
//
#define DISPLAY_SIZEX   (FB_RESX + (LAYERS_NUMBER - 1) * LAYERS_OFFSET)
#define DISPLAY_SIZEY   (FB_RESY + (LAYERS_NUMBER - 1) * LAYERS_OFFSET)

//*****************************************************************************
//
//! @brief test Display Controller's four layers.
//!
//! This function demonstrates DC's four layers blending with respective color formats.
//!
//! @return None.
//
//*****************************************************************************
void
test_nemadc_4layers(void)
{
    nemadc_layer_t sLayer0 = {0};
    sLayer0.resx = FB_RESX;
    sLayer0.resy = FB_RESY;
    sLayer0.buscfg = 0;
    sLayer0.format = NEMADC_RGBA8888;
    sLayer0.blendmode = NEMADC_BL_SRC;
    sLayer0.stride = nemadc_stride_size(sLayer0.format, sLayer0.resx);
    sLayer0.startx = 0;
    sLayer0.starty = 0;
    sLayer0.sizex = sLayer0.resx;
    sLayer0.sizey = sLayer0.resy;
    sLayer0.alpha = 0xFF;                               // This value is invalid if current layer doesn't enable feature NEMADC_FORCE_A or NEMADC_MODULATE_A
    sLayer0.flipx_en = 0;
    sLayer0.flipy_en = 0;
    sLayer0.baseaddr_virt = tsi_malloc(sLayer0.resy * sLayer0.stride);
    sLayer0.baseaddr_phys = (unsigned)(sLayer0.baseaddr_virt);

    nemadc_layer_t sLayer1 = {0};
    sLayer1.resx = FB_RESX;
    sLayer1.resy = FB_RESY;
    sLayer1.buscfg = 0;
    sLayer1.format = NEMADC_RGB24;
    sLayer1.blendmode = NEMADC_BL_SIMPLE;
    sLayer1.stride = nemadc_stride_size(sLayer1.format, sLayer1.resx);
    sLayer1.startx = LAYERS_OFFSET;
    sLayer1.starty = LAYERS_OFFSET;
    sLayer1.sizex = sLayer1.resx;
    sLayer1.sizey = sLayer1.resy;
    sLayer1.alpha = 0x80;                               // Global alpha value(0x00~0xff)
    sLayer1.flipx_en = 0;
    sLayer1.flipy_en = 0;
    sLayer1.baseaddr_virt = tsi_malloc(sLayer1.resy * sLayer1.stride);
    sLayer1.baseaddr_phys = (unsigned)(sLayer1.baseaddr_virt);

    nemadc_layer_t sLayer2 = {0};
    sLayer2.resx = FB_RESX;
    sLayer2.resy = FB_RESY;
    sLayer2.buscfg = 0;
    sLayer2.format = NEMADC_RGB565;
    sLayer2.blendmode = NEMADC_BL_SIMPLE;
    sLayer2.stride = nemadc_stride_size(sLayer2.format, sLayer2.resx);
    sLayer2.startx = 2 * LAYERS_OFFSET;
    sLayer2.starty = 2 * LAYERS_OFFSET;
    sLayer2.sizex = sLayer2.resx;
    sLayer2.sizey = sLayer2.resy;
    sLayer2.alpha = 0x80;                               // Global alpha value(0x00~0xff)
    sLayer2.flipx_en = 0;
    sLayer2.flipy_en = 0;
    sLayer2.baseaddr_virt = tsi_malloc(sLayer2.resy * sLayer2.stride);
    sLayer2.baseaddr_phys = (unsigned)(sLayer2.baseaddr_virt);

    nemadc_layer_t sLayer3 = {0};
    sLayer3.resx = FB_RESX;
    sLayer3.resy = FB_RESY;
    sLayer3.buscfg = 0;
    sLayer3.format = NEMADC_A8;
    sLayer3.blendmode = NEMADC_BL_SIMPLE;
    sLayer3.stride = nemadc_stride_size(sLayer3.format, sLayer3.resx);
    sLayer3.startx = 3 * LAYERS_OFFSET;
    sLayer3.starty = 3 * LAYERS_OFFSET;
    sLayer3.sizex = sLayer3.resx;
    sLayer3.sizey = sLayer3.resy;
    sLayer3.alpha = 0x80;                               // Global alpha value(0x00~0xff)
    sLayer3.flipx_en = 0;
    sLayer3.flipy_en = 0;
    sLayer3.baseaddr_virt = tsi_malloc(sLayer3.resy * sLayer3.stride);
    sLayer3.baseaddr_phys = (unsigned)(sLayer3.baseaddr_virt);

    //
    // Fill color formats for per layers.
    //
    uint8_t *pLayers[4];

    pLayers[0] = sLayer0.baseaddr_virt;
    pLayers[1] = sLayer1.baseaddr_virt;
    pLayers[2] = sLayer2.baseaddr_virt;
    pLayers[3] = sLayer3.baseaddr_virt;

    for (uint32_t ui32Idx = 0; ui32Idx < FB_RESX*FB_RESY; ui32Idx++)
    {
        //
        // Fill layer 0 with red color,its color format is RGBA8888.
        //
        pLayers[0][ui32Idx * 4 + 0] = 0xFF;     //Red 8 bits
        pLayers[0][ui32Idx * 4 + 1] = 0x00;     //Green 8 bits
        pLayers[0][ui32Idx * 4 + 2] = 0x00;     //Blue 8 bits
        pLayers[0][ui32Idx * 4 + 3] = 0xFF;     //Alpha 8 bits

        //
        // Fill layer 1 with Green color,its color format is RGB24.
        //
        pLayers[1][ui32Idx * 3 + 0] = 0x00;     //Red 8 bits
        pLayers[1][ui32Idx * 3 + 1] = 0xFF;     //Green 8 bits
        pLayers[1][ui32Idx * 3 + 2] = 0x00;     //Blue 8 bits

        //
        // Fill layer 2 with Blue color,its color format is RGB565.
        //
        pLayers[2][ui32Idx * 2 + 0] = 0x1F;     //Green 3 bits and blue 5 bits
        pLayers[2][ui32Idx * 2 + 1] = 0x00;     //Red 5 bits and green 3 bits

        //
        // Fill layer 3 with only Alpha,its color format is A8.
        //
        pLayers[3][ui32Idx + 0] = 0xFF;         //Alpha 8 bits
    }

    //
    // Please flush the D-cache if CPU has directly written frame buffers.
    //
    nema_buffer_t bo;
    bo.size = sLayer0.resy * sLayer0.stride;
    bo.base_phys = sLayer0.baseaddr_phys;
    nema_buffer_flush(&bo);

    bo.size = sLayer1.resy * sLayer1.stride;
    bo.base_phys = sLayer1.baseaddr_phys;
    nema_buffer_flush(&bo);

    bo.size = sLayer2.resy * sLayer2.stride;
    bo.base_phys = sLayer2.baseaddr_phys;
    nema_buffer_flush(&bo);

    bo.size = sLayer3.resy * sLayer3.stride;
    bo.base_phys = sLayer3.baseaddr_phys;
    nema_buffer_flush(&bo);

    nemadc_set_layer(0, &sLayer0);
    //
    // Force the global alpha(sLayer1.alpha) as the first layer alpha, if it has no alpha added for it.
    //
    nemadc_set_layer(1, &sLayer1);
    nemadc_reg_write(NEMADC_REG_LAYER_MODE(1), nemadc_reg_read(NEMADC_REG_LAYER_MODE(1)) | NEMADC_FORCE_A);
    //
    // Force the global alpha(sLayer2.alpha) as the second layer alpha, if it has no alpha added for it.
    //
    nemadc_set_layer(2, &sLayer2);
    nemadc_reg_write(NEMADC_REG_LAYER_MODE(2), nemadc_reg_read(NEMADC_REG_LAYER_MODE(2)) | NEMADC_FORCE_A);
    //
    // Premultiply the global alpha(sLayer3.alpha) with the third layer alpha, if it has no alpha added 0xFF as the default alpha.
    //
    nemadc_set_layer(3, &sLayer3);
    nemadc_reg_write(NEMADC_REG_LAYER_MODE(3), nemadc_reg_read(NEMADC_REG_LAYER_MODE(3)) | NEMADC_MODULATE_A);

    nemadc_transfer_frame_prepare(g_sDispCfg.eTEType != DISP_TE_DISABLE);
    if (g_sDispCfg.eTEType == DISP_TE_DISABLE)
    {
        //
        //It's necessary to launch frame manually when TE is disabled.
        //
        nemadc_transfer_frame_launch();
    }
    nemadc_wait_vsync();

    nemadc_layer_disable(0);
    nemadc_layer_disable(1);
    nemadc_layer_disable(2);
    nemadc_layer_disable(3);

    tsi_free(sLayer0.baseaddr_virt);
    tsi_free(sLayer1.baseaddr_virt);
    tsi_free(sLayer2.baseaddr_virt);
    tsi_free(sLayer3.baseaddr_virt);

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

    //
    // Configure the board for low power operation.
    //
    am_bsp_low_power_init();

    //
    //  Enable the I-Cache and D-Cache.
    //
    am_hal_cachectrl_icache_enable();
    am_hal_cachectrl_dcache_enable(true);

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
    am_util_stdio_printf("nemadc_4layers example.\n");

    //
    // Set the display region to center
    //
    sDisplayPanelConfig.ui16ResX = DISPLAY_SIZEX;
    if (sDisplayPanelConfig.ui16ResX < g_sDispCfg.ui16ResX)
    {
        ui16MinX = (g_sDispCfg.ui16ResX - sDisplayPanelConfig.ui16ResX) >> 1;
        ui16MinX = (ui16MinX >> 1) << 1;
    }
    else
    {
        ui16MinX = 0;
    }

    sDisplayPanelConfig.ui16ResY = DISPLAY_SIZEY;
    if (sDisplayPanelConfig.ui16ResY < g_sDispCfg.ui16ResY)
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
            am_devices_dc_dsi_raydium_init(((am_devices_dc_dsi_raydium_config_t *) &sDisplayPanelConfig));
            break;
        default:
            break;
    }

    test_nemadc_4layers();

    while (1)
    {
    }
}

