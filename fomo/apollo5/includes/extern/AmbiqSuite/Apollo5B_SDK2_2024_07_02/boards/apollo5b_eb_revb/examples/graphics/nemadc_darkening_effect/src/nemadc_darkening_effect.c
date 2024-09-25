//*****************************************************************************
//
//! @file nemadc_darkening_effect.c
//!
//! @brief Darkening effect example.
//!
//! This example demonstrates how to implement pull-down menu and darkening effects
//! with the DC's SPI4/DSPI/QSPI/DSI interfaces which is determined by structure
//! variables g_sDispCfg.eInterface on BSP. modify g_sDispCfg.eInterface could change
//! interface.Please change interface before function am_bsp_disp_pins_enable() invoked,
//! if needed.
//!
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

#include "nemadc_darkening_effect.h"
#include "pinwheel_456x456_rgba565.h"
#include "umbrella_456x456_rgba565.h"
#include "nema_utils.h"

//
// It has the restriction that the layer size is a multiple of 4 for Apollo5.
//
#define FB_RESX     456
#define FB_RESY     456
#define MIN_STEP    4

//*****************************************************************************
//
//! @brief darkening effect.
//!
//! @return None.
//
//*****************************************************************************
void
nemadc_darkening_effect(void)
{
    int32_t i32Offset = 0;
    bool bSign = 0;
    uint8_t ui8Step = MIN_STEP;

    //
    // Layer 0 is background layer
    //
    nemadc_layer_t sLayer0 = {0};
    sLayer0.resx = FB_RESX;
    sLayer0.resy = FB_RESY;
    sLayer0.buscfg = 0;
    sLayer0.format = NEMADC_RGB565;
    sLayer0.stride = sLayer0.resx * 2;
    sLayer0.blendmode = NEMADC_BL_SRC;
    sLayer0.startx = 0;
    sLayer0.starty = 0;
    sLayer0.sizex = sLayer0.resx;
    sLayer0.sizey = sLayer0.resy;
    sLayer0.alpha = 0xff;
    sLayer0.flipx_en      = 0;
    sLayer0.flipy_en      = 0;
    sLayer0.baseaddr_virt = tsi_malloc(sLayer0.resy * sLayer0.stride);
    sLayer0.baseaddr_phys = (unsigned)(sLayer0.baseaddr_virt);
    nema_memcpy((char*)sLayer0.baseaddr_virt, pinwheel_456x456_rgba565, sizeof(pinwheel_456x456_rgba565));
    //
    // Layer 2 is the layer for pull-down menu
    //
    nemadc_layer_t sLayer2 = {0};
    sLayer2.resx = FB_RESX;
    sLayer2.resy = FB_RESY;
    sLayer2.buscfg = 0;
    sLayer2.format = NEMADC_RGB565;
    sLayer2.stride = sLayer2.resx * 2;
    sLayer2.blendmode = (NEMADC_BF_ONE | NEMADC_BF_ONE);
    sLayer2.startx = 0;
    sLayer2.starty = 0;
    sLayer2.sizex = sLayer2.resx;
    sLayer2.sizey = sLayer2.resy;
    sLayer2.alpha = 0xff;
    sLayer2.flipx_en      = 0;
    sLayer2.flipy_en      = 0;
    sLayer2.baseaddr_virt = tsi_malloc(sLayer2.resy * sLayer2.stride);
    sLayer2.baseaddr_phys = (unsigned)(sLayer2.baseaddr_virt);
    nema_memcpy((char*)sLayer2.baseaddr_virt, umbrella_456x456_rgba565, sizeof(umbrella_456x456_rgba565));

    //
    // Layer 1 is just for implementing darkening effect, we can assign any texture to this layer.
    //
    nemadc_layer_t sLayer1 = {0};
    sLayer1.resx = FB_RESX;
    sLayer1.resy = FB_RESY;
    sLayer1.buscfg = 0;
    sLayer1.format = NEMADC_RGB565;
    sLayer1.stride = sLayer1.resx * 2;
    sLayer1.blendmode = (NEMADC_BF_ZERO | (NEMADC_BF_GLBALPHA << 4));
    sLayer1.startx = 0;
    sLayer1.starty = 0;
    sLayer1.sizex = sLayer1.resx;
    sLayer1.sizey = sLayer1.resy;
    sLayer1.alpha = 0xff;
    sLayer1.flipx_en      = 0;
    sLayer1.flipy_en      = 0;
    sLayer1.baseaddr_virt = sLayer2.baseaddr_virt;
    sLayer1.baseaddr_phys = sLayer2.baseaddr_phys;
    //
    // Program NemaDC Layer0 and Layer1
    //
    while (1)
    {
        if (i32Offset == 0)
        {
            nemadc_set_layer(0, &sLayer0);
            nemadc_layer_disable(1);
            nemadc_layer_disable(2);
        }
        else if (i32Offset == FB_RESX)
        {
            sLayer2.startx = 0;
            nemadc_set_layer(2, &sLayer2);
            nemadc_layer_disable(0);
            nemadc_layer_disable(1);
        }
        else
        {
            sLayer2.startx = FB_RESX - i32Offset;
            sLayer1.alpha = 0xff - i32Offset * 0xff / FB_RESX;
            //
            // This function includes layer enable.
            //
            nemadc_set_layer(0, &sLayer0);
            //
            // This function includes layer enable.
            //
            nemadc_set_layer(1, &sLayer1);
            //
            // This function includes layer enable.
            //
            nemadc_set_layer(2, &sLayer2);
        }

        nemadc_transfer_frame_prepare(g_sDispCfg.eTEType != DISP_TE_DISABLE);
        if ( g_sDispCfg.eTEType == DISP_TE_DISABLE )
        {
            //
            //It's necessary to launch frame manually when TE is disabled.
            //
            nemadc_transfer_frame_launch();
        }
        nemadc_wait_vsync();

        i32Offset = bSign ? (i32Offset - ui8Step) : (i32Offset + ui8Step);

        if (i32Offset >= FB_RESX)
        {
            //! minus
            bSign = 1;
            i32Offset = FB_RESX;
        }

        if (i32Offset <= 0)
        {
            //! plus
            bSign = 0;
            i32Offset = 0;
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
    // panel's max resolution
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
    // Global interrupt enable
    //
    am_hal_interrupt_master_enable();

    //
    // Initialize the printf interface for ITM/SWO output.
    //
    am_bsp_itm_printf_enable();
    //
    // Clear the terminal and print the banner.
    //
    am_util_stdio_terminal_clear();

    //
    // Print a banner.
    //
    am_util_stdio_printf("nemadc_darking_effect example.\n");

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

    nemadc_darkening_effect();

    while (1)
    {
    }

}

