//*****************************************************************************
//
//! @file nemadc_dsi_power_test.c
//!
//! @brief DSI example.
//!
//! This example demonstrates DSI power saving sequence.
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

#include "nemadc_dsi_power_test.h"
#include "am_devices_dc_dsi_raydium.h"
#include "oli_200x200_rgba.h"

//
// It has the restriction that the layer size is a multiple of 4 for Apollo5.
//
#define FB_RESX 200
#define FB_RESY 200

// #define ULPS_POWER_OPTIM     //!< ULPS mode with all configurations for power optimization.
#define ULPS_DSI_PD             //!< ULPS mode with all configurations for power optimization and with DISPPHY powered down.

//#define BEFORE_DSI_POWER_ON   //!< Block the running process to prepare the environment for measuring the VDD18 current that DSI powered down.
//#define BEFORE_ENTER_ULPS     //!< Block the running process to prepare the environment for measuring the VDD18 current that DSI in HP mode.
//#define AFTER_ENTER_ULPS      //!< Block the running process to prepare the environment for measuring the VDD18 current that DSI in ULPS mode.
//#define AFTER_EXIT_ULPS       //!< Block the running process to prepare the environment for measuring the VDD18 current that DSI out ULPS mode.

//#define DISPLAY_ON_OFF        //!< Enable display powering on/off.

#define ULPS_PATTERN_ON

#ifdef ULPS_PATTERN_ON
    #define ULPS_PATTERN_ON_OFF true
#else
    #define ULPS_PATTERN_ON_OFF false
#endif

static uint32_t dsi_reg_function   = 0;
static uint32_t dsi_reg_afetrim1   = 0;

//*****************************************************************************
//
//! @brief Disable DSI to reduce power consumption.
//!
//! Retained two important present registers value for DSI power on.
//!
//! @return true
//
//*****************************************************************************
static bool
dphy_power_down(void)
{
    bool status;
    am_hal_pwrctrl_periph_enabled(AM_HAL_PWRCTRL_PERIPH_DISPPHY, &status);
    if ( status )
    {
        //
        // Get DSI functional programming register
        //
        dsi_reg_function = DSI->DSIFUNCPRG;

        //
        // Get DSI trim register that included DSI frequency
        //
        dsi_reg_afetrim1 = DSI->AFETRIM1 & 0x0000007F;

        am_hal_dsi_napping(ULPS_PATTERN_ON_OFF);
    }
    return true;
}

//*****************************************************************************
//
//! @brief Enable DSI
//!
//! Resume the previous DSI configuration.
//!
//! @return bool.
//
//*****************************************************************************
static bool
dphy_power_up(void)
{
    if (dsi_reg_afetrim1 == 0 && dsi_reg_function == 0)
    {
        //
        // DSI configuration parameters are invalid.
        //
        return false;
    }

    uint32_t ui32FreqTrim = _FLD2VAL(DSI_AFETRIM1_AFETRIM1, dsi_reg_afetrim1);
    uint8_t ui8LanesNum = (uint8_t)_FLD2VAL(DSI_DSIFUNCPRG_DATALANES, dsi_reg_function);
    uint8_t ui8DBIBusWidth = (uint8_t)_FLD2VAL(DSI_DSIFUNCPRG_REGNAME, dsi_reg_function);
    if (4 == ui8DBIBusWidth)
    {
        ui8DBIBusWidth = 16;
    }
    else if (3 == ui8DBIBusWidth)
    {
        ui8DBIBusWidth = 8;
    }
    else if (2 == ui8DBIBusWidth)
    {
        ui8DBIBusWidth = 9;
    }
    else
    {
        return false;
    }

    am_hal_dsi_wakeup(ui8LanesNum, ui8DBIBusWidth, ui32FreqTrim, ULPS_PATTERN_ON_OFF);
    return true;
}

//*****************************************************************************
//
//! @brief Disable Display Controller
//!
//! @return true.
//
//*****************************************************************************
static bool
dc_power_down(void)
{
    bool status;
    am_hal_pwrctrl_periph_enabled(AM_HAL_PWRCTRL_PERIPH_DISP, &status);
    if ( status )
    {
        nemadc_backup_registers();
        am_hal_pwrctrl_periph_disable(AM_HAL_PWRCTRL_PERIPH_DISP);
    }
    return true;
}

//*****************************************************************************
//
//! @brief Enable & Restore Display controller
//!
//! Resume the previous DC configuration
//!
//! @return bool
//
//*****************************************************************************
static bool
dc_power_up(void)
{
    bool status;
    am_hal_pwrctrl_periph_enabled(AM_HAL_PWRCTRL_PERIPH_DISP, &status);
    if ( !status )
    {
        am_hal_pwrctrl_periph_enable(AM_HAL_PWRCTRL_PERIPH_DISP);
        if (0 != nemadc_init())
        {
            //
            // Initialize DC failed.
            //
            return false;
        }
        return nemadc_restore_registers();
    }
    return true;
}

//*****************************************************************************
//
//! @brief Enable display
//!
//! @note the panel exit sleep mode.
//!
//! @return 0
//
//*****************************************************************************
int32_t
display_panel_enable(void)
{
#ifdef DISPLAY_ON_OFF
    nemadc_mipi_cmd_write(MIPI_exit_sleep_mode, NULL, 0, true, false);
    am_util_delay_ms(130);
#endif
    return 0;
}

