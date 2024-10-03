//*****************************************************************************
//
//! @file mspi_dma.c
//!
//! @brief mspi dma opeariton.
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
#include "multi_dma.h"
#include "am_devices_mspi_psram_aps25616n.h"

#define AM_BSP_MSPI_FLASH_DEVICE_IS25WX064
//#define AM_BSP_MSPI_FLASH_DEVICE_ATXP032

#if defined(AM_BSP_MSPI_FLASH_DEVICE_IS25WX064)
#include "am_devices_mspi_is25wx064.h"
#else
#include "am_devices_mspi_atxp032.h"
#endif

//
//! the abstracted marco for the various mspi devices
//
#if defined(AM_BSP_MSPI_FLASH_DEVICE_ATXP032)
    #define am_abstract_mspi_devices_config_t         am_devices_mspi_atxp032_config_t
    #define am_abstract_mspi_devices_timing_config_t  am_devices_mspi_atxp032_sdr_timing_config_t
    #define AM_ABSTRACT_MSPI_SUCCESS                  AM_DEVICES_MSPI_ATXP032_STATUS_SUCCESS
    #define AM_ABSTRACT_MSPI_ERROR                    AM_DEVICES_MSPI_ATXP032_STATUS_ERROR
    #define AM_DEVICES_MSPI_FLASH_SECTOR_SHIFT        16
#elif defined(AM_BSP_MSPI_FLASH_DEVICE_IS25WX064)
    #define am_abstract_mspi_devices_config_t         am_devices_mspi_is25wx064_config_t
    #define am_abstract_mspi_devices_timing_config_t  am_devices_mspi_is25wx064_timing_config_t
    #define AM_ABSTRACT_MSPI_SUCCESS                  AM_DEVICES_MSPI_IS25WX064_STATUS_SUCCESS
    #define AM_ABSTRACT_MSPI_ERROR                    AM_DEVICES_MSPI_IS25WX064_STATUS_ERROR
    #define AM_DEVICES_MSPI_FLASH_SECTOR_SHIFT        17
#endif

//
//! the universal struct for the various mspi devices struct
//! each internal function pointer members refer to their different drivers
//
typedef struct
{
    uint8_t  devName[30];
    uint32_t (*mspi_init)(uint32_t ui32Module, am_abstract_mspi_devices_config_t *psMSPISettings, void **ppHandle, void **ppMspiHandle);
    uint32_t (*mspi_term)(void *pHandle);

    uint32_t (*mspi_read_id)(void *pHandle);

    uint32_t (*mspi_read)(void *pHandle, uint8_t *pui8RxBuffer,
                           uint32_t ui32ReadAddress,
                           uint32_t ui32NumBytes,
                           bool bWaitForCompletion);

    uint32_t (*mspi_read_adv)(void *pHandle, uint8_t *pui8RxBuffer,
                           uint32_t ui32ReadAddress,
                           uint32_t ui32NumBytes,
                           uint32_t ui32PauseCondition,
                           uint32_t ui32StatusSetClr,
                           am_hal_mspi_callback_t pfnCallback,
                           void *pCallbackCtxt);

    uint32_t (*mspi_read_callback)(void *pHandle, uint8_t *pui8RxBuffer,
                           uint32_t ui32ReadAddress,
                           uint32_t ui32NumBytes);

    uint32_t (*mspi_read_cb)(void *pHandle,
                           uint8_t *pui8RxBuffer,
                           uint32_t ui32ReadAddress,
                           uint32_t ui32NumBytes,
                           am_hal_mspi_callback_t pfnCallback,
                           void *pCallbackCtxt);

    uint32_t (*mspi_write)(void *pHandle, uint8_t *ui8TxBuffer,
                           uint32_t ui32WriteAddress,
                           uint32_t ui32NumBytes,
                           bool bWaitForCompletion);

    uint32_t (*mspi_write_adv)(void *pHandle,
                           uint8_t *puiTxBuffer,
                           uint32_t ui32WriteAddress,
                           uint32_t ui32NumBytes,
                           uint32_t ui32PauseCondition,
                           uint32_t ui32StatusSetClr,
                           am_hal_mspi_callback_t pfnCallback,
                           void *pCallbackCtxt);

    uint32_t (*mspi_mass_erase)(void *pHandle);
    uint32_t (*mspi_sector_erase)(void *pHandle, uint32_t ui32SectorAddress);

    uint32_t (*mspi_xip_enable)(void *pHandle);
    uint32_t (*mspi_xip_disable)(void *pHandle);
    uint32_t (*mspi_scrambling_enable)(void *pHandle);
    uint32_t (*mspi_scrambling_disable)(void *pHandle);
    uint32_t (*mspi_init_timing_check)(uint32_t ui32Module,
                                        am_abstract_mspi_devices_config_t *pDevCfg,
                                        am_abstract_mspi_devices_timing_config_t *pDevSdrCfg);
    uint32_t (*mspi_init_timing_apply)(void *pHandle,
                                        am_abstract_mspi_devices_timing_config_t *pDevSdrCfg);
} mspi_device_func_t;

