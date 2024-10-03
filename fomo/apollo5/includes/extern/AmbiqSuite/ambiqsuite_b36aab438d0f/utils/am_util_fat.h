//*****************************************************************************
//
//! @file am_util_fat.h
//!
//! @brief A simple FAT file system for use with Ambiq Micro products.
//!
//! These functions set up a partition in flash to use for file I/O.
//!
//! @addtogroup fat FAT - FAT file system
//! @ingroup utils
//! @{
//
//*****************************************************************************

//*****************************************************************************
//
// ${copyright}
//
// This is part of revision ${version} of the AmbiqSuite Development Package.
//
//*****************************************************************************
#ifndef AM_UTIL_FAT_H
#define AM_UTIL_FAT_H

#include "am_mcu_apollo.h"

#ifdef __cplusplus
extern "C"
{
#endif
//*****************************************************************************
//
// Macro definitions
//
//*****************************************************************************

// #### INTERNAL BEGIN ####
// Some of these defines may be more suited as block-level defines and may
//  need to be moved to devices.
// #### INTERNAL END ####

//
// Define the flash partition that will contain the file system.
//

//
//! This particular partition uses the latter half of the 2nd instance.
//
#define AM_FAT_FLASH_ADDRESS            (0x00000000 + ((2*256*3/4) * 1024))
#define AM_FAT_FLASH_BYTES              (256/2 * 1024)

//
//! Define the bytes per sector.  By FAT history and convention, this value
//!  is expected to be 512.
//
#define AM_FAT_BYTES_PER_SECTOR         512

//
//! Define the sectors per cluster. For the initial implementation, this
//!  value is 1. It would only need to be increased if a very large flash
//!  partition is used for a file system, which would almost certainly require
//!  use of external flash.
//
#define AM_FAT_SECTORSPERCLUSTER        1

//
//! Define the number of FAT tables. This value is either 1 for a single
//!  allocation table or 2 to create a backup table. No other values are allowed.
//
#define AM_FAT_NUM_FATS                 1

//
//! Define the count of root directory entries. This value includes the 2 default
//!  entries for "." and "..".  Therefore, the actual number of entries is 2 less
//!  than the defined value.
//! Note that the more root entries that are allocated, the larger the FAT table
//!  becomes.
//
#define AM_FAT_ROOT_ENTRIES_COUNT       32

//
//! The number of clusters is determined from previously specified parameters.
//! Note that the count of clusters does not include the 2 reserved clusters.
//
#define AM_FAT_TOTALSECTORS             (AM_FAT_FLASH_BYTES/AM_FAT_BYTES_PER_SECTOR)
#define AM_FAT_TOTALCLUSTERS            (AM_FAT_TOTALSECTORS/AM_FAT_SECTORSPERCLUSTER)
#define AM_FAT_COUNTOFCLUSTERS          (AM_FAT_TOTALCLUSTERS-2)

//
//! Associate sectors and clusters with flash page size.
//
#define AM_FAT_SECTORSPERFLASHPAGE      (AM_HAL_FLASH_PAGE_SIZE / AM_FAT_BYTES_PER_SECTOR)
#define AM_FAT_CLUSTERSPERFLASHPAGE     (AM_HAL_FLASH_PAGE_SIZE / (AM_FAT_BYTES_PER_SECTOR * AM_FAT_SECTORSPERCLUSTER))

//
// Set structure alignment to 1 byte
//
#pragma pack(1)
//*****************************************************************************
//
//! BIOS Parameter Block (BPB) structure
//
//*****************************************************************************
typedef struct
{

    //
    //! Offsets 0 through 35 make up the original FAT8 BS and BPB.
    //

    //
    //!  0: Boot sector
    //

    //
    //!  0: Jump instruction to boot code
    //
    volatile uint8_t  BS_jmpBoot[3];

    //
    //!  3: OEM Name identifier
    //
    volatile uint8_t  BS_OEMName[8];

    //
    //! 11: Count of Bytes per Sector
    //
    volatile uint16_t BPB_BytsPerSec;

    //
    //! 13: Number of Sectors per Allocation Unit
    //
    volatile uint8_t  BPB_SecPerClus;

    //
    //! 14: Number of reserved sectors in the reserved region of the volume
    //
    volatile uint16_t BPB_RsvdSecCnt;

    //
    //! 16: Count of FATs
    //
    volatile uint8_t  BPB_NumFATs;

    //
    //! 17: Count of 32-byte directory entries in the root directory
    //
    volatile uint16_t BPB_RootEntCnt;

    //
    //! 19: Total count of sectors on the volume. This count includes the count
    //!     of all sectors in all four regions of the volume
    //
    volatile uint16_t BPB_TotSec16;

    //
    //! 21: Legal values: 0xF0,0xF8-0xFF
    //!     0xF8 = fixed (non-removable) media
    //!     0xF0 = removable media.
    //
    volatile uint8_t  BPB_Media;

    //
    //! 22: 16-bit count of sectors occupied by one FAT.
    //
    volatile uint16_t BPB_FATSz16;

    //
    //! 24: Sectors per track for interrupt 0x13.
    //!     Set to 0 for Ambiq implementation
    //
    volatile uint16_t BPB_SecPerTrk;

    //
    //! 26: Number of heads for interrupt 0x13.
    //!     Set to 0 for Ambiq implementation
    //
    volatile uint16_t BPB_NumHeads;

    //
    //! 28: Count of hidden sectors receding the partition that contains the FAT
    //!     volume. Only relevant for media visible on interrupt 0x13.
    //!     Set to 0 for Ambiq implementation
    //
    volatile uint32_t BPB_HiddSec;

    //
    //! 32: Total count of sectors on the volume.
    //!     Set to 0 for FAT16.
    //
    volatile uint32_t BPB_TotSec32;

    //
    //! Offsets 36 thru end-of-sector make up the Extended BPB for FAT12 and
    //!  FAT16 volumes.
    //

    //
    //!  36: Interrupt 0x13 drive number. Set value to 0x80 or 0x00.
    //
    volatile uint8_t  BS_DrvNum;

    //
    //!  37: Reserved. Set value to 0x0.
    //
    volatile uint8_t  BS_Reserved1;

    //
    //!  38: Extended boot signature.  Set to 0x29 if either of the following
    //!      two fields are non-zero.
    //
    volatile uint8_t  BS_BootSig;

    //
    //!  39: Volume serial number.
    //
    volatile uint32_t BS_VolID;

    //
    //!  43: Volume label. Matches the 11-byte volume label recorded in the root
    //!      directory.
    //
    volatile uint8_t  BS_VolLab[11];

    //
    //!  54: The string "FAT16"
    //!      directory.
    //
    volatile uint8_t  BS_FilSysType[8];

    //
    //!  62: Set to 0x00
    //
    volatile uint8_t  BS_UnnamedDataShouldBe0[448];

    //
    //!  510: Set to 0x55 (at offset 510) and 0xAA (at offset 511)
    //
    volatile uint16_t  Signature_word;

    //
    //!  512: All remaining bytes in the sector set to 0x00
    //!       (ony for media where BPB_BytsPerSec > 512)
    //
} am_util_fat_BPB_t;

//
// Restore the default structure alignment
//
#pragma pack()

//*****************************************************************************
//
// External function definitions
//
//*****************************************************************************
//*****************************************************************************
//
//! @brief Initializes the file system.
//!
//! This function initializes the global file system functions which are used
//! for file I/O.
//
//! @param psFatCfg - pointer to FAT configuration
//!
//! @return 0 if successful.
//
//*****************************************************************************
extern int am_util_fat_init(am_util_fat_BPB_t *psFatCfg);

#ifdef __cplusplus
}
#endif

#endif // AM_UTIL_FAT_H

//*****************************************************************************
//
// End Doxygen group.
//! @}
//
//*****************************************************************************

