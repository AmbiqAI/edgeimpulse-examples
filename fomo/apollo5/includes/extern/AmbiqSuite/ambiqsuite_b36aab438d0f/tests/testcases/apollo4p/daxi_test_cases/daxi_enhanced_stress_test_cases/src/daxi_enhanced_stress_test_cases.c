//*****************************************************************************
//
//! @file daxi_enhanced_stress_test.c
//!
//! @brief Run nemagfx_enhanced_stess_test with vary DAXI settings.
//!
//! This test includes multiple tasks. We delete all of tasks except setup tast
//! and idle task after they run to the end of task. Change DAXI setting and
//! resume setup task in idle task, and recreate these tasks in setup task.
//!
//! This test also supports cache test, please enable macro "CACHE_TEST" in
//! config-template.ini if you want to run it.
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

#include "nemagfx_enhanced_stress_test.h"
#include "am_mcu_apollo.h"
#include "am_bsp.h"
#include "am_util.h"
#include "nema_hal.h"
#include "daxi_test_common.h"
#include "psram.h"

//*****************************************************************************
//
// Macro definitions
//
//*****************************************************************************
#define AM_DEBUG_PRINTF

//*****************************************************************************
//
// Global Variables
//
//*****************************************************************************
static uint32_t        ui32DMATCBBuffer2[2560];
void            *g_pPsramHandle;
void            *g_pMSPIPsramHandle;

am_devices_mspi_psram_config_t g_sMspiPsramConfig =
{
#ifdef AM_PART_APOLLO4P
    .eDeviceConfig            = AM_HAL_MSPI_FLASH_HEX_DDR_CE1,
#else
    .eDeviceConfig            = AM_HAL_MSPI_FLASH_OCTAL_DDR_CE1,
#endif
    .eClockFreq               = AM_HAL_MSPI_CLK_96MHZ,
    .ui32NBTxnBufLength       = sizeof(ui32DMATCBBuffer2) / sizeof(uint32_t),
    .pNBTxnBuf                = ui32DMATCBBuffer2,
    .ui32ScramblingStartAddr  = 0,
    .ui32ScramblingEndAddr    = 0,
};

//! MSPI interrupts.
static const IRQn_Type MspiInterrupts[] =
{
    MSPI0_IRQn,
#if defined(AM_PART_APOLLO3P) || defined(AM_PART_APOLLO4) || defined(AM_PART_APOLLO4B)
    MSPI1_IRQn,
    MSPI2_IRQn,
#endif
};

//
// Take over the interrupt handler for whichever MSPI we're using.
//
#define psram_mspi_isr                                                          \
    am_mspi_isr1(MSPI_PSRAM_MODULE)
#define am_mspi_isr1(n)                                                        \
    am_mspi_isr(n)
#define am_mspi_isr(n)                                                         \
    am_mspi ## n ## _isr

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
    am_bsp_debug_printf_enable();
}

//*****************************************************************************
//
// Disable printing to the console.
//
//*****************************************************************************
void
disable_print_interface(void)
{
    //
    // Initialize a debug printing interface.
    //
    am_bsp_debug_printf_disable();
    am_hal_itm_disable();
}

//*****************************************************************************
//
// MSPI ISRs.
//
//*****************************************************************************
void psram_mspi_isr(void)
{
   uint32_t      ui32Status;

#ifdef SYSTEM_VIEW
   traceISR_ENTER();
#endif

   am_hal_mspi_interrupt_status_get(g_pMSPIPsramHandle, &ui32Status, false);

   am_hal_mspi_interrupt_clear(g_pMSPIPsramHandle, ui32Status);

   am_hal_mspi_interrupt_service(g_pMSPIPsramHandle, ui32Status);

#ifdef SYSTEM_VIEW
   traceISR_EXIT();
#endif
}

//We need to set up a small heap to be used by Clib, delete this may cause unexpected error.
__attribute__ ((section(".heap")))
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
    uint32_t ui32Status;
    //
    // Configure the board for low power operation.
    //
    am_bsp_low_power_init();
    //
    // Initialize the printf interface for ITM/SWO output.
    //
