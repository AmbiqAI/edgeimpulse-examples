//*****************************************************************************
//
//! @file nemadc_xspi_configurations.c
//!
//! @brief NemaDC SPI4,DSPI and QSPI clock polarity/phase configurations example.
//!
//! This example demonstrates how to drive a SPI4,DSPI,QSPI panel.
//!
//! 4-wire SPI includes 4 signals,
//!   * Chip select (CSX)
//!   * SPI clock (CLK)
//!   * SPI bidirectional data interface (DATA)
//!   * Data and command switch (DCX).
//!
//! 1P1T 2-wire Dual-SPI interface includes 4 signals,
//!   * Chip select (CSX)
//!   * SPI clock (CLK)
//!   * Data interface 0 (DATA0)
//!   * Data interface 1 (DATA1).
//!
//! Quad-SPI interface includes 6 signals,
//!   * Chip select (CSX)
//!   * SPI clock (CLK)
//!   * Data interface 0 (DATA0)
//!   * Data interface 1 (DATA1).
//!   * Data interface 2 (DATA2).
//!   * Data interface 3 (DATA3).
//!
//! When define TESTMODE_EN to 1, this example runs at test pattern mode.
//! When define TESTMODE_EN to 0, this example runs at image display mode.
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

#include "nemadc_xspi_configurations.h"
#include "am_devices_dc_xspi_raydium.h"
#include "oli_200x200_rgba.h"

#define TESTMODE_EN 1

//
// It has the restriction that the layer size is a multiple of 4 for Apollo5.
//
#if TESTMODE_EN
    #define FB_RESX 400
    #define FB_RESY 400
#else
    #define FB_RESX 200
    #define FB_RESY 200
#endif


static void
configure(nemadc_initial_config_t *psDCConfig, uint32_t ui32Mode)
{
    uint32_t cfg = 0;

    cfg = ui32Mode | MIPICFG_SPI_CSX_V | MIPICFG_DBI_EN | MIPICFG_RESX | psDCConfig->ui32PixelFormat;
    if ( psDCConfig->bTEEnable )
    {
        cfg |= MIPICFG_DIS_TE;
    }
    //
    // disable clock gates, Rev A0 Work around
    //
    nemadc_reg_write(NEMADC_REG_CLKCTRL_CG, NemaDC_clkctrl_cg_clk_en);

    nemadc_MIPI_CFG_out(cfg);

    // Program NemaDC to transfer a resx*resy region
    nemadc_timing(psDCConfig->ui16ResX, 1, 1, 1,
                  psDCConfig->ui16ResY, 1, 1, 1);
}

