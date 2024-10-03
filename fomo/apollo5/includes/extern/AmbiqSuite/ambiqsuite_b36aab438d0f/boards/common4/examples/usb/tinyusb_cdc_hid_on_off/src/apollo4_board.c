//*****************************************************************************
//
//! @file apollo4_board.c
//!
//! @brief contains board init code
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
#include "apollo4_board.h"
#include "am_devices_led.h"

//*****************************************************************************
//
// Insert compiler version at compile time.
//
//*****************************************************************************
#define STRINGIZE_VAL(n)                    STRINGIZE_VAL2(n)
#define STRINGIZE_VAL2(n)                   #n

#ifdef __GNUC__
#define COMPILER_VERSION                    ("GCC " __VERSION__)
#elif defined(__ARMCC_VERSION)
#define COMPILER_VERSION                    ("ARMCC " STRINGIZE_VAL(__ARMCC_VERSION))
#elif defined(__KEIL__)
#define COMPILER_VERSION                    "KEIL_CARM " STRINGIZE_VAL(__CA__)
#elif defined(__IAR_SYSTEMS_ICC__)
#define COMPILER_VERSION                    __VERSION__
#else
#define COMPILER_VERSION                    "Compiler unknown"
#endif

//*****************************************************************************
//
// Common init for many examples
//
//*****************************************************************************
void
board_init(char * pcStartupString)
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
    //am_bsp_low_power_init();

    //
    // Initialize the printf interface for ITM output
    //
    am_bsp_debug_printf_enable();

    //
    // Print the banner.
    //
    am_util_stdio_terminal_clear();
    am_util_stdio_printf("%s\n\n", pcStartupString);

    am_util_stdio_printf("Compiler Name: %s\n\n", COMPILER_VERSION);

    //
    // Print the device info.
    //
    am_util_id_device(&sIdDevice);
    am_util_stdio_printf("Vendor Name: %s\n", sIdDevice.pui8VendorName);
    am_util_stdio_printf("Device type: %s\n\n", sIdDevice.pui8DeviceName);
#if (AM_BSP_NUM_LEDS > 0)
    am_devices_led_array_init(am_bsp_psLEDs, AM_BSP_NUM_LEDS);
    am_devices_led_array_out(am_bsp_psLEDs, AM_BSP_NUM_LEDS, 0);
#endif
}

uint32_t board_millis(void)
{
    return 0;
}

void
board_led_write(bool bLedState)
{
#if (AM_BSP_NUM_LEDS > 0)
    if (bLedState)
    {
        am_devices_led_on(am_bsp_psLEDs, 0);
    }
    else
    {
        am_devices_led_off(am_bsp_psLEDs, 0);
    }
#endif
}

