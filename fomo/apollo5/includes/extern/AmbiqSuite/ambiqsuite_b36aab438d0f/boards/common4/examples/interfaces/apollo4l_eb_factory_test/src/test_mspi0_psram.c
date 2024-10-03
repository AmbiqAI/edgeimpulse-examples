//*****************************************************************************
//
// ${copyright}
//
// This is part of revision ${version} of the AmbiqSuite Development Package.
//
//*****************************************************************************

#include "am_mcu_apollo.h"
#include "am_bsp.h"
#include "am_devices_mspi_psram_aps25616n.h"
#include "am_util.h"
#include "factory_test_helpers.h"
//*****************************************************************************
//
// Macro definitions
//
//*****************************************************************************

#define PSRAM_TEST_MODULE               0
#define MSPI_BUFFER_SIZE                (1024)
#define DDR_PSRAM_TARGET_ADDRESS        (0)

//*****************************************************************************
//
// Variables Definition
//
//*****************************************************************************
static uint8_t         g_TXBuffer[MSPI_BUFFER_SIZE];
static uint8_t         g_RXBuffer[MSPI_BUFFER_SIZE];

uint32_t ui32DMATCBBuffer1[2560];
void            *g_pPsramHandle;
void            *g_pMSPIPsramHandle;
am_devices_mspi_psram_config_t MSPI_PSRAM_HexCE0MSPIConfig =
{
    .eDeviceConfig            = AM_HAL_MSPI_FLASH_OCTAL_DDR_CE0,
    .eClockFreq               = AM_HAL_MSPI_CLK_48MHZ,
    .ui32NBTxnBufLength       = sizeof(ui32DMATCBBuffer1) / sizeof(uint32_t),
    .pNBTxnBuf                = ui32DMATCBBuffer1,
    .ui32ScramblingStartAddr  = 0,
    .ui32ScramblingEndAddr    = 0,
};

//! MSPI interrupts.
static const IRQn_Type mspi_interrupts[] =
{
    MSPI0_IRQn,
    MSPI1_IRQn,
    MSPI2_IRQn,
};

//
// Take over the interrupt handler for whichever MSPI we're using.
//
#define psram_mspi_isr                                                          \
    am_mspi_isr1(PSRAM_TEST_MODULE)
#define am_mspi_isr1(n)                                                        \
    am_mspi_isr(n)
#define am_mspi_isr(n)                                                         \
    am_mspi ## n ## _isr

//*****************************************************************************
//
// MSPI ISRs.
//
//*****************************************************************************
void psram_mspi_isr(void *argc)
{
    uint32_t      ui32Status;

    am_hal_mspi_interrupt_status_get(g_pMSPIPsramHandle, &ui32Status, false);

    am_hal_mspi_interrupt_clear(g_pMSPIPsramHandle, ui32Status);

    am_hal_mspi_interrupt_service(g_pMSPIPsramHandle, ui32Status);
}

int mspi0_hex_ddr_aps256_psram_test(uint32_t mode)
{
    uint32_t ui32Status;
    am_util_debug_printf("Apollo4Plus MSPI HEX DDR APS25616N PSRAM Example\n\n");

    //
    // Configure the MSPI and PSRAM Device.
    //
    ui32Status = am_devices_mspi_psram_aps25616n_ddr_init(PSRAM_TEST_MODULE, &MSPI_PSRAM_HexCE0MSPIConfig, &g_pPsramHandle, &g_pMSPIPsramHandle);

    if (AM_DEVICES_MSPI_PSRAM_STATUS_SUCCESS != ui32Status)
    {
      am_util_stdio_printf("Failed to configure the MSPI and PSRAM Device correctly!\n");
      am_util_stdio_printf("Must ensure Device is Connected and Properly Initialized!\n");
      goto _fail;
    }

    NVIC_SetPriority(mspi_interrupts[PSRAM_TEST_MODULE], AM_IRQ_PRIORITY_DEFAULT);
    NVIC_EnableIRQ(mspi_interrupts[PSRAM_TEST_MODULE]);

    am_hal_interrupt_master_enable();

    //
    // Generate data into the Sector Buffer
    //
    for (uint32_t i = 0; i < MSPI_BUFFER_SIZE; i++)
    {
      g_TXBuffer[i] = (i & 0xFF);
      g_RXBuffer[i] = 0x00;
    }

    //
    // Make sure we aren't in XIP mode.
    //
    ui32Status = am_devices_mspi_psram_aps25616n_ddr_disable_xip(g_pPsramHandle);
    if (AM_DEVICES_MSPI_PSRAM_STATUS_SUCCESS != ui32Status)
    {
      am_util_stdio_printf("Failed to disable XIP mode in the MSPI!\n");
      goto _fail;
    }

    //
    // Write the TX buffer into the target sector.
    //
    am_util_debug_printf("Writing %d Bytes to Address 0x%X\n", MSPI_BUFFER_SIZE, DDR_PSRAM_TARGET_ADDRESS);

    ui32Status = am_devices_mspi_psram_aps25616n_ddr_write(g_pPsramHandle, g_TXBuffer, DDR_PSRAM_TARGET_ADDRESS, MSPI_BUFFER_SIZE, true);

    if (AM_DEVICES_MSPI_PSRAM_STATUS_SUCCESS != ui32Status)
    {
      am_util_stdio_printf("Failed to write buffer to Flash Device!\n");
      goto _fail;
    }

    //
    // Read the data back into the RX buffer.
    //
    am_util_debug_printf("Read %d Bytes to Address 0x%X\n", MSPI_BUFFER_SIZE, DDR_PSRAM_TARGET_ADDRESS);

    ui32Status = am_devices_mspi_psram_aps25616n_ddr_read(g_pPsramHandle, g_RXBuffer, DDR_PSRAM_TARGET_ADDRESS, MSPI_BUFFER_SIZE, true);


    if (AM_DEVICES_MSPI_PSRAM_STATUS_SUCCESS != ui32Status)
    {
      am_util_stdio_printf("Failed to read buffer to Flash Device!\n");
      goto _fail;
    }

    //
    // Compare the buffers
    //
    am_util_debug_printf("Comparing the TX and RX Buffers\n");
    //
    // check if block data match or not
    //
    if ( memory_compare( (const void *)g_TXBuffer, (const void *)g_RXBuffer, MSPI_BUFFER_SIZE ) != 0 )
    {
        am_util_stdio_printf("TX and RX buffers failed to compare!\n");
        goto _fail;
    }

    //
    //  End banner.
    //
    am_util_debug_printf( "\nApollo4Plus MSPI HEX DDR APS25616N PSRAM Example Complete\n");


    //
    // Clean up the MSPI before exit.
    //
    NVIC_DisableIRQ(mspi_interrupts[PSRAM_TEST_MODULE]);

    ui32Status = am_devices_mspi_psram_aps25616n_ddr_deinit(g_pPsramHandle);
    if (AM_DEVICES_MSPI_PSRAM_STATUS_SUCCESS != ui32Status)
    {
        am_util_stdio_printf("Failed to shutdown the MSPI and Flash Device!\n");
        goto _fail;
    }
    return 0;
_fail:
    return -1;
}
