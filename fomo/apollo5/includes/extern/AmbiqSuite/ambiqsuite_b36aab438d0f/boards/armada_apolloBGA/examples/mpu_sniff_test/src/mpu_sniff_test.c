//*****************************************************************************
//
//! @file mpu_sniff_test.c
//!
//! @brief Checks to make sure that the MPU exists and is functioning.
//!
//! Runs a few quick tests to make sure that the MPU is alive.
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
#include "am_util.h"

#include "mpu.h"
#include "regions.h"

//*****************************************************************************
//
// Macro definitions
//
//*****************************************************************************

//*****************************************************************************
//
// Global Variables
//
//*****************************************************************************

//*****************************************************************************
//
//! Clears the terminal screen.
//
//*****************************************************************************
void
util_clear_terminal_screen()
{
    //
    // Escape codes to clear a terminal screen and put the cursor in the top
    // left corner.
    //
    am_hal_uart_string_transmit_polled(AM_BSP_UART_BTLE_INST, "\033[2J\033[H");
    return;
}

//*****************************************************************************
//
//! Prints a hex number.
//
//*****************************************************************************
void
util_print_hex(uint32_t ulHex)
{
    uint32_t ulTemp = ulHex;
    char     cHex;
    int      i;

    for ( i = 0; i < 8; i++ )
    {
        cHex = (char)((ulTemp >> 28) & 0x0000000f);
        ulTemp <<= 4;

        if ( cHex > 9 )
        {
            cHex = (cHex-10) + 0x41;
        }
        else
        {
            cHex =  cHex    + 0x30;
        }

        am_hal_uart_char_transmit_polled(AM_BSP_UART_BTLE_INST, cHex);
    }
}

//*****************************************************************************
//
// Fault handler for the MPU
//
//*****************************************************************************
void
MPUFaultHandler(void)
{
    am_hal_uart_string_transmit_polled(AM_BSP_UART_BTLE_INST,
                            "MPU Enabled, UART NOT accessible. GOOD\r\n");
    while (1);
}

//*****************************************************************************
//
// Main
//
//*****************************************************************************
int
main(void)
{
    uint32_t ui32_TYPE;
    //
    // Initialize all of the peripherals specified by the BSP.
    //
    am_bsp_init();

    //
    // Enable the ITM.
    //
    am_hal_itm_enable();

    //
    // Initialize the printf interface for polled uart.
    //
    am_util_stdio_printf_init((am_util_stdio_print_char_t) am_bsp_itm_string_print);

    //
    // Clear the terminal.
    //
    util_clear_terminal_screen();

    //
    // print the value of the MPU TYPE register and check for presence
    //
    ui32_TYPE = mpu_type_get();
    if ( ui32_TYPE == 0x00000800 )
    {
        am_util_stdio_printf("MPU_TYPE register says MPU is present %u\r\n", ui32_TYPE);
    }
    else
    {
        am_util_stdio_printf("ERROR: MPU_TYPE register says MPU NOT PRESENT\r\n");
        return -1;
    }

    //
    // Set the MPU regions.
    //
    mpu_region_configure(&sFlash, true);
    mpu_region_configure(&sSRAM, true);
    mpu_region_configure(&sPeripherals, true);

    //
    // Enable the MPU
    //
    mpu_global_configure(true, true, false);

    //
    // Can we print a message?  Really can we still write to the UART?
    //
    am_hal_uart_string_transmit_polled(AM_BSP_UART_BTLE_INST,
                "ERROR: MPU Enabled, UART still accessible (shouldn't be).");
    am_util_stdio_printf("ERROR: MPU Enabled, UART still accessible (shouldn't be).");

    //
    // Loop forever
    //
    while (1)
    {
    }
}
