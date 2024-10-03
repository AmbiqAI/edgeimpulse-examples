//*****************************************************************************
//
//! @file nemagfx_coverflow.c
//!
//! @brief nemagfx_coverflow example.
//! this exampe demostration software AA at the edge of the picture
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


#include "am_devices_mspi_psram_aps25616n.h"
#include "am_devices_mspi_psram_aps25616n.c"


#ifdef LOAD_FROM_PSRAM
//AM_SHARED_RW uint32_t        g_ui32DMATCBBuffer[2560];
void            *g_pPSRAMHandle;
void            *g_pMSPIHandle;


am_devices_mspi_psram_config_t MSPI_PSRAM_HexCE1MSPIConfig =
{
    .eDeviceConfig            = AM_HAL_MSPI_FLASH_HEX_DDR_CE0,
    .eClockFreq               = AM_HAL_MSPI_CLK_48MHZ,
    .ui32NBTxnBufLength       = 0,
    .pNBTxnBuf                = NULL,
    .ui32ScramblingStartAddr  = 0,
    .ui32ScramblingEndAddr    = 0,
};

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

    //
    // Configure the board for low power operation.
    //
    am_bsp_low_power_init();

    //
    //  Enable the I-Cache and D-Cache.
    //
    am_hal_cachectrl_icache_enable();
    am_hal_cachectrl_dcache_enable(true);

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
    ui32Status = am_devices_mspi_psram_aps25616n_ddr_init(MSPI_PSRAM_MODULE, &MSPI_PSRAM_HexCE1MSPIConfig, &g_pPSRAMHandle, &g_pMSPIHandle);
    if (AM_DEVICES_MSPI_PSRAM_STATUS_SUCCESS != ui32Status)
    {
        am_util_stdio_printf("Failed to configure the MSPI and PSRAM Device correctly!\n");
    }
    NVIC_SetPriority(mspi_interrupts[MSPI_PSRAM_MODULE], AM_IRQ_PRIORITY_DEFAULT);
    NVIC_EnableIRQ(mspi_interrupts[MSPI_PSRAM_MODULE]);

    //
    // Enable XIP mode.
    //
    ui32Status = am_devices_mspi_psram_aps25616n_ddr_enable_xip(g_pPSRAMHandle);
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
    am_devices_display_init(RESX,
                            RESY,
                            COLOR_FORMAT_RGB888,
                            false);

    am_hal_pwrctrl_periph_enable(AM_HAL_PWRCTRL_PERIPH_GFX);
    //
    // Global interrupt enable
    //
    am_hal_interrupt_master_enable();

    //
    // Initialize NemaGFX
    //
    nema_init();

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

