//*****************************************************************************
//
//! @file common.c
//!
//! @brief Initialize the mspi psram.
//!
//! AM_DEBUG_PRINTF
//! If enabled, debug messages will be sent over ITM.
//!
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
// Global includes for this project.
//
//*****************************************************************************
#include "nemagfx_enhanced_stress_test.h"
#if defined(PSRAM_APS25616N)
#include "am_devices_mspi_psram_aps25616n.c" 
#elif defined(PSRAM_W958D6NW)
#include "am_devices_mspi_psram_w958d6nw.c" 
#else
#error "Unknown PSRAM Device"
#endif

//
// Typedef - to encapsulate device driver functions
//
mspi_psram_device_func_t mspi_psram_device_func = 
{
#if defined(PSRAM_APS12808L)
    .devName = "MSPI DDR PSRAM",
    .mspi_init = am_devices_mspi_psram_aps12808l_ddr_init,
    .mspi_term = am_devices_mspi_psram_aps12808l_ddr_deinit,
    .mspi_read = am_devices_mspi_psram_aps12808l_ddr_read,
    .mspi_nonblocking_read = am_devices_mspi_psram_aps12808l_ddr_nonblocking_read
    .mspi_write = am_devices_mspi_psram_aps12808l_ddr_write,    
    .mspi_xip_enable = am_devices_mspi_psram_aps12808l_ddr_enable_xip,
    .mspi_xip_disable = am_devices_mspi_psram_aps12808l_ddr_disable_xip,
#elif defined(PSRAM_APS25616N)
    .devName = "MSPI HEX DDR PSRAM",
    .mspi_init = am_devices_mspi_psram_aps25616n_ddr_init,
    .mspi_term = am_devices_mspi_psram_aps25616n_ddr_deinit,
    .mspi_read = am_devices_mspi_psram_aps25616n_ddr_read,
    .mspi_nonblocking_read = am_devices_mspi_psram_aps25616n_ddr_nonblocking_read,
    .mspi_write = am_devices_mspi_psram_aps25616n_ddr_write,
    .mspi_xip_enable = am_devices_mspi_psram_aps25616n_ddr_enable_xip,
    .mspi_xip_disable = am_devices_mspi_psram_aps25616n_ddr_disable_xip,
    .mspi_init_timing_check = am_devices_mspi_psram_aps25616n_ddr_init_timing_check,
    .mspi_init_timing_apply = am_devices_mspi_psram_aps25616n_apply_ddr_timing,
#elif defined(PSRAM_W958D6NW)
    .devName = "MSPI HEX DDR WINBOND PSRAM",
    .mspi_init = am_devices_mspi_psram_w958d6nw_ddr_init,
    .mspi_term = am_devices_mspi_psram_w958d6nw_ddr_deinit,
    .mspi_read = am_devices_mspi_psram_w958d6nw_ddr_read,
    .mspi_nonblocking_read = am_devices_mspi_psram_w958d6nw_ddr_nonblocking_read,
    .mspi_write = am_devices_mspi_psram_w958d6nw_ddr_write,
    .mspi_xip_enable = am_devices_mspi_psram_w958d6nw_ddr_enable_xip,
    .mspi_xip_disable = am_devices_mspi_psram_w958d6nw_ddr_disable_xip,
    .mspi_init_timing_check = am_devices_mspi_psram_hex_ddr_init_timing_check,
    .mspi_init_timing_apply = am_devices_mspi_psram_apply_hex_ddr_timing, 
#else
#pragma message("Unknown PSRAM Device")   
#endif    
};

static uint32_t ui32DMATCBBuffer2[2560];
void            *g_pPsramHandle;
void            *g_pMSPIPsramHandle;


