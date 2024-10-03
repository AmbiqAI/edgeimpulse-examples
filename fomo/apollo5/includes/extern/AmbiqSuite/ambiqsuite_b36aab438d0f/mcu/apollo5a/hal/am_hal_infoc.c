//*****************************************************************************
//
//! @file am_hal_infoc.c
//!
//! @brief Functions for INFOC functions
//!
//! @addtogroup infoc INFOC - One-Time Programmable
//! @ingroup apollo5a_hal
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

//
// poll on the AIB acknowledge bit
//
#define AM_HAL_INFOC_WAIT_ON_AIB_ACK_BIT()  \
    while (CRYPTO->AIBFUSEPROGCOMPLETED_b.AIBFUSEPROGCOMPLETED == 0)

//*****************************************************************************
//
//! @brief Validate the INFOC Offset and whether it is within range
//!
//! @param  offset -  word aligned offset in INFOC to be read
//!
//! @return Returns AM_HAL_STATUS_SUCCESS or OUT_OF_RANGE
//
//*****************************************************************************
static uint32_t
validate_infoc_offset(uint32_t offset)
{
    if ( offset & 0x3 )
    {
        return AM_HAL_STATUS_INVALID_ARG;
    }

    if ( MCUCTRL->SHADOWVALID_b.INFOCSELOTP )
    {
        if ( offset > (AM_REG_OTP_INFOC_SIZE - 4) )
        {
            return AM_HAL_STATUS_OUT_OF_RANGE;
        }
    }
    else
    {
        if ( offset > (AM_REG_INFOC_SIZE - 4) )
        {
            return AM_HAL_STATUS_OUT_OF_RANGE;
        }
    }

    return AM_HAL_STATUS_SUCCESS;

} // validate_infoc_offset()

//*****************************************************************************
//
// brief Check that Crypto is available and that INFOC is powered up.
//
//*****************************************************************************
static uint32_t
validate_infoc_accessible(void)
{
    if ( ( MCUCTRL->SHADOWVALID_b.INFOCSELOTP == 1 )    &&
         ( (PWRCTRL->DEVPWRSTATUS_b.PWRSTCRYPTO == PWRCTRL_DEVPWRSTATUS_PWRSTCRYPTO_OFF)    ||
           (PWRCTRL->MEMPWRSTATUS_b.PWRSTOTP    == 0)                                       ||
           (CRYPTO->HOSTCCISIDLE_b.HOSTCCISIDLE == 0) ) )
    {
        //
        // Crypto or INFOC is not accessible
        //
        return AM_HAL_STATUS_INVALID_OPERATION;
    }
    else
    {
        return AM_HAL_STATUS_SUCCESS;
    }

} // validate_infoc_accessible()

//*****************************************************************************
//
// Read INFOC word
//
// Retrieve a word of data from INFOC.
//
// Note: The caller is responsible for determining whether INFOC is MRAM or OTP
//       and if OTP, powering up OTP before calling this function.
//       Determine OTP or MRAM via: MCUCTRL->SHADOWVALID_b.INFOCSELOTP
//
//       The current power status of OTP can be determined via
//       am_hal_pwrctrl_periph_enabled(AM_HAL_PWRCTRL_PERIPH_OTP, &bEnabled);
//
//       OTP is enabled or disabled via:
//       am_hal_pwrctrl_periph_enable(AM_HAL_PWRCTRL_PERIPH_OTP);
//       am_hal_pwrctrl_periph_disable(AM_HAL_PWRCTRL_PERIPH_OTP);
//
//*****************************************************************************
uint32_t
am_hal_infoc_read_word(uint32_t offset, uint32_t *pVal)
{
    uint32_t ui32status = AM_HAL_STATUS_SUCCESS;

#ifndef AM_HAL_DISABLE_API_VALIDATION
    ui32status = validate_infoc_offset(offset);
    if (ui32status != AM_HAL_STATUS_SUCCESS)
    {
        return ui32status;
    }

    if ( !pVal )
    {
        return AM_HAL_STATUS_INVALID_ARG;
    }
#endif

    ui32status = validate_infoc_accessible();
    if ( ui32status != AM_HAL_STATUS_SUCCESS )
    {
        *pVal = 0x0;
        return ui32status;
    }

    *pVal = AM_REGVAL(AM_REG_OTP_INFOC_BASEADDR + offset);

    return ui32status;
} // am_hal_infoc_read_word()

//*****************************************************************************
//
// Write INFOC word
//
// Write a word to the supplied offset in the INFOC.
//
// Note: The caller is responsible for determining whether INFOC is MRAM or OTP
//       and if OTP, powering up OTP before calling this function.
//       Determine OTP or MRAM via: MCUCTRL->SHADOWVALID_b.INFOCSELOTP
//
//       The current power status of OTP can be determined via
//       am_hal_pwrctrl_periph_enabled(AM_HAL_PWRCTRL_PERIPH_OTP, &bEnabled);
//
//       OTP is enabled or disabled via:
//       am_hal_pwrctrl_periph_enable(AM_HAL_PWRCTRL_PERIPH_OTP);
//       am_hal_pwrctrl_periph_disable(AM_HAL_PWRCTRL_PERIPH_OTP);
//
//*****************************************************************************
uint32_t
am_hal_infoc_write_word(uint32_t offset, uint32_t value)
{
    uint32_t ui32status = AM_HAL_STATUS_SUCCESS;

#ifndef AM_HAL_DISABLE_API_VALIDATION
    ui32status = validate_infoc_offset(offset);
    if (ui32status != AM_HAL_STATUS_SUCCESS)
    {
        return ui32status;
    }
#endif

    ui32status = validate_infoc_accessible();
    if ( ui32status != AM_HAL_STATUS_SUCCESS )
    {
        return ui32status;
    }

    AM_REGVAL(AM_REG_OTP_INFOC_BASEADDR + offset) = value;

    AM_HAL_INFOC_WAIT_ON_AIB_ACK_BIT();

    //
    // Read back the value to compare
    //
    if ((AM_REGVAL(AM_REG_OTP_INFOC_BASEADDR + offset) & value) != value)
    {
        ui32status = AM_HAL_STATUS_FAIL;
    }

    return ui32status;

} // am_hal_infoc_write_word()

//*****************************************************************************
//
// End Doxygen group.
//! @}
//
//*****************************************************************************
