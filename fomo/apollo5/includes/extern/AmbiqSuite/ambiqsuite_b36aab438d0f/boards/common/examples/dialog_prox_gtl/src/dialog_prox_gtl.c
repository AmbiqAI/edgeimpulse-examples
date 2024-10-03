//*****************************************************************************
//
//! @file dialog_prox_gtl.c
//!
//! @brief Example to load a binary into a Dialog device.
//!
//! This example loads a binary image into an attached Dialog radio through the
//! Dialog HCI protocol over SPI.
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
#include "am_mcu_apollo.h"
#include "am_bsp.h"
#include "am_util.h"
#include "am_devices.h"

#include "binary_array.h"

//*****************************************************************************
//
// Global connection parameters.
//
//*****************************************************************************
bool g_bConnected;
uint16_t g_ui16ConnectionHandle;

//*****************************************************************************
//
// Global message buffers.
//
//*****************************************************************************
am_devices_da14580_gtl_msg_t sRxMessage;
uint8_t pui8RxParams[100];

am_devices_da14580_gtl_msg_t sTxMessage;
uint8_t pui8TxParams[100];
// FIXME: Move to arguments so user can select.
uint32_t g_ui32Module = 0;

//*****************************************************************************
//
// Global BLE GAP configuration structure.
//
//*****************************************************************************
am_devices_da14580_gapm_start_advertise_cmd_t sAdvertiseCommand =
{
    .ui8Code = 14,
    .ui8AddrSrc = 0,
    .ui16IntvMin = 1100,
    .ui16IntvMax = 1100,
    .ui8ChannelMap = 7,
    .ui8Mode = 1,

    .ui8AdvDataLen = 20,
    .pui8AdvData = "\x07\x03\x03\x18\x02\x18\x04\x18" "\x0B\x09HELLO AM!!",

    .ui8ScanRspDataLen = 10,
    .pui8ScanRspData = "\x09\xFF\x00\x60\x52\x57\x2D\x42\x4C\x45",

    .ui16State = 0
};

//*****************************************************************************
//
// Check the UART for incoming GTL messages from the DA14580
//
// If a message was found, write it to the given message structure, and return
// true.
//
//*****************************************************************************
bool
check_messages(am_devices_da14580_gtl_msg_t *psMessage)
{
    uint32_t ui32Index;
    char *pcMessagePtr;
    char cReceived;

    //
    // If the FIFO is empty, just return.
    //
    if ((AM_REG(UART, FR) & AM_REG_UART_FR_RXFE_M))
    {
        return false;
    }

    //
    // Read the first byte from the FIFO.
    //
    am_hal_uart_char_receive_polled(g_ui32Module, &cReceived);

    //
    // If the first byte wasn't a 'start of frame', just return now.
    //
    if (cReceived != 0x5)
    {
        return false;
    }

    //
    // Set the message pointer to the beginning of the GTL message structure.
    //
    pcMessagePtr = (char *) psMessage;

    //
    // The message structure is packed, so we should be able to read in data
    // from the UART on a per-byte basis like this. Once we've read in the 8
    // bytes that comprise the header, we will check the parameter length, and
    // read the rest of the data in a more normal way.
    //
    for (ui32Index = 0; ui32Index < 8; ui32Index++)
    {
        am_hal_uart_char_receive_polled(g_ui32Module, pcMessagePtr + ui32Index);
    }

    //
    // Check the parameter length, and read any remaining parameters into the
    // params array.
    //
    for (ui32Index = 0; ui32Index < psMessage->ui16ParamLength; ui32Index++)
    {
        am_hal_uart_char_receive_polled(g_ui32Module, (char *)(psMessage->pui8Params) +
                                        ui32Index);
    }

    return true;
}

