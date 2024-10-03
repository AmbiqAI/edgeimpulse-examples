//*****************************************************************************
//
//! @file tinyusb_cdc_msc_hfrc2.c
//!
//! @brief tinyusb cdc-acm / msc example.
//!
//! @addtogroup usb_examples USB Examples
//!
//! @defgroup tinyusb_cdc_msc_hfrc2 TinyUSB CDC MSC HFRC2 Example
//! @ingroup usb_examples
//! @{
//!
//! Purpose: This example demonstrates a composite USB device including
//! one USB CDC-ACM and one mass storage device.
//! This defaults to USB-HIGH speed operation.
//!
//! This example show how to enable HFRC2 -tune. In this mode, the HFRC2 FLL is
//! usually off to minimize frequency jitter and save power. Periodically
//! HFRC2 tune will be called under application control
//! to adjust the output frequency. This will compensate for frequency drift
//! due to temperature changes.
//!
//! The COM port will echo back the
//! the input from the terminal and the mass storage device will mount as a disk
//! when connecting to other OS like Windows and Linux.
//!
//*****************************************************************************

//*****************************************************************************
//
// ${copyright}
//
// This is part of revision ${version} of the AmbiqSuite Development Package.
//
//*****************************************************************************

#include "board.h"
#include "tusb.h"
#include "hfrc2_test_utils.h"
#include "usb_timer_utils.h"

#if defined(AM_PART_APOLLO4B)
#warning "USB HIGH-SPEED mode is not supported for this processor"
#endif


//*****************************************************************************
// MACRO CONSTANT TYPEDEF PROTOTYPES
//*****************************************************************************

//
//! output HFRC2 clock on a pin
//
#define ENABLE_HFRC2_CLOCKOUT  0

//
//! Enable HFRC2 tune. The HFRC2 FLL will be off except when
//! the HFRC2 FFL is briefly enabled and uses the XTHS to
//! adjust the frequency. It is recommend HFRC2-recompute be
//! called at 10Mhz.
//
#define DO_HFRC2_REFRESH 1

//
//! Blink pattern
//! - 250 ms  : device not mounted
//! - 1000 ms : device mounted
//! - 2500 ms : device is suspended
//
enum
{
    BLINK_NOT_MOUNTED = 250,
    BLINK_MOUNTED = 1000,
    BLINK_SUSPENDED = 2500,
};

//
//! declare RAM variables used in this file
//
typedef struct
{
    uint32_t ui32_count_10_seconds;
    uint32_t ui32_num_HFRC2_corrections;
    uint32_t ui32_blink_interval_ms;
    volatile bool bTimerIntOccurred;
}
tusb_vars_t;

//
//! allocate RAM variables used in this file
//
static tusb_vars_t g_localv;

//
//! Static Function Prototypes
//

static void led_blinking_task(void);
static void cdc_task(void);

//*****************************************************************************
//
//! @brief callback function for timer
//! this is set to be called at 1hz from the timer isr
//! This provides a clock to time the HFRC2 clock refresh activities
//! It is recommended that the HFRC2 refresh be run every ten seconds and
//! it it best if it is not run in an ISR, simply because it has a 1 msec delay
//! when run in fault mode.
//
//*****************************************************************************
static void
usb_timer_cb(void)
{
    g_localv.bTimerIntOccurred = true;
}

