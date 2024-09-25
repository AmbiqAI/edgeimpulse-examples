//*****************************************************************************
//
//! @file nemadc_dbi_configurations.c
//!
//! @brief NemaDC example.
//!
//! This example demonstrates how to drive the DBI-B(8080) interface panel.
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

#include "nemadc_dbi_configurations.h"
#include "am_devices_dc_dbi_novatek.h"
#include "Scotty_rgba565.h"

//*****************************************************************************
//
// Main Function
//
//*****************************************************************************
int
main(void)
{
    uint32_t ui32ColorFormat;
    nemadc_initial_config_t sDCConfig;
    am_devices_dc_config_t sDisplayPanelConfig;
    //
    // External power on
    //
    am_bsp_external_pwr_on();
    am_util_delay_ms(100);
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
    am_util_stdio_printf("nemadc_dbi_configurations example.\n");

    g_sDispCfg.eInterface = DISP_IF_DBI;

    am_bsp_disp_pins_enable();

    am_hal_clkgen_control(AM_HAL_CLKGEN_CONTROL_DISPCLKSEL_HFRC48, NULL);
    am_hal_clkgen_control(AM_HAL_CLKGEN_CONTROL_DCCLK_ENABLE, NULL);

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

    ui32ColorFormat = MIPICFG_16RGB565_OPT0;    //MIPICFG_16RGB888_OPT0

    am_devices_dc_dbi_novatek_hardware_reset();
    //
    // Set color coding
    //
    sDCConfig.ui32PixelFormat = ui32ColorFormat;
    am_devices_dc_dbi_set_parameters(&sDisplayPanelConfig, &sDCConfig);

    //
    // Initialize DC driver panel with DBI interface
    //
    nemadc_configure(&sDCConfig);

    //
    // Initialize the panel
    //
    am_devices_dc_dbi_novatek_init(&sDisplayPanelConfig);

    //
    // get color format
    //
    uint32_t ui32CF = ui32ColorFormat & 0x7;
    nemadc_layer_t sLayer0 = {0};

    if (ui32CF == MIPI_DCS_RGB332)
    {
        sLayer0.format = NEMADC_RGB332;
    }
    else if (ui32CF == MIPI_DCS_RGB444)
    {
        sLayer0.format = NEMADC_RGBA4444;
    }
    else if (ui32CF == MIPI_DCS_RGB565)
    {
        sLayer0.format = NEMADC_RGB565;
    }
    else if (ui32CF == MIPI_DCS_RGB888)
    {
        sLayer0.format = NEMADC_RGB24;
    }

    sLayer0.resx      = sDCConfig.ui16ResX;
    sLayer0.resy      = sDCConfig.ui16ResY;
    sLayer0.buscfg    = 0;
    sLayer0.blendmode = NEMADC_BL_SRC;
    sLayer0.stride    = nemadc_stride_size(sLayer0.format, sLayer0.resx);
    sLayer0.startx    = 0;
    sLayer0.starty    = 0;
    sLayer0.alpha     = 0xff;
    sLayer0.flipx_en  = 0;
    sLayer0.flipy_en  = 0;
    sLayer0.baseaddr_virt = tsi_malloc(sLayer0.resy*sLayer0.stride);
    sLayer0.baseaddr_phys = (unsigned)(sLayer0.baseaddr_virt);

    nema_memcpy((char*)sLayer0.baseaddr_virt, Scotty_rgba565, sizeof(Scotty_rgba565));

    nemadc_set_layer(0, &sLayer0);

    nemadc_transfer_frame_prepare(false);

    //
    //It's necessary to launch frame manually when TE is disabled.
    //
    nemadc_transfer_frame_launch();

    nemadc_wait_vsync();

    nemadc_layer_disable(0);
    tsi_free(sLayer0.baseaddr_virt);

    while (1)
    {
    }
}

