//*****************************************************************************
//
//! @file nemagfx_enhanced_stress_test.c
//!
//! @brief NemaGFX example.
//! This example uses MSPI, IOM, DMA, GPU, and DC to read/write data from different
//! ram sections to put enormous pressure on the AXI bus.
//! The GUI task also demonstrates how to use the partial frame buffer and ping-pong
//! buffer features in a pipeline workflow for DMA, GPU, and DC. In GUI task,
//! DMA copy texture from external PSRAM to internal SSRAM or ExtendedRAM,
//! GPU render these textures to frame buffer, and DC transfer the framebuffer to
//! display panel, these three steps are organized in a pipeline to make DMA, GPU,
//! and DC work in parallel.
//!
//! SMALLFB
//! undefine this to disable partial framebuffer features
//! SMALLFB_STRIPES
//! controls how many stripes to divide the whole framebuffer
//!
//! Note: This example needs PSRAM devices connected to MSPI0, if you encounter
//! hardfault, please check your PSRAM setting.You are supposed to see a digital
//! Quartz clock if GUI task runs successfully.
//
//*****************************************************************************
//*****************************************************************************
//
// ${copyright}
//
// This is part of revision ${version} of the AmbiqSuite Development Package.
//
//*****************************************************************************

#include "mspi_dma_xip_parallel_stress_test_cases.h"

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
__attribute__ ((section(".heap")))
uint32_t clib_heap[100];

//*****************************************************************************
//
// Main Function
//
//*****************************************************************************
int
main(void)
{
    //
    //  Enable the I-Cache and D-Cache.
    //
    am_hal_cachectrl_icache_enable();
    am_hal_cachectrl_dcache_enable(true);

    //
    // Initialize the printf interface for ITM/SWO output.
    //
    am_bsp_itm_printf_enable();

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

#if (XIP_TASK_ENABLE == 1)
    if (MSPI_FLASH_STATUS_SUCCESS != mspi_psram_xip_code_init() )
    {
        am_util_stdio_printf("Failed to configure the MSPI and PSRAM Device correctly!\n");
    }
#endif

#if (MSPI_TASK_ENABLE == 1)
     // When MSPI0 is at hex mode, we couldn't use MSPI1
    if ( ((g_sMspiPsramConfig.eDeviceConfig == AM_HAL_MSPI_FLASH_HEX_DDR_CE1) || (g_sMspiPsramConfig.eDeviceConfig == AM_HAL_MSPI_FLASH_HEX_DDR_CE0)) 
        && (MSPI_FLASH_MODULE == 1))
    {
        am_util_stdio_printf("When MSPI0 is at hex mode, we couldn't use MSPI1");
    }
    else if( MSPI_PSRAM_STATUS_SUCCESS != mspi_flash_init() )
    {
        am_util_stdio_printf("Failed to configure the MSPI and Flash Device correctly!\n");
    }    
#endif
 
    //
    // Initialize the printf interface for ITM output
    //
    am_bsp_debug_printf_enable();

    //
    // Initialize plotting interface.
    //
    am_util_stdio_printf("Apollo5a mspi_dma_xip_parallel_stress_test_cases \n");
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

