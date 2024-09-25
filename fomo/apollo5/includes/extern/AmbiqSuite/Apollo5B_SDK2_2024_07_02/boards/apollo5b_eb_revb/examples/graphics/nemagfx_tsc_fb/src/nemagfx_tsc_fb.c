//*****************************************************************************
//
//! @file nemagfx_tsc_fb.c
//!
//! @brief NemaGFX example.
//! Nemagfx_tsc_fb is a demo of TSC formats frame-buffer compression. It will
//! significantly save memories.
//! Note:  the width and height of the frame-buffer should be 4-pixels aligned
//!
//! AM_DEBUG_PRINTF
//! If enabled, debug messages will be sent over ITM.
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

//*****************************************************************************
//
// This application has a large number of common include files. For
// convenience, we'll collect them all together in a single header and include
// that everywhere.
//
//*****************************************************************************
#include "nemagfx_tsc_fb.h"
#include "oli_200x200_tsc4.h"
#include "oli_200x200_tsc6.h"
#include "oli_200x200_tsc6a.h"
#include "oli_200x200_tsc12.h"
#include "oli_200x200_tsc12a.h"

#define RESX 200
#define RESY 200

#if (DISP_CTRL_IP == DISP_CTRL_IP_MSPI)
#error "MSPI driving display could not support TSC format."
#endif

//
// Define GPU/DC used structures with default parameters.
//
img_obj_t g_sFB = {{0}, RESX, RESY, -1, 0, NEMA_TSC4, 0};
img_obj_t g_sSrc = {{0}, RESX, RESY, -1, 0, NEMA_TSC4, 0};
nemadc_layer_t g_sDCLayer = {(void *)0, 0, RESX, RESY, -1, 0, 0, RESX, RESY, 0xff, NEMADC_BL_SRC, 0, NEMADC_TSC4, 0, 0, 0, 0, 0, 0, 0, 0};

//*****************************************************************************
//
//! @brief Initialize destination and source buffers
//!
//! @param ui8DesColorFormat     - destination color format
//! @param ui8SrcColorFormat     - source color format
//!
//! @note This function initialize both destination and source frame buffers.
//! please make sure the source format should be one of TSC formats.
//!
//! @return none.
//
//*****************************************************************************
void
load_objects(uint8_t ui8DesColorFormat, uint8_t ui8SrcColorFormat)
{
    //
    // Initialize Frame buffer.
    //
    g_sFB.format = ui8DesColorFormat;
    uint32_t size = nema_texture_size(g_sFB.format, 0, g_sFB.w, g_sFB.h);
    g_sFB.bo = nema_buffer_create(size);
    memset((void*)(g_sFB.bo.base_phys), 0, size);
    if ( g_sFB.format == NEMA_TSC4 )
    {
        g_sDCLayer.format = NEMADC_TSC4;
    }
    else if ( g_sFB.format == NEMA_TSC6 )
    {
        g_sDCLayer.format = NEMADC_TSC6;
    }
    else if ( g_sFB.format == NEMA_TSC6A )
    {
        g_sDCLayer.format = NEMADC_TSC6A;
    }
    else if ( g_sFB.format == NEMA_TSC12 )
    {
        g_sDCLayer.format = NEMADC_TSC12;
    }
    else if ( g_sFB.format == NEMA_TSC12A )
    {
        g_sDCLayer.format = NEMADC_TSC12A;
    }
    else
    {
        g_sFB.format = NEMA_RGB24;
        g_sDCLayer.format = NEMADC_RGB24;
    }

    g_sDCLayer.stride = nemadc_stride_size(g_sDCLayer.format, g_sDCLayer.resx);
    g_sDCLayer.baseaddr_phys = g_sFB.bo.base_phys;
    g_sDCLayer.baseaddr_virt = g_sFB.bo.base_virt;

    //
    // Initialize source buffer
    //
    g_sSrc.format = ui8SrcColorFormat;
    size = nema_texture_size(g_sSrc.format, 0, g_sSrc.w, g_sSrc.h);
    g_sSrc.bo = nema_buffer_create(size);

    if ( g_sSrc.format == NEMA_TSC4 )
    {
        nema_memcpy(g_sSrc.bo.base_virt, oli_200x200_tsc4, sizeof(oli_200x200_tsc4));
    }
    else if ( g_sSrc.format == NEMA_TSC6 )
    {
        nema_memcpy(g_sSrc.bo.base_virt, oli_200x200_tsc6, sizeof(oli_200x200_tsc6));
    }
    else if ( g_sSrc.format == NEMA_TSC6A )
    {
        nema_memcpy(g_sSrc.bo.base_virt, oli_200x200_tsc6a, sizeof(oli_200x200_tsc6a));
    }
    else if ( g_sSrc.format == NEMA_TSC12 )
    {
        nema_memcpy(g_sSrc.bo.base_virt, oli_200x200_tsc12, sizeof(oli_200x200_tsc12));
    }
    else if ( g_sSrc.format == NEMA_TSC12A )
    {
        nema_memcpy(g_sSrc.bo.base_virt, oli_200x200_tsc12a, sizeof(oli_200x200_tsc12a));
    }
    else
    {
        am_util_stdio_printf("Please select valid source color format!\n");
        while(1);
    }
}

