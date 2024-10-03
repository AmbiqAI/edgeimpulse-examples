//*****************************************************************************
//
//! @file FlashOS.h
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

/* -----------------------------------------------------------------------------
 * Copyright (c) 2014 ARM Ltd.
 *
 * This software is provided 'as-is', without any express or implied warranty.
 * In no event will the authors be held liable for any damages arising from
 * the use of this software. Permission is granted to anyone to use this
 * software for any purpose, including commercial applications, and to alter
 * it and redistribute it freely, subject to the following restrictions:
 *
 * 1. The origin of this software must not be misrepresented; you must not
 *    claim that you wrote the original software. If you use this software in
 *    a product, an acknowledgment in the product documentation would be
 *    appreciated but is not required.
 *
 * 2. Altered source versions must be plainly marked as such, and must not be
 *    misrepresented as being the original software.
 *
 * 3. This notice may not be removed or altered from any source distribution.
 *
 *
 * $Date:        14. Jan 2014
 * $Revision:    V1.00
 *
 * Project:      FlashOS Headerfile for Flash drivers
 * --------------------------------------------------------------------------- */

/* History:
 *  Version 1.00
 *    Initial release
 */

#define VERS       1           // Interface Version 1.01

#define UNKNOWN    0           // Unknown
#define ONCHIP     1           // On-chip Flash Memory
#define EXT8BIT    2           // External Flash Device on 8-bit  Bus
#define EXT16BIT   3           // External Flash Device on 16-bit Bus
#define EXT32BIT   4           // External Flash Device on 32-bit Bus
#define EXTSPI     5           // External Flash Device on SPI

// *****************************************************************************
// Ambiq Micro specific
// *****************************************************************************
//
// Define the following AM_DEVICE as:
//  1  = Apollo
//  2  = Apollo2  (Shelby, Apollo2 and Apollo2 Blue)
//  3  = Apollo3  (Corvette, Apollo3 Blue)
//  3p = Apollo3p (Stingray, Apollo3 Blue Plus)
//  4  = Apollo4  (Falcon)
//  5  = Apollo5  (Cayenne)
//

#define AM_DEV_APOLLO       1
#define AM_DEV_APOLLO2      2
#define AM_DEV_APOLLO3      3
#define AM_DEV_APOLLO3P     (3 + 'p')   // Apollo3 Blue Plus (Stingray)
#define AM_DEV_APOLLO4      4
#define AM_DEV_APOLLO5      5

#ifndef AM_DEVICE
#define AM_DEVICE           AM_DEV_APOLLO5
#endif

#ifndef AM_DEV_SBL
#define AM_DEV_SBL          0           // 0=RAW, 1=SBL (normal case)
#endif


//
// Determine device definitions.
//
#if AM_DEV_SBL  // SBL is the normal case
#else
//#warning Building RAW version!
#endif

#if AM_DEVICE == AM_DEV_APOLLO5
//
// Apollo5 (Cayenne)
// The project file contains 3 targets, each target defines the following.
//
#if AM_DEV_SBL  // SBL is the normal case
#ifndef AM_DEVNAME
#define AM_DEVNAME      "Apollo5"
#endif
#ifndef AM_STARTADDR
//#define AM_STARTADDR    0x0
#define AM_STARTADDR    0x00410000  // Assumes SBL occupies 1st 64KB
#endif
#ifndef AM_NVSIZE
#define AM_NVSIZE       0x003F0000  // Total NVRAM size
#endif
#else           // RAW
#ifndef AM_DEVNAME
#define AM_DEVNAME      "Apollo5nbl"
#endif
#ifndef AM_STARTADDR
#define AM_STARTADDR    0x00400000
#endif
#ifndef AM_NVSIZE
#define AM_NVSIZE       0x00200000
#endif
#endif // AM_DEV_SBL
#define AM_PGSIZE       (8 * 1024)

#elif AM_DEVICE == AM_DEV_APOLLO4
//
// Apollo4 (Falcon)
// The project file contains 3 targets, each target defines the following.
//
#if AM_DEV_SBL  // SBL is the normal case
#ifndef AM_DEVNAME
#define AM_DEVNAME      "Apollo4"
#endif
#ifndef AM_STARTADDR
//#define AM_STARTADDR    0x0
#define AM_STARTADDR    0x00018000
#endif
#ifndef AM_NVSIZE
#define AM_NVSIZE       0x00200000  // Total NVRAM size
#endif
#else           // RAW
#ifndef AM_DEVNAME
#define AM_DEVNAME      "Apollo4nbl"
#endif
#ifndef AM_STARTADDR
#define AM_STARTADDR    0x00000000
#endif
#ifndef AM_NVSIZE
#define AM_NVSIZE       0x00200000
#endif
#endif // AM_DEV_SBL
#define AM_PGSIZE       (8 * 1024)


