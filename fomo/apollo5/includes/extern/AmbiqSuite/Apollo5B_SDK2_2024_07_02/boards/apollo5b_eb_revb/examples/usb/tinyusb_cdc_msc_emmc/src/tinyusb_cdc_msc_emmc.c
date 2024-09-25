//*****************************************************************************
//
//! @file tinyusb_cdc_msc.c
//!
//! @brief tinyusb cdc-acm and mass storage USB example (this example needs eMMC card).
//!
//! This example demonstrates a composite USB device including one USB CDC-ACM
//! and one mass storage device. the COM port will echo back the
//! the input from the terminal and the mass storage device will be disk when
//! connecting to other OS like Windows and Linux.
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

#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include "am_mcu_apollo.h"
#include "am_bsp.h"
#include "am_util.h"

#include "board.h"
#include "tusb.h"
#include "msc_emmc_disc.h"

#define FORMAT_DISK_ERASE       0
#define FORMAT_DISK_FAT12       0

//--------------------------------------------------------------------+
// MACRO CONSTANT TYPEDEF PROTYPES
//--------------------------------------------------------------------+
#define ENABLE_SYSPLL_FOR_HS ((BOARD_DEVICE_RHPORT_SPEED == OPT_MODE_HIGH_SPEED) && defined(AM_PART_APOLLO5B) && !defined(APOLLO5_FPGA))

#ifdef AM_CDC_USE_APP_BUF
// Size for each app buffer in bytes
#ifdef AM_CFG_USB_DMA_MODE_1
    #define APP_BUF_SIZE (32 * 1024)
    #define APP_BUF_COUNT 2
#else //AM_CFG_USB_DMA_MODE_1
    #define APP_BUF_SIZE (CFG_TUD_CDC_RX_BUFSIZE)
    #define APP_BUF_COUNT 3
#endif //AM_CFG_USB_DMA_MODE_1
CFG_TUSB_MEM_ALIGN AM_SHARED_RW uint8_t gBuf[APP_BUF_COUNT][APP_BUF_SIZE] = {0};
uint32_t gBufValidCnt[APP_BUF_COUNT] = {0};
uint8_t gTxPtr = APP_BUF_COUNT - 1;
uint8_t gRxPtr = 0;
#endif //AM_CDC_USE_APP_BUF

const char welcome_text[] = "\r\nTinyUSB CDC MSC eMMC device example\r\n";

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

#if ENABLE_SYSPLL_FOR_HS
static void* gp_syspll_handle = NULL;
#endif

void led_blinking_task(void);
void cdc_task(void);
#if ENABLE_SYSPLL_FOR_HS
static void syspll_init(void);
#endif


extern void format_emmc_filesystem(void);
extern void erase_emmc_disk(void);

/*------------- MAIN -------------*/
int main(void)
{
    board_init();
    emmc_init();

#if FORMAT_DISK_ERASE
    //
    // This will erase hte entire disk
    //
    erase_emmc_disk();
#endif
#if FORMAT_DISK_FAT12
    //
    // If disk not formated, Windows will not read disk
    // Only needs to be oerformed once
    //
    format_emmc_filesystem();
#endif

    #if ENABLE_SYSPLL_FOR_HS
    syspll_init();
    #endif

    tusb_init();

    #ifdef AM_CDC_USE_APP_BUF
    tud_cdc_rx_buf_assign(gBuf[0], sizeof(gBuf[0]));
    #endif //AM_CDC_USE_APP_BUF

    while (1)
    {
        //
        // tinyusb device task
        //
        tud_task();

        //
        // USB CDC Task
        //
        cdc_task();

        //
        // Not implemented
        //
        led_blinking_task();
    }

#if 0   // Avoid compiler warning, unreachable statement
    return 0;
#endif
}

//*********************************************************************
//
//! Device callbacks
//
//*********************************************************************

//*********************************************************************
//
// Invoked when device is mounted
//
//*********************************************************************
AM_USED void
tud_mount_cb(void)
{
    blink_interval_ms = BLINK_MOUNTED;
}

//*********************************************************************
//
// Invoked when device is unmounted
//
//*********************************************************************
AM_USED void
tud_umount_cb(void)
{
    blink_interval_ms = BLINK_NOT_MOUNTED;
}

//*********************************************************************
//
// Invoked when usb bus is suspended
// remote_wakeup_en : if host allow us  to perform remote wakeup
// Within 7ms, device must draw an average of current less than 2.5 mA from bus
//
//*********************************************************************
AM_USED void
tud_suspend_cb(bool remote_wakeup_en)
{
    (void) remote_wakeup_en;
    blink_interval_ms = BLINK_SUSPENDED;
}

//*********************************************************************
//
// Invoked when usb bus is resumed
//
//*********************************************************************
AM_USED void
tud_resume_cb(void)
{
    blink_interval_ms = BLINK_MOUNTED;
}

