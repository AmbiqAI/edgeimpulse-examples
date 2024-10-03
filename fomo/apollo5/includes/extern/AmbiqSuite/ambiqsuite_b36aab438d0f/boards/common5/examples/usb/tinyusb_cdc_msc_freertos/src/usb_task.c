
//*****************************************************************************
//
//! @file usb_task.c
//!
//! @brief Task to handle GUI operations.
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
//*****************************************************************************
//
// Global includes for this project.
//
//*****************************************************************************
#include "tinyusb_cdc_msc_freertos.h"

TimerHandle_t blinky_tm;

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

const char welcome_text[] = "\r\nTinyUSB CDC MSC device with FreeRTOS example\r\n";


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


void led_blinky_cb(TimerHandle_t xTimer)
{
    (void) xTimer;
    static bool led_state = false;

    board_led_write(led_state);
    led_state = 1 - led_state; // toggle
}


// USB Device Driver task
// This top level thread process all usb events and invoke callbacks
void usb_device_task(void* param)
{
    (void) param;

    // This should be called after scheduler/kernel is started.
    // Otherwise it could cause kernel issue since USB IRQ handler does use RTOS queue API.
    tusb_init();

    #ifdef AM_CDC_USE_APP_BUF
    tud_cdc_rx_buf_assign(gBuf[0], sizeof(gBuf[0]));
    #endif //AM_CDC_USE_APP_BUF

    // soft timer for blinky
    blinky_tm = xTimerCreate(NULL, pdMS_TO_TICKS(BLINK_NOT_MOUNTED), true, NULL, led_blinky_cb);
    xTimerStart(blinky_tm, 0);

    // RTOS forever loop
    while (1)
    {
        // tinyusb device task
        tud_task();
    }
}

//--------------------------------------------------------------------+
// Device callbacks
//--------------------------------------------------------------------+

// Invoked when device is mounted
AM_USED void tud_mount_cb(void)
{
    xTimerChangePeriod(blinky_tm, pdMS_TO_TICKS(BLINK_MOUNTED), 0);
}

// Invoked when device is unmounted
AM_USED void tud_umount_cb(void)
{
    xTimerChangePeriod(blinky_tm, pdMS_TO_TICKS(BLINK_NOT_MOUNTED), 0);
}

// Invoked when usb bus is suspended
// remote_wakeup_en : if host allow us  to perform remote wakeup
// Within 7ms, device must draw an average of current less than 2.5 mA from bus
AM_USED void tud_suspend_cb(bool remote_wakeup_en)
{
    (void) remote_wakeup_en;
    xTimerChangePeriod(blinky_tm, pdMS_TO_TICKS(BLINK_SUSPENDED), 0);
}

// Invoked when usb bus is resumed
AM_USED void tud_resume_cb(void)
{
    xTimerChangePeriod(blinky_tm, pdMS_TO_TICKS(BLINK_MOUNTED), 0);
}

//--------------------------------------------------------------------+
// USB CDC
//--------------------------------------------------------------------+
void cdc_task(void *pvParameters)
{
    // RTOS forever loop
    while ( 1 )
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
        // If buffer-reset occurred
        else if ( rx_state == TUD_APP_BUF_STATE_BUF_RESET )
        {
            // re-assign current buffer to receive, discarding data received
            // before buffer reset.
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
        #endif

        // Allow lower priority tasks to run
        taskYIELD ();
    }
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

// Invoked when CDC interface received data from host
AM_USED void tud_cdc_rx_cb(uint8_t itf)
{
    (void) itf;
}

