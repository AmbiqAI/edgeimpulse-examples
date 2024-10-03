//*****************************************************************************
//
//! @file tinyusb_cdc_hid_on_off.c
//!
//! @brief TinyUSB HID On/Off Example.
//!
//! @addtogroup usb_examples USB Examples
//
//! @defgroup tinyusb_cdc_hid_on_off TinyUSB HID On/Off Example
//! @ingroup usb_examples
//! @{
//!
//! Purpose: This example demonstrates how to use the USB dcd_powerdown()
//! and dcd_powerup() to shutdown the USB subsystem,
//! change the profile, then power back up with that changed profile.
//!
//! This example uses CDC, CDC-composite, and CDC-HID descriptors
//! one of these USB profile is active when the USB enumerates.
//!
//! Some notes: a different USB PID is used for each profile.
//! On an evb, SW2 is used to disconnect,
//! the SW1 is used to advance the usb configuration and re-connect.
//!
//! A PC program like USBDview can be used to watch the configured devices change
//! as the user scrolls through the USB profiles.
//!
//! The actual usb disconnect and reconnect code is in usb_disconnect.c
//! This uses tinyusb callbacks and tinyusb deferred function calls
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

#include "apollo4_board.h"
#include "tusb.h"
#include "usb_disconnect.h"
#include "usb_descriptors.h"

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

static void led_blinking_task(void);
static void cdc_task(void);

//--------------------------------------------------------------------+
// Device callbacks
//--------------------------------------------------------------------+

// ****************************************************************************
// Invoked when device is mounted
// ****************************************************************************
AM_USED void
tud_mount_cb(void)
{
    blink_interval_ms = BLINK_MOUNTED;
}

// ****************************************************************************
//
// Invoked when device is unmounted
//
// ****************************************************************************
AM_USED void
tud_umount_cb(void)
{
    blink_interval_ms = BLINK_NOT_MOUNTED;
}

// ****************************************************************************
//
// Invoked when usb bus is suspended
// remote_wakeup_en: if host allow us  to perform remote wakeup
// Within 7ms, device must draw an average of current less than 2.5 mA from bus
//
// ****************************************************************************
AM_USED void
tud_suspend_cb(bool remote_wakeup_en)
{
    (void) remote_wakeup_en;
    blink_interval_ms = BLINK_SUSPENDED;
}

// ****************************************************************************
//
// Invoked when usb bus is resumed
//
// ****************************************************************************
AM_USED void
tud_resume_cb(void)
{
    blink_interval_ms = BLINK_MOUNTED;
}

// ****************************************************************************
// USB CDC
// ****************************************************************************
static void cdc_task(void)
{
    char buf[64];
    //
    // connected() check for DTR bit
    // Most but not all terminal client set this when making connection
    //
    if ( tud_cdc_connected() )
    {
        //
        // connected and there are data available
        //
        if ( tud_cdc_available() )
        {
            //
            // read and echo back
            //
            uint32_t count = tud_cdc_read(buf, sizeof(buf));

            for ( uint32_t i = 0; i < count; i++ )
            {
                tud_cdc_write_char(buf[i]);

                if ( buf[i] == '\r' )
                {
                    tud_cdc_write_char('\n');
                }
            }

            tud_cdc_write_flush();
        }
    }
}

// ****************************************************************************
//
// Invoked when cdc when line state changed e.g connected/disconnected
//
// ****************************************************************************
AM_USED void
tud_cdc_line_state_cb(uint8_t itf, bool dtr, bool rts)
{
    (void) itf;
    (void) rts;

    // connected
    if ( dtr )
    {
        // print an initial message when connected
        tud_cdc_write_str("\r\nTinyUSB CDC device example\r\n");
        tud_cdc_write_flush();
    }
}

// ****************************************************************************
//
// Invoked when CDC interface received data from host
//
// ****************************************************************************
AM_USED void
tud_cdc_rx_cb(uint8_t itf)
{
  (void) itf;

}

//--------------------------------------------------------------------+
// USB HID
//--------------------------------------------------------------------+

// ****************************************************************************
//
// Invoked when received GET_REPORT control request
// Application must fill buffer report's content and return its length.
// Return zero will cause the stack to STALL request
//
// ****************************************************************************
AM_USED uint16_t
tud_hid_get_report_cb(uint8_t itf,
                      uint8_t report_id,
                      hid_report_type_t report_type,
                      uint8_t* buffer,
                      uint16_t reqlen)
{
    // TODO not Implemented
    (void) itf;
    (void) report_id;
    (void) report_type;
    (void) buffer;
    (void) reqlen;

    return 0;
}

// ****************************************************************************
// Invoked when received SET_REPORT control request or
// received data on OUT endpoint ( Report ID = 0, Type = 0 )
// ****************************************************************************
AM_USED void
tud_hid_set_report_cb(uint8_t itf,
                      uint8_t report_id,
                      hid_report_type_t report_type,
                      const uint8_t *buffer,
                      uint16_t bufsize)
{
    // This example doesn't use multiple report and report ID
    (void) itf;
    (void) report_id;
    (void) report_type;

    // echo back anything we received from host
    tud_hid_report(0, buffer, bufsize);
}

// ****************************************************************************
// BLINKING TASK
// ****************************************************************************
static void led_blinking_task(void)
{
    static uint32_t start_ms = 0;
    static bool led_state = false;

    // Blink every interval ms
    if ( board_millis() - start_ms < blink_interval_ms)
    {
        return; // not enough time
    }
    start_ms += blink_interval_ms;

    board_led_write(led_state);
    led_state = 1 - led_state; // toggle
}

// ****************************************************************************
// main
// ****************************************************************************
int main(void)
{
    board_init("USB CDC HID On/Off Example");
    //
    // setup buttons for USB disconnect/connect events
    //
    usb_disconnect_init();

    //
    // set initial USB profile to simple CDC only
    //
    usb_desc_set_profile(e_PROFILE_CDC);
    tusb_init();

    while (1)
    {
        //
        // scan buttons to disconnect or reconnect USB
        // this also will change the USB profile on each reconnect
        //
        usb_disconnect_periodic(false);
        //
        // tinyusb device task
        //
        tud_task();

        led_blinking_task();

        cdc_task();
    }

#if 0   // Avoid compiler warning, unreachable statement
    return 0;
#endif
}

//*****************************************************************************
//
// End Doxygen group.
//! @}
//
//*****************************************************************************

