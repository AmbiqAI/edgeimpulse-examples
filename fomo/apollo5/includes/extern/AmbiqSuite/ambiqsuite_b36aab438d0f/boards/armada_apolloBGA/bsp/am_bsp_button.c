//*****************************************************************************
//
//! @file am_bsp_button.c
//!
//! @brief armada-apolloBGA board support for button interaction.
//!
//! These functions implement the button management for the BSP for
//! armada-apolloBGA.
//!
//! AM_BSP_BUTTON_NUM       defines the muximum Number of user Buttons
//! AM_BSP_BUTTON_NUM_MCU   defines the muximum Number of user Buttons on the
//! MCU itself.
//!
//! For BUTTON parameters higher than AM_BSP_BUTTON_NUM_MCU, off chip I2C GPIO
//! buttons will be used on Ambiq boards.  Here, AM_BSP_BUTTON_NUM and
//! AM_BSP_BUTTON_NUM_MCU are the same.
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
#ifndef AM_BSP_GPIO_BUTTON0
#define AM_BSP_GPIO_BUTTON0                 -1
#endif

#ifndef AM_BSP_GPIO_BUTTON1
#define AM_BSP_GPIO_BUTTON1                 -1
#endif

#ifndef AM_BSP_GPIO_BUTTON2
#define AM_BSP_GPIO_BUTTON2                 -1
#endif

#ifndef AM_BSP_GPIO_BUTTON3
#define AM_BSP_GPIO_BUTTON3                 -1
#endif

#ifndef AM_BSP_GPIO_BUTTON4
#define AM_BSP_GPIO_BUTTON4                 -1
#endif

#ifndef AM_BSP_GPIO_BUTTON5
#define AM_BSP_GPIO_BUTTON5                 -1
#endif

#ifndef AM_BSP_GPIO_BUTTON6
#define AM_BSP_GPIO_BUTTON6                 -1
#endif

#ifndef AM_BSP_GPIO_BUTTON7
#define AM_BSP_GPIO_BUTTON7                 -1
#endif

//*****************************************************************************
//
// Mapping array for BUTTON number to Apollo GPIO bits for the armada-apolloBGA
// board.
//
//*****************************************************************************
static const int32_t i32ButtonPinNumber[] =
{
    AM_BSP_GPIO_BUTTON0,
    AM_BSP_GPIO_BUTTON1,
    AM_BSP_GPIO_BUTTON2,
    AM_BSP_GPIO_BUTTON3,
    AM_BSP_GPIO_BUTTON4,
    AM_BSP_GPIO_BUTTON5,
    AM_BSP_GPIO_BUTTON6,
    AM_BSP_GPIO_BUTTON7
};

//*****************************************************************************
//
// Function that gets the value of a single button.
//
//*****************************************************************************
int32_t
am_bsp_button_get(uint32_t ui32ButtonNum)
{
    //
    // Check to make sure that we're not asking for a button that we don't
    // have.
    //
    if ( ui32ButtonNum > AM_BSP_BUTTON_NUM - 1 )
    {
        return -1000;
    }

    //
    // Make sure that we're not asking for a button without a pin.
    //
    if ( i32ButtonPinNumber[ui32ButtonNum] < 0 )
    {
        return -1001;
    }

    return am_hal_gpio_input_bit_read(i32ButtonPinNumber[ui32ButtonNum]);
}

//*****************************************************************************
//
//  Function that gets the value of all of the buttons
//
//  Walk the list of GPIO pins attached to buttons and accumulate
//  their values.
//
//  NOTE: Only works for less than 31 buttons
//
//*****************************************************************************
int32_t
am_bsp_button_get_all(void)
{
    int i;
    uint32_t ui32Value;

    //
    // Read the value of each GPIO pin attached to a button
    //
    ui32Value = 0;
    for ( i = 0; i < AM_BSP_BUTTON_NUM; i++ )
    {
       if ( i32ButtonPinNumber[i] < 0 )
       {
           return -1001;
       }

       //
       // OR the button's state into the return value.
       //
       ui32Value |= am_hal_gpio_input_bit_read(i32ButtonPinNumber[i]) << i;
    }

    return ui32Value;
}
