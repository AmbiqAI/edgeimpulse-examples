//*****************************************************************************
//
//! @file tinyusb_cdc.c
//!
//! @brief tinyusb cdc-acm example.
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

#include "board.h"
#include "tusb.h"

// #### INTERNAL BEGIN ####
#define ENABLE_HP_FOR_TEST       0
// #### INTERNAL END ####

#define ENABLE_SYSPLL_FOR_HS ((BOARD_DEVICE_RHPORT_SPEED == OPT_MODE_HIGH_SPEED) && defined(AM_PART_APOLLO5B) && !defined(APOLLO5_FPGA))

// #define CDC_OUT_SPEED_TEST_ONLY
// #define CDC_IN_SPEED_TEST_ONLY

#ifdef AM_CDC_USE_APP_BUF
// Size for each app buffer in bytes
#ifdef AM_CFG_USB_DMA_MODE_1
    #define APP_BUF_SIZE (32 * 1024)
    #define APP_BUF_COUNT 2
#else
    #define APP_BUF_SIZE (CFG_TUD_CDC_RX_BUFSIZE)
    #define APP_BUF_COUNT 3
#endif
CFG_TUSB_MEM_ALIGN AM_SHARED_RW uint8_t gBuf[APP_BUF_COUNT][APP_BUF_SIZE] = {0};
uint32_t gBufValidCnt[APP_BUF_COUNT] = {0};
uint8_t gTxPtr = APP_BUF_COUNT - 1;
uint8_t gRxPtr = 0;
#else //AM_CDC_USE_APP_BUF
CFG_TUSB_MEM_ALIGN uint8_t gBuf[CFG_TUD_CDC_RX_BUFSIZE] = {0};
#endif //AM_CDC_USE_APP_BUF

const char welcome_text[] = "\r\nTinyUSB CDC device example\r\n";

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
//! The functions in the array below are prototyped as TU_ATTR_WEAK in Tinyusb.
//! The linkers were not always including these functions in the executable.
//! Using this array forces the linker to keep the function.
//!
//! In summary,the Tinyusb functions prototyped as weak (macro TU_ATTR_WEAK)
//! and implemented in the user code should be added to the array below.
//
// ****************************************************************************
volatile const void *pTUSB_WeakFcnPointers[] =
{
    (void *)tud_mount_cb,
#ifndef TUSB_ADDED_FUNCTIONS
    (void *)tud_umount_cb,
#endif
    (void *)tud_suspend_cb,
    (void *)tud_resume_cb,
    (void *)tud_cdc_line_state_cb,
    (void *)tud_cdc_rx_cb,
};


/*------------- MAIN -------------*/
int main(void)
{
    board_init();
    // #### INTERNAL BEGIN ####
    #if ENABLE_HP_FOR_TEST
    if ( am_hal_pwrctrl_mcu_mode_select(AM_HAL_PWRCTRL_MCU_MODE_HIGH_PERFORMANCE) != AM_HAL_STATUS_SUCCESS )
    {
        am_util_stdio_printf("Enter HP mode failed!\n");
    }
    #endif
    // #### INTERNAL END ####

    #if ENABLE_SYSPLL_FOR_HS
    am_hal_clkmgr_clock_config(AM_HAL_CLKMGR_CLK_ID_SYSPLL, AM_HAL_USB_PHY_CLK_FREQ_HZ, NULL);
    #endif // ENABLE_SYSPLL_FOR_HS

    tusb_init();

    #ifdef AM_CDC_USE_APP_BUF
    for (uint32_t j = 0; j < (sizeof(gBuf[0]) / 512); j++)
    {
        for (uint32_t i = 0; i < 512; i++)
        {
            gBuf[0][j * 512 + i] = j % 256;
            #if APP_BUF_COUNT > 1
            gBuf[1][j * 512 + i] = j % 256;
            #endif
            #if APP_BUF_COUNT > 2
            gBuf[2][j * 512 + i] = j % 256;
            #endif
        }
    }
    tud_cdc_rx_buf_assign(gBuf[0], sizeof(gBuf[0]));
    #else //AM_CDC_USE_APP_BUF
    for (uint32_t i = 0; i < CFG_TUD_CDC_RX_BUFSIZE; i++)
    {
        gBuf[i] = i % 256;
    }
    #endif //AM_CDC_USE_APP_BUF


    while (1)
    {
        tud_task(); // tinyusb device task

        led_blinking_task();

        cdc_task();
    }

    #if 0   // Avoid compiler warning, unreachable statement
    return 0;
    #endif
}

//--------------------------------------------------------------------+
// Device callbacks
//--------------------------------------------------------------------+

// Invoked when device is mounted
AM_USED void tud_mount_cb(void)
{
    blink_interval_ms = BLINK_MOUNTED;
}

// Invoked when device is unmounted
AM_USED void tud_umount_cb(void)
{
    blink_interval_ms = BLINK_NOT_MOUNTED;
}

// Invoked when usb bus is suspended
// remote_wakeup_en : if host allow us  to perform remote wakeup
// Within 7ms, device must draw an average of current less than 2.5 mA from bus
AM_USED void tud_suspend_cb(bool remote_wakeup_en)
{
    (void) remote_wakeup_en;
    blink_interval_ms = BLINK_SUSPENDED;
}

