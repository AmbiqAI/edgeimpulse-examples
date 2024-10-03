//*****************************************************************************
//
//! @file tinyusb_cdc.c
//!
//! @brief tinyusb cdc-acm example.
//!
//! @addtogroup usb_examples USB Examples
//
//! @defgroup tinyusb_cdc TinyUSB CDC ACM Example
//! @ingroup usb_examples
//! @{
//!
//! Purpose: This example demonstrates how to use the USB CDC-ACM device class. it will
//! echo back the the input from the terminal tool.
//
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

#include "dcd_apollo4.h"
#include "board.h"
#include "tusb.h"

#define ENABLE_INSERT_NL_AFTER_CR 1

//--------------------------------------------------------------------+
// MACRO CONSTANT TYPEDEF PROTYPES
//--------------------------------------------------------------------+

/* Blink pattern
 * - 250 ms  : device not mounted
 * - 1000 ms : device mounted
 * - 2500 ms : device is suspended
 */
enum
{
    BLINK_NOT_MOUNTED = 250,
    BLINK_MOUNTED = 1000,
    BLINK_SUSPENDED = 2500,
};

static uint32_t blink_interval_ms = BLINK_NOT_MOUNTED;

void led_blinking_task(void);

void cdc_task(void);

// ****************************************************************************
//
//! @note For Keil compilers, any Tinyusb callbacks that are prototyped as TU_ATTR_WEAK
//! will be optimized out by the linker. To force the linker to include these functions,
//! simply put the attribute 'used" in front of the function. This is accomplished with
//! the macro AM_USED
//
// ****************************************************************************

//--------------------------------------------------------------------+
//  MAIN
//--------------------------------------------------------------------+
int main(void)
{
    board_init();
    tusb_init();

    while (1)
    {
        tud_task(); // tinyusb device task
        led_blinking_task();

        cdc_task();
    }

}

//--------------------------------------------------------------------+
// Device callbacks
//--------------------------------------------------------------------+
//
// Invoked when device is mounted
//
 AM_USED void tud_mount_cb(void)
{
    blink_interval_ms = BLINK_MOUNTED;

}

#ifndef TUSB_ADDED_FUNCTIONS
//
// Invoked when device is unmounted
//
AM_USED void tud_umount_cb(void)
{
    blink_interval_ms = BLINK_NOT_MOUNTED;
}
#endif

//--------------------------------------------------------------------+
// Invoked when usb bus is suspended
// remote_wakeup_en : if host allow us  to perform remote wakeup
// Within 7ms, device must draw an average of current less than 2.5 mA from bus
//--------------------------------------------------------------------+
AM_USED void tud_suspend_cb(bool remote_wakeup_en)
{
    (void) remote_wakeup_en;
    blink_interval_ms = BLINK_SUSPENDED;
}

//--------------------------------------------------------------------+
// Invoked when usb bus is resumed
//--------------------------------------------------------------------+
AM_USED void tud_resume_cb(void)
{
    blink_interval_ms = BLINK_MOUNTED;
}

//--------------------------------------------------------------------+
// USB CDC
//--------------------------------------------------------------------+
void cdc_task(void)
{
    // connected() check for DTR bit
    // Most but not all terminal client set this when making connection
    // if ( tud_cdc_connected() )
    {
        // Check for rx and tx buffer watermark
        uint32_t rx_count = tud_cdc_available();
        uint32_t tx_avail = tud_cdc_write_available();

        // If data received and there is enough tx buffer to echo back. Else
        // keep rx FIFO and wait for next service.
        if ((rx_count != 0) && (tx_avail >= rx_count))
        {
            uint8_t buf_rx[CFG_TUD_CDC_RX_BUFSIZE];
            #if ENABLE_INSERT_NL_AFTER_CR
            uint8_t buf_tx[CFG_TUD_CDC_TX_BUFSIZE];
            uint32_t tx_idx = 0;
            #endif

            // read and echo back
            uint32_t count = tud_cdc_read(buf_rx, sizeof(buf_rx));

            #if ENABLE_INSERT_NL_AFTER_CR
            uint32_t tx_surplus = tx_avail - rx_count;
            for (uint32_t i = 0; i < count; i++)
            {
                buf_tx[tx_idx++] = buf_rx[i];
                if ( (buf_rx[i] == '\r') && tx_surplus )
                {
                    buf_tx[tx_idx++] = '\n';
                    tx_surplus--;
                }
            }
            tud_cdc_write(buf_tx, tx_idx);
            #else
            tud_cdc_write(buf_rx, count);
            #endif
            tud_cdc_write_flush();
        }
    }
}

//--------------------------------------------------------------------+
// Invoked when cdc when line state changed e.g connected/disconnected
//--------------------------------------------------------------------+
AM_USED void tud_cdc_line_state_cb(uint8_t itf, bool dtr, bool rts)
{
    (void) itf;
    (void) rts;

    // connected
    if ( dtr )
    {
        // print initial message when connected
        tud_cdc_write_str("\r\nTinyUSB CDC device example\r\n");
        tud_cdc_write_flush();
    }
}

//--------------------------------------------------------------------+
// Invoked when CDC interface received data from host
//--------------------------------------------------------------------+
AM_USED void tud_cdc_rx_cb(uint8_t itf)
{
    cdc_task();
}

//--------------------------------------------------------------------+
//
//--------------------------------------------------------------------+
AM_USED void tud_cdc_tx_complete_cb(uint8_t itf)
{
    cdc_task();
}

//--------------------------------------------------------------------+
// BLINKING TASK
//--------------------------------------------------------------------+
void led_blinking_task(void)
{
    static uint32_t start_ms = 0;
    static bool led_state = false;

    // Blink every interval ms
    if ( (board_millis() - start_ms) < blink_interval_ms)
    {
        return; // not enough time
    }
    start_ms += blink_interval_ms;

    board_led_write(led_state);
    led_state = 1 - led_state; // toggle
}

//*****************************************************************************
//
// End Doxygen group.
//! @}
//
//*****************************************************************************

