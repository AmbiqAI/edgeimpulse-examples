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
#include "nemagfx_enhanced_stress_test.h"

#if (MSPI_TASK_ENABLE == 1)

#if defined(NOR_FLASH_ADESTO_ATXP032)
# include "am_devices_mspi_atxp032.c"
#elif defined(NAND_FLASH_DOSILICON_DS35X1GA)
# include "am_devices_mspi_ds35x1ga.c"
#else
# error "Unknown Flash Device"
#endif


#if defined(NAND_FLASH_DOSILICON_DS35X1GA)
static uint32_t mspi_ds35x1ga_init(uint32_t ui32Module,
                                           am_devices_mspi_flash_config_t *psMSPISettings,
                                           void **ppHandle, void **ppMspiHandle)
{
  uint32_t ui32Status;
  ui32Status = am_devices_mspi_ds35x1ga_init(ui32Module,
                                                (const am_devices_mspi_flash_config_t*)psMSPISettings,
                                               ppHandle, ppMspiHandle);
      if (MSPI_FLASH_STATUS_SUCCESS != ui32Status)
    {
        return MSPI_FLASH_STATUS_ERROR;
    }
    // Return the status.
    return MSPI_FLASH_STATUS_SUCCESS;
}
static uint32_t mspi_ds35x1ga_read(void *pHandle, uint8_t *pui8RxBuffer,
                                           uint32_t ui32ReadAddress,
                                           uint32_t ui32NumBytes,
                                           bool bWaitForCompletion)
{
    uint32_t ui32Status;
    uint8_t ui8EccStatus;

    ui32Status = am_devices_mspi_ds35x1ga_read(pHandle, ui32ReadAddress, pui8RxBuffer, ui32NumBytes,
                                               NULL, 0, &ui8EccStatus);
  
    am_util_debug_printf("page %d read status 0x%x!\n", ui32ReadAddress, ui32Status);
    am_util_debug_printf("page %d read ECC status 0x%x!\n", ui32ReadAddress, ui8EccStatus);
    
    if (MSPI_FLASH_STATUS_SUCCESS != ui32Status)
    {
        return MSPI_FLASH_STATUS_ERROR;
    }
    // Return the status.
    return MSPI_FLASH_STATUS_SUCCESS;
}

static uint32_t mspi_ds35x1ga_write(void *pHandle, uint8_t *ui8TxBuffer,
                                            uint32_t ui32WriteAddress,
                                            uint32_t ui32NumBytes,
                                            bool bWaitForCompletion)
{
    uint32_t ui32Status;

    ui32Status = am_devices_mspi_ds35x1ga_write(pHandle, ui32WriteAddress, ui8TxBuffer,ui32NumBytes,
                                                NULL, 0);                   

    am_util_debug_printf("page %d write status 0x%x!\n", NAND_TEST_PAGE, ui32Status);
    if (MSPI_FLASH_STATUS_SUCCESS != ui32Status)
    {
        return MSPI_FLASH_STATUS_ERROR;
    }
    // Return the status.
    return MSPI_FLASH_STATUS_SUCCESS;
}
#endif


//
// Typedef - to encapsulate device driver functions
//
mspi_flash_device_func_t mspi_flash_device_func =
{
  
#if defined(NOR_FLASH_ADESTO_ATXP032)
    .devName = "MSPI FLASH ATXP032",
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

#elif defined(NAND_FLASH_DOSILICON_DS35X1GA)
    .devName = "MSPI FLASH DS35X1GA",
    .mspi_init = mspi_ds35x1ga_init,
    .mspi_term = am_devices_mspi_ds35x1ga_deinit,
    .mspi_read = mspi_ds35x1ga_read,
    .mspi_write = mspi_ds35x1ga_write,
    .mspi_mass_erase = NULL,
    .mspi_sector_erase = am_devices_mspi_ds35x1ga_block_erase,
    .mspi_xip_enable = NULL,
    .mspi_xip_disable = NULL,
    .mspi_init_timing_check = am_devices_mspi_ds35x1ga_sdr_init_timing_check,
    .mspi_init_timing_apply = am_devices_mspi_ds35x1ga_apply_sdr_timing,
    //    .mspi_init_timing_check = NULL,
    //    .mspi_init_timing_apply = NULL,
#else
#pragma message("Unknown FLASH Device")  
#endif  
};

static uint32_t ui32DMATCBBuffer1[2560];
void            *g_pMSPIFlashHandle;
void            *g_pFlashHandle;

#if defined(NOR_FLASH_ADESTO_ATXP032)
am_devices_mspi_flash_config_t g_sMspiFlashConfig =
{
    .eDeviceConfig = AM_HAL_MSPI_FLASH_OCTAL_CE0,
    .eClockFreq = AM_HAL_MSPI_CLK_48MHZ,
    .pNBTxnBuf = ui32DMATCBBuffer1,
    .ui32NBTxnBufLength = (sizeof(ui32DMATCBBuffer1)/sizeof(uint32_t)),
    .ui32ScramblingStartAddr = 0,
    .ui32ScramblingEndAddr = 0,
};
#elif defined(NAND_FLASH_DOSILICON_DS35X1GA)
am_devices_mspi_flash_config_t g_sMspiFlashConfig =
{
    .eDeviceConfig = AM_HAL_MSPI_FLASH_QUAD_CE0_1_1_4,
    .eClockFreq = AM_HAL_MSPI_CLK_48MHZ,
    .pNBTxnBuf = ui32DMATCBBuffer1,
    .ui32NBTxnBufLength = (sizeof(ui32DMATCBBuffer1)/sizeof(uint32_t)),
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

#ifdef SYSTEM_VIEW
   traceISR_ENTER();
#endif

    am_hal_mspi_interrupt_status_get(g_pMSPIFlashHandle, &ui32Status, false);

    am_hal_mspi_interrupt_clear(g_pMSPIFlashHandle, ui32Status);

    am_hal_mspi_interrupt_service(g_pMSPIFlashHandle, ui32Status);

#ifdef SYSTEM_VIEW
   traceISR_EXIT();
#endif
}

uint32_t mspi_flash_init(void)
{
    uint32_t ui32Status;

    //
    // we couldn't do timing scan on FPGA
    //     
#if !defined(APOLLO4_FPGA) && defined(am_devices_mspi_flash_timing_config_t)
    am_devices_mspi_flash_timing_config_t MSPITimingConfig;
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

#endif // APOLLO4_FPGA

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

#if !defined(APOLLO4_FPGA) && defined(am_devices_mspi_flash_timing_config_t)
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