// Invoked when usb bus is resumed
AM_USED void tud_resume_cb(void)
{
    blink_interval_ms = BLINK_MOUNTED;
}

// Invoked when CDC interface received data from host
AM_USED void tud_cdc_rx_cb(uint8_t itf)
{
    #ifndef CDC_IN_SPEED_TEST_ONLY
    cdc_task();
    #endif
}

// Invoked when a TX is complete and therefore space becomes available in TX buffer
AM_USED void tud_cdc_tx_complete_cb(uint8_t itf)
{
    #ifndef CDC_OUT_SPEED_TEST_ONLY
    cdc_task();
    #endif
}

//--------------------------------------------------------------------+
// USB CDC
//--------------------------------------------------------------------+
#if defined(CDC_IN_SPEED_TEST_ONLY)
#define IN_SPEED_TEST_START_DELAY_PER_LOOP (10)
#define IN_SPEED_TEST_START_DELAY_LOOP_CNT (100)
bool bStartTest = false;
uint32_t ui32StartTestDelay = 0;
void cdc_task(void)
{
    if ( !bStartTest )
    {
        ui32StartTestDelay = 0;
        return;
    }
    else if ( ui32StartTestDelay < IN_SPEED_TEST_START_DELAY_LOOP_CNT )
    {
        ui32StartTestDelay ++;
        am_util_delay_ms(IN_SPEED_TEST_START_DELAY_PER_LOOP);
        return;
    }

    #ifdef AM_CDC_USE_APP_BUF
    tud_app_buf_state_t tx_state = tud_cdc_tx_buf_state_get();
    // If TX Buffer status is completed/idle/bus-reset
    if ( (tx_state != TUD_APP_BUF_STATE_BUSY) && bStartTest)
    {
        // Assign buffer to send to USB host
        tud_cdc_tx_buf_assign_send(gBuf[0], sizeof(gBuf[0]));
    }
    #else //AM_CDC_USE_APP_BUF
    uint32_t tx_avail = tud_cdc_write_available();
    if ( tx_avail && bStartTest )
    {
        tud_cdc_write(gBuf, tx_avail);
        tud_cdc_write_flush();
    }
    #endif //AM_CDC_USE_APP_BUF
}

// Invoked when cdc when line state changed e.g connected/disconnected
AM_USED void tud_cdc_line_state_cb(uint8_t itf, bool dtr, bool rts)
{
    if ( dtr )
    {
        bStartTest = true;
    }
    else
    {
        bStartTest = false;
    }
}

#elif defined(CDC_OUT_SPEED_TEST_ONLY)

void cdc_task(void)
{
    #ifdef AM_CDC_USE_APP_BUF
    tud_app_buf_state_t rx_state = tud_cdc_rx_buf_state_get();

    // If received buffer ready or buffer reset occurred
    if ( (rx_state == TUD_APP_BUF_STATE_RX_READY) || (rx_state == TUD_APP_BUF_STATE_BUF_RESET) )
    {
        // discard received data and assign new buffer
        tud_cdc_rx_buf_assign(gBuf[0], sizeof(gBuf[0]));
    }
    #else //AM_CDC_USE_APP_BUF
    uint32_t rx_count = tud_cdc_available();
    if ( rx_count )
    {
        tud_cdc_read(gBuf, sizeof(gBuf));
    }
    #endif //AM_CDC_USE_APP_BUF
}

// Invoked when cdc when line state changed e.g connected/disconnected
AM_USED void tud_cdc_line_state_cb(uint8_t itf, bool dtr, bool rts)
{

}

#else

void cdc_task(void)
{
    #ifdef AM_CDC_USE_APP_BUF
    tud_app_buf_state_t rx_state = tud_cdc_rx_buf_state_get();
    tud_app_buf_state_t tx_state = tud_cdc_tx_buf_state_get();

    // If TX is completed/idle/bus-reset
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
        //Nothing is received into the buffer. Receive back into same buffer
        else
        {
            // Start receiving from USB Host
            tud_cdc_rx_buf_assign(gBuf[gRxPtr], sizeof(gBuf[gRxPtr]));
        }
    }
    // If buffer reset occured
    else if ( rx_state == TUD_APP_BUF_STATE_BUF_RESET )
    {
        // re-assign current buffer to receive, discard data received before
        // buffer reset.
        tud_cdc_rx_buf_assign(gBuf[gRxPtr], sizeof(gBuf[gRxPtr]));
    }

    #else //AM_CDC_USE_APP_BUF
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

            // read and echo back
            uint32_t count = tud_cdc_read(buf_rx, sizeof(buf_rx));
            tud_cdc_write(buf_rx, count);
            tud_cdc_write_flush();
        }
    }
    #endif //AM_CDC_USE_APP_BUF
}

// Invoked when cdc when line state changed e.g connected/disconnected
AM_USED void tud_cdc_line_state_cb(uint8_t itf, bool dtr, bool rts)
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
#endif

//--------------------------------------------------------------------+
// BLINKING TASK
//--------------------------------------------------------------------+
void led_blinking_task(void)
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
