//*****************************************************************************
//
//! @file apollo4_secBLProc.C
//!
//! @brief Utilities for A simple secondary bootloader program example template for Apollo4
//!
//! Purpose: This program is an example template for a secondary bootloader program for Apollo4.
//! It demonstrates how to access OTP key area. It demonstrates how to use the Ambiq SBL OTA
//! framework for customer specific OTAs, e.g. to support external psram, or to support more
//! advanced auth/enc schemes. It demonstrates how to validate & transfer control to the real
//! main program image (assumed to be at address specified by MAIN_PROGRAM_ADDR_IN_FLASH in mram)
//! after locking the OTP area before exiting
//!
//! This example only demonstrates nonsecure boot and nonsecure OTA.
//!
//! Printing takes place over the ITM at 1M Baud.
//!
//! Additional Information:
//! To exercise this program:
//! Flash the main program at 0x80000 (MAIN_PROGRAM_ADDR_IN_FLASH)
//! Link this program at the address suitable for SBL(0x18000)
//! configuration
//! To test OTA - construct images using magic numbers in the range matching
//! AM_IMAGE_MAGIC_CUST
//! To test OTP key area access - need to keep OTP->Security->PLONEXIT as 1
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

#include "apollo4_secBLProc.h"
#include <string.h>

//
//! Bitmasks signifying the bit to be cleared for OTA success/failure
//
#define OTA_DONE_FAILURE_CLRMASK      0x1
#define OTA_DONE_SUCCESS_CLRMASK      0x3

//
//! Number of most significant bits in w113 used for load address
//
#define AM_IMAGE_LOAD_ADDR_MSB_BITS   30
#define AM_IMAGE_GET_LOADADDR(opt1)   (((am_image_opt1_t *)(opt1))->fw.loadAddrMsb << (32 - AM_IMAGE_LOAD_ADDR_MSB_BITS))

typedef struct
{
    am_image_hdr_common_t common;   //!< the first two longwords in the blob
    am_image_auth_info_t auth_info; //!< the large block that may come after the hdr
    am_image_enc_info_t enc_info;   //!< encryption info
    am_image_opt_info_t opt_info;   //!< options 4 longword block
}
am_thirdparty_image_hdr_t;


typedef struct
{
    am_image_hdr_common_t *common;      //!< pointer to heaer
    uint32_t am_imager_hdr_len;         //!< length of header
    am_image_auth_info_t *auth_info;    //!< pointer to large block
    uint32_t am_image_auth_info_len;    //!< length of large block
    am_image_enc_info_t *enc_info;      //!< pointer to encryption info
    uint32_t am_image_enc_info_len;     //!< length of encryption info
    am_image_opt_info_t *opt_info;      //!< pointer to option info
    uint32_t am_image_opt_len;          //!< length to option info
}
am_image_ptrs_t;

am_image_ptrs_t g_am_ptrs;

#define MRAM_PAGE_SIZE 256

//
// Static function prototypes
//
static am_secvl_ret_codes_t secondarybl_flashMramFromMram(
    uint32_t ui32SourceAddrMram,
    uint32_t ui32DestAddrMram,
    uint32_t ui32Size,
    uint32_t uiProgramKey);

static am_secvl_ret_codes_t secondarybl_processOtaImage(
    uint32_t otaImagePtr,
    am_hal_mcuctrl_device_t *psDevice,
    uint32_t *pui32DestAddr);

static bool secondarybl_check_image_address(
    uint32_t ui32StartAddr,
    uint32_t ui32Size,
    am_hal_mcuctrl_device_t *psDevice);

