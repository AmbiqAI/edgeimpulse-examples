//*****************************************************************************
//
//! @file ble_power_up_helpers.c
//!
//! @brief Helper functions to run the tests for the ble_power_up test.
//!
//! @addtogroup
//! @ingroup
//! @{
//
//*****************************************************************************

#include <string.h>
#include "am_mcu_apollo.h"
#include "am_bsp.h"
#include "am_util.h"

#include "ble_power_up_helpers.h"

//*****************************************************************************
//
// ${copyright}
//
// This is part of revision ${version} of the AmbiqSuite Development Package.
//
//*****************************************************************************

//*****************************************************************************
//
// Test configuration.
//
//*****************************************************************************
#define NUM_TRIALS                  10

//*****************************************************************************
//
// Timeout variables.
//
//*****************************************************************************
#define GENERIC_TIMEOUT             1
#define POWER_UP_TIMEOUT_MS         1000
#define HCI_RESPONSE_TIMEOUT        4000

//*****************************************************************************
//
// Helper macros.
//
//*****************************************************************************
#define WHILE_TIMEOUT(expr, timeout, errno)                                      \
    {                                                                         \
        uint32_t ui32Timeout = 0;                                             \
        while ((expr))                                                        \
        {                                                                     \
            if (ui32Timeout == (timeout))                                     \
            {                                                                 \
                return (errno);                                               \
            }                                                                 \
                                                                              \
            am_util_delay_ms(1);                                              \
            ui32Timeout++;                                                    \
        }                                                                     \
    }

//*****************************************************************************
//
// Forward declarations.
//
//*****************************************************************************
static void print_packet(const uint8_t *pui8Packet, uint32_t ui32Length);
static uint32_t check_response(const uint8_t *pui8Data, uint32_t ui32Length);
static uint32_t send_command(const uint8_t *pui8Data, uint32_t ui32Length);
static uint32_t test_commands(void);

//*****************************************************************************
//
// Global variables.
//
//*****************************************************************************
void *BLE;

//*****************************************************************************
//
// Return codes.
//
//*****************************************************************************
typedef enum
{
    BLE_POWER_SUCCESS,
    BLE_POWER_ERROR_MODULE_NOT_DISABLED,
    BLE_POWER_ERROR_FEATURE_NOT_ENABLED,
    BLE_POWER_ERROR_POWER_STATUS,
    BLE_POWER_ERROR_POWER_STATE,
    BLE_NO_HCI_RESPONSE,
    BLE_NO_STATUS,
    BLE_BAD_RESPONSE,
    BLE_HAL_ERROR,
    BLE_PATCH_ERROR,
}
ble_power_test_status_t;

//*****************************************************************************
//
// Print messages for the main application.
//
//*****************************************************************************
static void
ble_power_up_print_status(uint32_t ui32Status)
{
    switch (ui32Status)
    {
        case BLE_POWER_SUCCESS:
            am_util_stdio_printf("Success\n");
            break;

        case BLE_POWER_ERROR_MODULE_NOT_DISABLED:
            am_util_stdio_printf("ERROR, BLE enabled when it was expected to be off.");
            break;

        case BLE_POWER_ERROR_FEATURE_NOT_ENABLED:
            am_util_stdio_printf("ERROR, FEATUREENABLE could not be set.");
            break;

        case BLE_POWER_ERROR_POWER_STATUS:
            am_util_stdio_printf("ERROR, DEVPWRSTATUS didn't get set.");
            break;

        case BLE_POWER_ERROR_POWER_STATE:
            am_util_stdio_printf("ERROR, BLE module powered, but BSTATUS doesn't indicate 'active'.");
            break;

        case BLE_NO_HCI_RESPONSE:
            am_util_stdio_printf("ERROR, HCI command not acknowledged.");
            break;

        case BLE_NO_STATUS:
            am_util_stdio_printf("ERROR, STATUS signal is stuck LOW.");
            break;

        case BLE_PATCH_ERROR:
            am_util_stdio_printf("ERROR, patching failed.");
            break;

        case BLE_BAD_RESPONSE:
            break;

        case BLE_HAL_ERROR:
            break;
    }

    am_util_stdio_printf("\n");
}

