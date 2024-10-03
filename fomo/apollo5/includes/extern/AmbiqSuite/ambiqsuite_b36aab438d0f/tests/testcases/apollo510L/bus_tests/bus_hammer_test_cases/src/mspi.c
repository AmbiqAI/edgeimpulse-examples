//*****************************************************************************
//
//! @file mspi.c
//!
//! @brief MSPI setup and functions for the bus hammer test
//*****************************************************************************

//*****************************************************************************
//
// ${copyright}
//
// This is part of revision ${version} of the AmbiqSuite Development Package.
//
//*****************************************************************************

#include "mspi.h"
#include "nema_utils.h" //used for nema_get_time()

AM_SHARED_RW uint32_t        ui32DMATCBBuffer[4][2560];
void            *g_pMSPIDeviceHandle[4];
void            *g_pMSPIHandle[4];

volatile float g_fDMAStopTime = 0.f;
volatile bool g_bDMADone = false;

// Platform specific MSPI mapping
#ifdef APOLLO5_FPGA
const MSPIDeviceType_t g_sMSPIDeviceTypes[4] =
{
    PSRAM_APS25616N,
    FLASH_ATXP032,
    NONE,
    NONE
};
#else
const MSPIDeviceType_t g_sMSPIDeviceTypes[4] =
{
    PSRAM_APS25616N,
    FLASH_IS25WX064,
    NONE,
    NONE
    //PSRAM_APS25616N // not currently needed on EB
};
#endif

const uint8_t* g_ui32MSPIXIPAddresses[4] =
{
    (uint8_t*)(MSPI0_APERTURE_START_ADDR),
    (uint8_t*)(MSPI1_APERTURE_START_ADDR),
    (uint8_t*)(MSPI2_APERTURE_START_ADDR),
    (uint8_t*)(MSPI3_APERTURE_START_ADDR)
};

am_devices_mspi_psram_config_t g_sMspiPsramConfig[4] =
{
    {
        .eDeviceConfig            = AM_HAL_MSPI_FLASH_HEX_DDR_CE0,
        // 96MHz MSPI SCLK w/ DDR == 192MHz Mtransfers/s
        .eClockFreq               = AM_HAL_MSPI_CLK_192MHZ,
        .ui32NBTxnBufLength       = sizeof(ui32DMATCBBuffer[0]) / sizeof(uint32_t),
        .pNBTxnBuf                = ui32DMATCBBuffer[0],
        .ui32ScramblingStartAddr  = 0,
        .ui32ScramblingEndAddr    = 0,
    },
    {
        //FIXME!!!
        .eDeviceConfig            = AM_HAL_MSPI_FLASH_HEX_DDR_CE0,
        // 96MHz MSPI SCLK w/ DDR == 192MHz Mtransfers/s
        .eClockFreq               = AM_HAL_MSPI_CLK_192MHZ,
        .ui32NBTxnBufLength       = sizeof(ui32DMATCBBuffer[1]) / sizeof(uint32_t),
        .pNBTxnBuf                = ui32DMATCBBuffer[1],
        .ui32ScramblingStartAddr  = 0,
        .ui32ScramblingEndAddr    = 0,
    },
    {
        //FIXME!!!
        .eDeviceConfig            = AM_HAL_MSPI_FLASH_HEX_DDR_CE0,
        // 96MHz MSPI SCLK w/ DDR == 192MHz Mtransfers/s
        .eClockFreq               = AM_HAL_MSPI_CLK_192MHZ,
        .ui32NBTxnBufLength       = sizeof(ui32DMATCBBuffer[2]) / sizeof(uint32_t),
        .pNBTxnBuf                = ui32DMATCBBuffer[2],
        .ui32ScramblingStartAddr  = 0,
        .ui32ScramblingEndAddr    = 0,
    },
    {
        .eDeviceConfig            = AM_HAL_MSPI_FLASH_HEX_DDR_CE0,
        // 96MHz MSPI SCLK w/ DDR == 192MHz Mtransfers/s
        .eClockFreq               = AM_HAL_MSPI_CLK_192MHZ,
        .ui32NBTxnBufLength       = sizeof(ui32DMATCBBuffer[3]) / sizeof(uint32_t),
        .pNBTxnBuf                = ui32DMATCBBuffer[3],
        .ui32ScramblingStartAddr  = 0,
        .ui32ScramblingEndAddr    = 0,
    }
};

