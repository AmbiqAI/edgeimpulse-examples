//*****************************************************************************
//
//! @file apollo4_secbl_otp.c
//!
//! @brief OTP access functions
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

#include <string.h>

#include "apollo4_secbl.h"
#include "apollo4_secbl_otp.h"

// Get the Authentication key
// It checks for the key index being revoked, and if not - extracts the corresponding
// key from OTP
bool ssbl_otp_get_auth_key(uint32_t authKey, uint8_t *pAuthKey)
{
#if AUTH_ALGO == AUTH_DEFAULT_RSA3072_ALGO
#if 0
    uint32_t revokeMask;
    // Each 256b HMAC key takes two slots in the key space
    uint32_t keyMask = 0xC0000000;
    if (!SSBL_RANGE_CHECK(authKey, SSBL_MIN_KEYIDX_INFO0, (SSBL_MAX_KEYIDX_INFO0 + 1)) ||
        (authKey & 0x1) // 256b AuthKey needs to be an even number
        )
    {
        return false;
    }
    revokeMask = AM_REGVAL(INFO0_AREVTRACK_ADDR);
    keyMask >>= (authKey - SSBL_MIN_KEYIDX_INFO0);
    if ((revokeMask & keyMask) != keyMask)
    {
        am_util_stdio_printf("Auth Key # %d revoked mask 0x%x!!\n", authKey, revokeMask);
        return false;
    }
#endif
// #### INTERNAL BEGIN ####
    // TODO: Key Wrapping
// #### INTERNAL END ####

    uint32_t cc_addr = AM_REGVAL(AM_REG_INFO0_CERTCHAINPTR_ADDR);
    am_hal_certchain_t *pcc = (am_hal_certchain_t*)cc_addr;

    switch ( authKey )
    {
        case 0:
            memcpy(pAuthKey, (uint8_t*)pcc->pRootCert, SHA384_SIZE);
            break;
        case 2:
            memcpy(pAuthKey, (uint8_t*)pcc->pContentCert, SHA384_SIZE);
            break;
        case 1:
            memcpy(pAuthKey, (uint8_t*)pcc->pKeyCert, SHA384_SIZE);
            break;
        default:
            return false;
    }

    return true;
#else
#error "please return specifc auth keys from OTP here"
#endif
}

// Get the Key Encryption key
// It checks for the key index being revoked, and if not - extracts the corresponding
// key from OTP
bool ssbl_otp_get_kek(uint32_t kek, uint8_t *pKek)
{
#if CRYPT_ALGO == CRYPT_DEFAULT_AES128
#if 0
    uint32_t revokeMask;
    // Each 256b HMAC key takes two slots in the key space
    uint32_t keyMask = 0x80000000;
    if (!SSBL_RANGE_CHECK(kek, SSBL_MIN_KEYIDX_INFO0, (SSBL_MAX_KEYIDX_INFO0 + 1)))
    {
        return false;
    }
    revokeMask = AM_REGVAL(INFO0_KREVTRACK_ADDR);
    keyMask >>= (kek - SSBL_MIN_KEYIDX_INFO0);
    if ((revokeMask & keyMask) != keyMask)
    {
        am_util_stdio_printf("KEK Key # %d revoked mask 0x%x!!\n", kek, revokeMask);
        return false;
    }
#endif
// #### INTERNAL BEGIN ####
    // TODO: Key Wrapping
// #### INTERNAL END ####
    am_hal_security_128bkey_t keyVal;
    if ( kek == 0 )
    {
        for ( uint8_t i = 0; i < 4; i++ )
        {
            am_hal_otp_read_word(AM_REG_OTP_KCP0_O + 4 * i, &keyVal.keyword[i]);
        }
    }
    else if ( kek == 1 )
    {
        for ( uint8_t i = 0; i < 4; i++ )
        {
            am_hal_otp_read_word(AM_REG_OTP_KCE0_O + 4 * i, &keyVal.keyword[i]);
        }
    }
    else
    {
        return false;
    }
    memcpy(pKek, (uint8_t *)&keyVal, AES128_SIZE);
    return true;
#else
#error "please return the specific kek from the OTP here"
#endif
}


