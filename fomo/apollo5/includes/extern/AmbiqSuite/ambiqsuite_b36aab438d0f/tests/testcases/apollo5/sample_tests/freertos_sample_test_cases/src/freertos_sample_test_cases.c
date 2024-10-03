//*****************************************************************************
//
//! @file freertos_sample_test_cases.c
//!
//! @brief Example test case program which demonstrates using freeRTOS
//!        in conjunction with the unity test framework
//!
//!        The following GPIOs are configured for observation:
//!
//!        Cayenne  Turbo FMC 
//!          GPIO     GPIO           Toggles
//!        -------  --------- ---------------------------------
//!           5         0     on each iteration of Hello Task loop
//!           6         1     on each iteration of Idle Task
//!           14        2     before deepsleep entry
//!           7         3     immediately following deepsleep exit
//!           4         4     on each Timer ISR entry
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

#include "unity.h"
#include "am_mcu_apollo.h"
#include "am_util.h"
#include "freertos_sample_test_cases.h"

//*****************************************************************************
//
// Global Variables
//
//*****************************************************************************
bool g_bTestEnd = false;

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
    // Clear status GPIOs
    am_hal_gpio_state_write(GPIO_HELLO_TASK, AM_HAL_GPIO_OUTPUT_CLEAR);
    am_hal_gpio_state_write(GPIO_IDLE_TASK, AM_HAL_GPIO_OUTPUT_CLEAR);
    am_hal_gpio_state_write(GPIO_SLEEP_ENTRY, AM_HAL_GPIO_OUTPUT_CLEAR);
    am_hal_gpio_state_write(GPIO_SLEEP_EXIT, AM_HAL_GPIO_OUTPUT_CLEAR);
    am_hal_gpio_state_write(GPIO_TIMER_ISR, AM_HAL_GPIO_OUTPUT_CLEAR);

    // Initialize status GPIOs as output
    am_hal_gpio_pinconfig(GPIO_HELLO_TASK, am_hal_gpio_pincfg_output);
    am_hal_gpio_pinconfig(GPIO_IDLE_TASK, am_hal_gpio_pincfg_output);
    am_hal_gpio_pinconfig(GPIO_SLEEP_ENTRY, am_hal_gpio_pincfg_output);
    am_hal_gpio_pinconfig(GPIO_SLEEP_EXIT, am_hal_gpio_pincfg_output);
    am_hal_gpio_pinconfig(GPIO_TIMER_ISR, am_hal_gpio_pincfg_output);

}

void
globalTearDown(void)
{
}

//*****************************************************************************
//
// "No FreeRTOS" Test - this test runs first and does not use FreeRTOS 
//
//*****************************************************************************

void freertos_nortos_test(void)
{
    uint32_t ui32Status = 0;

    //
    // Initialize plotting interface.
    //
    am_util_stdio_printf("\nRunning freertos_nortos_test\n");

    TEST_ASSERT_TRUE(true); 

}

//*****************************************************************************
//
// FreeRTOS Sample Test 
//
//*****************************************************************************

void freertos_sample_test(void)
{
    uint32_t ui32Status = 0;


    //
    // Initialize plotting interface.
    //
    am_util_stdio_printf("\nRunning freertos_sample_test\n");

    // 
    // Output clock configuration info
    //
    am_util_stdio_printf("\nconfigCPU_CLOCK_HZ=%dHz", configCPU_CLOCK_HZ);

    run_tasks();

    //
    // We shouldn't ever get here.
    //
    while (1)
    {
    }
}

//*****************************************************************************
//
// This test won't run because the previous test doesn't return from run_tasks()
//
//*****************************************************************************

void freertos_wontrun_test(void)
{
    uint32_t ui32Status = 0;

    //
    // Initialize plotting interface.
    //
    am_util_stdio_printf("\nRunning freertos_wontrun_test\n");
    am_util_stdio_printf("\nThis test won't run because the previous");
    am_util_stdio_printf("\ntest doesn't return from run_tasks()\n");

    TEST_ASSERT_TRUE(true); 

}