//    am_bsp_itm_printf_enable();

    // Set GPIO102 high to enable VDD18 load switch
    am_hal_gpio_pinconfig(AM_BSP_GPIO_VDD18_SWITCH, g_AM_BSP_GPIO_VDD18_SWITCH);
    am_hal_gpio_output_set(AM_BSP_GPIO_VDD18_SWITCH);
    
    //
    // Print the banner.
    //
    am_util_stdio_terminal_clear();
    am_util_stdio_printf("Apollo4 DAXI Test Cases\n\n");

    //
    // Enable global IRQ.
    //
    am_hal_interrupt_master_enable();
    //
    // DAXI init
    //
    am_util_stdio_printf("Default DAXI setting is 0x%08X.\n", CPU->DAXICFG);
    am_util_stdio_printf("Default cache setting is 0x%08X.\n", CPU->CACHECFG);
    daxi_test_cfg_init();
    cache_test_cfg_init();

    //
    // Power up extended memory.
    //
    am_hal_pwrctrl_dsp_memory_config_t cfg = g_DefaultDSPMemCfg;
    am_hal_pwrctrl_dsp_memory_config(AM_HAL_DSP0, &cfg);
#if !defined(AM_PART_APOLLO4P)
    am_hal_pwrctrl_dsp_memory_config(AM_HAL_DSP1, &cfg);
#endif
    //
    // Configure the MSPI and PSRAM Device.
    //
#ifdef AM_PART_APOLLO4P
#ifdef WINBOND_PSRAM
    ui32Status = am_devices_mspi_psram_w958d6nw_ddr_init(MSPI_PSRAM_MODULE, &g_sMspiPsramConfig, &g_pPsramHandle, &g_pMSPIPsramHandle);
#else
    ui32Status = am_devices_mspi_psram_aps25616n_ddr_init(MSPI_PSRAM_MODULE, &g_sMspiPsramConfig, &g_pPsramHandle, &g_pMSPIPsramHandle);
#endif
#else
    ui32Status = am_devices_mspi_psram_aps12808l_ddr_init(MSPI_PSRAM_MODULE, &g_sMspiPsramConfig, &g_pPsramHandle, &g_pMSPIPsramHandle);
#endif
    TEST_ASSERT_TRUE(AM_DEVICES_MSPI_PSRAM_STATUS_SUCCESS == ui32Status);
    if (AM_DEVICES_MSPI_PSRAM_STATUS_SUCCESS != ui32Status)
    {
        am_util_stdio_printf("Failed to configure the MSPI and PSRAM Device correctly!\n");
    }
    NVIC_SetPriority(MspiInterrupts[MSPI_PSRAM_MODULE], PSRAM_ISR_PRIORITY);
    NVIC_EnableIRQ(MspiInterrupts[MSPI_PSRAM_MODULE]);

    //
    // Enable XIP mode.
    //
#ifdef AM_PART_APOLLO4P
#ifdef WINBOND_PSRAM
    ui32Status = am_devices_mspi_psram_w958d6nw_ddr_enable_xip(g_pPsramHandle);
#else
    ui32Status = am_devices_mspi_psram_aps25616n_ddr_enable_xip(g_pPsramHandle);
#endif
#else
    ui32Status = am_devices_mspi_psram_aps12808l_ddr_enable_xip(g_pPsramHandle);
#endif
    TEST_ASSERT_TRUE(AM_DEVICES_MSPI_PSRAM_STATUS_SUCCESS == ui32Status);
    if (AM_DEVICES_MSPI_PSRAM_STATUS_SUCCESS != ui32Status)
    {
        am_util_stdio_printf("Failed to enable XIP mode in the MSPI!\n");
    }


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
    // Clear GPIO102 to disable VDD18 load switch
    am_hal_gpio_output_clear(AM_BSP_GPIO_VDD18_SWITCH);
}

//*****************************************************************************
//
// Main Function
//
//*****************************************************************************
int
daxi_enhanced_stress_test(void)
{
    //
    // Initialize the printf interface for ITM output
    //
    am_bsp_debug_printf_enable();

    //
    // Enable printing to the console.
    //
#ifdef AM_DEBUG_PRINTF
    enable_print_interface();
#endif

    PWRCTRL->SSRAMPWREN = PWRCTRL_SSRAMPWREN_PWRENSSRAM_ALL;  // enable all
    while(PWRCTRL->SSRAMPWRST == 0);

    //
    // Initialize timer
    //
    am_hal_timer_config_t       TimerConfig;
    am_hal_timer_default_config_set(&TimerConfig);
    TimerConfig.eInputClock = AM_HAL_TIMER_CLOCK_HFRC_DIV16;
    TimerConfig.eFunction = AM_HAL_TIMER_FN_EDGE;

    am_hal_timer_config(0, &TimerConfig);
    am_hal_timer_start(0);

    //
    // Initialize plotting interface.
    //
    am_util_debug_printf("daxi_enhanced_stress_test Example\n");

    run_tasks();

    //
    // We shouldn't ever get here.
    //
    while (1)
    {
    }

}

