//*****************************************************************************
//
//! @file lvgl_enhanced_stress_test.c
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

#include "lvgl_enhanced_stress_test.h"

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
    // Add a wait before the program starts.  The debugger/J-Link are being locked out.
    //
    // am_util_delay_ms(5000);

    //
    // Configure the board for low power operation.
    //
    am_bsp_low_power_init();

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

    if (MSPI_FLASH_STATUS_SUCCESS != mspi_psram_init() )
    {
        am_util_stdio_printf("Failed to configure the MSPI and PSRAM Device correctly!\n");
    }
#if (MSPI_TASK_ENABLE == 1)
     // When MSPI0 is at hex mode, we couldn't use MSPI1
    if ( ((g_sMspiPsramConfig.eDeviceConfig == AM_HAL_MSPI_FLASH_HEX_DDR_CE1) || (g_sMspiPsramConfig.eDeviceConfig == AM_HAL_MSPI_FLASH_HEX_DDR_CE0))
        && (MSPI_FLASH_MODULE == 1))
    {
        am_util_stdio_printf("When MSPI0 is at hex mode, we couldn't use MSPI1");
    }
    else if ( MSPI_PSRAM_STATUS_SUCCESS != mspi_flash_init() )
    {
        am_util_stdio_printf("Failed to configure the MSPI and Flash Device correctly!\n");
    }
#endif
#ifdef CPU_RUN_IN_HP_MODE
    if ( am_hal_pwrctrl_mcu_mode_select(AM_HAL_PWRCTRL_MCU_MODE_HIGH_PERFORMANCE) != AM_HAL_STATUS_SUCCESS )
    {
        am_util_stdio_printf("HP_LP:Enter HP mode failed!\n");
    }
#endif

#ifdef GPU_RUN_IN_HP_MODE
    am_hal_pwrctrl_periph_disable(AM_HAL_PWRCTRL_PERIPH_GFX);

    //
    //Switch to HP mode.
    //
    am_hal_pwrctrl_gpu_mode_e current_mode;
    am_hal_pwrctrl_gpu_mode_select(AM_HAL_PWRCTRL_GPU_MODE_HIGH_PERFORMANCE);
    am_hal_pwrctrl_gpu_mode_status(&current_mode);
    if ( AM_HAL_PWRCTRL_GPU_MODE_HIGH_PERFORMANCE != current_mode )
    {
        am_util_stdio_printf("gpu switch to HP mode failed!\n");
    }
#endif

#if LV_MEM_CUSTOM == 1
    tcm_heap_init(LV_MEM_TCM_ADR, LV_MEM_TCM_SIZE, 128);
    ssram_heap_init(LV_MEM_SSRAM_ADR, LV_MEM_SSRAM_SIZE, 16*1024);
    psram_heap_init(LV_MEM_PSRAM_ADR, LV_MEM_PSRAM_SIZE, 64*1024);
#endif

    // Init LVGL.
    lv_init();

    //
    // Initialize the printf interface for ITM output
    //
    am_bsp_debug_printf_enable();

    //
    // Initialize plotting interface.
    //
    am_util_stdio_printf("lvgl enhance stress test Example\n");
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

