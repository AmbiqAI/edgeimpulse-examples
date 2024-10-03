//*****************************************************************************
//
//! @file usb_dma_test_cases
//!
//! @brief Test cases for USB DMA.
//!
//! usb_dma_test_dma_size:
//! - This test case is doing sweep test on DMA size by sweeping DMA size of
//!   1 to 512, by sending the said number of bytes to USB host via CDC, and
//!   then receives the same numbers of bytes from Host looped back data, and
//!   execute data compare check on them
//!   After the firmware starts running, execute script at
//!   tools/usb_tests/usb_cdc_host_loopback at the PC Apollo5b is connected
//!   so that the USB Host will loopback the data it received.
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

#include "unity.h"
#include "am_mcu_apollo.h"
#include "am_bsp.h"
#include "am_util.h"

#include "board.h"
#include "tusb.h"

#define DMA_SWEEP_SIZE       (512)
#define ENABLE_SYSPLL_FOR_HS ((BOARD_DEVICE_RHPORT_SPEED == OPT_MODE_HIGH_SPEED) && defined(AM_PART_APOLLO5B) && !defined(APOLLO5_FPGA))
#define TERMINATE_TEST_TIMEOUT_LOOP 200
#define TERMINATE_TEST_MESSAGE "Terminate Host Loopback"
AM_SHARED_RW uint8_t gBufTx[DMA_SWEEP_SIZE];
AM_SHARED_RW uint8_t gBufRx[DMA_SWEEP_SIZE];


//--------------------------------------------------------------------+
// MACRO CONSTANT TYPEDEF PROTYPES
//--------------------------------------------------------------------+
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


//*****************************************************************************
//
// Optional setup/tear-down functions.
//
// These will be called before and after each test function listed below.
//
//*****************************************************************************
void
setUp(void)
{

}

void
tearDown(void)
{

}

//*****************************************************************************
//
// Optional Global setup/tear-down.
//
// globalSetUp() will get called before the test group starts, and
// globalTearDown() will get called after it ends. These won't be necessary for
// every test.
//
//*****************************************************************************
void
globalSetUp(void)
{
    am_util_stdio_printf("APOLLO5 USB DMA Tests\n");
}

void
globalTearDown(void)
{
}


