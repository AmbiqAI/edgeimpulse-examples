//*****************************************************************************
//
//  amotas_api.h
//! @file
//!
//! @brief Ambiq Micro's demonstration of AMOTA service.
//
//*****************************************************************************

//*****************************************************************************
//
// ${copyright}
//
// This is part of revision ${version} of the AmbiqSuite Development Package.
//
//*****************************************************************************
#ifndef AMOTAS_API_H
#define AMOTAS_API_H

#include "timers.h"
#include "am_mcu_apollo.h"
#include "am_bsp.h"
#include "am_util.h"
#include "am_util_bootloader.h"

#include "am_util_multi_boot.h"
#include "am_hal_security.h"

#ifdef __cplusplus
extern "C"
{
#endif

/*
 * DEFINES
 ****************************************************************************************
 */
#define AMOTA_PACKET_SIZE           (512 + 16)    // Bytes

#define AMOTA_FW_STORAGE_INTERNAL   0
#define AMOTA_FW_STORAGE_EXTERNAL   1
// includes 2bytes length indicating the total OTA packet length
//     and 1byte last packet flag
#define AMOTA_FIRST_PKT_HDR_LEN     3

//The imageId field of amotaHeaderInfo_t is used to get the type of downloading image
//define image id macros to identify the type of OTA image and process corresbonding logic
//the OTA image can be for SBL, host application or others in future
#define AMOTA_IMAGE_ID_SBL          (0x00000001)
#define AMOTA_IMAGE_ID_APPLICATION  (0x00000002)

#define AMOTA_ENCRYPTED_SBL_SIZE    (32 * 1024)
#define AMOTA_INVALID_SBL_STOR_ADDR (0xFFFFFFFF)

// Need temporary buf equal to one flash page size (larger of int or ext, if ext flash is supported)
// We accumulate data in this buffer and perform Writes only on page boundaries in
// multiple of page lentghs
#if (AM_DEVICES_SPIFLASH_PAGE_SIZE > AM_HAL_FLASH_PAGE_SIZE)
#define AMOTAS_TEMP_BUFSIZE     AM_DEVICES_SPIFLASH_PAGE_SIZE
#else
#define AMOTAS_TEMP_BUFSIZE     AM_HAL_FLASH_PAGE_SIZE
#endif

// Protection against NULL pointer
#define FLASH_OPERATE(pFlash, func) ((pFlash)->func ? (pFlash)->func() : 0)

/*
 * ENUMERATIONS
 ****************************************************************************************
 */



/*
 * STRUCTURE DEFINITIONS
 ****************************************************************************************
 */
//
// Data structure for flash operation
//
typedef struct
{
    uint8_t     writeBuffer[AMOTAS_TEMP_BUFSIZE]   __attribute__((aligned(4))); // needs to be 32-bit word aligned.
    uint16_t    bufferIndex;
}amotasFlashOp_t;

//
// FW header information
//
typedef struct
{
    uint32_t    encrypted;
    uint32_t    fwStartAddr;            // Address to install the image
    uint32_t    fwLength;
    uint32_t    fwCrc;
    uint32_t    secInfoLen;
    uint32_t    resvd1;
    uint32_t    resvd2;
    uint32_t    resvd3;
    uint32_t    version;
    uint32_t    fwDataType;             //binary type
    uint32_t    storageType;
    uint32_t    imageId;                //use to identify the ota image is for application, SBL or others
}
amotaHeaderInfo_t;

//
// 32-byte Metadata added ahead of the actual image and behind of 48-byte AMOTA header
//
typedef struct
{
    //Word 0
    uint32_t blobSize:24;
    uint32_t resvd1:2;
    uint32_t crcCheck:1;
    uint32_t enc:1;
    uint32_t authCheck:1;
    uint32_t ccIncluded:1;
    uint32_t ambiq:1;
    uint32_t resvd2:1;

    //Word 1
    uint32_t crc;

    //Word 2
    uint32_t authKeyIdx:8;
    uint32_t encKeyIdx:8;
    uint32_t authAlgo:4;
    uint32_t encAlgo:4;
    uint32_t resvd3:8;

    //Word 3
    uint32_t resvd4;

    //Word 4
    uint32_t magicNum:8;
    uint32_t resvd5:24;

    //Word5-7
    uint32_t resvd6;
    uint32_t resvd7;
    uint32_t resvd8;
}
amotaMetadataInfo_t;

//
// FW packet information
//
typedef struct
{
    uint16_t    offset;
    uint16_t    len;                        // data plus checksum
    uint8_t     data[AMOTA_PACKET_SIZE]  __attribute__((aligned(4)));   // needs to be 32-bit word aligned.
}
amotaPacket_t;

//
// Firmware Address
//
typedef struct
{
    uint32_t    addr;
    uint32_t    offset;
}
amotasNewFwFlashInfo_t;


/* Control block */
typedef struct
{
    amotaHeaderInfo_t       fwHeader;
    amotasNewFwFlashInfo_t  newFwFlashInfo;
    amotaMetadataInfo_t     metaData;
}amotasCb_t;


/*
 * FUNCTION DECLARATIONS
 ****************************************************************************************
 */

void amota_init(void);
void amota_packet_header_handle(uint8_t *header_data);
void amota_reset_start(void);
void amota_update_flag(void);
void amota_write2flash(uint16_t len, uint8_t *buf, bool last_pkt_flag);


#ifdef __cplusplus
}
#endif

#endif // AMOTAS_API_H