mspi_device_func_t mspi_device_func =
{
#if defined(AM_BSP_MSPI_FLASH_DEVICE_IS25WX064)
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
    .mspi_read_cb = am_devices_mspi_is25wx064_read_cb,
// ##### INTERNAL END #####
    .mspi_write = am_devices_mspi_is25wx064_write,
    .mspi_mass_erase = am_devices_mspi_is25wx064_mass_erase,
    .mspi_sector_erase = am_devices_mspi_is25wx064_sector_erase,
    .mspi_xip_enable = am_devices_mspi_is25wx064_enable_xip,
    .mspi_xip_disable = am_devices_mspi_is25wx064_disable_xip,
    .mspi_scrambling_enable = am_devices_mspi_is25wx064_enable_scrambling,
    .mspi_scrambling_disable = am_devices_mspi_is25wx064_disable_scrambling,
#else
    .devName = "MSPI FLASH ATXP032",
    .mspi_init = am_devices_mspi_atxp032_init,
    .mspi_init_timing_check = am_devices_mspi_atxp032_sdr_init_timing_check,
    .mspi_init_timing_apply = am_devices_mspi_atxp032_apply_sdr_timing,
    .mspi_term = am_devices_mspi_atxp032_deinit,
    .mspi_read_id = am_devices_mspi_atxp032_id,
    .mspi_read = am_devices_mspi_atxp032_read,
    .mspi_read_adv = am_devices_mspi_atxp032_read_adv,
// ##### INTERNAL BEGIN #####
    .mspi_read_callback = am_devices_mspi_atxp032_read_cb,
    .mspi_read_cb = am_devices_mspi_is25wx064_read_cb,
// ##### INTERNAL END #####
    .mspi_write = am_devices_mspi_atxp032_write,
    .mspi_mass_erase = am_devices_mspi_atxp032_mass_erase,
    .mspi_sector_erase = am_devices_mspi_atxp032_sector_erase,
    .mspi_xip_enable = am_devices_mspi_atxp032_enable_xip,
    .mspi_xip_disable = am_devices_mspi_atxp032_disable_xip,
    .mspi_scrambling_enable = am_devices_mspi_atxp032_enable_scrambling,
    .mspi_scrambling_disable = am_devices_mspi_atxp032_disable_scrambling,
#endif
};
//*****************************************************************************
//
// Test defines
//
//*****************************************************************************

#define MSPI_PSRAM_MODULE               0

#define MSPI_FLASH_MODULE               1

#define PSRAM_TARGET_ADDRESS            0
#define MSPI_PSRAM_DMA_RX_BUF_LENGTH    (20*1024)

#define FLASH_TARGET_ADDRESS            0
#define MSPI_FLASH_DMA_RX_BUF_LENGTH    (20*1024)  // 4K example buffer size.

#define PSRAM_WAIT_TIMEOUT_MS           (1000)
#define FLASH_WAIT_TIMEOUT_MS           (1000)
//
// PSRAM & FLASH buffer location
//
AM_SHARED_RW uint8_t g_ui8MspiPsramRxBuffer[MSPI_PSRAM_DMA_RX_BUF_LENGTH];
AM_SHARED_RW uint8_t g_ui8MspiFlashRxBuffer[MSPI_FLASH_DMA_RX_BUF_LENGTH];
//
// PSRAM device config
//
static AM_SHARED_RW uint32_t        ui32DMATCBBuffer2[2560];
void            *g_pPsramHandle;
void            *g_pMSPIPsramHandle;
float g_PsramStartTime, g_PsramStopTime;
float g_FlashStartTime, g_FlashStopTime;

am_devices_mspi_psram_config_t g_sMspiPsramConfig =
{
    .eDeviceConfig            = AM_HAL_MSPI_FLASH_HEX_DDR_CE0,
    .eClockFreq               = AM_HAL_MSPI_CLK_250MHZ,
    .ui32NBTxnBufLength       = sizeof(ui32DMATCBBuffer2) / sizeof(uint32_t),
    .pNBTxnBuf                = ui32DMATCBBuffer2,
    .ui32ScramblingStartAddr  = 0,
    .ui32ScramblingEndAddr    = 0,
};

static AM_SHARED_RW uint32_t        ui32DMATCBBuffer1[2560];
void            *g_pMSPIFlashHandle;
void            *g_pFlashHandle;

