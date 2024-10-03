//*****************************************************************************
//
//! @file am_util_fat.c
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

#include <stdint.h>
#include <stdbool.h>
#include <stdarg.h>
#include <stddef.h>
#include "am_util_fat.h"
#include "am_util_string.h"
#include "am_mcu_apollo.h"

//*****************************************************************************
//
// Global Variables
//
//*****************************************************************************

//
//! FAT BIOS Parameter Block structure
//
am_util_fat_BPB_t g_psFATBPB =
{
    .BS_jmpBoot     = {0, 0, 0},
    .BS_OEMName     = "Ambiq",
    .BPB_BytsPerSec = AM_FAT_BYTES_PER_SECTOR,
    .BPB_SecPerClus = AM_FAT_SECTORSPERCLUSTER,
    .BPB_RsvdSecCnt = (1 + AM_FAT_NUM_FATS + ((AM_FAT_ROOT_ENTRIES_COUNT * 32) / AM_FAT_BYTES_PER_SECTOR)) * AM_FAT_SECTORSPERCLUSTER,
    .BPB_NumFATs    = AM_FAT_NUM_FATS,
    .BPB_RootEntCnt = AM_FAT_ROOT_ENTRIES_COUNT,
    .BPB_TotSec16   = AM_FAT_FLASH_BYTES / AM_FAT_BYTES_PER_SECTOR,
    .BPB_Media      = 0xF8,
    .BPB_FATSz16    = AM_FAT_FLASH_BYTES / (AM_FAT_BYTES_PER_SECTOR * AM_FAT_SECTORSPERCLUSTER) * 2,
    .BPB_SecPerTrk  = 0,
    .BPB_NumHeads   = 0,
    .BPB_HiddSec    = 0,
    .BPB_TotSec32   = 0,
    .BS_DrvNum      = 0,
    .BS_Reserved1   = 0,
    .BS_BootSig     = 0x29,
    .BS_VolID       = 0x12345678,
    .BS_VolLab      = "AmbiqFAT",
    .BS_FilSysType  = "FAT16",
    .BS_UnnamedDataShouldBe0 = {},
    .Signature_word = 0x55AA
};

am_util_fat_BPB_t *g_psFatCfg = &g_psFATBPB;

//*****************************************************************************
//
// Initializes the file system.
//
//*****************************************************************************
int
am_util_fat_init(am_util_fat_BPB_t *psFatCfg)
{
    //
    // Create a progress counter to return an error code if necessary.
    //
    int ix, iNumPgs;
    int iFunctionProgress = 0;

    if ( psFatCfg == NULL )
    {
        //
        // Use default FAT configuration
        //
        g_psFatCfg = &g_psFATBPB;
    }
    else
    {
        g_psFatCfg = psFatCfg;
    }

    iFunctionProgress++;
    if ( g_psFatCfg->BPB_BytsPerSec != 512 )
    {
        return iFunctionProgress;
    }

    iFunctionProgress++;
    if ( g_psFatCfg->BPB_RsvdSecCnt != (1 + AM_FAT_NUM_FATS + ((AM_FAT_ROOT_ENTRIES_COUNT * 32) / AM_FAT_BYTES_PER_SECTOR)) * AM_FAT_SECTORSPERCLUSTER )
    {
        return iFunctionProgress;
    }

    iFunctionProgress++;
    if ( g_psFatCfg->BPB_TotSec16 != (AM_FAT_FLASH_BYTES / AM_FAT_BYTES_PER_SECTOR) )
    {
        return iFunctionProgress;
    }

    iFunctionProgress++;
    if ( g_psFatCfg->BPB_Media != 0xF8 )
    {
        return iFunctionProgress;
    }

    iFunctionProgress++;
    if ( g_psFatCfg->BPB_FATSz16 != (AM_FAT_FLASH_BYTES / (AM_FAT_BYTES_PER_SECTOR * AM_FAT_SECTORSPERCLUSTER) * 2) )
    {
        return iFunctionProgress;
    }

    iFunctionProgress++;
    if ( (g_psFatCfg->BPB_SecPerTrk != 0 )      ||
         (g_psFatCfg->BPB_NumHeads  != 0 )      ||
         (g_psFatCfg->BPB_HiddSec   != 0 )      ||
         (g_psFatCfg->BPB_TotSec32  != 0 )      ||
         (g_psFatCfg->BPB_HiddSec   != 0 )      ||
         (g_psFatCfg->BS_DrvNum     != 0 )      ||
         (g_psFatCfg->BS_Reserved1  != 0 ) )
    {
        return iFunctionProgress;
    }

    iFunctionProgress++;
    if ( g_psFatCfg->BS_BootSig != 0x29 )
    {
        return iFunctionProgress;
    }

    iFunctionProgress++;
#if 1
    if ( am_util_string_strncmp((const char*)g_psFatCfg->BS_FilSysType, "FAT16", 6) )
    {
        return iFunctionProgress;
    }
#else
    if ( (g_psFatCfg->BS_FilSysType[0] != 'F')  ||
         (g_psFatCfg->BS_FilSysType[1] != 'A')  ||
         (g_psFatCfg->BS_FilSysType[2] != 'T')  ||
         (g_psFatCfg->BS_FilSysType[3] != '1')  ||
         (g_psFatCfg->BS_FilSysType[4] != '6')  ||
         (g_psFatCfg->BS_FilSysType[5] != 0x00) )
        return iFunctionProgress;
#endif

    iFunctionProgress++;
    for ( ix = 0; ix < sizeof(g_psFatCfg->BS_UnnamedDataShouldBe0); ix++ )
    {
        if ( g_psFatCfg->BS_UnnamedDataShouldBe0[ix] != 0x00 )
        {
            return iFunctionProgress;
        }
    }

    //
    // Okay, FAT table looks reasonable.  Now, let's save it to flash.
    // Start by initializing (erasing) the flash pages where the entire
    //  file system will reside.
    //
    iFunctionProgress++;
    iNumPgs = AM_FAT_TOTALCLUSTERS / AM_FAT_CLUSTERSPERFLASHPAGE;
    for ( ix = 0; ix < iNumPgs; ix++ )
    {
        uint32_t uAddr = AM_FAT_FLASH_ADDRESS + ix * AM_HAL_FLASH_PAGE_SIZE;
        am_hal_flash_page_erase(AM_HAL_FLASH_PROGRAM_KEY,
                                AM_HAL_FLASH_ADDR2INST(uAddr),
                                AM_HAL_FLASH_ADDR2PAGE(uAddr) );
    }

#if 0
    //
    // All pages to be used by the file system are erased.
    // Now, program in the BPB and FAT.
    //
    am_hal_flash_program_main(AM_HAL_FLASH_PROGRAM_KEY,
                              g_psFatCfg,

    am_hal_flash_program_main(uint32_t ui32Value, uint32_t *pui32Src,
                          uint32_t *pui32Dst, uint32_t ui32NumWords)
#endif

    return 0;
}

//*****************************************************************************
//
// End Doxygen group.
//! @}
//
//*****************************************************************************

