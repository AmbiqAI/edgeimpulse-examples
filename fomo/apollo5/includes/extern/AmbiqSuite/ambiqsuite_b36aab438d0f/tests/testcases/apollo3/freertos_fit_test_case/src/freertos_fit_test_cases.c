//*****************************************************************************
//
//! @file freertos_fit_test_cases.c
//!
//! @brief test case fot testing multi tasks running with ble.
//!
//! This example implements a BLE heart rate sensor within the FreeRTOS
//! framework. To minimize power usage, this application is compiled without
//! debug printing by default (the "lp" version of this example excludes
//! them by default).  To enable debug printing, add the following project-level
//! macro definitions.
//!
//! AM_DEBUG_PRINTF
//! WSF_TRACE_ENABLED=1
//!
//! When defined, debug messages will be sent over ITM/SWO.
//!
//! Note that when these macros are defined, the device will never achieve deep
//! sleep, only normal sleep, due to the ITM (and thus the HFRC) being enabled.
//
//*****************************************************************************

//*****************************************************************************
//
// ${copyright}
//
// This is part of revision ${version} of the AmbiqSuite Development Package.
//
//*****************************************************************************

//*****************************************************************************
//
// This application has a large number of common include files. For
// convenience, we'll collect them all together in a single header and include
// that everywhere.
//
//*****************************************************************************
#include "freertos_fit_test_cases.h"
#include "rtos.h"

//void *pWidget;
//char errStr[128];
FREERTOS_FIT_BLE_STATUS eBLE_Power_Status = BLE_POWER_OFF;

//*****************************************************************************
//
// Optional setup/tear-down functions.
//
// These will be called before and after each test function listed below.
//
//*****************************************************************************
void
setUp(void)
{
}

void
tearDown(void)
{
}

//*****************************************************************************
//
// Optional Global setup/tear-down.
//
// globalSetUp() will get called before the test group starts, and
// globalTearDown() will get called after it ends. These won't be necessary for
// every test.
//
//*****************************************************************************
void
globalSetUp(void)
{
    //
    // Print the banner.
    //
    am_util_stdio_terminal_clear();
    am_util_stdio_printf("Apollo3 BLE Test Cases\n\n");

    //
    // Enable debug pins.
    //
    // 30.6 - SCLK
    // 31.6 - MISO
    // 32.6 - MOSI
    // 33.4 - CSN
    // 35.7 - SPI_STATUS
    // 41.1 - IRQ (not configured, also SWO)
    //
    const am_hal_gpio_pincfg_t g_AM_PIN_30_BLE_SCK    = { .uFuncSel = 6 };
    const am_hal_gpio_pincfg_t g_AM_PIN_31_BLE_MISO   = { .uFuncSel = 6 };
    const am_hal_gpio_pincfg_t g_AM_PIN_32_BLE_MOSI   = { .uFuncSel = 6 };
    const am_hal_gpio_pincfg_t g_AM_PIN_33_BLE_CSN    = { .uFuncSel = 4 };
    const am_hal_gpio_pincfg_t g_AM_PIN_35_BLE_STATUS = { .uFuncSel = 7 };
    //const am_hal_gpio_pincfg_t g_AM_PIN_41_BLE_IRQ    = { .uFuncSel = 1 };
    am_hal_gpio_pinconfig(30, g_AM_PIN_30_BLE_SCK);
    am_hal_gpio_pinconfig(31, g_AM_PIN_31_BLE_MISO);
    am_hal_gpio_pinconfig(32, g_AM_PIN_32_BLE_MOSI);
    am_hal_gpio_pinconfig(33, g_AM_PIN_33_BLE_CSN);
    am_hal_gpio_pinconfig(35, g_AM_PIN_35_BLE_STATUS);
    //am_hal_gpio_pinconfig(41, g_AM_PIN_41_BLE_IRQ);

#ifndef NOFPU
    //
    // Enable the floating point module, and configure the core for lazy
    // stacking.
    //
    am_hal_sysctrl_fpu_enable();
    am_hal_sysctrl_fpu_stacking_enable(true);
#else
    am_hal_sysctrl_fpu_disable();
#endif

    //
    // Configure the board for low power.
    //
    am_bsp_low_power_init();

    // Turn off unused Flash & SRAM
#if defined(AM_PART_APOLLO3)
    am_hal_pwrctrl_memory_enable(AM_HAL_PWRCTRL_MEM_SRAM_96K);
    am_hal_pwrctrl_memory_enable(AM_HAL_PWRCTRL_MEM_FLASH_512K);
#endif
#if defined(AM_PART_APOLLO3P)
    am_hal_pwrctrl_memory_enable(AM_HAL_PWRCTRL_MEM_SRAM_128K);
    am_hal_pwrctrl_memory_enable(AM_HAL_PWRCTRL_MEM_FLASH_1M);
#endif
#ifdef AM_PART_APOLLO
    //
    // SRAM bank power setting.
    // Need to match up with actual SRAM usage for the program
    // Current usage is between 32K and 40K - so disabling upper 3 banks
    //
    am_hal_mcuctrl_sram_power_set(AM_HAL_MCUCTRL_SRAM_POWER_DOWN_5 |
                                  AM_HAL_MCUCTRL_SRAM_POWER_DOWN_6 |
                                  AM_HAL_MCUCTRL_SRAM_POWER_DOWN_7,
                                  AM_HAL_MCUCTRL_SRAM_POWER_DOWN_5 |
                                  AM_HAL_MCUCTRL_SRAM_POWER_DOWN_6 |
                                  AM_HAL_MCUCTRL_SRAM_POWER_DOWN_7);

#if 0 // Not turning off the Flash as it may be needed to download the image
    //
    // Flash bank power set.
    //
    am_hal_mcuctrl_flash_power_set(AM_HAL_MCUCTRL_FLASH_POWER_DOWN_1);
#endif
#endif // AM_PART_APOLLO

#ifdef AM_PART_APOLLO2
#if 0 // Not turning off the Flash as it may be needed to download the image
    am_hal_pwrctrl_memory_enable(AM_HAL_PWRCTRL_MEMEN_FLASH512K);
#endif
    am_hal_pwrctrl_memory_enable(AM_HAL_PWRCTRL_MEMEN_SRAM64K);
#endif // AM_PART_APOLLO2

    //
    // Enable printing to the console.
    //
#ifdef AM_DEBUG_PRINTF
    am_bsp_itm_printf_enable();
#endif

    //
    // Initialize plotting interface.
    //
    am_util_stdio_printf("FreeRTOS Fit Multi Task Test Case\n");

//    am_widget_ble_setup(&pWidget, errStr);

//    am_hal_interrupt_master_enable();
}

void
globalTearDown(void)
{
}

void
freertos_fit_test(void)
{
    TEST_ASSERT_EQUAL_INT32(0, run_tasks());
}


