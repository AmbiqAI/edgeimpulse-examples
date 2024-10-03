//*****************************************************************************
//
//! @file am_bsp_led.c
//!
//! @brief armada-apolloBGA board support for GPIO
//!
//! Structures and functions to support LED operations.
//! NOTE  Armada needs complement drive for LEDS  0 on the pin == LED on
//!
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
// Make sure there are definitions for all of the buttons we might have.
//
//*****************************************************************************
#ifndef AM_BSP_GPIO_LED0
#define AM_BSP_GPIO_LED0                    0xffffffff
#endif

#ifndef AM_BSP_GPIO_LED1
#define AM_BSP_GPIO_LED1                    0xffffffff
#endif

#ifndef AM_BSP_GPIO_LED2
#define AM_BSP_GPIO_LED2                    0xffffffff
#endif

#ifndef AM_BSP_GPIO_LED3
#define AM_BSP_GPIO_LED3                    0xffffffff
#endif

#ifndef AM_BSP_GPIO_LED4
#define AM_BSP_GPIO_LED4                    0xffffffff
#endif

#ifndef AM_BSP_GPIO_LED5
#define AM_BSP_GPIO_LED5                    0xffffffff
#endif

#ifndef AM_BSP_GPIO_LED6
#define AM_BSP_GPIO_LED6                    0xffffffff
#endif

#ifndef AM_BSP_GPIO_LED7
#define AM_BSP_GPIO_LED7                    0xffffffff
#endif

//*****************************************************************************
//
// Mapping array for LED # to Apollo GPIO bits for the armada-apollo MCU board
// for the armada-apollo MCU board the LEDS on the PMOD are reversed in order
//
//*****************************************************************************
static const uint32_t led_gpio_number[] =
{
    AM_BSP_GPIO_LED0,
    AM_BSP_GPIO_LED1,
    AM_BSP_GPIO_LED2,
    AM_BSP_GPIO_LED3,
    AM_BSP_GPIO_LED4,
    AM_BSP_GPIO_LED5,
    AM_BSP_GPIO_LED6,
    AM_BSP_GPIO_LED7
};

//*****************************************************************************
//
// Initialize LED Pins
//
//*****************************************************************************
void
am_bsp_led_init(void)
{
    uint32_t i;

    //
    //  Turn off all LEDs directly attached to the MCU
    //
    for (i = 0; i < AM_BSP_LED_NUM; i++ )
    {
        am_hal_gpio_out_bit_set(led_gpio_number[i]);
    }
}

//*****************************************************************************
//
// Function that turns on requested LED
//
//*****************************************************************************
void
am_bsp_led_on(uint32_t ui32LEDNum)
{
    if (ui32LEDNum < AM_BSP_LED_NUM)
    {
        //
        // Turn on the specified LED (on chip or off chip
        //
        am_hal_gpio_out_bit_clear(led_gpio_number[ui32LEDNum]);
    }
}

//*****************************************************************************
//
// Function that turns off requested LED
//
//*****************************************************************************
void
am_bsp_led_off(uint32_t ui32LEDNum)
{
    if (ui32LEDNum < AM_BSP_LED_NUM)
    {
        //
        // Turn off the specified LED (on chip or off chip
        //
        am_hal_gpio_out_bit_set(led_gpio_number[ui32LEDNum]);
    }
}

//*****************************************************************************
//
// Function that outputs value to LEDs
//
//*****************************************************************************
void
am_bsp_led_out(uint32_t ui32Value)
{
    int i;

    for (i = 0; i < AM_BSP_LED_NUM; i++)
    {
        if ( ui32Value & (1 << i) )
        {
            am_bsp_led_on(i);
        }
        else
        {
            am_bsp_led_off(i);
        }
    }
}

//*****************************************************************************
//
// Function that gets the value of an LED
//
//*****************************************************************************
int
am_bsp_led_get(uint32_t ui32LEDNum)
{
    //
    // Check to make sure that the requested LED number is in range.
    //
    if (ui32LEDNum < AM_BSP_LED_NUM)
    {
        //
        // Read the GPIO for the chosen LED.
        //
        return (am_hal_gpio_out_bit_read(led_gpio_number[ui32LEDNum])) ? 0: 1;
    }
    else
    {
        return -1;
    }
}

//*****************************************************************************
//
// Function that toggles the state of the requested LED
//
//*****************************************************************************
void
am_bsp_led_toggle(uint32_t ui32LEDNum)
{
    if ( ui32LEDNum < AM_BSP_LED_NUM )
    {
        //
        // Toggle the state specified LED (on chip or off chip
        //
        am_hal_gpio_out_bit_toggle(led_gpio_number[ui32LEDNum]);
    }
}
