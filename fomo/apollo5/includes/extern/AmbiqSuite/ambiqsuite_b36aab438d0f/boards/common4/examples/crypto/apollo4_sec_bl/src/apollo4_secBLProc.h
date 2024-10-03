//*****************************************************************************
//
//! @file apollo4_secBLProc.h
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

#ifndef APOLLO4_SECBLPROC_H
#define APOLLO4_SECBLPROC_H

#include "am_mcu_apollo.h"
#include "am_bsp.h"
#include "am_util.h"

#define MAIN_PROGRAM_ADDR_IN_FLASH    0x100000ul // This would normally come from OTP

//!
//! enable anonymous structs for Keil
//!
#if defined(__ARMCC_VERSION)
    #pragma anon_unions
#endif

//!
//! define return codes for this example's operation
//!
typedef enum
{
    eSBL_OK,
    eSBL_INVALID_OTA = 0x20,
    eSBL_INVALID_SIZE,
    eSBL_INVALID_CRC,
    eSBL_INCONSISTENT_SIZE,
    eSBL_INCONSISTENT_SIZE2,
    eSBL_INCONSISTENT_SIZE3,
    eSBL_INCONSISTENT_BLOB,
    eSBL_UNHANDLED_TYPE,
    eSBL_INVALID_BOOT_ADDR,
    eSBL_INVALID_IMAGE,
    eSBL_ZERO_BUFF,
    eSBL_MISALIGNED_ADDR,

    eSBL_x32 = 0x10000000,
}
am_secvl_ret_codes_t;

typedef struct
{
    // keyrevoke
    uint8_t magicNum;
    uint8_t undef1[3];
    uint32_t undef2[3];

}
am_secbl_keyRevoke_t;

//
//! definitions for the blob/data header
//! this is the second header in the blob
//
typedef struct
{
    // for Secure or Non Secure
    struct
    {
        uint32_t magicNum: 8;
        uint32_t ccSize: 11;
        uint32_t resw0: 13;
    };
    uint32_t loadAddress;
    uint32_t reserved[2];
}
am_secbl_secureNonSecure_t;

typedef struct
{
    uint8_t magicNum;
    uint8_t undef1[3];
    struct
    {
        uint32_t offset: 12;
        uint32_t size: 12;
        uint32_t res: 8;
    };
    uint32_t info_key;
    uint32_t w4;

}
am_secbl_INFO0_t;

typedef struct
{
    struct
    {
        uint32_t magicNum: 8;
        uint32_t otaProcRequest: 1;
        uint32_t sblOTA: 1;
        uint32_t w0Res0: 22;
    };
    uint32_t address;
    uint32_t programKey;
    uint32_t w4;
}
am_secbl_wiredUpdate_t;

typedef struct
{
    am_image_opt0_t opt0;
    uint32_t w123Res[3];
}
am_secbl_certChainUpdate_t;

//!
//! the second header will be one of these structs
//!
typedef union
{
    am_secbl_keyRevoke_t keyReoke;
    am_secbl_secureNonSecure_t secureNonSec;
    am_secbl_INFO0_t info0;
    am_secbl_wiredUpdate_t wriredUpdate;
    am_secbl_certChainUpdate_t certChainUpdate;
}
am_ex_secUpdate_opt_u;

//!
//! ensure a compiler does not create a struct the wrong size
//!
typedef char check_my_secUpdate_size[sizeof(am_ex_secUpdate_opt_u) == 16 ? 1 : -1] ;

//!
//! struct describing OTA pointer
//!
typedef union
{
    uint32_t OTAPOINTER;                  /*!< (@ 0x00000264) OTA (Over the Air) Update Pointer/Status. Reset
                                                                    only by POA                                                */

    struct
    {
        uint32_t OTAVALID   : 1;            /*!< [0..0] Indicates that an OTA update is valid                              */
        uint32_t OTASBLUPDATE : 1;          /*!< [1..1] Indicates that the sbl_init has been updated                       */
        uint32_t OTAPOINTER : 30;           /*!< [31..2] Flash page pointer with updated OTA image                         */
    } OTAPOINTER_b;

}
am_secbl_OTA_ptr_t  ;

