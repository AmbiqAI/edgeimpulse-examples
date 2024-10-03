//*****************************************************************************
//
//! @file common_emmc_fatfs.c
//!
//! @brief Shared instances and functions involving eMMC and FatFS
//!
//*****************************************************************************

#include "complex_test_cases.h"
#include "common_emmc_fatfs.h"
#include "task.h"

SemaphoreHandle_t g_xFatFsMutex; /* Mutex for FatFs thread safety*/
SemaphoreHandle_t g_xFsScrubMutex; /* Mutex preventing filesystem scrubbing */

AM_SHARED_RW FATFS g_sEMMCFatFs ALIGN(32);  /* Filesystem object */
AM_SHARED_RW uint8_t pui8FormatWorkBuf[FF_MAX_SS] ALIGN(32); /* working buffer for format process */
AM_SHARED_RW char eMMCPath[4] ALIGN(32);
am_hal_card_t eMMCs;

//*****************************************************************************
//
// Initialize or reinitialize the FS
//
//*****************************************************************************
bool scrub_fs(void)
{
    FRESULT res;
    xSemaphoreTake(g_xFatFsMutex, portMAX_DELAY);
    res = f_mount(&g_sEMMCFatFs, (TCHAR const*)eMMCPath, 0);
    if (res != FR_OK)
        return false;
    res = f_mkfs((TCHAR const*)eMMCPath, 0, pui8FormatWorkBuf, sizeof(pui8FormatWorkBuf));
    if (res != FR_OK)
        return false;
    res = f_mkdir(FILE_DIR_TEMP);
    if (res != FR_OK)
        return false;
    res = f_mkdir(FILE_DIR_MUSIC);
    if (res != FR_OK)
        return false;
    xSemaphoreGive(g_xFatFsMutex);
    return true;
}

//*****************************************************************************
//
// Initialize EMMC and FatFS for use by other tasks
//
//*****************************************************************************

bool EMMCFatFsSetup(void)
{
    g_sEMMC_config.eHost = AM_HAL_SDHC_CARD_HOST;
    g_sEMMC_hw.pDevHandle = &eMMCs; // Card state structure is not in mmc_apollo5.c for some reason
    // TODO: revisit SDIO bus speeds when updating FPGAs
    g_sEMMC_config.ui32Clock = 48000000; //48MHz
    g_sEMMC_config.eBusWidth = AM_HAL_HOST_BUS_WIDTH_8;
    g_sEMMC_config.eUHSMode = AM_HAL_HOST_UHS_DDR50;

    if(g_xFatFsMutex == NULL)
    {
        g_xFatFsMutex = xSemaphoreCreateMutex();
        g_xFsScrubMutex = xSemaphoreCreateMutex();
        configASSERT(g_xFatFsMutex != NULL);
        configASSERT(g_xFsScrubMutex != NULL);
        if(!scrub_fs())
            return false;
        am_util_stdio_printf("EMMC and FatFS ready\r\n");
    }
    return true;
}
