//*****************************************************************************
//
//! @file tinyusb_cdc_msc_freertos_disconnect.c
//!
//! @brief tinyusb cdc-acm and mass storage FreeRTOS USB disconnect example.
//!
//! @addtogroup usb_examples USB Examples
//
//! @defgroup tinyusb_cdc_msc_freertos_disconnect TinyUSB CDC ACM Mass Storage FreeRTOS Disconnect Example
//! @ingroup usb_examples
//! @{
//!
//! Purpose: This example demonstrates a composite USB device including one USB CDC-ACM
//! and one mass storage device. the COM port will echo back the
//! the input from the terminal and the mass storage device will be disk when
//! connecting to other OS like Windows and Linux. The CDC-ACM and Mass storage
//! functions will be handled by two separate FreeRTOS tasks.
//!
//! This example demonstrates how to use disconnect and reconnect
//! (USB V bus voltage lost and restore) to shutdown / restart the usb module
//! Since this is an example, designed to run on eval boards without USB V bus sensing,
//! the two evb buttons have been drafted to simulate USB V bus voltage
//! lost and recovered. The file usb_disconnect.c contains the
//! button init and button scan code and also the method used to inform the
//! tinyusb stack that it needs to service power up/power down calls.
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

#include "FreeRTOS.h"
#include "task.h"
#include "timers.h"
#include "board.h"
#include "tusb.h"
#include "dcd_apollo4.h"
#include "usb_disconnect.h"
#include "am_mcu_apollo.h"

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
    BUTTON_SCAN_RATE = 41,
};

TimerHandle_t blinky_tm;
osal_mutex_def_t gMutexDefUSB;
osal_mutex_t gMutexUSB = NULL;

// Increase stack size when debug log is enabled
#if CFG_TUSB_DEBUG
  #define USBD_STACK_SIZE     (3*configMINIMAL_STACK_SIZE)
#else
  #define USBD_STACK_SIZE     (3*configMINIMAL_STACK_SIZE/2)
#endif

#define CDC_STACK_SIZE      2*configMINIMAL_STACK_SIZE
#define BTN_STACK_SIZE      configMINIMAL_STACK_SIZE

static void led_blinky_cb(TimerHandle_t xTimer);
static void usb_device_task(void* param);
static void cdc_task(void* params);
static void button_scan_task(void* params);

// ****************************************************************************
//
//! @note For Keil compilers, any Tinyusb callbacks that are prototyped as TU_ATTR_WEAK
//! will be optimized out by the linker. To force the linker to include these functions,
//! simply put the attribute 'used" in front of the function. This is accomplished with
//! the macro AM_USED
//
// ****************************************************************************

//--------------------------------------------------------------------+
// Main
//--------------------------------------------------------------------+
int main(void)
{
    board_init();

    gMutexUSB = osal_mutex_create(&gMutexDefUSB);

    NVIC_SetPriority(USB0_IRQn, NVIC_configMAX_SYSCALL_INTERRUPT_PRIORITY);

    // soft timer for blinky
    blinky_tm = xTimerCreate(NULL, pdMS_TO_TICKS(BLINK_NOT_MOUNTED), true, NULL, led_blinky_cb);
    xTimerStart(blinky_tm, 0);

    // Create a task for tinyusb device stack
    (void) xTaskCreate( usb_device_task, "usbd", USBD_STACK_SIZE, NULL, configMAX_PRIORITIES-1, NULL);

    // Create CDC task
    (void) xTaskCreate( cdc_task, "cdc", CDC_STACK_SIZE, NULL, configMAX_PRIORITIES-2, NULL);

    vTaskStartScheduler();

    return 0;
}

//--------------------------------------------------------------------+
// USB Device Driver task
// This top level thread process all usb events and invoke callbacks
//--------------------------------------------------------------------+
static void
usb_device_task(void* param)
{
    (void) param;

    // This should be called after scheduler/kernel is started.
    // Otherwise, it could cause kernel issue since USB IRQ handler does use RTOS queue API.
    usb_disconnect_init();

    dvd_set_vddPresent( 0, true, false ) ;
    tusb_init();

    // Create a task for button scanning
    (void) xTaskCreate( button_scan_task, "btnscan", BTN_STACK_SIZE, NULL, configMAX_PRIORITIES-2, NULL);

    // RTOS forever loop
    while (1)
    {
        // tinyusb device task, when using FREE RTOS this won't typically return
        tud_task();
    }
}