//*****************************************************************************
//
//! @brief Test SPI4,DSPI,QSPI interface with CLOCK polarity/phase and pixel format
//!
//! @param ui32SPIMode      SPI mode(SPI4,DSPI,QSPI and Clock polarity/phase).
//! @param i32PixelFormat   Panel pixel format.
//!
//! @return None.
//
//*****************************************************************************
void
test_MIPI_SPI(uint32_t ui32SPIMode, int32_t i32PixelFormat)
{
    uint16_t ui16MinX, ui16MinY;
    nemadc_initial_config_t sDCConfig;
    am_devices_dc_xspi_raydium_config_t sDisplayPanelConfig;

    am_bsp_disp_pins_enable();
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

    //
    // Initialize the display
    //
    am_devices_dc_xspi_raydium_hardware_reset();
    sDCConfig.ui32PixelFormat = i32PixelFormat;
    sDisplayPanelConfig.ui32PixelFormat = sDCConfig.ui32PixelFormat;
    switch (g_sDispCfg.eInterface)
    {
        case DISP_IF_SPI4:
            sDCConfig.eInterface = DISP_INTERFACE_SPI4;
            break;
        case DISP_IF_DSPI:
            sDCConfig.eInterface = DISP_INTERFACE_DSPI;
            break;
        case DISP_IF_QSPI:
            sDCConfig.eInterface = DISP_INTERFACE_QSPI;
            break;
        default:
            ; //NOP
    }
    configure(&sDCConfig, ui32SPIMode);
    am_devices_dc_xspi_raydium_init(&sDisplayPanelConfig);

#if TESTMODE_EN
    nemadc_layer_enable(0);
    if ( 0!=(ui32SPIMode & MIPICFG_QSPI) )
    {
        uint32_t dbi_cfg = nemadc_reg_read(NEMADC_REG_DBIB_CFG);
        nemadc_MIPI_CFG_out(dbi_cfg | MIPICFG_SPI_HOLD);
        nemadc_MIPI_out(MIPI_DBIB_CMD | MIPI_MASK_QSPI | CMD1_DATA4);
        nemadc_MIPI_out(MIPI_DBIB_CMD | MIPI_MASK_QSPI | MIPI_CMD24 |
                        (MIPI_write_memory_start << CMD_OFFSET));
        //
        // Enable frame end interrupt
        //
        nemadc_reg_write(NEMADC_REG_INTERRUPT, 1 << 4);
        //
        // Send One Frame
        //
        nemadc_set_mode(NEMADC_ONE_FRAME | NEMADC_TESTMODE);
        //
        // Wait for transfer to be completed
        //
        nemadc_wait_vsync();

        nemadc_MIPI_CFG_out(dbi_cfg);
    }
    else if ( 0!=(ui32SPIMode & MIPICFG_DSPI) )
    {
        uint32_t dbi_cfg = nemadc_reg_read(NEMADC_REG_DBIB_CFG);
        nemadc_MIPI_CFG_out(dbi_cfg & (~MIPICFG_DSPI) & (~MIPICFG_QSPI));
        //
        // Start MIPI Panel Memory Write
        //
        nemadc_MIPI_out(MIPI_DBIB_CMD | MIPI_write_memory_start);
        nemadc_MIPI_CFG_out(((dbi_cfg & (~MIPICFG_SPI4)) | MIPICFG_SPI3)
                            | MIPICFG_SPIDC_DQSPI | MIPICFG_SPI_HOLD);
        //
        // Enable frame end interrupt
        //
        nemadc_reg_write(NEMADC_REG_INTERRUPT, 1 << 4);
        //
        // Send One Frame
        //
        nemadc_set_mode(NEMADC_ONE_FRAME | NEMADC_TESTMODE);
        //
        // Wait for transfer to be completed
        //
        nemadc_wait_vsync();

        nemadc_MIPI_CFG_out(dbi_cfg);
    }
    else
    {
        //
        // Start MIPI Panel Memory Write
        //
        nemadc_MIPI_out(MIPI_DBIB_CMD | MIPI_write_memory_start);
        //
        // Enable frame end interrupt
        //
        nemadc_reg_write(NEMADC_REG_INTERRUPT, 1 << 4);
        //
        // Send One Frame
        //
        nemadc_set_mode(NEMADC_ONE_FRAME | NEMADC_TESTMODE);
        //
        // Wait for transfer to be completed
        //
        nemadc_wait_vsync();
    }

#else
    //
    // send layer 0 to display via NemaDC
    //
    nemadc_layer_t sLayer0 = {0};
    sLayer0.resx          = FB_RESX;
    sLayer0.resy          = FB_RESY;
    sLayer0.buscfg        = 0;
    sLayer0.format        = NEMADC_RGBA8888;
    sLayer0.blendmode     = NEMADC_BL_SRC;
    sLayer0.stride        = sLayer0.resx * 4;
    sLayer0.startx        = 0;
    sLayer0.starty        = 0;
    sLayer0.sizex         = sLayer0.resx;
    sLayer0.sizey         = sLayer0.resy;
    sLayer0.alpha         = 0xff;
    sLayer0.flipx_en      = 0;
    sLayer0.flipy_en      = 0;
    sLayer0.baseaddr_virt = tsi_malloc(sLayer0.resy * sLayer0.stride);
    sLayer0.baseaddr_phys = (unsigned)(sLayer0.baseaddr_virt);

    nema_memcpy((char*)sLayer0.baseaddr_virt, ui8Oli200x200RGBA, sizeof(ui8Oli200x200RGBA));

    //
    // Program NemaDC Layer0,this function includes layer enable.
    //
    nemadc_set_layer(0, &sLayer0);

    nemadc_transfer_frame_prepare(false);
    //
    //It's necessary to launch frame manually when TE is disabled.
    //
    nemadc_transfer_frame_launch();
    nemadc_wait_vsync();

    tsi_free(sLayer0.baseaddr_virt);
#endif
}

