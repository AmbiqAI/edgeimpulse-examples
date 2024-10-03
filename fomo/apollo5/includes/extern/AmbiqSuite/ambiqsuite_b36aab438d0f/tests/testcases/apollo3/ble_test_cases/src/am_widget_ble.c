//*****************************************************************************
//
//! @file am_widget_ble.c
//!
//! @brief BLE test widget.
//!
//! @addtogroup
//! @ingroup
//! @{
//
//*****************************************************************************

//*****************************************************************************
//
// ${copyright}
//
// This is part of revision ${version} of the AmbiqSuite Development Package.
//
//*****************************************************************************

#include <stdint.h>
#include <stdbool.h>
#include "string.h"
#include "am_mcu_apollo.h"
#include "am_widget_ble.h"
#include "am_util.h"

//*****************************************************************************
//
// Global variables.
//
//*****************************************************************************
void *pBLE;

//
// Variables for observing the state of the BLE module.
//
typedef struct
{
    uint32_t ui32Status;
    uint32_t ui32Count;
}
statuscount_t;

statuscount_t g_pui32IntStatus[1024];
uint32_t StatusIdx = 0;

//
// Data buffers.
//
am_hal_ble_buffer(512) psWriteBuffer;
am_hal_ble_buffer(512) psReadBuffer;

//
// Non-blocking test case tracking variables.
//
bool g_bNonBlockingEnabled = false;
volatile bool bNonBlockingReadComplete = false;
volatile bool bStatusHigh = false;

//*****************************************************************************
//
// Static function prototypes.
//
//*****************************************************************************
static bool buffer_compare(void *b1, void *b2, uint32_t len);

//*****************************************************************************
//
// Standard error-check function.
//
//*****************************************************************************
#define ERROR_CHECK(x) error_check(x, __FILE__, __LINE__)

