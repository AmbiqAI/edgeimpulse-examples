//*****************************************************************************
//
//! @file apollo4_board.c
//!
//! @brief contains the common board init function
//!
//! @addtogroup usb_examples USB Examples
//!
//! @defgroup tinyusb_cdc_hfrc2 TinyUSB CDC HFRC2 Example
//! @ingroup usb_examples
//! @{
//!
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
#include "am_hal_global.h"

//*****************************************************************************
//
// Main
//
//*****************************************************************************
void
board_init(void)
{
    am_util_id_t sIdDevice;

    //
    // Global interrupt enable
    //
    am_hal_interrupt_master_enable();

    //
    // Set the default cache configuration
    //
    am_hal_cachectrl_config(&am_hal_cachectrl_defaults);
    am_hal_cachectrl_enable();

    //
    // Configure the board for low power operation.
    //
    am_bsp_low_power_init();

    //
    // Initialize the printf interface for ITM output
    //
    am_bsp_debug_printf_enable();

    //
    // Print the banner.
    //
    am_util_stdio_terminal_clear();
    am_util_stdio_printf("USB CDC HS Example\n\n");

    //
    // Print the device info.
    //
    am_util_id_device(&sIdDevice);
    am_util_stdio_printf("Vendor Name: %s\n", sIdDevice.pui8VendorName);
    am_util_stdio_printf("Device type: %s\n\n", sIdDevice.pui8DeviceName);
}

uint32_t board_millis(void)
{
    return 0;
}

void board_led_write(bool bLedState)
{
}

//*****************************************************************************
//
// End Doxygen group.
//! @}
//
//*****************************************************************************