//*****************************************************************************
//
//! @brief demonstrate SPI4/DSPI/QSPI interfaces
//!
//! @note Please make sure the panel could support these interfaces.
//!
//! @return status.
//
//*****************************************************************************
static int
nemadc_xspi_configurations(void)
{
    nema_sys_init();
    //
    // Initialize NemaDC
    //
    if (nemadc_init() != 0)
    {
        return -2;
    }
    //
    // CO5300 couldn't support SPI4 interface.
    //
    if (g_sDispCfg.eIC != DISP_IC_CO5300)
    {
        //
        // demonstrate SPI4 configuration.
        //
        g_sDispCfg.eInterface = DISP_IF_SPI4;

        //
        // SPI4 driving display with clock polarity = 0&phase = 0(default)
        //
        test_MIPI_SPI(MIPICFG_SPI4, MIPICFG_1RGB888_OPT0);
        am_util_delay_ms(1000);
        test_MIPI_SPI(MIPICFG_SPI4, MIPICFG_1RGB666_OPT0);
        am_util_delay_ms(1000);
        //
        // SPI4 driving display with clock polarity = 1&phase = 1
        //
        test_MIPI_SPI(MIPICFG_SPI4 | MIPICFG_SPI_CPOL | MIPICFG_SPI_CPHA,
                    MIPICFG_1RGB565_OPT0);
        am_util_delay_ms(1000);
        test_MIPI_SPI(MIPICFG_SPI4 | MIPICFG_SPI_CPOL | MIPICFG_SPI_CPHA,
                    MIPICFG_1RGB332_OPT0);
        am_util_delay_ms(1000);
    }

    //
    // demonstrate DSPI configuration.
    //
    g_sDispCfg.eInterface = DISP_IF_DSPI;
    //
    // DSPI driving display with clock polarity = 0&phase = 0(default)
    //
    test_MIPI_SPI(MIPICFG_DSPI | MIPICFG_SPI4, MIPICFG_2RGB888_OPT0);
    am_util_delay_ms(1000);
    //
    // DSPI driving display with clock polarity = 1&phase = 1
    //
    test_MIPI_SPI(MIPICFG_DSPI | MIPICFG_SPI4 | MIPICFG_SPI_CPOL | MIPICFG_SPI_CPHA,
                  MIPICFG_2RGB666_OPT0);
    am_util_delay_ms(1000);
    test_MIPI_SPI(MIPICFG_DSPI | MIPICFG_SPI4 | MIPICFG_SPI_CPOL | MIPICFG_SPI_CPHA,
                  MIPICFG_2RGB565_OPT0);
    am_util_delay_ms(1000);

    //
    // demonstrate QSPI configuration.
    //
    g_sDispCfg.eInterface = DISP_IF_QSPI;
    //
    // QSPI driving display with clock polarity = 0&phase = 0(default)
    //
    test_MIPI_SPI(MIPICFG_QSPI | MIPICFG_SPI4, MIPICFG_4RGB565_OPT0);
    am_util_delay_ms(1000);
    //
    // QSPI driving display with clock polarity = 1&phase = 1
    //
    test_MIPI_SPI(MIPICFG_QSPI | MIPICFG_SPI4 | MIPICFG_SPI_CPOL | MIPICFG_SPI_CPHA,
                    MIPICFG_4RGB666_OPT0);
    am_util_delay_ms(1000);
    test_MIPI_SPI(MIPICFG_QSPI | MIPICFG_SPI4 | MIPICFG_SPI_CPOL | MIPICFG_SPI_CPHA,
                    MIPICFG_4RGB888_OPT0);

    return 0;
}

int
main(void)
{
    //
    // External power on
    //
    am_bsp_external_pwr_on();
    am_util_delay_ms(100);
    am_bsp_low_power_init();
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
    am_util_stdio_printf("nemadc_xspi_configurations example.\n");

    am_hal_clkgen_control(AM_HAL_CLKGEN_CONTROL_DISPCLKSEL_HFRC96, NULL);
    am_hal_clkgen_control(AM_HAL_CLKGEN_CONTROL_DCCLK_ENABLE, NULL);

    am_hal_pwrctrl_periph_enable(AM_HAL_PWRCTRL_PERIPH_GFX);
    am_hal_pwrctrl_periph_enable(AM_HAL_PWRCTRL_PERIPH_DISP);

    nemadc_xspi_configurations();

    while (1)
    {
    }
}