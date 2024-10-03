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
#include "am_devices_mspi_atxp032.h"
//*****************************************************************************
//
// Test defines
//
//*****************************************************************************

#define MSPI_PSRAM_MODULE               0

#if (DISPLAY_MSPI_INST == 2)
#define MSPI_FLASH_MODULE               1
#else
#define MSPI_FLASH_MODULE               2
#endif

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
static uint32_t        ui32DMATCBBuffer2[2560];
void            *g_pPsramHandle;
void            *g_pMSPIPsramHandle;

am_devices_mspi_psram_config_t g_sMspiPsramConfig =
{
#ifdef APS25616N_OCTAL_MODE_EN
    .eDeviceConfig            = AM_HAL_MSPI_FLASH_OCTAL_DDR_CE0,
#else
    .eDeviceConfig            = AM_HAL_MSPI_FLASH_HEX_DDR_CE0,
#endif
    .eClockFreq               = AM_HAL_MSPI_CLK_96MHZ,
    .ui32NBTxnBufLength       = sizeof(ui32DMATCBBuffer2) / sizeof(uint32_t),
    .pNBTxnBuf                = ui32DMATCBBuffer2,
    .ui32ScramblingStartAddr  = 0,
    .ui32ScramblingEndAddr    = 0,
};

static uint32_t        ui32DMATCBBuffer1[2560];
void            *g_pMSPIFlashHandle;
void            *g_pFlashHandle;

//
// FLASH device config
//
am_devices_mspi_atxp032_config_t g_sMspiFlashConfig =
{
    .eDeviceConfig            = AM_HAL_MSPI_FLASH_OCTAL_CE0,
    .eClockFreq               = AM_HAL_MSPI_CLK_48MHZ,
    .pNBTxnBuf                = ui32DMATCBBuffer1,
    .ui32NBTxnBufLength       = (sizeof(ui32DMATCBBuffer1) / sizeof(uint32_t)),
    .ui32ScramblingStartAddr  = 0,
    .ui32ScramblingEndAddr    = 0,
};

//am_devices_mspi_atxp032_sdr_timing_config_t g_MSPITimingConfig;
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

volatile static bool g_bMspiFlashDMAFinish = false;
volatile static bool g_bMspiPsramDMAFinish = false;


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
        am_hal_gpio_output_clear(DEBUG_PIN_MSPI0);
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
        am_hal_gpio_output_clear(DEBUG_PIN_MSPI1);
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

    ui32Status = am_devices_mspi_psram_aps25616n_ddr_nonblocking_read(g_pPsramHandle,
                                                         g_ui8MspiPsramRxBuffer,
                                                         0,
                                                         MSPI_PSRAM_DMA_RX_BUF_LENGTH,
                                                         MspiPsramTransferCallback,
                                                         NULL);

    if ( ui32Status != 0 )
    {
        am_util_stdio_printf("PSRAM: Start read failed! errorcode=%d\n", ui32Status);
        return false;
    }
    else
    {
        am_hal_gpio_output_set(DEBUG_PIN_MSPI0);
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

    am_util_stdio_printf("please notice MSPI1 pins have be occupied by MSPI0 or not !!.\n");

    //
    // Configure the MSPI and Flash Device.
    //
    ui32Status = am_devices_mspi_atxp032_init(MSPI_FLASH_MODULE, (void*)&g_sMspiFlashConfig, &g_pFlashHandle, &g_pMSPIFlashHandle);
    if (AM_DEVICES_MSPI_ATXP032_STATUS_SUCCESS != ui32Status)
    {
        am_util_stdio_printf("Failed to configure the MSPI and Flash Device correctly!\n");
        return false;
    }
    //
    // Make sure we aren't in XIP mode.
    //
    ui32Status = am_devices_mspi_atxp032_disable_xip(g_pFlashHandle);
    if (AM_DEVICES_MSPI_ATXP032_STATUS_SUCCESS != ui32Status)
    {
        am_util_stdio_printf("Failed to disable XIP mode in the MSPI!\n");
        return false;
    }

    //
    // Deinit the flash device.
    //
    ui32Status = am_devices_mspi_atxp032_deinit(g_pFlashHandle);
    if (AM_DEVICES_MSPI_ATXP032_STATUS_SUCCESS != ui32Status)
    {
        am_util_stdio_printf("Failed to shutdown the MSPI and Flash Device!\n");\
        return false;
    }

    //
    // Init the flash device to work at 96MHZ.
    //
    g_sMspiFlashConfig.eClockFreq = AM_HAL_MSPI_CLK_96MHZ;
    g_sMspiFlashConfig.eDeviceConfig = AM_HAL_MSPI_FLASH_OCTAL_CE0;

    ui32Status = am_devices_mspi_atxp032_init(MSPI_FLASH_MODULE, (void*)&g_sMspiFlashConfig, &g_pFlashHandle, &g_pMSPIFlashHandle);
    if (AM_DEVICES_MSPI_ATXP032_STATUS_SUCCESS != ui32Status)
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

    ui32Status = am_devices_mspi_atxp032_read_cb(g_pFlashHandle,
                                    g_ui8MspiFlashRxBuffer,
                                    FLASH_TARGET_ADDRESS << 16,
                                    MSPI_FLASH_DMA_RX_BUF_LENGTH,
                                    MspiFlashTransferCallback,
                                    NULL);

    if ( ui32Status != 0 )
    {
        am_util_stdio_printf("FLASH: Start read failed! errorcode=%d\n", ui32Status);
        return false;
    }
    else
    {
        am_hal_gpio_output_set(DEBUG_PIN_MSPI1);
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