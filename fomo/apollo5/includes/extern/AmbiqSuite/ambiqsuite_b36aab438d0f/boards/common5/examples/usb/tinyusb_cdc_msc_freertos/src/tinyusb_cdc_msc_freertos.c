//*****************************************************************************
//
//! @file tinyusb_cdc_msc_freertos.c
//!
//! @brief tinyusb cdc-acm and mass storage FreeRTOS USB example.
//!
//! This example demonstrates a composite USB device including one USB CDC-ACM
//! and one mass storage device. the COM port will echo back the
//! the input from the terminal and the mass storage device will be disk when
//! connecting to other OS like Windows and Linux. The CDC-ACM and Mass storage
//! functions will be handled by two separate FreeRTOS tasks.
//!
//!
//*****************************************************************************

//*****************************************************************************
//
// ${copyright}
//
// This is part of revision ${version} of the AmbiqSuite Development Package.
//
//*****************************************************************************

/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2019 Ha Thach (tinyusb.org)
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 */

//*****************************************************************************
//
// ${copyright}
//
// This is part of revision ${version} of the AmbiqSuite Development Package.
//
//*****************************************************************************

//*****************************************************************************
//
// Global includes for this project.
//
//*****************************************************************************
#include "tinyusb_cdc_msc_freertos.h"

#define ENABLE_SYSPLL_FOR_HS ((BOARD_DEVICE_RHPORT_SPEED == OPT_MODE_HIGH_SPEED) && defined(AM_PART_APOLLO5B) && !defined(APOLLO5_FPGA))

//*****************************************************************************
//
// Main Function
//
//*****************************************************************************
int main(void)
{
    board_init();

    #if ENABLE_SYSPLL_FOR_HS
    am_hal_clkmgr_clock_config(AM_HAL_CLKMGR_CLK_ID_SYSPLL, AM_HAL_USB_PHY_CLK_FREQ_HZ, NULL);
    #endif // ENABLE_SYSPLL_FOR_HS

    NVIC_SetPriority(USB0_IRQn, USB_ISR_PRIORITY);

    //
    // Initialize plotting interface.
    //
    am_util_stdio_printf("tinyusb cdc msc freertos.\n");

    //
    // Run the application.
    //
    run_tasks();

    //
    // We shouldn't ever get here.
    //
    while (1)
    {
    }
}