//
// FLASH device config for Apollo5 EB
//
am_devices_mspi_is25wx064_config_t g_sMspiFlashIS25WX064Config[4] =
{
    {
        .eDeviceConfig            = AM_HAL_MSPI_FLASH_OCTAL_CE0,
        .eClockFreq               = AM_HAL_MSPI_CLK_48MHZ,
        .ui32NBTxnBufLength       = sizeof(ui32DMATCBBuffer[0]) / sizeof(uint32_t),
        .pNBTxnBuf                = ui32DMATCBBuffer[0],
        .ui32ScramblingStartAddr  = 0,
        .ui32ScramblingEndAddr    = 0,
    },
    {
        .eDeviceConfig            = AM_HAL_MSPI_FLASH_OCTAL_CE0,
        .eClockFreq               = AM_HAL_MSPI_CLK_48MHZ,
        .ui32NBTxnBufLength       = sizeof(ui32DMATCBBuffer[1]) / sizeof(uint32_t),
        .pNBTxnBuf                = ui32DMATCBBuffer[1],
        .ui32ScramblingStartAddr  = 0,
        .ui32ScramblingEndAddr    = 0,
    },
    {
        .eDeviceConfig            = AM_HAL_MSPI_FLASH_OCTAL_CE0,
        .eClockFreq               = AM_HAL_MSPI_CLK_48MHZ,
        .ui32NBTxnBufLength       = sizeof(ui32DMATCBBuffer[2]) / sizeof(uint32_t),
        .pNBTxnBuf                = ui32DMATCBBuffer[2],
        .ui32ScramblingStartAddr  = 0,
        .ui32ScramblingEndAddr    = 0,
    },
    {
        .eDeviceConfig            = AM_HAL_MSPI_FLASH_OCTAL_CE0,
        .eClockFreq               = AM_HAL_MSPI_CLK_48MHZ,
        .ui32NBTxnBufLength       = sizeof(ui32DMATCBBuffer[3]) / sizeof(uint32_t),
        .pNBTxnBuf                = ui32DMATCBBuffer[3],
        .ui32ScramblingStartAddr  = 0,
        .ui32ScramblingEndAddr    = 0,
    }
};
//
// FLASH device config for FPGA Turbo board
//
am_devices_mspi_atxp032_config_t g_sMspiFlashATXP032Config[4] =
{
    {
        .eDeviceConfig            = AM_HAL_MSPI_FLASH_OCTAL_CE0,
        .eClockFreq               = AM_HAL_MSPI_CLK_48MHZ,
        .ui32NBTxnBufLength       = sizeof(ui32DMATCBBuffer[0]) / sizeof(uint32_t),
        .pNBTxnBuf                = ui32DMATCBBuffer[0],
        .ui32ScramblingStartAddr  = 0,
        .ui32ScramblingEndAddr    = 0,
    },
    {
        .eDeviceConfig            = AM_HAL_MSPI_FLASH_OCTAL_CE0,
        .eClockFreq               = AM_HAL_MSPI_CLK_48MHZ,
        .ui32NBTxnBufLength       = sizeof(ui32DMATCBBuffer[1]) / sizeof(uint32_t),
        .pNBTxnBuf                = ui32DMATCBBuffer[1],
        .ui32ScramblingStartAddr  = 0,
        .ui32ScramblingEndAddr    = 0,
    },
    {
        .eDeviceConfig            = AM_HAL_MSPI_FLASH_OCTAL_CE0,
        .eClockFreq               = AM_HAL_MSPI_CLK_48MHZ,
        .ui32NBTxnBufLength       = sizeof(ui32DMATCBBuffer[2]) / sizeof(uint32_t),
        .pNBTxnBuf                = ui32DMATCBBuffer[2],
        .ui32ScramblingStartAddr  = 0,
        .ui32ScramblingEndAddr    = 0,
    },
    {
        .eDeviceConfig            = AM_HAL_MSPI_FLASH_OCTAL_CE0,
        .eClockFreq               = AM_HAL_MSPI_CLK_48MHZ,
        .ui32NBTxnBufLength       = sizeof(ui32DMATCBBuffer[3]) / sizeof(uint32_t),
        .pNBTxnBuf                = ui32DMATCBBuffer[3],
        .ui32ScramblingStartAddr  = 0,
        .ui32ScramblingEndAddr    = 0,
    }
};

