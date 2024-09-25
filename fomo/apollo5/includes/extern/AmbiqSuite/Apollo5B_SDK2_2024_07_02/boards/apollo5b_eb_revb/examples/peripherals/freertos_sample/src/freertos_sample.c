//*****************************************************************************
//
//! @file freertos_sample.c
//!
//! @brief Example program which demonstrates using freeRTOS
//!
//!        The following GPIOs are configured for observation:
//!
//!        Apollo5a Turbo FMC
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
// Copyright (c) 2024, Ambiq Micro, Inc.
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// 1. Redistributions of source code must retain the above copyright notice,
// this list of conditions and the following disclaimer.
//
// 2. Redistributions in binary form must reproduce the above copyright
// notice, this list of conditions and the following disclaimer in the
// documentation and/or other materials provided with the distribution.
//
// 3. Neither the name of the copyright holder nor the names of its
// contributors may be used to endorse or promote products derived from this
// software without specific prior written permission.
//
// Third party software included in this distribution is subject to the
// additional license terms as defined in the /docs/licenses directory.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
// ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
// LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
// CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
// SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
// INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
// CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
// ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
// POSSIBILITY OF SUCH DAMAGE.
//
// This is part of revision release_a5b_sdk2-748191cd0 of the AmbiqSuite Development Package.
//
//*****************************************************************************

#include "am_mcu_apollo.h"
#include "am_util.h"
#include "freertos_sample.h"

//*****************************************************************************
//
// Global Variables
//
//*****************************************************************************

//*****************************************************************************
//
// FreeRTOS Sample Test
//
//*****************************************************************************

int main(void)
{

    //
    // Clear status GPIOs
    //
    am_hal_gpio_state_write(GPIO_HELLO_TASK, AM_HAL_GPIO_OUTPUT_CLEAR);
    am_hal_gpio_state_write(GPIO_IDLE_TASK, AM_HAL_GPIO_OUTPUT_CLEAR);
    am_hal_gpio_state_write(GPIO_SLEEP_ENTRY, AM_HAL_GPIO_OUTPUT_CLEAR);
    am_hal_gpio_state_write(GPIO_SLEEP_EXIT, AM_HAL_GPIO_OUTPUT_CLEAR);
    am_hal_gpio_state_write(GPIO_TIMER_ISR, AM_HAL_GPIO_OUTPUT_CLEAR);

    //
    // Initialize status GPIOs as output
    //
    am_hal_gpio_pinconfig(GPIO_HELLO_TASK, am_hal_gpio_pincfg_output);
    am_hal_gpio_pinconfig(GPIO_IDLE_TASK, am_hal_gpio_pincfg_output);
    am_hal_gpio_pinconfig(GPIO_SLEEP_ENTRY, am_hal_gpio_pincfg_output);
    am_hal_gpio_pinconfig(GPIO_SLEEP_EXIT, am_hal_gpio_pincfg_output);
    am_hal_gpio_pinconfig(GPIO_TIMER_ISR, am_hal_gpio_pincfg_output);

    //
    // Configure the board for low power operation.
    //
    am_bsp_low_power_init();

    //
    //  Enable the I-Cache and D-Cache.
    //
    am_hal_cachectrl_icache_enable();
    am_hal_cachectrl_dcache_enable(true);

    //
    // Initialize the printf interface for ITM output
    //
    if (am_bsp_debug_printf_enable())
    {
        // Cannot print - so no point proceeding
        while(1);
    }

    //
    // Initialize plotting interface.
    //
    am_util_stdio_printf("\nRunning freertos_sample\n");

    //
    // Start the tasks.
    //
    run_tasks();

    //
    // We shouldn't ever get here.
    //
    while (1)
    {
    }
}
