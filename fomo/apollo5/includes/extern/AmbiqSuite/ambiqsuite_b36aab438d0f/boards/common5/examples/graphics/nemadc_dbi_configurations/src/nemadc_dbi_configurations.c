//*****************************************************************************
//
//! @file nemadc_dbi_configurations.c
//!
//! @brief NemaDC example.
//!
//! This example demonstrates how to drive the DBI-B(8080) interface panel.
//! It is also a simple example of dynamically controlling the power of DC.
//
//*****************************************************************************

//*****************************************************************************
//
// ${copyright}
//
// This is part of revision ${version} of the AmbiqSuite Development Package.
//
//*****************************************************************************

#include "nemadc_dbi_configurations.h"
#include "am_devices_dc_dbi_novatek.h"
#include "Scotty_rgba565.h"

//*****************************************************************************
//
//! @brief Disable the power & clock for display controller to save power
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

        am_hal_clkgen_control(AM_HAL_CLKGEN_CONTROL_DCCLK_DISABLE, NULL);
    }
    return true;
}

//*****************************************************************************
//
//! @brief Enable display controller
//!
//! @note Enable clock & power for display controller, then restore the configuration
//!
//! @return true if success, otherwise return false.
//
//*****************************************************************************
static bool
dc_power_up(void)
{
    bool status;
    am_hal_pwrctrl_periph_enabled(AM_HAL_PWRCTRL_PERIPH_DISP, &status);
    if ( !status )
    {
        am_hal_clkgen_control(AM_HAL_CLKGEN_CONTROL_DCCLK_ENABLE, NULL);

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
    //
    // Please select any proper clock source.
    //
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

    while(1)
    {
        if(sLayer0.format == NEMADC_RGB565)
        {
            sLayer0.format = NEMADC_BGR565;
        }
        else
        {
            sLayer0.format = NEMADC_RGB565;
        }
        nemadc_set_layer(0, &sLayer0);

        nemadc_transfer_frame_prepare(false);

        nemadc_transfer_frame_launch();

        nemadc_wait_vsync();

        dc_power_down();
        am_util_delay_ms(100);
        dc_power_up();
    }

}