//! MSPI interrupts.
static const IRQn_Type MspiInterrupts[] =
{
    MSPI0_IRQn,
    MSPI1_IRQn,
    MSPI2_IRQn,
    MSPI3_IRQn,
};

//*****************************************************************************
//
//  MSPI ISRs.
//
//*****************************************************************************

void am_mspi0_isr(void)
{
   uint32_t      ui32Status;

   am_hal_mspi_interrupt_status_get(g_pMSPIHandle[0], &ui32Status, false);

   am_hal_mspi_interrupt_clear(g_pMSPIHandle[0], ui32Status);

   am_hal_mspi_interrupt_service(g_pMSPIHandle[0], ui32Status);
}

void am_mspi1_isr(void)
{
   uint32_t      ui32Status;

   am_hal_mspi_interrupt_status_get(g_pMSPIHandle[1], &ui32Status, false);

   am_hal_mspi_interrupt_clear(g_pMSPIHandle[1], ui32Status);

   am_hal_mspi_interrupt_service(g_pMSPIHandle[1], ui32Status);
}

void am_mspi3_isr(void)
{
   uint32_t      ui32Status;

   am_hal_mspi_interrupt_status_get(g_pMSPIHandle[3], &ui32Status, false);

   am_hal_mspi_interrupt_clear(g_pMSPIHandle[3], ui32Status);

   am_hal_mspi_interrupt_service(g_pMSPIHandle[3], ui32Status);
}

//*****************************************************************************
//
//  Log completion time when the DMA transfer completes
//
//*****************************************************************************
void
dma_callback(void *pCallbackCtxt, uint32_t status)
{
    if(!g_bDMADone)
    {
        g_fDMAStopTime = nema_get_time();
        g_bDMADone = true;
    }
}

//*****************************************************************************
//
// Set up an AP25616N PSRAM device
//
// mspi: MSPI interface index
//
//*****************************************************************************
void
setup_mspi_psram_ap25616n(uint32_t mspi)
{
    uint32_t ui32Status;
    am_util_stdio_printf("\nSet up AP25616N PSRAM on MSPI%d\n", mspi);

#ifndef APOLLO5_FPGA
    //
    // Run MSPI DDR timing scan
    //
    am_devices_mspi_psram_ddr_timing_config_t MSPIDdrTimingConfig;
    am_util_debug_printf("Starting MSPI DDR Timing Scan: \n");
    if ( AM_DEVICES_MSPI_PSRAM_STATUS_SUCCESS == am_devices_mspi_psram_aps25616n_ddr_init_timing_check(mspi, &(g_sMspiPsramConfig[mspi]), &MSPIDdrTimingConfig) )
    {
        am_util_debug_printf("==== Scan Result: RXDQSDELAY0 = %d \n", MSPIDdrTimingConfig.ui32Rxdqsdelay);
    }
    else
    {
        TEST_FAIL_MESSAGE("DDR scan failed, no valid setting.");
    }
#endif

    //
    // Configure the MSPI and PSRAM Device.
    //
    ui32Status = am_devices_mspi_psram_aps25616n_ddr_init(mspi, &(g_sMspiPsramConfig[mspi]), &(g_pMSPIDeviceHandle[mspi]), &(g_pMSPIHandle[mspi]));
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(AM_DEVICES_MSPI_PSRAM_STATUS_SUCCESS, ui32Status, "Failed to configure the MSPI and PSRAM Device correctly!\n");
    NVIC_SetPriority(MspiInterrupts[mspi], PSRAM_ISR_PRIORITY);
    NVIC_EnableIRQ(MspiInterrupts[mspi]);

#ifndef APOLLO5_FPGA
    //
    // Apply DDR timing setting
    //
    ui32Status = am_devices_mspi_psram_aps25616n_apply_ddr_timing(g_pMSPIDeviceHandle[mspi], &MSPIDdrTimingConfig);
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(AM_DEVICES_MSPI_PSRAM_STATUS_SUCCESS, ui32Status, "Failed to apply the timming scan parameter!\n");
#endif

#ifndef DISABLE_XIP
    //
    // Enable XIP mode.
    //
    ui32Status = am_devices_mspi_psram_aps25616n_ddr_enable_xip(g_pMSPIDeviceHandle[mspi]);
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(AM_DEVICES_MSPI_PSRAM_STATUS_SUCCESS, ui32Status, "Failed to enable XIP mode in the MSPI!\n");
#endif
}

