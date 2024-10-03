//*****************************************************************************
//
//! @file app_task.c
//!
//! @brief A simple SDMMC lock&unlock test example.
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

#define ALIGN(x) __attribute__((aligned(1 << x)))

#define HIGH_CAPACITY
#define TEST_SECROR 0x0

#ifdef HIGH_CAPACITY
#define TEST_ADDR TEST_SECROR
#else
#define TEST_ADDR TEST_SECROR*512
#endif

#define SDHC 0
#define SLOT 0

//
// SDMA or ADMA, total DMA transfer length should not greater than 4K
//
#define BUF1_LEN 512*2
#define BUF2_LEN 512*2

static SemaphoreHandle_t comp;

TaskHandle_t app_task_handle;

void am_sdio_isr(void)
{
    nex_irqhandler(SDHC, SLOT);
}

//
// This callback function is called from 'nex_api_thread' task
//
static void UsrFunctionPtr(SD_CALL_BACK_INFO *info)
{
    xSemaphoreGive(comp);
}

static uint8_t wrbuf1[BUF1_LEN] ALIGN(12);

static void pio_or_sdma_write(uint32_t addr, uint32_t len)
{
    if (len > BUF1_LEN)
    {
        APP_PRINT("out of boundary of write buffer\n");
        return;
    }

    // Random data will cause bit-flip
    for ( int i = 0; i < len; i++ )
    {
        wrbuf1[i] = rand() % 255;
    }

    APP_PRINT("Write Data\n");

    //
    // Calling SendData API
    //
    SendData(wrbuf1, len, addr, SDHC, SLOT);
    xSemaphoreTake(comp, portMAX_DELAY);
    APP_PRINT("Write Completed \n");
}

static uint8_t rdbuf1[BUF1_LEN] ALIGN(12);

static void pio_or_sdma_read(uint32_t addr, uint32_t len)
{
    if (len > BUF1_LEN)
    {
        APP_PRINT("out of boundary of write buffer\n");
        return;
    }

    memset(rdbuf1, 0x55, len);

    APP_PRINT("Read Data\n");

    //
    // Calling SendData API
    //
    GetData(rdbuf1, len, addr, SDHC, SLOT);
    xSemaphoreTake(comp, portMAX_DELAY);
    APP_PRINT("Read Completed \n");

}

static void check_if_rwbuf_match(uint8_t *rdbuf, uint8_t *wrbuf, uint32_t len)
{
    int i;
    for ( i = 0; i < len; i++ )
    {
        if (*(wrbuf + i) != *(rdbuf + i))
        {
            APP_PRINT("read and write buffer does not match from %d\n", i);
            break;
        }
    }

    if (i == len)
    {
        APP_PRINT("%d bytes Write and Read data matched\n", len);
    }
}

typedef enum
{
    NEX_XFER_PIO,
    NEX_XFER_DMA,
    NEX_XFER_ADMA2,
} xfer_mode_e;

static void set_dma_mode(struct sdio_bus_specific_info * businfo, xfer_mode_e mode)
{
    switch (mode)
    {
        case NEX_XFER_PIO:  //PIO
            businfo->dma_enable_disable = 0;  // = 0 for PIO and 1 for DMA and ADMA
            businfo->dma_type = 0;            // = 0 for PIO, 0 for DMA and 1 for ADMA
            APP_PRINT("PIO mode is set\n");
            break;
        case NEX_XFER_DMA:  //DMA
            businfo->dma_enable_disable = 1;
            businfo->dma_type = 0;
            APP_PRINT("SDMA mode is set\n");
            break;
        case NEX_XFER_ADMA2:  //ADMA2
            businfo->dma_enable_disable = 1;
            businfo->dma_type = 1;
            APP_PRINT("ADMA2 mode is set \n");
            break;
        default: //DMA - default
            businfo->dma_enable_disable = 1;
            businfo->dma_type = 0;
            APP_PRINT("default SDMA is set\n");
            break;
    }
}

static void set_bus_width(struct sdio_bus_specific_info * businfo, int bus_width)
{
    if ((businfo->bus_width != bus_width) && (bus_width >= 0) && (bus_width <= 6))
    {
        businfo->bus_width = bus_width;
        APP_PRINT("set the bus width %d\n", bus_width);
    }
    else
    {
        APP_PRINT("use the default bus width %d\n", businfo->bus_width);
    }
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

    write_prot_grp_size = erase_unit_size*ext_csd->hc_wp_grp_size;
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
    APP_PRINT("done\n");
}

