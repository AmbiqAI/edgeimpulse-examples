//*****************************************************************************
//
//! @file ble_power_up.c
//!
//! @brief Example test case program.
//
//*****************************************************************************

//*****************************************************************************
//
// ${copyright}
//
// This is part of revision ${version} of the AmbiqSuite Development Package.
//
//*****************************************************************************

#include "unity.h"
#include "am_mcu_apollo.h"
#include "ble_power_up_helpers.h"

//*****************************************************************************
//
// Test configuration variables.
//
//*****************************************************************************
#define USE_UART_PRINTF          0

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
    // Set the clock frequency.
    //
    am_hal_clkgen_control(AM_HAL_CLKGEN_CONTROL_SYSCLK_MAX, 0);
    am_bsp_low_power_init();

    //AM_REG(PWRCTRL, SUPPLYSRC) = AM_REG_PWRCTRL_SUPPLYSRC_BLEBUCKEN_EN;

    //
    // Initialize the printf interface. We will either use UART or ITM
    // depending on the configuration.
    //
#if USE_UART_PRINTF
    am_bsp_uart_printf_enable();
#else
    am_bsp_itm_printf_enable();
#endif

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

    //
    // Print the banner.
    //
    am_util_stdio_terminal_clear();
    am_util_stdio_printf("Apollo3 BLE Power-up Test Case\n\n");

    //
    // Print the part information.
    //
    am_util_id_t sIdDevice;
    am_util_id_device(&sIdDevice);
    am_util_stdio_printf("Device Info:\n"
                         "\tPart number: 0x%08X\n"
                         "\tChip ID0:    0x%08X\n"
                         "\tChip ID1:    0x%08X\n"
                         "\tRevision:    0x%08X (Rev%c%c)\n\n",
                         sIdDevice.sMcuCtrlDevice.ui32ChipPN,
                         sIdDevice.sMcuCtrlDevice.ui32ChipID0,
                         sIdDevice.sMcuCtrlDevice.ui32ChipID1,
                         sIdDevice.sMcuCtrlDevice.ui32ChipRev,
                         sIdDevice.ui8ChipRevMaj, sIdDevice.ui8ChipRevMin );

    am_hal_interrupt_master_enable();
}

void
globalTearDown(void)
{
}

//*****************************************************************************
//
// Make sure that a blocking read/write pair can be executed.
//
//*****************************************************************************
void
ble_power_cycle_test(void)
{
    TEST_ASSERT_EQUAL_INT32(0, ble_test_run());
}