//*****************************************************************************
//
//! @brief main
//
//*****************************************************************************
int main(void)
{
    board_init();
    //
    // start the 1Hz timer
    //
    timer_init( TMR_TICKS_PER_SECOND * 1, usb_timer_cb);

    //
    // this function will init the usb and manage the high-speed clocks
    //
    tusb_init();

#if ENABLE_HFRC2_CLOCKOUT == 1
    uint32_t ui32ClockoutStatus = hfrc2_enable_hfrc2_clock_out(AM_HAL_CLKGEN_CLKOUT_HFRC2_24M, 33); // enable pin out
    if (ui32ClockoutStatus)
    {
        am_util_stdio_printf("clockout start error\n") ;
        while(1) ;
    }
#endif

    //
    // This boolean is true when the MCUCTRL->HFRC2 (tune) register is available,
    // It is available in Apollo4 Plus devices with TrimRev 6 and greater.
    // If the Apollo4 Plus is TrimRev5 or less, use the HFRC2 Tune patch at
    // support.ambiq.com

    //
    // start by assuming HFRC2 tune is available
    //
    bool bHFRC2TuneRegisterAvailable = true ;

    while (1)
    {
        tud_task(); // tinyusb device task

        cdc_task();
        if (g_localv.bTimerIntOccurred)
        {
            //
            // 1Hz timer interrupt occurred
            //
            g_localv.bTimerIntOccurred = false;
            if (++g_localv.ui32_count_10_seconds >= 10 )
            {
                g_localv.ui32_count_10_seconds = 0;

                //
                // it is recommended this function be called every 10 seconds.
                // in the default mode, this will take just a bit over 1008 usec
                //
#if DO_HFRC2_REFRESH == 1
                if (bHFRC2TuneRegisterAvailable)
                {
                    uint32_t status = am_hal_clkgen_HFRC2_adj_recompute(&tReComputeCtrlDefault);
                    if (status)
                    {
                        if ( status == AM_HAL_STATUS_INVALID_OPERATION )
                        {
                            //
                            // If HFRC2 tune (recompute) call fails,
                            // depending on the calling mode, the call to
                            // am_hal_clkgen_HFRC2_adj_recompute could have
                            // left HFRC2-adjust enabled.(check setting of
                            // tReComputeCtrlDefault.eHF2RecomputeControl)
                            // The user should check to ensure this setting is
                            // correct for their application.
                            //
                            // the call to  am_hal_clkgen_HFRC2_adj_recompute, recompute-tune-register
                            // isn't available. Setting this false prevents further attempts.
                            //
                            bHFRC2TuneRegisterAvailable = false;
                        }
                        am_util_stdio_printf("USB adjusted failed %08lx\n", status);
                    }
                    else
                    {

                        am_util_stdio_printf("USB adjusted output %08lx\n", CLKGEN->HF2VAL);
                    }
                }
#endif
            }

            led_blinking_task();
        }
    }
}

//--------------------------------------------------------------------+
// Device callbacks
//--------------------------------------------------------------------+

//*****************************************************************************
//
//! @brief  Invoked when device is mounted
//
//*****************************************************************************
AM_USED void
tud_mount_cb(void)
{
    g_localv.ui32_blink_interval_ms = BLINK_MOUNTED;
}

#ifndef TUSB_ADDED_FUNCTIONS

//*****************************************************************************
//
//! @brief  Invoked when device is unmounted
//
//*****************************************************************************
AM_USED void
tud_umount_cb(void)
{
    g_localv.ui32_blink_interval_ms = BLINK_NOT_MOUNTED;
}

#endif

//*****************************************************************************
//
//! @brief Invoked when usb bus is suspended
//!
//! @param remote_wakeup_en - if host allow us to perform remote wakeup
//! Within 7ms, device must draw an average of current less than 2.5 mA from bus
//
//*****************************************************************************
AM_USED void
tud_suspend_cb(bool remote_wakeup_en)
{
    (void) remote_wakeup_en;
    g_localv.ui32_blink_interval_ms = BLINK_SUSPENDED;
}
//*****************************************************************************
//
//! @brief Invoked when usb bus is resumed
//
//*****************************************************************************
AM_USED void
tud_resume_cb(void)
{
    g_localv.ui32_blink_interval_ms = BLINK_MOUNTED;
}

//*****************************************************************************
//
//! @brief USB CDC management
//
//*****************************************************************************
static void
cdc_task(void)
{
    // connected() check for DTR bit
    // Most, but not all. terminal client set this when making connection
    // if ( tud_cdc_connected() )
    {
        //
        // connected and there are data available
        //
        if (tud_cdc_available())
        {
            uint8_t buf[64];

            //
            // read and echo back
            //
            uint32_t count = tud_cdc_read(buf, sizeof(buf));

            for (uint32_t i = 0; i < count; i++)
            {
                tud_cdc_write_char(buf[i]);

                if (buf[i] == '\r')
                {
                    tud_cdc_write_char('\n');
                }
            }

            tud_cdc_write_flush();
        }
    }
}

//*****************************************************************************
//
//! @brief  Callback invoked when cdc when line state changed e.g connected/disconnected
//
//*****************************************************************************
AM_USED void
tud_cdc_line_state_cb(uint8_t itf, bool dtr, bool rts)
{
    (void) itf;
    (void) rts;

    //
    // connected
    //
    if (dtr)
    {
        // print initial message when connected
        tud_cdc_write_str("\r\nTinyUSB CDC MSC HS device example\r\n");
        tud_cdc_write_flush();
    }
}

//*****************************************************************************
//
//! @brief  Invoked when CDC interface received data from host
//
//*****************************************************************************
AM_USED void
tud_cdc_rx_cb(uint8_t itf)
{
    (void) itf;
}

//*****************************************************************************
//
//! @brief  blinking task
//
//*****************************************************************************
//
static void led_blinking_task(void)
{
    static bool led_state = false;

    // Blink every interval ms

    board_led_write(led_state);
    led_state = 1 - led_state; // toggle
}
//*****************************************************************************
//
// End Doxygen group.
//! @}
//
//*****************************************************************************