//*****************************************************************************
//
//this is a utility to print the keybanks
//
//*****************************************************************************
am_secvl_ret_codes_t
secondarybl_ProcOtp(void)
{
    //
    // If OTP->Security->PLONEXIT was set, SBL will keep OTP open upon transferring to main image
    // This allows the secondary bootloader to use the keys in OTP to perform any necessary validations
    // of images and/or OTA upgrades
    //

    uint32_t ui32Security = 0;

    uint32_t status = am_hal_otp_read_word(AM_REG_OTP_SECURITY_O, &ui32Security);
    if (status != AM_HAL_STATUS_SUCCESS)
    {
        am_util_stdio_printf("OTP read failure:\n");
        return (am_secvl_ret_codes_t) status;
    }
    if (ui32Security & AM_REG_OTP_SECURITY_PLONEXIT_Msk)
    {
        am_hal_security_128bkey_t keyVal;
        for (int i = 0; i < 4; i++)
        {
            am_hal_otp_read_word(AM_REG_OTP_WRAP_KEY0_O + 4 * i, &keyVal.keyword[i]);
        }
        //
        // PROTLOCK Open
        // This should also mean that Customer key is accessible
        // All the OTP space keys are available to be read - and used for OTA and image verification
        // In this template - we'll just print the values to confirm that we've access to it
        //
        am_util_stdio_printf("Customer Key: 0x%x-0x%x-0x%x-0x%x\r\n",
                             keyVal.keyword[0], keyVal.keyword[1],
                             keyVal.keyword[2], keyVal.keyword[3]);

        //
        // Print keybanks
        //
        for (uint32_t i = 0; i < 4; i++)
        {
            am_util_stdio_printf("Key bank %d:\n", i);
            for (uint32_t j = 0; j < 16; j++)
            {
                for (uint32_t k = 0; k < 4; k++)
                {
                    am_hal_otp_read_word(AM_REG_OTP_KEYBANK_CUST_QUADRANT0_KEY0_O + 16 * j + 4 * k, &keyVal.keyword[k]);
                }
                am_util_stdio_printf("0x%x-0x%x-0x%x-0x%x\r\n", keyVal.keyword[0], keyVal.keyword[1], keyVal.keyword[2],
                                     keyVal.keyword[3]);
            }
        }
    }
    return eSBL_OK;
}

//*****************************************************************************
//
//! @brief  check address template
//!
//! @details This will check to ensure the starting and ending addresses for
//! the target are in a valid range
//!
//! @param ui32StartAddr  - image start address
//! @param ui32Size       - image size
//! @param psDevice
//!
//! @return true if the addresses are valid
//
//*****************************************************************************
static bool secondarybl_check_image_address(uint32_t ui32StartAddr,
                                            uint32_t ui32Size,
                                            am_hal_mcuctrl_device_t *psDevice)
{

    //
    // first check mram address
    //
    uint32_t addLimStart = AM_HAL_MRAM_ADDR;
    uint32_t addLimEnd = addLimStart + psDevice->ui32MRAMSize;
    uint32_t endAddr = ui32StartAddr + ui32Size;
    am_util_stdio_printf("Mram Test :0x%x, size:0x%x, lo:0x%x, hi:0x%x \n",
                         ui32StartAddr, ui32Size, addLimStart, addLimEnd);
    // start addr is OK
    if ((ui32StartAddr >= addLimStart) && (ui32StartAddr < addLimEnd))
    {
        return (endAddr < addLimEnd);
    }

    //
    // example can check if loaded into sram here:
    //
    addLimStart = SRAM_BASEADDR;
    addLimEnd = addLimStart + SSRAM0_MAX_SIZE;
    if ((ui32StartAddr >= addLimStart) && (ui32StartAddr < addLimEnd))
    {
        return (endAddr < addLimEnd);
    }

    return false;
}

