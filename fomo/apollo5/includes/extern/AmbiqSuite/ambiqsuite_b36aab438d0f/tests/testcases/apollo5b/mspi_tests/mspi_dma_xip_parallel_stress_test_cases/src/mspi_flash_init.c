//*****************************************************************************
//
//! @file mspi_flash_init.c
//!
//! @brief Initialize the mspi flash.
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
#include "mspi_dma_xip_parallel_stress_test_cases.h"

#if (MSPI_TASK_ENABLE == 1)

#if defined(NOR_FLASH_ADESTO_ATXP032)
# include "am_devices_mspi_atxp032.c"
#elif defined(NOR_FLASH_ISSI_IS25WX064)
# include "am_devices_mspi_is25wx064.c"
#else
# error "Unknown Flash Device"
#endif

//
// Typedef - to encapsulate device driver functions
//
mspi_flash_device_func_t mspi_flash_device_func =
{
  
#if defined(NOR_FLASH_ADESTO_ATXP032)
    .devName = "ADESTO ATXP032",
    .mspi_init = am_devices_mspi_atxp032_init,
    .mspi_term = am_devices_mspi_atxp032_deinit,
    .mspi_read = am_devices_mspi_atxp032_read,
    .mspi_write = am_devices_mspi_atxp032_write,
    .mspi_mass_erase = am_devices_mspi_atxp032_mass_erase,
    .mspi_sector_erase = am_devices_mspi_atxp032_sector_erase,
    .mspi_xip_enable = am_devices_mspi_atxp032_enable_xip,
    .mspi_xip_disable = am_devices_mspi_atxp032_disable_xip,
    .mspi_init_timing_check = am_devices_mspi_atxp032_sdr_init_timing_check,
    .mspi_init_timing_apply = am_devices_mspi_atxp032_apply_sdr_timing,
    //    .mspi_init_timing_check = NULL,
    //    .mspi_init_timing_apply = NULL,
#elif defined(NOR_FLASH_ISSI_IS25WX064)
    .devName = "MSPI FLASH IS25WX064",
    .mspi_init = am_devices_mspi_is25wx064_init,
    .mspi_init_timing_check = am_devices_mspi_is25wx064_init_timing_check,
    .mspi_init_timing_apply = am_devices_mspi_is25wx064_apply_ddr_timing,
    .mspi_term = am_devices_mspi_is25wx064_deinit,
    .mspi_read_id = am_devices_mspi_is25wx064_id,
    .mspi_read = am_devices_mspi_is25wx064_read,
    .mspi_read_adv = am_devices_mspi_is25wx064_read_adv,
// ##### INTERNAL BEGIN #####
    .mspi_read_callback = am_devices_mspi_is25wx064_read_callback,
// ##### INTERNAL END #####
    .mspi_write = am_devices_mspi_is25wx064_write,
    .mspi_mass_erase = am_devices_mspi_is25wx064_mass_erase,
    .mspi_sector_erase = am_devices_mspi_is25wx064_sector_erase,
    .mspi_xip_enable = am_devices_mspi_is25wx064_enable_xip,
    .mspi_xip_disable = am_devices_mspi_is25wx064_disable_xip,
    .mspi_scrambling_enable = am_devices_mspi_is25wx064_enable_scrambling,
    .mspi_scrambling_disable = am_devices_mspi_is25wx064_disable_scrambling,
#else
#pragma message("Unknown FLASH Device")  
#endif  
};

AM_SHARED_RW uint32_t FlashDMATCBBuffer[2560];
void            *g_pMSPIFlashHandle;
void            *g_pFlashHandle;

am_devices_mspi_flash_timing_config_t MSPITimingConfig;

#define MSPI_TEST_FREQ  AM_HAL_MSPI_CLK_96MHZ

#define DEFAULT_TIMEOUT         10000