//
// FLASH device config
//
am_abstract_mspi_devices_config_t g_sMspiFlashConfig =
{
    #if defined(AM_BSP_MSPI_FLASH_DEVICE_IS25WX064)
    .eDeviceConfig = AM_HAL_MSPI_FLASH_OCTAL_CE0_1_8_8,
#else
    .eDeviceConfig = AM_HAL_MSPI_FLASH_OCTAL_CE0,
    #endif
    .eClockFreq               = AM_HAL_MSPI_CLK_96MHZ,
    .pNBTxnBuf                = ui32DMATCBBuffer1,
    .ui32NBTxnBufLength       = (sizeof(ui32DMATCBBuffer1) / sizeof(uint32_t)),
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

#define flash_mspi_isr                                                          \
    am_mspi_isr1(MSPI_FLASH_MODULE)
#define am_mspi_isr1(n)                                                        \
    am_mspi_isr(n)
#define am_mspi_isr(n)                                                         \
    am_mspi ## n ## _isr

volatile bool g_bMspiFlashDMAFinish = true;
volatile bool g_bMspiPsramDMAFinish = true;

//*****************************************************************************
//
// MSPI ISRs.
//
//*****************************************************************************
void psram_mspi_isr(void)
{
   uint32_t      ui32Status;

   am_hal_mspi_interrupt_status_get(g_pMSPIPsramHandle, &ui32Status, false);

   am_hal_mspi_interrupt_clear(g_pMSPIPsramHandle, ui32Status);

   am_hal_mspi_interrupt_service(g_pMSPIPsramHandle, ui32Status);
}

void
MspiPsramTransferCallback(void *pCallbackCtxt, uint32_t status)
{
    if ((status != AM_HAL_STATUS_SUCCESS) && (status != AM_HAL_MSPI_FIFO_FULL_CONDITION))
    {
        am_util_stdio_printf("\nMSPI: PSRAM Read Failed 0x%x\n", status);
    }
    else
    {
        g_bMspiPsramDMAFinish = true;
        g_PsramStopTime = get_time();
    }
}

//*****************************************************************************
//
// MSPI ISRs.
//
//*****************************************************************************
void
flash_mspi_isr(void)
{
    uint32_t      ui32Status;

    am_hal_mspi_interrupt_status_get(g_pMSPIFlashHandle, &ui32Status, false);

    am_hal_mspi_interrupt_clear(g_pMSPIFlashHandle, ui32Status);

    am_hal_mspi_interrupt_service(g_pMSPIFlashHandle, ui32Status);
}

void
MspiFlashTransferCallback(void *pCallbackCtxt, uint32_t status)
{
    if ((status != AM_HAL_STATUS_SUCCESS) && (status != AM_HAL_MSPI_FIFO_FULL_CONDITION))
    {
        am_util_stdio_printf("\nMSPI: FLASH Read Failed 0x%x\n", status);
    }
    else
    {
        g_bMspiFlashDMAFinish = true;
        g_FlashStopTime = get_time();
    }
}

//*****************************************************************************
//
// mspi psram init.
//
//*****************************************************************************
bool
psram_init(void)
{
    uint32_t ui32Status;
    //
    // Configure the MSPI and PSRAM Device.
    //
    ui32Status = am_devices_mspi_psram_aps25616n_ddr_init(MSPI_PSRAM_MODULE,
                                                         &g_sMspiPsramConfig,
                                                         &g_pPsramHandle,
                                                         &g_pMSPIPsramHandle);
    if (AM_DEVICES_MSPI_PSRAM_STATUS_SUCCESS != ui32Status)
    {
        am_util_stdio_printf("Failed to configure the MSPI and PSRAM Device correctly!\n");
        return false;
    }

    NVIC_SetPriority(MspiInterrupts[MSPI_PSRAM_MODULE], AM_IRQ_PRIORITY_DEFAULT);
    NVIC_EnableIRQ(MspiInterrupts[MSPI_PSRAM_MODULE]);

    //
    // Enable XIP mode.
    //
    ui32Status = am_devices_mspi_psram_aps25616n_ddr_enable_xip(g_pPsramHandle);
    if (AM_DEVICES_MSPI_PSRAM_STATUS_SUCCESS != ui32Status)
    {
        am_util_stdio_printf("Failed to enable XIP mode in the MSPI!\n");
        return false;
    }

    return true;
}

//*****************************************************************************
//
// mspi psram start transfer.
//
//*****************************************************************************
bool
psram_start(void)
{
    uint32_t ui32Status;

    g_bMspiPsramDMAFinish = false;
    AM_CRITICAL_BEGIN
    ui32Status = am_devices_mspi_psram_aps25616n_ddr_nonblocking_read(g_pPsramHandle,
                                                         g_ui8MspiPsramRxBuffer,
                                                         0,
                                                         MSPI_PSRAM_DMA_RX_BUF_LENGTH,
                                                         MspiPsramTransferCallback,
                                                         NULL);
    g_PsramStartTime = get_time();
    AM_CRITICAL_END
    if ( ui32Status != 0 )
    {
        am_util_stdio_printf("PSRAM: Start read failed! errorcode=%d\n", ui32Status);
        return false;
    }
    else
    {
        return true;
    }
}

//*****************************************************************************
//
// mspi flash init.
//
//*****************************************************************************
bool
flash_init(void)
{
    uint32_t ui32Status;

    am_util_stdio_printf("Please check if MSPI1 pins is occupied by MSPI0 or not!\n");
    //
    // Configure the MSPI and Flash Device.
    //
    ui32Status = mspi_device_func.mspi_init(MSPI_FLASH_MODULE, (void*)&g_sMspiFlashConfig, &g_pFlashHandle, &g_pMSPIFlashHandle);
    if (AM_ABSTRACT_MSPI_SUCCESS != ui32Status)
    {
        am_util_stdio_printf("Failed to configure the MSPI and Flash Device correctly!\n");
        return false;
    }
    //
    // Make sure we aren't in XIP mode.
    //
    ui32Status = mspi_device_func.mspi_xip_disable(g_pFlashHandle);
    if (AM_ABSTRACT_MSPI_SUCCESS != ui32Status)
    {
        am_util_stdio_printf("Failed to disable XIP mode in the MSPI!\n");
        return false;
    }
    //
    // Deinit the flash device.
    //
    ui32Status = mspi_device_func.mspi_term(g_pFlashHandle);
    if (AM_ABSTRACT_MSPI_SUCCESS != ui32Status)
    {
        am_util_stdio_printf("Failed to shutdown the MSPI and Flash Device!\n");\
        return false;
    }
    //
    // Init the flash device to work at 96MHZ.
    //
    g_sMspiFlashConfig.eClockFreq = AM_HAL_MSPI_CLK_96MHZ;
    g_sMspiFlashConfig.eDeviceConfig = AM_HAL_MSPI_FLASH_OCTAL_CE0;

    ui32Status = mspi_device_func.mspi_init(MSPI_FLASH_MODULE, (void*)&g_sMspiFlashConfig, &g_pFlashHandle, &g_pMSPIFlashHandle);
    if (AM_ABSTRACT_MSPI_SUCCESS != ui32Status)
    {
        am_util_stdio_printf("Failed to configure the MSPI and Flash Device correctly!\n");
        return false;
    }

    NVIC_SetPriority(MspiInterrupts[MSPI_FLASH_MODULE], AM_IRQ_PRIORITY_DEFAULT);
    NVIC_EnableIRQ(MspiInterrupts[MSPI_FLASH_MODULE]);
    return true;
}

//*****************************************************************************
//
// mspi flash start transfer.
//
//*****************************************************************************
bool
flash_start(void)
{
    uint32_t ui32Status;

    g_bMspiFlashDMAFinish = false;
    AM_CRITICAL_BEGIN
    ui32Status = mspi_device_func.mspi_read_cb(g_pFlashHandle,
                                    g_ui8MspiFlashRxBuffer,
                                    FLASH_TARGET_ADDRESS << 16,
                                    MSPI_FLASH_DMA_RX_BUF_LENGTH,
                                    MspiFlashTransferCallback,
                                    NULL);
    g_FlashStartTime = get_time();
    AM_CRITICAL_END
    if ( ui32Status != 0 )
    {
        am_util_stdio_printf("FLASH: Start read failed! errorcode=%d\n", ui32Status);
        return false;
    }
    else
    {
        return true;
    }
}

//*****************************************************************************
//
// Wait psram complete.
//
//*****************************************************************************
bool
psram_wait(void)
{
    uint32_t count = 0;

    while( (!g_bMspiPsramDMAFinish)
        && (count < PSRAM_WAIT_TIMEOUT_MS))
    {
        am_util_delay_ms(1);
        count ++;
    }

    if(count >= PSRAM_WAIT_TIMEOUT_MS)
    {
        return false;
    }
    else
    {
        return true;
    }
}

//*****************************************************************************
//
// Wait flash transation to complete.
//
//*****************************************************************************
bool
flash_wait(void)
{
    uint32_t count = 0;

    while( (!g_bMspiFlashDMAFinish)
        && (count < FLASH_WAIT_TIMEOUT_MS))
    {
        am_util_delay_ms(1);
        count ++;
    }

    if(count >= FLASH_WAIT_TIMEOUT_MS)
    {
        return false;
    }
    else
    {
        return true;
    }
}