//*****************************************************************************
//
// Sends a GTL packet over the UART.
//
//*****************************************************************************
void
send_gtl(am_devices_da14580_gtl_msg_t *psMessage)
{
    uint32_t ui32Index;
    char *pcMessagePtr;

    //
    // Set the send pointer equal to the beginning of the message header.
    //
    pcMessagePtr = (char *) psMessage;

    //
    // Transmit the "start of frame" character.
    //
    am_hal_uart_char_transmit_polled(g_ui32Module, 0x5);

    //
    // Transmit the first 8 bytes of the GTL packet.
    //
    for (ui32Index = 0; ui32Index < 8; ui32Index++)
    {
        am_hal_uart_char_transmit_polled(g_ui32Module, pcMessagePtr[ui32Index]);
    }

    //
    // Set the send pointer equal to the beginning of the parameter list.
    //
    pcMessagePtr = (char *) (psMessage->pui8Params);

    //
    // Transmit the parameter list.
    //
    for (ui32Index = 0; ui32Index < psMessage->ui16ParamLength; ui32Index++)
    {
        am_hal_uart_char_transmit_polled(g_ui32Module, pcMessagePtr[ui32Index]);
    }
}

//*****************************************************************************
//
// Handler function prototypes.
//
//*****************************************************************************
void device_ready_handler(am_devices_da14580_gtl_msg_t* psRxMessage,
                          am_devices_da14580_gtl_msg_t* psTxMessage);

void prox_database_complete_handler(am_devices_da14580_gtl_msg_t* psRxMessage,
                                    am_devices_da14580_gtl_msg_t* psTxMessage);

void gapm_event_handler(am_devices_da14580_gtl_msg_t* psRxMessage,
                        am_devices_da14580_gtl_msg_t* psTxMessage);

void gapm_device_reset_handler(am_devices_da14580_gtl_msg_t* psRxMessage,
                               am_devices_da14580_gtl_msg_t* psTxMessage);

void gapm_device_config_complete(am_devices_da14580_gtl_msg_t* psRxMessage,
                                 am_devices_da14580_gtl_msg_t* psTxMessage);

void gapc_connection_req_handler(am_devices_da14580_gtl_msg_t* psRxMessage,
                                 am_devices_da14580_gtl_msg_t* psTxMessage);

void gapc_disconnect_event_handler(am_devices_da14580_gtl_msg_t* psRxMessage,
                                   am_devices_da14580_gtl_msg_t* psTxMessage);

void power_level_reading_handler(am_devices_da14580_gtl_msg_t* psRxMessage,
                                 am_devices_da14580_gtl_msg_t* psTxMessage);

void empty_handler(am_devices_da14580_gtl_msg_t* psRxMessage,
                   am_devices_da14580_gtl_msg_t* psTxMessage);


//*****************************************************************************
//
// Structure for attaching function pointers to ID values.
//
//*****************************************************************************
typedef struct
{
    uint32_t ui32ID;
    void (*pfnHandler)(am_devices_da14580_gtl_msg_t* psRxMessage,
                       am_devices_da14580_gtl_msg_t* psTxMessage);
}
gtl_handler_t;

//*****************************************************************************
//
// Structure containing GTL Handler Functions
//
//*****************************************************************************
gtl_handler_t psGTLHandlers[] =
{
    {AM_DEVICES_DA14580_LLC_RD_PW_LVL_EVT, power_level_reading_handler},
    {AM_DEVICES_DA14580_GAPM_CMP_EVT, gapm_event_handler},
    {AM_DEVICES_DA14580_GAPM_STACK_READY, device_ready_handler},
    {AM_DEVICES_DA14580_PROXR_CREATE_DB_CFM, prox_database_complete_handler},
    {AM_DEVICES_DA14580_PROXR_DISABLE_IND, empty_handler},
    {AM_DEVICES_DA14580_PROXR_ERROR_IND, empty_handler},
    {AM_DEVICES_DA14580_GAPC_CONNECTION_REQ, gapc_connection_req_handler},
    {AM_DEVICES_DA14580_GAPC_DISCONNECT_EVT, gapc_disconnect_event_handler},
};

#define NUM_GTL_HANDLERS                                                      \
    (sizeof(psGTLHandlers) / sizeof(gtl_handler_t))

//*****************************************************************************
//
// Structure containing GTL Handler Functions
//
//*****************************************************************************
gtl_handler_t psGAPMHandlers[] =
{
    {0x1, gapm_device_reset_handler},
    {0x3, gapm_device_config_complete},
};

#define NUM_GAPM_HANDLERS                                                     \
    (sizeof(psGAPMHandlers) / sizeof(gtl_handler_t))