//*****************************************************************************
//
//! @brief process an ota image
//!
//! @param otaImagePtr      - pointer to ota image in RAM or MRAM
//! @param psDevice         - device parameters for this Micro
//! @param pui32DestAddr    - final destination address embedded in ota
//!
//! @return            - am_secvl_ret_codes_t
//
//*****************************************************************************
static am_secvl_ret_codes_t
secondarybl_processOtaImage(uint32_t otaImagePtr,
                            am_hal_mcuctrl_device_t *psDevice,
                            uint32_t *pui32DestAddr)
{
    //
    // This template assumes OTA images using the same formal as used by Main.
    // users can use any image format
    //
    uint32_t otaImageAddr = AM_HAL_OTA_GET_BLOB_PTR(otaImagePtr);
    g_am_ptrs.common = (am_image_hdr_common_t *) otaImageAddr;

    //
    // Valid OTA image
    // Make sure the image is contained within SRAM or is it MRAM or both?
    //
    uint32_t ui32DescTotalLen = g_am_ptrs.common->w0.s.blobSize & ~0x03;

    if (false == secondarybl_check_image_address(otaImageAddr, ui32DescTotalLen, psDevice))
    {
        // Invalidate this OTA for subsequent processing
        // @todo this does not invalidate
        am_util_stdio_printf("Found bad OTA pointing to: image address=0x%x, size 0x%x\n",
                             otaImagePtr,
                             ui32DescTotalLen);

        return eSBL_INVALID_SIZE;
    }
    //
    // Indicate Failure
    //?am_hal_flash_clear_bits(AM_HAL_MRAM_PROGRAM_KEY, &pOtaDesc[descCount], AM_HAL_SECURE_OTA_OTA_DONE_FAILURE_CLRMASK);
    //

    //
    // Check if a crc appended?
    //
    if (g_am_ptrs.common->w0.s.crcCheck)
    {
        uint32_t ComputedCrc = 0;
        uint32_t ui32CrcStatus = am_hal_crc32((uint32_t) &g_am_ptrs.common->w2.ui32, ui32DescTotalLen - 8,
                                              &ComputedCrc);
        if ((ui32CrcStatus) || (g_am_ptrs.common->crc != ComputedCrc))
        {
            am_util_stdio_printf("Invalid CRC\n");
            return eSBL_INVALID_CRC;
        }
    }
    //
    // this variable holds the size of the blob. This is computed based on the
    // options set in the headers. Start with the standard header size
    //
    uint32_t ui32Leng_b = sizeof(am_image_hdr_common_t);  // this the header size
    g_am_ptrs.am_imager_hdr_len = sizeof(am_image_hdr_common_t); // this for the header

    //
    // is the authentication area included in the blob.
    //
    if (g_am_ptrs.common->w0.s.authCheck)
    {
        g_am_ptrs.auth_info = (am_image_auth_info_t *) ((uint32_t) otaImageAddr + ui32Leng_b);
        g_am_ptrs.am_image_auth_info_len = sizeof(am_image_auth_info_t);

        //
        // advance length since authentication block is included
        //
        ui32Leng_b += sizeof(am_image_auth_info_t);  // auth is included
    }

    //
    // is the encryption block included
    //
    if (g_am_ptrs.common->w0.s.enc)
    {
        g_am_ptrs.enc_info = (am_image_enc_info_t *) (otaImageAddr + ui32Leng_b);
        g_am_ptrs.am_image_enc_info_len = sizeof(am_image_enc_info_t);
        //
        // advance length since encryption block is included
        //
        ui32Leng_b += sizeof(am_image_enc_info_t);  // encr is included
    }
    // next is the ota image blob and header

    if (ui32DescTotalLen < ui32Leng_b)
    {
        am_util_stdio_printf("Invalid total size\n");
        return eSBL_INCONSISTENT_SIZE;
    }

    //
    // at this point, location offset (ui32Leng_b) of the secondary header
    // has been computed, assign a pointer to this location
    //
    am_ex_secUpdate_opt_u *psecHdr = (am_ex_secUpdate_opt_u *) (otaImageAddr + ui32Leng_b);

    //
    // process the magic number. This will detirmine what type of header psecHdr points to
    //
    uint32_t ui32MagicNum = psecHdr->secureNonSec.magicNum;
    if (!AM_IMAGE_MAGIC_CUST(ui32MagicNum))
    {
        //
        // unknown OTA
        //
        am_util_stdio_printf("Found unexpected OTA\n");

        //
        // Indicate Failure
        //
        return eSBL_INVALID_OTA;
    }

    // compute bytes after this header
    ui32Leng_b += sizeof(am_ex_secUpdate_opt_u);
    if (ui32DescTotalLen < ui32Leng_b)
    {
        return eSBL_INCONSISTENT_SIZE2;
    }

    // number of bytes after the second header
    uint32_t ui32bytesLeftAfterHeader = ui32DescTotalLen - ui32Leng_b;
    bool ccIncluded = g_am_ptrs.common->w0.s.ccIncluded == 1;

    switch (ui32MagicNum)
    {
        case AM_IMAGE_MAGIC_SECURE:
            am_util_stdio_printf("Unhandled type: AM_IMAGE_MAGIC_SECURE  \n");
            return eSBL_UNHANDLED_TYPE;
        case AM_IMAGE_MAGIC_OEM_CHAIN:

            return secondarybl_ProcessCertificateChain((am_secbl_certChainUpdate_t *) psecHdr,
                                                       ui32bytesLeftAfterHeader);
        case AM_IMAGE_MAGIC_NONSECURE:
            if (ccIncluded)
            {

                //
                // this is not really non-secure with this set, so error this out
                // see section 4.4 (Apollo4 and Apollo4 Blue Secure Update User's Guide)
                // although may want to process this in the future.
                //
                return eSBL_INCONSISTENT_BLOB;
            }
            return secondarybl_ProcessNonSecure((am_secbl_secureNonSecure_t *) psecHdr,
                                                ui32bytesLeftAfterHeader,
                                                ccIncluded,
                                                pui32DestAddr);
        case AM_IMAGE_MAGIC_INFO0:
            return secondarybl_ProcessInfo0Update((am_secbl_INFO0_t *) psecHdr,
                                                  ui32bytesLeftAfterHeader);
        case AM_IMAGE_MAGIC_CONTAINER:
            am_util_stdio_printf("Unhandled type: AM_IMAGE_MAGIC_CONTAINER  \n");
            return eSBL_UNHANDLED_TYPE;
        case AM_IMAGE_MAGIC_KEYREVOKE:
            return secondarybl_ProcessKeyrevoke((am_secbl_keyRevoke_t *) psecHdr, ui32bytesLeftAfterHeader);
        case AM_IMAGE_MAGIC_DOWNLOAD:
            am_util_stdio_printf("Unhandled type: AM_IMAGE_MAGIC_DOWNLOAD  \n");
            return secondarybl_ProcessDownload((am_secbl_wiredUpdate_t *) psecHdr,
                                               ui32bytesLeftAfterHeader,
                                               pui32DestAddr);

    } // switch ui32MagicNum

    am_util_stdio_printf("Unhandled type: invalid magic num  \n");
    return eSBL_INVALID_OTA;
} // fcn secondarybl_processOtaImage

