//*****************************************************************************
//
//! @file display_tests.c
//!
//! @brief Manufacturing tests for the display device.
//
//! Sweeping across the screen to find screen defects.
//! @verbatim
//! This example uses IOM1, Mode 0, interrupt mode.
//! SCK  Pin 8 (active low)
//! MISO Pin 10
//! MOSI Pin 9
//! CS   Pin 12 (Active High)
//! 8 bit per transfer.
//! MSB first.
//! @endverbatim
//
//*****************************************************************************

//*****************************************************************************
//
// ${copyright}
//
// This is part of revision ${version} of the AmbiqSuite Development Package.
//
//*****************************************************************************

#include <string.h>
#include "am_mcu_apollo.h"
#include "am_hal_iom.h"
#include "am_bsp.h"
#include "am_devices.h"
#include "am_util.h"
#include "mustang_3.h"

//! Spi mode 0 or 3.
#define SPI_MODE        (0)
//! HAL support for IOM includes polling, (non-blocking), and queue.
#define HAL_IOM_MODE    (IOM_QUEUE)
#define QUEUESIZE       (64)
#define DISPLAY_IOM     (1)
// M1nCE0
#define DISPLAY_CE      (0)


//*****************************************************************************
//
// DISPLAY peripheral assignments.
//
//*****************************************************************************
#define AM_BSP_DISPLAY_LS013B4DN4_PRESENT    1
#define AM_BSP_DISPLAY_IOM                   1
#define AM_BSP_DISPLAY_CS                    0

//*****************************************************************************
//
// DISPLAY pins.
//
//*****************************************************************************
#define AM_BSP_GPIO_DISPLAY_CS          12
#define AM_BSP_GPIO_CFG_DISPLAY_CS      AM_HAL_PIN_OUTPUT

//*****************************************************************************
//
// IOM Mode Definitions for this board.
//
//*****************************************************************************
#define AM_BSP_IOM0_SPI_MODE                0
#define AM_BSP_IOM1_SPI_MODE                1
#define AM_BSP_IOM2_SPI_MODE                1

//
// Define which display is to be used.
//
//#define QG9696TSWHG04           true        // SSD1327 96x96, 4 bit grayscale.
#define LS013B4DN04             true        // Sharp 96x96, 1 bit OLED.
//#define ST7735                  true        // Sitronix 160x128 TFT, 18 bit Color.

//
// Allow display device selection, add more defines or struct as needed.
//
#ifdef LS013B4DN04
#define DISPLAY_WIDTH           AM_DISPLAY_LS013B4DN04_WIDTH
#define DISPLAY_HEIGHT          AM_DISPLAY_LS013B4DN04_HEIGHT
#define DISPLAY_DEVICEINFO      am_devices_display_ls013b4dn04_t
#define DISPLAY_INIT            am_devices_display_ls013b4dn04_init
#define DISPLAY_CLEAR           am_devices_display_ls013b4dn04_screen_clear
#define DISPLAY_COPYLINES       am_devices_display_ls013b4dn04_lines_copy
#define DISPLAY_REFRESH         am_devices_display_ls013b4dn04_hw_refresh
#elif QG9696TSWHG04
#define DISPLAY_WIDTH           AM_DISPLAY_QG9696TSWHG04_WIDTH
#define DISPLAY_HEIGHT          AM_DISPLAY_QG9696TSWHG04_HEIGHT
#define DISPLAY_DEVICEINFO      am_devices_display_qg9696tswhg04_t
#define DISPLAY_INIT            am_devices_display_qg9696tswhg04_init
#define DISPLAY_CLEAR           am_devices_display_qg9696tswhg04_screen_clear
#define DISPLAY_COPYLINES       am_devices_display_qg9696tswhg04_lines_copy
#define DISPLAY_REFRESH         am_devices_display_qg9696tswhg04_hw_refresh
#elif ST7735
#define DISPLAY_WIDTH           AM_DISPLAY_ST7735_WIDTH
#define DISPLAY_HEIGHT          AM_DISPLAY_ST7735_HEIGHT
#define DISPLAY_DEVICEINFO      am_devices_display_st7735_t
#define DISPLAY_INIT            am_devices_display_st7735_init
#define DISPLAY_CLEAR           am_devices_display_st7735_screen_clear
#define DISPLAY_COPYLINES       am_devices_display_st7735_lines_copy
#define DISPLAY_REFRESH         am_devices_display_st7735_hw_refresh
#else
#error "DISPLAY NOT DEFINED."
#endif

