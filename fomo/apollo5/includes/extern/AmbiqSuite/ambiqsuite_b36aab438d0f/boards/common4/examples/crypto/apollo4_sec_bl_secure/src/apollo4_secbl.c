//*****************************************************************************
//
//! @file apollo4_secbl.c
//!
//! @brief A simple secondary bootloader program example for Apollo4
//!
//! Purpose: This program is an example template for a secondary bootloader program for Apollo4.
//! It demonstrates how to access OTP key area. It demonstrates how to use the Ambiq SBL OTA
//! framework for customer specific OTAs, e.g. to support external psram, or to support more
//! advanced auth/enc schemes. It demonstrates how to validate & transfer control to the real
//! main program image (assumed to be at address specified by MAIN_PROGRAM_ADDR_IN_FLASH in mram)
//! after locking the OTP area before exiting
//!
//! Printing takes place over the ITM at 1M Baud.
//!
//! Additional Information:
//! To exercise this program:
//! Flash the main program at 0x80000  if executing in the internal mram
//! Flash the main program at 0x40000000 if excuting in the MSPI1 XIP PSRAM
//! Link this program at the address suitable for SBL nonsecure (0xC000) or secure (0xC100)
//! configuration
//! To test OTA - construct images using magic numbers in the range matching
//! AM_IMAGE_MAGIC_CUST
//! To test OTP key area access - need to keep OTP->Security->PLONEXIT as 0
//! apollo4_evb
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

//! Bitmasks signifying the bit to be cleared for OTA success/failure
#define OTA_DONE_FAILURE_CLRMASK      0x1
#define OTA_DONE_SUCCESS_CLRMASK      0x3

//! Number of most significant bits in w113 used for load address
#define AM_IMAGE_LOAD_ADDR_MSB_BITS           30
#define AM_IMAGE_GET_LOADADDR(opt1)         (((am_image_opt1_t *)(opt1))->fw.loadAddrMsb << (32 - AM_IMAGE_LOAD_ADDR_MSB_BITS))

uint8_t tempBuf[TEMP_BUF_SIZE];

//
// Program an image from one place to another - doing optional decryption along with
// The function expects the image to be in the common format defined by Ambiq
//
bool
install_ota_image(am_ssbl_flash_t *pSrcFlash, am_ssbl_flash_t *pDstFlash, uint32_t pSrc, uint32_t pDst, uint32_t ui32NumBytes, ssbl_decrypt_info_t *pDecryptInfo)
{
    // clearSize = offsetof(am_thirdparty_image_hdr_t, signatureClear)
    uint32_t clearSize = pDecryptInfo->clearSize;

    //
    // Erase the entire destination flash area firstly
    //
    if (!pDstFlash->flash_erase || pDstFlash->flash_erase(pDst, ui32NumBytes))
    {
        return false;
    }

    if (pDecryptInfo->bDecrypt)
    {
        ssbl_crypt_init(pDecryptInfo->key);
    }

    while (ui32NumBytes)
    {
        uint32_t blockSize = (ui32NumBytes > sizeof(tempBuf)) ? sizeof(tempBuf): ui32NumBytes;
        if (pSrcFlash->flash_read(tempBuf, pSrc, blockSize))
        {
            return false;
        }

        if (pDecryptInfo->bDecrypt)
        {
            // Do in place decryption
            ssbl_crypt_decrypt((uint8_t *)tempBuf + clearSize, (uint8_t *)tempBuf + clearSize,
                         blockSize - clearSize, pDecryptInfo->iv);
            clearSize = 0;
        }

        if (pDstFlash->flash_write(tempBuf, pDst, blockSize))
        {
            return false;
        }

        pSrc += sizeof(tempBuf);
        pDst += sizeof(tempBuf);
        ui32NumBytes -= blockSize;
    }

    return true;
}