//*****************************************************************************
//
// Tear down an AP25616N PSRAM device
//
//*****************************************************************************
void
teardown_mspi_psram_ap25616n(uint32_t mspi)
{
    //
    // Need to make sure all pending XIPMM transactions are flushed
    //
    am_hal_cachectrl_dcache_invalidate(NULL, true);

    uint32_t ui32Status;
    ui32Status = am_devices_mspi_psram_aps25616n_ddr_disable_xip(g_pMSPIDeviceHandle[mspi]);
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(AM_DEVICES_MSPI_PSRAM_STATUS_SUCCESS, ui32Status, "Failure to disable XIP");

    ui32Status = am_devices_mspi_psram_aps25616n_ddr_deinit(g_pMSPIDeviceHandle[mspi]);
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(AM_DEVICES_MSPI_PSRAM_STATUS_SUCCESS, ui32Status, "Failure to shutdown AP25616N PSRAM");
}

//*****************************************************************************
//
// Set up an IS25WX064 flash device
//
// mspi: MSPI interface index
// *flash_prep_buffer: location of the fully populated target pattern structure (usually mram_target)
// flash_prep_length: length of flash_prep_buffer in bytes
// flash_prep_dma: location of the DMA pattern
// flash_prep_dma_length: length of the DMA pattern in bytes
//
//*****************************************************************************
void
setup_mspi_flash_is25wx064(uint32_t mspi,
                           uint8_t *flash_prep_buffer,
                           uint32_t flash_prep_length,
                           uint8_t *flash_prep_dma,
                           uint32_t flash_prep_dma_length)
{
    uint32_t ui32Status;
    am_util_stdio_printf("\nSet up IS25WX064 Flash on MSPI%d\n", mspi);
    ui32Status = am_devices_mspi_is25wx064_init(mspi, (void*)&g_sMspiFlashIS25WX064Config[mspi], &g_pMSPIDeviceHandle[mspi], &g_pMSPIHandle[mspi]);
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(AM_DEVICES_MSPI_IS25WX064_STATUS_SUCCESS, ui32Status, "IS25WX064 flash init failed");
    NVIC_SetPriority(MspiInterrupts[mspi], AM_IRQ_PRIORITY_DEFAULT);
    NVIC_EnableIRQ(MspiInterrupts[mspi]);
    am_util_stdio_printf("Writing flash device on mspi %d\n", mspi);
    ui32Status = am_devices_mspi_is25wx064_disable_xip(g_pMSPIDeviceHandle[mspi]);
    ui32Status = am_devices_mspi_is25wx064_mass_erase(g_pMSPIDeviceHandle[mspi]);
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0, ui32Status, "IS25WX064 flash erase failed");
    ui32Status = am_devices_mspi_is25wx064_write(g_pMSPIDeviceHandle[mspi], flash_prep_dma, 0x0, flash_prep_dma_length, true);
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0, ui32Status, "IS25WX064 flash write failed");
    ui32Status = am_devices_mspi_is25wx064_write(g_pMSPIDeviceHandle[mspi], flash_prep_buffer, MSPI_TARGET_MEM_OFFSET, flash_prep_length, true);
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0, ui32Status, "IS25WX064 flash write failed");
    ui32Status = am_devices_mspi_is25wx064_enable_xip(g_pMSPIDeviceHandle[mspi]);
    TEST_ASSERT_EQUAL_MEMORY_MESSAGE(flash_prep_dma,
        g_ui32MSPIXIPAddresses[mspi],
        flash_prep_dma_length,
        "Flash verify failed");
    TEST_ASSERT_EQUAL_MEMORY_MESSAGE(flash_prep_buffer,
        g_ui32MSPIXIPAddresses[mspi] + MSPI_TARGET_MEM_OFFSET,
        flash_prep_length,
        "Flash verify failed");
}