//*****************************************************************************
//
// @brief template for AM_IMAGE_MAGIC_INFO0
// @param psecUnsec
// @param ui32NumBytesLeft
// @return
//
//*****************************************************************************
am_secvl_ret_codes_t
secondarybl_ProcessInfo0Update(am_secbl_INFO0_t *psecUnsec,
                               uint32_t ui32NumBytesLeft)
{
    am_util_stdio_printf("Unhandled type: AM_IMAGE_MAGIC_INFO0  \n");
    return eSBL_UNHANDLED_TYPE;

} // fcn secondarybl_ProcessInfo0Update


//*****************************************************************************
//
// template for AM_IMAGE_MAGIC_OEM_CHAIN
//
//*****************************************************************************
am_secvl_ret_codes_t
secondarybl_ProcessCertificateChain(am_secbl_certChainUpdate_t *psecUnsec,
                                    uint32_t ui32NumBytesLeft)
{
    am_util_stdio_printf("Unhandled type: AM_IMAGE_MAGIC_OEM_CHAIN  \n");
    return eSBL_UNHANDLED_TYPE;

} // fcn secondarybl_ProcessCertificateChain

//*****************************************************************************
//
// template for AM_IMAGE_MAGIC_OEM_CHAIN
//
//*****************************************************************************
am_secvl_ret_codes_t
secondarybl_ProcessKeyrevoke(am_secbl_keyRevoke_t *psecUnsec,
                             uint32_t ui32NumBytesLeft)
{
    am_util_stdio_printf("Unhandled type: AM_IMAGE_MAGIC_KEYREVOKE  \n");
    return eSBL_UNHANDLED_TYPE;

} // fcn secondarybl_ProcessKeyrevoke