//*****************************************************************************
//
//! @brief Disable display
//!
//! @note the panel disbled and enter sleep mode.
//!
//! @return 0
//
//*****************************************************************************
int32_t
display_panel_disable(void)
{
#ifdef DISPLAY_ON_OFF
    nemadc_mipi_cmd_write(MIPI_set_display_off, NULL, 0, true, false);
    am_util_delay_ms(20);

    nemadc_mipi_cmd_write(MIPI_enter_sleep_mode, NULL, 0, true, false);
    am_util_delay_ms(120);
#endif
    return 0;
}
//*****************************************************************************
//
//! @brief this function disable DC,D_PHY and Display panel
//!
//! @return None.
//
//*****************************************************************************
void
gfx_power_down(void)
{
    display_panel_disable();
    dc_power_down();
    dphy_power_down();
}

//*****************************************************************************
//
//! @brief this function enable DC,D_PHY and Display panel.
//!
//! @return None.
//
//*****************************************************************************
void
gfx_power_up(void)
{
    dphy_power_up();
    dc_power_up();
    display_panel_enable();
}

//*****************************************************************************
//
//! @brief initialize DSI interface.
//! @return 0 - Pass, Others - Fail.
//
//*****************************************************************************
void
dsi_power_test(void)
{
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
    // Program NemaDC Layer0.This function includes layer enable.
    //
    nemadc_set_layer(0, &sLayer0);
    nemadc_transfer_frame_prepare(g_sDispCfg.eTEType != DISP_TE_DISABLE);
    if ( g_sDispCfg.eTEType == DISP_TE_DISABLE )
    {
        //
        //It's necessary to launch frame manually when TE is disabled.
        //
        nemadc_transfer_frame_launch();
    }
    nemadc_wait_vsync();

#ifdef BEFORE_ENTER_ULPS
    am_util_stdio_printf("\nBefore entering ULPS mode.Please measure the current of VDD18.\n");
    while (1);
#endif

#ifdef ULPS_POWER_OPTIM
    am_hal_dsi_ulps_entry(); //!< Enter ULPS
#endif
#ifdef ULPS_DSI_PD
    am_hal_dsi_napping(ULPS_PATTERN_ON_OFF); //!< Enter ULPS and power DSI off.
#endif

#ifdef AFTER_ENTER_ULPS
    am_util_stdio_printf("\nAfter entering ULPS mode.Please measure the current of VDD18.\n");
    while (1);
#endif
    am_util_delay_ms(200);

#ifdef ULPS_POWER_OPTIM
    am_hal_dsi_ulps_exit(); //!< Exit ULPS
#endif
#ifdef ULPS_DSI_PD
    am_hal_dsi_wakeup(g_sDispCfg.ui8NumLanes, g_sDispCfg.eDbiWidth, g_sDispCfg.eDsiFreq, ULPS_PATTERN_ON_OFF); //!< Exit ULPS and power DSI on.
#endif

#ifdef AFTER_EXIT_ULPS
    am_util_stdio_printf("\nAfter exiting ULPS mode.Please measure the current of VDD18.\n");
    while (1);
#endif
    am_util_stdio_printf("\nFlicker the image intentionally.\n");
    while (1)
    {
        //
        // To flicker image intentionally.
        //
        if (sLayer0.format == NEMADC_BGRA8888)
        {
            //
            // To display image with original color.
            //
            sLayer0.format = NEMADC_RGBA8888;
        }
        else
        {
            //
            // Display image with red color and blue color exchanged.
            //
            sLayer0.format = NEMADC_BGRA8888;
        }
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
#ifdef DISPLAY_ON_OFF
        nemadc_mipi_cmd_write(MIPI_set_display_on, NULL, 0, true, false);
        am_util_delay_ms(200);
#endif

        //
        // Run ULPS entry/exit again
        //
        gfx_power_down();
        am_util_delay_ms(20);
        gfx_power_up();

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
    am_devices_dc_dsi_raydium_config_t sDisplayPanelConfig;

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
    am_util_stdio_printf("nemadc_dsi_power_test example.\n");

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
    sDisplayPanelConfig.ui16MinX = ui16MinX + g_sDispCfg.ui16Offset;
    sDisplayPanelConfig.ui16MinY = ui16MinY;
    sDisplayPanelConfig.bTEEnable = (g_sDispCfg.eTEType != DISP_TE_DISABLE);
    sDisplayPanelConfig.bFlip = g_sDispCfg.bFlip;

    if ( g_sDispCfg.eInterface != DISP_IF_DSI )
    {
        am_util_stdio_printf("Invalid display interface.\n");
        return 0; //!< If the interface in BSP isn't set to DSI, then return.
    }

#ifdef BEFORE_DSI_POWER_ON
    am_util_stdio_printf("\nBefore DSI power on.Please measure the current of VDD18.\n");
    while (1);
#endif

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
    //
    // Initialize the display
    //
    am_devices_dc_dsi_raydium_hardware_reset();
    sDCConfig.eInterface = DISP_INTERFACE_DBIDSI;
    sDCConfig.ui32PixelFormat = ui32MipiCfg;
    sDisplayPanelConfig.ui32PixelFormat = sDCConfig.ui32PixelFormat;
    nemadc_configure(&sDCConfig);
    am_devices_dc_dsi_raydium_init(&sDisplayPanelConfig);

    dsi_power_test();

    while (1)
    {
    }

}
