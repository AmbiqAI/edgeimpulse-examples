//*****************************************************************************
//
//! @file app_task.c
//!
//! @brief A simple SDMMC packed commands block read and write example.
//!
//! This example use nex_api functions 'eMMCPackedWrite' and 'eMMCPackedRead' to write some
//! data or read some data the SD or MMC card.
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

#define SDHC 0
#define SLOT 0

#define TEST_ADDR1 0x800
#define TEST_ADDR2 0x1000

//
// SDMA or ADMA, total DMA transfer length should not greater than 4K
//
#define BUF1_LEN (512 * 2)
#define BUF2_LEN (512 * 2)

TaskHandle_t app_task_handle;

void am_sdio_isr(void)
{
    nex_irqhandler(SDHC, SLOT);
}

//
// TODO: need to check if 4K alignment is needed
//
static uint8_t wrbuf1[BUF1_LEN] ALIGN(12);
static uint8_t wrbuf2[BUF2_LEN] ALIGN(12);

//
// TODO: need to check if 4K alignment is needed
//
static uint8_t rdbuf1[BUF1_LEN] ALIGN(12);
static uint8_t rdbuf2[BUF2_LEN] ALIGN(12);

static void packed_cmds_write(void)
{
    int32_t retval;
    static NEX_MMC_EXT_CSD toshiba_emmc_ext_csd;

    struct packedcmd pcmd;
    struct packedcmd_entry entry[2];

    // Random data will cause bit-flip
    for (int i = 0; i < BUF1_LEN; i++)
    {
        wrbuf1[i] = rand() % 255;
    }

    for (int i = 0; i < BUF2_LEN; i++)
    {
        wrbuf2[i] = rand() % 255;
    }

    APP_PRINT("Packed commands Write Data\n");

    pcmd.num_entries = 2;
    pcmd.rw = 1;

    memset(&entry[0], 0, 2*(sizeof(struct packedcmd_entry)));
    entry[0].cmd23arg.no_of_blks = 2;
    entry[0].cmd23arg.rel_wr_req = 0;
    entry[0].cmd23arg.packedcmd  = 0;
    entry[0].cmd23arg.context_id = 0;
    entry[0].cmd23arg.tag_req    = 0;
    entry[0].cmd23arg.forced_prg = 0;
    entry[0].addr = TEST_ADDR1;
    entry[0].data = wrbuf1;

    entry[1].cmd23arg.no_of_blks = 2;
    entry[1].cmd23arg.rel_wr_req = 0;
    entry[1].cmd23arg.packedcmd  = 0;
    entry[1].cmd23arg.context_id = 0;
    entry[1].cmd23arg.tag_req    = 0;
    entry[1].cmd23arg.forced_prg = 0;
    entry[1].addr = TEST_ADDR2;
    entry[1].data = wrbuf2;

    pcmd.packed_entry = entry;

    //
    // Calling eMMCPackedWrite API
    //
    retval = eMMCPackedWrite(&pcmd, SDHC, SLOT);

    if ( !retval )
    {
        APP_PRINT("Packed Commands Write Completed\n");
    }
    else
    {
        APP_PRINT("Packed Commands Write Failed\n");
    }

    GetCardSpecificInfo_ExtCSD(&toshiba_emmc_ext_csd, SDHC, SLOT);

    APP_PRINT("PACKED_COMMAND_STATUS [36] = %x\n", toshiba_emmc_ext_csd.packed_cmd_status);
    APP_PRINT("PACKED_FAILURE_INDEX  [35] = %x\n", toshiba_emmc_ext_csd.packed_fail_index);

}

