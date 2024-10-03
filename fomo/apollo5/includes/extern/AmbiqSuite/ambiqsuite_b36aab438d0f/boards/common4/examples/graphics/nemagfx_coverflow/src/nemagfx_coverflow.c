//*****************************************************************************
//
//! @file nemagfx_coverflow.c
//!
//! @brief NemaGFX Coverflow Example.
//!
//! @addtogroup graphics_examples Graphics Examples
//!
//! @defgroup nemagfx_coverflow NemaGFX Coverflow Example
//! @ingroup graphics_examples
//! @{
//!
//! Purpose: This example demostration software AA at the edge of the picture
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
#include "nemagfx_coverflow.h"
#include "coverflow.h"

#ifndef USE_HEX_MODE_PSRAM
#include "am_devices_mspi_psram_aps12808l.h"
#include "am_devices_mspi_psram_aps12808l.c"
#else
#include "am_devices_mspi_psram_aps25616n.h"
#include "am_devices_mspi_psram_aps25616n.c"
#endif

#ifdef LOAD_FROM_PSRAM
AM_SHARED_RW uint32_t        g_ui32DMATCBBuffer[2560];
void            *g_pPSRAMHandle;
void            *g_pMSPIHandle;
#ifdef AM_PART_APOLLO4B
// AXI Scratch buffer
// Need to allocate 20 Words even though we only need 16, to ensure we have 16 Byte alignment
AM_SHARED_RW uint32_t axiScratchBuf[20];
#endif
#ifndef USE_HEX_MODE_PSRAM
am_devices_mspi_psram_config_t MSPI_PSRAM_OctalCE1MSPIConfig =
{
    .eDeviceConfig            = AM_HAL_MSPI_FLASH_OCTAL_DDR_CE1,
    .eClockFreq               = AM_HAL_MSPI_CLK_96MHZ,
    .ui32NBTxnBufLength       = sizeof(g_ui32DMATCBBuffer) / sizeof(uint32_t),
    .pNBTxnBuf                = g_ui32DMATCBBuffer,
    .ui32ScramblingStartAddr  = 0,
    .ui32ScramblingEndAddr    = 0,
};
#else
am_devices_mspi_psram_config_t MSPI_PSRAM_HexCE1MSPIConfig =
{
#ifdef APS25616N_OCTAL_MODE_EN
    .eDeviceConfig            = AM_HAL_MSPI_FLASH_OCTAL_DDR_CE1,
#else
    .eDeviceConfig            = AM_HAL_MSPI_FLASH_HEX_DDR_CE1,
#endif
    .eClockFreq               = AM_HAL_MSPI_CLK_96MHZ,
    .ui32NBTxnBufLength       = sizeof(g_ui32DMATCBBuffer) / sizeof(uint32_t),
    .pNBTxnBuf                = g_ui32DMATCBBuffer,
    .ui32ScramblingStartAddr  = 0,
    .ui32ScramblingEndAddr    = 0,
};
#endif
//! MSPI interrupts.
static const IRQn_Type mspi_interrupts[] =
{
    MSPI0_IRQn,
    MSPI1_IRQn,
    MSPI2_IRQn,
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
void
psram_mspi_isr(void)
{
    uint32_t ui32Status;

    am_hal_mspi_interrupt_status_get(g_pMSPIHandle, &ui32Status, false);

    am_hal_mspi_interrupt_clear(g_pMSPIHandle, ui32Status);

    am_hal_mspi_interrupt_service(g_pMSPIHandle, ui32Status);
}
#endif // LOAD_FROM_PSRAM

// #### INTERNAL BEGIN ####
void
vddc_vddf_trim(void)
{
    am_util_delay_ms(3000);
    //#warning: need AI key  // AI Key
    //#warning: need AI key  // AI Key
    //#warning: need AI key  // AI Key
    //#warning: need AI key  // AI Key
    //mcuctrlpriv.ldoreg1.coreldoactivetrim 0x40040080 bit9~0
    uint32_t ui32Val0 = AM_REGVAL(0x40020080);
    //AM_REGVAL(0x40020080) = ui32Val0 + 24; //22; // please increase trim value with a proper value, to increase VDDC to 750mV.
    am_util_delay_ms(100);
    //mcuctrlpriv.ldoreg2.memldoactivetrim 0x40040088 bit5~0
    ui32Val0 = AM_REGVAL(0x40020088);
    AM_REGVAL(0x40020088) = ui32Val0 + 24;  //28; // please increase trim value with a proper value, to increase VDDF to 950mV.
    am_util_delay_ms(100);
}
// #### INTERNAL END ####

//*****************************************************************************
//
// Main Function
//
//*****************************************************************************
int
main(void)
{
    uint32_t ui32Status;
    //
    // External power on
    //
    am_bsp_external_pwr_on();
    am_util_delay_ms(100);
    am_bsp_low_power_init();
// #### INTERNAL BEGIN ####
    //
    // !!!! if DSI or MRAM textures are used, elevate VDDF !!!
    //
    //vddc_vddf_trim();
// #### INTERNAL END ####

    //
    // Initialize the printf interface for ITM/SWO output.
    //
    am_bsp_itm_printf_enable();
    //
    // Clear the terminal and print the banner.
    //
    am_util_stdio_terminal_clear();
#ifdef LOAD_FROM_PSRAM
#ifndef USE_HEX_MODE_PSRAM
    ui32Status = am_devices_mspi_psram_aps12808l_ddr_init(MSPI_PSRAM_MODULE, &MSPI_PSRAM_OctalCE1MSPIConfig, &g_pPSRAMHandle, &g_pMSPIHandle);
#else
    ui32Status = am_devices_mspi_psram_aps25616n_ddr_init(MSPI_PSRAM_MODULE, &MSPI_PSRAM_HexCE1MSPIConfig, &g_pPSRAMHandle, &g_pMSPIHandle);
#endif
    if (AM_DEVICES_MSPI_PSRAM_STATUS_SUCCESS != ui32Status)
    {
        am_util_stdio_printf("Failed to configure the MSPI and PSRAM Device correctly!\n");
    }
    NVIC_SetPriority(mspi_interrupts[MSPI_PSRAM_MODULE], AM_IRQ_PRIORITY_DEFAULT);
    NVIC_EnableIRQ(mspi_interrupts[MSPI_PSRAM_MODULE]);

    //
    // Enable XIP mode.
    //
#ifndef USE_HEX_MODE_PSRAM
    ui32Status = am_devices_mspi_psram_aps12808l_ddr_enable_xip(g_pPSRAMHandle);
#else
    ui32Status = am_devices_mspi_psram_aps25616n_ddr_enable_xip(g_pPSRAMHandle);
#endif
    if (AM_DEVICES_MSPI_PSRAM_STATUS_SUCCESS == ui32Status)
    {
        am_util_stdio_printf("Enable XIP mode in the MSPI!\n");
    }
    else
    {
        am_util_stdio_printf("Failed to enable XIP mode in the MSPI!\n");
    }
#endif // LOAD_FROM_PSRAM

#ifdef AM_DEBUG_PRINTF
    am_bsp_debug_printf_enable();
#endif

    //
    // Initialize display
    //
#if defined (AM_PART_APOLLO4B)
    //
    // DSI output frequency
    //
    g_sDispCfg.eDsiFreq = AM_HAL_DSI_FREQ_TRIM_X12;
    am_devices_display_init(RESX,
                            RESY,
                            COLOR_FORMAT_RGB565,
                            false);
#else
    am_devices_display_init(RESX,
                            RESY,
                            COLOR_FORMAT_RGB888,
                            false);
#endif

    am_hal_pwrctrl_periph_enable(AM_HAL_PWRCTRL_PERIPH_GFX);
    //
    // Global interrupt enable
    //
    am_hal_interrupt_master_enable();

    //
    // Initialize NemaGFX
    //
    nema_init();

    am_hal_timer_config_t sTimerConfig;

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

#ifdef BAREMETAL
    am_util_stdio_printf("BAREMETAL coverflow Example\n");
    cover_flow();
#else //!< BAREMETAL
    //
    // Initialize plotting interface.
    //
    am_util_debug_printf("FreeRTOS coverflow Example\n");

    //
    // Run the application.
    //
    run_tasks();
#endif //!< BAREMETAL

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

