//*****************************************************************************
//
//! @file nemagfx_test_common.c
//!
//! @brief NemaGFX test cases common source code.
//
//*****************************************************************************

//*****************************************************************************
//
// ${copyright}
//
// This is part of revision ${version} of the AmbiqSuite Development Package.
//
//*****************************************************************************
#include "nemagfx_test_common.h"

//*****************************************************************************
//
// PSRAM setting
//
//*****************************************************************************
#ifdef USE_PSRAM

#ifndef APOLLO5_FPGA
#define RUN_PSRAM_TIMING_SCAN
#endif

//*****************************************************************************
//
// Global Variables
//
//*****************************************************************************
//static uint32_t        ui32DMATCBBuffer[2560];
void            *g_pPsramHandle;
void            *g_pMSPIPsramHandle;

am_devices_mspi_psram_config_t g_sMspiPsramConfig =
{
    .eDeviceConfig            = AM_HAL_MSPI_FLASH_HEX_DDR_CE0,
#ifdef NEMAGFX_COMMON_USE_192M_PSRAM
    // 96MHz MSPI SCLK w/ DDR == 192MHz Mtransfers/s
    .eClockFreq               = AM_HAL_MSPI_CLK_192MHZ,
#else
    // 48MHz MSPI SCLK w/ DDR == 96MHz Mtransfers/s
    .eClockFreq               = AM_HAL_MSPI_CLK_96MHZ,
#endif
    .ui32NBTxnBufLength       = 0,
    .pNBTxnBuf                = NULL,
    .ui32ScramblingStartAddr  = 0,
    .ui32ScramblingEndAddr    = 0,
};
#endif

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
globalSetUp(void)
{

    //
    // Global interrupt enable
    //
    am_hal_interrupt_master_enable();

    //
    // Print the banner
    //
    am_util_stdio_terminal_clear();
    am_util_stdio_printf("apollo5 NemaGFX Test Cases\n\n");

#ifdef NEMA_USE_CUSTOM_MALLOC
    tcm_heap_init(LV_MEM_TCM_ADR, LV_MEM_TCM_SIZE, 256);
    ssram_heap_init(LV_MEM_SSRAM_ADR, LV_MEM_SSRAM_SIZE, 16*1024);
    psram_heap_init(LV_MEM_PSRAM_ADR, LV_MEM_PSRAM_SIZE, 64*1024);
#endif

    am_hal_pwrctrl_periph_enable(AM_HAL_PWRCTRL_PERIPH_GFX);

    //
    // Initialize NemaGFX
    //
    if(nema_init() != 0)
    {
        am_util_stdio_printf("GPU init failed!\n");
    }

#ifdef USE_PSRAM

    uint32_t ui32Status;

    //
    // Run MSPI DDR timing scan
    //
#ifdef RUN_PSRAM_TIMING_SCAN
    am_devices_mspi_psram_ddr_timing_config_t MSPIDdrTimingConfig;
    am_util_stdio_printf("Starting MSPI DDR Timing Scan: \n");
    if ( AM_DEVICES_MSPI_PSRAM_STATUS_SUCCESS == am_devices_mspi_psram_aps25616n_ddr_init_timing_check(MSPI_PSRAM_MODULE, &g_sMspiPsramConfig, &MSPIDdrTimingConfig) )
    {
        am_util_stdio_printf("==== Scan Result: RXDQSDELAY0 = %d \n", MSPIDdrTimingConfig.ui32Rxdqsdelay);
    }
    else
    {
        am_util_stdio_printf("==== Scan Result: Failed, no valid setting.  \n");
    }
#endif

    //
    // Configure the MSPI and PSRAM Device.
    //
    ui32Status = am_devices_mspi_psram_aps25616n_ddr_init(MSPI_PSRAM_MODULE, &g_sMspiPsramConfig, &g_pPsramHandle, &g_pMSPIPsramHandle);
    if (AM_DEVICES_MSPI_PSRAM_STATUS_SUCCESS != ui32Status)
    {
        am_util_stdio_printf("Failed to configure the MSPI and PSRAM Device correctly!\n");
    }

    //
    // Apply DDR timing setting
    //
#ifdef RUN_PSRAM_TIMING_SCAN
    ui32Status = am_devices_mspi_psram_aps25616n_apply_ddr_timing(g_pPsramHandle, &MSPIDdrTimingConfig);
    if (AM_HAL_STATUS_SUCCESS != ui32Status)
    {
        am_util_stdio_printf("Failed to apply the timming scan parameter!\n");
    }
#endif

    //
    // Enable XIP mode.
    //
    ui32Status = am_devices_mspi_psram_aps25616n_ddr_enable_xip(g_pPsramHandle);
    if (AM_DEVICES_MSPI_PSRAM_STATUS_SUCCESS != ui32Status)
    {
        am_util_stdio_printf("Failed to enable XIP mode in the MSPI!\n");
    }
#endif

}

void
globalTearDown(void)
{
#ifdef USE_PSRAM

    uint32_t ui32Status;

    //
    // Need to make sure all pending XIPMM transactions are flushed
    //
    am_hal_cachectrl_dcache_invalidate(NULL, true);

    ui32Status = am_devices_mspi_psram_aps25616n_ddr_disable_xip(g_pPsramHandle);
    if (AM_DEVICES_MSPI_PSRAM_STATUS_SUCCESS != ui32Status)
    {
        am_util_stdio_printf("Failed to disable XIP mode in the MSPI!\n");
    }

    ui32Status = am_devices_mspi_psram_aps25616n_ddr_deinit(g_pPsramHandle);
    if (AM_DEVICES_MSPI_PSRAM_STATUS_SUCCESS != ui32Status)
    {
        am_util_stdio_printf("Failed to deinit the MSPI and PSRAM Device correctly!\n");
    }
#endif
}
