//*****************************************************************************
//
//! @file am_bsp.c
//!
//! @brief Top level functions for performing board initialization.
//
//*****************************************************************************

//*****************************************************************************
//
// ${copyright}
//
// This is part of revision ${version} of the AmbiqSuite Development Package.
//
//*****************************************************************************

#include "am_mcu_apollo.h"
#include "am_bsp.h"

//*****************************************************************************
//
// Global Variables
//
//*****************************************************************************

//*****************************************************************************
//
//! @brief Initialize the board support package (BSP).
//!
//! This function initializes the board support package (BSP).
//!
//! @return None.
//
//*****************************************************************************
void
am_bsp_init(void)
{
    //
    // Initialize the GPIOs.
    //
    am_bsp_gpio_init();

    //
    // Initialize the UART.
    //
    am_bsp_uart_init();

    //
    //  Initialize the LEDs.
    //
    am_bsp_led_init();

    //
    //  Initialize the I/O Masters.
    //
    am_bsp_iom_init();
}

//*****************************************************************************
//
//! @brief ITM-based string print function.
//!
//! This function is used for printing a string via the ITM.
//!
//! @return None.
//
//*****************************************************************************
void
am_bsp_itm_string_print(char *pcString)
{
    am_hal_itm_print(pcString);
}

//*****************************************************************************
//
//! @brief UART-based string print function.
//!
//! This function is used for printing a string via the UART, which for some
//! MCU devices may be multi-module.
//!
//! @return None.
//
//*****************************************************************************
void
am_bsp_uart_string_print(char *pcString)
{
    am_hal_uart_string_transmit_polled(0, pcString);
}

