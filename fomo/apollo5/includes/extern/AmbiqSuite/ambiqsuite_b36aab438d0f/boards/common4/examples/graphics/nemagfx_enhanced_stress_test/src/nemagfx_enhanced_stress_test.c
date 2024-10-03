//*****************************************************************************
//
//! @file nemagfx_enhanced_stress_test.c
//!
//! @brief NemaGFX Enhanced Stress Test Example.
//!
//! @addtogroup graphics_examples Graphics Examples
//!
//! @defgroup nemagfx_enhanced_stress_test NemaGFX Enhanced Stress Test Example
//! @ingroup graphics_examples
//! @{
//!
//! Purpose: This example uses MSPI, IOM, DMA, GPU, and DC to read/write data from different
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
//! @note This example needs PSRAM devices connected to MSPI0, if you encounter
//! hardfault, please check your PSRAM setting.You are supposed to see a digital
//! Quartz clock if GUI task runs successfully.
//! STMBv1
//! ----------------------------------------------------------------------------
//! | STMBv1 Fly-wires:                  
//! ----------------------------------------------------------------------------
//! | *SDI (J17) <-> D0 (J17) for DC SPI read 
//! ----------------------------------------------------------------------------
//! | STMBv1 Switches:
//! ----------------------------------------------------------------------------
//! |            [SW17:OB ] [SW16:OB] [SW11:OB  ] [SW10:Hi-Z]
//! | [SW25:X16] [SW21:ON ] [SW24:ON] [SW23:Flash] [SW12:WB  ] [SW18:ON ]
//! |            [SW15:ETM] [SW20:ON] [SW14:X16 ] [SW19:ON  ] [SW13:PDM]
//! ----------------------------------------------------------------------------
//
//*****************************************************************************
//*****************************************************************************
//
// ${copyright}
//
// This is part of revision ${version} of the AmbiqSuite Development Package.
//
//*****************************************************************************

#include "nemagfx_enhanced_stress_test.h"

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

//
// We need to set up a small heap to be used by Clib, delete this may cause unexpected error.
//
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
    //am_util_delay_ms(5000);

    //
    // Configure the board for low power operation.
    //
    am_bsp_low_power_init();

#ifdef CONFIG_DAXI
    //
    //  am_bsp_low_power_init will config DAXI with the default setting,
    //  If you need a special setting, set it here.
    //
    am_hal_daxi_config_t sDAXIConfig =
    {
        .bDaxiPassThrough         = false,
        .bAgingSEnabled           = true,
        .eAgingCounter            = AM_HAL_DAXI_CONFIG_AGING_1024,
        .eNumBuf                  = AM_HAL_DAXI_CONFIG_NUMBUF_32,
        .eNumFreeBuf              = AM_HAL_DAXI_CONFIG_NUMFREEBUF_3,
    };
    am_hal_daxi_config(&sDAXIConfig);
    am_hal_delay_us(100);
#endif
    //
    //  Set up the cache.
    //
    am_hal_cachectrl_config_t sCacheConfig =
    {
        .bLRU                       = true,
        .eDescript                  = AM_HAL_CACHECTRL_DESCR_2WAY_128B_2048E,
        .eMode                      = AM_HAL_CACHECTRL_CONFIG_MODE_INSTR_DATA,
    };
    am_hal_cachectrl_config(&sCacheConfig);
    am_hal_cachectrl_enable();

#ifndef AM_PART_APOLLO4L
    //
    // Config DSP ram
    //
    am_hal_pwrctrl_dsp_memory_config_t      DSPMemCfg =
    {
        .bEnableICache      = true,
        .bRetainCache       = false,
        .bEnableRAM         = true,
        .bActiveRAM         = false,
        .bRetainRAM         = true
    };

    am_hal_pwrctrl_dsp_memory_config(AM_HAL_DSP0, &DSPMemCfg);
    am_hal_pwrctrl_dsp_memory_config(AM_HAL_DSP1, &DSPMemCfg);
#endif

    //
    // Initialize the printf interface for ITM/SWO output.
    //
    am_bsp_itm_printf_enable();

    //
    // Clear the terminal and print the banner.
    //
    am_util_stdio_terminal_clear();

    //
    // Configure the SEGGER SystemView Interface.
    //
#ifdef SYSTEM_VIEW
    SEGGER_SYSVIEW_Conf();
#endif

    //
    // Enable global IRQ.
    //
    am_hal_interrupt_master_enable();
		
#if(GUI_TASK_ENABLE ==1)		

    if (MSPI_FLASH_STATUS_SUCCESS != mspi_psram_init() )
    {
        am_util_stdio_printf("Failed to configure the MSPI and PSRAM  Device correctly!\n");
    }
#endif
#if (MSPI_TASK_ENABLE == 1)
    //
    // When MSPI0 is at hex mode, we couldn't use MSPI1
    //
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
    // Initialize the printf interface for Debug output
    //
    am_bsp_debug_printf_enable();

    //
    // Print start of Tests.
    //
    am_util_stdio_printf("nemagfx_stress_test Example\n");

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

//*****************************************************************************
//
// End Doxygen group.
//! @}
//
//*****************************************************************************