//*****************************************************************************
//
//! @brief Demonstrate the expected destination color format with any TSC sources.
//!
//! @param ui8DesColorFormat     - destination color format
//! @param ui8SrcColorFormat     - source color format
//!
//! @note This function render the TSC color format source into the desired format.
//!
//! @return 0.
//
//*****************************************************************************
int32_t
demonstrate_tsc_format(uint8_t ui8DesColorFormat, uint8_t ui8SrcColorFormat)
{
    load_objects(ui8DesColorFormat, ui8SrcColorFormat);
    nema_cmdlist_t g_sCL;
    g_sCL = nema_cl_create_sized(0x1000);

    nema_cl_bind(&g_sCL);

    nema_bind_dst_tex(g_sFB.bo.base_phys, g_sFB.w, g_sFB.h, (nema_tex_format_t)(g_sFB.format), g_sFB.stride);
    nema_bind_src_tex(g_sSrc.bo.base_phys, g_sSrc.w, g_sSrc.h, (nema_tex_format_t)(g_sSrc.format), -1, NEMA_FILTER_BL);
    nema_set_clip(0, 0, RESX, RESY);

    nema_set_blend_blit(NEMA_BL_SRC);
    nema_blit(0, 0);

    nema_cl_unbind();
    nema_cl_submit(&g_sCL);
    nema_cl_wait(&g_sCL);
    nema_cl_rewind(&g_sCL);
    nemadc_set_layer(0, &g_sDCLayer);
    nemadc_transfer_frame_prepare(g_sDispCfg.eTEType != DISP_TE_DISABLE);
    if ( g_sDispCfg.eTEType == DISP_TE_DISABLE )
    {
        //
        //It's necessary to launch frame manually when TE is disabled.
        //
        nemadc_transfer_frame_launch();
    }
    nemadc_wait_vsync();

    nema_cl_destroy(&g_sCL);
    nema_buffer_destroy(&g_sSrc.bo);
    nema_buffer_destroy(&g_sFB.bo);
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
    // Print a banner.
    //
    am_util_stdio_printf("nemadc_tsc_fb example.\n");

#ifdef AM_DEBUG_PRINTF
    am_bsp_debug_printf_enable();
#endif

    //
    // Set the display region to center
    //
    if (RESX < g_sDispCfg.ui16ResX)
    {
        sDisplayPanelConfig.ui16ResX = RESX;
    }
    else
    {
        sDisplayPanelConfig.ui16ResX = g_sDispCfg.ui16ResX;
    }
    ui16MinX = (g_sDispCfg.ui16ResX - sDisplayPanelConfig.ui16ResX) >> 1;
    ui16MinX = (ui16MinX >> 1) << 1;

    if (RESY < g_sDispCfg.ui16ResY)
    {
        sDisplayPanelConfig.ui16ResY = RESY;
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

    //
    // If you want to change display configurations in example, please change them here as below.
    //
    // g_sDispCfg.ui8NumLanes = 1;
    // g_sDispCfg.eDbiWidth = AM_HAL_DSI_DBI_WIDTH_8;
    // g_sDispCfg.eDsiFreq = AM_HAL_DSI_FREQ_TRIM_X12;
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
            sDisplayPanelConfig.ui32PixelFormat = ui32MipiCfg;
            nemadc_configure(&sDCConfig);
            am_devices_dc_dsi_raydium_init(((am_devices_dc_dsi_raydium_config_t *) &sDisplayPanelConfig));
            break;
        default:
            ; //NOP
    }

    //
    // Global interrupt enable
    //
    am_hal_interrupt_master_enable();

    //
    // Initialize NemaGFX
    //
    nema_init();

    //
    // Demonstrate TSC formats
    //
    demonstrate_tsc_format(NEMA_TSC4, NEMA_TSC4);
    am_util_delay_ms(1000);

    demonstrate_tsc_format(NEMA_TSC6, NEMA_TSC6);
    am_util_delay_ms(1000);

    demonstrate_tsc_format(NEMA_TSC6A, NEMA_TSC6A);
    am_util_delay_ms(1000);

#ifndef AM_PART_APOLLO5A
    //
    // Apollo5b and later version can support TSC12 formats
    //
    demonstrate_tsc_format(NEMA_TSC12, NEMA_TSC12);
    am_util_delay_ms(1000);

    demonstrate_tsc_format(NEMA_TSC12A, NEMA_TSC12A);
    am_util_delay_ms(1000);
#endif
    //
    // We shouldn't ever get here.
    //
    while (1)
    {
    }

}