//*****************************************************************************
//
//! @brief this is a utility to print the keybanks
//!
//! @return am_secvl_ret_codes_t status
//
//*****************************************************************************
extern am_secvl_ret_codes_t secondarybl_ProcOtp(void );
//*****************************************************************************
//!
//! @brief process all OTA that have been loaded in RAM or MRAM
//!
//! @param otaPtr           - pointer to the list of OTA/Blob pointers
//! @param pui32DestAddr    - pointer to extracted address from OTA (output)
//! @param psDevice         - info for this Ambiq device.
//!
//! @return am_secvl_ret_codes_t
//
//*****************************************************************************
extern am_secvl_ret_codes_t secondarybl_ProcOTAs(
    am_secbl_OTA_ptr_t otaPtr,
    uint32_t *pui32DestAddr,
    am_hal_mcuctrl_device_t *psDevice);

//*****************************************************************************
//
//! @brief handler for AM_IMAGE_MAGIC_NONSECURE
//!
//! @param psecUnsec         - pointer to second blob header
//! @param ui32NumBytesLeft  - number of bytes left, including header length
//! @param bCCIncluded       - certificate is included in the data (3.2.1)
//! @param pui32DestAddr     - extracted address for these ota block destination
//!
//! @return
//
//*****************************************************************************
extern am_secvl_ret_codes_t secondarybl_ProcessNonSecure(
    am_secbl_secureNonSecure_t *psecUnsec,
    uint32_t ui32NumBytesLeft,
    bool bCCIncluded,
    uint32_t *pui32DestAddr);

//*****************************************************************************
//
//! @brief handler for AM_IMAGE_MAGIC_DOWNLOAD
//!
//! @param psecUnsec         - pointer to second blob header
//! @param ui32NumBytesLeft  - number of bytes left, including header length
//! @param pui32DestAddr     - extracted address for these ota block destination
//!
//! @return am_secvl_ret_codes_t status
//
//*****************************************************************************
extern am_secvl_ret_codes_t secondarybl_ProcessDownload(
    am_secbl_wiredUpdate_t *psecUnsec,
    uint32_t ui32NumBytesLeft,
    uint32_t *pui32DestAddr);

//*****************************************************************************
//
//! @brief template for AM_IMAGE_MAGIC_INFO0
//!
//! @param psecUnsec            - pointer to image header
//! @param ui32NumBytesLeft     - number of bytes after header
//!
//! @return am_secvl_ret_codes_t status
//
//*****************************************************************************
extern  am_secvl_ret_codes_t secondarybl_ProcessInfo0Update(
    am_secbl_INFO0_t *psecUnsec,
    uint32_t ui32NumBytesLeft) ;

//*****************************************************************************
//
//! @brief template for AM_IMAGE_MAGIC_OEM_CHAIN
//!
//! @param psecUnsec            - pointer to image header
//! @param ui32NumBytesLeft     - number of bytes after header
//!
//! @return am_secvl_ret_codes_t status
//
//*****************************************************************************
extern  am_secvl_ret_codes_t secondarybl_ProcessCertificateChain(
    am_secbl_certChainUpdate_t *psecUnsec,
    uint32_t ui32NumBytesLeft) ;
//*****************************************************************************
//
//! @brief template for AM_IMAGE_MAGIC_OEM_CHAIN
//!
//! @param psecUnsec        - pointer to image header
//! @param ui32NumBytesLeft - number of bytes after header
//!
//! @return am_secvl_ret_codes_t status
//
//*****************************************************************************
extern am_secvl_ret_codes_t secondarybl_ProcessKeyrevoke(
    am_secbl_keyRevoke_t *psecUnsec,
    uint32_t ui32NumBytesLeft) ;


#endif //APOLLO4_SECBLPROC_H
