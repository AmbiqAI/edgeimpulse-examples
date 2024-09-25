//*****************************************************************************
//
//! @file gui_task.c
//!
//! @brief Task to handle GUI operations.
//!
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

//*****************************************************************************
//
// Global includes for this project.
//
//*****************************************************************************
#include "lvgl_benchmark.h"

#include "lv_demo_benchmark.h"

//*****************************************************************************
//
// Macro definitions
//
//*****************************************************************************

//*****************************************************************************
//
// GUI task handle.
//
//*****************************************************************************
TaskHandle_t GuiTaskHandle;

//*****************************************************************************
//
// Texture.
//
//*****************************************************************************


//*****************************************************************************
//
// External variable definitions
//
//*****************************************************************************
extern am_util_stdio_print_char_t g_pfnCharPrint;


//*****************************************************************************
//
// GUI functions
//
//*****************************************************************************


//*****************************************************************************
//
// Task function.
//
//*****************************************************************************
void
GuiTask(void *pvParameters)
{
    am_util_stdio_printf("GUI task start!\n");

#if LV_USE_LOG == 1
    lv_log_register_print_cb((lv_log_print_g_cb_t)g_pfnCharPrint);
#endif

    lv_demo_benchmark_set_max_speed(true);
    lv_demo_benchmark();
    //lv_demo_benchmark_run_scene(40);


    while (1)
    {
        lv_timer_handler();

        //vTaskDelay(1);
    }
}

