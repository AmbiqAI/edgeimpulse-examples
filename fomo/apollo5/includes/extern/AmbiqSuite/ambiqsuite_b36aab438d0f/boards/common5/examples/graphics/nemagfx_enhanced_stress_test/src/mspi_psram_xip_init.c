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
#include "am_devices_mspi_psram_aps25616n.h"
#elif defined(PSRAM_APS25616BA)
#include "am_devices_mspi_psram_aps25616ba_1p2v.h"
#elif defined(PSRAM_W958D6NW)
#include "am_devices_mspi_psram_w958d6nw.c"
#else
#error "Unknown PSRAM Device"
#endif

//
// Typedef - to encapsulate device driver functions
//
static mspi_psram_device_func_t mspi_psram_device_func =
{
#if defined(PSRAM_APS12808L)
    .devName = "APS12808",
    .mspi_init = am_devices_mspi_psram_aps12808l_ddr_init,
    .mspi_term = am_devices_mspi_psram_aps12808l_ddr_deinit,
    .mspi_read = am_devices_mspi_psram_aps12808l_ddr_read,
    .mspi_nonblocking_read = am_devices_mspi_psram_aps12808l_ddr_nonblocking_read
    .mspi_write = am_devices_mspi_psram_aps12808l_ddr_write,
    .mspi_xip_enable = am_devices_mspi_psram_aps12808l_ddr_enable_xip,
    .mspi_xip_disable = am_devices_mspi_psram_aps12808l_ddr_disable_xip,
#elif defined(PSRAM_APS25616N)
    .devName = "APS25616N",
    .mspi_init = am_devices_mspi_psram_aps25616n_ddr_init,
    .mspi_term = am_devices_mspi_psram_aps25616n_ddr_deinit,
    .mspi_read = am_devices_mspi_psram_aps25616n_ddr_read,
    .mspi_nonblocking_read = am_devices_mspi_psram_aps25616n_ddr_nonblocking_read,
    .mspi_write = am_devices_mspi_psram_aps25616n_ddr_write,
    .mspi_xip_enable = am_devices_mspi_psram_aps25616n_ddr_enable_xip,
    .mspi_xip_disable = am_devices_mspi_psram_aps25616n_ddr_disable_xip,
    .mspi_init_timing_check = am_devices_mspi_psram_aps25616n_ddr_init_timing_check,
    .mspi_init_timing_apply = am_devices_mspi_psram_aps25616n_apply_ddr_timing,
#elif defined(PSRAM_APS25616BA)
    .devName = "APS25616BA",
    .mspi_init = am_devices_mspi_psram_aps25616ba_ddr_init,
    .mspi_term = am_devices_mspi_psram_aps25616ba_ddr_deinit,
    .mspi_read = am_devices_mspi_psram_aps25616ba_ddr_read,
    .mspi_nonblocking_read = am_devices_mspi_psram_aps25616ba_ddr_nonblocking_read,
    .mspi_write = am_devices_mspi_psram_aps25616ba_ddr_write,
    .mspi_xip_enable = am_devices_mspi_psram_aps25616ba_ddr_enable_xip,
    .mspi_xip_disable = am_devices_mspi_psram_aps25616ba_ddr_disable_xip,
    .mspi_init_timing_check = am_devices_mspi_psram_aps25616ba_ddr_init_timing_check,
    .mspi_init_timing_apply = am_devices_mspi_psram_aps25616ba_apply_ddr_timing,
#elif defined(PSRAM_W958D6NW)
    .devName = "WINBOND W958D6NW",
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

AM_SHARED_RW uint32_t ui32DMATCBBufferPSRAMXIP[2560];
void            *g_pPsramXipCodeHandle;
void            *g_pMSPIPsramXipCodeHandle;


#if defined(PSRAM_APS12808L)
am_devices_mspi_psram_config_t g_sMspiPsramConfig =
{
    .eDeviceConfig            = PSRAM_DEVICE_CONFIG,
    .eClockFreq               = AM_HAL_MSPI_CLK_96MHZ,
    .ui32NBTxnBufLength       = sizeof(ui32DMATCBBufferPSRAMXIP) / sizeof(uint32_t),
    .pNBTxnBuf                = ui32DMATCBBuffer2,
    .ui32ScramblingStartAddr  = 0x00010000,
    .ui32ScramblingEndAddr    = 0x00010000,
};
#elif defined(PSRAM_APS25616N)
static am_devices_mspi_psram_config_t g_sMspiPsramConfig =
{
    .eDeviceConfig            = AM_HAL_MSPI_FLASH_HEX_DDR_CE0,
    .eClockFreq               = AM_HAL_MSPI_CLK_96MHZ,
    .ui32NBTxnBufLength       = sizeof(ui32DMATCBBufferPSRAMXIP) / sizeof(uint32_t),
    .pNBTxnBuf                = ui32DMATCBBufferPSRAMXIP,
    .ui32ScramblingStartAddr  = 0x00010000,
    .ui32ScramblingEndAddr    = 0x00010000,
};
#elif defined(PSRAM_APS25616BA)
static am_devices_mspi_psram_config_t g_sMspiPsramConfig =
{
    .eDeviceConfig            = AM_HAL_MSPI_FLASH_HEX_DDR_CE0,
    .eClockFreq               = AM_HAL_MSPI_CLK_96MHZ,
    .ui32NBTxnBufLength       = sizeof(ui32DMATCBBufferPSRAMXIP) / sizeof(uint32_t),
    .pNBTxnBuf                = ui32DMATCBBuffer2,
    .ui32ScramblingStartAddr  = 0x00010000,
    .ui32ScramblingEndAddr    = 0x00010000,
};
#elif defined(PSRAM_W958D6NW)
am_devices_mspi_psram_config_t g_sMspiPsramConfig =
{
    .eDeviceConfig            = PSRAM_DEVICE_CONFIG,
    .eClockFreq               = AM_HAL_MSPI_CLK_96MHZ,
    .ui32NBTxnBufLength       = sizeof(ui32DMATCBBufferPSRAMXIP) / sizeof(uint32_t),
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
    MSPI3_IRQn,
};

//
// Take over the interrupt handler for whichever MSPI we're using.
//
#define psram_mspi3_isr                                                          \
    am_mspi_isr1(MSPI_PSRAM_XIPCODE_MODULE)
#define am_mspi_isr1(n)                                                        \
    am_mspi_isr(n)
#define am_mspi_isr(n)                                                         \
    am_mspi ## n ## _isr


//*****************************************************************************
//
// MSPI ISRs.
//
//*****************************************************************************
void psram_mspi3_isr(void)
{
   uint32_t      ui32Status;

#ifdef SYSTEM_VIEW
   traceISR_ENTER();
#endif

   am_hal_mspi_interrupt_status_get(g_pMSPIPsramXipCodeHandle, &ui32Status, false);

   am_hal_mspi_interrupt_clear(g_pMSPIPsramXipCodeHandle, ui32Status);

   am_hal_mspi_interrupt_service(g_pMSPIPsramXipCodeHandle, ui32Status);

#ifdef SYSTEM_VIEW
   traceISR_EXIT();
#endif
}

//*****************************************************************************
//
// Set up MSPI clock for the device.
//
//*****************************************************************************
static const char* clkfreq_string(am_hal_mspi_clock_e clk)
{
    const char* clk_str = "error";
    switch(clk)
    {
        case AM_HAL_MSPI_CLK_250MHZ:
            clk_str = "250";
            break;
        case AM_HAL_MSPI_CLK_192MHZ:
            clk_str = "192";
            break;
        case AM_HAL_MSPI_CLK_125MHZ:
            clk_str = "125";
            break;
        case AM_HAL_MSPI_CLK_96MHZ:
            clk_str = "96";
            break;
        case AM_HAL_MSPI_CLK_48MHZ:
            clk_str = "48";
            break;
        default :
            clk_str = "error";
    }

    return clk_str;
}

static const char* device_mode_string(am_hal_mspi_device_e mode)
{
    const char* mode_str = "error";
    switch(mode)
    {
        case AM_HAL_MSPI_FLASH_SERIAL_CE0:
        case AM_HAL_MSPI_FLASH_SERIAL_CE1:
        case AM_HAL_MSPI_FLASH_SERIAL_CE0_3WIRE:
        case AM_HAL_MSPI_FLASH_SERIAL_CE1_3WIRE:
            mode_str = "Serial";
            break;
        case AM_HAL_MSPI_FLASH_DUAL_CE0:
        case AM_HAL_MSPI_FLASH_DUAL_CE1:
        case AM_HAL_MSPI_FLASH_DUAL_CE0_1_1_2:
        case AM_HAL_MSPI_FLASH_DUAL_CE1_1_1_2:
        case AM_HAL_MSPI_FLASH_DUAL_CE0_1_2_2:
        case AM_HAL_MSPI_FLASH_DUAL_CE1_1_2_2:
            mode_str = "Dual";
            break;
        case AM_HAL_MSPI_FLASH_QUAD_CE0:
        case AM_HAL_MSPI_FLASH_QUAD_CE1:
        case AM_HAL_MSPI_FLASH_QUAD_CE0_1_1_4:
        case AM_HAL_MSPI_FLASH_QUAD_CE1_1_1_4:
        case AM_HAL_MSPI_FLASH_QUAD_CE0_1_4_4:
        case AM_HAL_MSPI_FLASH_QUAD_CE1_1_4_4:
            mode_str = "Quad";
            break;
        case  AM_HAL_MSPI_FLASH_OCTAL_CE0:
        case  AM_HAL_MSPI_FLASH_OCTAL_CE1:
            mode_str = "Octal";
            break;
        case  AM_HAL_MSPI_FLASH_OCTAL_DDR_CE0:
        case  AM_HAL_MSPI_FLASH_OCTAL_DDR_CE1:
            mode_str = "DDR Octal";
            break;
        case AM_HAL_MSPI_FLASH_HEX_DDR_CE0:
        case AM_HAL_MSPI_FLASH_HEX_DDR_CE1:
            mode_str = "DDR Hex";
            break;
        default :
            mode_str = "error";
            break ;
    }

    return mode_str;
}

uint32_t mspi_psram_xip_code_init(void)
{
    uint32_t ui32Status;

    am_util_stdio_printf("PSRAM device name is %s, MSPI#%d interface mode is %s @ %s MHz !\n\n",\
                        mspi_psram_device_func.devName,         \
                        MSPI_PSRAM_XIPCODE_MODULE ,                     \
                        device_mode_string(g_sMspiPsramConfig.eDeviceConfig),
                        clkfreq_string(g_sMspiPsramConfig.eClockFreq) );


#if !defined(APOLLO5_FPGA) && defined(am_devices_mspi_psram_timing_config_t)
    //
    // Run MSPI DDR timing scan
    //
    am_devices_mspi_psram_ddr_timing_config_t MSPIDdrTimingConfig;
    am_util_stdio_printf("Starting MSPI DDR Timing Scan: \n");
    if ( AM_DEVICES_MSPI_PSRAM_STATUS_SUCCESS == mspi_psram_device_func.mspi_init_timing_check(MSPI_PSRAM_XIPCODE_MODULE, &g_sMspiPsramConfig, &MSPIDdrTimingConfig) )
    {
        //am_util_stdio_printf("==== Scan Result: RXDQSDELAY0 = %d \n", MSPIDdrTimingConfig.sTimingCfg.ui8RxDQSDelay);
        am_util_debug_printf("==== Scan Result: RXDQSDELAY0 = %d \n", MSPIDdrTimingConfig.ui32Rxdqsdelay);
    }
    else
    {
        am_util_stdio_printf("==== Scan Result: Failed, no valid setting.  \n");
        vErrorHandler();
    }
#endif



    //
    // Configure the MSPI and PSRAM Device.
    //
    ui32Status = mspi_psram_device_func.mspi_init(MSPI_PSRAM_XIPCODE_MODULE, &g_sMspiPsramConfig, &g_pPsramXipCodeHandle, &g_pMSPIPsramXipCodeHandle);
    if (AM_DEVICES_MSPI_PSRAM_STATUS_SUCCESS != ui32Status)
    {
        am_util_stdio_printf("Failed to configure the MSPI and PSRAM Device correctly!\n");
        return MSPI_PSRAM_STATUS_ERROR;
    }
    NVIC_SetPriority(MspiInterrupts[MSPI_PSRAM_XIPCODE_MODULE], PSRAM_ISR_PRIORITY);
    NVIC_EnableIRQ(MspiInterrupts[MSPI_PSRAM_XIPCODE_MODULE]);

#if !defined(APOLLO5_FPGA) && defined(am_devices_mspi_psram_timing_config_t)
    //
    // Apply DDR timing setting
    //
    ui32Status = mspi_psram_device_func.mspi_init_timing_apply(g_pPsramXipCodeHandle, &MSPIDdrTimingConfig);
    if (AM_HAL_STATUS_SUCCESS != ui32Status)
    {
        am_util_stdio_printf("Failed to apply the timming scan parameter!\n");
        return MSPI_PSRAM_STATUS_ERROR;
    }
#endif

    //
    // Enable XIP mode.
    //
    ui32Status = mspi_psram_device_func.mspi_xip_enable(g_pPsramXipCodeHandle);
    if (AM_DEVICES_MSPI_PSRAM_STATUS_SUCCESS != ui32Status)
    {
        am_util_stdio_printf("Failed to enable XIP mode in the MSPI!\n");
        return MSPI_PSRAM_STATUS_ERROR;
    }

    return MSPI_PSRAM_STATUS_SUCCESS;
}
