//*****************************************************************************
//
//! @file hfrc2_test_utils.c
//!
//! @brief tinyusb hfrc2 utilities
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
#include "hfrc2_test_utils.h"

//
//! used to output HFRC2 clock on pin 33
//
const am_hal_gpio_pincfg_t g_gpio_pincfg_clkout33 =
{
    .GP.cfg_b.uFuncSel         = GPIO_PINCFG33_FNCSEL33_CLKOUT,
    .GP.cfg_b.eGPOutCfg        = AM_HAL_GPIO_PIN_OUTCFG_DISABLE,
    .GP.cfg_b.eDriveStrength   = AM_HAL_GPIO_PIN_DRIVESTRENGTH_1P0X,
    .GP.cfg_b.ePullup          = AM_HAL_GPIO_PIN_PULLUP_NONE,
    .GP.cfg_b.eGPInput         = AM_HAL_GPIO_PIN_INPUT_ENABLE,
    .GP.cfg_b.eGPRdZero        = AM_HAL_GPIO_PIN_RDZERO_READPIN,
    .GP.cfg_b.eIntDir          = AM_HAL_GPIO_PIN_INTDIR_LO2HI,
    .GP.cfg_b.uSlewRate        = 0,
    .GP.cfg_b.uNCE             = 0,
    .GP.cfg_b.eCEpol           = 0,
    .GP.cfg_b.ePowerSw         = 0,
    /* The CLKOUT function does not enable input, so force it */
    .GP.cfg_b.eForceInputEn    = 1,
    .GP.cfg_b.eForceOutputEn   = 0,
    .GP.cfg_b.uRsvd_0          = 0,
    .GP.cfg_b.uRsvd_1          = 0,

};

//
//! used to output HFRC2 clock on pin 80
//
const am_hal_gpio_pincfg_t g_gpio_pincfg_clkout80 =
{
    .GP.cfg_b.uFuncSel         = GPIO_PINCFG80_FNCSEL80_CLKOUT,
    .GP.cfg_b.eGPOutCfg        = AM_HAL_GPIO_PIN_OUTCFG_DISABLE,
    .GP.cfg_b.eDriveStrength   = AM_HAL_GPIO_PIN_DRIVESTRENGTH_1P0X,
    .GP.cfg_b.ePullup          = AM_HAL_GPIO_PIN_PULLUP_NONE,
    .GP.cfg_b.eGPInput         = AM_HAL_GPIO_PIN_INPUT_ENABLE,
    .GP.cfg_b.eGPRdZero        = AM_HAL_GPIO_PIN_RDZERO_READPIN,
    .GP.cfg_b.eIntDir          = AM_HAL_GPIO_PIN_INTDIR_LO2HI,
    .GP.cfg_b.uSlewRate        = 0,
    .GP.cfg_b.uNCE             = 0,
    .GP.cfg_b.eCEpol           = 0,
    .GP.cfg_b.ePowerSw         = 0,
    /* The CLKOUT function does not enable input, so force it */
    .GP.cfg_b.eForceInputEn    = 1,
    .GP.cfg_b.eForceOutputEn   = 0,
    .GP.cfg_b.uRsvd_0          = 0,
    .GP.cfg_b.uRsvd_1          = 0,

};

//*****************************************************************************
//
// Enable HFRC2 pin output
//
//*****************************************************************************
uint32_t hfrc2_enable_hfrc2_clock_out(am_hal_clkgen_clkout_e eClockOut, uint32_t pinNumber)
{

    am_hal_gpio_pincfg_t pinCfg = g_gpio_pincfg_clkout33 ;
    if ( pinNumber == 80 )
    {
        pinCfg = g_gpio_pincfg_clkout80;
    }
    else
    {
        pinNumber = 33 ;
    }

    if (AM_HAL_STATUS_SUCCESS != am_hal_gpio_pinconfig(pinNumber, pinCfg))
    {
        return AM_HAL_STATUS_INVALID_OPERATION;
    }

    CLKGEN->CLKOUT_b.CKSEL = eClockOut;
    CLKGEN->CLKOUT_b.CKEN = 1;

    return AM_HAL_STATUS_SUCCESS;

}
//*****************************************************************************
//
// End Doxygen group.
//! @}
//
//*****************************************************************************


