//*****************************************************************************
//
//! @file hello_task.c
//!
//! @brief Simple task with a while() loop that implements a vTaskDelay of
//!     1 second on every iteration.  After HELLO_TASK_COUNT iterations (set
//!     in src/freertos_sample_test_cases.h), the task breaks out of the while(1)
//!     loop, sets bHelloTaskComplete to "true", and suspends/deletes itself.
//!     The bHelloTaskComplete variable is monitored by the ApplicationIdleTask.
//!
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
#include "freertos_sample.h"
#include "hello_task.h"

//*****************************************************************************
//
// Hello task handle.
//
//*****************************************************************************
TaskHandle_t hello_task_handle;

//*****************************************************************************
//
// Global variables and templates
//
//*****************************************************************************
volatile uint32_t g_ui32IdleCycleCount = 0;
volatile bool g_bHelloTaskComplete = false;

//*****************************************************************************
//
// Perform initial setup for the hello task.
//
//*****************************************************************************
void
HelloTaskSetup(void)
{
    am_util_stdio_printf("Hello task: setup\r\n");
}

//*****************************************************************************
//
// HelloTask - will delete itself after HELLO_TASK_COUNT loops complete
//
//*****************************************************************************

void
HelloTask(void *pvParameters)
{
    uint32_t i = 0;
    const TickType_t xDelay = 1000 / portTICK_PERIOD_MS; // 1 second delay
    am_util_stdio_printf("\r\nRunning HelloTask\r\n");

    while (i < HELLO_TASK_COUNT)
    {
        // Toggle an observation GPIO
        am_hal_gpio_output_toggle(GPIO_HELLO_TASK);
        // Print out number of iterations
        am_util_stdio_printf("\r\nHelloTask Iteration: %d\r\n", i++);
        // Print out number of Idle Cycles since the last iteration
        am_util_stdio_printf("HelloTask Idle Cycle Count: %d\r\n", g_ui32IdleCycleCount);
        // Reset the Idle Cycle Count
        g_ui32IdleCycleCount = 0;
        // Delay (block) the task for a specified period
        vTaskDelay(xDelay);
    }
    // Indicate the HelloTask loops are complete
    g_bHelloTaskComplete = true;
    // Suspend and delete the task
    vTaskDelete(NULL);
}