//*****************************************************************************
//
// Tear down an IS25WX064 flash device
//
//*****************************************************************************
void
teardown_mspi_flash_is25wx064(uint32_t mspi)
{
    uint32_t ui32Status;
    //
    // Disable XIP mode.
    //
    ui32Status = am_devices_mspi_is25wx064_disable_xip(g_pMSPIDeviceHandle[mspi]);
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(AM_DEVICES_MSPI_IS25WX064_STATUS_SUCCESS, ui32Status, "Failure to disable XIP");
    //
    // Deinit the flash device.
    //
    ui32Status = am_devices_mspi_is25wx064_deinit(g_pMSPIDeviceHandle[mspi]);
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(AM_DEVICES_MSPI_IS25WX064_STATUS_SUCCESS, ui32Status, "Failure to shutdown IS25WX064");
}

//*****************************************************************************
//
// Set up an ATXP032 flash device
//
// mspi: MSPI interface index
// *flash_prep_buffer: location of the fully populated target pattern structure (usually mram_target)
// flash_prep_length: length of flash_prep_buffer in bytes
// flash_prep_dma: location of the DMA pattern
// flash_prep_dma_length: length of the DMA pattern in bytes
//
//*****************************************************************************
void
setup_mspi_flash_atxp032(uint32_t mspi,
                         uint8_t *flash_prep_buffer,
                         uint32_t flash_prep_length,
                         uint8_t *flash_prep_dma,
                         uint32_t flash_prep_dma_length)
{
    uint32_t ui32Status;
    uint32_t buffer_index;
    am_util_stdio_printf("\nSet up ATXP032 Flash on MSPI%d\n", mspi);
    am_hal_interrupt_master_disable();

    ui32Status = am_devices_mspi_atxp032_init(mspi, (void*)&g_sMspiFlashATXP032Config[mspi], &g_pMSPIDeviceHandle[mspi], &g_pMSPIHandle[mspi]);
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(AM_DEVICES_MSPI_ATXP032_STATUS_SUCCESS, ui32Status, "ATXP032 flash init failed");

    NVIC_SetPriority(MspiInterrupts[mspi], AM_IRQ_PRIORITY_DEFAULT);
    NVIC_EnableIRQ(MspiInterrupts[mspi]);
    am_hal_interrupt_master_enable();

    am_util_stdio_printf("Writing flash device on mspi %d\n", mspi);
    ui32Status = am_devices_mspi_atxp032_disable_xip(g_pMSPIDeviceHandle[mspi]);
    ui32Status = am_devices_mspi_atxp032_sector_erase(g_pMSPIDeviceHandle[mspi], 0);
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0, ui32Status, "ATXP032 flash erase failed");
    ui32Status = am_devices_mspi_atxp032_write(g_pMSPIDeviceHandle[mspi], flash_prep_dma, 0x0, flash_prep_dma_length, true);
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0, ui32Status, "ATXP032 flash write failed");

    buffer_index = 0;
    while(buffer_index < flash_prep_length)
    {
        ui32Status = am_devices_mspi_atxp032_sector_erase(g_pMSPIDeviceHandle[mspi],
                                                          MSPI_TARGET_MEM_OFFSET + buffer_index);
        TEST_ASSERT_EQUAL_UINT32_MESSAGE(0, ui32Status, "ATXP032 flash erase failed");
        ui32Status = am_devices_mspi_atxp032_write(g_pMSPIDeviceHandle[mspi],
                                                   flash_prep_buffer + buffer_index,
                                                   MSPI_TARGET_MEM_OFFSET + buffer_index,
                                                   ((flash_prep_length - buffer_index) > AM_DEVICES_MSPI_ATXP032_SECTOR_SIZE) ?
                                                        AM_DEVICES_MSPI_ATXP032_SECTOR_SIZE :
                                                        flash_prep_length - buffer_index,
                                                   true);
        TEST_ASSERT_EQUAL_UINT32_MESSAGE(0, ui32Status, "ATXP032 flash write failed");
        buffer_index += AM_DEVICES_MSPI_ATXP032_SECTOR_SIZE;
    }
    ui32Status = am_devices_mspi_atxp032_enable_xip(g_pMSPIDeviceHandle[mspi]);
    TEST_ASSERT_EQUAL_MEMORY_MESSAGE(flash_prep_dma,
        g_ui32MSPIXIPAddresses[mspi],
        flash_prep_dma_length,
        "Flash verify failed");
    TEST_ASSERT_EQUAL_MEMORY_MESSAGE(flash_prep_buffer,
        g_ui32MSPIXIPAddresses[mspi] + MSPI_TARGET_MEM_OFFSET,
        flash_prep_length,
        "Flash verify failed");
}

