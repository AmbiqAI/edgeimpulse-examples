//*****************************************************************************
//
//! @file pwr_gpio_utils.h
//!
//! @brief This will manage GPIO for the example
//!
//! @addtogroup power_examples Power Examples
//!
//! @defgroup pwr_32mhz 32Mhz XTHS and HFRC2 Power Example
//! @ingroup power_examples
//! @{
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
#ifndef PWR_GPIO_UTILS_H
#define PWR_GPIO_UTILS_H

#include <stdint.h>
#include <stdbool.h>
#include "am_mcu_apollo.h"

//
//! Enumerate the debug pin names
//
typedef enum
{
    eDEBUG_PIN_FIRST,   //!< Keep this name it is used in a loop as the first element
    eDEBUG_PIN_1 = eDEBUG_PIN_FIRST,
    eDEBUG_PIN_2,
    eDEBUG_PIN_SLEEP,   //!< pin low when in sleep, high when not sleeping
    eMAX_DEBUG_PINS
}
dbg_pin_names_e;

//*****************************************************************************
//
//! @brief      - init a debug GPIO pin
//!
//! @param debug pin number - GPIO debug is enabled when this is true
//!
//! @note       if pin number is invalid (>500), the debug pin is assumed deactivated
//!
//! @return     - Standard hal status
//!
//
//*****************************************************************************
extern uint32_t gpio_debug_init(uint32_t ui32GpioPinNUmber);

//*****************************************************************************
//
//! @brief      - init the debug GPIO pins
//!
//! @param      - pointer to an array of debug pin numbers
//!
//! @return     - Standard hal status
//!
//
//*****************************************************************************
extern uint32_t gpio_dbg_pin_init( uint32_t *pui32DbgPinTable);


uint32_t gpio_enable_hfrc2_clk_output(am_hal_clkgen_clkout_e clokOut, uint32_t pinNumber);

#endif //PWR_GPIO_UTILS_H
//*****************************************************************************
//
// End Doxygen group.
//! @}
//
//*****************************************************************************

