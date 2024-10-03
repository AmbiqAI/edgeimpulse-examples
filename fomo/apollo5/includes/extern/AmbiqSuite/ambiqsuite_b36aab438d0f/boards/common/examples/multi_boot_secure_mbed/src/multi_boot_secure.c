//*****************************************************************************
//
//! @file multi_boot_secure.c
//!
//! @brief Secure Bootloader Function hooks - sample implementation
//! This file contains a sample implementation of the customizable secure boot
//! function hooks
//!
//! It implements an AES-128 based encryption scheme in CBC mode for image
//! confidentiality. The Clear image CRC is used as an image signature to ensure
//! image authentication and integrity.
//! Optionally (compile time switch HMAC_SECTRAILER), the NEW_IMAGE
//! message can be authenticated before taking any action related to new image.
//! An HMAC using SHA256 is computed to authenticate the security trailer.
//!
//! This is just a sample implementation to serve as a reference and is supplied
//! on "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND. It uses a
//! 3rd party implementation from ARM (mbed) for cryptographic functions.
//!
//! Key Size is 128 Bits, determined based on keyidx from a key bank
//!
//! Encryption:
//! CipherText[i] = AES-128((PlainText[i] ^ IV[i]), Key)
//! IV[i+1] = CipherText[i]
//!
//! Decryption:
//! PlainText[i] = (AES-128(CipherText[i], Key) ^ IV[i])
//! IV[i+1] = CipherText[i]
//!
//! IV[0] is shared between Encryption and Decryption (in clear)
//!
//! Image Signature is computed as ClearCRC
//!
//! Optional Signature HMAC is computed over multiboot_security_info_t part of
//! the trailer, using SHA256, and the same key as used for AES
//
//*****************************************************************************


//*****************************************************************************
//
// ${copyright}
//
// This is part of revision ${version} of the AmbiqSuite Development Package.
//
//*****************************************************************************

#include "am_mcu_apollo.h"
#include "am_bsp.h"
#include "am_util.h"
#include "am_bootloader.h"
#include "am_multi_boot.h"
#include "am_multi_boot_config.h"

#include "am_multi_boot_secure.h"


#define MAX_KEYS                8

#define HMAC_SECTRAILER

#ifdef HMAC_SECTRAILER
// Security Trailer Signature uses HMAC
#include "mbedtls/md.h"
#define SHA_SIZE                32 // 256b for SHA-256
#define SIGNATURE_SIZE          SHA_SIZE
#endif

#include "mbedtls/aes.h"
#define KEY_SIZE                16 // 16 bytes = 128b
#define IV_SIZE                 16 // 16 bytes = 128b

#include "mbedtls/memory_buffer_alloc.h"

// Test only - key index 6 is invalid
uint32_t g_ui32KeyValid             = 0xFDFFFFFF;

// Location in flash/infospace where the key valid mask is maintained
// Set to a known value in the data segment just for test purpose
#define KEY_VALID_MASK_LOCATION     &g_ui32KeyValid

typedef struct
{
    // Area Authenticated by Signature - Begin
    uint32_t protection;
    uint32_t *pui32LinkAddress;
    uint32_t imageLength;
    // For authentication & integrity validation of the image
    uint32_t clearCrc;
    uint8_t initVector[IV_SIZE];
    // Area Authenticated by Signature - End
} multiboot_security_info_t;

// This is the security trailer definition for this sample implementation
// This would be the trailing part of AM_BOOTLOADER_NEW_IMAGE message
typedef struct
{
    uint32_t keyIdx;
    multiboot_security_info_t secInfo;
#ifdef HMAC_SECTRAILER
    // Validate this message itself
    uint8_t signature[SIGNATURE_SIZE];
#endif
} multiboot_security_trailer_t;

typedef struct
{
    uint32_t version;
    uint32_t offset;
    uint32_t expectedClearCrc;
    uint32_t clearCRC;
    uint8_t initVector[IV_SIZE];
    uint8_t key[KEY_SIZE];
} multiboot_secure_state_t;

static multiboot_secure_state_t g_sSecState;

static uint8_t keyTbl[MAX_KEYS][KEY_SIZE] =
{
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF},
    {0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA},
    {0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55},
    {0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11},
    {0xA5, 0xA5, 0xA5, 0xA5, 0xA5, 0xA5, 0xA5, 0xA5, 0xA5, 0xA5, 0xA5, 0xA5, 0xA5, 0xA5, 0xA5, 0xA5},
    {0x66, 0x66, 0x66, 0x66, 0x66, 0x66, 0x66, 0x66, 0x66, 0x66, 0x66, 0x66, 0x66, 0x66, 0x66, 0x66},
    {0xDE, 0xAD, 0xBE, 0xEF, 0xDE, 0xAD, 0xBE, 0xEF, 0xDE, 0xAD, 0xBE, 0xEF, 0xDE, 0xAD, 0xBE, 0xEF},
};

mbedtls_aes_context ctxAes;

