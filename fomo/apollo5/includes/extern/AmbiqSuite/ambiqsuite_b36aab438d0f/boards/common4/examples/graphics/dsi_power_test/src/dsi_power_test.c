//*****************************************************************************
//
//! @file dsi_power_test.c
//!
//! @brief DSI Power example.
//!
//! @addtogroup graphics_examples Graphics Examples
//!
//! @defgroup dsi_power_test DSI Power Example
//! @ingroup graphics_examples
//! @{
//!
//! Purpose: This example demonstrates DSI power saving sequence.
//!
//
//*****************************************************************************

//*****************************************************************************
//
// ${copyright}
//
// This is part of revision ${version} of the AmbiqSuite Development Package.
//
//*****************************************************************************

#include "dsi_power_test.h"
#include "oli_200x200_rgba.h"

#define FB_RESX 200
#define FB_RESY 200

// #define ULPS_POWER_OPTIM  //!< ULPS mode with all configurations for power optimization.
#define ULPS_DSI_PD //!< ULPS mode with all configurations for power optimization and with DISPPHY powered down.

#ifndef BEFORE_ENTER_ULPS
    #define BEFORE_ENTER_ULPS 0
#endif

#ifndef AFTER_ENTER_ULPS
    #define AFTER_ENTER_ULPS 0
#endif

#ifndef AFTER_EXIT_ULPS
    #define AFTER_EXIT_ULPS 0
#endif

//*****************************************************************************
//
//! @brief Test DBI2DSI interface.
//! @param ui32PixelFormat - Panel pixel format
//! @return None.
//
//*****************************************************************************
void
test_MIPI_DSI(uint32_t ui32PixelFormat)
{
    //
    // panel's max resolution
    //
    uint16_t ui16MinX, ui16MinY;
    nemadc_initial_config_t sDCConfig;
    am_devices_dc_dsi_raydium_config_t sDisplayPanelConfig;
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

    am_devices_dc_dsi_raydium_hardware_reset();

    sDCConfig.eInterface = DISP_INTERFACE_DBIDSI;
    sDCConfig.ui32PixelFormat = ui32PixelFormat;
    sDisplayPanelConfig.ui32PixelFormat = ui32PixelFormat;
    nemadc_configure(&sDCConfig);
    am_devices_dc_dsi_raydium_init(&sDisplayPanelConfig);
    //
    // send layer 0 to display via NemaDC and DSI
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
    nemadc_set_layer(0, &sLayer0); //!< This function includes layer enable.
    nemadc_transfer_frame_prepare(false);
    //
    //It's necessary to launch frame manually when TE is disabled.
    //
    nemadc_transfer_frame_launch();
    nemadc_wait_vsync();

#if BEFORE_ENTER_ULPS
    while (1);
#endif

#ifdef ULPS_POWER_OPTIM
    am_hal_dsi_ulps_entry(); //!< Enter ULPS
#endif
#ifdef ULPS_DSI_PD
    am_hal_dsi_napping(false); //!< Enter ULPS and power DSI off.
#endif

#if AFTER_ENTER_ULPS
    while (1);
#endif
    am_util_delay_ms(200);

#ifdef ULPS_POWER_OPTIM
    am_hal_dsi_ulps_exit(); //!< Exit ULPS
#endif
#ifdef ULPS_DSI_PD
    am_hal_dsi_wakeup(g_sDispCfg.ui8NumLanes, g_sDispCfg.eDbiWidth, g_sDispCfg.eDsiFreq, false); //!< Exit ULPS and power DSI on.
#endif

#if AFTER_EXIT_ULPS
    while (1);
#endif
// while(1) // Internal test
{
    if (sLayer0.format == NEMADC_BGRA8888)
    {
        sLayer0.format = NEMADC_RGBA8888; //!< Display an image with red color and blue color exchanged.
    }
    else
    {
        sLayer0.format = NEMADC_BGRA8888; //!< Display an image with red color and blue color exchanged.
    }
    nemadc_set_layer(0, &sLayer0);

    nemadc_transfer_frame_prepare(false);
    //
    //It's necessary to launch frame manually when TE is disabled.
    //
    nemadc_transfer_frame_launch();
    nemadc_wait_vsync();

    tsi_free(sLayer0.baseaddr_virt);
    //
    // Run ULPS entry/exit again
    //
#ifdef ULPS_POWER_OPTIM
    am_hal_dsi_ulps_entry(); //!< Enter ULPS
#endif
#ifdef ULPS_DSI_PD
    am_hal_dsi_napping(false); //!< Enter ULPS and power DSI off.
#endif
    am_util_delay_ms(20);
#ifdef ULPS_POWER_OPTIM
    am_hal_dsi_ulps_exit(); //!< Exit ULPS
#endif
#ifdef ULPS_DSI_PD
    am_hal_dsi_wakeup(g_sDispCfg.ui8NumLanes, g_sDispCfg.eDbiWidth, g_sDispCfg.eDsiFreq, false); //!< Exit ULPS and power DSI on.
#endif
}
}

//*****************************************************************************
//
//! @brief initialize DSI interface.
//! @return 0 - Pass, Others - Fail.
//
//*****************************************************************************
int32_t
dsi_power_test(void)
{
    uint8_t ui8DbiWidth = g_sDispCfg.eDbiWidth;
    pixel_format_t eFormat = FMT_RGB888;
    uint8_t ui8LanesNum = g_sDispCfg.ui8NumLanes;
    uint32_t ui32MipiCfg = MIPICFG_8RGB888_OPT0;
    uint32_t ui32FreqTrim = g_sDispCfg.eDsiFreq;

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
            return -3;
    }

    nema_sys_init();
    //
    // Initialize NemaDC
    //
    if (nemadc_init() != 0)
    {
        return -2;
    }
    //
    // Initialize DSI
    //
    if (am_hal_dsi_para_config(ui8LanesNum, ui8DbiWidth, ui32FreqTrim, false) != 0)
    {
        return -1;
    }

    test_MIPI_DSI(ui32MipiCfg);

    return 0;
}

//*****************************************************************************
//
// Main function
//
//*****************************************************************************
int
main(void)
{
    am_util_delay_ms(100);
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
    // This example is just for DSI test. If you want to change display configurations in example, please change them here as below.
    //
    // g_sDsiCfg.ui8NumLanes = 1;
    // g_sDsiCfg.eDbiWidth = AM_HAL_DSI_DBI_WIDTH_8;
    // g_sDsiCfg.eDsiFreq = AM_HAL_DSI_FREQ_TRIM_X12;
    if ( g_sDispCfg.eInterface != DISP_IF_DSI )
    {
        return 0; //!< If the interface in BSP isn't set to DSI, then return.
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
    am_bsp_disp_pins_enable();
    //
    // VDD18 control callback function
    //
    am_hal_dsi_register_external_vdd18_callback(am_bsp_external_vdd18_switch);

    //
    // Enable DSI power and configure DSI clock.
    //
    am_hal_dsi_init();

    //
    // Global interrupt enable
    //
    am_hal_interrupt_master_enable();

    am_hal_pwrctrl_periph_enable(AM_HAL_PWRCTRL_PERIPH_GFX);
    am_hal_pwrctrl_periph_enable(AM_HAL_PWRCTRL_PERIPH_DISP);

    dsi_power_test();
    //
    // Disable DSI power and DSI clock.
    //
    am_hal_dsi_deinit();

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

