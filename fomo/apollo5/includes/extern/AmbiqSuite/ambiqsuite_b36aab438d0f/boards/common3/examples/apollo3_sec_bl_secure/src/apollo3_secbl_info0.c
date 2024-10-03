//*****************************************************************************
//
//! @file apollo3_secbl_info0.c
//!
//! @brief info0 access functions
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

#include "apollo3_secbl.h"
#include "apollo3_secbl_info0.h"

// Get the Authentication key
// It checks for the key index being revoked, and if not - extracts the corresponding
// key from Info0
bool ssbl_info0_get_auth_key(uint32_t authKey, uint8_t *pAuthKey)
{
#if AUTH_ALGO == AUTH_DEFAULT_SHA256_HMAC_ALGO
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

// #### INTERNAL BEGIN ####
    // TODO: Key Wrapping
// #### INTERNAL END ####
    memcpy(pAuthKey, (uint8_t *)(INFO0_AUTHKEY_ADDR + (authKey - SSBL_MIN_KEYIDX_INFO0) * SSBL_KEYIDX_BYTES), SHA256_SIZE);
    return true;
#else
#error "please return specifc auth keys from info0 here"
#endif
}

// Get the Key Encryption key
// It checks for the key index being revoked, and if not - extracts the corresponding
// key from Info0
bool ssbl_info0_get_kek(uint32_t kek, uint8_t *pKek)
{
#if CRYPT_ALGO == CRYPT_DEFAULT_AES128
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
// #### INTERNAL BEGIN ####
    // TODO: Key Wrapping
// #### INTERNAL END ####
    memcpy(pKek, (uint8_t *)(INFO0_KEK_ADDR + (kek - SSBL_MIN_KEYIDX_INFO0) * SSBL_KEYIDX_BYTES), AES128_SIZE);
    return true;
#else
#error "please return the specific kek from the info0 here"
#endif
}