/*------------- MAIN -------------*/
static uint32_t gui32UsbDmaSizeTest_ByteCounter = 1;
static bool     gbUsbDmaSizeTest_DataSent = false;
static bool     gbUsbHostConnected = false;
tud_app_buf_state_t eTxState;
void usb_dma_test_dma_size(void)
{
    board_init();

    #if ENABLE_SYSPLL_FOR_HS
    am_hal_clkmgr_clock_config(AM_HAL_CLKMGR_CLK_ID_SYSPLL, AM_HAL_USB_PHY_CLK_FREQ_HZ, NULL);
    #endif

    tusb_init();

    am_util_stdio_printf("Starting USB DMA size sweep test:");

    while (1)
    {
        tud_task(); // tinyusb device task

        if(gbUsbHostConnected && (gui32UsbDmaSizeTest_ByteCounter <= DMA_SWEEP_SIZE))
        {
            eTxState = tud_cdc_tx_buf_state_get();
            if(gbUsbDmaSizeTest_DataSent == false)
            {
                if ((gui32UsbDmaSizeTest_ByteCounter - 1) % 16 == 0)
                {
                    am_util_stdio_printf("\n");
                }
                am_util_stdio_printf("%3d..", gui32UsbDmaSizeTest_ByteCounter);

                // tud_app_buf_state_t eTxState = tud_cdc_tx_buf_state_get();
                if(eTxState != TUD_APP_BUF_STATE_BUSY)
                {
                    // Generate Test Data
                    for(uint32_t i=0; i<gui32UsbDmaSizeTest_ByteCounter; i++)
                    {
                        //gBufTx[i] = rand();
                        gBufTx[i] = i&0xFF;
                    }

                    // Send Buffer to USB Host
                    tud_cdc_tx_buf_assign_send(gBufTx, gui32UsbDmaSizeTest_ByteCounter);

                    // Receive Buffer from USB Host
                    tud_cdc_rx_buf_assign(gBufRx, gui32UsbDmaSizeTest_ByteCounter);

                    // Mark as sent
                    gbUsbDmaSizeTest_DataSent = true;
                }
            }
            else
            {
                tud_app_buf_state_t eRxState = tud_cdc_rx_buf_state_get();
                // Rx Buffer is received and ready for processing
                if (eRxState == TUD_APP_BUF_STATE_RX_READY)
                {
                    uint32_t ui32RecvCnt = tud_cdc_rx_buf_recv_count_get();
                    TEST_ASSERT_EQUAL_UINT32(gui32UsbDmaSizeTest_ByteCounter, ui32RecvCnt);
                    TEST_ASSERT_EQUAL_MEMORY_ARRAY(gBufTx, gBufRx, gui32UsbDmaSizeTest_ByteCounter, 1);

                    // Continue to next cycle
                    gui32UsbDmaSizeTest_ByteCounter ++;
                    gbUsbDmaSizeTest_DataSent = false;
                    if(gui32UsbDmaSizeTest_ByteCounter > DMA_SWEEP_SIZE)
                    {
                        // DMA Size Sweep Test Completed
                        am_util_stdio_printf("\n");
                        break;
                    }
                }
                // Rx buffer is discarded due to reset
                else if(eRxState == TUD_APP_BUF_STATE_BUF_RESET)
                {
                    // Reissue rx buffer
                    tud_cdc_rx_buf_assign(gBufRx, gui32UsbDmaSizeTest_ByteCounter);
                }

                // Tx buffer is discarded due to reset
                if(eTxState == TUD_APP_BUF_STATE_BUF_RESET)
                {
                    // Reissue rx buffer
                    tud_cdc_tx_buf_assign_send(gBufTx, gui32UsbDmaSizeTest_ByteCounter);
                }
            }
        }
    }

    // Send Terminate command to Host Loopback Script and wait for disconnect
    sprintf((char *)gBufTx, TERMINATE_TEST_MESSAGE);
    tud_cdc_tx_buf_assign_send(gBufTx, sizeof(TERMINATE_TEST_MESSAGE));
    for(uint32_t i=0; i<TERMINATE_TEST_TIMEOUT_LOOP; i++)
    {
        tud_task(); // tinyusb device task

        am_util_delay_ms(10);
    }
}

//--------------------------------------------------------------------+
// Device callbacks
//--------------------------------------------------------------------+

// Invoked when device is mounted
void tud_mount_cb(void)
{
    return;
}

// Invoked when device is unmounted
void tud_umount_cb(void)
{
    return;
}

// Invoked when usb bus is suspended
// remote_wakeup_en : if host allow us  to perform remote wakeup
// Within 7ms, device must draw an average of current less than 2.5 mA from bus
void tud_suspend_cb(bool remote_wakeup_en)
{
    (void) remote_wakeup_en;
    return;
}

// Invoked when usb bus is resumed
void tud_resume_cb(void)
{
    return;
}

// Invoked when CDC interface received data from host
void tud_cdc_rx_cb(uint8_t itf)
{
    (void) itf;
    return;
}

// Invoked when a TX is complete and therefore space becomes available in TX buffer
void tud_cdc_tx_complete_cb(uint8_t itf)
{
    (void) itf;
    return;
}

// Invoked when cdc when line state changed e.g connected/disconnected
void tud_cdc_line_state_cb(uint8_t itf, bool dtr, bool rts)
{
    (void) itf;
    (void) rts;

    if (dtr)
    {
        // Reset Test Case if Host has just been connected
        if(!gbUsbHostConnected)
        {
            gui32UsbDmaSizeTest_ByteCounter = 1;
            gbUsbDmaSizeTest_DataSent = false;
        }
        gbUsbHostConnected = true;
    }
    else
    {
        gbUsbHostConnected = false;
    }
    return;
}
