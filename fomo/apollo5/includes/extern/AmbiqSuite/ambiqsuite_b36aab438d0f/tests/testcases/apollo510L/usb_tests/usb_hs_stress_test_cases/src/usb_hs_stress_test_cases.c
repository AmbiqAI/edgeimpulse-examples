//*****************************************************************************
//
//! @file usb_hs_stress_test.c
//!
//! @brief USB High Speed stress test example.
//! This example uses Display, PDM to I2S, USB HS to read/write data from different
//! sections to put enormous pressure on the HFRC.
//! The USB device is configured as a cdc msc device.
//! PDM interface to receive audio signals in 16 kHz
//! sample rate (or 1.536 MHz PDM clock) from an external digital microphone and
//! forwards converted PCM data by the I2S interface to an external I2S device.
//! The GUI, Render and Display tasks are configured to display a watch.
//! IOX mode 1 is used for this example.
//
//*****************************************************************************
//*****************************************************************************
//
// ${copyright}
//
// This is part of revision ${version} of the AmbiqSuite Development Package.
//
//*****************************************************************************

#include "usb_hs_stress_test.h"

//*****************************************************************************
//
// Macro definitions
//
//*****************************************************************************
#define AM_DEBUG_PRINTF

//*****************************************************************************
//
// External variable definitions
//
//*****************************************************************************
extern am_devices_mspi_psram_config_t g_sMspiPsramConfig;

//*****************************************************************************
//
// Global Variables
//
//*****************************************************************************

//We need to set up a small heap to be used by Clib, delete this may cause unexpected error.
// __attribute__ ((section(".heap")))

AM_SECTION("HEAP")
uint32_t clib_heap[100];

//*****************************************************************************
//
// Optional Global setup.
//
// globalSetUp() will get called before the test group starts, and
//
//*****************************************************************************
void
globalSetUp(void)
{
    //Global setup
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
globalTearDown(void)
{
    // Global teardown
}

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
// Main Function
//
//*****************************************************************************
int
usb_hs_stress_test(void)
{
	//
    // Add a wait before the program starts.  The debugger/J-Link are being locked out.
    //
     //am_util_delay_ms(5000);

    am_hal_cachectrl_icache_disable();
    am_hal_cachectrl_dcache_disable();


    //
    // Clear the terminal and print the banner.
    //
    am_util_stdio_terminal_clear();

    //
    // Enable global IRQ.
    //
    am_hal_interrupt_master_enable();

    PWRCTRL->CPUPWRCTRL_b.SLEEPMODE = 1;


    if (MSPI_FLASH_STATUS_SUCCESS != mspi_psram_init() )
    {
        am_util_stdio_printf("Failed to configure the MSPI and PSRAM Device correctly!\n");
    }


    //
    // Initialize the printf interface for ITM output
    //
    am_bsp_debug_printf_enable();

    //
    // Initialize plotting interface.
    //
    am_util_stdio_printf("USB_HS_stress_test \n");
    //
    // Run the application.
    //
    run_tasks();

    //
    // We shouldn't never get here.
    //
    while (1)
    {
    }

}

