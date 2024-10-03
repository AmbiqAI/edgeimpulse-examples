//*****************************************************************************
//
//! @file apollo4_secbl_image.c
//!
//! @brief ota image and boot images' validation functions
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
#include "apollo4_secbl_image.h"
#include "apollo4_secbl_otp.h"

#ifdef SECURE_BOOT

//
// get the OTP authentication enforce information
//
static bool ssbl_get_AuthEnforce(void)
{
    uint32_t ui32ReadVal;
    am_hal_otp_read_word(AM_REG_OTP_SEC_POL_O, &ui32ReadVal);
    return (((ui32ReadVal & AM_REG_OTP_SEC_POL_AUTH_ENFORCE_Msk) >> AM_REG_OTP_SEC_POL_AUTH_ENFORCE_Pos) == ENFORCE_ENABLE);
}

//
// get the OTP encrypt enforce information
//
static bool ssbl_get_EncEnforce(void)
{
    uint32_t ui32ReadVal;
    am_hal_otp_read_word(AM_REG_OTP_SEC_POL_O, &ui32ReadVal);
    return (((ui32ReadVal & AM_REG_OTP_SEC_POL_ENC_ENFORCE_Msk) >> AM_REG_OTP_SEC_POL_ENC_ENFORCE_Pos) == ENFORCE_ENABLE);
}

//
// Boot image validation function
//
bool ssbl_validate_boot_image(am_thirdparty_image_hdr_t *pComHdr, am_ssbl_flash_t *pFlash)
{
    uint8_t authKey[DIGEST_SIZE];
    uint8_t digest[DIGEST_SIZE];
    uint32_t crc;

    bool bAuthEnforce = ssbl_get_AuthEnforce();

    if (bAuthEnforce && !pComHdr->common.w0.s.authCheck)
    {
        // Security policy violation
        am_util_stdio_printf("No Boot Authentication for image\n");
        return false;
    }

    // Do signature verification & CRC, if enabled
    if (pComHdr->common.w0.s.authCheck)
    {
        // Check Auth Key
        if ((pComHdr->common.w2.s.authAlgo != AUTH_ALGO) || !ssbl_otp_get_auth_key(pComHdr->common.w2.s.authKeyIdx, authKey))
        {
            return false;
        }
        if ( !ssbl_auth_verify(authKey, (uint32_t)pComHdr + offsetof(am_thirdparty_image_hdr_t, enc_info),
                               pComHdr->common.w0.s.blobSize - offsetof(am_thirdparty_image_hdr_t, enc_info), digest, pComHdr->auth_info.ui8Signature))
        {
            am_util_stdio_printf("Boot Authentication failed for image\n");
            return false;
        }
    }

    if (pComHdr->common.w0.s.crcCheck)
    {
        uint32_t crcBytes;
        uint32_t crcSize = pComHdr->common.w0.s.blobSize - offsetof(am_image_hdr_common_t, w2);
        uint8_t  *pCrcStartAddr = (uint8_t *)&(pComHdr->common.w2);

        crc = 0;

        if (pFlash->bInternal)
        {
            if (am_hal_crc32((uint32_t)pCrcStartAddr, crcSize, &crc) != AM_HAL_STATUS_SUCCESS)
            {
                am_util_stdio_printf("CRC calculation failed for image\n");
                return false;
            }
        }
        else
        {
            while (crcSize > 0)
            {
                crcBytes = crcSize > TEMP_BUF_SIZE ? TEMP_BUF_SIZE : crcSize;
                memcpy(tempBuf, pCrcStartAddr, crcBytes);
                if (am_hal_crc32((uint32_t)tempBuf, crcBytes, &crc) != AM_HAL_STATUS_SUCCESS)
                {
                    am_util_stdio_printf("CRC calculation failed for image\n");
                    return false;
                }
                crcSize -= crcBytes;
                pCrcStartAddr += crcBytes;
            }
        }

        if (crc != pComHdr->common.crc)
        {
            am_util_stdio_printf("CRC verification failed for image\n");
            return false;
        }
    }
    return true;
}

#endif

