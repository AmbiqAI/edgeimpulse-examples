//*****************************************************************************
//
//! @file pwr_iom_utils.h
//!
//! utilities used to setup IOM pins
//!
//! @addtogroup power_examples Power Examples
//!
//! @defgroup pwr_iom_spi SPI Power Example
//! @ingroup power_examples
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
#ifndef PWR_IOM_UTILS_H
#define PWR_IOM_UTILS_H

#include "am_mcu_apollo.h"

//
//! @brief each pin can be setup as normal(spi/i2c mode), off, hi output, low output
//
typedef enum
{
    IOM_PIN_OFF,
    IOM_PIN_NOMRAL,
    IOM_PIN_GPIO_LO,
    IOM_PIN_GPIO_HI,
}
iom_pin_out_mode_e;

//
//! @brief This struct contains the IOM number and the operating mode for each IOM pin
//! and whether the IOM is SPI or I2C.
//
typedef struct
{
    uint32_t ui32IomNumber;
    iom_pin_out_mode_e eSCKPin;
    iom_pin_out_mode_e eMisoPin;
    iom_pin_out_mode_e eMosiPin;
    iom_pin_out_mode_e eCSPin;
    am_hal_iom_mode_e  eIomMode;       //<! SPI or I2C
}
iom_pins_setup_t;


// ****************************************************************************
//
//! @brief init the IOM pins depending on selected configuration
//!
//! @param psIOM_cfg  pointer to iom pin setup strcture
//!
//! @return
//
// ****************************************************************************
uint32_t pwr_iom_gpio_setup(const iom_pins_setup_t *psIOM_cfg);

// ****************************************************************************
//! @brief This will retrive the debug pin setups for this module
//! These are the pins used for GPIO debug tracking
//!
//! @param psIOM_cfg  - pointer to iom config, the dbug pins may vary depending on
//!                     IOM used
//! @param pui32DbgPinArray     - The debug pin (pin numbers) map. This is an array
//! @param ui32MaxDebugPins    - The number of pins to puopulate in the array
//!
//! @return always returns HAL standard success (0)
// ****************************************************************************
uint32_t
pwr_iom_getDebugPins(const iom_pins_setup_t *psIOM_cfg,
                     uint32_t *pui32DbgPinArray,
                     uint32_t ui32MaxDebugPins);



#endif // PWR_IOM_UTILS_H
//*****************************************************************************
//
// End Doxygen group.
//! @}
//
//*****************************************************************************