//*****************************************************************************
//
// Enables the BLE module.
//
//*****************************************************************************
uint32_t
ble_test_run(void)
{
    uint32_t ui32Trial, ui32Status;

    uint32_t ui32NumFailures = 0;

    for (ui32Trial = 0; ui32Trial < NUM_TRIALS; ui32Trial++)
    {
        //
        // Print something to signal the start of the trial.
        //
        am_util_stdio_printf("Trial %d: ", ui32Trial + 1);
        AM_UPDATE_TEST_PROGRESS();

        //
        // Perform the error-sensitive operations.
        //
        do
        {
            //
            // Power on.
            //
            am_hal_ble_initialize(0, &BLE);
            ui32Status = am_hal_ble_power_control(BLE, AM_HAL_BLE_POWER_ACTIVE);
            if (ui32Status != BLE_POWER_SUCCESS)
            {
              am_util_stdio_printf("Failed to enable BLEIF power domain\n");
              break;
            }
            
            //
            // Configure.
            //
            ui32Status = am_hal_ble_config(BLE, &am_hal_ble_default_config);
            if (ui32Status != BLE_POWER_SUCCESS)
            {
              am_util_stdio_printf("Failed to configure the BLEIF\n");
            }
            
            //
            // Delay 1s for 32768Hz clock stability. This isn't required unless this is
            // our first run immediately after a power-up.
            //
            if ( ui32Trial == 0 )
            {
              am_util_stdio_printf("First trial. Delaying for crystal stability...", ui32Status);
              am_util_delay_ms(1000);
              am_util_stdio_printf(" DONE\n", ui32Status);
            }
            
            //
            // Apply the default patch.
            //
            ui32Status = am_hal_ble_boot(BLE);
            if (ui32Status != BLE_POWER_SUCCESS)
            {
              am_util_stdio_printf("Failed to initialize the BLE controller.  Status = %8.8X\n",ui32Status);
              break;
            }

            //
            // Send more commands and check for errors.
            //
            am_util_stdio_printf("\n");
            ui32Status = test_commands();
            if (ui32Status != BLE_POWER_SUCCESS)
            {
                break;
            }

        } while (0);

        //
        // Print errors and power down, no matter what happened.
        //
        ble_power_up_print_status(ui32Status);
        if (ui32Status != BLE_POWER_SUCCESS)
        {
            ui32NumFailures++;
        }

        am_hal_ble_power_control(BLE, AM_HAL_BLE_POWER_OFF);
        am_hal_ble_deinitialize(BLE);
        am_util_delay_ms(100);
    }

    return ui32NumFailures;
}

//*****************************************************************************
//
// A series of test commands to send.
//
//*****************************************************************************
uint8_t pui8Reset[] =                  {0x01, 0x03, 0x0C, 0x00};
uint8_t pui8ResetResponse[] =          {0x04, 0x0E, 0x04, 0x05, 0x03, 0x0C, 0x00};

uint8_t pui8SetEventMask[] =           {0x01, 0x01, 0x0C, 0x08, 0x90, 0x88, 0x00, 0x02, 0x00, 0x80, 0x00, 0x20};
uint8_t pui8SetEventMaskResponse[] =   {0x04, 0x0E, 0x04, 0x05, 0x01, 0x0C, 0x00};

uint8_t pui8SetLEEventMask[] =         {0x01, 0x01, 0x20, 0x08, 0xFF, 0xFF, 0x0F, 0x00, 0x00, 0x00, 0x00, 0x00};
uint8_t pui8SetLEEventMaskResponse[] = {0x04, 0x0E, 0x04, 0x05, 0x01, 0x20, 0x00};

uint8_t pui8SetEventMask2[] =          {0x01, 0x63, 0x0C, 0x08, 0x00, 0x00, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00};
uint8_t pui8SetEventMask2Response[] =  {0x04, 0x0E, 0x04, 0x05, 0x63, 0x0C, 0x00};

uint8_t pui8GetPublicKey[] =           {0x01, 0x25, 0x20, 0x00};
uint8_t pui8GetPublicKeyResponse[] =   {0x04, 0x0F, 0x04, 0x00, 0x05, 0x25, 0x20};

typedef struct
{
    uint8_t *pui8Command;
    uint32_t ui32CmdLen;
    uint8_t *pui8Response;
    uint32_t ui32RespLen;
}
hci_command_test_t;

#define COMMAND(x)  {x, sizeof(x), x ## Response, sizeof(x ## Response)}

hci_command_test_t g_psTestCommands[] =
{
    COMMAND(pui8Reset),
    COMMAND(pui8SetEventMask),
    COMMAND(pui8SetLEEventMask),
    COMMAND(pui8SetEventMask2),
    COMMAND(pui8GetPublicKey),
};

#define NUM_COMMANDS (sizeof(g_psTestCommands) / sizeof(g_psTestCommands[0]))

am_hal_ble_buffer(512) psTxBuffer;
am_hal_ble_buffer(512) psRxBuffer;