//
// Ota image validation function
// If Encrypted, it populates the pDecryptInfo with required information for decryption later
// Also - it copies the first 256B of the image in clear to the local SRAM provided (pMainHdr)
bool ssbl_validate_ota_image(uint32_t ui32OtaImageAddr, am_ssbl_flash_t *pFlash, ssbl_decrypt_info_t *pDecryptInfo, am_main_image_hdr_t *pMainHdr)
{
    uint8_t authKey[DIGEST_SIZE];
    uint8_t digest[DIGEST_SIZE];
    uint8_t iv[CRYPT_SIZE];
    uint32_t crc;

    // Need to enforce security policy
    // e.g. this is where one can mandate authentication
    bool bAuthEnforce = ssbl_get_AuthEnforce();
    bool bEncEnforce = ssbl_get_EncEnforce();
    am_thirdparty_image_hdr_t *pComHdr = (am_thirdparty_image_hdr_t *)pMainHdr;

    // Read the headers
    if (pFlash->flash_read((uint8_t *)pComHdr, ui32OtaImageAddr, sizeof(*pMainHdr)) != AM_HAL_STATUS_SUCCESS)
    {
        return false;
    }

#if 0
    if (bAuthEnforce && !pComHdr->w2.s.authInstall)
    {
        // Security policy violation
        am_util_stdio_printf("No Install Authentication for image\n");
        return false;
    }
#endif
#ifdef SECURE_BOOT
    if (bAuthEnforce && !pComHdr->common.w0.s.authCheck)
    {
        // Security policy violation
        am_util_stdio_printf("No Boot Authentication for image\n");
        return false;
    }
#endif

    if (bEncEnforce && !pComHdr->common.w0.s.enc)
    {
        // Security policy violation
        am_util_stdio_printf("No Encryption for image\n");
        return false;
    }

    // If enabled - check for install authentication
    // Thereafter check for boot authentication & CRC after decryption
//    if (pComHdr->w2.s.authInstall || pComHdr->common.w0.s.authCheck)
    {
        // Check Auth Key
        if ((pComHdr->common.w2.s.authAlgo != AUTH_ALGO) || !ssbl_otp_get_auth_key(pComHdr->common.w2.s.authKeyIdx, authKey))
        {
            return false;
        }
    }

//    if (pComHdr->w2.s.authInstall)
    {
        // Signature is computed over rest of the image

        if ( !ssbl_auth_verify(authKey, (uint32_t)pComHdr + offsetof(am_thirdparty_image_hdr_t, enc_info),
                    pComHdr->common.w0.s.blobSize - offsetof(am_thirdparty_image_hdr_t, enc_info), digest, pComHdr->auth_info.ui8Signature) )
        {
            am_util_stdio_printf("Install Authentication failed for image\n");
            return false;
        }
    }

    if (!pComHdr->common.w0.s.crcCheck && !pComHdr->common.w0.s.authCheck)
    {
        // Nothing more needed
        return true;
    }

    // Now we know we need to iterate over whole blob
//    if (pComHdr->w0.s.encrypted)
    {
        uint8_t kek[CRYPT_SIZE];
        if ((pComHdr->common.w2.s.encAlgo != CRYPT_ALGO) || !ssbl_otp_get_kek(pComHdr->common.w2.s.encKeyIdx, kek))
        {
            return false;
        }
        // Decrypt the key first
        memset(iv, 0, CRYPT_SIZE);
        ssbl_crypt_init((uint8_t *)kek);
        ssbl_crypt_decrypt((uint8_t *)&pComHdr->enc_info, pDecryptInfo->key, CRYPT_SIZE, (uint8_t *)iv);
        // Now decrypt the image
        pDecryptInfo->bDecrypt = true;
        pDecryptInfo->clearSize =
            (pComHdr->opt_info.opt0.s.magicNum == AM_IMAGE_MAGIC_NONSECURE) ? 0 : offsetof(am_thirdparty_image_hdr_t, auth_info);

        memcpy(iv, IMG_CRYPT_IV(pComHdr), CRYPT_SIZE);
        memcpy(pDecryptInfo->iv, IMG_CRYPT_IV(pComHdr), CRYPT_SIZE);
        ssbl_crypt_init(pDecryptInfo->key);
        // Do in place decryption
        ssbl_crypt_decrypt(pComHdr->auth_info.ui8Signature, pComHdr->auth_info.ui8Signature,
                     sizeof (*pMainHdr) - offsetof(am_thirdparty_image_hdr_t, auth_info), iv);
    }

    if (pComHdr->common.w0.s.crcCheck)
    {
        am_hal_crc32((uint32_t)&(pComHdr->common.w2), (sizeof (*pMainHdr) - offsetof(am_image_hdr_common_t, w2)), &crc);
    }

    if (pComHdr->common.w0.s.crcCheck & (crc != pComHdr->common.crc))
    {
        am_util_stdio_printf("CRC verification failed for image\n");
        return false;
    }

    return true;
}


#ifdef SECURE_BOOT

//
// Ensure the child images referenced for the main image are valid
//
bool ssbl_validate_child_images(uint32_t *pChildPtr)
{
    uint32_t numChild = 0;
    am_ssbl_flash_t *pFlash;
    // Check for child images
    while ((numChild < AM_IMAGE_MAX_CHILD_IMAGE) && (*pChildPtr != 0xFFFFFFFF))
    {
        am_thirdparty_image_hdr_t *pHdr = (am_thirdparty_image_hdr_t *)pChildPtr;

        if (!ssbl_find_flash_of_image(*pChildPtr, pHdr->common.w0.s.blobSize, &pFlash) || !pFlash->bXip)
        {
            am_util_stdio_printf("Invalid child image\n");
            return false;
        }
        if (!ssbl_validate_boot_image(pHdr, pFlash))
        {
            am_util_stdio_printf("Validation failed for child image\n");
            return false;
        }
        numChild++;
        pChildPtr++;
    }
    return true;
}
#endif

