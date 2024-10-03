//*****************************************************************************
//
//! @file app_task.c
//!
//! @brief A simple SDMMC erase unlock and write protection removing example.
//!
//! This example use nex_api functions 'MMCLockUnlock' and 'eMMCPackedRead' to
//! lock, unlock the sd card, set and clear password.
//
//*****************************************************************************

//*****************************************************************************
//
// ${copyright}
//
// This is part of revision ${version} of the AmbiqSuite Development Package.
//
//*****************************************************************************

#include <stdlib.h>
#include <string.h>

#include "FreeRTOS.h"
#include "semphr.h"
#include "task.h"

#include "nex_api.h"

#include "am_bsp.h"
#include "am_mcu_apollo.h"
#include "am_util.h"

#define APP_PRINT(format, ...)  am_util_stdio_printf("[APP] %33s %04d | " format, __func__, __LINE__, ##__VA_ARGS__)

#define SDHC 0
#define SLOT 0

TaskHandle_t app_task_handle;

void am_sdio_isr(void)
{
    nex_irqhandler(SDHC, SLOT);
}

static NEX_MMC_CSD_V4  toshiba_emmc_csd;
static NEX_MMC_EXT_CSD toshiba_emmc_ext_csd;

static uint32_t emmc_capacity;
static uint32_t data_sector_size;
static uint32_t erase_unit_size;
static uint32_t write_prot_grp_size;

static void dump_write_protect_setting(NEX_MMC_CSD_V4 *csd, NEX_MMC_EXT_CSD *ext_csd)
{
    APP_PRINT("\n");
    APP_PRINT("===== CSD write protection information =====\n");
    APP_PRINT("CSD - Device size                : 0x%x\n", csd->c_size);
    APP_PRINT("CSD - Device size multiplier     : 0x%x\n", csd->c_size_mult);

    APP_PRINT("CSD - Erase group size           : 0x%x\n", csd->erase_grp_size);
    APP_PRINT("CSD - Erase group size multiplier: 0x%x\n", csd->erase_grp_mult);

    APP_PRINT("CSD - Write protect group size   : 0x%x\n", csd->wp_grp_size);
    APP_PRINT("CSD - Write protect enable       : 0x%x\n", csd->wp_grp_enable);

    APP_PRINT("CSD - Permanent write protection : 0x%x\n", csd->perm_write_protect);
    APP_PRINT("CSD - Temporary write protection : 0x%x\n", csd->tmp_write_prot);

    APP_PRINT("\n");
    APP_PRINT("===== EXT_CSD write protection information =====\n");
    APP_PRINT("EXT_CSD - Sector Count                           : 0x%x\n", ext_csd->sec_count);
    APP_PRINT("EXT_CSD - high-capacity erase-unit size          : 0x%x\n", ext_csd->hc_erase_grp_size);
    APP_PRINT("EXT_CSD - High-density erase group definition    : 0x%x\n", ext_csd->erase_grp_defn);
    APP_PRINT("EXT_CSD - High-capacity write protect group size : 0x%x\n", ext_csd->hc_wp_grp_size);
    APP_PRINT("EXT_CSD - Native sector size                     : 0x%x\n", ext_csd->native_sector_size);
    APP_PRINT("EXT_CSD - Sector size emulation                  : 0x%x\n", ext_csd->use_native_sector);
    APP_PRINT("EXT_CSD - Sector size                            : 0x%x\n", ext_csd->data_sector_size);
    APP_PRINT("EXT_CSD - User write protection                  : 0x%x\n", ext_csd->user_wp);

    APP_PRINT("\n");
    data_sector_size = ext_csd->data_sector_size ? 4096 : 512;
    emmc_capacity = ext_csd->sec_count*data_sector_size;
    APP_PRINT("eMMC capacity is                 : %d MB\n", emmc_capacity / (1024 * 1024));

    // Erase Unit Size = 512Kbyte HC_ERASE_GRP_SIZE
    erase_unit_size = 512 * ext_csd->hc_erase_grp_size * 1024;
    APP_PRINT("eMMC erase unit size is          : %d MB\n", erase_unit_size / (1024 * 1024));

    write_prot_grp_size = erase_unit_size * ext_csd->hc_wp_grp_size;
    // Write protect group size = 512KB * HC_ERASE_GRP_SIZE * HC_WP_GRP_SIZE
    APP_PRINT("eMMC write protect group size is : %d MB\n", write_prot_grp_size / (1024 * 1024));
}

static void clear_all_write_grp_setting(void)
{
    // Clear all write protection groups
    uint32_t cnt = 0;
    uint32_t wp_sector;
    uint32_t total_sectors = emmc_capacity / data_sector_size;
    uint32_t wp_grp_sectors = write_prot_grp_size / data_sector_size;

    APP_PRINT("clearing all the write protections ...\n");
    for (wp_sector = 0x0; wp_sector < total_sectors; wp_sector += wp_grp_sectors)
    {
        eMMCClrWriteProt(wp_sector, SDHC, SLOT);
        am_util_stdio_printf(".");
        if (cnt++ == 84)
        {
          am_util_stdio_printf("\n");
          cnt = 0;
        }
    }
    am_util_stdio_printf("\n");
    APP_PRINT("done\n");
}

static void erase_unlock_device(void)
{
    struct mmc_lock_data data;

    //
    // erase the device lock unconditionally
    //
    memset(&data, 0, sizeof(struct mmc_lock_data));

    data.erase = 1;
    MMCLockUnlock(data, SDHC, SLOT);

    uint32_t erase_unlock_status;
    do
    {
        // APP_PRINT("waiting 1s\n");
        // vTaskDelay(pdMS_TO_TICKS(1000));
        APP_PRINT("Checking the device status\n");
        GetDeviceStatus_CMD13(&erase_unlock_status, SDHC, SLOT);
        APP_PRINT("Device status : 0x%x\n", erase_unlock_status);
    } while (erase_unlock_status & (0x1 << 25));

    APP_PRINT("Device erase unLocked successfully\n");
}

void app_emmc_recovery_task(void *pvParameters)
{

    while (IsCardInitialized(SDHC, SLOT) == SD_FALSE)
    {
        vTaskDelay(pdMS_TO_TICKS(10));
        APP_PRINT("Checking if the card is ready again\n");
    }

    if (MMC_MODE_eMMC == CardType(SDHC, SLOT))
    {
        goto _not_emmc_device;
    }

    //
    // Get card specific write protection information from CSD and EXT_CSD
    //
    GetCardSpecificInfo_CSD(&toshiba_emmc_csd, SDHC, SLOT);
    GetCardSpecificInfo_ExtCSD(&toshiba_emmc_ext_csd, SDHC, SLOT);
    dump_write_protect_setting(&toshiba_emmc_csd, &toshiba_emmc_ext_csd);

    //
    // erase unlock the emmc device
    //
    erase_unlock_device();

    //
    // remove all write protection grp settings
    //
    clear_all_write_grp_setting();

    APP_PRINT("eMMC device is recovered without any password or write protections\n");
    goto _suspend;

_not_emmc_device:
    APP_PRINT("Not an eMMC device\n");
_suspend:
    vTaskSuspend(NULL);
}
