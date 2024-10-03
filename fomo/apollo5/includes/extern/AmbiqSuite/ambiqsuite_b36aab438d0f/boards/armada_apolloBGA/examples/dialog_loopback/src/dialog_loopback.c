//*****************************************************************************
//
//! @file dialog_loopback.c
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

#ifdef USE_DA14581
#include "dialog_loopback_image_581.h"
#else
#include "dialog_loopback_image.h"
#endif


//*****************************************************************************
//
// Global Variables
//
//*****************************************************************************
#define NUMBER2SEND          (8)
#define ENOUGH_IS_ENOUGH (1024*1024)


//*****************************************************************************
//
// Global Variables
//
//*****************************************************************************
uint32_t g_SentBytes   = 0;
uint32_t g_EnoughBytes = 0;
uint32_t g_ErrorFlag   = 0;

//*****************************************************************************
//
// Configure the pins for using the UART pins on the PMOD instead of the pins
// on the FTDI mini-module.
//
// This function assumes that am_bsp_init() has already been run, so it only
// changes a small number of pins.
//
//*****************************************************************************
void
pmod_uart_configure(void)
{
    //
    // Switch the UART from the FTDI port to the PMOD port.
    //
    am_hal_gpio_pin_config(22, AM_HAL_PIN_22_UARTTX);
    am_hal_gpio_pin_config(23, AM_HAL_PIN_23_UARTRX);
    am_hal_gpio_pin_config(35, AM_HAL_GPIO_INPUT);
    am_hal_gpio_pin_config(36, AM_HAL_GPIO_INPUT);
}

//*****************************************************************************
//
// Configure the pins for using the UART pins on the PMOD instead of the pins
// on the FTDI mini-module.
//
// This function assumes that am_bsp_init() has already been run, so it only
// changes a small number of pins.
//
//*****************************************************************************
void
ftdi_uart_configure(void)
{
    //
    // Switch the UART from the PMOD port to the FTDI port.
    //
    am_hal_gpio_pin_config(22, AM_HAL_GPIO_INPUT);
    am_hal_gpio_pin_config(23, AM_HAL_GPIO_INPUT);
    am_hal_gpio_pin_config(35, AM_HAL_PIN_35_UARTTX);
    am_hal_gpio_pin_config(36, AM_HAL_PIN_36_UARTRX);
}

//*****************************************************************************
//
// Main function.
//
//*****************************************************************************
uint32_t dialog_time_delay;
int
main(void)
{
    volatile uint32_t ui32Status;
    uint8_t           ui8TestChar;
    uint8_t           ui8ExpectChar;
    uint8_t           ui8TempChar;
    uint32_t          ui32Index;

    //
    // Initialize board-essential peripherals.
    //
    am_bsp_init();

    //
    // Enable the ITM.
    //
    am_hal_itm_enable();

    //
    // Initialize the printf interface for ITM/SWO output.
    //
    am_util_stdio_printf_init((am_util_stdio_print_char_t) am_bsp_itm_string_print);

    //
    // Clear the terminal.
    //
    am_util_stdio_terminal_clear();

    //
    // Print the banner.
    //
    am_util_stdio_printf("Dialog Radio Loopback Test\r\n");


    //
    // Route UART traffic through the PMOD instead of the FTDI mini-module.
    //
    pmod_uart_configure();

    //
    // Enable UART FIFO operation.
    //
    am_hal_uart_fifo_config(AM_BSP_UART_BTLE_INST,
                            AM_HAL_UART_TX_FIFO_1_2 | AM_HAL_UART_RX_FIFO_1_2);

    //
    // Force RTS low
    //
    am_hal_gpio_out_bit_clear(AM_BSP_GPIO_BLE_UART_RTS);

    //
    // Assert and Deassert RESET to the Dialog device.
    //
    am_hal_gpio_out_bit_set(AM_BSP_GPIO_BLE_RESET);
    for ( ui32Index = 0; ui32Index < 10000; ui32Index++ )
    {
        dialog_time_delay = ui32Index;
    }
    am_hal_gpio_out_bit_clear(AM_BSP_GPIO_BLE_RESET);

#ifdef USE_DA14581
    //
    // Notify user that the appropriate Dialog download succeeded
    //
    am_util_stdio_printf("Downloaded the Loopback binary to the Dialog DA14581 Radio\r\n");

    //
    // Transmit the Dialog firmware image across the PMOD UART port.
    //
    am_devices_da14580_uart_boot(g_pui8BinaryArray_581,
                                 BINARY_ARRAY_LENGTH_581,
                                 AM_BSP_UART_BTLE_INST);
#else
    //
    // Notify user that the appropriate Dialog download succeeded
    //
    am_util_stdio_printf("Downloaded the Loopback binary to the Dialog DA14580 Radio\r\n");

    am_devices_da14580_uart_boot(g_pui8LoopbackImage,
                                 LOOPBACK_IMAGE_LENGTH,
                                 AM_BSP_UART_BTLE_INST);
#endif

    //
    // Clear any UART errors that may have come up in the reboot process.
    //
    ui32Status = AM_REG(UART, IES);
    AM_REG(UART, IES) = ui32Status;

    //
    // Clean out the pesky zero byte in the FIFO
    //
    am_hal_uart_char_receive_polled(AM_BSP_UART_BTLE_INST, (char*)&ui8TempChar);

    //
    // start sending at blank
    //
    ui8TestChar   = ' ';
    ui8ExpectChar = ' ';

    //
    // Now send some characters to the Radio
    //
    am_util_stdio_printf("Sending %d bytes\r\n", NUMBER2SEND);
    for ( ui32Index = 0; ui32Index < NUMBER2SEND; ui32Index++ )
    {
        am_hal_uart_char_transmit_polled(AM_BSP_UART_BTLE_INST, ui8TestChar);
        ui8TestChar++;
        g_SentBytes++;
    }

    //
    // Loop forever.
    //
    while(g_EnoughBytes < ENOUGH_IS_ENOUGH)
    {
        am_util_stdio_printf("Sent %d Received %d\r\n", g_SentBytes, g_EnoughBytes);

        //
        // wait for the RX side to catch up
        //
        while(g_SentBytes != g_EnoughBytes)
        {
          am_hal_uart_char_receive_polled(AM_BSP_UART_BTLE_INST, (char*)&ui8TempChar);
          if ( ui8TempChar != ui8ExpectChar )
          {
              am_util_stdio_printf("ERROR: Received 0x%x Expected 0x%x\r\n",
                                   ui8TempChar, ui8ExpectChar);
              g_ErrorFlag++;
          }
          ui8ExpectChar++;
          g_EnoughBytes++;
        }

        //
        // Now send some more characters to the Radio
        //
        am_util_stdio_printf("Sending %d bytes\r\n", NUMBER2SEND);
        for ( ui32Index = 0; ui32Index < NUMBER2SEND; ui32Index++ )
        {
          am_hal_uart_char_transmit_polled(AM_BSP_UART_BTLE_INST, ui8TestChar);
          ui8TestChar++;
          g_SentBytes++;
        }
    }

    //
    // put it back
    //
    ftdi_uart_configure();

    //
    //  check for errors and print error message if needed
    //
    if ( g_ErrorFlag )
    {
        am_util_stdio_printf("ERROR ERROR ERROR %d\r\n", g_ErrorFlag);
    }

    return g_ErrorFlag;
}


