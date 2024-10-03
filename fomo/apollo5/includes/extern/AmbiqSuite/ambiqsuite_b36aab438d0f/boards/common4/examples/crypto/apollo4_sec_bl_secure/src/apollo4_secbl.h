//*****************************************************************************
//!
//! @file apollo4_secbl.h
//!
//! @brief the header file of second bootloader main program
//!
//*****************************************************************************

//*****************************************************************************
//
// ${copyright}
//
// This is part of revision ${version} of the AmbiqSuite Development Package.
//
//*****************************************************************************

#ifndef APOLLO4_SECBL_H
#define APOLLO4_SECBL_H

#include "am_mcu_apollo.h"
#include "am_bsp.h"
#include "am_util.h"

#include "apollo4_secbl_otp.h"
#include "apollo4_secbl_image.h"
#include "apollo4_secbl_mram.h"
#include "apollo4_secbl_mspi_psram.h"
#include "apollo4_secbl_secure.h"

#define SECURE_BOOT

#define AUTH_DEFAULT_RSA3072_ALGO     1
#define AUTH_ALGO                     AUTH_DEFAULT_RSA3072_ALGO
#define CRYPT_DEFAULT_AES128          1
#define CRYPT_ALGO                    CRYPT_DEFAULT_AES128

#define MAIN_PROGRAM_ADDR_IN_FLASH    0x80000 // This could be hardcoded, or derived from INFO0 (preferred)

// Customer Proprietary image in internal flash
#define MAGIC_CUST_PROP                 0xC2

// Proprietary Image format to convey Customer Specific secondary OTA descriptor
// This could be used for images resident out of internal flash for example
#define MAGIC_CUST_SECOTADESC           0xC1

// min <= val < max
#define SSBL_RANGE_CHECK(val, min, max)   ((!(min) || ((val) >= (min))) && ((val) < (max)))

typedef struct
{
    // First 4 bytes to be same as am_image_hdr_common_t
    union
    {
        uint32_t     ui32;
        struct
        {
            uint32_t    blobSize     : 22;
            uint32_t    rsvd         : 4;
            uint32_t    crcCheck     : 1;
            uint32_t    enc          : 1;
            uint32_t    authCheck    : 1;
            uint32_t    ccIncluded   : 1;
            uint32_t    ambiq        : 1;
            uint32_t    rsvd1        : 1;
        } s;
    } w0;
    uint32_t ota[AM_HAL_SECURE_OTA_MAX_OTA + 1];
} ssbl_cust_otadesc_t;

static inline bool validate_cust_prop_image(am_image_hdr_common_t *pComHdr, uint32_t **ppVtor)
{
    return false;
}

static inline bool process_cust_prop_ota(uint32_t otaImagePtr)
{
    return false;
}

#define TEMP_BUF_SIZE  AM_HAL_FLASH_PAGE_SIZE

extern uint8_t tempBuf[TEMP_BUF_SIZE];

#endif // APOLLO4_SECBL_H