//
// Process OTA descriptor list
//
bool
process_ota_desc_list(uint32_t *pOtaDesc)
{
    static uint32_t ui32RecursiveCount = 0;
    uint32_t i = 0;
    bool bOtaSuccess = false;
    bool bNoRollback = false;
    am_thirdparty_image_hdr_t *pComHdr, ComHdr;
    am_main_image_hdr_t mainHdr;
    am_ssbl_flash_t *pSrcFlash;
    am_ssbl_flash_t *pDstFlash;
    ssbl_decrypt_info_t decryptInfo;
    uint32_t size = 0;
    uint32_t pSrc;
    uint32_t pDst;

    //
    // Prevent potential dead loop by wrong ota list or attack
    //
    if (ui32RecursiveCount++ > 1)
    {
        return false;
    }

    uint32_t otaImagePtr = pOtaDesc[0];

    while (otaImagePtr != AM_HAL_SECURE_OTA_OTA_LIST_END_MARKER)
    {
        if (!AM_HAL_OTA_IS_VALID(otaImagePtr))
        {
            // This OTA has already been invalidated...Skip
            goto move_to_next_ota_image;
        }

        //
        // This template assumes OTA images using the same formal as used by Main.
        // Users can select any image format - as long as the first byte (magic#) is kept the same.
        //
        otaImagePtr = AM_HAL_OTA_GET_BLOB_PTR(otaImagePtr);

        //
        // Only check if the image header is within the flash
        // since we don't know the length of image blob yet
        //
        if (!ssbl_find_flash_of_image(otaImagePtr, sizeof(am_thirdparty_image_hdr_t), &pSrcFlash))
        {
            goto invalidate_ota_image;
        }

        pComHdr = &ComHdr;

        if (pSrcFlash->flash_read((uint8_t *)pComHdr, otaImagePtr, sizeof(am_thirdparty_image_hdr_t)) != AM_HAL_STATUS_SUCCESS)
        {
            goto invalidate_ota_image;
        }

        //
        // Check if whole image blob is within the flash
        //
        if (!ssbl_find_flash_of_image(otaImagePtr, pComHdr->common.w0.s.blobSize, &pSrcFlash))
        {
            // Invalidate this OTA for subsequent processing
            am_util_stdio_printf("Found bad OTA pointing to: image address=0x%x, size 0x%x\n", otaImagePtr, pComHdr->common.w0.s.blobSize);
            goto invalidate_ota_image;
        }

        memset((void *)&decryptInfo, 0x0, sizeof(decryptInfo));

        switch (pComHdr->opt_info.opt0.s.magicNum)
        {
           case MAGIC_CUST_PROP:
                // customer specific image formatting and processing
                bOtaSuccess = process_cust_prop_ota(otaImagePtr);
                break;

            case MAGIC_CUST_SECOTADESC:
                // Special case
                // The OTA image blob(s) is in external memory
                // This OTA blob just contains a list of the images
                am_util_stdio_printf("Cust Specific OTA Available - OTA Desc @0x%x\n", otaImagePtr);
                bOtaSuccess = process_ota_desc_list(&((ssbl_cust_otadesc_t *)otaImagePtr)->ota[0]);
                break;

#ifndef SECURE_BOOT
            case AM_IMAGE_MAGIC_NONSECURE:
                // We only install the blob inside
                size = pComHdr->common.w0.s.blobSize - sizeof(am_thirdparty_image_hdr_t);
                pSrc = otaImagePtr + sizeof(am_thirdparty_image_hdr_t);

                //
                // Process OTA
                am_util_stdio_printf("Found OTA @ 0x%x magic 0x%x - size 0x%x\n", otaImagePtr, pComHdr->opt_info.opt0.s.magicNum, size);
                // We can perform any necessary verification/decryption here before installing a valid OTA image
                bOtaSuccess = ssbl_validate_ota_image(otaImagePtr, pSrcFlash, &decryptInfo, &mainHdr);
                break;
#else
            case AM_IMAGE_MAGIC_OEM_CHAIN:
                // We install the whole blob
                size = pComHdr->common.w0.s.blobSize;
                pSrc = otaImagePtr;

                //
                // Process OTA
                am_util_stdio_printf("Found OTA @ 0x%x magic 0x%x - size 0x%x\n", otaImagePtr, pComHdr->opt_info.opt0.s.magicNum, size);
                // We can perform any necessary verification/decryption here before installing a valid OTA image
                bOtaSuccess = ssbl_validate_ota_image(otaImagePtr, pSrcFlash, &decryptInfo, &mainHdr);
                break;
            case AM_IMAGE_MAGIC_SECURE:
                // We install the whole blob
                size = pComHdr->common.w0.s.blobSize;
                pSrc = otaImagePtr;

                //
                // Process OTA
                am_util_stdio_printf("Found OTA @ 0x%x magic 0x%x - size 0x%x\n", otaImagePtr, pComHdr->opt_info.opt0.s.magicNum, size);
                // We can perform any necessary verification/decryption here before installing a valid OTA image
                if (ssbl_validate_ota_image(otaImagePtr, pSrcFlash, &decryptInfo, &mainHdr))
                {
                    // Need to validate all the child images as well
                    bOtaSuccess = ssbl_validate_child_images(mainHdr.childPtr);
                }
                break;
#endif
        }

        if (!bOtaSuccess || !size )
        {
            goto invalidate_ota_image;
        }

        pDst = AM_IMAGE_GET_LOADADDR(&mainHdr);
        am_util_stdio_printf("To be installed at 0x%x\n", pDst);

        //
        // Install ota image
        //
        if (!ssbl_find_flash_of_image(pDst, size, &pDstFlash) || !pDstFlash->bXip)
        {
            bOtaSuccess = false;
            goto invalidate_ota_image;
        }

#ifdef SECURE_BOOT
        bNoRollback = ssbl_otp_get_NoRollback();
        if (bNoRollback)
        {
            am_thirdparty_image_hdr_t curImage;

            if (pDstFlash->flash_read((void *)&curImage, pDst, sizeof(curImage)) != AM_HAL_STATUS_SUCCESS)
            {
                goto invalidate_ota_image;
            }

            if (curImage.opt_info.opt0.s.magicNum == mainHdr.common.opt_info.opt0.s.magicNum)
            {
                // We assume that in this case there is an existing image - on which version check can be performed
#if 0 //TODO/Need to write new version get API
                if (curImage.versionKeyWord.s.version >= mainHdr.common.versionKeyWord.s.version)
                {
                    am_util_stdio_printf("Version rollback check failed\n");
                    bOtaSuccess = false;
                    goto invalidate_ota_image;
                }
#endif
            }
        }
#endif

        if (!install_ota_image(pSrcFlash, pDstFlash, pSrc, pDst, size, &decryptInfo))
        {
            am_util_stdio_printf("Failed to install OTA @ 0x%x\n", pDst);
            bOtaSuccess = false;
        }
        else
        {
            am_util_stdio_printf("Successfully installed OTA @ 0x%x\n", pDst);
        }

invalidate_ota_image:
        // Provide OTA Feedback to application
        pOtaDesc[i] &= ~(bOtaSuccess ? OTA_DONE_SUCCESS_CLRMASK : OTA_DONE_FAILURE_CLRMASK);

move_to_next_ota_image:
        if (i++ == AM_HAL_SECURE_OTA_MAX_OTA)
        {
            am_util_stdio_printf("Exceeded maximum OTAs\n", i);
            return false;
        }
        otaImagePtr = pOtaDesc[i];
    }

    return true;
}

