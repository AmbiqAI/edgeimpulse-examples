//*****************************************************************************
//
//! @file common_emmc_fatfs.h
//!
//! @brief Shared instances and structures involving emmc and FatFS
//!
//*****************************************************************************

#ifndef COMMON_EMMC_FATFS_H
#define COMMON_EMMC_FATFS_H

#include "mmc_apollo5.h"
#include "FreeRTOS.h"
#include "semphr.h"

extern SemaphoreHandle_t g_xFatFsMutex; /* Mutex over FatFS eMMC access */
extern SemaphoreHandle_t g_xFsScrubMutex; /* Mutex preventing filesystem scrubbing */


extern FATFS g_sEMMCFatFs;  /* Filesystem object */


//*****************************************************************************
//
// External function definitions.
//
//*****************************************************************************

bool EMMCFatFsSetup(void);
bool scrub_fs(void);

#endif //COMMON_EMMC_FATFS_H