#if defined(NOR_FLASH_ADESTO_ATXP032)
am_devices_mspi_flash_config_t g_sMspiFlashConfig =
{
    .eDeviceConfig = AM_HAL_MSPI_FLASH_OCTAL_CE0,
    .eClockFreq = AM_HAL_MSPI_CLK_48MHZ,
    .pNBTxnBuf = FlashDMATCBBuffer,
    .ui32NBTxnBufLength = (sizeof(FlashDMATCBBuffer)/sizeof(uint32_t)),
    .ui32ScramblingStartAddr = 0,
    .ui32ScramblingEndAddr = 0,
};
#elif defined(NOR_FLASH_ISSI_IS25WX064)
am_devices_mspi_flash_config_t g_sMspiFlashConfig =
{
#if defined(AM_BSP_MSPI_FLASH_DEVICE_IS25WX064)
    .eDeviceConfig = AM_HAL_MSPI_FLASH_OCTAL_CE0_1_8_8,
#else
    .eDeviceConfig = AM_HAL_MSPI_FLASH_OCTAL_CE0,
#endif

    .eClockFreq = MSPI_TEST_FREQ,
    .pNBTxnBuf = FlashDMATCBBuffer,
    .ui32NBTxnBufLength = (sizeof(FlashDMATCBBuffer) / sizeof(uint32_t)),
    .ui32ScramblingStartAddr = 0,
    .ui32ScramblingEndAddr = 0,
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
#define flash_mspi_isr                                                          \
    am_mspi_isr1(MSPI_FLASH_MODULE)
#define am_mspi_isr1(n)                                                        \
    am_mspi_isr(n)
#define am_mspi_isr(n)                                                         \
    am_mspi ## n ## _isr

//*****************************************************************************
//
// MSPI ISRs.
//
//*****************************************************************************
void flash_mspi_isr(void)
{
    uint32_t      ui32Status;
    am_hal_mspi_interrupt_status_get(g_pMSPIFlashHandle, &ui32Status, false);

    am_hal_mspi_interrupt_clear(g_pMSPIFlashHandle, ui32Status);

    am_hal_mspi_interrupt_service(g_pMSPIFlashHandle, ui32Status);
}

extern const uint8_t* clkfreq_string(am_hal_mspi_clock_e clk);
extern const uint8_t* device_mode_string(am_hal_mspi_device_e mode);
uint32_t mspi_flash_init(void)
{
    uint32_t ui32Status;

    // Invalidate D-Cache to make sure CPU sees the new data when loaded
    //am_hal_cachectrl_dcache_invalidate(NULL, false);
	
    am_util_stdio_printf("Flash device name is %s, MSPI#%d interface mode is %s @ %s MHz !\n\n",\
                        mspi_flash_device_func.devName,         \
                        MSPI_FLASH_MODULE ,                     \
                        device_mode_string(g_sMspiFlashConfig.eDeviceConfig), 
                        clkfreq_string(g_sMspiFlashConfig.eClockFreq) );
    //
    // we couldn't do timing scan on FPGA
    //     
#if !defined(APOLLO5_FPGA) && defined(am_devices_mspi_flash_timing_config_t)

    am_util_stdio_printf("Starting MSPI SDR Timing Scan: \n");
    if (mspi_flash_device_func.mspi_init_timing_check != NULL)
    {
        ui32Status = mspi_flash_device_func.mspi_init_timing_check(MSPI_FLASH_MODULE,
                                                               (void *)&g_sMspiFlashConfig,
                                                               &MSPITimingConfig);
        if (MSPI_FLASH_STATUS_SUCCESS != ui32Status)
        {
            am_util_stdio_printf("Scan Result: Failed, no valid setting.  \n");
            return MSPI_FLASH_STATUS_ERROR;
        }             
    }

#endif // APOLLO5_FPGA

    //
    // Configure the MSPI and Flash Device.
    //
    ui32Status = mspi_flash_device_func.mspi_init(MSPI_FLASH_MODULE, (void *)&g_sMspiFlashConfig, &g_pFlashHandle, &g_pMSPIFlashHandle);
    if (MSPI_FLASH_STATUS_SUCCESS != ui32Status)
    {
        am_util_stdio_printf("Failed to configure the MSPI and Flash Device correctly!\n");
        return MSPI_FLASH_STATUS_ERROR;
    }
    NVIC_SetPriority(MspiInterrupts[MSPI_FLASH_MODULE], FLASH_ISR_PRIORITY);
    NVIC_EnableIRQ(MspiInterrupts[MSPI_FLASH_MODULE]);

#if !defined(APOLLO5_FPGA) && defined(am_devices_mspi_flash_timing_config_t)
    //
    //  Set the SDR timing from previous scan.
    //
    if (mspi_flash_device_func.mspi_init_timing_apply != NULL)
    {
        ui32Status = mspi_flash_device_func.mspi_init_timing_apply(g_pFlashHandle, &MSPITimingConfig);  
        if (MSPI_FLASH_STATUS_SUCCESS != ui32Status)
        {
            am_util_stdio_printf("Failed to apply mspi timing .  \n");
            return MSPI_FLASH_STATUS_ERROR;
        }   
    }    
#endif

    //
    // Make sure we aren't in XIP mode.
    //
    if (mspi_flash_device_func.mspi_xip_disable != NULL)
    {
        ui32Status = mspi_flash_device_func.mspi_xip_disable(g_pFlashHandle);
        if (MSPI_FLASH_STATUS_SUCCESS != ui32Status)
        {
            am_util_stdio_printf("Failed to disable XIP mode in the MSPI!\n");
            return MSPI_FLASH_STATUS_ERROR;
        }        
    }  
    
    return MSPI_FLASH_STATUS_SUCCESS;  
}

#endif