//*****************************************************************************
//
// Tear down an ATXP032 flash device
//
//*****************************************************************************
void
teardown_mspi_flash_atxp032(uint32_t mspi)
{
    uint32_t ui32Status;
    //
    // Disable XIP mode.
    //
    ui32Status = am_devices_mspi_atxp032_disable_xip(g_pMSPIDeviceHandle[mspi]);
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(AM_DEVICES_MSPI_ATXP032_STATUS_SUCCESS, ui32Status, "Failure to disable XIP");

    //
    // Deinit the flash device.
    //
    ui32Status = am_devices_mspi_atxp032_deinit(g_pMSPIDeviceHandle[mspi]);
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(AM_DEVICES_MSPI_ATXP032_STATUS_SUCCESS, ui32Status, "Failure to shutdown ATXP032");
}

//*****************************************************************************
//
// Use DMA to read from an MSPI device
//
// mspi: MSPI device index
// *rx_buffer: Buffer to receive data read from the DMA source
// read_address: Start of the data in the MSPI device to read
// num_bytes: Number of bytes to read
//
// Returns: 0 if successful, error code otherwise
//
//*****************************************************************************
uint32_t
mspi_dma_read(uint8_t mspi, uint8_t *rx_buffer, uint32_t read_address, uint32_t num_bytes)
{
    uint32_t ui32Status;
    static uint32_t dummy_context;
    switch(g_sMSPIDeviceTypes[mspi])
    {
        case PSRAM_APS25616N:
            ui32Status = am_devices_mspi_psram_aps25616n_ddr_nonblocking_read(g_pMSPIDeviceHandle[mspi],
                rx_buffer,
                read_address,
                num_bytes,
                dma_callback,
                (void*)&dummy_context);
            break;
        case FLASH_IS25WX064:
            ui32Status = am_devices_mspi_is25wx064_read_cb(g_pMSPIDeviceHandle[mspi],
                rx_buffer,
                read_address,
                num_bytes,
                dma_callback,
                (void*)&dummy_context);

            break;
        case FLASH_ATXP032:
            ui32Status = am_devices_mspi_atxp032_read_cb(g_pMSPIDeviceHandle[mspi],
                rx_buffer,
                read_address,
                num_bytes,
                dma_callback,
                (void*)&dummy_context);
            break;
        default:
            am_util_stdio_printf("Unknown flash device type!");
            ui32Status = 1;
            break;
    }
    return ui32Status;
}