#define DISPLAY_DRAWINFO        am_util_draw_2d1bpp_context_t

//*****************************************************************************
//
// Frame buffer - must be 32-bit aligned.
// Declare as uint32_t, but define as uint8_t.
//
//*****************************************************************************
#define g_ui8_display_buffer    ((uint8_t*) g_ui32_display_buffer)
uint32_t g_ui32_display_buffer[(DISPLAY_WIDTH * DISPLAY_HEIGHT) / 8 / 4];

//*****************************************************************************
//
// Declare display device and utils drawing data structures.
//
//*****************************************************************************
DISPLAY_DRAWINFO      g_sDisplayDrawInfo;
DISPLAY_DEVICEINFO    g_sDisplayDeviceInfo;

//*****************************************************************************
//
// IOM configuration structures.
//
//*****************************************************************************
#if (AM_BSP_IOM1_SPI_MODE == 1)
#if defined(QG9696TSWHG04)
//
// The MBRD 96x96 display requires polarity and phase both be 1 for proper
// SPI data alignment.
//
am_hal_iom_config_t g_sIOMConfig =
{
    .ui32ClockFrequency = AM_HAL_IOM_8MHZ,
    .ui32InterfaceMode = AM_HAL_IOM_SPIMODE,
    .ui8WriteThreshold = 4,
    .ui8ReadThreshold = 60,
    .bSPHA = 1,
    .bSPOL = 1,
};
#elif defined(LS013B4DN04)
//
// The Sharp display is speced for 1MHZ.
//
am_hal_iom_config_t g_sIOMConfig =
{
    .ui32ClockFrequency = AM_HAL_IOM_1MHZ,  // limited by sharp display
    .ui32InterfaceMode = AM_HAL_IOM_SPIMODE,
    .ui8WriteThreshold = 4,
    .ui8ReadThreshold = 60,
    .bSPHA = 0,
    .bSPOL = 0,
};
#elif defined(ST7735)
//
// The display is speced for 24Mhz
//
am_hal_iom_config_t g_sIOMConfig =
{
    .ui32ClockFrequency = AM_HAL_IOM_12MHZ,
    .ui32InterfaceMode = AM_HAL_IOM_SPIMODE,
    .ui8WriteThreshold = 4,
    .ui8ReadThreshold = 60,
    .bSPHA = 0,
    .bSPOL = 0,
};
#else
#error "SPI Config not defined for this display."
#endif
#endif

// IOM1 control section.
// Driver performs yield and wait functions.
// IOM1 ISR is defined here. Common for all.

// Memory for the IOM Display Queue.
am_hal_iom_queue_entry_t g_psIOMDisplayQueue[32];

//*****************************************************************************
//
// Interrupt handler for Display IOM
//
//*****************************************************************************
/** Take am_iomaster1_isr from startup_xxx.c default handler. */
#if 0
#if (HAL_IOM_MODE == IOM_QUEUE)
AM_IOMASTER_ISR_QUEUE(1)
#elif (HAL_IOM_MODE == IOM_NONBLOCK)
AM_IOMASTER_ISR_NB(1)
#endif
#else
void
am_iomaster1_isr(void)
{
    uint32_t ui32IntStatus;

    //
    // Read and clear the interrupt status.
    //
    ui32IntStatus = am_hal_iom_int_status_get(1, false);
    am_hal_iom_int_clear(1, ui32IntStatus);

    //
    // Service FIFO interrupts as necessary, and call IOM callbacks as
    // transfers are completed.
    //
    am_hal_iom_queue_service(1, ui32IntStatus);
}
#endif

