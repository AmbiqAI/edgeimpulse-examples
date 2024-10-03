//*****************************************************************************
//
//! @file am_devices_da14581.c
//!
//! @brief Support functions for the Dialog Semiconductor DA14581 BTLE radio.
//!
//! @addtogroup da14581 DA14581 BTLE Radio Device Driver
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
#include "am_devices_da14581.h"

//*****************************************************************************
//
//! @name MacroDefinitions
// @addtogroup da14581
//! @brief Macro Definitions
//! @{
//
//*****************************************************************************
#define DIALOG_BOOT_STX                     0x02
#define DIALOG_BOOT_SOH                     0x01
#define DIALOG_BOOT_ACK                     0x06
#define DIALOG_BOOT_NACK                    0x15

//! @} MacroDefinitions

//*****************************************************************************
//
//! BLE MAC address for the Dialog radio.
//
//*****************************************************************************
static uint8_t g_BLEMacAddress[6] =
{0x01, 0x00, 0x00, 0xCA, 0xEA, 0x80};

//*****************************************************************************
//
//! Location in the Diaog binary where the MAC address is stored.
//
//*****************************************************************************
static const uint32_t g_ui32MacIndex = 1000;

//*****************************************************************************
//
// Sets the MAC address to send to the dialog radio on the next boot up.
//
//*****************************************************************************
void
am_devices_da14581_mac_set(const uint8_t *pui8MacAddress)
{
    uint32_t i;

    //
    // Copy the 6-byte MAC address into our global variable.
    //
    for ( i = 0; i < 6; i++ )
    {
        g_BLEMacAddress[i] = *pui8MacAddress++;
    }
}

