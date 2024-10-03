//*****************************************************************************
//
//! @file aas_app.c
//!
//! @brief Ambiq Audio Suite app main.
//!
//! This example prints a "Hello World" message with some device info
//! over SWO at 1M baud. To see the output of this program, use J-Link
//! SWO Viewer (or similar viewer appl) and configure SWOClock for 1000.
//! The example sleeps after it is done printing.
//
//*****************************************************************************

//*****************************************************************************
//
// ${copyright}
//
// This is part of revision ${version} of the AmbiqSuite Development Package.
//
//*****************************************************************************

#include "aas_app.h"
#include "rtos.h"

void aas_app_select_hp_power_mode(void)
{
    am_hal_pwrctrl_mcu_mode_e eCurrentPowerMode;
    uint32_t ui32Ret;

#ifdef AM_PART_APOLLO5A
    // TODO: As the example will crash when CPU freq is 250MHz, so add a
    // workaround here to force the trim to 6 and cpu hp freq to 192MHz,
    // then invoke the hal API to set the mcu mode to high performance.
    if (g_ui32TrimVer > 6)
    {
        am_util_stdio_printf("\nCurrent Trim Ver: %d\n", g_ui32TrimVer);
        am_util_stdio_printf("Force Trim Ver to 6, CPU 192MHz !!!\n");
        am_util_delay_ms(1);
        g_ui32TrimVer               = 6;
        CLKGEN->MISC_b.CPUHPFREQSEL = CLKGEN_MISC_CPUHPFREQSEL_HP192MHz;
    }
#endif

    ui32Ret = am_hal_pwrctrl_mcu_mode_select(AM_HAL_PWRCTRL_MCU_MODE_HIGH_PERFORMANCE);
    if (ui32Ret == AM_HAL_STATUS_SUCCESS)
    {
        am_util_stdio_printf("Switched to High Performance mode (250MHz)\n");
    }
    else
    {
        am_util_stdio_printf("ERROR: Switched to High Performance mode fail!\n");
    }

    ui32Ret = am_hal_pwrctrl_mcu_mode_status(&eCurrentPowerMode);
    if (ui32Ret != AM_HAL_STATUS_SUCCESS)
    {
        am_util_stdio_printf("ERROR (%d) getting current power mode status\n", ui32Ret);
    }
    else
    {
        if (eCurrentPowerMode == AM_HAL_PWRCTRL_MCU_MODE_HIGH_PERFORMANCE)
        {
            am_util_stdio_printf("Running at High Performance Mode (250MHz)\n");
        }
        else if (eCurrentPowerMode == AM_HAL_PWRCTRL_MCU_MODE_LOW_POWER)
        {
            am_util_stdio_printf("Running at Low Power Mode (96MHz)\n");
        }
        else
        {
            am_util_stdio_printf("ERROR:Running at unknown power mode (%d)\n", eCurrentPowerMode);
        }
    }

}

//*****************************************************************************
//
// Main
//
//*****************************************************************************
int
main(void)
{
    //
    // Configure the board for low power operation.
    //
    am_bsp_low_power_init();

    //
    //  Enable the I-Cache and D-Cache.
    //
    am_hal_cachectrl_icache_enable();
    am_hal_cachectrl_dcache_enable(true);

    am_util_delay_ms(10);
#ifndef AM_DEBUG_PRINTF
    am_hal_pwrctrl_control(AM_HAL_PWRCTRL_CONTROL_DIS_PERIPHS_ALL, 0);
#endif
#if defined (AM_PART_APOLLO5B)
    am_hal_pwrctrl_control(AM_HAL_PWRCTRL_CONTROL_CRYPTO_POWERDOWN, 0);

    MCUCTRL->APBDMACTRL_b.HYSTERESIS = 0x0;
    MCUCTRL->DBGCTRL = 0;
#endif

#ifdef AM_DEBUG_PRINTF
    //
    // Start the ITM interface.
    //
    am_bsp_itm_printf_enable();

    //
    // Print the banner.
    //
    am_util_stdio_terminal_clear();
    am_util_stdio_printf("==============================================\n");
    am_util_stdio_printf("Ambiq Audio Suite(AAS) Example.\n\n");

    am_util_delay_ms(50);
#endif

#ifdef AAS_MCU_HIGH_PERFORMANCE
    aas_app_select_hp_power_mode();
#endif



    //
    // Run the application.
    //
    run_tasks();

    am_util_stdio_printf("Error: AAS app exit from run_tasks(). \n\n");
    while (1)
    {
        // run_tasks() is infinite loop. Should not get here.
    }
}
