//*****************************************************************************
//
//! @file freertos_mspi_iom_display.c
//!
//! @brief Example demonstrating frame buffer compositing and display rendering
//!
//! This example demonstrates frame buffer compositing and display rendering
//! using the hardware assisted MSPI to IOM transfer under FreeRTOS.
//! To demonstrate full speed frame composition and rendering, a ping-pong
//! Frame buffer is used in the PSRAM, and composition is done in parallel,
//! while rendering is ongoing from alternate frame buffer
//!
//! At initialization, both the Display and PSRAM are initialized and the Src images
//! initialized
//!
//! Composition:
//! Composition uses two source images, again in PSRAM.
//! Src1 & Src2 - both containing a small vertical bar.
//! Composition creates a sliding bar effect, with the Src1 and Src2 bars moving
//! in opposite directions, overlapped with each other. In addition the background
//! color is changed for each update as well.
//! There is a compile time configuration for a less intense composition, which
//! only loads one Src image (Src2) and creates a scrolling effect.
//! Compositing is done in internal SRAM, with small fragments (limited to 1K size)
//! of Src images brought in to SRAM from PSRAM using DMA.
//! CPU computes the final image fragment using these two Src image fragments
//! brought into SRAM, which is then DMA'ed to PSRAM.
//! Compile time modes allow direct composition in PSRAM using XIPMM (or a combination
//! of XIP and XIPMM) as well.
//! The process repeats till the whole image is constructed.
//!
//! Rendering:
//! This example demonstrates transferring a large buffer from a PSRAM device
//! connected on MSPI, to a Display device connected to IOM, using hardware
//! handshaking in Apollo3 - with minimal CPU involvement.
//! The Display is connected to IOM using SPI interface, and hence the transactions
//! are limited in size to 4095 Bytes.
//! However MSPI PSRAM imposes further limit on max transaction size as the page size
//! of 1K limits us to use max 1K bursts
//! The program here creates a command queue for both the MSPI and IOM, to
//! create a sequence of transactions - each reading a segment of the source
//! buffer to a temp buffer in internal SRAM, and then writing the same to the
//! Display using the IOM. It uses hardware handshaking so that the IOM transaction
//! is started only once the segement is read out completely from MSPI PSRAM.
//! To best utilize the buses, a ping-pong model is used using two temporary
//! buffers in SRAM. This allows the interfaces to not idle while waiting for
//! other to finish - essentially achieving close to the bandwidth achieved by
//! the slower of the two.
//! Rendering is synchronized with the TE signal from the display to ensure
//! no tearing on the display as the display buffer is being updated
//!
//! XIP:
//! If enabled, a timer is started to run a prime function out of PSRAM periodically
//!
//! XIPMM:
//! If enabled, a timer is started to run a demo function to exercise XIPMM out of PSRAM
//!
//! Configurable parameters at compile time:
//! IOM to use (DISPLAY_IOM_MODULE)
//! Display device to use (define one of DISPLAY_DEVICE_* to 1)
//! MSPI PSRAM to use - uses compile time definitions from am_device_mspi_psram.h
//! FRAME_SIZE - total size of transaction - controlled by ROW_NUM & COLUMN_NUM of display
//! SPI_TXN_SIZE - size of temporary ping-pong buffer
//! CPU_SLEEP_GPIO - tracks CPU sleeping on analyzer (High when asleep)
//! Most of the time is actually spent doing the composition, as evident by monitoring
//! TEST_GPIO1 (high during XIP) & TEST_GPIO2 (high during compose)
//!
//! Operating modes:
//! SEQLOOP not defined (normal mode) - The CQ is programmed each iteration using HAL APIs
//! SEQLOOP - Create sequence once, which repeats when triggered by callback at the
//! end of each iteration (should be used if the same framebuffer is used every time)
//! CQ_RAW - Uses Preconstructed CQ for IOM and MSPI (only small changes done at
//! run time) - to save on the time to program the same at run time
//!
//! Memory Impacts of modes: If CQ_RAW is used - the buffer supplied to HAL for CQ could be very small,
//! as the raw CQ is supplied by the application. So overall memory usage is still about the same
//!
//! Independent Controls:
//! Composition Levels:
//!     Less Intense Compositing: MINIMAL_CPU_COMPOSITIING
//!     No Compositing (Only MSPI-IOM Xfer Rendering): NO_COMPOSITING
//! Hardware DCX: USE_HW_DCX (Defined in am_bsp.h - requires recompiling bsp as well)
//! Start MSPI transactions ASAP (Pre-fill temp buffers as soon as available): START_MSPI_IOM_XFER_ASAP
//! Enable Periodic XIP function: ENABLE_XIP
//!     XIP region is uncached: XIP_UNCACHED
//!     XIP Function frequency: XIP_FREQ_HZ
//! Enable Periodic XIPMM function: ENABLE_XIPMM (only applicable to Apollo3-B0)
//!     XIPMM Function frequency: XIPMM_FREQ_HZ
//! Peripheral Frequency:
//!     IOM: IOM_FREQ
//!     MSPI: MSPI_FREQ
//! Composition Modes:
//!     How to Read Src Buffer: MODE_SRCBUF_READ (DMA/XIP/XIPMM <Apollo3-B0 Only>)
//!     How to Write to Frame Buffer: MODE_DESTBUF_WRITE (DMA/XIPMM <Apollo3-B0 Only>)
//!
//! FrameBuffer Configuration:
//! ** One Framebuffer - Sequential Composition and Rendering: SEQLOOP
//
//! @verbatim
//! Pin connections:
//! IOM:
//! Particular IOM to use for this example is controlled by macro DISPLAY_IOM_MODULE
//! This example use apollo3_evb board connected to a display board
//! Default pin settings for this example using IOM1 are:
//! #define AM_DISPLAY_GPIO_CSB            13
//! #define AM_DISPLAY_GPIO_TE_PIN         19
//! #define AM_DISPLAY_GPIO_A0             9
//! #define AM_DISPLAY_GPIO_SDA            7
//! #define AM_DISPLAY_GPIO_SCK            8
//!
//! MSPI:
//! The MSPI PSRAM device uses is controlled by macro DISPLAY_DEVICE_* (set one of them to 1)
//! This example uses apollo3_evb board connected to a psram board
//! #define AM_BSP_GPIO_MSPI_CE0            1
//! #define AM_BSP_MSPI_CE0_CHNL            1
//! #define AM_BSP_GPIO_MSPI_D0             22
//! #define AM_BSP_GPIO_MSPI_D1             26
//! #define AM_BSP_GPIO_MSPI_D2             4
//! #define AM_BSP_GPIO_MSPI_D3             23
//! #define AM_BSP_GPIO_MSPI_SCK            24
//!
//
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
// This application has a large number of common include files. For
// convenience, we'll collect them all together in a single header and include
// that everywhere.
//
//*****************************************************************************
#include "freertos_mspi_iom_display.h"
#include "rtos.h"

