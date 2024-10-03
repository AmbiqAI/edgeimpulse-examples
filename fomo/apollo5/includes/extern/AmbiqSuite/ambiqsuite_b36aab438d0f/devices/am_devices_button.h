//*****************************************************************************
//
//! @file am_devices_button.h
//!
//! @brief Functions for controlling an array of Buttons
//!
//! @addtogroup button Button Device Driver
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
#ifndef AM_DEVICES_BUTTON_H
#define AM_DEVICES_BUTTON_H

#ifdef __cplusplus
extern "C"
{
#endif

//*****************************************************************************
//
// Number of "ticks" to delay before registering a button press or release.
//
//*****************************************************************************
#define AM_DEVICES_BUTTON_DEBOUNCE_DELAY    0x4

//*****************************************************************************
//! @name ButtonPolarity
//! @brief Button polarity macros
//! @{
//
//*****************************************************************************
#define AM_DEVICES_BUTTON_NORMAL_HIGH       0x1
#define AM_DEVICES_BUTTON_NORMAL_LOW        0x0

//! end ButtonPolarity
//! @}

//*****************************************************************************
//
//! Structure for keeping track of buttons.
//
//*****************************************************************************
typedef struct
{
    uint32_t ui32GPIONumber;
    uint32_t ui32Polarity;
    uint32_t ui32Count;
    bool bPressed;
    bool bChanged;
}
am_devices_button_t;

//
//! stuct used by BSP where pin config is included in button definition
//
typedef struct
{
    am_devices_button_t sDevButton ;
    //
    //! pointer to pin config for button
    //
    am_hal_gpio_pincfg_t *tPinCfg ;
}
am_devices_button_pin_cfg_t;

//*****************************************************************************
//
//! Macro for declaring a button structure.
//
//*****************************************************************************
#define AM_DEVICES_BUTTON(ui32GPIONumber, ui32Polarity)                       \
    {ui32GPIONumber, ui32Polarity, 0, 0, 0}

//*****************************************************************************
//
//! @name ButtonStateMarcos
//! @brief Macros for checking button state.
//! @{
//
//*****************************************************************************
#define am_devices_button_is_up(button)                                       \
    ((button).bPressed == false)

#define am_devices_button_is_down(button)                                     \
    ((button).bPressed == true)

#define am_devices_button_pressed(button)                                     \
    (((button).bPressed == true) && ((button).bChanged == true))

#define am_devices_button_released(button)                                    \
    (((button).bPressed == false) && ((button).bChanged == true))

//! end ButtonStateMarcos
//! @}

//*****************************************************************************
//
// External function definitions
//
//*****************************************************************************
//*****************************************************************************
//
//! @brief This initializes a button to the default/disabled state.
//!
//! @param psButton is a pointer to a button structure.
//!
//! This function configures disables a button and resets the associated data.
//
//*****************************************************************************
extern void am_devices_button_init(am_devices_button_t *psButton);

//*****************************************************************************
//
//! @brief This configures the necessary pins for an array of buttons.
//!
//! @param psButtons is an array of button structures.
//! @param ui32NumButtons is the total number of buttons in the array.
//!
//! This function configures the GPIOs for an array of buttons.
//
//*****************************************************************************
extern void am_devices_button_array_init(am_devices_button_t *psButtons,
                                         uint32_t ui32NumButtons);

//*****************************************************************************
//
//! @brief Configures the pin for a button with pincfg param
//! and reads and debounces the pin, then disables the button
//!
//! @param psButton is an array of button structures.
//! @param am_hal_pingcfg is the pinConfig parameter
//!
//! This function configures the GPIO for a buttons.
//
//*****************************************************************************
extern void am_devices_button_tick_pin_cfg(am_devices_button_t *psButton,
                                           am_hal_gpio_pincfg_t am_hal_pingcfg) ;

//*****************************************************************************
//
//! @brief Configures the pin for a button with a default pincfg
//! then reads and debounces the pin, then disables the button
//!
//! @note this is the legacy call
//!
//! @param psButton is an array of button structures.
//!
//! This function configures the GPIO for a buttons.
//
//*****************************************************************************
extern void am_devices_button_tick(am_devices_button_t *psButton);

//*****************************************************************************
//
//! @brief Configures the necessary pins for an array of buttons.
//! then this will read, debounce, and disable the button pins
//!
//! @note this is the legacy call that uses a default pincfg
//!
//! @param psButtons is an array of button structures.
//! @param ui32NumButtons is the total number of buttons in the array.
//!
//! This function configures the GPIOs for an array of buttons.
//
//*****************************************************************************
extern void am_devices_button_array_tick(am_devices_button_t *psButtons,
                                         uint32_t ui32NumButtons);
//*****************************************************************************
//
//! @brief Configures the necessary pins for an array of buttons.
//! then this will read, debounce, and disable the button pins
//!
//! @param psButtons is an array of button structures.
//! @param am_hal_pingcfgs an array of pincfgs, one for each button
//! @param ui32NumButtons is the total number of buttons in the array.
//!
//! This function configures the GPIOs for an array of buttons.
//
//*****************************************************************************
extern void am_devices_button_array_tick_pin_cfg(am_devices_button_t *psButtons,
                                                 am_hal_gpio_pincfg_t *am_hal_pingcfgs,
                                                 uint32_t ui32NumButtons) ;

//*****************************************************************************
//
//! @brief Configures the necessary pins for an array of buttons.
//! then this will read, debounce, and disable the button pins
//!
//! @param psButtonInfo is an array of button pin config structures.
//! @param ui32NumButtons is the total number of buttons in the array.
//!
//! This function configures the GPIOs for an array of buttons.
//
//*****************************************************************************
extern void am_devices_button_array_pin_config(am_devices_button_pin_cfg_t *psButtonInfo,
                                             uint32_t ui32NumButtons ) ;

#ifdef __cplusplus
}
#endif

#endif // AM_DEVICES_BUTTON_H

//*****************************************************************************
//
// End Doxygen group.
//! @}
//
//*****************************************************************************