//*********************************************************************
//
// USB CDC
//
//*********************************************************************
void
cdc_task(void)
{
    #ifdef AM_CDC_USE_APP_BUF
    tud_app_buf_state_t rx_state = tud_cdc_rx_buf_state_get();
    tud_app_buf_state_t tx_state = tud_cdc_tx_buf_state_get();

    // If TX buffer status is completed/idle/bus-reset
    if ( tx_state != TUD_APP_BUF_STATE_BUSY )
    {
        // Clear buffer valid count for current buffer slot
        gBufValidCnt[gTxPtr] = 0;

        // If there is new buffer to transmit
        uint8_t nextTxPtr = ( gTxPtr + 1 ) % APP_BUF_COUNT;
        if ( gBufValidCnt[nextTxPtr] != 0 )
        {
            // Transmit buffer and update TX Buffer pointer to the buffer slot
            // being transmitted
            tud_cdc_tx_buf_assign_send(gBuf[nextTxPtr], gBufValidCnt[nextTxPtr]);
            gTxPtr = nextTxPtr;
        }
    }

    // If current RX buffer has done receiving data
    if ( rx_state == TUD_APP_BUF_STATE_RX_READY )
    {
        uint8_t  nextRxPtr = gRxPtr;
        uint32_t recv_cnt = tud_cdc_rx_buf_recv_count_get();

        // If received count is not zero
        if ( recv_cnt > 0 )
        {
            nextRxPtr = ( gRxPtr + 1 ) % APP_BUF_COUNT;
            // If next buffer is empty
            if ( gBufValidCnt[nextRxPtr] == 0 )
            {
                // Record buffer valid count for current buffer slot
                gBufValidCnt[gRxPtr] = recv_cnt;

                // Increment pointer to the empty slot to start receiving
                gRxPtr = nextRxPtr;

                // Start receiving from USB Host
                tud_cdc_rx_buf_assign(gBuf[gRxPtr], sizeof(gBuf[gRxPtr]));
            }
        }
        // Nothing is received into the buffer. Receive back into same buffer
        else
        {
            // Start receiving from USB Host
            tud_cdc_rx_buf_assign(gBuf[gRxPtr], sizeof(gBuf[gRxPtr]));
        }
    }
    // If buffer reset occurred
    else if ( rx_state == TUD_APP_BUF_STATE_BUF_RESET )
    {
        // re-assign current buffer to receive, discarding data received before
        // buffer reset.
        tud_cdc_rx_buf_assign(gBuf[gRxPtr], sizeof(gBuf[gRxPtr]));
    }
    #else //AM_CDC_USE_APP_BUF

    // connected() check for DTR bit
    // Most but not all terminal client set this when making connection
    // if ( tud_cdc_connected() )
    {
        // connected and there are data available
        uint32_t rx_count = tud_cdc_available();
        uint32_t tx_avail = tud_cdc_write_available();
        if ( (rx_count != 0) && (tx_avail >= rx_count))
        {
            uint8_t buf_rx[CFG_TUD_CDC_RX_BUFSIZE];
            uint32_t count = tud_cdc_read(buf_rx, sizeof(buf_rx));
            tud_cdc_write(buf_rx, count);
            tud_cdc_write_flush();
        }
    }
    #endif //AM_CDC_USE_APP_BUF
}

//*********************************************************************
//
// Invoked when cdc when line state changed e.g connected/disconnected
//
//*********************************************************************
AM_USED void
tud_cdc_line_state_cb(uint8_t itf, bool dtr, bool rts)
{
    (void) itf;
    (void) rts;

    #ifdef AM_CDC_USE_APP_BUF
    if ( dtr )
    {
        tud_app_buf_state_t tx_state = tud_cdc_tx_buf_state_get();
        if ( tx_state != TUD_APP_BUF_STATE_BUSY )
        {
            if ( gBufValidCnt[gTxPtr] == 0 )
            {
                sprintf((char *)gBuf[gTxPtr], "%s", welcome_text);
                tud_cdc_tx_buf_assign_send(gBuf[gTxPtr], sizeof(welcome_text));
            }
        }
    }
    #else //AM_CDC_USE_APP_BUF
    // connected
    if ( dtr )
    {
        // print initial message when connected
        tud_cdc_write_str(welcome_text);
        tud_cdc_write_flush();
    }
    #endif //AM_CDC_USE_APP_BUF
}

//*********************************************************************
//
// Invoked when CDC interface received data from host
//
//*********************************************************************
AM_USED void
tud_cdc_rx_cb(uint8_t itf)
{
    (void) itf;
}


#if ENABLE_SYSPLL_FOR_HS
//*****************************************************************************
//
// Initialization of System PLL for USB usage
//
//*****************************************************************************
static void syspll_init(void)
{
    // start XTAL_HS
    am_hal_mcuctrl_control(AM_HAL_MCUCTRL_CONTROL_EXTCLK32M_KICK_START, 0);
    am_util_delay_ms(200);

    // Initialize System PLL handle
    am_hal_syspll_initialize(0, &gp_syspll_handle);

    // Configure System PLL
    am_hal_syspll_config_t syspll_cfg_usb =  AM_HAL_SYSPLL_DEFAULT_CFG_USB;
    am_hal_syspll_configure(gp_syspll_handle, &syspll_cfg_usb);

    // Enable System PLL
    am_hal_syspll_enable(gp_syspll_handle);

    // Wait for System PLL Lock
    am_hal_syspll_lock_wait(gp_syspll_handle);
}
#endif

//*********************************************************************
//
// BLINKING TASK
//
//*********************************************************************
void
led_blinking_task(void)
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
