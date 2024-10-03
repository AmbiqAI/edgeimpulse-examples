//*****************************************************************************
//
//! @file mve_workload_test_cases.c
//!
//! @brief Example test case program which demonstrates using freeRTOS
//!        in conjunction with the unity test framework, to showcase
//!        the mve_task which exercises the Helium/MVE matrix multiplication
//!        engine.  This task can be ported to other stress test cases in order
//!        to stress the MVE.
//!
//!        NOTE: When porting the mve_task to other projects, it is necessary
//!        to include the libcmsis-nn.a precompiled library and Include files.
//!        See config-template.ini for details.
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
//!           8         5     on each MVE task entry
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
#include "mve_workload_test_cases.h"

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
    am_hal_gpio_state_write(GPIO_MVE_TASK, AM_HAL_GPIO_OUTPUT_CLEAR);

    // Initialize status GPIOs as output
    am_hal_gpio_pinconfig(GPIO_HELLO_TASK, am_hal_gpio_pincfg_output);
    am_hal_gpio_pinconfig(GPIO_IDLE_TASK, am_hal_gpio_pincfg_output);
    am_hal_gpio_pinconfig(GPIO_SLEEP_ENTRY, am_hal_gpio_pincfg_output);
    am_hal_gpio_pinconfig(GPIO_SLEEP_EXIT, am_hal_gpio_pincfg_output);
    am_hal_gpio_pinconfig(GPIO_TIMER_ISR, am_hal_gpio_pincfg_output);
    am_hal_gpio_pinconfig(GPIO_MVE_TASK, am_hal_gpio_pincfg_output);

}

void
globalTearDown(void)
{
}

//*****************************************************************************
//
// FreeRTOS Sample Test 
//
//*****************************************************************************

void mve_workload_test(void)
{
    uint32_t ui32Status = 0;


    //
    // Initialize plotting interface.
    //
    am_util_stdio_printf("\nRunning mve_workload_test\n");

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
