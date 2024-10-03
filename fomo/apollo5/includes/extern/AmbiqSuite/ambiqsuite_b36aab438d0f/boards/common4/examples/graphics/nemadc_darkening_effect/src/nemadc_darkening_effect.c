//*****************************************************************************
//
//! @file nemadc_darkening_effect.c
//!
//! @brief NemaDC Darkening effect example.
//!
//! @addtogroup graphics_examples Graphics Examples
//!
//! @defgroup nemadc_darkening_effect NemaDC Darkening Effect Example
//! @ingroup graphics_examples
//! @{
//!
//! Purpose: This example demonstrates how to implement pull-down menu and darkening effects with DC.
//! This example only supports MIPI DSI interface.
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

#include "nemadc_darkening_effect.h"
#include "windmill_402x476_rgba565.h"
#include "umbrella_402x476_rgba565.h"

#define FB_RESX 402
#define FB_RESY 476

//*****************************************************************************
//
//! @brief Test DBI2DSI interface.
//!
//! @param ui32PixelFormat - Panel pixel format
//!
//! @return None.
//
//*****************************************************************************
void
nemadc_darkening_effect()
{
    int32_t i32Offset = 0;
    bool bSign = 0;
    uint8_t ui8Step = 2;

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
    memcpy((char*)sLayer0.baseaddr_virt, g_ui8Windmill402x476RGBA565, sizeof(g_ui8Windmill402x476RGBA565));
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
    memcpy((char*)sLayer2.baseaddr_virt, g_ui8Umbralla402x476RGBA565, sizeof(g_ui8Umbralla402x476RGBA565));
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

        nemadc_transfer_frame_prepare(false);
        //
        // start frame transfer with DPI disabled
        //
        nemadc_transfer_frame_launch();
        nemadc_wait_vsync();
        i32Offset = bSign ? (i32Offset - ui8Step) : (i32Offset + ui8Step);

        if (i32Offset >= FB_RESX)
        {
            //! minus
            bSign = 1;
        }

        if (i32Offset <= 0)
        {
            //! plus
            bSign = 0;
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
    uint32_t ui32MipiCfg = MIPICFG_16RGB888_OPT0;    //!< default config
    uint16_t ui16MinX, ui16MinY;
    nemadc_initial_config_t sDCConfig;
    am_devices_dc_dsi_raydium_config_t sDisplayPanelConfig;

    uint8_t ui8DbiWidth = g_sDispCfg.eDbiWidth;
    pixel_format_t eFormat = FMT_RGB888;
    uint8_t ui8LanesNum = g_sDispCfg.ui8NumLanes;
    uint32_t ui32FreqTrim = g_sDispCfg.eDsiFreq;
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
    // This example is just for DSI test. If you want to change display configurations in example, please change them here as below.
    //
    // g_sDispCfg.ui8NumLanes = 1;
    // g_sDispCfg.eDbiWidth = AM_HAL_DSI_DBI_WIDTH_8;
    // g_sDispCfg.eDsiFreq = AM_HAL_DSI_FREQ_TRIM_X12;
    if (g_sDispCfg.eInterface != DISP_IF_DSI)
    {
        //
        // If the interface in BSP isn't set to DSI, then return.
        //
        return 0;
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

    nema_sys_init();
    //
    // Initialize NemaDC
    //
    if (nemadc_init() != 0)
    {
        return -2;
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
    //
    // Initialize DSI
    //
    if (am_hal_dsi_para_config(ui8LanesNum, ui8DbiWidth, ui32FreqTrim, false) != 0)
    {
        return -1;
    }

    am_devices_dc_dsi_raydium_hardware_reset();
    sDCConfig.eInterface = DISP_INTERFACE_DBIDSI;
    sDCConfig.ui32PixelFormat = ui32MipiCfg;
    sDisplayPanelConfig.ui32PixelFormat = ui32MipiCfg;
    nemadc_configure(&sDCConfig);
    am_devices_dc_dsi_raydium_init(&sDisplayPanelConfig);

    nemadc_darkening_effect();

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

