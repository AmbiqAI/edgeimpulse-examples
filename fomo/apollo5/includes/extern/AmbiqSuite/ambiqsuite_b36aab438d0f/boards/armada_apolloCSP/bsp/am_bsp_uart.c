//*****************************************************************************
//
//! @file am_bsp_uart.c
//!
//! @brief Functions for configuring the UART for the board.
//
//*****************************************************************************

//*****************************************************************************
//
// ${copyright}
//
// This is part of revision ${version} of the AmbiqSuite Development Package.
//
//*****************************************************************************

#include "am_bsp.h"

//*****************************************************************************
//
// UART configuration.
//
//*****************************************************************************
am_hal_uart_config_t g_sUart =
{
    // 112000 Baudrate
    112000,

    // 8 data bits.
    AM_HAL_UART_DATA_BITS_8,

    // Use one stop bit.
    false,

    // No parity.
    AM_HAL_UART_PARITY_NONE,

    // No flow control.
    AM_HAL_UART_FLOW_CTRL_NONE
};

//*****************************************************************************
//
//! @brief Configures the UART for the board.
//!
//! This function configures the UART for the board.
//!
//! @return 0 for success, nonzero for error.
//
//*****************************************************************************
int
am_bsp_uart_init(void)
{
    //
    // Enable the clock to the UART.
    //
    am_hal_uart_clock_enable(AM_BSP_UART_PRINT_INST);

    //
    // Make sure the uart module is disabled to allow for BAUDRATE
    // programmation:
    //
    am_hal_uart_disable(AM_BSP_UART_PRINT_INST);

    //
    // Configure the UART.
    //
    am_hal_uart_config(AM_BSP_UART_PRINT_INST, &g_sUart);

    //
    // Enable the UART.
    //
    am_hal_uart_enable(AM_BSP_UART_PRINT_INST);

    //
    // Clear all uart interrupt flags:
    // After nUARTRST it is possible to get a modem ctrl delta interrupt
    // depending on initial state of modem lines.
    // 10 interrupt flags can be cleared.
    //
    am_hal_uart_int_clear(AM_BSP_UART_PRINT_INST, 0x000003ff);

    //
    // return success
    //
    return 0;
}