//*****************************************************************************
//
//  Runs a UART based boot sequence for a Dialog radio device.
//
//*****************************************************************************
bool
am_devices_da14581_uart_boot(const uint8_t *pui8BinData, uint32_t ui32NumBytes,
                             uint32_t ui32UartModule)
{
    uint32_t ui32Index;
    uint8_t ui8CRCValue;
    uint8_t ui8TxData;
    char ui8RxChar;

    //
    // Poll the RX lines until we get some indication that the dialog radio is
    // present and ready to receive data.
    //
    do
    {
        am_hal_uart_char_receive_polled(ui32UartModule, &ui8RxChar);
    }
    while (ui8RxChar != DIALOG_BOOT_STX);

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
    while (ui8RxChar != DIALOG_BOOT_ACK);

    //
    // Initialize the CRC value to zero.
    //
    ui8CRCValue = 0;

    //
    // Send the binary image over to the dialog device one byte at a time,
    // keeping track of the CRC as we go.
    //
    for (ui32Index = 0; ui32Index < ui32NumBytes; ui32Index++)
    {
        if ((ui32Index >= g_ui32MacIndex) && (ui32Index < g_ui32MacIndex + 6))
        {
            ui8TxData = g_BLEMacAddress[ui32Index - g_ui32MacIndex];
        }
        else
        {
            ui8TxData = pui8BinData[ui32Index];
        }

        ui8CRCValue ^= ui8TxData;
        am_hal_uart_char_transmit_polled(ui32UartModule, ui8TxData);
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
    while (!AM_BFR(UART, FR, TXFE) || AM_BFR(UART, FR, BUSY));

    return 0;
}

// #### INTERNAL BEGIN ####
//*****************************************************************************
//
// Handles Initialization of the DA14581 after SPI HCI Download
//
//*****************************************************************************
bool
am_devices_da14581_init(am_devices_da14581_t *psDevice)
{
    uint32_t ui32Buffer[1];
    uint8_t *ui8Command  = (uint8_t *)ui32Buffer;
    uint8_t *ui8Response = (uint8_t *)ui32Buffer;

    if ( psDevice->ui32Mode )
    { // then SPI mode
        //
        // check DREADY state, must be asserted here or failure
        //
#if AM_APOLLO3_GPIO
        uint32_t ui32GpioRd;
        am_hal_gpio_state_read(psDevice->ui32DREADY,
                               AM_HAL_GPIO_INPUT_READ,
                               &ui32GpioRd);
        if ( !ui32GpioRd )
        {
            return false;
        }
#else // AM_APOLLO3_GPIO
        if ( !am_hal_gpio_input_bit_read(psDevice->ui32DREADY) )
        {
            return false;
        }
#endif // AM_APOLLO3_GPIO

        //
        //  enable the IOM
        //
        am_hal_iom_enable(psDevice->ui32IOMModule);

        //
        // assert chip select via GPIO
        //
#if AM_APOLLO3_GPIO
        am_hal_gpio_state_write(psDevice->ui32IOMChipSelect, AM_HAL_GPIO_OUTPUT_CLEAR);
#else // AM_APOLLO3_GPIO
        am_hal_gpio_out_bit_clear(psDevice->ui32IOMChipSelect);
#endif // AM_APOLLO3_GPIO

        //
        // Use the IOM to read the one byte SPI XPORT direction control
        //
        ui8Command[0] = 0x08;
        am_hal_iom_spi_write(psDevice->ui32IOMModule,
                             0, // arbitrary chip select, we are using GPIO
                             ui32Buffer, 1, AM_HAL_IOM_RAW | AM_HAL_IOM_CS_LOW);
        am_hal_iom_spi_read(psDevice->ui32IOMModule,
                            0, // arbitrary chip select, we are using GPIO
                            ui32Buffer, 1, AM_HAL_IOM_RAW);

        //
        // remove chip select via GPIO
        //
#if AM_APOLLO3_GPIO
        am_hal_gpio_state_write(psDevice->ui32IOMChipSelect, AM_HAL_GPIO_OUTPUT_SET);
#else // AM_APOLLO3_GPIO
        am_hal_gpio_out_bit_set(psDevice->ui32IOMChipSelect);
#endif // AM_APOLLO3_GPIO

        //
        // We must have received a 0x6 token back from the DA14581
        //
        if ( ui8Response[0] != AM_DEVICES_DA14581_SPI_XPORT_CTS )
        {
            return false;
        }

    }
    else // UART mode
    {
        //
        // NOT CURRENTLY SUPPORTED HERE
        //
        return false;
    }

    return true;
}

//*****************************************************************************
//
//  Sends Command Packets to the DA14581 on the SPI Xport
//
//*****************************************************************************
void
am_devices_da14581_spi_send(am_devices_da14581_t *psDevice, uint32_t options,
                            uint8_t type, uint8_t *pData, uint16_t len)
{
    int32_t i32I;
    uint32_t ui32IOMBuffer[16];
    uint8_t  *Command = (uint8_t *)ui32IOMBuffer;

    //
    //  for now we can only handle 64 byte max size xfers
    //
    if ( len > 63 )
    {
        while (1);
    }

    //
    // Copy the values into our command buffer
    //
    Command[0] = type;
    for (i32I = 1; i32I < (len + 1); i32I++)
    {
        Command[i32I] = *pData++;
    }

    //
    // assert chip select via GPIO
    //
#if AM_APOLLO3_GPIO
    am_hal_gpio_state_write(psDevice->ui32IOMChipSelect, AM_HAL_GPIO_OUTPUT_CLEAR);
#else // AM_APOLLO3_GPIO
    am_hal_gpio_out_bit_clear(psDevice->ui32IOMChipSelect);
#endif // AM_APOLLO3_GPIO

    //
    // Use the IOM to send the HCI command or data
    //
    am_hal_iom_spi_write(psDevice->ui32IOMModule,
                             0, // arbitrary chip select, we are using GPIO
                             ui32IOMBuffer, len + 1,
                             options | AM_HAL_IOM_RAW );

    //
    // Keep chip select low if requested
    //
    if ( !(options & AM_HAL_IOM_CS_LOW) )
    {
        //
        // remove chip select via GPIO
        //
#if AM_APOLLO3_GPIO
        am_hal_gpio_state_write(psDevice->ui32IOMChipSelect, AM_HAL_GPIO_OUTPUT_SET);
#else // AM_APOLLO3_GPIO
        am_hal_gpio_out_bit_set(psDevice->ui32IOMChipSelect);
#endif // AM_APOLLO3_GPIO
    }

}

//*****************************************************************************
//
// Recieve Bytes from the DA14581 on the SPI Xport
//
//*****************************************************************************
void
am_devices_da14581_spi_receive_bytes(am_devices_da14581_t *psDevice,
                                     uint32_t options,
                                     uint8_t *pData, uint16_t len)
{
    int32_t i32I;
    uint32_t ui32IOMBuffer[16];
    uint8_t  *Response = (uint8_t *)ui32IOMBuffer;

    //
    //  for now we can only handle 64 byte max size xfers
    //
    if ( len > 64 )
    {
        while (1);
    }

    //
    // assert chip select via GPIO
    //
#if AM_APOLLO3_GPIO
    am_hal_gpio_state_write(psDevice->ui32IOMChipSelect, AM_HAL_GPIO_OUTPUT_CLEAR);
#else // AM_APOLLO3_GPIO
    am_hal_gpio_out_bit_clear(psDevice->ui32IOMChipSelect);
#endif // AM_APOLLO3_GPIO

    //
    // Use the IOM to receive bytes from the SPI interface
    //
    am_hal_iom_spi_read(psDevice->ui32IOMModule,
                        0, // arbitrary chip select, we are using GPIO
                        ui32IOMBuffer, len, (options | AM_HAL_IOM_RAW));

    //
    // remove chip select via GPIO
    //
#if AM_APOLLO3_GPIO
    am_hal_gpio_state_write(psDevice->ui32IOMChipSelect, AM_HAL_GPIO_OUTPUT_SET);
#else // AM_APOLLO3_GPIO
    am_hal_gpio_out_bit_set(psDevice->ui32IOMChipSelect);
#endif // AM_APOLLO3_GPIO

    //
    // Copy the values to caller's response buffer
    //
    for (i32I = 0; i32I < len; i32I++)
    {
        *pData++ = Response[i32I];
    }
}
// #### INTERNAL END ####

//*****************************************************************************
//
// End Doxygen group.
//! @}
//
//*****************************************************************************