//*****************************************************************************
//
// Enable printing to the console.
//
//*****************************************************************************
void
enable_print_interface(void)
{
    //
    // Initialize a debug printing interface.
    //
    am_bsp_itm_printf_enable();
}


void
config_fastgpio(uint32_t pin)
{
    //
    // Configure the pins that are to be used for Fast GPIO.
    //
    am_hal_gpio_fastgpio_enable(pin);
    am_hal_gpio_fastgpio_clr(pin);
    //
    // Configure the pins that are to be used for Fast GPIO.
    //
    am_hal_gpio_fastgpio_disable(pin);
    am_hal_gpio_fastgpio_clr(pin);
    AM_HAL_GPIO_MASKCREATE(sGpioIntMask);
    am_hal_gpio_fast_pinconfig(AM_HAL_GPIO_MASKBIT(psGpioIntMask, pin),
                                         g_AM_HAL_GPIO_OUTPUT, 0);
}

//*****************************************************************************
//
// Main Function
//
//*****************************************************************************
int
main(void)
{
    //
    // Set the clock frequency
    //
    am_hal_clkgen_control(AM_HAL_CLKGEN_CONTROL_SYSCLK_MAX, 0);

#if 1
    //
    // Set the default cache configuration
    //
    am_hal_cachectrl_config(&am_hal_cachectrl_defaults);
    am_hal_cachectrl_enable();
#endif

#ifndef NOFPU
    //
    // Enable the floating point module, and configure the core for lazy
    // stacking.
    //
    am_hal_sysctrl_fpu_enable();
    am_hal_sysctrl_fpu_stacking_enable(true);
#else
    am_hal_sysctrl_fpu_disable();
#endif

    //
    // Configure the board for low power.
    //
    am_bsp_low_power_init();

    //
    // Enable printing to the console.
    //
#ifdef AM_DEBUG_PRINTF
    enable_print_interface();
#endif

    //
    // Initialize plotting interface.
    //
    am_util_debug_printf("FreeRTOS PSRAM Stress Example\n");

    // GPIO used to track the CPU sleeping
    //
    // Configure the pins that are to be used for Fast GPIO.
    //
    am_hal_gpio_pinconfig(CPU_SLEEP_GPIO, g_AM_HAL_GPIO_OUTPUT_12);
    DEBUG_GPIO_LOW(CPU_SLEEP_GPIO);
    am_hal_gpio_pinconfig(TEST_GPIO1, g_AM_HAL_GPIO_OUTPUT_12);
    DEBUG_GPIO_LOW(TEST_GPIO1);
    am_hal_gpio_pinconfig(TEST_GPIO2, g_AM_HAL_GPIO_OUTPUT_12);
    DEBUG_GPIO_LOW(TEST_GPIO2);
    am_hal_gpio_pinconfig(TEST_GPIO3, g_AM_HAL_GPIO_OUTPUT_12);
    DEBUG_GPIO_LOW(TEST_GPIO3);

    //
    // Run the application.
    //
    run_tasks();

    //
    // We shouldn't ever get here.
    //
    while (1)
    {
    }

}