//*****************************************************************************
//
// IOM setup function.
//
//*****************************************************************************
void
iom_ctl_init(am_hal_iom_config_t *psIOMConfig)
{
    //
    // Setup the pins for SPI mode. This task is always IOM1.
    //
    am_bsp_iom_spi_pins_enable(1);

    //
    // Set the required configuration settings for the IOM.
    //
    am_hal_iom_config(1, psIOMConfig);

    //
    // Enable interrupts for command complete and fifo threshold events, which
    // are needed for the iom_ctrl write and read functions.
    //
    am_hal_iom_int_enable(1, AM_HAL_IOM_INT_CMDCMP | AM_HAL_IOM_INT_THR);

    //
    // Set the iom interrupt priority to a level that allows interacting with
    // FreeRTOS API calls.
    //
//    am_hal_interrupt_priority_set(AM_HAL_INTERRUPT_IOMASTER1,
//                                  configMAX_SYSCALL_INTERRUPT_PRIORITY);

    //
    // Enable interrupts to IOM
    //
    am_hal_interrupt_enable(AM_HAL_INTERRUPT_IOMASTER1);

    //
    // Initalize the DISPLAY IOM queue.
    //
    am_hal_iom_queue_init(1, g_psIOMDisplayQueue, 32);
}

//*****************************************************************************
//
// Enable ITM printing.
//
//*****************************************************************************

void enable_itm_print(char *msg)
{
    //
    // Setup ITM pin for plotting
    //
    am_bsp_pin_enable(ITM_SWO);

    //
    // Initialize the printf interface for ITM/SWO output.
    //
    am_util_stdio_printf_init((am_util_stdio_print_char_t) am_bsp_itm_string_print);

    //
    // Initialize the SWO GPIO pin
    //
    am_bsp_pin_enable(ITM_SWO);

    //
    // Enable the ITM.
    //
    am_hal_itm_enable();

    //
    // Enable debug printf messages using ITM on SWO pin
    //
    am_bsp_debug_printf_enable();

    //
    // Clear the terminal, and print a message to show that we're up and
    // running.
    //
    am_util_stdio_terminal_clear();
    if ( msg != NULL )
    {
        am_util_stdio_printf(msg);
    }
}

//*****************************************************************************
//
// Timer configurations.
//
//*****************************************************************************
am_hal_ctimer_config_t g_sTimer0 =
{
    // Don't link timers.
    0,

    // Set up Timer0A.
    (AM_HAL_CTIMER_FN_PWM_REPEAT |
     AM_HAL_CTIMER_INT_ENABLE    |
     AM_HAL_CTIMER_LFRC_1HZ),

    // No configuration for Timer0B.
    0,
};

//*****************************************************************************
//
// Init function for Timer A0.
//
//*****************************************************************************
void
timerA0_init(uint32_t ui32Refresh)
{
    uint32_t ui32Period, ui32LFRCcfg;
    am_hal_ctimer_config_t sTimer0 =
    {
        // Don't link timers.
        0,

        // Set up Timer0A.
        (AM_HAL_CTIMER_FN_PWM_REPEAT |
         AM_HAL_CTIMER_INT_ENABLE    |
         AM_HAL_CTIMER_LFRC_1HZ),

        // No configuration for Timer0B.
        0,
    };

    ui32LFRCcfg = AM_HAL_CTIMER_FN_PWM_REPEAT | AM_HAL_CTIMER_INT_ENABLE;

    if ( ( ui32Refresh > 32000 )        &&
         ( ui32Refresh < 512000 ) )
    {
        ui32LFRCcfg |= AM_HAL_CTIMER_LFRC_1_16HZ;
        ui32Period   = ui32Refresh / (16*1000) + 1;
    }
    else if ( ( ui32Refresh > 1000 )    &&
              ( ui32Refresh <  32000 ) )
    {
        ui32LFRCcfg |= AM_HAL_CTIMER_LFRC_1HZ;
        ui32Period   = ui32Refresh / 1000 + 1;
    }
    else if ( ( ui32Refresh >= 1 )      &&
              ( ui32Refresh <= 1000 ) )
    {
        ui32LFRCcfg |= AM_HAL_CTIMER_LFRC_512HZ;
        ui32Period   = ui32Refresh * 512 / 1000 + 1;
    }
    else
    {
        return;
    }

    sTimer0.ui32TimerAConfig = ui32LFRCcfg;

    //
    // Enable the LFRC.
    //
    am_hal_clkgen_osc_start(AM_HAL_CLKGEN_OSC_LFRC);

    //
    // Set up timer A0 but don't start it yet
    //
    am_hal_ctimer_clear(0, AM_HAL_CTIMER_TIMERA);
    am_hal_ctimer_config(0, &sTimer0);
    am_hal_ctimer_period_set(0,
                             AM_HAL_CTIMER_TIMERA,
                             ui32Period,
                             (ui32Period >> 1) );

    //
    // Clear the timer Interrupt
    //
    am_hal_ctimer_int_clear(AM_HAL_CTIMER_INT_TIMERA0);
}

