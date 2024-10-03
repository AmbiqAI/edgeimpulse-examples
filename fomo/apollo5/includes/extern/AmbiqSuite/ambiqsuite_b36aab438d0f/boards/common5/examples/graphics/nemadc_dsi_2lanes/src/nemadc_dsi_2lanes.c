//*****************************************************************************
//
//! @file nemadc_dsi_2lanes.c
//!
//! @brief DSI example.
//!
//! This example demonstrates how to drive the DSI block with 2 data lanes
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

#include "nema_utils.h"
#include "nemadc_dsi_2lanes.h"
#include "am_devices_dc_dsi_novatek.h"
#include "golden_320x360_rgba565.h"

//
// It has the restriction that the layer size is a multiple of 4 for Apollo5.
//
#define FB_RESX 320
#define FB_RESY 360

//*****************************************************************************
//
//! @brief Demonstrate 2lanes DSI feature.
//!
//! This function initializes the frame buffer and transfers the frame.
//!
//! @return None.
//
//*****************************************************************************
void
test_2lanes_dsi(void)
{
    nemadc_layer_t sLayer0 = {0};
    sLayer0.resx = FB_RESX;
    sLayer0.resy = FB_RESY;
    sLayer0.buscfg = 0;
    sLayer0.format = NEMADC_RGB565;
    sLayer0.blendmode = NEMADC_BL_SRC;
    sLayer0.stride = nemadc_stride_size(sLayer0.format, sLayer0.resx);
    sLayer0.startx = 0;
    sLayer0.starty = 0;
    sLayer0.sizex = sLayer0.resx;
    sLayer0.sizey = sLayer0.resy;
    sLayer0.alpha = 0xFF;
    sLayer0.flipx_en = 0;
    sLayer0.flipy_en = 0;
    sLayer0.baseaddr_virt = tsi_malloc(sLayer0.resy * sLayer0.stride);
    sLayer0.baseaddr_phys = (unsigned)(sLayer0.baseaddr_virt);

    nema_memcpy((char*)sLayer0.baseaddr_virt, golden_360_rgba565, sizeof(golden_360_rgba565));
    //
    // Loop forever
    //
    while(1)
    {
        nemadc_set_layer(0, &sLayer0);
        nemadc_transfer_frame_prepare(g_sDispCfg.eTEType != DISP_TE_DISABLE);
        if (g_sDispCfg.eTEType == DISP_TE_DISABLE)
        {
            //
            //It's necessary to launch frame manually when TE is disabled.
            //
            nemadc_transfer_frame_launch();
        }
        nemadc_wait_vsync();
        //
        // Added a delay to avoid image shake when transferring frames back to back, we think it should be the panel limitation.
        //
// #### INTERNAL BEGIN ####
        //
        // More information about this, please refer to ticket CAYNSWS-1623
        //
// #### INTERNAL END ####
        am_util_delay_us(20);
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
    // panel's resolution.
    //
    uint32_t ui32MipiCfg = MIPICFG_16RGB888_OPT0;
    uint16_t ui16MinX, ui16MinY;
    nemadc_initial_config_t sDCConfig;
    am_devices_dc_config_t sDisplayPanelConfig;
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
    am_util_stdio_printf("nemadc_dsi_2lanes example.\n");

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

    g_sDispCfg.eIC = DISP_IC_NT38350;
    g_sDispCfg.ui16ResX = FB_RESX;
    g_sDispCfg.ui16ResY = FB_RESY;
    g_sDispCfg.ui16Offset = 0;
    g_sDispCfg.ui8NumLanes = 2;

    am_bsp_disp_pins_enable();
    
    //
    // Set the display region to center
    //
    sDisplayPanelConfig.ui16ResX = FB_RESX;
    if (sDisplayPanelConfig.ui16ResX < g_sDispCfg.ui16ResX)
    {
        ui16MinX = (g_sDispCfg.ui16ResX - sDisplayPanelConfig.ui16ResX) >> 1;
        ui16MinX = (ui16MinX >> 1) << 1;
    }
    else
    {
        ui16MinX = 0;
    }

    sDisplayPanelConfig.ui16ResY = FB_RESY;
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
    sDisplayPanelConfig.ui16MinX = ui16MinX;
    sDisplayPanelConfig.ui16MinY = ui16MinY;
    sDisplayPanelConfig.bTEEnable = (g_sDispCfg.eTEType != DISP_TE_DISABLE);
    sDisplayPanelConfig.bFlip = g_sDispCfg.bFlip;
    sDisplayPanelConfig.ui8Lanes = g_sDispCfg.ui8NumLanes;

    if ( g_sDispCfg.eInterface != DISP_IF_DSI )
    {
        am_util_stdio_printf("Invalid display interface.\n");
        return 0; //!< If the interface in BSP isn't set to DSI, then return.
    }

    //
    // VDD18 control callback function
    //
    am_hal_dsi_register_external_vdd18_callback(am_bsp_external_vdd18_switch);
    //
    // Enable DSI power and configure DSI clock.
    //
    am_hal_dsi_init();

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
    //
    // Configure Lanes, DBI interface width and DSI frequency.
    //
    uint8_t ui8LanesNum = g_sDispCfg.ui8NumLanes;
    uint8_t ui8DbiWidth = g_sDispCfg.eDbiWidth;

    //
    // The DSI trim value will be updated when we get the exact clock limitation of display driver IC for RGB565 and RGB888
    //
#if 1
    //
    // AM_HAL_DSI_FREQ_TRIM_X7 means DSI frequency is 12MHz X 7 = 84MHz(DDR)
    //
    g_sDispCfg.eDsiFreq = AM_HAL_DSI_FREQ_TRIM_X7;

    //
    // Selected RGB565 as DSI pixel format.
    //
    pixel_format_t eFormat = FMT_RGB565;
#else
    //
    // AM_HAL_DSI_FREQ_TRIM_X10 means DSI frequency is 12MHz X 10 = 120MHz(DDR)
    //
    g_sDispCfg.eDsiFreq = AM_HAL_DSI_FREQ_TRIM_X10;

    //
    // Selected RGB888 as DSI pixel format.
    //
    pixel_format_t eFormat = FMT_RGB888;
#endif
    uint32_t ui32FreqTrim = g_sDispCfg.eDsiFreq;
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

    am_devices_dc_dsi_novatek_hardware_reset();
    sDCConfig.eInterface = DISP_INTERFACE_DBIDSI;
    sDCConfig.ui32PixelFormat = ui32MipiCfg;
    sDisplayPanelConfig.ui32PixelFormat = sDCConfig.ui32PixelFormat;
    nemadc_configure(&sDCConfig);
    am_devices_dc_dsi_novatek_init(&sDisplayPanelConfig);

    test_2lanes_dsi();

    while (1)
    {

    }

}