//*****************************************************************************
//
//! @brief this function will move data from one part of MRAM to another
//!
//! @param ui32SourceAddrMram address of source in MRAM
//! @param ui32DestAddrMram   address of dest in MRAM
//! @param ui32Size           number of bytes to move
//! @param uiProgramKey       mram program key
//!
//! @return am_secvl_ret_codes_t status codes and return codes from bootloader
//
//*****************************************************************************
static am_secvl_ret_codes_t
secondarybl_flashMramFromMram(
    uint32_t ui32SourceAddrMram,
    uint32_t ui32DestAddrMram,
    uint32_t ui32Size,
    uint32_t uiProgramKey)
{

    if (ui32Size == 0)
    {
        am_util_stdio_printf("secondarybl_flashMramFromMram: Zero buff\n");
        return eSBL_ZERO_BUFF;
    }
    if (ui32DestAddrMram & 0xF)
    {
        am_util_stdio_printf("secondarybl_flashMramFromMram: misaligned dest addr %0X\n", ui32DestAddrMram);
        return eSBL_MISALIGNED_ADDR;   // misaligned
    }

    uint32_t ui32endAddr = ui32DestAddrMram + ui32Size;
    uint32_t ui32PageStart = ui32DestAddrMram / MRAM_PAGE_SIZE;
    uint32_t ui32PageEnd = ui32endAddr / MRAM_PAGE_SIZE;
    uint32_t ui32NumPages = ui32PageEnd - ui32PageStart + 1;
    uint32_t tempBuff[MRAM_PAGE_SIZE / 4];

    uint32_t rstat = 0;
    uint32_t ui32NumLoops = 0;

    if (ui32DestAddrMram != ui32PageStart * MRAM_PAGE_SIZE)
    {
        //
        // start not page aligned, finsih the page up then get into normal
        // full page sized loops
        //
        uint32_t ui32NumB = ui32PageStart * MRAM_PAGE_SIZE + MRAM_PAGE_SIZE - ui32DestAddrMram;
        memcpy(tempBuff, (void *) ui32SourceAddrMram, ui32NumB);
        rstat = (uint32_t) am_hal_mram_main_program(uiProgramKey, tempBuff, (uint32_t *) ui32DestAddrMram, ui32NumB);
        ui32SourceAddrMram += ui32NumB;
        ui32Size -= ui32NumB;
        ui32PageStart++;
        ui32DestAddrMram += ui32NumB;
        ui32NumLoops++;

    }

    while ((rstat == 0) && (ui32Size != 0))
    {
        uint32_t ui32NumB = ui32Size > MRAM_PAGE_SIZE ? MRAM_PAGE_SIZE : ui32Size;
        memcpy(tempBuff, (void *) ui32SourceAddrMram, ui32NumB);
        rstat = (uint32_t) am_hal_mram_main_program(uiProgramKey, tempBuff, (uint32_t *) ui32DestAddrMram, ui32NumB);
        ui32SourceAddrMram += ui32NumB;
        ui32Size -= ui32NumB;
        ui32PageStart++;
        ui32DestAddrMram += ui32NumB;
        ui32NumLoops++;
    }

    am_util_stdio_printf("secondarybl_flashMramFromMram: status %ld, numLoops %d, numPagesExpected %d\n",
                         rstat, ui32NumLoops, ui32NumPages);

    return (am_secvl_ret_codes_t) rstat;
} // fcn secondarybl_flashMramFromMram

//*****************************************************************************
//
// handler for AM_IMAGE_MAGIC_DOWNLOAD
//
//*****************************************************************************
am_secvl_ret_codes_t
secondarybl_ProcessDownload(am_secbl_wiredUpdate_t *psecUnsec,
                            uint32_t ui32NumBytesLeft,
                            uint32_t *pui32DestAddr)
{
    //
    // Process OTA
    // We can perform any necessary verification/decryption here before installing a valid OTA image
    // install
    // Operate only in flash page multiples
    //

    uint32_t ui32DestAddr = psecUnsec->address; // msb of address to load imager to

    uint32_t ui32DataStartAddr = (uint32_t) psecUnsec + sizeof(am_secbl_wiredUpdate_t);

    return secondarybl_flashMramFromMram(
        ui32DataStartAddr,
        ui32DestAddr,
        ui32NumBytesLeft,
        psecUnsec->programKey);


} // fcn secondarybl_ProcessDownload