//*****************************************************************************
//
// Function that calls other functions (or handlers) based on an ID
//
//*****************************************************************************
bool
call_gtl_handler(uint32_t ui32ID,
                 am_devices_da14580_gtl_msg_t* psRxMessage,
                 am_devices_da14580_gtl_msg_t* psTxMessage,
                 gtl_handler_t *psHandlerList,
                 uint32_t ui32NumHandlers)
{
    uint32_t ui32Index;
    gtl_handler_t *psHandler;

    //
    // Loop over the list of handlers
    //
    for (ui32Index = 0; ui32Index < ui32NumHandlers; ui32Index++)
    {
        psHandler = &psHandlerList[ui32Index];

        //
        // Check each entry to see if one of them matches the given ID value.
        //
        if (psHandler->ui32ID == ui32ID)
        {
            //
            // If we find a match, call the handler, and return.
            //
            psHandler->pfnHandler(psRxMessage, psTxMessage);
            return true;
        }
    }

    return false;
}

//*****************************************************************************
//
// Handles events from the "GAPM" task by rerouting them to the correct message
// handler.
//
//*****************************************************************************
void
gapm_event_handler(am_devices_da14580_gtl_msg_t* psRxMessage,
                   am_devices_da14580_gtl_msg_t* psTxMessage)
{
    uint32_t ui32ID;

    //
    // This message type needs a parameter. Halt if the parameter is not there.
    //
    if (psRxMessage->ui16ParamLength == 0)
    {
        while (1);
    }

    //
    // GAPM events are all grouped together under the same top-level ID, so
    // we'll need to get the actual message ID from the parameter list.
    //
    ui32ID = psRxMessage->pui8Params[0];

    //
    // Call the handler associated with this GAPM ID.
    //
    call_gtl_handler(ui32ID, psRxMessage, psTxMessage, psGAPMHandlers,
                     NUM_GAPM_HANDLERS);
}

//*****************************************************************************
//
// Handles the "device-ready" GTL message.
//
//*****************************************************************************
void
device_ready_handler(am_devices_da14580_gtl_msg_t* psRxMessage,
                     am_devices_da14580_gtl_msg_t* psTxMessage)
{
    //
    // Send a RESET command to the GAPM controller. This should generate a
    // "GAPM reset command complete" event.
    //
    psTxMessage->ui16ID = 0x3402;
    psTxMessage->ui16Dest = 0x0D;
    psTxMessage->ui16Source = 0x3F;
    psTxMessage->ui16ParamLength = 0x1;
    psTxMessage->pui8Params[0] = 0x1;

    send_gtl(psTxMessage);
}

//*****************************************************************************
//
// Handles the "gapm device reset complete" GTL message.
//
//*****************************************************************************
void
gapm_device_reset_handler(am_devices_da14580_gtl_msg_t* psRxMessage,
                          am_devices_da14580_gtl_msg_t* psTxMessage)
{
    //
    // Send a PROXR_CREATE_DB_REQ
    //
    psTxMessage->ui16ID = 0x4003;
    psTxMessage->ui16Dest = 0x10;
    psTxMessage->ui16Source = 0x3F;
    psTxMessage->ui16ParamLength = 0x1;

    //
    // PROXR_IAS_TXPS_SUP
    //
    psTxMessage->pui8Params[0] = 0x1;

    send_gtl(psTxMessage);
}

//*****************************************************************************
//
// Handles the "gapm device configuration complete" GTL message.
//
//*****************************************************************************
void
gapm_device_config_complete(am_devices_da14580_gtl_msg_t* psRxMessage,
                            am_devices_da14580_gtl_msg_t* psTxMessage)
{
    uint8_t *pui8OriginalParams;

    //
    // Prepare a "Start advertising" command.
    //
    psTxMessage->ui16ID = AM_DEVICES_DA14580_ADVERTISE_CMD;
    psTxMessage->ui16Dest = 0x0D;
    psTxMessage->ui16Source = 0x3F;
    psTxMessage->ui16ParamLength =
        sizeof(am_devices_da14580_gapm_start_advertise_cmd_t);

    //
    // Save the original parameter list location.
    //
    pui8OriginalParams = psTxMessage->pui8Params;

    //
    // Use the Global advertising command as the parameter list for this
    // command.
    //
    psTxMessage->pui8Params = (uint8_t *)(&sAdvertiseCommand);

    //
    // Send the command.
    //
    send_gtl(psTxMessage);

    //
    // Restore the old parameter list.
    //
    psTxMessage->pui8Params = pui8OriginalParams;
}