#elif AM_DEVICE == AM_DEV_APOLLO3P
//
// Apollo3p (Stingray)
//
#define AM_DEVNAME  "Ambiq Micro Apollo3 Blue Plus"
#if AM_DEV_SBL
#define AM_STARTADDR    0x0000C000
#define AM_NVSIZE       0x001F4000  // 2MB flash
#else           // RAW
#define AM_STARTADDR    0x00000000
#define AM_NVSIZE       0x00200000  // 2MB flash
#endif
#define AM_PGSIZE       (8 * 1024)


#elif AM_DEVICE == AM_DEV_APOLLO3
//
// Apollo3 (Corvette)
//
#define AM_DEVNAME  "Ambiq Micro Apollo3 Blue Plus"
#if AM_DEV_SBL
#define AM_STARTADDR    0x0000C000
#define AM_NVSIZE       0x000F4000
#else           // RAW
#define AM_STARTADDR    0x00000000
#define AM_NVSIZE       0x00100000  // 1MB flash
#endif
#define AM_PGSIZE       (8 * 1024)



#elif AM_DEVICE == AM_DEV_APOLLO2
//
// Apollo2 (Shelby)
//
#define AM_DEVNAME  "Ambiq Micro Apollo 2 (and Blue)"
#define AM_STARTADDR    0x00000000
#define AM_NVSIZE       0x00100000  // 1MB flash
#define AM_PGSIZE       (8 * 1024)



#elif AM_DEVICE == AM_DEV_APOLLO
//
// Apollo (Torino)
//
#define AM_DEVNAME  "Ambiq Micro Apollo",       // DevName[128]: Device Name
#define AM_STARTADDR    0x00000000
#define AM_NVSIZE       0x00080000  // 512KB flash
#define AM_PGSIZE       (2 * 1024)

#else
#error AM_DEVICE invalid!
#endif












//
// It's unclear why, but SECTOR_NUM must be 512 for any device. Otherwise, with
//  anything smaller (for some reason) the structure gets corrupted at run time.
//  This is unfortunate because it adds 4KB of wasted space to the size of SRAM
//  needed.  Also, PAGE_MAX doesn't seem to be used and I commented it out.
//  - Ron 12/14/15
//
// *****************************************************************************

#define SECTOR_NUM 512          // Max Number of Sector Items
//#define PAGE_MAX   65536       // Max Page Size for Programming (Not Used)


struct FlashSectors
{
  unsigned long   szSector;    // Sector Size in Bytes
  unsigned long AddrSector;    // Address of Sector
};

#define SECTOR_END 0xFFFFFFFF, 0xFFFFFFFF

struct FlashDevice
{
   unsigned short     Vers;    // Version Number and Architecture
   char       DevName[128];    // Device Name and Description
   unsigned short  DevType;    // Device Type: ONCHIP, EXT8BIT, EXT16BIT, ...
   unsigned long    DevAdr;    // Default Device Start Address
   unsigned long     szDev;    // Total Size of Device
   unsigned long    szPage;    // Programming Page Size
   unsigned long       Res;    // Reserved for future Extension
   unsigned char  valEmpty;    // Content of Erased Memory

   unsigned long    toProg;    // Time Out of Program Page Function
   unsigned long   toErase;    // Time Out of Erase Sector Function

   struct FlashSectors sectors[SECTOR_NUM];
};

#define FLASH_DRV_VERS (0x0100 + VERS)  // Driver Version, do not modify!

// Flash Programming Functions (Called by FlashOS)
extern          int  Init        (unsigned long adr,   // Initialize Flash
                                  unsigned long clk,
                                  unsigned long fnc);
extern          int  UnInit      (unsigned long fnc);  // De-initialize Flash
extern          int  BlankCheck  (unsigned long adr,   // Blank Check
                                  unsigned long sz,
                                  unsigned char pat);
extern          int  EraseChip   (void);               // Erase complete Device
extern          int  EraseSector (unsigned long adr);  // Erase Sector Function
extern          int  ProgramPage (unsigned long adr,   // Program Page Function
                                  unsigned long sz,
                                  unsigned char *buf);
extern unsigned long Verify      (unsigned long adr,   // Verify Function
                                  unsigned long sz,
                                  unsigned char *buf);