//
// Verify the main image & any linked child images
// Also applies necessary protections on successful validation
// Returns the Start of the image
//
uint32_t ssbl_validate_main_image(uint32_t **ppVtor)
{
    uint32_t imageAddr;
    uint32_t sp;
    uint32_t reset;
    am_ssbl_flash_t *pFlash;
    am_thirdparty_image_hdr_t *pComHdr;

    imageAddr = MAIN_PROGRAM_ADDR_IN_FLASH;
    pComHdr = (am_thirdparty_image_hdr_t *)imageAddr;

    if (!ssbl_find_flash_of_image(imageAddr, 0x0, &pFlash) || !pFlash->bXip)
    {
        am_util_stdio_printf("Invalid main image\n");
        return AM_HAL_STATUS_FAIL;
    }

    //
    // Enable the flash XIP mode
    //
    if (pFlash->bXip && pFlash->flash_enable_xip)
    {
        if (pFlash->flash_enable_xip() != AM_HAL_STATUS_SUCCESS)
        {
            return AM_HAL_STATUS_FAIL;
        }
    }

    *ppVtor = 0;

#ifndef SECURE_BOOT
    reset = *((uint32_t *)(imageAddr + 4));
    if (!ssbl_find_flash_of_image(imageAddr, (reset - imageAddr + 4), &pFlash))
    {
        am_util_stdio_printf("Invalid main image\n");
        return AM_HAL_STATUS_FAIL;
    }
#else
    if (!ssbl_validate_boot_image(pComHdr, pFlash))
    {
        am_util_stdio_printf("Validation failed for main image\n");
        return AM_HAL_STATUS_FAIL;
    }
    // Check for child images
    am_main_image_hdr_t *pMainHdr = (am_main_image_hdr_t *)imageAddr;
    uint32_t *pChildPtr = pMainHdr->childPtr;
    uint32_t numChild = 0;
    while ((numChild < AM_IMAGE_MAX_CHILD_IMAGE) && (*pChildPtr != 0xFFFFFFFF))
    {
        am_thirdparty_image_hdr_t *pHdr = (am_thirdparty_image_hdr_t *)pChildPtr;

        if (!ssbl_find_flash_of_image((uint32_t)pChildPtr, pHdr->common.w0.s.blobSize, &pFlash) || !pFlash->bXip)
        {
            am_util_stdio_printf("Invalid child image\n");
            return AM_HAL_STATUS_FAIL;
        }

        if (!ssbl_validate_boot_image(pHdr, pFlash))
        {
            am_util_stdio_printf("Validation failed for child image\n");
            return AM_HAL_STATUS_FAIL;
        }

        numChild++;
        pChildPtr++;
    }
    imageAddr = (uint32_t)(pMainHdr + 1);
    reset = *((uint32_t *)(imageAddr + 4));
    if ((reset + 4) > (imageAddr + pComHdr->common.w0.s.blobSize - sizeof(*pMainHdr)))
    {
        am_util_stdio_printf("Invalid main image\n");
        return AM_HAL_STATUS_FAIL;
    }

#endif

    sp = *((uint32_t *)imageAddr);
    //
    // Make sure the SP & Reset vector are sane
    // Validate the Stack Pointer
    // Validate the reset vector
    //
    if ((sp < SRAM_BASEADDR) || \
        (sp >= (SRAM_BASEADDR + SRAM_SIZE)))
    {
        am_util_stdio_printf("Invalid main image\n");
        return AM_HAL_STATUS_FAIL;
    }
    else
    {
        am_util_stdio_printf("Found valid main image - SP:0x%x RV:0x%x\n", sp, reset);
        *ppVtor = (uint32_t *)imageAddr;
#ifdef SECURE_BOOT
        // Apply protections
        if (pFlash->flash_write_protect)
        {
            // Write protect
            pFlash->flash_write_protect(imageAddr, pComHdr->common.w0.s.blobSize);
        }
        if (pFlash->flash_copy_protect)
        {
            // Copy protect
            pFlash->flash_copy_protect(imageAddr, pComHdr->common.w0.s.blobSize);
        }
        // Apply protections for Child images
        pChildPtr = pMainHdr->childPtr;
        while (numChild--)
        {
            am_image_hdr_common_t *pComHdr = (am_image_hdr_common_t *)pChildPtr;

            // Apply protections
            if (pFlash->flash_write_protect)
            {
                // Write protect
                pFlash->flash_write_protect((uint32_t)pComHdr, pComHdr->w0.s.blobSize);
            }
            if (pFlash->flash_copy_protect)
            {
                // Copy protect
                pFlash->flash_copy_protect((uint32_t)pComHdr, pComHdr->w0.s.blobSize);
            }
            pChildPtr++;
        }
#endif
        am_util_stdio_printf("Will transfer control over to this image after locking things down\n\n\n\n");

        return AM_HAL_STATUS_SUCCESS;
    }
}