#if defined(PSRAM_APS12808L)
am_devices_mspi_psram_config_t g_sMspiPsramConfig =
{
    .eDeviceConfig            = PSRAM_DEVICE_CONFIG,
    .eClockFreq               = AM_HAL_MSPI_CLK_96MHZ,
    .ui32NBTxnBufLength       = sizeof(ui32DMATCBBuffer2) / sizeof(uint32_t),
    .pNBTxnBuf                = ui32DMATCBBuffer2,
    .ui32ScramblingStartAddr  = 0,
    .ui32ScramblingEndAddr    = 0,
};
#elif defined(PSRAM_APS25616N)
am_devices_mspi_psram_config_t g_sMspiPsramConfig =
{
    .eDeviceConfig            = PSRAM_DEVICE_CONFIG,
    .eClockFreq               = AM_HAL_MSPI_CLK_96MHZ,
    .ui32NBTxnBufLength       = sizeof(ui32DMATCBBuffer2) / sizeof(uint32_t),
    .pNBTxnBuf                = ui32DMATCBBuffer2,
    .ui32ScramblingStartAddr  = 0,
    .ui32ScramblingEndAddr    = 0,
};
#elif defined(PSRAM_W958D6NW)
am_devices_mspi_psram_config_t g_sMspiPsramConfig =
{
    .eDeviceConfig            = PSRAM_DEVICE_CONFIG,
    .eClockFreq               = AM_HAL_MSPI_CLK_96MHZ,
    .ui32NBTxnBufLength       = sizeof(ui32DMATCBBuffer2) / sizeof(uint32_t),
    .pNBTxnBuf                = ui32DMATCBBuffer2,
    .ui32ScramblingStartAddr  = 0,
    .ui32ScramblingEndAddr    = 0,
};
#endif

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

   am_hal_mspi_interrupt_status_get(g_pMSPIPsramHandle, &ui32Status, false);

   am_hal_mspi_interrupt_clear(g_pMSPIPsramHandle, ui32Status);

   am_hal_mspi_interrupt_service(g_pMSPIPsramHandle, ui32Status);

#ifdef SYSTEM_VIEW
   traceISR_EXIT();
#endif
}

uint32_t mspi_psram_init(void)
{
    uint32_t ui32Status;
#if !defined(APOLLO4_FPGA) && defined(am_devices_mspi_psram_timing_config_t)
    //
    // Run MSPI DDR timing scan
    //
    am_devices_mspi_psram_ddr_timing_config_t MSPIDdrTimingConfig;
    am_util_stdio_printf("Starting MSPI DDR Timing Scan: \n");
    if ( AM_DEVICES_MSPI_PSRAM_STATUS_SUCCESS == mspi_psram_device_func.mspi_init_timing_check(MSPI_PSRAM_MODULE, &g_sMspiPsramConfig, &MSPIDdrTimingConfig) )
    {
        am_util_debug_printf("==== Scan Result: RXDQSDELAY0 = %d \n", MSPIDdrTimingConfig.ui32Rxdqsdelay);
    }
    else
    {
        am_util_debug_printf("==== Scan Result: Failed, no valid setting.  \n");
    }
#endif

    //
    // Configure the MSPI and PSRAM Device.
    //
    ui32Status = mspi_psram_device_func.mspi_init(MSPI_PSRAM_MODULE, &g_sMspiPsramConfig, &g_pPsramHandle, &g_pMSPIPsramHandle);
    if (AM_DEVICES_MSPI_PSRAM_STATUS_SUCCESS != ui32Status)
    {
        am_util_stdio_printf("Failed to configure the MSPI and PSRAM Device correctly!\n");
        return MSPI_PSRAM_STATUS_ERROR;
    }
    NVIC_SetPriority(MspiInterrupts[MSPI_PSRAM_MODULE], PSRAM_ISR_PRIORITY);
    NVIC_EnableIRQ(MspiInterrupts[MSPI_PSRAM_MODULE]);

#if !defined(APOLLO4_FPGA) && defined(am_devices_mspi_psram_timing_config_t)
    //
    // Apply DDR timing setting
    //
    ui32Status = mspi_psram_device_func.mspi_init_timing_apply(g_pPsramHandle, &MSPIDdrTimingConfig);
    if (AM_HAL_STATUS_SUCCESS != ui32Status)
    {
        am_util_stdio_printf("Failed to apply the timming scan parameter!\n");
        return MSPI_PSRAM_STATUS_ERROR;
    }
#endif

    //
    // Enable XIP mode.
    //
    ui32Status = mspi_psram_device_func.mspi_xip_enable(g_pPsramHandle);
    if (AM_DEVICES_MSPI_PSRAM_STATUS_SUCCESS != ui32Status)
    {
        am_util_stdio_printf("Failed to enable XIP mode in the MSPI!\n");
        return MSPI_PSRAM_STATUS_ERROR;
    }  
  
    return MSPI_PSRAM_STATUS_SUCCESS;
}
