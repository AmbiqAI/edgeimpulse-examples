//*****************************************************************************
//
//! @file daxi_transition_effects.c
//!
//! @brief Run nemagfx_transition_effects with vary DAXI settings, record the
//! overall run time for each DAXI setting.
//!
//! The frame buffer is in PSRAM and texture is in SSRAM by default. This test
//! also supports cache test, please enable macro "CACHE_TEST" in config-template.ini
//! if you want to run it.
//!
//! We can also run this test in XIP mode by loading bin into PSRAM with the
//! loader example in
//! boards\common4\examples\interfaces\mspi_ddr_psram_loader
//! and following the guideline in this path.
//!
//! The transition effects include
//! NEMA_TRANS_LINEAR_H,
//! NEMA_TRANS_CUBE_H,
//! NEMA_TRANS_INNERCUBE_H,
//! NEMA_TRANS_STACK_H,
//! NEMA_TRANS_LINEAR_V,
//! NEMA_TRANS_CUBE_V,
//! NEMA_TRANS_INNERCUBE_V,
//! NEMA_TRANS_STACK_V,
//! NEMA_TRANS_FADE,
//! NEMA_TRANS_FADE_ZOOM,
//! NEMA_TRANS_MAX,
//! NEMA_TRANS_NONE
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
#include "stdint.h"
#include "nemagfx_transition_effects.h"
#include "am_devices_mspi_psram_aps25616n.h"

extern bool transition_effects(void);

// #### INTERNAL BEGIN ####
#if (defined(LOAD_FROM_PSRAM) || defined(FB_IN_PSRAM)) && !defined(XIP_EXECUTION)

uint32_t        DMATCBBuffer[2560];
void            *g_pDevHandle;
void            *g_pHandle;

//! MSPI interrupts.
static const IRQn_Type mspi_interrupts[] =
{
    MSPI0_IRQn,
#if defined(AM_PART_APOLLO3P) || defined(AM_PART_APOLLO4) || defined(AM_PART_APOLLO4B)
    MSPI1_IRQn,
    MSPI2_IRQn,
#endif
};

am_devices_mspi_psram_config_t MSPI_PSRAM_HexCE1MSPIConfig =
{
#ifdef APS25616N_OCTAL_MODE_EN
    .eDeviceConfig            = AM_HAL_MSPI_FLASH_OCTAL_DDR_CE1,
#else
    .eDeviceConfig            = AM_HAL_MSPI_FLASH_HEX_DDR_CE1,
#endif
    .eClockFreq               = AM_HAL_MSPI_CLK_96MHZ,
    .ui32NBTxnBufLength       = sizeof(DMATCBBuffer) / sizeof(uint32_t),
    .pNBTxnBuf                = DMATCBBuffer,
    .ui32ScramblingStartAddr  = 0,
    .ui32ScramblingEndAddr    = 0,
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
// MSPI ISRs.
//
//*****************************************************************************
void psram_mspi_isr(void)
{
    uint32_t      ui32Status;

    am_hal_mspi_interrupt_status_get(g_pHandle, &ui32Status, false);

    am_hal_mspi_interrupt_clear(g_pHandle, ui32Status);

    am_hal_mspi_interrupt_service(g_pHandle, ui32Status);
}

#endif // (defined(LOAD_FROM_PSRAM) || defined(FB_IN_PSRAM)) && !defined(XIP_EXECUTION)
// #### INTERNAL END ####

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
int32_t
daxi_transiton_effects_test(void)
{
    bool bTestPass = true;
    //
    // External power on
    //
    // am_bsp_external_pwr_on();
    am_util_delay_ms(100);
    am_bsp_low_power_init();
// #### INTERNAL BEGIN ####
    //
    // if DSI or MRAM textures are used, elevate VDDF
    //
    //vddc_vddf_trim();
#if 0
    //
    // Initialize DSP RAM.
    //
    am_hal_pwrctrl_dsp_memory_config_t sCfg = g_DefaultDSPMemCfg;
    sCfg.bEnableICache = 0;
    sCfg.bRetainCache = 0;
    am_hal_pwrctrl_dsp_memory_config(AM_HAL_DSP0, &sCfg);
    //am_hal_pwrctrl_dsp_memory_config(AM_HAL_DSP1, &sCfg);
#endif
 // #### INTERNAL END ####

    //
    // Initialize the printf interface for ITM/SWO output.
    //
    am_bsp_itm_printf_enable();
    //
    // Clear the terminal and print the banner.
    //
    am_util_stdio_terminal_clear();
    //
    // Global interrupt enable
    //
    am_hal_interrupt_master_enable();
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

// #### INTERNAL BEGIN ####
#if (defined(LOAD_FROM_PSRAM) || defined(FB_IN_PSRAM)) && !defined(XIP_EXECUTION)
    uint32_t ui32Status;

    //
    // Init PSRAM device.
    //
    ui32Status = am_devices_mspi_psram_aps25616n_ddr_init(MSPI_PSRAM_MODULE, &MSPI_PSRAM_HexCE1MSPIConfig, &g_pDevHandle, &g_pHandle);
    if (AM_DEVICES_MSPI_PSRAM_STATUS_SUCCESS != ui32Status)
    {
        am_util_stdio_printf("Failed to configure the MSPI and PSRAM Device correctly!\n");
    }
    NVIC_SetPriority(mspi_interrupts[MSPI_PSRAM_MODULE], AM_IRQ_PRIORITY_DEFAULT);
    NVIC_EnableIRQ(mspi_interrupts[MSPI_PSRAM_MODULE]);

    //
    // Enable XIP mode.
    //
    ui32Status = am_devices_mspi_psram_aps25616n_ddr_enable_xip(g_pDevHandle);
    if (AM_DEVICES_MSPI_PSRAM_STATUS_SUCCESS == ui32Status)
    {
        am_util_stdio_printf("Enable XIP mode in the MSPI!\n");
    }
    else
    {
        am_util_stdio_printf("Failed to enable XIP mode in the MSPI!\n");
    }
#endif // (defined(LOAD_FROM_PSRAM) || defined(FB_IN_PSRAM)) && !defined(XIP_EXECUTION)
// #### INTERNAL END ####
#ifndef USE_STIMER
    am_hal_timer_config_t sTimerConfig;
    uint32_t ui32Status;
    ui32Status = am_hal_timer_default_config_set(&sTimerConfig);
    if (AM_HAL_STATUS_SUCCESS != ui32Status)
    {
        am_util_stdio_printf("Failed to initialize a timer configuration structure with default values!\n");
    }
    sTimerConfig.eInputClock = AM_HAL_TIMER_CLOCK_HFRC_DIV16;
    sTimerConfig.eFunction = AM_HAL_TIMER_FN_UPCOUNT;
    ui32Status = am_hal_timer_config(0, &sTimerConfig);
    if (AM_HAL_STATUS_SUCCESS != ui32Status)
    {
        am_util_stdio_printf("Failed to configure a timer!\n");
    }
    ui32Status = am_hal_timer_start(0);
    if (AM_HAL_STATUS_SUCCESS != ui32Status)
    {
        am_util_stdio_printf("Failed to start a timer!\n");
    }
#endif
#ifdef BAREMETAL
    bTestPass = transition_effects();
#else //!< BAREMETAL
    //
    // Initialize plotting interface.
    //
    am_util_debug_printf("FreeRTOS NemaGFX_SW Transition Effect Example\n");

    //
    // Run the application.
    //
    run_tasks();
#endif //!< BAREMETAL
    return bTestPass;
}