static void packed_cmds_read(void)
{
    int32_t retval;

    static NEX_MMC_EXT_CSD toshiba_emmc_ext_csd;

    struct packedcmd pcmd;
    struct packedcmd_entry entry[2];

    memset(rdbuf1, 0x55, BUF1_LEN);
    memset(rdbuf2, 0x55, BUF2_LEN);

    pcmd.num_entries = 2;
    pcmd.rw = 0;

    APP_PRINT("Packed commands Read Data\n");

    memset(&entry[0], 0, 2*(sizeof(struct packedcmd_entry)));

    entry[0].cmd23arg.no_of_blks = 2;
    entry[0].cmd23arg.rel_wr_req = 0;
    entry[0].cmd23arg.packedcmd  = 0;
    entry[0].cmd23arg.context_id = 0;
    entry[0].cmd23arg.tag_req    = 0;
    entry[0].cmd23arg.forced_prg = 0;
    entry[0].addr = TEST_ADDR1;
    entry[0].data = rdbuf1;

    entry[1].cmd23arg.no_of_blks = 2;
    entry[1].cmd23arg.rel_wr_req = 0;
    entry[1].cmd23arg.packedcmd  = 0;
    entry[1].cmd23arg.context_id = 0;
    entry[1].cmd23arg.tag_req    = 0;
    entry[1].cmd23arg.forced_prg = 0;
    entry[1].addr = TEST_ADDR2;
    entry[1].data = rdbuf2;

    pcmd.packed_entry = &entry[0];

    //
    // Calling eMMCPackedRead API
    //
    retval = eMMCPackedRead(&pcmd, SDHC, SLOT);

    if ( !retval )
    {
        APP_PRINT("Packed Read Completed\n");
    }
    else
    {
        APP_PRINT("Packed Read Failed\n");
    }

    GetCardSpecificInfo_ExtCSD(&toshiba_emmc_ext_csd, SDHC, SLOT);

    APP_PRINT("PACKED_COMMAND_STATUS [36] = %x\n", toshiba_emmc_ext_csd.packed_cmd_status);
    APP_PRINT("PACKED_FAILURE_INDEX  [35] = %x\n", toshiba_emmc_ext_csd.packed_fail_index);
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

//
// PIO, SDMA, ADMA2 mode setting
//
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
            businfo->dma_enable_disable = 0;    // = 0 for PIO and 1 for DMA and ADMA
            businfo->dma_type = 0;              // = 0 for PIO, 0 for DMA and 1 for ADMA
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

void app_emmc_packed_cmds_blockrw_task(void *pvParameters)
{
    int count = 0;

    struct sdio_bus_specific_info businfo;

    APP_PRINT("sdmmc packed write & read commands test example\n");

    while (IsCardInitialized(SDHC, SLOT) == SD_FALSE)
    {
        vTaskDelay(pdMS_TO_TICKS(10));
        APP_PRINT("Checking if the card is ready again\n");
    }

    if (MMC_MODE_SDIO == CardType(SDHC, SLOT))
    {
        goto _suspend;
    }

    // SDMMC_SetVoltage(SDHC, SLOT, POWER_180);
    // GetBusSpecificInformation(&businfo, SDHC, SLOT);

    while (count++ < 1000 )
    {

        //
        // using default SDMA mode and 4bit bus width block Write & Read
        //

        // Multiple blocks read & write (CMD18 & CMD25)
        // length is multiple of sector size (512 bytes)
        // set_dma_mode(&businfo, 1);
        // set_bus_width(&businfo, SD_BUS_WIDTH4);

        packed_cmds_write();
        packed_cmds_read();
        check_if_rwbuf_match(rdbuf1, wrbuf1, BUF1_LEN);
        check_if_rwbuf_match(rdbuf2, wrbuf2, BUF2_LEN);

        // check the heap_4 memory status
        APP_PRINT("%d -> heap_4 free heap size is : %d\n", count, xPortGetFreeHeapSize());
        APP_PRINT("%d -> heap_4 minimum free heap size is : %d\n", count, xPortGetMinimumEverFreeHeapSize());

    } // End of while

    //
    // set back the default dma mode
    //
    // set_dma_mode(&businfo, 1);
    // SetBusSpecificInformation(&businfo, SDHC, SLOT);

_suspend:
    vTaskSuspend(NULL);
}