void dump_info_keys(void)
{
#ifdef DUMP_INFO0_KEYS
    //
    // If OTP->Security->PLONEXIT was set, SBL will keep OTP open upon transferring to main image
    // This allows the secondary bootloader to use the keys in OTP to perform any necessary validations
    // of images and/or OTA upgrades
    //
    // The code segment below is just demonstrating a way to access those keys
    //
    uint32_t ui32Security = 0;
    am_hal_otp_read_word(AM_REG_OTP_SECURITY_O, &ui32Security);
    if (ui32Security & AM_REG_OTP_SECURITY_PLONEXIT_Msk)
    {
        am_hal_security_128bkey_t keyVal;
        uint8_t i = 0;
        for ( i = 0; i < 4; i++ )
        {
            am_hal_otp_read_word(AM_REG_OTP_WRAP_KEY0_O + 4 * i, &keyVal.keyword[i]);
        }
        //
        // PROTLOCK Open
        // This should also mean that Customer key is accessible
        // All the infospace keys are available to be read - and used for OTA and image verification
        // In this template - we'll just print the values to confirm that we've access to it
        //
        am_util_stdio_printf("Customer Key: 0x%x-0x%x-0x%x-0x%x\r\n", keyVal.keyword[0], keyVal.keyword[1], keyVal.keyword[2], keyVal.keyword[3]);

        //
        // Print keybanks
        //
        for ( i = 0; i < 4; i++ )
        {
            am_util_stdio_printf("Key bank %d:\n", i);
            for ( uint8_t j = 0; j < 16; j++ )
            {
                for ( uint8_t k = 0; k < 4; k++ )
                {
                    am_hal_otp_read_word(AM_REG_OTP_KEYBANK_CUST_QUADRANT0_KEY0_O + 16 * j + 4 * k, &keyVal.keyword[k]);
                }
                am_util_stdio_printf("0x%x-0x%x-0x%x-0x%x\r\n", keyVal.keyword[0], keyVal.keyword[1], keyVal.keyword[2], keyVal.keyword[3]);
            }
        }
    }
#endif
}

