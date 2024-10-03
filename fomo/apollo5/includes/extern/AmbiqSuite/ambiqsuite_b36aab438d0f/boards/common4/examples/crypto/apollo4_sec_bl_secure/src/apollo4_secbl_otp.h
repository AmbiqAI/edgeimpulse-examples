//*****************************************************************************
//!
//! @file apollo4_secbl_otp.h
//!
//! @brief the header file of OTP access functions
//!
//*****************************************************************************

//*****************************************************************************
//
// ${copyright}
//
// This is part of revision ${version} of the AmbiqSuite Development Package.
//
//*****************************************************************************

#ifndef APOLLO4_SECBL_OTP_H
#define APOLLO4_SECBL_OTP_H

#include "am_mcu_apollo.h"

#define ENFORCE_ENABLE                  2

//
// get the OTP rollback information
//
static inline bool ssbl_otp_get_NoRollback(void)
{
    return true;
}

//*****************************************************************************
//
//! @brief get the authentification key from the OTP
//!
//! @param autheKey - auth key index
//! @param pAuthKey - the authentification key
//!
//! @return true if the valid auth key can be found
//
//*****************************************************************************
bool ssbl_otp_get_auth_key(uint32_t authKey, uint8_t *pAuthKey);

//*****************************************************************************
//
//! @brief get the key encryption key
//!
//! @param kek - key encryption key index
//! @param pKey - the key encryption key
//!
//! @return true if key encryption key is valid
//
//*****************************************************************************
bool ssbl_otp_get_kek(uint32_t kek, uint8_t *pKek);

#endif
