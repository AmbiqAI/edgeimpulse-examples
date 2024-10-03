//*****************************************************************************
//!
//! @file apollo4_secbl_image.h
//!
//! @brief the header file of secondary bootloader image validation functions
//!
//*****************************************************************************


//*****************************************************************************
//
// ${copyright}
//
// This is part of revision ${version} of the AmbiqSuite Development Package.
//
//*****************************************************************************

#ifndef APOLLO4_SECBL_IMAGE_H
#define APOLLO4_SECBL_IMAGE_H

#include "apollo4_secbl_secure.h"
#include "apollo4_secbl_mram.h"

#define SRAM_SIZE AM_HAL_MRAM_SRAM_SIZE

#define AM_IMAGE_NUM_TRAILING_WORDS_TO_1K    ((1024 - sizeof(am_thirdparty_image_hdr_t))/4)
#define AM_IMAGE_MAX_CHILD_IMAGE             AM_IMAGE_NUM_TRAILING_WORDS_TO_1K

typedef struct
{
    am_image_hdr_common_t common;
    am_image_auth_info_t auth_info;
    am_image_enc_info_t enc_info;
    am_image_opt_info_t opt_info;
} am_thirdparty_image_hdr_t;

typedef struct
{
    am_thirdparty_image_hdr_t common;
    uint32_t        childPtr[AM_IMAGE_MAX_CHILD_IMAGE];
} am_main_image_hdr_t;

#ifdef SECURE_BOOT
//*****************************************************************************
//
//! @brief verify the boot image
//!
//! @param pComHdr - the pointer that holds the image header information
//! @param pFlash - the flash driver instance that the image resides
//!
//! @return ture if the image is valid
//
//*****************************************************************************
bool ssbl_validate_boot_image(am_thirdparty_image_hdr_t *pComHdr, am_ssbl_flash_t *pFlash);
#endif

//*****************************************************************************
//
//! @brief verify the ota image
//!
//! @param ui32OtaImageAddr - the ota image start address
//! @param pFlash - the flash driver instance that the image resides
//! @param pDecryptInfo - the information that decryption algorithm needs
//! @param pMainHdr - the pointer that holds the main image header information
//!
//! @return true if the ota image is valid
//
//*****************************************************************************
bool ssbl_validate_ota_image(uint32_t ui32OtaImageAddr, am_ssbl_flash_t *pFlash, ssbl_decrypt_info_t *pDecryptInfo, am_main_image_hdr_t *pMainHdr);
//*****************************************************************************
//
//! @brief verify the child image
//!
//! @param pChildPtr - the child image start address
//!
//! @return true if the child image is valid
//
//*****************************************************************************
bool ssbl_validate_child_images(uint32_t *pChildPtr);

//*****************************************************************************
//
//! @brief verify the main image
//!
//! @param ppVtor - hold the address that bootloader can jump to
//!
//!  Verify the main image & any linked child images
//!  Also applies necessary protections on successful validation
//!  Returns the Start of the image
//!
//! @return AM_HAL_STATUS_SUCCESS if all images are valid
//
//*****************************************************************************
uint32_t ssbl_validate_main_image(uint32_t **ppVtor);

#endif
