//*****************************************************************************
//
//! @file am_hal_otp.h
//!
//! @brief Functions for OTP functions
//!
//! @addtogroup otp OTP - One-Time Programmable
//! @ingroup bronco_hal
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

#ifndef AM_HAL_OTP_H
#define AM_HAL_OTP_H

#define AM_REG_OTP_SIZE 0x2000

#ifdef __cplusplus
extern "C"
{
#endif

//*****************************************************************************
//
//! @brief  Read OTP word
//!
//! @param  offset -  word aligned offset in OTP to be read
//! @param  pVal -  Pointer to word for returned data
//!
//! This will retrieve the OTP information
//!
//! @return Returns AM_HAL_STATUS_SUCCESS on success
//
//*****************************************************************************
uint32_t am_hal_otp_read_word(uint32_t offset, uint32_t *pVal);

//*****************************************************************************
//
//! @brief  Write OTP word
//!
//! @param  offset -  word aligned offset in OTP to be read
//! @param  value -  value to be written
//!
//! This will write a word to the supplied offset in the OTP
//!
//! @return Returns AM_HAL_STATUS_SUCCESS on success
//
//*****************************************************************************
uint32_t am_hal_otp_write_word(uint32_t offset, uint32_t value);

#ifdef __cplusplus
}
#endif

#endif // AM_HAL_OTP_H

//*****************************************************************************
//
// End Doxygen group.
//! @}
//
//*****************************************************************************