//*****************************************************************************
//
// Timer Interrupt Service Routine (ISR)
//
//*****************************************************************************
uint32_t g_xcnt = 0;
uint32_t g_pix  = 0;

void
am_ctimer_isr(void)
{
    //
    // Clear TimerA0 Interrupt (write to clear).
    //
    am_hal_ctimer_int_clear(AM_HAL_CTIMER_INT_TIMERA0);
}

//*****************************************************************************
//
// Main
//
//*****************************************************************************
int
main(void)
{
    uint32_t uCount, uCountHWRefresh, u32WaitMS;
    uint32_t x1, y1, x2, y2;

    //
    // Set the clock frequency.
    //
    am_hal_clkgen_sysclk_select(AM_HAL_CLKGEN_SYSCLK_MAX);

    //
    // Set the default cache configuration
    //
    am_hal_cachectrl_enable(&am_hal_cachectrl_defaults);

    //
    // Configure the board for low power operation.
    //
    am_bsp_low_power_init();

    //
    // Enable power to IOM.
    //
    am_hal_iom_pwrctrl_enable(AM_BSP_DISPLAY_IOM);

    //
    // Configure the IOM.
    //
    iom_ctl_init(&g_sIOMConfig);

    //
    // Initialize the display device data structure using BSP assigned IOM.
    // First, fill in the application-responsible members.
    //
    g_sDisplayDeviceInfo.pui8Framebuffer   = (uint8_t*)g_ui32_display_buffer;
    g_sDisplayDeviceInfo.ui32Module        = AM_BSP_DISPLAY_IOM;
    g_sDisplayDeviceInfo.ui32ChipSelect    = AM_BSP_DISPLAY_CS;
    g_sDisplayDeviceInfo.ui32ChipSelectPin = AM_BSP_GPIO_DISPLAY_CS;
#if defined(QG9696TSWHG04)
    g_sDisplayDeviceInfo.ui32DCselect      = AM_BSP_GPIO_DISPLAY_D_C;

    //
    // This display has 2 additional signals that are controlled via GPIO.
    // Initialize these signals.
    //
    am_bsp_pin_enable(DISPLAY_D_C);
    am_bsp_pin_enable(DISPLAY_BACKLT);

    //
    // Turn on the display backlight boost
    //
    AM_HAL_GPIO_ENS(AM_BSP_GPIO_DISPLAY_BACKLT) = AM_HAL_GPIO_ENS_M(AM_BSP_GPIO_DISPLAY_BACKLT);
    AM_HAL_GPIO_WTS(AM_BSP_GPIO_DISPLAY_BACKLT) = AM_HAL_GPIO_WTS_M(AM_BSP_GPIO_DISPLAY_BACKLT);
#endif

    //
    // Set the IOM configuration and enable the interface
    // The BSP specifies that IOM instance 1 is to be used for the display,
    // and that CS is driven on IOM1_DISPLAY_CS (GPIO 12).
    //
    am_bsp_pin_enable(DISPLAY_CS);
    // @bug Duplicated in iom1_ctl_init
    am_bsp_iom_spi_pins_enable(AM_BSP_DISPLAY_IOM);


    //
    // Initialize the display device, callbacks are in the driver.
    //
    DISPLAY_INIT(&g_sDisplayDeviceInfo);

    //
    // Now that we have the device info, fill in the relevant g_sDisplayDrawInfo members.
    //
    g_sDisplayDrawInfo.pui8Framebuffer  = g_sDisplayDeviceInfo.pui8Framebuffer;
    g_sDisplayDrawInfo.ui32DrawWidth    = g_sDisplayDeviceInfo.ui32DisplayWidth;
    g_sDisplayDrawInfo.ui32DrawHeight   = g_sDisplayDeviceInfo.ui32DisplayHeight;
    g_sDisplayDrawInfo.ui32DrawPolarity = g_sDisplayDeviceInfo.ui32DisplayBkgndPolarity;

    //
    // Initialize the refresh timer (TimerA0) using the refresh value that
    // was returned by the display driver.
    //
    if ( g_sDisplayDeviceInfo.ui32HwRefreshMs == 0 )
    {
        timerA0_init(200);
    }
    else
    {
        timerA0_init(g_sDisplayDeviceInfo.ui32HwRefreshMs);
    }

    //
    // Enable the timer interrupt in the NVIC.
    //
    am_hal_interrupt_enable(AM_HAL_INTERRUPT_CTIMER);
    am_hal_interrupt_master_enable();

    //
    // Enable the timer Interrupt.
    //
    am_hal_ctimer_int_enable(AM_HAL_CTIMER_INT_TIMERA0);

    //
    // Start timer A0
    //
    am_hal_ctimer_start(0, AM_HAL_CTIMER_TIMERA);

    //
    // Initialize the printf interface for ITM/SWO output.
    //
    enable_itm_print("Display Hardware Test\r\n");

    //
    // We are done printing. Disable debug printf messages on ITM.
    //
    //am_bsp_debug_printf_disable();

    //
    // Select a font to use in when drawing strings to the display.
    //
//  am_util_draw_2d1bpp_font_select(&g_sDisplayDrawInfo, AM_FONT_SELECT_PIXELDREAMS_09x20);
    am_util_draw_2d1bpp_font_select(&g_sDisplayDrawInfo, AM_FONT_SELECT_PIXELDREAMS_08x17);

    //
    // Select a pause time for each image.
    // 0 will sleep between images, otherwise it's the number of milliseconds
    //  to wait between images.
    //
    u32WaitMS = 500;
//    u32WaitMS = 1;

    //
    // Set up a counter to periodically activate the hw_refresh() cycle.
    //
    if ( u32WaitMS == 0 )
    {
        uCountHWRefresh = 1;
    }
    else
    {
        if ( (u32WaitMS * 5) <= 1000 )
        {
            uCountHWRefresh = 1000 / (u32WaitMS * 5);
        }
        else
        {
            uCountHWRefresh = 1;
        }
    }

    //
    // Draw a bitmap.
    //
    x1 = 0;
    y1 = 0;
    DISPLAY_CLEAR(&g_sDisplayDeviceInfo);
    am_util_draw_2d1bpp_bitmap_draw(&g_sDisplayDrawInfo, g_u8_bmpMustangSplash3_96x96,
        DISPLAY_WIDTH, DISPLAY_HEIGHT, x1, y1, AM_UTIL_DRAW_ROPS_INVERT);
    DISPLAY_COPYLINES(&g_sDisplayDeviceInfo, 0, DISPLAY_HEIGHT-1);
    if ( u32WaitMS == 0 )
    {
        am_hal_sysctrl_sleep(AM_HAL_SYSCTRL_SLEEP_DEEP);
    }
    else
    {
        am_util_delay_ms(u32WaitMS);
    }

    //
    // Start the loop
    //
    uCount = 0;
    int count = 0;
    char counter[10] = {0, };
    int ymin, ymax;
    while (1)
    {
        if ( uCount++ >= uCountHWRefresh )
        {
            //
            // The hardware refresh should occur about once per second.
            //
            DISPLAY_REFRESH(&g_sDisplayDeviceInfo, false);
            uCount = 0;
        }

        //
        // Draw some text.
        //
        if (count < 10000)
        {
            am_util_stdio_sprintf(counter, "Test: %d", count++);
        }
        else
        {
            strcpy(counter, "Test: OK");
        }

        DISPLAY_CLEAR(&g_sDisplayDeviceInfo);
        am_util_draw_2d1bpp_string_draw(&g_sDisplayDrawInfo, (uint8_t*)counter,
            0, DISPLAY_HEIGHT / 2, AM_UTIL_DRAW_ROPS_COPY, 1);
        DISPLAY_COPYLINES(&g_sDisplayDeviceInfo, 0, DISPLAY_HEIGHT - 1);

        if ( u32WaitMS == 0 )
        {
            am_hal_sysctrl_sleep(AM_HAL_SYSCTRL_SLEEP_DEEP);
        }
        else
        {
            am_util_delay_ms(u32WaitMS);
        }

        //
        // Draw a center rectangle
        // x1y1 - starting point, x2y2 - width height.
        DISPLAY_CLEAR(&g_sDisplayDeviceInfo);
        /* Square screen for now.. */
        /* Min to Max */
        for ( int i = 0; i < DISPLAY_WIDTH / 2; i++ )
        {
            am_util_draw_2d1bpp_rectangle_draw(&g_sDisplayDrawInfo,
                DISPLAY_WIDTH / 2 - i, DISPLAY_HEIGHT / 2 - i, i * 2, i * 2, AM_UTIL_DRAW_ROPS_COPY, 1, 0);
            ymin = DISPLAY_HEIGHT / 2 - i;
            ymax = ymin + i * 2;
            DISPLAY_COPYLINES(&g_sDisplayDeviceInfo, ymin, ymax);
            am_util_delay_ms(10);
        }
        /* Max to Min. */
        for ( int i = DISPLAY_WIDTH / 2; i > 0; i-- )
        {
            am_util_draw_2d1bpp_rectangle_draw(&g_sDisplayDrawInfo,
                DISPLAY_WIDTH / 2 - i, DISPLAY_HEIGHT / 2 - i, i * 2, i * 2, AM_UTIL_DRAW_ROPS_COPY, 0, 0);
            ymin = DISPLAY_HEIGHT / 2 - i;
            ymax = ymin + i*2;
            DISPLAY_COPYLINES(&g_sDisplayDeviceInfo, ymin, ymax);
            am_util_delay_ms(10);
        }

        //
        // Draw a rectangle
        //
        DISPLAY_CLEAR(&g_sDisplayDeviceInfo);
        /* Square screen for now.. */
        for (int i = 0; i < DISPLAY_WIDTH; i++)
        {
            am_util_draw_2d1bpp_rectangle_draw(&g_sDisplayDrawInfo,
                DISPLAY_WIDTH - i, DISPLAY_HEIGHT - i, i, i, AM_UTIL_DRAW_ROPS_COPY, 1, 0);
            ymin = DISPLAY_HEIGHT-i;
            ymax = ymin + i;
            DISPLAY_COPYLINES(&g_sDisplayDeviceInfo, ymin, ymax);
            am_util_delay_ms(10);
        }

        /* Reverse, to clear screen. */
        for (int i = 0; i < DISPLAY_WIDTH; i++)
        {
            am_util_draw_2d1bpp_rectangle_draw(&g_sDisplayDrawInfo,
                0, 0, i, i, AM_UTIL_DRAW_ROPS_COPY, 0, 0);
            ymin = 0;
            ymax = ymin + i;
            DISPLAY_COPYLINES(&g_sDisplayDeviceInfo, ymin, ymax);
            am_util_delay_ms(10);
        }

        //
        // Draw a line
        //

        /* Draw upper half diagonal lines. */
        DISPLAY_CLEAR(&g_sDisplayDeviceInfo);
        /* Reverse upper */
        for ( int i = 0; i < DISPLAY_WIDTH; i++ )
        {
            am_util_draw_2d1bpp_line_draw(&g_sDisplayDrawInfo,
                DISPLAY_WIDTH-1, DISPLAY_HEIGHT-i, DISPLAY_WIDTH-i, DISPLAY_HEIGHT-1,
                1, AM_UTIL_DRAW_ROPS_COPY, 1);
            ymin = DISPLAY_HEIGHT-i-1;
            ymax = DISPLAY_HEIGHT-1;
            DISPLAY_COPYLINES(&g_sDisplayDeviceInfo, 0, ymax);
            am_util_delay_ms(10);
        }
        /* Reverse lower */
        for ( int i = 0; i < DISPLAY_WIDTH; i++ )
        {
            am_util_draw_2d1bpp_line_draw(&g_sDisplayDrawInfo,
                DISPLAY_WIDTH-i, 0, 0, DISPLAY_HEIGHT-i, 1, AM_UTIL_DRAW_ROPS_COPY, 1);
            ymin = 0;
            ymax = DISPLAY_HEIGHT-i;
            DISPLAY_COPYLINES(&g_sDisplayDeviceInfo, ymin, ymax);
            am_util_delay_ms(10);
        }
        for ( int i = 0; i < DISPLAY_WIDTH; i++ )
        {
            am_util_draw_2d1bpp_line_draw(&g_sDisplayDrawInfo,
                i, 0, 0, i, 1, AM_UTIL_DRAW_ROPS_COPY, 0);
            ymin = 0;
            ymax = i;
            DISPLAY_COPYLINES(&g_sDisplayDeviceInfo, ymin, ymax);
            am_util_delay_ms(10);
        }
        /* Draw lower half diagonal lines. */
        for ( int i = 0; i < DISPLAY_WIDTH; i++ )
        {
            am_util_draw_2d1bpp_line_draw(&g_sDisplayDrawInfo,
                DISPLAY_WIDTH - 1, i, i, DISPLAY_HEIGHT - 1, 1, AM_UTIL_DRAW_ROPS_COPY, 0);
            ymin = i;
            ymax = DISPLAY_HEIGHT-1;
            DISPLAY_COPYLINES(&g_sDisplayDeviceInfo, ymin, ymax);
            am_util_delay_ms(10);
        }


        DISPLAY_CLEAR(&g_sDisplayDeviceInfo);
        /* Vertical lines */
        for ( int i = 0; i < DISPLAY_WIDTH; i++ )
        {
            am_util_draw_2d1bpp_line_draw(&g_sDisplayDrawInfo,
                0, DISPLAY_HEIGHT - i, DISPLAY_WIDTH - 1, DISPLAY_HEIGHT - i, 1, AM_UTIL_DRAW_ROPS_COPY, 1);
            ymin = DISPLAY_HEIGHT-i;
            ymax = DISPLAY_HEIGHT-i;
            DISPLAY_COPYLINES(&g_sDisplayDeviceInfo, ymin, ymax);
            am_util_delay_ms(10);
        }
        /* Reverse Vertical */
        for ( int i = 0; i < DISPLAY_WIDTH; i++ )
        {
            am_util_draw_2d1bpp_line_draw(&g_sDisplayDrawInfo,
                0, i, DISPLAY_HEIGHT-1, i, 1, AM_UTIL_DRAW_ROPS_COPY, 0);
            ymin = i;
            ymax = i;
            DISPLAY_COPYLINES(&g_sDisplayDeviceInfo, ymin, ymax);
            am_util_delay_ms(10);
        }

        if ( u32WaitMS == 0 )
        {
            am_hal_sysctrl_sleep(AM_HAL_SYSCTRL_SLEEP_DEEP);
        }
        else
        {
            am_util_delay_ms(u32WaitMS);
        }

        //
        // Draw an ellipse.
        //
        // Get center of the ellipse.
        x1 = DISPLAY_WIDTH / 2;
        y1 = DISPLAY_HEIGHT / 2;

        DISPLAY_CLEAR(&g_sDisplayDeviceInfo);

        for ( x2 = 1, y2 = 1; x2 < DISPLAY_WIDTH / 2; x2++, y2++ )
        {
            am_util_draw_2d1bpp_ellipse_draw(&g_sDisplayDrawInfo, x1, y1, x2, y2, AM_UTIL_DRAW_ROPS_COPY, 1, 1);
            DISPLAY_COPYLINES(&g_sDisplayDeviceInfo, 0, DISPLAY_HEIGHT-1);
            am_util_delay_ms(10);
        }

        for ( x2 = DISPLAY_WIDTH / 2, y2 = DISPLAY_HEIGHT / 2; x2 > 0; x2--, y2-- )
        {
            am_util_draw_2d1bpp_ellipse_draw(&g_sDisplayDrawInfo, x1, y1, x2, y2, AM_UTIL_DRAW_ROPS_COPY, 0, 0);
            DISPLAY_COPYLINES(&g_sDisplayDeviceInfo, y1 - y2, y1 + y2);
            am_util_delay_ms(10);
        }

        if ( u32WaitMS == 0 )
        {
            am_hal_sysctrl_sleep(AM_HAL_SYSCTRL_SLEEP_DEEP);
        }
        else
        {
            am_util_delay_ms(u32WaitMS);
        }
    }
}


