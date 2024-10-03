//*****************************************************************************
//
//! @file tinyusb_cdc_dual_ports.c
//!
//! @brief tinyusb two cdc-acm composite USB example.
//!
//! This example demonstrates how to add two USB CDC-ACM COM ports. it will echo
//! back the the input from one terminal and copy the input to another terminal
//! at the same time.
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
#include <ctype.h>

#include "board.h"
#include "tusb.h"
#include "am_util_delay.h"

#define ENABLE_SYSPLL_FOR_HS ((BOARD_DEVICE_RHPORT_SPEED == OPT_MODE_HIGH_SPEED) && defined(AM_PART_APOLLO5B) && !defined(APOLLO5_FPGA))


//------------- prototypes -------------//
static void cdc_task(void);

/*------------- MAIN -------------*/
int main(void)
{
    board_init();

    #if ENABLE_SYSPLL_FOR_HS
    am_hal_clkmgr_clock_config(AM_HAL_CLKMGR_CLK_ID_SYSPLL, AM_HAL_USB_PHY_CLK_FREQ_HZ, NULL);
    #endif // ENABLE_SYSPLL_FOR_HS

    tusb_init();

    while (1)
    {
        tud_task(); // tinyusb device task
        cdc_task();
    }

#if 0   // Avoid compiler warning, unreachable statement
    return 0;
#endif
}

// echo to either Serial0 or Serial1
// with Serial0 as all lower case, Serial1 as all upper case
static void echo_serial_port(uint8_t itf, uint8_t buf[], uint32_t count)
{
    for ( uint32_t i = 0; i < count; i++ )
    {
        if (itf == 0)
        {
            // echo back 1st port as lower case
            if (isupper(buf[i]))
            {
                buf[i] += 'a' - 'A';
            }
        }
        else
        {
            // echo back additional ports as upper case
            if (islower(buf[i]))
            {
                buf[i] -= 'a' - 'A';
            }
        }

        tud_cdc_n_write_char(itf, buf[i]);

        if ( buf[i] == '\r' )
        {
            tud_cdc_n_write_char(itf, '\n');
        }
    }
    tud_cdc_n_write_flush(itf);
}

//--------------------------------------------------------------------+
// USB CDC
//--------------------------------------------------------------------+
static void cdc_task(void)
{
    uint8_t itf;

    for (itf = 0; itf < CFG_TUD_CDC; itf++)
    {
        // connected() check for DTR bit
        // Most but not all terminal client set this when making connection
        if ( tud_cdc_n_connected(itf) )
        {
            uint32_t rx_count = tud_cdc_n_available(itf);
            uint32_t tx_avail_0 = tud_cdc_n_write_available(0);
            uint32_t tx_avail_1 = tud_cdc_n_write_available(1);

             if ((rx_count != 0) && (tx_avail_0 >= rx_count) && (tx_avail_1 >= rx_count))
            {
                uint8_t buf[64];

                uint32_t count = tud_cdc_n_read(itf, buf, sizeof(buf));

                // echo back to both serial ports
                echo_serial_port(0, buf, count);
                echo_serial_port(1, buf, count);
            }
        }
    }
}