static int32_t lock_device(void)
{
    int32_t ret;
    struct mmc_lock_data data;
    //
    // lock the device again
    //
    APP_PRINT("Locking the Device\n");

    //Lock Device
    memset(&data, 0, sizeof(struct mmc_lock_data));

    data.set_pwd = 1;
    data.lock_unlock = 1;
    data.pwd_len = 5;
    data.pwd[0] = 'a';
    data.pwd[1] = 'm';
    data.pwd[2] = 'b';
    data.pwd[3] = 'i';
    data.pwd[4] = 'q';

    ret = MMCLockUnlock(data, SDHC, SLOT);
    if ( ret != SD_SUCCESS )
    {
        APP_PRINT("Failed to Lock Device\n");
    }
    else
    {
        APP_PRINT("Device Locked successfully\n");
    }

    return ret;
}

static int32_t unlock_device(void)
{
    int32_t ret;
    struct mmc_lock_data data;
    //
    // unlock the device unconditionally
    //
    memset(&data, 0, sizeof(struct mmc_lock_data));

    data.lock_unlock = 0;
    data.clr_pwd = 1;
    data.pwd_len = 5;
    data.pwd[0] = 'a';
    data.pwd[1] = 'm';
    data.pwd[2] = 'b';
    data.pwd[3] = 'i';
    data.pwd[4] = 'q';

    ret = MMCLockUnlock(data, SDHC, SLOT);
    if ( ret != SD_SUCCESS )
    {
        APP_PRINT("Failed to unLock Device\n");
    }
    else
    {
        APP_PRINT("Device UnLocked successfully\n");
    }

    return ret;
}

static void erase_unlock_device(void)
{
    struct mmc_lock_data data;

    //
    // clear all write grp settings
    //
    // clear_all_write_grp_setting();

    //
    // erase the device lock unconditionally
    //
    memset(&data, 0, sizeof(struct mmc_lock_data));

    data.erase = 1;
    MMCLockUnlock(data, SDHC, SLOT);

    // delay 10 second to check again
    uint32_t erase_unlock_status;
    do
    {
        APP_PRINT("waiting 1s\n");
        vTaskDelay(pdMS_TO_TICKS(1000));
        GetDeviceStatus_CMD13(&erase_unlock_status, SDHC, SLOT);
        APP_PRINT("Device status : 0x%x\n", erase_unlock_status);
    } while (erase_unlock_status & (0x1 << 25));

    APP_PRINT("Device erase unLocked successfully\n");
}

void app_mmc_lock_unlock_task(void *pvParameters)
{
    struct sdio_bus_specific_info businfo;

    while (IsCardInitialized(SDHC, SLOT) == SD_FALSE)
    {
        vTaskDelay(pdMS_TO_TICKS(10));
        APP_PRINT("Checking if the card is ready again\n");
    }

    if (MMC_MODE_SDIO == CardType(SDHC, SLOT))
    {
        goto _suspend;
    }

    comp = xSemaphoreCreateBinary();
    if (NULL == comp)
    {
         goto _suspend;
    }

    RegisterISRCallBack(UsrFunctionPtr, SDHC, SLOT);

    // SDMMC_SetVoltage(SDHC, SLOT, POWER_180);
    // GetBusSpecificInformation(&businfo, SDHC, SLOT);

    //
    // Get card specific write protection information from CSD and EXT_CSD
    //
    GetCardSpecificInfo_CSD(&toshiba_emmc_csd, SDHC, SLOT);
    GetCardSpecificInfo_ExtCSD(&toshiba_emmc_ext_csd, SDHC, SLOT);
    dump_write_protect_setting(&toshiba_emmc_csd, &toshiba_emmc_ext_csd);

    //
    // Erase unlock the emmc device
    //
    erase_unlock_device();

    //
    // try to write some data. it will fail.
    //
    lock_device();
    pio_or_sdma_write(TEST_ADDR, BUF1_LEN);

    //
    // try to read back some data. it will not match.
    //
    pio_or_sdma_read(TEST_ADDR, BUF1_LEN);
    check_if_rwbuf_match(rdbuf1, wrbuf1, BUF1_LEN);
    APP_PRINT("Data can't be written before Device UnLocked\n");

    unlock_device();

    //
    // Write some data again
    //
    pio_or_sdma_write(TEST_ADDR, BUF1_LEN);
    pio_or_sdma_read(TEST_ADDR, BUF1_LEN);
    check_if_rwbuf_match(rdbuf1, wrbuf1, BUF1_LEN);

    //
    // set back the default dma mode
    //
    // set_dma_mode(&businfo, NEX_XFER_DMA);
    // SetBusSpecificInformation(&businfo, SDHC, SLOT);

_suspend:
    vTaskSuspend(NULL);
}
