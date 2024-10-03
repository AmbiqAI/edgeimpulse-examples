//*****************************************************************************
//
//! @file apollo4_board.c
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

#if defined (AM_FREERTOS)
#include "FreeRTOS.h"
#include "task.h"
// FreeRTOS has STimer in used already.
#else
#define WAKE_INTV_MS    (50)
#define WAKE_INTV_TICKS ((WAKE_INTV_MS * 32768) / 1000)

static uint32_t s_ui32Millis = 0;


void am_stimer_cmpr0_isr(void)
{
    //
    // Check the timer interrupt status.
    //
    am_hal_stimer_int_clear(AM_HAL_STIMER_INT_COMPAREA);
    am_hal_stimer_compare_delta_set(0, WAKE_INTV_TICKS);
    s_ui32Millis += WAKE_INTV_MS;
}

void stimer_init(void)
{
    //
    // Enable compare A interrupt in STIMER
    //
    am_hal_stimer_int_enable(AM_HAL_STIMER_INT_COMPAREA);

    //
    // Enable the timer interrupt in the NVIC.
    //
    NVIC_SetPriority(STIMER_CMPR0_IRQn, AM_IRQ_PRIORITY_DEFAULT);
    NVIC_EnableIRQ(STIMER_CMPR0_IRQn);

    //
    // Configure the STIMER and run
    //
    am_hal_stimer_config(AM_HAL_STIMER_CFG_CLEAR | AM_HAL_STIMER_CFG_FREEZE);
    am_hal_stimer_compare_delta_set(0, WAKE_INTV_TICKS);
    am_hal_stimer_config(AM_HAL_STIMER_XTAL_32KHZ |
                         AM_HAL_STIMER_CFG_COMPARE_A_ENABLE);

}
#endif

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
    am_util_stdio_printf("USB CDC Disconnect Example\n\n");

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

#if defined (AM_FREERTOS)
    // FreeRTOS has STimer in used already.
#else
    stimer_init();
    am_hal_interrupt_master_enable();
#endif
}

uint32_t
board_millis(void)
{
#if defined (AM_FREERTOS)
    TickType_t xTick = xTaskGetTickCount();
    return (uint32_t)xTick;
#else
    return s_ui32Millis;
#endif
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
    return;
}

