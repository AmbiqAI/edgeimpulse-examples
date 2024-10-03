//*****************************************************************************
//
//! @file nemagfx_vg_circular_bar.c
//!
//! @brief NemaGFX Vector Graphics Circular Bar Example.
//!
//! @addtogroup graphics_examples Graphics Examples
//!
//! @defgroup nemagfx_vg_circular_bar NemaGFX Vector Graphics Circular Bar Example
//! @ingroup graphics_examples
//! @{
//!
//! Purpose: This example draw a circular bar image.See the watch_face.png in
//! resource folder.
//
//*****************************************************************************
//*****************************************************************************
//
// ${copyright}
//
// This is part of revision ${version} of the AmbiqSuite Development Package.
//
//*****************************************************************************

#include "nemagfx_vg_circular_bar.h"

#include "psram.h"

//*****************************************************************************
//
// Global Variables
//
//*****************************************************************************
static uint32_t        g_ui32DMATCBBuffer[2560];
void                   *g_pPSRAMHandle;
void                   *g_pMSPIHandle;

am_devices_mspi_psram_config_t s_PSRAM_Config =
{
#if defined(AM_BSP_MSPI_PSRAM_DEVICE_APS12808L)
    .eDeviceConfig            = AM_BSP_MSPI_PSRAM_MODULE_OCTAL_DDR_CE,
#elif defined(AM_BSP_MSPI_PSRAM_DEVICE_APS25616N)
    .eDeviceConfig            = AM_HAL_MSPI_FLASH_HEX_DDR_CE0,
#endif
    .eClockFreq               = AM_HAL_MSPI_CLK_96MHZ,
    .ui32NBTxnBufLength       = sizeof(g_ui32DMATCBBuffer) / sizeof(uint32_t),
    .pNBTxnBuf                = g_ui32DMATCBBuffer,
    .ui32ScramblingStartAddr  = 0,
    .ui32ScramblingEndAddr    = 0,
};

//! MSPI interrupts.
static const IRQn_Type MspiInterrupts[] =
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
void psram_mspi_isr(void)
{
   uint32_t      ui32Status;

#ifdef SYSTEM_VIEW
   traceISR_ENTER();
#endif

   am_hal_mspi_interrupt_status_get(g_pMSPIHandle, &ui32Status, false);

   am_hal_mspi_interrupt_clear(g_pMSPIHandle, ui32Status);

   am_hal_mspi_interrupt_service(g_pMSPIHandle, ui32Status);

#ifdef SYSTEM_VIEW
   traceISR_EXIT();
#endif
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
    // Configure the board for low power operation.
    //
    am_bsp_low_power_init();

    //Power on extended ram(DSP ram).
    am_hal_pwrctrl_dsp_memory_config_t sDSPMemCfg = g_DefaultDSPMemCfg;
    am_hal_pwrctrl_dsp_memory_config(AM_HAL_DSP0, &sDSPMemCfg);
    am_hal_pwrctrl_dsp_memory_config(AM_HAL_DSP1, &sDSPMemCfg);

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

    //
    // Configure the MSPI and PSRAM Device.
    //
#if defined(AM_BSP_MSPI_PSRAM_DEVICE_APS12808L)
    uint32_t ui32Status = am_devices_mspi_psram_aps12808l_ddr_init(MSPI_PSRAM_MODULE, &s_PSRAM_Config, &g_pPSRAMHandle, &g_pMSPIHandle);
#elif defined(AM_BSP_MSPI_PSRAM_DEVICE_APS25616N)
    uint32_t ui32Status = am_devices_mspi_psram_aps25616n_ddr_init(MSPI_PSRAM_MODULE, &s_PSRAM_Config, &g_pPSRAMHandle, &g_pMSPIHandle);
#endif

    if (AM_DEVICES_MSPI_PSRAM_STATUS_SUCCESS != ui32Status)
    {
        am_util_stdio_printf("Failed to configure the MSPI and PSRAM Device correctly!\n");
    }
    NVIC_SetPriority(MspiInterrupts[MSPI_PSRAM_MODULE], PSRAM_ISR_PRIORITY);
    NVIC_EnableIRQ(MspiInterrupts[MSPI_PSRAM_MODULE]);

    //
    // Enable XIP mode.
    //
#if defined(AM_BSP_MSPI_PSRAM_DEVICE_APS12808L)
    ui32Status = am_devices_mspi_psram_aps12808l_ddr_enable_xip(g_pPSRAMHandle);
#elif defined(AM_BSP_MSPI_PSRAM_DEVICE_APS25616N)
    ui32Status = am_devices_mspi_psram_aps25616n_ddr_enable_xip(g_pPSRAMHandle);
#endif

    if (AM_DEVICES_MSPI_PSRAM_STATUS_SUCCESS != ui32Status)
    {
        am_util_stdio_printf("Failed to enable XIP mode in the MSPI!\n");
    }

    //
    // Initialize plotting interface.
    //
    am_util_stdio_printf("nemagfx_vg_circular_bar Example\n");

#ifdef RUN_IN_HP_MODE
    if ( am_hal_pwrctrl_mcu_mode_select(AM_HAL_PWRCTRL_MCU_MODE_HIGH_PERFORMANCE) != AM_HAL_STATUS_SUCCESS )
    {
        am_util_stdio_printf("HP_LP:Enter HP mode failed!\n");
    }
#endif

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

