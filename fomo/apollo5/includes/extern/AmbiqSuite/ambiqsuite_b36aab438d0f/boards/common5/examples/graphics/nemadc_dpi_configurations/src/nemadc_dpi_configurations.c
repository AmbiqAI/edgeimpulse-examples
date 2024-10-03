//*****************************************************************************
//
//! @file nemadc_dpi_configurations.c
//!
//! @brief NemaDC DPI(Display Pixel Interface) configuration example.
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

#include "nemadc_dpi_configurations.h"
#include "am_devices_dc_dpi_japandisplayinc.h"
#include "test_card_rgba2222.h"

//
// It has the restriction that the layer size is a multiple of 4 for Apollo5.
//
#define FB_RESX     400
#define FB_RESY     240

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
    am_devices_dc_dpi_timer_config_t sTimerConfig =
    {
        .ui32TimerNum = 7,
        .eTimerClock = AM_HAL_TIMER_CLOCK_HFRC_DIV16,
        .ui32Frequency = 1
    };
    nemadc_initial_config_t sDCConfig;
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
    am_util_stdio_printf("nemadc_dpi_configurations example.\n");
    //
    // Please select proper clock source for the panel
    //
    am_hal_clkgen_control(AM_HAL_CLKGEN_CONTROL_DISPCLKSEL_HFRC12, NULL);
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

    g_sDispCfg.eInterface = DISP_IF_DPI;

    am_bsp_disp_pins_enable();

    am_devices_dc_dpi_japandisplayinc_init(&sTimerConfig, &sDCConfig);

    nemadc_configure(&sDCConfig);

    nemadc_layer_t sLayer0 = {0};

    sLayer0.resx     = FB_RESX;
    sLayer0.resy     = FB_RESY;
    sLayer0.buscfg   = 0;
    //
    // Any supported RGB color formats are optionable.
    //
    sLayer0.format   = NEMADC_RGBA2222;
    sLayer0.blendmode = NEMADC_BL_SRC;
    sLayer0.stride   = nemadc_stride_size(sLayer0.format, sLayer0.resx);
    sLayer0.startx   = 0;
    sLayer0.starty   = 0;
    sLayer0.alpha    = 0xff;
    sLayer0.flipx_en  = 0;
    sLayer0.flipy_en  = 0;
    sLayer0.baseaddr_virt = tsi_malloc(sLayer0.resy*sLayer0.stride);
    sLayer0.baseaddr_phys = (unsigned)(sLayer0.baseaddr_virt);
    nema_memcpy(sLayer0.baseaddr_virt, test_card_rgba2222, sizeof(test_card_rgba2222));

    am_devices_dc_dpi_japandisplayinc_timer_start();

    while (1)
    {
        //
        // config SD(1U<< 15),CM(1U<<16) signals of DPI.
        //
        nemadc_reg_write(NEMADC_REG_GPIO, 0x01 << 15);

        nemadc_set_layer(0, &sLayer0);
        nemadc_transfer_frame_launch();
        nemadc_wait_vsync();

        dc_power_down();
        am_util_delay_ms(100);
        dc_power_up();
    }
}
