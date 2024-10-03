//*****************************************************************************
//
//! @file hid_controller.c
//!
//! @brief Provides APIs for host and device to communicate via HID
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

#include "usb_descriptors.h"
#include "hid_controller.h"

//--------------------------------------------------------------------+
// Structure definitions
//--------------------------------------------------------------------+
typedef struct
{
    hid_rx_cb rx_req_cb;
    hid_tx_cmpl_cb tx_cmpl_cb;
    void        *rx_req_param;
    void        *tx_cmpl_param;
} hid_parameter_t;

static hid_parameter_t hid_parameter = {
    .rx_req_cb    = NULL,
    .tx_cmpl_cb = NULL,
    .rx_req_param = NULL,
    .tx_cmpl_param = NULL,
};

//--------------------------------------------------------------------+
// USB HID
//--------------------------------------------------------------------+

//--------------------------------------------------------------------+
// Invoked when received GET_REPORT control request
// Application must fill buffer report's content and return its length.
// Return zero will cause the stack to STALL request
//--------------------------------------------------------------------+
uint16_t tud_hid_get_report_cb(uint8_t itf, uint8_t report_id, hid_report_type_t report_type, uint8_t* buffer, uint16_t reqlen)
{
    // TODO not Implemented
    (void) itf;
    (void) report_id;
    (void) report_type;
    (void) buffer;
    (void) reqlen;

    return 0;
}

// Invoked when received SET_REPORT control request or
// received data on OUT endpoint ( Report ID = 0, Type = 0 )
void tud_hid_set_report_cb(uint8_t itf, uint8_t report_id, hid_report_type_t report_type, uint8_t const* buffer, uint16_t bufsize)
{
    // This example doesn't use multiple report and report ID
    (void) itf;
    (void) report_id;
    (void) report_type;

    if (hid_parameter.rx_req_cb)
    {
        hid_parameter.rx_req_cb(buffer, bufsize,
                                    hid_parameter.rx_req_param);
    }
}

// Invoked when sent REPORT successfully to host
void tud_hid_report_complete_cb(uint8_t instance, uint8_t const* report, uint16_t len)
{
    (void) instance;
    (void) report;
    // am_util_debug_printf(
    //     "Hid has sended %d bytes\n",
    //     len);

    if (hid_parameter.tx_cmpl_cb && len)
    {
        hid_parameter.tx_cmpl_cb(len, hid_parameter.tx_cmpl_param);
    }
}

bool is_hid_ready(void)
{
    return tud_hid_n_ready(0);
}

bool hid_send_data(const uint8_t *buf, uint32_t bufsize)
{
    // devcie send data to host through HID.
    return tud_hid_report(0, buf, bufsize);
}

void hid_register_req_cb(hid_rx_cb cb, void *param)
{
    hid_parameter.rx_req_cb    = cb;
    hid_parameter.rx_req_param = param;
}

void hid_register_tx_cmpl_cb(hid_tx_cmpl_cb cb, void *param)
{
    hid_parameter.tx_cmpl_cb    = cb;
    hid_parameter.tx_cmpl_param = param;
}
