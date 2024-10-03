//*****************************************************************************
//
// ${copyright}
//
// This is part of revision ${version} of the AmbiqSuite Development Package.
//
//*****************************************************************************

#ifndef IOS_BOOT_HOST_H
#define IOS_BOOT_HOST_H

#define AM_BOOTLOADER_IMAGEHDR_OFFSET_KEK     64
#define AM_BOOTLOADER_KEK_SIZE                16
#define AM_BOOTLOADER_IMAGEHDR_SIZE           (AM_BOOTLOADER_IMAGEHDR_OFFSET_KEK + AM_BOOTLOADER_KEK_SIZE + 16)

#define FLASH_PAGE_SIZE                       (8 * 1024)
#define MAX_SPLIT_SIZE                        0x48000 // 288K
#define MAX_UPDATE_SIZE                       (AM_BOOTLOADER_IMAGEHDR_SIZE + MAX_SPLIT_SIZE)
#define MAX_IOS_MSG_SIZE                      (2 * 1024)  // 2K is ok
#define MAX_CHUNK_SIZE                        (MAX_IOS_MSG_SIZE - 12)

typedef struct
{
  uint32_t                      length  : 16;
  uint32_t                      resv    : 14;
  uint32_t                      bEnd    : 1;
  uint32_t                      bStart  : 1;
} am_secboot_ios_pkthdr_t;

typedef enum
{
    AM_SECBOOT_WIRED_MSGTYPE_HELLO,
    AM_SECBOOT_WIRED_MSGTYPE_STATUS,
    AM_SECBOOT_WIRED_MSGTYPE_OTADESC,
    AM_SECBOOT_WIRED_MSGTYPE_UPDATE,
    AM_SECBOOT_WIRED_MSGTYPE_ABORT,
    AM_SECBOOT_WIRED_MSGTYPE_RECOVER,
    AM_SECBOOT_WIRED_MSGTYPE_RESET,
    AM_SECBOOT_WIRED_MSGTYPE_ACK,
    AM_SECBOOT_WIRED_MSGTYPE_DATA,
} am_secboot_wired_msgtype_e;

typedef struct
{
    uint32_t                     crc32; // First word
    uint16_t                     msgType; // am_secboot_wired_msgtype_e
    uint16_t                     length;
} am_secboot_wired_msghdr_t;

typedef struct
{
    am_secboot_wired_msghdr_t       hdr;
    uint32_t                        otaDescAddr;
} am_secboot_wired_msg_ota_desc_t;

typedef enum
{
    AM_SECBOOT_WIRED_IMAGETYPE_SBL,
    AM_SECBOOT_WIRED_IMAGETYPE_AM3P,
    AM_SECBOOT_WIRED_IMAGETYPE_PATCH,
    AM_SECBOOT_WIRED_IMAGETYPE_AMBIQ_LAST = AM_SECBOOT_WIRED_IMAGETYPE_PATCH,
    AM_SECBOOT_WIRED_IMAGETYPE_MAIN,
    AM_SECBOOT_WIRED_IMAGETYPE_CHILD,
    AM_SECBOOT_WIRED_IMAGETYPE_CUSTPATCH,
    AM_SECBOOT_WIRED_IMAGETYPE_NONSECURE,
    AM_SECBOOT_WIRED_IMAGETYPE_INFO0,
    AM_SECBOOT_WIRED_IMAGETYPE_INFO0_NOOTA = 32,
} am_secboot_wired_imagetype_e;

// Command used to download an OTA blob
typedef struct
{
    am_secboot_wired_msghdr_t       hdr;
    uint32_t                        totalSize; // Total Size of image blob
    uint32_t                        crc32; // CRC of the complete image blob
    uint32_t                        size; // Length of useful data following in this message
} am_secboot_wired_msg_update_t;

// Abort Wired Update process
// If this option is set - SBL quits the host connection mode
#define AM_SECBOOT_MSG_ABORT_OPTIONS_QUIT       0x1
typedef struct
{
    am_secboot_wired_msghdr_t       hdr;
    uint32_t                        options;
} am_secboot_wired_msg_abort_t;

// Reset Device
// Any accumulated OTA's will be processed on next boot
typedef struct
{
    am_secboot_wired_msghdr_t       hdr;
#define AM_SECBOOT_MSG_RESET_OPTIONS_POI        0x1
#define AM_SECBOOT_MSG_RESET_OPTIONS_POR        0x2
    uint32_t                        options;
} am_secboot_wired_msg_reset_t;

#define AM_SECBOOT_MSG_STATUS_WORDS_AMINFO      16
// Message from Device
typedef struct
{
    am_secboot_wired_msghdr_t       hdr;
    uint32_t                        version; // SBL Version
    // Max OTA Blob size that can be downloaded using UPDATE command
    uint32_t                        maxImageSize;
    uint32_t                        currentStatus;
    uint32_t                        currentState;
    // Additional info for Ambiq use only
    uint32_t                        amInfo[AM_SECBOOT_MSG_STATUS_WORDS_AMINFO];
} am_secboot_wired_msg_status_t;

typedef enum
{
    AM_SECBOOT_WIRED_ACK_STATUS_SUCCESS,
    AM_SECBOOT_WIRED_ACK_STATUS_FAILURE,
    AM_SECBOOT_WIRED_ACK_STATUS_INVALID_INFO0,
    AM_SECBOOT_WIRED_ACK_STATUS_CRC,
    AM_SECBOOT_WIRED_ACK_STATUS_SEC,
    AM_SECBOOT_WIRED_ACK_STATUS_MSG_TOO_BIG,
    AM_SECBOOT_WIRED_ACK_STATUS_UNKNOWN_MSGTYPE,
    AM_SECBOOT_WIRED_ACK_STATUS_INVALID_ADDR,
    AM_SECBOOT_WIRED_ACK_STATUS_INVALID_OPERATION,
    AM_SECBOOT_WIRED_ACK_STATUS_INVALID_PARAM,
    AM_SECBOOT_WIRED_ACK_STATUS_SEQ,
    AM_SECBOOT_WIRED_ACK_STATUS_TOO_MUCH_DATA,
} am_secboot_wired_ack_status_e;

typedef struct
{
    am_secboot_wired_msghdr_t       hdr;
    uint32_t                        msgType;
    uint32_t                        status; // am_secboot_wired_ack_status_e
    uint32_t                        seqNum;
} am_secboot_wired_msg_ack_t;

typedef struct
{
    am_secboot_wired_msghdr_t       hdr;
    uint32_t                        seqNum;
} am_secboot_wired_msg_data_t;

#endif // IOS_BOOT_HOST_H