//*****************************************************************************
//
// AM_IMAGE_MAGIC_NONSECURE
//
//*****************************************************************************
am_secvl_ret_codes_t
secondarybl_ProcessNonSecure(am_secbl_secureNonSecure_t *psecUnsec,
                             uint32_t ui32NumBytesLeft,
                             bool bCCIncluded,
                             uint32_t *pui32DestAddr)
{
    //
    // Process OTA
    // We can perform any necessary verification/decryption here before installing a valid OTA image
    // install
    // Operate only in flash page multiples
    // the data starts after the end of the header + the ui32Size of the cert (if there)
    //
    uint32_t ui32CertSize = 0;
    // is ther
    uint32_t ui32HeaderAddr = (uint32_t) psecUnsec;     // this is address of the start of header
    if (bCCIncluded)
    {
        // get the length
        ui32CertSize = psecUnsec->ccSize;
        if (ui32NumBytesLeft <= ui32CertSize)
        {
            return eSBL_INCONSISTENT_SIZE3;
        }
        ui32NumBytesLeft -= ui32CertSize;
    }

    //
    // find the start address of the data (image) that will be loaded into mram
    //
    uint32_t ui32DataStartAddr = ui32HeaderAddr + sizeof(am_secbl_secureNonSecure_t) + ui32CertSize;

    uint32_t ui32DestAddr = psecUnsec->loadAddress;

    am_util_stdio_printf(
        "ready to write data HeaderStart = %lX, CertSize = %lX, Buff Addr = %lX, Buff Dest Addr = %lX, bytes to write = %lX\n",
        ui32HeaderAddr, ui32CertSize, ui32DataStartAddr, ui32DestAddr, ui32NumBytesLeft);

    *pui32DestAddr = ui32DestAddr;

    return secondarybl_flashMramFromMram(
        ui32DataStartAddr,
        ui32DestAddr,
        ui32NumBytesLeft,
        AM_HAL_MRAM_PROGRAM_KEY);

} // fcn secondarybl_ProcessNonSecure

//*****************************************************************************
// process all OTA that have been loaded in RAM or MRAM
//*****************************************************************************
am_secvl_ret_codes_t
secondarybl_ProcOTAs(am_secbl_OTA_ptr_t otaPtr,
                     uint32_t *pui32DestAddr,
                     am_hal_mcuctrl_device_t *psDevice)
{

    am_util_stdio_printf("OTA pointer - OTA Desc @0x%x\n", otaPtr.OTAPOINTER);

    am_secvl_ret_codes_t ret_val = eSBL_OK;

    //
    // Process OTA's
    //

    if (otaPtr.OTAPOINTER_b.OTAVALID)
    {
        // exact actual address from pointer (lower two bits are flags)
        uint32_t *pOtaDesc = (uint32_t *) (otaPtr.OTAPOINTER & ~0x3);
        uint32_t descCount = 0;

        am_util_stdio_printf("OTA Available - OTA Desc @0x%x\n", pOtaDesc);
        //
        // Make sure the OTA list is valid
        // Whole OTA list is skipped if it is constructed incorrectly
        //
        while (1)
        {
            uint32_t otaImagePtr = pOtaDesc[descCount];
            if (otaImagePtr == AM_HAL_SECURE_OTA_OTA_LIST_END_MARKER)
            {
                am_util_stdio_printf("ota processing complete\n");
                am_util_delay_ms(200);
                break;
            }

            if (AM_HAL_OTA_IS_VALID(otaImagePtr))
            {
                uint32_t status = secondarybl_processOtaImage(otaImagePtr, psDevice, pui32DestAddr);
                if (status)
                {
                    am_util_stdio_printf("process OTA failure\n");
                    pOtaDesc[descCount] &= ~(OTA_DONE_FAILURE_CLRMASK);
                }
                else
                {
                    am_util_stdio_printf("process OTA success\n");
                    //
                    // hard fault when trying to clear this bit in MRAM
                    //
                    //pOtaDesc[descCount] &= ~(OTA_DONE_SUCCESS_CLRMASK);   // this is in mram??? can't clear bits
                }
            }
            else
            {
                am_util_stdio_printf("ota already valid\n");
            }

            if (++descCount == AM_HAL_SECURE_OTA_MAX_OTA)
            {
                am_util_stdio_printf("Exceeded maximum OTAs %d\n", descCount);
                break;
            }

        } // while
    }
    else  // otaPtr.OTAPOINTER_b.OTAVALID
    {
        am_util_stdio_printf("OTA pointer - Not valid @0x%x\n", otaPtr.OTAPOINTER);
        ret_val = eSBL_INVALID_OTA;

    } // otaPtr.OTAPOINTER_b.OTAVALID

    //
    // Clear OTA_POINTER
    //
    MCUCTRL->OTAPOINTER = 0;

    return ret_val;
}
