//*****************************************************************************
//
//! @file webusb_controller.c
//!
//! @brief The APIs of webUSB to communicate with Audio Tool.
//!
//! This example demonstrates how to use the USB CDC-ACM device class. it will
//! echo back the the input from the terminal tool.
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

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "am_util_debug.h"

#include "tusb.h"
#include "usb_task.h"

#include "usb_descriptors.h"
#include "webusb_controller.h"

//--------------------------------------------------------------------+
// Structure definitions
//--------------------------------------------------------------------+
typedef struct
{
    webusb_rx_cb rx_msg_cb;
    webusb_rx_cb rx_raw_cb;
    void        *rx_msg_param;
    void        *rx_raw_param;
} webusb_parameter_t;

//--------------------------------------------------------------------+
// MACRO CONSTANT TYPEDEF PROTYPES
//--------------------------------------------------------------------+
#define BYTES_TO_UINT16(n, p)                                                  \
    {                                                                          \
        n = (((uint16_t)(p)[0] << 8) + (uint16_t)(p)[1]);                      \
    }
#define BSTREAM_TO_UINT16(n, p)                                                \
    {                                                                          \
        BYTES_TO_UINT16(n, p);                                                 \
        p += 2;                                                                \
    }

// WebUSB connections status
extern bool webusb_connected;
extern void dcd_deInit(uint8_t rhport);

#define REWRIT_NUMBER                         3
// The max of USB2.0 bulk type packet size is 512 in High Speed.
static uint8_t rx_buf[512];

static webusb_parameter_t webusb_parameter = {
    .rx_msg_cb    = NULL,
    .rx_raw_cb    = NULL,
    .rx_msg_param = NULL,
    .rx_raw_param = NULL,
};

// The type of frame received via WebUSB
enum
{
    RX_INVALID   = 0,
    RX_ACORE_MSG = 1, // Audio core message
    RX_RAW_DATA  = 2, // Raw data
    RX_DISABLE_USB  = 3, // Disable USB
};

// ****************************************************************************
//
//! The functions in the array below are prototyped as TU_ATTR_WEAK in Tinyusb.
//! The linkers were not always including these functions in the executable.
//! Using this array forces the linker to keep the function.
//!
//! In summary,the Tinyusb functions prototyped as weak (macro TU_ATTR_WEAK)
//! and implemented in the user code should be added to the array below.
//
// ****************************************************************************
volatile const void *pTUSB_WeakFcnPointers[] = {
    (void *)tud_mount_cb,
#ifndef TUSB_ADDED_FUNCTIONS
    (void *)tud_umount_cb,
#endif
    (void *)tud_suspend_cb, (void *)tud_resume_cb, (void *)tud_vendor_rx_cb,
};

//--------------------------------------------------------------------+
// Device callbacks
//--------------------------------------------------------------------+

// Invoked when device is mounted
void tud_mount_cb(void)
{
    webusb_connected = false;
}

#ifndef TUSB_ADDED_FUNCTIONS
// Invoked when device is unmounted
void tud_umount_cb(void)
{
    webusb_connected = false;
}
#endif

// Invoked when usb bus is suspended
// remote_wakeup_en : if host allow us  to perform remote wakeup
// Within 7ms, device must draw an average of current less than 2.5 mA from bus
void tud_suspend_cb(bool remote_wakeup_en)
{
    (void)remote_wakeup_en;
    webusb_connected = false;
}

// Invoked when usb bus is resumed
void tud_resume_cb(void)
{
    webusb_connected = false;
}

// Invoked when received new data
void tud_vendor_rx_cb(uint8_t itf)
{
    (void)itf;
    uint32_t bytes_rx = 0;

    // check whether the remain size of RX ring-buffer is zero
    while (tud_vendor_available())
    {
        uint16_t frame_header = 0;
        uint8_t *p            = rx_buf;

        bytes_rx = tud_vendor_read(rx_buf, sizeof(rx_buf));
        // The size of frame header is 2.
        BSTREAM_TO_UINT16(frame_header, p);

        // Skip frame header, then transfer buffer to callback function
        switch (frame_header)
        {
            case RX_ACORE_MSG:
                if (webusb_parameter.rx_msg_cb)
                {
                    webusb_parameter.rx_msg_cb(p, bytes_rx - sizeof(frame_header),
                                               webusb_parameter.rx_msg_param);
                }
                break;

            case RX_RAW_DATA:
                if (webusb_parameter.rx_raw_cb)
                {
                    webusb_parameter.rx_raw_cb(p, bytes_rx - sizeof(frame_header),
                                               webusb_parameter.rx_raw_param);
                }
                break;

            case RX_DISABLE_USB:
                usb_task_teardown();
                break;
            default:
                am_util_debug_printf(
                    "Error: Unsupported type of Rx Frame Header: %d\n",
                    frame_header);
                break;
        }
    }
}

uint32_t webusb_send_data(uint8_t *buf, uint32_t bufsize)
{
    uint32_t bytes_tx  = 0;
    uint32_t bufremain = bufsize;

    if (webusb_connected && buf)
    {
        int i = 0;
        do
        {
            // When buf can't be written all at once, write again
            // until the upper limit of rewrite numbers is reached or buf is
            // written all.
            if (i == REWRIT_NUMBER)
            {
                break;
            }

            bytes_tx = tud_vendor_write((void *)(buf + bufsize - bufremain),
                                        bufremain);
            bufremain -= bytes_tx;

            i++;
        } while (bufremain);
    }

    if (bufremain)
    {
        // Collects the amount of data that has not been written
    }

    return bufsize - bufremain;
}

uint32_t webusb_send_data_flush(uint8_t *buf, uint32_t bufsize)
{
    uint32_t bytes_tx = 0;

    bytes_tx = tud_vendor_write((void *)buf, bufsize);
    tud_vendor_write_flush();

    return bytes_tx;
}

void webusb_task(void)
{
    tud_task();
}

void webusb_init(void)
{
    // init tinyusb
    tusb_init();
}

void webusb_deinit(void)
{
    // shutdown and powerdown the usb
    dcd_deInit(0);
}

void webusb_register_msg_cb(webusb_rx_cb cb, void *param)
{
    webusb_parameter.rx_msg_cb    = cb;
    webusb_parameter.rx_msg_param = param;
}

void webusb_register_raw_cb(webusb_rx_cb cb, void *param)
{
    webusb_parameter.rx_raw_cb    = cb;
    webusb_parameter.rx_raw_param = param;
}