//*****************************************************************************
//
// Send an HCI command.
//
//*****************************************************************************
static uint32_t
send_command(const uint8_t *pui8Data, uint32_t ui32Length)
{
    uint32_t ui32Status;

    //
    // Always send from our global buffer.
    //
    memcpy(psTxBuffer.bytes, pui8Data, ui32Length);

    //
    // Print the HCI command.
    //
    am_util_stdio_printf("TX: ");
    print_packet(pui8Data, ui32Length);

    //
    // Send the HCI command.
    //
    ui32Status = am_hal_ble_blocking_hci_write(BLE,
                                               AM_HAL_BLE_RAW,
                                               psTxBuffer.words,
                                               ui32Length);
    if (ui32Status == AM_HAL_STATUS_TIMEOUT)
    {
        return BLE_NO_STATUS;
    }
    else
    {
        return BLE_POWER_SUCCESS;
    }
}

//*****************************************************************************
//
// Obtain a response for the last command sent, and compare it against the
// provided buffer.
//
//*****************************************************************************
static uint32_t
check_response(const uint8_t *pui8Data, uint32_t ui32Length)
{
    uint32_t i;
    uint32_t ui32Status;
    uint32_t ui32BytesReceived;

    //
    // Wait for the IRQ bit, and then read a response.
    //
    WHILE_TIMEOUT(BLEIF->INTSTAT_b.BLECIRQ == 0,
                  HCI_RESPONSE_TIMEOUT,
                  BLE_NO_HCI_RESPONSE);

    BLEIF->INTCLR_b.BLECIRQ = 1;

    //
    // Read into the global buffer.
    //
    ui32Status = am_hal_ble_blocking_hci_read(BLE, psRxBuffer.words,
                                              &ui32BytesReceived);

    if (ui32Status != AM_HAL_STATUS_SUCCESS)
    {
        return BLE_BAD_RESPONSE;
    }

    //
    // Print RX packet.
    //
    am_util_stdio_printf("RX: ");
    print_packet(psRxBuffer.bytes, ui32BytesReceived);

    for (i = 0; i < ui32Length; i++)
    {
        if (pui8Data[i] != psRxBuffer.bytes[i])
        {
            am_util_stdio_printf("ERROR, response incorrect. Expected:\n    ");
            print_packet(pui8Data, ui32Length);
            return BLE_BAD_RESPONSE;
        }
    }

    BLEIF->BLEDBG_b.IOCLKON = BLEIF->BSTATUS_b.BLEIRQ ? 1 : 0;

    return BLE_POWER_SUCCESS;
}

//*****************************************************************************
//
// Run all of our test commands, and check for failures.
//
//*****************************************************************************
static uint32_t
test_commands(void)
{
    uint32_t i;
    uint32_t ui32Status;

    BLEIF->INTCLR_b.BLECIRQ = 1;

    for (i = 0; i < NUM_COMMANDS; i++)
    {
        ui32Status = send_command(g_psTestCommands[i].pui8Command,
                                  g_psTestCommands[i].ui32CmdLen);

        if (ui32Status != BLE_POWER_SUCCESS)
        {
            return ui32Status;
        }

        ui32Status = check_response(g_psTestCommands[i].pui8Response,
                                    g_psTestCommands[i].ui32RespLen);

        if (ui32Status != BLE_POWER_SUCCESS)
        {
            return ui32Status;
        }
    }

    AM_UPDATE_TEST_PROGRESS();

    //
    // Check for the second part of the encryption command
    //
    uint8_t pui8EncryptionComplete[] = {0x04, 0x3E, 0x42, 0x08, 0x00};

    ui32Status = check_response(pui8EncryptionComplete, 5);

    if (ui32Status != BLE_POWER_SUCCESS)
    {
        am_util_stdio_printf("Encryption failure.\n");
        return ui32Status;
    }

    return BLE_POWER_SUCCESS;
}

//*****************************************************************************
//
// Print an HCI packet.
//
//*****************************************************************************
static void
print_packet(const uint8_t *pui8Packet, uint32_t ui32Length)
{
    uint32_t i;

    for (i = 0; i < ui32Length; i++)
    {
        am_util_stdio_printf("%02X ", pui8Packet[i]);

        if (i % 8 == 7)
        {
            am_util_stdio_printf("\n    ");
        }
    }

    am_util_stdio_printf("\n");

    if (ui32Length % 8)
    {
        am_util_stdio_printf("\n");
    }
}

//*****************************************************************************
//
// End Doxygen group.
//! @}
//
//*****************************************************************************