//*****************************************************************************
//
// Handles the "Proximity Database Creation Complete" message.
//
//*****************************************************************************
void
prox_database_complete_handler(am_devices_da14580_gtl_msg_t* psRxMessage,
                               am_devices_da14580_gtl_msg_t* psTxMessage)
{
    am_devices_da14580_gapm_dev_cfg_t *psDeviceConfig;

    //
    // Prepare a device configuration command.
    //
    psTxMessage->ui16ID = 0x3404;
    psTxMessage->ui16Dest = 0x0D;
    psTxMessage->ui16Source = 0x3F;
    psTxMessage->ui16ParamLength = sizeof(am_devices_da14580_gapm_dev_cfg_t);

    //
    // Alias the parameter list pointer to make it easier to set the necessary
    // fields.
    //
    psDeviceConfig = ((am_devices_da14580_gapm_dev_cfg_t *)
                      (psTxMessage->pui8Params));

    psDeviceConfig->ui8Operation = 0x3;
    psDeviceConfig->ui8Role = 0xA;
    psDeviceConfig->ui16Appearance = 0x0000;
    psDeviceConfig->ui8AppearanceWritePerm = 0;
    psDeviceConfig->ui8NameWritePerm = 0;
    psDeviceConfig->ui16MinConnectInterval = 8;
    psDeviceConfig->ui16MaxConnectInterval = 16;
    psDeviceConfig->ui16ConnectLatency = 0;
    psDeviceConfig->ui16SupervisionTimeout = 100;
    psDeviceConfig->ui8PrivacyFlags = 0;

    send_gtl(psTxMessage);
}

//*****************************************************************************
//
// Handles connection requests.
//
//*****************************************************************************
void
gapc_connection_req_handler(am_devices_da14580_gtl_msg_t* psRxMessage,
                            am_devices_da14580_gtl_msg_t* psTxMessage)
{
    if (!g_bConnected)
    {
        //
        // This event requires parameters. We should break if they are not
        // there.
        //
        if (psRxMessage->ui16ParamLength < 16)
        {
            while (1);
        }

        //
        // Save the connection handle.
        //
        g_ui16ConnectionHandle = (psRxMessage->pui8Params[0] |
                                  (psRxMessage->pui8Params[1] << 8));

        //
        // Send a command to ask for the TX power level.
        //
        psTxMessage->ui16ID = 0x0404;
        psTxMessage->ui16Dest = 0x01;
        psTxMessage->ui16Source = 0x3F;
        psTxMessage->ui16ParamLength = 3;
        psTxMessage->pui8Params[0] = psRxMessage->pui8Params[0];
        psTxMessage->pui8Params[1] = psRxMessage->pui8Params[1];
        psTxMessage->pui8Params[2] = 0;

        send_gtl(psTxMessage);

        //
        // Send a connection confirmation.
        //
        psTxMessage->ui16ID = 0x3802;
        psTxMessage->ui16Dest = (g_ui16ConnectionHandle << 8) | 0xE;
        psTxMessage->ui16Source = 0x3F;
        psTxMessage->ui16ParamLength = 3;
        psTxMessage->pui8Params[0] = psRxMessage->pui8Params[0];
        psTxMessage->pui8Params[1] = psRxMessage->pui8Params[1];
        psTxMessage->pui8Params[2] = 0;

        send_gtl(psTxMessage);
    }
}

//*****************************************************************************
//
// Handler for disconnect events.
//
//*****************************************************************************
void
gapc_disconnect_event_handler(am_devices_da14580_gtl_msg_t* psRxMessage,
                              am_devices_da14580_gtl_msg_t* psTxMessage)
{
    uint16_t ui16DisconnectHandle;

    //
    // Check the connection handle.
    //
    ui16DisconnectHandle = (psRxMessage->pui8Params[0] |
                            (psRxMessage->pui8Params[1] << 8));

    //
    // Make sure this matches the handle of the device we last connected to.
    //
    if (ui16DisconnectHandle == g_ui16ConnectionHandle)
    {
        //
        // We have a match! Change our state back to "unconnected".
        //
        g_bConnected = false;

        //
        // Forward the disconnect message on to the proximity profile.
        //
        psTxMessage->ui16ID = 0x3803;
        psTxMessage->ui16Dest = 0xE;
        psTxMessage->ui16Source = 0x3F;
        psTxMessage->ui16ParamLength = 3;
        psTxMessage->pui8Params[0] = psRxMessage->pui8Params[0];
        psTxMessage->pui8Params[1] = psRxMessage->pui8Params[1];
        psTxMessage->pui8Params[2] = psRxMessage->pui8Params[2];

        send_gtl(psTxMessage);

        //
        // Now we need to configure the device again.
        //
        prox_database_complete_handler(psRxMessage, psTxMessage);
    }

}

