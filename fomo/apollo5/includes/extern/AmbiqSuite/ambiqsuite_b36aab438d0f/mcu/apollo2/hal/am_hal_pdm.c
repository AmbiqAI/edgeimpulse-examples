//*****************************************************************************
//
//  am_hal_pdm.c
//! @file
//!
//! @brief Functions for interfacing with Pulse Density Modulation (PDM).
//!
//! @addtogroup pdm2 DMEMS Microphon3 (PDM)
//! @ingroup apollo2hal
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

#include <stdint.h>
#include <stdbool.h>
#include "am_mcu_apollo.h"

//*****************************************************************************
//
//! @brief Configure the PDM module.
//!
//! This function reads the an \e am_hal_pdm_config_t structure and uses it to
//! set up the PDM module.
//!
//! Please see the information on the am_hal_pdm_config_t configuration
//! structure, found in am_hal_pdm.h, for more information on the parameters
//! that may be set by this function.
//!
//! @return None.
//
//*****************************************************************************
void
am_hal_pdm_config(am_hal_pdm_config_t *psConfig)
{
    //
    // setup the PDM PCFG register
    //
    AM_REG(PDM, PCFG) = psConfig->ui32PDMConfigReg;

    //
    // setup the PDM VCFG register
    //
    AM_REG(PDM, VCFG) = psConfig->ui32VoiceConfigReg;

    //
    // setup the PDM FIFO Threshold register
    //
    AM_REG(PDM, FTHR) = psConfig->ui32FIFOThreshold;

    //
    // Flush the FIFO for good measure.
    //
    am_hal_pdm_fifo_flush();
}

//*****************************************************************************
//
//! @brief Enable the PDM module.
//!
//! This function enables the PDM module in the mode previously defined by
//! am_hal_pdm_config().
//!
//! @return None.
//
//*****************************************************************************
void
am_hal_pdm_enable(void)
{
    AM_REG(PDM, PCFG) |=  AM_REG_PDM_PCFG_PDMCORE_EN;
    AM_REG(PDM, VCFG) |=  ( AM_REG_PDM_VCFG_IOCLKEN_EN      |
                            AM_REG_PDM_VCFG_PDMCLK_EN       |
                            AM_REG_PDM_VCFG_RSTB_NORM );
}

//*****************************************************************************
//
//! @brief Disable the PDM module.
//!
//! This function disables the PDM module.
//!
//! @return None.
//
//*****************************************************************************
void
am_hal_pdm_disable(void)
{
    AM_REG(PDM, PCFG) &= ~ AM_REG_PDM_PCFG_PDMCORE_EN;
    AM_REG(PDM, VCFG) &= ~ (  AM_REG_PDM_VCFG_IOCLKEN_EN    |
                              AM_REG_PDM_VCFG_PDMCLK_EN     |
                              AM_REG_PDM_VCFG_RSTB_NORM );
}

//*****************************************************************************
//
//! @brief Return the PDM Interrupt status.
//!
//! @param bEnabledOnly - return only the enabled interrupts.
//!
//! Use this function to get the PDM interrupt status.
//!
//! @return intrrupt status
//
//*****************************************************************************
uint32_t
am_hal_pdm_int_status_get(bool bEnabledOnly)
{
    if ( bEnabledOnly )
    {
        uint32_t ui32RetVal = AM_REG(PDM, INTSTAT);
        return ui32RetVal & AM_REG(PDM, INTEN);
    }
    else
    {
        return AM_REG(PDM, INTSTAT);
    }
}

//*****************************************************************************
//
//  End the doxygen group
//! @}
//
//*****************************************************************************
