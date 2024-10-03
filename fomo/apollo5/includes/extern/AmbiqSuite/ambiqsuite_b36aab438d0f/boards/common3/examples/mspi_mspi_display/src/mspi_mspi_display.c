//*****************************************************************************
//
//! @file mspi_mspi_display.c
//!
//! @brief Example demonstrating the hardware assisted MSPI to MSPI transfer
//!
//! Purpose: This example demonstrates transferring a large buffer from a PSRAM device
//! connected on MSPI, to a Display device connected to another MSPI, using hardware
//! handshaking in Apollo3 Blue Plus - with minimal CPU involvement.
//!
//! At initialization, both the Display and PSRAM are initialized and a set image
//! data is written to the PSRAM for transfer to Display.
//!
//! The program here creates a command queue for both the MSPIs, to
//! create a sequence of transactions - each reading a segment of the source
//! buffer to a temp buffer in internal SRAM, and then writing the same to the
//! Display using the MSPI. It uses hardware handshaking so that the Display transaction
//! is started only once the segement is read out completely from MSPI PSRAM.
//!
//! To best utilize the buses, a ping-pong model is used using two temporary
//! buffers in SRAM. This allows the interfaces to not idle while waiting for
//! other to finish - essentially achieving close to the bandwidth achieved by
//! the slower of the two.
//!
//! Additional Information:
//! Configurable parameters at compile time:
//! Display MSPI to use (DISPLAY_MSPI_MODULE)
//! MSPI PSRAM to use - uses compile time definitions from am_device_mspi_psram.h
//! FRAME_SIZE - total size of transaction - controlled by ROW_NUM & COLUMN_NUM of display
//! TEMP_BUFFER_SIZE - size of temporary ping-pong buffer
//! CPU_SLEEP_GPIO - tracks CPU sleeping on analyzer
//!
//! Operating modes:
//! CQ_RAW - Uses Preconstructed CQ (only small changes done at run time) - to save on the time to program the same at run time
//!
//! Memory Impacts of modes: If CQ_RAW is used - the buffer supplied to HAL for CQ could be very small,
//! as the raw CQ is supplied by the application. So overall memory usage is still about the same
//!
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
#include "mspi_mspi_display.h"


//*****************************************************************************
//
// Interrupt handler for the GPIO pins.
//
//*****************************************************************************
void am_gpio_isr(void)
{
    //
    // Read and clear the GPIO interrupt status.
    //
#if defined(AM_PART_APOLLO3P)
    AM_HAL_GPIO_MASKCREATE(GpioIntStatusMask);

    am_hal_gpio_interrupt_status_get(false, pGpioIntStatusMask);
    am_hal_gpio_interrupt_clear(pGpioIntStatusMask);
    am_hal_gpio_interrupt_service(pGpioIntStatusMask);
#elif defined(AM_PART_APOLLO3)
    uint64_t ui64Status;

    am_hal_gpio_interrupt_status_get(false, &ui64Status);
    am_hal_gpio_interrupt_clear(ui64Status);
    am_hal_gpio_interrupt_service(ui64Status);
#else
    #error Unknown device.
#endif
}

int
main(void)
{
    //
    // Set the clock frequency
    //
    am_hal_clkgen_control(AM_HAL_CLKGEN_CONTROL_SYSCLK_MAX, 0);

    am_hal_cachectrl_config(&am_hal_cachectrl_defaults);
    am_hal_cachectrl_enable();


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

    am_hal_gpio_pinconfig(DBG1_GPIO, g_AM_HAL_GPIO_OUTPUT_12);
    DEBUG_GPIO_LOW(DBG1_GPIO);
    am_hal_gpio_pinconfig(DBG2_GPIO, g_AM_HAL_GPIO_OUTPUT_12);
    DEBUG_GPIO_LOW(DBG2_GPIO);
    //
    // Run display
    //
    display_init();

    // PSRAM
    psram_init();
    psram_data_init();

    // Render first FB
    init_mspi_mspi_xfer();
    start_mspi_mspi_xfer(g_actFb, FRAME_SIZE);

    while(1)
    {
#ifndef IMAGE_BMP
        compose();
#endif
        //
        // Disable interrupt while we decide whether we're going to sleep.
        //
        uint32_t ui32IntStatus = am_hal_interrupt_master_disable();
        if (g_bTEInt)
        {
            // New buffer composed, and Display ready to take it
            //
            // Enable interrupts
            //
            am_hal_interrupt_master_set(ui32IntStatus);
            g_bDisplayDone = false;
            g_bTEInt = false;
#ifndef IMAGE_BMP
            g_bNewDisplay = false;
            g_bNewFB = true;
#endif
            if (start_mspi_mspi_xfer(g_actFb, FRAME_SIZE))
            {
                DEBUG_PRINT("Unable to initiate MSPI IOM transfer\n");
                while(1);
            }
        }
        else
        {
            am_hal_sysctrl_sleep(true);
            //
            // Enable interrupts
            //
            am_hal_interrupt_master_set(ui32IntStatus);
        }
    }
}
