//*****************************************************************************
//
//! @file nemadc_qspi_test.c
//!
//! @brief NemaDC QSPI Example.
//!
//! @addtogroup graphics_examples Graphics Examples
//!
//! @defgroup nemadc_qspi_test NemaDC QSPI Example
//! @ingroup graphics_examples
//! @{
//!
//! Purpose: This example demonstrates how to drive display panel with Quad-SPI interface.
//!
//! Quad-SPI interface includes 6 signals,
//!   * Chip select (CSX)
//!   * SPI clock (CLK)
//!   * Data interface 0 (DATA0)
//!   * Data interface 1 (DATA1).
//!   * Data interface 2 (DATA2).
//!   * Data interface 3 (DATA3).
//!
//! Quad-SPI adds two more I/O lines (DATA2 and DATA3) and sends 4 data bits per
//! clock cycle. During the write sequence the display controller writes one or
//! more bytes of information to the display module via the interface. The write
//! sequence is initiated when CSX is driven from high to low and ends when CSX
//! is pulled high. In this example, when send commands, SPI interface works at
//! SPI4 mode. When send frame data, SPI interface works at Qual-SPI mode. Panel
//! must be set to Quad-SPI mode through configuring related pins to correct H/L
//! level.
//!
//! When define TESTMODE_EN to 1 in nemadc_qspi_test.c, this example runs at test pattern mode.
//! When define TESTMODE_EN to 0, this example runs at image display mode.
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
#include "nemadc_qspi_test.h"
#include "oli_200x200_rgba.h"
#include "am_devices_dc_xspi_raydium.h"

#define TESTMODE_EN 1
#if TESTMODE_EN
    #define FB_RESX 360
    #define FB_RESY 360
#else
    #define FB_RESX 200
    #define FB_RESY 200
#endif

//*****************************************************************************
//
//! @brief Test QSPI interface
//!
//! @param i32PixelFormat Panel pixel format.
//!
//! @return None.
//
//*****************************************************************************
void
test_MIPI_qspi(int i32PixelFormat)
{
    //
    // panel's max resolution
    //
    uint16_t ui16MinX, ui16MinY;
    nemadc_initial_config_t sDCConfig;
    am_devices_dc_xspi_raydium_config_t sDisplayPanelConfig;

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

    am_devices_dc_xspi_raydium_hardware_reset();
    sDCConfig.eInterface = DISP_INTERFACE_QSPI;
    sDCConfig.ui32PixelFormat = i32PixelFormat;
    sDisplayPanelConfig.ui32PixelFormat = sDCConfig.ui32PixelFormat;
    nemadc_configure(&sDCConfig);
    am_devices_dc_xspi_raydium_init(&sDisplayPanelConfig);

#if TESTMODE_EN
    nemadc_layer_enable(0);
    uint32_t dbi_cfg = nemadc_reg_read(NEMADC_REG_DBIB_CFG);
    nemadc_MIPI_CFG_out(dbi_cfg | MIPICFG_SPI_HOLD);
    nemadc_MIPI_out(MIPI_DBIB_CMD | MIPI_MASK_QSPI | CMD1_DATA4);
    nemadc_MIPI_out(MIPI_DBIB_CMD | MIPI_MASK_QSPI | MIPI_CMD24 |
                    (MIPI_write_memory_start << CMD_OFFSET));
    nemadc_reg_write(NEMADC_REG_INTERRUPT, 1 << 4); //!< Enable frame end interrupt
    nemadc_set_mode(NEMADC_ONE_FRAME | NEMADC_TESTMODE);
    //
    // Wait for transfer to be completed
    //
    nemadc_wait_vsync();
    nemadc_MIPI_CFG_out(dbi_cfg);
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
    memcpy((char*)sLayer0.baseaddr_virt, ui8Oli200x200RGBA, sizeof(ui8Oli200x200RGBA));
    //
    // Program NemaDC Layer0.This function includes layer enable.
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

int
nemadc_qspi_test()
{
    unsigned int ui32Config = nemadc_get_config();

    if (ui32Config & NEMADC_CFG_DBIB)
    {
        test_MIPI_qspi(MIPICFG_4RGB565_OPT0);
        am_util_delay_ms(1000);
        test_MIPI_qspi(MIPICFG_4RGB666_OPT0);
        am_util_delay_ms(1000);
        test_MIPI_qspi(MIPICFG_4RGB888_OPT0);
// #### INTERNAL BEGIN ####
//        am_util_delay_ms(1000);
        //
        // fail
        //
//        test_MIPI_qspi(MIPICFG_4RGB444_OPT0);
//        am_util_delay_ms(1000);
        //
        // not implemented
        //
//        test_MIPI_qspi(MIPICFG_4RGB332_OPT0);
//        am_util_delay_ms(1000);
        //
        // not implemented
        //
//        test_MIPI_qspi(MIPICFG_4RGB111_OPT0);
// #### INTERNAL END ####
    }

    return 1;
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
    // If you want to change display configurations in example, please change them here as below.
    //
    g_sDispCfg.eInterface = DISP_IF_QSPI;

    am_bsp_disp_pins_enable();

    if (g_sDispCfg.eInterface != DISP_IF_QSPI)
    {
        return 0;   //!< If the interface in BSP isn't set to QSPI, then return.
    }

    am_hal_clkgen_control(AM_HAL_CLKGEN_CONTROL_DISPCLKSEL_HFRC48, NULL);
    am_hal_clkgen_control(AM_HAL_CLKGEN_CONTROL_DCCLK_ENABLE, NULL);

    am_hal_pwrctrl_periph_enable(AM_HAL_PWRCTRL_PERIPH_GFX);
    am_hal_pwrctrl_periph_enable(AM_HAL_PWRCTRL_PERIPH_DISP);

    nema_sys_init();
    //
    // Initialize NemaDC
    //
    if (nemadc_init() != 0)
    {
        return -2;
    }

    nemadc_qspi_test();

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