//*****************************************************************************
//
//! @brief  Hardcoded function - to Run supplied main program
//!
//! @param  r0 = vtor - address of the vector table
//!
//! @return Returns None
//
//*****************************************************************************
#if (defined (__ARMCC_VERSION)) && (__ARMCC_VERSION <  6000000)
static __asm void
bl_run_main(uint32_t *vtor)
{
    //
    // Store the vector table pointer of the new image into VTOR.
    //
    movw    r3, #0xED08
    movt    r3, #0xE000
    str     r0, [r3, #0]

    //
    // Load the new stack pointer into R1 and the new reset vector into R2.
    //
    ldr     r3, [r0, #0]
    ldr     r2, [r0, #4]

    //
    // Set the stack pointer for the new image.
    //
    mov     sp, r3

    //
    // Jump to the new reset vector.
    //
    bx      r2
}
#elif (defined (__ARMCC_VERSION)) && (__ARMCC_VERSION >= 6000000)
__attribute__((naked))
static void
bl_run_main(uint32_t *vtor)
{
    __asm
    (
        "   movw    r3, #0xED08\n\t"    // Store the vector table pointer of the new image into VTOR.
        "   movt    r3, #0xE000\n\t"
        "   str     r0, [r3, #0]\n\t"
        "   ldr     r3, [r0, #0]\n\t"   // Load the new stack pointer into R1 and the new reset vector into R2.
        "   ldr     r2, [r0, #4]\n\t"
        "   mov     sp, r3\n\t"         // Set the stack pointer for the new image.
        "   bx      r2\n\t"            // Jump to the new reset vector.
    );
}
#elif defined(__GNUC_STDC_INLINE__)
__attribute__((naked))
static void
bl_run_main(uint32_t *vtor)
{
    __asm
    (
        "   movw    r3, #0xED08\n\t"    // Store the vector table pointer of the new image into VTOR.
        "   movt    r3, #0xE000\n\t"
        "   str     r0, [r3, #0]\n\t"
        "   ldr     r3, [r0, #0]\n\t"   // Load the new stack pointer into R1 and the new reset vector into R2.
        "   ldr     r2, [r0, #4]\n\t"
        "   mov     sp, r3\n\t"         // Set the stack pointer for the new image.
        "   bx      r2\n\t"            // Jump to the new reset vector.
    );
}
#elif defined(__IAR_SYSTEMS_ICC__)
__stackless static inline void
bl_run_main(uint32_t *vtor)
{
    __asm volatile (
          "    movw    r3, #0xED08\n"    // Store the vector table pointer of the new image into VTOR.
          "    movt    r3, #0xE000\n"
          "    str     r0, [r3, #0]\n"
          "    ldr     r3, [r0, #0]\n"   // Load the new stack pointer into R1 and the new reset vector into R2.
          "    ldr     r2, [r0, #4]\n"
          "    mov     sp, r3\n"         // Set the stack pointer for the new image.
          "    bx      r2\n"            // Jump to the new reset vector.
          );
}
#else
#error Compiler is unknown, please contact Ambiq support team
#endif

//*****************************************************************************
//
// @brief  Helper function to Perform exit operations for a secondary bootloader
//
// @param  pImage - The address of the image to give control to
//
// This function does the necessary security operations while exiting from a
// a secondary bootloader program. If still open, it locks the OTP key region,
// as well as further updates to the flash protection register.
// It also checks if it needs to halt to honor a debugger request.
// If an image address is specified, control is transferred to the same on exit.
//
// @return Returns AM_HAL_STATUS_SUCCESS on success, if no image address specified
// If an image address is provided, a successful execution results in transfer to
// the image - and this function does not return.
//
//*****************************************************************************
uint32_t
am_hal_bootloader_exit(uint32_t *pImage)
{
    uint32_t status = AM_HAL_STATUS_SUCCESS;

    //
    // Lock the assets
    //
    if ( MCUCTRL->BOOTLOADER_b.PROTLOCK )
    {
        am_hal_security_128bkey_t keyVal;
        for ( uint8_t i = 0; i < 4; i++ )
        {
            am_hal_otp_read_word(AM_REG_OTP_WRAP_KEY0_O + 4 * i, &keyVal.keyword[i]);
        }
        bool bLockStatus;

        //
        // PROTLOCK Open
        // This should also mean that Customer key is accessible
        // Now lock the key by writing an incorrect value
        //
        keyVal.keyword[0] = ~keyVal.keyword[0];
        am_hal_security_set_key(AM_HAL_SECURITY_LOCKTYPE_CUSTOTP_PROG, &keyVal);

        status = am_hal_security_get_lock_status(AM_HAL_SECURITY_LOCKTYPE_CUSTOTP_PROG, &bLockStatus);

        if ((status != AM_HAL_STATUS_SUCCESS) || (bLockStatus))
        {
            return AM_HAL_STATUS_FAIL;
        }

        //
        // Lock the protection register to prevent further region locking
        // CAUTION!!! - Can not do writing on BOOTLOADER register as all writable
        //              bits in this register are Write 1 to clear
        //
        MCUCTRL->BOOTLOADER = _VAL2FLD(MCUCTRL_BOOTLOADER_PROTLOCK, 1);

        //
        // Check if we need to halt (debugger request)
        //
        if (MCUCTRL->SCRATCH0 & 0x1)
        {
            //
            // Debugger wants to halt
            //
            uint32_t dhcsr = AM_REGVAL(0xE000EDF0);

            //
            // Clear the flag in Scratch register
            //
            MCUCTRL->SCRATCH0 &= ~0x1;

            //
            // Halt the core
            //
            dhcsr = ((uint32_t)0xA05F << 16) | (dhcsr & 0xFFFF) | 0x3;
            AM_REGVAL(0xE000EDF0) = dhcsr;

            //
            // Resume from halt
            //
        }
    }

    //
    // Give control to supplied image
    //
    if (pImage)
    {
        bl_run_main(pImage);
        //
        // Does not return
        //
    }
    return status;
} // am_hal_bootloader_exit()

//****************************************************************************
// Main
//
//*****************************************************************************
int
main(void)
{
    //
    // Set the default cache configuration
    //
    am_hal_cachectrl_config(&am_hal_cachectrl_defaults);
    am_hal_cachectrl_enable();

    //
    // Configure the board for low power operation.
    //
    am_bsp_low_power_init();

    //
    // Enable debug printf messages using ITM on SWO pin
    //
    am_bsp_itm_printf_enable();

    //
    // Clear the terminal and print the banner.
    //
    am_util_stdio_terminal_clear();
    am_util_stdio_printf("This is Apollo4 Secondary Bootloader Reference Example!\n\n\n");

    //
    // Add internal & external flash interfaces
    //
    ssbl_flash_init();

    if (ssbl_flash_add(&mspi_psram_info) == SSBL_FLASH_INIT_FAIL)
    {
        am_util_stdio_printf("psram initilization failed!\n");
        while (1);
    }

    ssbl_secure_init();

    dump_info_keys();

    // Process OTA's
    // This reference example builds on top of the OTA framework already used by Ambiq SBL
    // Optionally - OEM may define their own means to communicate OTA information
    if ( MCUCTRL->OTAPOINTER_b.OTAVALID )
    {
        uint32_t *pOtaDesc = (uint32_t *)(MCUCTRL->OTAPOINTER & ~0x3);
        process_ota_desc_list(pOtaDesc);
    }

    //
    // Clear OTA_POINTER
    //
    MCUCTRL->OTAPOINTER_b.OTAVALID = 0;

    uint32_t *pVtor;
    //
    // Validate main image
    //
    ssbl_validate_main_image(&pVtor);

    //
    // Lock the assets (if needed) and give control to main
    //
    am_hal_bootloader_exit(pVtor);

    while(1);
}
