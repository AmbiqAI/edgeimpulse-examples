//*****************************************************************************
//
//! @file FlashDev.c
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

/***********************************************************************/
/*  This file is part of the ARM Toolchain package                     */
/*  Copyright KEIL ELEKTRONIK GmbH 2003 - 2006                         */
/***********************************************************************/
/*                                                                     */
/*  FlashDev.C:  Device Description for Ambiq Micro Apollo chip        */
/*                                                                     */
/***********************************************************************/

#include "FlashOS.h"        // FlashOS Structures


struct FlashDevice const FlashDevice  =
{
    FLASH_DRV_VERS,             // Vers:     Driver Version, do not modify!
    AM_DEVNAME,                 // DevName[128]: Device Name
    ONCHIP,                     // DevType:  Device Type
    AM_STARTADDR,               // DevAdr:   Device Start Address
    AM_NVSIZE,                  // szDev:    Device Size in Bytes (4MB - Apollo5)
    AM_PGSIZE,                  // szPage:   Programming Page Size
    0,                          // Res:      Reserved, must be 0
    0xFF,                       // valEmpty: Initial Content of Erased Memory
    5000,                       // toProg:   Program Page Timeout 1000 mSec (Stingray with SBL, 500 was not enough)
    5000,                       // toErase:  Erase Sector Timeout 1000 mSec
    // Specify Size and Address of Sectors
    // (Stingray 8KB sectors) (256 sectors beginning at addr 0x0)
    // (Apollo 2KB) (256 Sectors), 2KB=0x800
    AM_PGSIZE, 0x00000000,      // sectors[SECTOR_NUM]: Sector Size



/*
#if AM_DEVICE == AM_DEV_APOLLO4 // Apollo4 (Falcon)
#if AM_DEV_SBL  // SBL is the normal case
    "Ambiq Micro Apollo4",      // DevName[128]: Device Name
    ONCHIP,                     // DevType:  Device Type
    0x0000C000,                 // DevAdr:   Device Start Address
    0x000F4000,                 // szDev:    Device Size in Bytes (1MB - Apollo4)
#else           // RAW
    #warning Building RAW version!
    "Ambiq Micro Apollo4 (raw)",// DevName[128]: Device Name
    ONCHIP,                     // DevType:  Device Type
    0x00000000,                 // DevAdr:   Device Start Address
    0x00100000,                 // szDev:    Device Size in Bytes (1MB - Apollo4)
#endif
    8 * 1024,                   // szPage:   Programming Page Size
    0,                          // Res:      Reserved, must be 0
    0xFF,                       // valEmpty: Initial Content of Erased Memory
    1000,                       // toProg:   Program Page Timeout 500 mSec (Stingray with SBL, 500 was not enough)
    1000,                       // toErase:  Erase Sector Timeout 100 mSec
    // Specify Size and Address of Sectors
    // (Stingray 8KB sectors) (256 sectors beginning at addr 0x0)
    0x00002000, 0x00000000,     // sectors[SECTOR_NUM]: Sector Size
#elif AM_DEVICE == AM_DEV_APOLLO3       // Apollo3 (Corvette)
#if AM_DEVICE == AM_DEV_APOLLO3P        // Apollo3 Blue Plus (Stingray)
    "Ambiq Micro Apollo3 Blue Plus",    // DevName[128]: Device Name
    ONCHIP,                     // DevType:  Device Type
#if AM_DEV_SBL  // SBL is the normal case
    0x0000C000,                 // DevAdr:   Device Start Address
    0x001F4000,                 // szDev:    Device Size in Bytes (2MB - Apollo3p)
#else           // RAW
    #warning Building RAW version!
    0x00000000,                 // DevAdr:   Device Start Address
    0x00200000,                 // szDev:    Device Size in Bytes (2MB - Apollo3p)
#endif
    8 * 1024,                   // szPage:   Programming Page Size
    0,                          // Res:      Reserved, must be 0
    0xFF,                       // valEmpty: Initial Content of Erased Memory
    1000,                       // toProg:   Program Page Timeout 500 mSec (Stingray with SBL, 500 was not enough)
    1000,                       // toErase:  Erase Sector Timeout 100 mSec
    // Specify Size and Address of Sectors
    // (Stingray 8KB sectors) (256 sectors beginning at addr 0x0)
    0x00002000, 0x00000000,     // sectors[SECTOR_NUM]: Sector Size
#elif AM_DEVICE == AM_DEV_APOLLO3       // Apollo3 (Corvette)
    "Ambiq Micro Apollo3 Blue", // DevName[128]: Device Name
    ONCHIP,                     // DevType:  Device Type
    0x00000000,                 // DevAdr:   Device Start Address
    0x00100000,                 // szDev:    Device Size in Bytes (1MB - Apollo3)
    8 * 1024,                   // szPage:   Programming Page Size
    0,                          // Res:      Reserved, must be 0
    0xFF,                       // valEmpty: Initial Content of Erased Memory
    500,                        // toProg:   Program Page Timeout 500 mSec (Shelby FPGA)
    100,                        // toErase:  Erase Sector Timeout 100 mSec
    // Specify Size and Address of Sectors
    // (Corvette 8KB sectors) (128 sectors beginning at addr 0x0)
    0x00002000, 0x00000000,     // sectors[SECTOR_NUM]: Sector Size
#elif AM_DEVICE == AM_DEV_APOLLO2       // Apollo2 (Shelby)
    "Ambiq Micro Apollo 2 (and Blue)",  // DevName[128]: Device Name
    ONCHIP,                     // DevType:  Device Type
    0x000000,                   // DevAdr:   Device Start Address

  //0x00020000,                 // szDev:    Device Size in Bytes (128KB - Shelby FPGA)
    0x00100000,                 // szDev:    Device Size in Bytes (1MB - Apollo2)
    8*1024,                     // szPage:   Programming Page Size
    0,                          // Res:      Reserved, must be 0
    0xFF,                       // valEmpty: Initial Content of Erased Memory

    500,                        // toProg:   Program Page Timeout 500 mSec (Shelby FPGA)
    100,                        // toErase:  Erase Sector Timeout 100 mSec
    // Specify Size and Address of Sectors
    // (Shelby 8KB) (128 Sectors), 8KB=0x2000
    0x00002000, 0x00000000,     // sectors[SECTOR_NUM]: Sector Size
#elif AM_DEVICE == AM_DEV_APOLLO        // Apollo (Torino)
    "Ambiq Micro Apollo",       // DevName[128]: Device Name
    ONCHIP,                     // DevType:  Device Type
    0x000000,                   // DevAdr:   Device Start Address

    0x00080000,                 // szDev:    Device Size in Bytes (Apollo = 512KB)
    2*1024,                     // szPage:   Programming Page Size
    0,                          // Res:      Reserved, must be 0
    0xFF,                       // valEmpty: Initial Content of Erased Memory

    100,                        // toProg:   Program Page Timeout 100 mSec
    3000,                       // toErase:  Erase Sector Timeout 3000 mSec
    // Specify Size and Address of Sectors
    // (Apollo 2KB) (256 Sectors), 2KB=0x800
    0x00000800, 0x00000000,     // sectors[SECTOR_NUM]: Sector Size
#else
#error AM_DEVICE invalid!
#endif
*/

    SECTOR_END
};
