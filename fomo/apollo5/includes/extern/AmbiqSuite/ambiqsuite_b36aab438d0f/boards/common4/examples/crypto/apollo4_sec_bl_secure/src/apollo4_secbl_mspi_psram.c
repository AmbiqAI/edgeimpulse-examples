//*****************************************************************************
//
//! @file apollo4_secbl_mspi_psram.c
//!
//! @brief a psram interface for aps12808/25616 on the eb board
//!
//! Purpose: implement a secure bootloader psram interface for aps12808/25616
//!
//! Printing takes place over the ITM at 1M Baud.
//!
//! Additional Information:
//! Needs eb board with aps12808/25616 psram
//
//*****************************************************************************

//*****************************************************************************
//
// ${copyright}
//
// This is part of revision ${version} of the AmbiqSuite Development Package.
//
//*****************************************************************************

#include <string.h>
#include "am_mcu_apollo.h"
#include "am_bsp.h"
#include "am_util.h"

#include "apollo4_secbl_mspi_psram.h"
#include "am_devices_mspi_psram_aps25616n.h"

#define MSPI_PSRAM_MODULE      1
#define PSRAM_TOTAL_SIZE       (256 * 1024)

static uint32_t ui32DMATCBBuffer[2560];
void            *g_pPsramHandle;
void            *g_pMSPIPsramHandle;

am_devices_mspi_psram_config_t g_sMspiPsramConfig =
{
    .eDeviceConfig            = AM_BSP_MSPI_PSRAM_MODULE_OCTAL_DDR_CE,
    .eClockFreq               = AM_HAL_MSPI_CLK_96MHZ,
    .ui32NBTxnBufLength       = sizeof(ui32DMATCBBuffer) / sizeof(uint32_t),
    .pNBTxnBuf                = ui32DMATCBBuffer,
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
    uint32_t ui32Status;

    am_hal_mspi_interrupt_status_get(g_pMSPIPsramHandle, &ui32Status, false);

    am_hal_mspi_interrupt_clear(g_pMSPIPsramHandle, ui32Status);

    am_hal_mspi_interrupt_service(g_pMSPIPsramHandle, ui32Status);
}

static uint32_t ssbl_mspi_psram_init(void)
{
    uint32_t ui32Status;

    //
    // Run MSPI DDR timing scan
    //
    am_devices_mspi_psram_ddr_timing_config_t MSPIDdrTimingConfig;
    am_util_debug_printf("Starting MSPI DDR Timing Scan: \n");
    if ( AM_DEVICES_MSPI_PSRAM_STATUS_SUCCESS == am_devices_mspi_psram_aps25616n_ddr_init_timing_check(MSPI_PSRAM_MODULE, &g_sMspiPsramConfig, &MSPIDdrTimingConfig) )
    {
        am_util_debug_printf("==== Scan Result: RXDQSDELAY0 = %d \n", MSPIDdrTimingConfig.ui32Rxdqsdelay);
    }
    else
    {
        am_util_debug_printf("==== Scan Result: Failed, no valid setting.  \n");
    }

    //
    // Configure the MSPI and PSRAM Device.
    //
    ui32Status = am_devices_mspi_psram_aps25616n_ddr_init(MSPI_PSRAM_MODULE, &g_sMspiPsramConfig, &g_pPsramHandle, &g_pMSPIPsramHandle);
    if (AM_DEVICES_MSPI_PSRAM_STATUS_SUCCESS != ui32Status)
    {
        am_util_stdio_printf("Failed to configure the MSPI and PSRAM Device correctly!\n");
    }
    NVIC_SetPriority(MspiInterrupts[MSPI_PSRAM_MODULE], AM_IRQ_PRIORITY_DEFAULT);
    NVIC_EnableIRQ(MspiInterrupts[MSPI_PSRAM_MODULE]);
    am_hal_interrupt_master_enable();

    //
    // Apply DDR timing setting
    //
    ui32Status = am_devices_mspi_psram_aps25616n_apply_ddr_timing(g_pPsramHandle, &MSPIDdrTimingConfig);
    if (AM_HAL_STATUS_SUCCESS != ui32Status)
    {
        am_util_stdio_printf("Failed to apply the timming scan parameter!\n");
    }

    return AM_HAL_STATUS_SUCCESS;
}