//*****************************************************************************
//
// Handler for the "Read TX Power Level" command response.
//
//*****************************************************************************
void
power_level_reading_handler(am_devices_da14580_gtl_msg_t* psRxMessage,
                            am_devices_da14580_gtl_msg_t* psTxMessage)
{
    am_devices_da14580_proxr_enable_req_t* psCommand;

    //
    // Enable the proximity reporter profile
    //
    psTxMessage->ui16ID = AM_DEVICES_DA14580_PROXR_ENABLE_REQ;
    psTxMessage->ui16Dest = 0x10;
    psTxMessage->ui16Source = 0x3F;
    psTxMessage->ui16ParamLength =
        sizeof(am_devices_da14580_proxr_enable_req_t);

    //
    // Alias the parameter list to make it easier to set the necessary parameter fields.
    //
    psCommand = ((am_devices_da14580_proxr_enable_req_t *)
                 (psTxMessage->pui8Params));

    psCommand->ui16ConnectionHandle = g_ui16ConnectionHandle;
    psCommand->i8TXPLevel = psRxMessage->pui8Params[3];
    psCommand->ui8SecurityLevel = 1;
    psCommand->ui8LLSAlertLevel = 2;

    send_gtl(psTxMessage);
}

//*****************************************************************************
//
// Empty handler for events that require no action
//
//*****************************************************************************
void
empty_handler(am_devices_da14580_gtl_msg_t* psRxMessage,
              am_devices_da14580_gtl_msg_t* psTxMessage)
{
    //
    // Do nothing.
    //
}


//*****************************************************************************
//
// Main function.
//
//*****************************************************************************
int
main(void)
{
    volatile uint32_t ui32Status;

    //
    // Set the clock frequency.
    //
    am_hal_clkgen_sysclk_select(AM_HAL_CLKGEN_SYSCLK_MAX);

    //
    // Set the default cache configuration
    //
    am_hal_cachectrl_enable(&am_hal_cachectrl_defaults);

    //
    // Initialize board-essential peripherals.
    //
    //am_bsp_low_power_init();
    am_bsp_init();

    //
    // Enable UART FIFO operation.
    //
    am_hal_uart_fifo_config(g_ui32Module,
        AM_HAL_UART_TX_FIFO_1_2 | AM_HAL_UART_RX_FIFO_1_2);

    //
    // Assert and Deassert RESET to the Dialog device.
    //
    am_hal_gpio_out_bit_set(AM_BSP_GPIO_BLE_RESET);
    am_hal_gpio_out_bit_clear(AM_BSP_GPIO_BLE_RESET);

    //
    // Transmit the Dialog firmware image across the PMOD UART port.
    //
    am_devices_da14580_uart_boot(g_pui8BinaryArray, BINARY_ARRAY_LENGTH, g_ui32Module);

    //
    // Link our message structures to the arrays we've dedicated for parameter
    // data.
    //
    sRxMessage.pui8Params = pui8RxParams;
    sTxMessage.pui8Params = pui8TxParams;

    //
    // Clear any UART errors that may have come up in the reboot process.
    //
    ui32Status = AM_REG(UART, IES);
    AM_REG(UART, IES) = ui32Status;

    //
    // Loop forever.
    //
    while (1)
    {
        //
        // Check the GTL interface for incoming messages.
        //
        if (check_messages(&sRxMessage))
        {
            //
            // If there are any GTL messages waiting, call the appropriate
            // handler.
            //
            if (!call_gtl_handler(sRxMessage.ui16ID, &sRxMessage, &sTxMessage,
                                 psGTLHandlers, NUM_GTL_HANDLERS))
            {
                //
                // If we ever encounter a bad command, halt here for debug.
                //
                while (1);
            }
        }
    }
}