//*****************************************************************************
//
// Use DMA to write to an MSPI device
//
// mspi: Target MSPI device index
// *tx_buffer: Buffer containing data to write to the target device
// read_address: Location in the MSPI device where the data will be written
// num_bytes: Number of bytes to write
//
// Returns: 0 if successful, error code otherwise
//
//*****************************************************************************
uint32_t
mspi_dma_write(uint8_t mspi, uint8_t *tx_buffer, uint32_t write_address, uint32_t num_bytes)
{
    uint32_t ui32Status;
    static uint32_t dummy_context;
    switch(g_sMSPIDeviceTypes[mspi])
    {
        case PSRAM_APS25616N:
            ui32Status = am_devices_mspi_psram_aps25616n_ddr_nonblocking_write(g_pMSPIDeviceHandle[mspi],
                tx_buffer,
                write_address,
                num_bytes,
                dma_callback,
                (void*)&dummy_context);
            break;
        case FLASH_IS25WX064:
        case FLASH_ATXP032:
            TEST_FAIL_MESSAGE("DMA write to flash is unsupported in this test!");
            ui32Status = 1;
            break;
        default:
            am_util_stdio_printf("Unknown flash device type!");
            ui32Status = 1;
            break;
    }
    return ui32Status;
}

//*****************************************************************************
//
// Set up MSPI memory devices
//
// Initializes each MSPI device defined in g_sMSPIDeviceTypes for DMA and XIP,
// and writes the correct buffer contents if the device is nonvolatile.
//
// To allow the ability to simultaneously issue DMA and XIP operations against
// the same MSPI flash device, we put the DMA pattern at offset 0 for performing
// DMA reads, and the full target pattern at offset MSPI_TARGET_MEM_OFFSET for
// XIP reads.
//
// The memory patterns are not written to PSRAM devices as those are initialized
// along with the other volatile targets (SSRAM*, etc)
//
// *flash_prep_buffer: location of the fully populated target pattern structure (usually mram_target)
// flash_prep_length: length of flash_prep_buffer in bytes
// flash_prep_dma: location of the DMA pattern
// flash_prep_dma_length: length of the DMA pattern in bytes
//
//*****************************************************************************
void
setup_mspi(uint8_t *flash_prep_buffer,
           uint32_t flash_prep_length,
           uint8_t *flash_prep_dma,
           uint32_t flash_prep_dma_length)
{
    for(uint8_t i=0; i<4; i++)
    {
        switch(g_sMSPIDeviceTypes[i])
        {
            case NONE:
                break;
            case PSRAM_APS25616N:
                setup_mspi_psram_ap25616n(i);
                break;
            case FLASH_ATXP032:
                setup_mspi_flash_atxp032(i, flash_prep_buffer, flash_prep_length,
                                         flash_prep_dma, flash_prep_dma_length);
                break;
            case FLASH_IS25WX064:
                setup_mspi_flash_is25wx064(i, flash_prep_buffer, flash_prep_length,
                                           flash_prep_dma, flash_prep_dma_length);
                break;
        };
    }
}

void
teardown_mspi()
{
    for(uint8_t i=0; i<4; i++)
    {
        switch(g_sMSPIDeviceTypes[i])
        {
            case NONE:
                break;
            case PSRAM_APS25616N:
                teardown_mspi_psram_ap25616n(i);
                break;
            case FLASH_ATXP032:
                teardown_mspi_flash_atxp032(i);
                break;
            case FLASH_IS25WX064:
                teardown_mspi_flash_is25wx064(i);
                break;
        };
    }
}