static uint32_t ssbl_mspi_psram_erase(uint32_t ui32StartAddr, uint32_t ui32NumBytes)
{
    return AM_HAL_STATUS_SUCCESS;
}

static uint32_t ssbl_mspi_psram_write(uint8_t *pui8WrBuf, uint32_t ui32WriteAddr, uint32_t ui32NumBytes)
{
    uint32_t ui32Status;

    if ((ui32WriteAddr < MSPI_APERTURE_START_ADDRn(MSPI_PSRAM_MODULE)) || (ui32WriteAddr > MSPI_APERTURE_END_ADDRn(MSPI_PSRAM_MODULE)))
    {
        return AM_HAL_STATUS_FAIL;
    }

    ui32WriteAddr -= MSPI_APERTURE_START_ADDRn(MSPI_PSRAM_MODULE);
    if (ui32WriteAddr & (AM_DEVICES_MSPI_PSRAM_TEST_BLOCK_SIZE - 1) &&
       (ui32WriteAddr + ui32NumBytes) > PSRAM_TOTAL_SIZE)
    {
        return AM_HAL_STATUS_FAIL;
    }

    am_util_stdio_printf("Writing %d Bytes to Addr %d\n", ui32NumBytes, ui32WriteAddr);
    ui32Status = am_devices_mspi_psram_aps25616n_ddr_write(g_pPsramHandle, pui8WrBuf, ui32WriteAddr, ui32NumBytes, true);
    if (AM_DEVICES_MSPI_PSRAM_STATUS_SUCCESS != ui32Status)
    {
        am_util_stdio_printf("Failed to write buffer to PSRAM Device!\n");
        return AM_HAL_STATUS_FAIL;
    }

    return AM_HAL_STATUS_SUCCESS;
}

static uint32_t ssbl_mspi_psram_read(uint8_t *pui8RdBuf, uint32_t ui32ReadAddr, uint32_t ui32NumBytes)
{
    uint32_t ui32Status;
    //
    // Read the data back into the RX buffer.
    //
    if ((ui32ReadAddr < MSPI_APERTURE_START_ADDRn(MSPI_PSRAM_MODULE)) || (ui32ReadAddr > MSPI_APERTURE_END_ADDRn(MSPI_PSRAM_MODULE)))
    {
        return AM_HAL_STATUS_FAIL;
    }

    ui32ReadAddr -= MSPI_APERTURE_START_ADDRn(MSPI_PSRAM_MODULE);
    am_util_stdio_printf("Read %d Bytes from Addr %d\n", ui32NumBytes, ui32ReadAddr);
    ui32Status = am_devices_mspi_psram_aps25616n_ddr_read(g_pPsramHandle, (uint8_t *)pui8RdBuf, ui32ReadAddr, ui32NumBytes, true);
    if (AM_DEVICES_MSPI_PSRAM_STATUS_SUCCESS != ui32Status)
    {
        am_util_stdio_printf("Failed to read buffer to PSRAM Device!\n");
        return AM_HAL_STATUS_FAIL;
    }

    return AM_HAL_STATUS_SUCCESS;
}

static uint32_t ssbl_mspi_psram_enable_xip(void)
{
    return am_devices_mspi_psram_aps25616n_ddr_enable_xip(g_pPsramHandle);
}

static uint32_t ssbl_mspi_psram_disable_xip(void)
{
    return am_devices_mspi_psram_aps25616n_ddr_disable_xip(g_pPsramHandle);
}

am_ssbl_flash_t mspi_psram_info =
{
    .devName = "mspi aps25616n psram",
    .ui32BaseAddr = MSPI_APERTURE_START_ADDRn(MSPI_PSRAM_MODULE),
    .bXip = true,
    .bInternal = false,
    .ui32Size = PSRAM_TOTAL_SIZE,
    .flash_erase = ssbl_mspi_psram_erase,
    .flash_enable_xip = ssbl_mspi_psram_enable_xip,
    .flash_disable_xip = ssbl_mspi_psram_disable_xip,
    .flash_init = ssbl_mspi_psram_init,
    .flash_write = ssbl_mspi_psram_write,
    .flash_read = ssbl_mspi_psram_read,
};

