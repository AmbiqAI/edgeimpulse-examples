//*****************************************************************************
//
//! @file am_devices_da14580.c
//!
//! @brief Support functions for the Dialog Semiconductor DA14580 BTLE radio.
//!
//! @addtogroup da14580 DA14580 BTLE Radio Device Driver
//! @ingroup devices
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
#include "am_bsp.h"
#include "am_devices_da14580.h"

//*****************************************************************************
//
//! @name MacroDefinitions
//! @brief Macro definitions
//! @{
//
//*****************************************************************************
#define DIALOG_BOOT_STX                     0x02
#define DIALOG_BOOT_SOH                     0x01
#define DIALOG_BOOT_ACK                     0x06
#define DIALOG_BOOT_NACK                    0x15

//! @}   MacroDefinitions

//*****************************************************************************
//
// Runs a UART based boot sequence for a Dialog radio device.
//
//*****************************************************************************
bool
am_devices_da14580_uart_boot(const uint8_t *pui8BinData, uint32_t ui32NumBytes,
                             uint32_t ui32UartModule)
{
    uint32_t ui32Index;
    uint8_t ui8CRCValue;
    char ui8RxChar;

    //
    // Poll the RX lines until we get some indication that the dialog radio is
    // present and ready to receive data.
    //
    do
    {
        am_hal_uart_char_receive_polled(ui32UartModule, &ui8RxChar);
    }
    while ( ui8RxChar != DIALOG_BOOT_STX );

    //
    // Send the Start-of-Header signal and the length of the data download.
    //
    am_hal_uart_char_transmit_polled(ui32UartModule, DIALOG_BOOT_SOH);
    am_hal_uart_char_transmit_polled(ui32UartModule, ui32NumBytes & 0xFF);
    am_hal_uart_char_transmit_polled(ui32UartModule, (ui32NumBytes & 0xFF00) >> 8);

    //
    // Poll for the 'ACK' from the dialog device that signifies that the header
    // was recieved correctly.
    //
    do
    {
        am_hal_uart_char_receive_polled(ui32UartModule, &ui8RxChar);
    }
    while ( ui8RxChar != DIALOG_BOOT_ACK );

    //
    // Initialize the CRC value to zero.
    //
    ui8CRCValue = 0;

    //
    // Send the binary image over to the dialog device one byte at a time,
    // keeping track of the CRC as we go.
    //
    for ( ui32Index = 0; ui32Index < ui32NumBytes; ui32Index++ )
    {
        ui8CRCValue ^= pui8BinData[ui32Index];

        am_hal_uart_char_transmit_polled(ui32UartModule, pui8BinData[ui32Index]);
    }

    //
    // The Dialog device should respond back with a CRC value at the end of the
    // programming cycle. We should check here to make sure that they got the
    // same CRC result that we did. If it doesn't match, return with an error.
    //
    am_hal_uart_char_receive_polled(ui32UartModule, &ui8RxChar);

    if ( ui8RxChar != ui8CRCValue )
    {
        return 1;
    }

    //
    // If all is well, send the final 'ACK' to tell the dialog device that its
    // new image is correct. After this point, the dialog device should start
    // running the downloaded firmware.
    //
    am_hal_uart_char_transmit_polled(ui32UartModule, DIALOG_BOOT_ACK);

    //
    // Wait until the FIFO is actually empty and the UART is no-longer busy.
    //
    while ( !AM_BFR(UART, FR, TXFE) || AM_BFR(UART, FR, BUSY) );

    return 0;
}

//*****************************************************************************
//
// End Doxygen group.
//! @}
//
//*****************************************************************************