static void multiboot_decrypt(uint8_t * data, uint32_t size, uint8_t *key, uint32_t key_size, uint8_t * iv)
{
    mbedtls_aes_crypt_cbc( &ctxAes, MBEDTLS_AES_DECRYPT, size, iv, (const uint8_t *)data, data);
}

#if 0 // Not used - included here just for reference
static void multiboot_encrypt(uint8_t * data, uint32_t size8, uint8_t *key, uint32_t key_size, uint8_t * iv)
{
    mbedtls_aes_crypt_cbc( &ctxAes, MBEDTLS_AES_ENCRYPT, size, iv, (const uint8_t *)data, data);
}
#endif

static bool
validate_keyidx(uint32_t keyIdx)
{
    if ( keyIdx > MAX_KEYS )
    {
        return true;
    }
    // Validate the key index to make sure it has not been revoked
    return am_bootloader_check_index(keyIdx, (uint32_t *)KEY_VALID_MASK_LOCATION);
}

static bool
validate_sec_trailer(multiboot_security_trailer_t *pTrailer)
{
#ifdef HMAC_SECTRAILER
    // Check the signature with computed Value - for authentication
    // Optionally check the version for replay protection
    // Return 0 for success
    // Check for image signature to validate authenticity (and integrity)
    int i;
    uint8_t digest[SIGNATURE_SIZE];
    const mbedtls_md_info_t *md5_hmac = mbedtls_md_info_from_type(MBEDTLS_MD_SHA256);
    mbedtls_md_hmac(md5_hmac, keyTbl[pTrailer->keyIdx], KEY_SIZE, (const uint8_t *)&pTrailer->secInfo, sizeof(pTrailer->secInfo), digest);

    for (i = 0; i < SIGNATURE_SIZE; i++)
    {
        if (digest[i] != pTrailer->signature[i])
        {
            return true;
        }
    }
#endif
    // Validate signature of Seurity trailer
    return false;
}

// The buffer needed for mbed heap
// This has been sized to just meet the needs of this program
unsigned char g_mbed_buf[512];
// Multiboot function hooks - sample implementation

// Verify the security trailer & initialize the security params
int
init_multiboot_secure(uint32_t length, uint32_t *pData,
                      bool bStoreInSram, am_bootloader_image_t *psImage,
                      uint32_t *pProtect)
{
    int i;
    multiboot_security_trailer_t *pSec = (multiboot_security_trailer_t *)pData;
    // Verify the length
    // Need to allow the length to be more, as could be the case for OTA with padding
    if ( length < sizeof(*pSec) )
    {
        return -1;
    }
    // Validate key index
    if ( validate_keyidx(pSec->keyIdx) )
    {
        return -1;
    }
    mbedtls_memory_buffer_alloc_init(g_mbed_buf, sizeof(g_mbed_buf));

    // Now Authenticate the security trailer itself
    if ( validate_sec_trailer(pSec) )
    {
        return -1;
    }
    // We could reject the download if we can not validate if before flashing
    // by checking bStoreInSram herea - optional
    // Initialize State Variables
    if ( pSec->secInfo.imageLength != psImage->ui32NumBytes )
    {
        return -1;
    }
    if ( pSec->secInfo.pui32LinkAddress != psImage->pui32LinkAddress )
    {
        return -1;
    }
    g_sSecState.clearCRC = 0;
    g_sSecState.expectedClearCrc = pSec->secInfo.clearCrc;
    g_sSecState.offset = 0;
    for ( i = 0; i < KEY_SIZE; i++ )
    {
        g_sSecState.key[i] = keyTbl[pSec->keyIdx][i];
    }
    for ( i = 0; i < IV_SIZE; i++ )
    {
        // Initialize IV
        g_sSecState.initVector[i] = pSec->secInfo.initVector[i];
    }
    mbedtls_aes_setkey_dec( &ctxAes, (const uint8_t *)g_sSecState.key, KEY_SIZE*8);
    *pProtect = pSec->secInfo.protection; // Could be set if copy-protection is desired
    return 0;
}

void
multiboot_secure_decrypt(void *pData, uint32_t ui32NumBytes)
{
    // Do in place decryption
    // With knowledge of image structure, this can also grab the image version
    // from a fixed location from within image
    multiboot_decrypt((uint8_t *)pData, ui32NumBytes,
        g_sSecState.key, KEY_SIZE, g_sSecState.initVector);
    // Update the running CRC for clear image
    am_bootloader_partial_crc32(pData, ui32NumBytes, &g_sSecState.clearCRC);
    g_sSecState.offset += ui32NumBytes;
}

// Verify the authenticity and integrity of the Image
// return the clear CRC for integrity verification going forward
int
multiboot_secure_verify(uint32_t *pui32ClearCRC)
{
    if ( g_sSecState.clearCRC != g_sSecState.expectedClearCrc )
    {
        return -1;
    }
    // Update the CRC in psImage for integrity verification going forward
    *pui32ClearCRC = g_sSecState.clearCRC;
    return 0;
}

