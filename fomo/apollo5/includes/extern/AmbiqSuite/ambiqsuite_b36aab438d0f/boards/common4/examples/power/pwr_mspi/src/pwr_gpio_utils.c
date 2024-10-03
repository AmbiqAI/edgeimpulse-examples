//*****************************************************************************
//
//! @file pwr_gpio_utils.c
//!
//! @brief This will manage GPIO for the example
//!
//! @addtogroup power_examples Power Examples
//!
//! @defgroup pwr_mspi MSPI Power Example
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

//
//! Define max pin number.
//! @note Choose a number greater than MAX_pins of any Apollo4 that will ever exist.
//! At times, value 0xFFFF is used to denote an unused or invalid pin, so a number
//! between the actual device MAX-pin value and 0xFFFF is needed.
//
#define MAX_VALID_PIN_NUMBER 500

#include "pwr_gpio_utils.h"

//
//! pin config for GPIO debug output pins
//
const am_hal_gpio_pincfg_t g_gpio_default_output_cfg =
{
    .GP.cfg_b.uFuncSel         = 3,
    .GP.cfg_b.eGPOutCfg        = AM_HAL_GPIO_PIN_OUTCFG_PUSHPULL,
    .GP.cfg_b.eDriveStrength   = AM_HAL_GPIO_PIN_DRIVESTRENGTH_0P1X,
    .GP.cfg_b.ePullup          = AM_HAL_GPIO_PIN_PULLUP_NONE,
    .GP.cfg_b.eGPInput         = AM_HAL_GPIO_PIN_INPUT_NONE,
    .GP.cfg_b.eGPRdZero        = AM_HAL_GPIO_PIN_RDZERO_READPIN,
    .GP.cfg_b.eIntDir          = AM_HAL_GPIO_PIN_INTDIR_LO2HI,
    .GP.cfg_b.uSlewRate        = 0,
    .GP.cfg_b.uNCE             = 0,
    .GP.cfg_b.eCEpol           = 0,
    .GP.cfg_b.ePowerSw         = 0,
    .GP.cfg_b.eForceInputEn    = 0,
    .GP.cfg_b.eForceOutputEn   = 0,
    .GP.cfg_b.uRsvd_0          = 0,
    .GP.cfg_b.uRsvd_1          = 0,
};


//*****************************************************************************
//
// Init one debug GPIO pin
//
//*****************************************************************************
uint32_t
gpio_debug_init(uint32_t ui32GpioPinNUmber)
{
    //
    // debug pin setup
    //
    if (ui32GpioPinNUmber >= MAX_VALID_PIN_NUMBER)
    {
        //
        // this pin is invalid or unallocated
        // this is not necessarily an error, so
        // return status success
        //
        return AM_HAL_STATUS_SUCCESS;
    }

    return am_hal_gpio_pinconfig(ui32GpioPinNUmber, g_gpio_default_output_cfg);
}


//*****************************************************************************
//
// Init all the debug GPIO pins
//
//*****************************************************************************
uint32_t
gpio_dbg_pin_init(uint32_t *pui32DbgPinTable)
{
    uint32_t ui32stat = 0;

    for (dbg_pin_names_e i = eDEBUG_PIN_FIRST; i < eMAX_DEBUG_PINS; i++)
    {
        ui32stat += gpio_debug_init(pui32DbgPinTable[i]);
    }
    return ui32stat;
}
//*****************************************************************************
//
// End Doxygen group.
//! @}
//
//*****************************************************************************