//--------------------------------------------------------------------+
// Device callbacks
//--------------------------------------------------------------------+

//--------------------------------------------------------------------+
// Invoked when device is mounted
//--------------------------------------------------------------------+
AM_USED void
tud_mount_cb(void)
{
    xTimerChangePeriod(blinky_tm, pdMS_TO_TICKS(BLINK_MOUNTED), 0);
}

#ifndef TUSB_ADDED_FUNCTIONS

// Invoked when device is unmounted
// called when DCD_EVENT_UNPLUGGED event occurs
AM_USED void
tud_umount_cb(void)
{
    xTimerChangePeriod(blinky_tm, pdMS_TO_TICKS(BLINK_NOT_MOUNTED), 0);
}
#endif // !TUSB_ADDED_FUNCTIONS

//--------------------------------------------------------------------+
// Invoked when usb bus is suspended
// remote_wakeup_en : if host allow us  to perform remote wakeup
// Within 7ms, device must draw an average of current less than 2.5 mA from bus
//--------------------------------------------------------------------+
AM_USED void
tud_suspend_cb(bool remote_wakeup_en)
{
    (void) remote_wakeup_en;
    xTimerChangePeriod(blinky_tm, pdMS_TO_TICKS(BLINK_SUSPENDED), 0);
}

//--------------------------------------------------------------------+
// Invoked when usb bus is resumed
//--------------------------------------------------------------------+
AM_USED void
tud_resume_cb(void)
{
    xTimerChangePeriod(blinky_tm, pdMS_TO_TICKS(BLINK_MOUNTED), 0);
}

//--------------------------------------------------------------------+
// USB CDC
//--------------------------------------------------------------------+
static void
cdc_task(void* params)
{
    (void) params;

    // RTOS forever loop
    while ( 1 )
    {
        osal_mutex_lock(gMutexUSB, OSAL_TIMEOUT_WAIT_FOREVER);

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

        osal_mutex_unlock(gMutexUSB);

        // Allow lower priority tasks to run
        taskYIELD ();
    }
}

//--------------------------------------------------------------------+
// Invoked when cdc when line state changed e.g connected/disconnected
//--------------------------------------------------------------------+
AM_USED void
tud_cdc_line_state_cb(uint8_t itf, bool dtr, bool rts)
{
    (void) itf;
    (void) rts;

    // connected
    if ( dtr )
    {
        // print initial message when connected
        tud_cdc_write_str("\r\nTinyUSB CDC MSC device with FreeRTOS example\r\n");
        tud_cdc_write_flush();
    }
}

//--------------------------------------------------------------------+
// Invoked when CDC interface received data from host
//--------------------------------------------------------------------+
AM_USED void
tud_cdc_rx_cb(uint8_t itf)
{
    (void) itf;
}

//--------------------------------------------------------------------+
// BLINKING TASK
//--------------------------------------------------------------------+
static void
led_blinky_cb(TimerHandle_t xTimer)
{
    static bool led_state = false;
    (void) xTimer;

    board_led_write(led_state);
    led_state = 1 - led_state; // toggle

}

//--------------------------------------------------------------------+
// BUTTON SCANNING TASK
//--------------------------------------------------------------------+
static void
button_scan_task(void* params)
{
    (void)params;

    while(1)
    {
        osal_mutex_lock(gMutexUSB, OSAL_TIMEOUT_WAIT_FOREVER);
        usb_disconnect_periodic(false);
        osal_mutex_unlock(gMutexUSB);
        vTaskDelay(pdMS_TO_TICKS(BUTTON_SCAN_RATE));
    }
}

//*****************************************************************************
//
// End Doxygen group.
//! @}
//
//*****************************************************************************

