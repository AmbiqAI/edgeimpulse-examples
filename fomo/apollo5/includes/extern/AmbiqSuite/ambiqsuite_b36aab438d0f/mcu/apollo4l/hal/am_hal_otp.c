//*****************************************************************************
//
//! @file am_hal_otp.c
//!
//! @brief Implementation for One-Time Programmable Functionality
//!
//! @addtogroup otp_4l OTP - One-Time Programmable
//! @ingroup apollo4l_hal
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

//#define OTP_ACCESS_WA

/* poll on the AIB acknowledge bit */
#define AM_HAL_OTP_WAIT_ON_AIB_ACK_BIT()  \
    while (CRYPTO->AIBFUSEPROGCOMPLETED_b.AIBFUSEPROGCOMPLETED == 0)

//*****************************************************************************
//
//! @brief Validate the OTP Offset and whether it is within range
//!
//! @param  offset -  word aligned offset in OTP to be read
//!
//! @return Returns AM_HAL_STATUS_SUCCESS or OUT_OF_RANGE
//
//*****************************************************************************
static uint32_t validate_otp_offset(uint32_t offset)
{
    if ((offset & 0x3) || (offset > (AM_REG_OTP_SIZE - 4)))
    {
        return AM_HAL_STATUS_OUT_OF_RANGE;
    }
    else
    {
        return AM_HAL_STATUS_SUCCESS;
    }
}

//*****************************************************************************
//
// Read OTP word
//
// This will retrieve the OTP information
//
//*****************************************************************************
uint32_t am_hal_otp_read_word(uint32_t offset, uint32_t *pVal)
{
    uint32_t status = AM_HAL_STATUS_SUCCESS;

#ifndef AM_HAL_DISABLE_API_VALIDATION
    status = validate_otp_offset(offset);
    if (status != AM_HAL_STATUS_SUCCESS)
    {
        return status;
    }
    if (!pVal)
    {
        return AM_HAL_STATUS_INVALID_ARG;
    }
#endif
    if ((PWRCTRL->DEVPWRSTATUS_b.PWRSTCRYPTO == 0) || (CRYPTO->HOSTCCISIDLE_b.HOSTCCISIDLE == 0))
    {
        // Crypto is not accessible
        return AM_HAL_STATUS_INVALID_OPERATION;
    }

#ifdef OTP_ACCESS_WA
    AM_CRITICAL_BEGIN;
    *pVal = am_hal_load_ui32((uint32_t *)(AM_REG_OTP_BASEADDR + offset));
    AM_CRITICAL_END;
#else
    *pVal = AM_REGVAL(AM_REG_OTP_BASEADDR + offset);
#endif
    return status;
}

#ifdef OTP_ACCESS_WA
// #### INTERNAL BEGIN ####
#if 0
// Function used to create the OTP word write assembly.
void
otp_write_word_helper(uint32_t offset, uint32_t value)
{
#if 0
    AM_REGVAL(AM_REG_OTP_BASEADDR + offset) = value;
    AM_HAL_OTP_WAIT_ON_AIB_ACK_BIT();
#else
    // Write the word:
    // *(0x400C2000 + r0) = r1;
    __asm("movw r3, 0x2000");
    __asm("movt r3, 0x400C");
    __asm("str  r1, [r3, r0]");

    // AM_HAL_OTP_WAIT_ON_AIB_ACK_BIT();
    __asm("movw r3, 0x1f04");
    __asm("movt r3, 0x400C");
    __asm("loop:");
    __asm("ldr r0, [r3, #0]");
    __asm("lsls r0, r0, #31");
    __asm("beq.n loop");
#endif
}
#endif
// #### INTERNAL END ####

//
//! Writes the contents of R1 to the OTP offset pointed to by R0.
//
uint16_t otp_write_word_asm[] =
{
    0xf242, 0x0300, // movw r3, #8192 ; 0x2000
    0xf2c4, 0x030c, // movt r3, #16396 ; 0x400c
    0x5019,         // str  r1, [r3, r0]

    0xf641, 0x7304, // movw r3, #7940 ; 0x1f04
    0xf2c4, 0x030c, // movt r3, #16396 ; 0x400c
    0x6818,         // ldr  r0, [r3, #0] (loop)
    0x07c0,         // lsls r0, r0, #31
    0xd0fc,         // beq.n <loop>
    0x4770,         // bx lr
};

typedef void (*otp_write_word_t)(uint32_t, uint32_t);

otp_write_word_t otp_write_word_func = (otp_write_word_t)((uint8_t *)otp_write_word_asm + 1);
#endif

//*****************************************************************************
//
// Write OTP word
//
// This will write a word to the supplied offset in the OTP
//
//*****************************************************************************
uint32_t am_hal_otp_write_word(uint32_t offset, uint32_t value)
{
    uint32_t status = AM_HAL_STATUS_SUCCESS;

#ifndef AM_HAL_DISABLE_API_VALIDATION
    status = validate_otp_offset(offset);
    if (status != AM_HAL_STATUS_SUCCESS)
    {
        return status;
    }
#endif
    if ((PWRCTRL->DEVPWRSTATUS_b.PWRSTCRYPTO == 0) || (CRYPTO->HOSTCCISIDLE_b.HOSTCCISIDLE == 0))
    {
        // Crypto is not accessible
        return AM_HAL_STATUS_INVALID_OPERATION;
    }
#ifdef OTP_ACCESS_WA
    AM_CRITICAL_BEGIN;
    otp_write_word_func(offset, value);
    // Read back the value to compare
    if ((am_hal_load_ui32((uint32_t *)(AM_REG_OTP_BASEADDR + offset)) & value) != value)
    {
        status = AM_HAL_STATUS_FAIL;
    }

    AM_CRITICAL_END;
#else
    AM_REGVAL(AM_REG_OTP_BASEADDR + offset) = value;
    AM_HAL_OTP_WAIT_ON_AIB_ACK_BIT();
    // Read back the value to compare
    if ((AM_REGVAL(AM_REG_OTP_BASEADDR + offset) & value) != value)
    {
        status = AM_HAL_STATUS_FAIL;
    }
#endif

    return status;
}

//*****************************************************************************
//
// End Doxygen group.
//! @}
//
//*****************************************************************************