#define CONVERT_ERROR(x)                                                      \
    case x: am_util_stdio_printf(#x); break;

static void
print_error_message(uint32_t ui32ErrorCode)
{
    switch (ui32ErrorCode)
    {
        CONVERT_ERROR(AM_HAL_BLE_STATUS_BUS_BUSY)
        CONVERT_ERROR(AM_HAL_BLE_STATUS_IRQ_LOW)
        CONVERT_ERROR(AM_HAL_BLE_STATUS_SPI_NOT_READY)
        CONVERT_ERROR(AM_HAL_BLE_REQUESTING_READ)
        CONVERT_ERROR(AM_HAL_BLE_NO_HCI_RESPONSE)
        CONVERT_ERROR(AM_HAL_BLE_FEATURE_DISABLED)
        CONVERT_ERROR(AM_HAL_BLE_SHUTDOWN_FAILED)
        CONVERT_ERROR(AM_HAL_BLE_REGULATOR_FAILED)
        CONVERT_ERROR(AM_HAL_BLE_POWERUP_INCOMPLETE)
        CONVERT_ERROR(AM_HAL_BLE_HCI_PACKET_INCOMPLETE)
        CONVERT_ERROR(AM_HAL_BLE_FIFO_ERROR)
        CONVERT_ERROR(AM_HAL_BLE_32K_CLOCK_UNSTABLE)

        CONVERT_ERROR(AM_HAL_STATUS_TIMEOUT)

        default:
            am_util_stdio_printf("ERROR_UNKNOWN", ui32ErrorCode);
            break;
    }
}

static void
error_check(uint32_t ui32ErrorCode, const char *pcFile, uint32_t ui32Line)
{
    if (ui32ErrorCode != AM_HAL_STATUS_SUCCESS)
    {
        am_util_stdio_printf("File \"%s\", Line %d \n ERROR status: 0x%08X, ",
                             pcFile, ui32Line, ui32ErrorCode);

        print_error_message(ui32ErrorCode);
        while (1);
    }
}

//*****************************************************************************
//
// Set up the BLE module for testing.
//
//*****************************************************************************
void
am_widget_ble_setup(void **ppWidget, char *pErrStr)
{
    am_hal_ble_initialize(0, &pBLE);
    ERROR_CHECK(am_hal_ble_power_control(pBLE, AM_HAL_BLE_POWER_ACTIVE));
    ERROR_CHECK(am_hal_ble_config(pBLE, &am_hal_ble_default_config));

    am_util_stdio_printf("Initializing radio...\n");
    am_util_stdio_printf("If the test sticks here, try holding the Nationz reset during startup.\n");
    ERROR_CHECK(am_hal_ble_boot(pBLE));

    NVIC_EnableIRQ(BLE_IRQn);

    uint32_t ui32ExpectedInterrupts =
        (BLEIF_INTSTAT_B2MACTIVE_Msk    |
         BLEIF_INTSTAT_B2MSLEEP_Msk     |
         BLEIF_INTSTAT_DERR_Msk         |
         BLEIF_INTSTAT_BLECIRQ_Msk      |
         BLEIF_INTSTAT_BLECSSTAT_Msk    |
         BLEIF_INTSTAT_DCMP_Msk         |
         BLEIF_INTSTAT_ICMD_Msk         |
         BLEIF_INTSTAT_IACC_Msk         |
         BLEIF_INTSTAT_B2MST_Msk        |
         BLEIF_INTSTAT_FOVFL_Msk        |
         BLEIF_INTSTAT_FUNDFL_Msk       |
         BLEIF_INTSTAT_THR_Msk          |
         BLEIF_INTSTAT_CMDCMP_Msk);

    if (APOLLO3_GE_B0)
    {
        ui32ExpectedInterrupts |= BLEIF_INTSTAT_B2MSHUTDN_Msk;
    }

    am_hal_ble_int_enable(pBLE, ui32ExpectedInterrupts);

    am_util_stdio_printf("...Initialization complete.\n\n");
}

void
am_widget_ble_test_config(void *pWidget, am_hal_ble_config_t *pBleConfig)
{
    am_hal_ble_config(pBLE, pBleConfig);
}

bool
am_widget_ble_test_status_check(void *pWidget, char *pErrStr)
{
    uint8_t pui8RawResetCommand[4] = {0x1, 0x3, 0xC, 0x0};

    uint32_t ui32ExpectedInterrupts =
        (BLEIF_INTSTAT_B2MACTIVE_Msk    |
         BLEIF_INTSTAT_B2MSLEEP_Msk     |
         BLEIF_INTSTAT_DERR_Msk         |
         BLEIF_INTSTAT_BLECIRQ_Msk      |
         BLEIF_INTSTAT_BLECSSTAT_Msk    |
         BLEIF_INTSTAT_DCMP_Msk         |
         BLEIF_INTSTAT_ICMD_Msk         |
         BLEIF_INTSTAT_IACC_Msk         |
         BLEIF_INTSTAT_B2MST_Msk        |
         BLEIF_INTSTAT_FOVFL_Msk        |
         BLEIF_INTSTAT_FUNDFL_Msk       |
         BLEIF_INTSTAT_THR_Msk          |
         BLEIF_INTSTAT_CMDCMP_Msk);

    if (APOLLO3_GE_B0)
    {
        ui32ExpectedInterrupts |= BLEIF_INTSTAT_B2MSHUTDN_Msk;
    }

    //
    // Make sure we don't service interrupts we don't need.
    //
    g_bNonBlockingEnabled = 0;

    //
    // Enable all interrupts.
    //
    am_hal_ble_int_clear(pBLE, 0xFFFFFFFF);

    //
    // Force overflow
    //
    for (uint32_t ui32Index = 0; ui32Index < 1024; ui32Index++)
    {
        BLEIF->FIFOPUSH = 0x01234567;
    }

    //
    // Force underflow.
    //
    volatile uint32_t dummy;
    for (uint32_t ui32Index = 0; ui32Index < 8; ui32Index++)
    {
        dummy = BLEIF->FIFOPOP;
        BLEIF->FIFOPOP = 1;
    }

    //
    // Send a RESET command to set the Apollo up to do a read operation.
    //
    BLEIF->FIFOCTRL_b.FIFORSTN = 0;
    am_util_delay_us(100);
    BLEIF->FIFOCTRL_b.FIFORSTN = 1;

    am_hal_ble_int_clear(pBLE, 0xFFFFFFFF);
    memcpy(psWriteBuffer.bytes, pui8RawResetCommand, 4);
    ERROR_CHECK(am_hal_ble_blocking_hci_write(pBLE, AM_HAL_BLE_RAW, psWriteBuffer.words, 4));
    while ( BLEIF->BSTATUS_b.BLEIRQ == 0 );

    //
    // Force DMA error by trying to read into flash.
    //
    BLEIF->DMATARGADDR = 0x0;

    BLEIF->DMATOTCOUNT =  9;

    BLEIF->DMATRIGEN =  (BLEIF_DMATRIGEN_DTHREN_Msk     |
                         BLEIF_DMATRIGEN_DCMDCMPEN_Msk);

    BLEIF->DMACFG =  (_VAL2FLD(BLEIF_DMACFG_DMADIR, BLEIF_DMACFG_DMADIR_P2M)    |
                      _VAL2FLD(BLEIF_DMACFG_DMAPRI, BLEIF_DMACFG_DMAPRI_HIGH)   |
                      _VAL2FLD(BLEIF_DMACFG_DMAEN,  BLEIF_DMACFG_DMAEN_EN));

    BLEIF->OFFSETHI = 0;
    BLEIF->CMD = (_VAL2FLD(BLEIF_CMD_OFFSETCNT, 0)  |
                  _VAL2FLD(BLEIF_CMD_TSIZE, 9)      |
                  AM_HAL_BLE_READ);

    //
    // Force a BLE command error.
    //
    BLEIF->CMD = 0xFFFFFFFF;

    //
    // Delay to make sure all of the interrupts are received.
    //
    am_util_delay_ms(100);

    uint32_t ui32CombinedStatus = 0;

    for (uint32_t i = 0; i <= StatusIdx; i++)
    {
        ui32CombinedStatus |= g_pui32IntStatus[i].ui32Status;
    }

    if (ui32CombinedStatus == ui32ExpectedInterrupts)
    {
        return true;
    }
    else
    {
        return false;
    }
}

bool
am_widget_ble_test_blocking_hci(void *pWidget, char *pErrStr)
{
    uint8_t pui8ResetType = 0x1;
    uint8_t pui8ResetCommand[3] = {0x3, 0xC, 0x0};
    uint8_t pui8RawResetCommand[4] = {0x1, 0x3, 0xC, 0x0};
    uint8_t pui8ExpectedRead[] = {0x4, 0xE, 0x4, 0x5, 0x3, 0xC, 0x0};
    uint8_t ui8ExpectedReadLen = sizeof(pui8ExpectedRead);

    //
    // Make sure we don't get interrupts we don't need.
    //
    g_bNonBlockingEnabled = 0;

    //
    // Try a write/read with the normal API.
    //
    memcpy(psWriteBuffer.bytes, pui8ResetCommand, 3);
    ERROR_CHECK(am_hal_ble_blocking_hci_write(pBLE, pui8ResetType, psWriteBuffer.words, 3));
    while (BLEIF->BSTATUS_b.BLEIRQ == 0);
    ERROR_CHECK(am_hal_ble_blocking_hci_read(pBLE, psReadBuffer.words, 0));

    if (!buffer_compare(psReadBuffer.bytes, pui8ExpectedRead, ui8ExpectedReadLen))
    {
        am_util_stdio_sprintf(pErrStr, "Blocking normal hci write/read failure.\n");
        return false;
    }

    //
    // Try a "raw mode" command.
    //
    memcpy(psWriteBuffer.bytes, pui8RawResetCommand, 4);
    ERROR_CHECK(am_hal_ble_blocking_hci_write(pBLE, AM_HAL_BLE_RAW, psWriteBuffer.words, 4));
    while ( BLEIF->BSTATUS_b.BLEIRQ == 0 );
    ERROR_CHECK(am_hal_ble_blocking_hci_read(pBLE, psReadBuffer.words, 0));

    if (!buffer_compare(psReadBuffer.bytes, pui8ExpectedRead, ui8ExpectedReadLen))
    {
        am_util_stdio_sprintf(pErrStr, "Blocking raw hci write/read failure.\n");
        return false;
    }

    //
    // Try a vendor-specific command.
    //
    uint8_t pui8BdAddr[6] = {0x1, 0x2, 0x3, 0x4, 0x5, 0x6};
    uint8_t pui8BdAddrResponse[] = {0x04, 0x0E, 0x04, 0x05, 0x32, 0xFC, 0x00};
    uint8_t pui8BdAddrResponseLen = sizeof(pui8BdAddrResponse);

    am_hal_ble_vs_command_build(psWriteBuffer.words,
                                AM_HAL_BLE_SET_BD_ADDR_OPCODE,
                                AM_HAL_BLE_SET_BD_ADDR_LENGTH,
                                pui8BdAddr);

    ERROR_CHECK(am_hal_ble_blocking_hci_write(pBLE, AM_HAL_BLE_RAW, psWriteBuffer.words,
                                              AM_HAL_BLE_SET_BD_ADDR_LENGTH));

    while (BLEIF->BSTATUS_b.BLEIRQ == 0);

    ERROR_CHECK(am_hal_ble_blocking_hci_read(pBLE, psReadBuffer.words, 0));

    if (!buffer_compare(psReadBuffer.bytes, pui8BdAddrResponse, pui8BdAddrResponseLen))
    {
        am_util_stdio_sprintf(pErrStr, "Vendor-specific hci write/read failure.\n");
        return false;
    }

    return true;
}

//*****************************************************************************
//
// Call this function when non-blocking reads come back.
//
//*****************************************************************************
void
am_widget_ble_test_nonblocking_read_cb(uint8_t *pui8Data, uint32_t ui32Length, void *pvContext)
{
    bNonBlockingReadComplete = true;
}

void
am_widget_ble_test_nonblocking_write_cb(uint8_t *pui8Data, uint32_t ui32Length, void *pvContext)
{
    //
    // Lower wake.
    //
    ERROR_CHECK(am_hal_ble_wakeup_set(pBLE, 0));
    bStatusHigh = false;
}

uint32_t
check_hci_nb_read_packet(uint32_t ui32Timeout)
{
    uint32_t i, ui32Status;

    ui32Status = AM_HAL_STATUS_TIMEOUT;

    for (i = 0; i < ui32Timeout; i++)
    {
        if (bNonBlockingReadComplete)
        {
            ui32Status = AM_HAL_STATUS_SUCCESS;
            break;
        }

        am_util_delay_ms(1);
    }

    return ui32Status;
}

uint32_t
wait_for_status(uint32_t ui32Timeout)
{
    uint32_t i, ui32Status;

    ui32Status = AM_HAL_STATUS_TIMEOUT;

    for (i = 0; i < ui32Timeout; i++)
    {
        if (bStatusHigh)
        {
            ui32Status = AM_HAL_STATUS_SUCCESS;
            break;
        }

        am_util_delay_ms(1);
    }

    return ui32Status;
}

bool
am_widget_ble_test_nonblocking_hci(void *pWidget, char *pErrStr)
{
    uint8_t pui8ResetType = 0x1;
    uint8_t pui8ResetCommand[3] = {0x3, 0xC, 0x0};
    uint8_t pui8RawResetCommand[4] = {0x1, 0x3, 0xC, 0x0};
    uint8_t pui8ExpectedRead[] = {0x4, 0xE, 0x4, 0x5, 0x3, 0xC, 0x0};
    uint8_t ui8ExpectedReadLen = sizeof(pui8ExpectedRead);
    uint32_t ui32Error;

    //
    // Enable the nonblocking interface.
    //
    g_bNonBlockingEnabled = 1;
    bNonBlockingReadComplete = false;

    //
    // Try a write/read with the normal API.
    //
    memcpy(psWriteBuffer.bytes, pui8ResetCommand, 3);

    //
    // Wake up the BLE
    //
    bStatusHigh = false;
    ERROR_CHECK(am_hal_ble_wakeup_set(pBLE, 1));
    ui32Error = wait_for_status(1000);
    ERROR_CHECK(ui32Error);
    if (ui32Error)
    {
        am_util_stdio_sprintf(pErrStr, "BLE Controller not ready (STATUS LOW).");
        return false;
    }

    //
    // Send the message.
    //
    ERROR_CHECK(am_hal_ble_nonblocking_hci_write(pBLE, pui8ResetType, psWriteBuffer.words, 3,
                                                 am_widget_ble_test_nonblocking_write_cb, 0));

    //
    // Check to make sure we actually got a packet back.
    //
    ui32Error = check_hci_nb_read_packet(1000);
    ERROR_CHECK(ui32Error);
    if (ui32Error)
    {
        am_util_stdio_sprintf(pErrStr, "BLE Controller didn't respond.");
        return false;
    }

    if (!buffer_compare(psReadBuffer.bytes, pui8ExpectedRead, ui8ExpectedReadLen))
    {
        am_util_stdio_sprintf(pErrStr, "Non-blocking normal hci write/read failure.\n");
        return false;
    }

    //
    // Try a "raw mode" command.
    //
    bNonBlockingReadComplete = false;
    memcpy(psWriteBuffer.bytes, pui8RawResetCommand, 4);

    //
    // Wake up the BLE
    //
    bStatusHigh = false;
    ERROR_CHECK(am_hal_ble_wakeup_set(pBLE, 1));
    ui32Error = wait_for_status(1000);
    ERROR_CHECK(ui32Error);
    if (ui32Error)
    {
        am_util_stdio_sprintf(pErrStr, "BLE Controller not ready (STATUS LOW).");
        return false;
    }

    ERROR_CHECK(am_hal_ble_nonblocking_hci_write(pBLE, AM_HAL_BLE_RAW, psWriteBuffer.words, 4,
                                                 am_widget_ble_test_nonblocking_write_cb, 0));

    ui32Error = check_hci_nb_read_packet(1000);
    ERROR_CHECK(ui32Error);
    if (ui32Error)
    {
        am_util_stdio_sprintf(pErrStr, "BLE Controller didn't respond.");
        return false;
    }

    if (!buffer_compare(psReadBuffer.bytes, pui8ExpectedRead, ui8ExpectedReadLen))
    {
        am_util_stdio_sprintf(pErrStr, "Non-blocking raw hci write/read failure.\n");
        return false;
    }

    return true;
}

bool
am_widget_ble_sleep_test(void *pWidget, char *pErrStr)
{
    return true;
}

//*****************************************************************************
//
// Check two buffers for equality. Return true if they are the same.
//
//*****************************************************************************
static bool
buffer_compare(void *b1, void *b2, uint32_t len)
{
    uint8_t *p1 = b1;
    uint8_t *p2 = b2;

    for (uint32_t i = 0; i < len; i++)
    {
        if (p1[i] != p2[i])
        {
            return false;
        }
    }

    return true;
}

void
am_ble_isr(void)
{
    uint32_t ui32Status;

    //
    // Count interrupts.
    //
    ui32Status = am_hal_ble_int_status(pBLE, true);
    am_hal_ble_int_clear(pBLE, ui32Status);

    //
    // Save every unique combination of interrupts that comes in.
    //
    if (g_pui32IntStatus[StatusIdx].ui32Status != ui32Status)
    {
        StatusIdx++;
        g_pui32IntStatus[StatusIdx].ui32Status = ui32Status;
        g_pui32IntStatus[StatusIdx].ui32Count = 1;
    }
    else
    {
        g_pui32IntStatus[StatusIdx].ui32Count++;
    }

    while (StatusIdx == 1024);

    if (g_bNonBlockingEnabled)
    {
        //
        // Check to see if the BLE radio is asking for a read transaction. If
        // so, perform an HCI read.
        //
        if (ui32Status & BLEIF_INTEN_BLECIRQ_Msk)
        {
            am_hal_ble_int_clear(pBLE, BLEIF_INTEN_BLECIRQ_Msk);
            am_hal_ble_nonblocking_hci_read(pBLE, psReadBuffer.words,
                                            am_widget_ble_test_nonblocking_read_cb, 0);
        }
        //
        // Check to see if the BLE radio is asking for a read transaction. If
        // so, perform an HCI read.
        //
        if (ui32Status & BLEIF_INTEN_BLECSSTAT_Msk)
        {
            bStatusHigh = true;
        }

        //
        // Handle any transactions in progress.
        //
        am_hal_ble_int_service(pBLE, ui32Status);
    }
}

//*****************************************************************************
//
// End Doxygen group.
//! @}
//
//*****************************************************************************
