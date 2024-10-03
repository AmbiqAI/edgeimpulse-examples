//*****************************************************************************
//
//! @file app_task.c
//!
//! @brief A simple SDMMC block read and write example.
//!
//! This example use nex_api functions 'SendData' and 'GetData' to write some
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

#define HIGH_CAPACITY

#define APP_PRINT(format, ...)  am_util_stdio_printf("[APP] %33s %04d | " format, __func__, __LINE__, ##__VA_ARGS__)

#define ALIGN(x) __attribute__((aligned(1 << x)))

#define SDHC 0
#define SLOT 0

#define TEST_SECTOR 200

#ifdef HIGH_CAPACITY
#define TEST_ADDR TEST_SECTOR   // sector address
#else
#define TEST_ADDR TEST_SECTOR*512
#endif

#define TEST_COUNT 20
//
// SDMA or ADMA, total DMA transfer length should not greater than 4K
//
#define BUF1_LEN 512*5
#define BUF2_LEN 512*3

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
static uint8_t wrbuf2[BUF2_LEN] ALIGN(12);

static void adma2_write(uint32_t address)
{
    uint32_t len1 = BUF1_LEN;
    uint32_t len2 = BUF2_LEN;

    uint32_t total_length;

    SD_NEX_ADMA_TABLE adma_table;
    SD_NEX_ADMA_DESC adma_desc;
    SD_NEX_ADMA_LIST adma_list[2];

    total_length = len1 + len2;

    // Random data will cause bit-flip
    for (int i = 0; i < BUF1_LEN; i++)
    {
        wrbuf1[i] = rand() % 255;
    }

    // Random data will cause bit-flip
    for (int i = 0; i < BUF2_LEN; i++)
    {
        wrbuf2[i] = rand() % 255;
    }

    //
    // Filling ADMA Table for Read
    //
    adma_table.num_tables = 1;
    adma_table.desc       = &adma_desc;

    //
    // Filling Table
    //
    adma_table.desc[0].num_entries = 2;
    adma_table.desc[0].adma_list   = &adma_list[0];

    //
    // First entry of table
    //
    adma_table.desc[0].adma_list[0].buffer     = wrbuf1;
    adma_table.desc[0].adma_list[0].buffer_len = len1;
    adma_table.desc[0].adma_list[0].attrib     = ADMA2_ATTRIB_TRAN | ADMA2_ATTRIB_INT | ADMA2_ATTRIB_VALID;
    adma_table.desc[0].adma_list[0].link       = 0;

    //
    // Second entry of table
    //
    adma_table.desc[0].adma_list[1].buffer     = wrbuf2;
    adma_table.desc[0].adma_list[1].buffer_len = len2;
    adma_table.desc[0].adma_list[1].attrib     = ADMA2_ATTRIB_TRAN | ADMA2_ATTRIB_INT | ADMA2_ATTRIB_END | ADMA2_ATTRIB_VALID;
    adma_table.desc[0].adma_list[1].link       = 0;

    APP_PRINT("Write Data\n");

    //
    // Calling SendData API
    //
    SendData((void *)&adma_table, total_length, address, SDHC, SLOT);
    xSemaphoreTake(comp, portMAX_DELAY);
    APP_PRINT("Write Completed \n");
}

static uint8_t rdbuf1[BUF1_LEN] ALIGN(12);
static uint8_t rdbuf2[BUF2_LEN] ALIGN(12);

static void adma2_read(uint32_t address)
{
    uint32_t len1 = BUF1_LEN;
    uint32_t len2 = BUF2_LEN;
    uint32_t total_length;

    SD_NEX_ADMA_TABLE adma_table;
    SD_NEX_ADMA_DESC  adma_desc;
    SD_NEX_ADMA_LIST  adma_list[2];

    total_length = len1 + len2;

    memset(rdbuf1, 0x0, BUF1_LEN * sizeof(char));
    memset(rdbuf2, 0x0, BUF2_LEN * sizeof(char));

    //
    // Filling ADMA Table for Read
    //
    adma_table.num_tables = 1;
    adma_table.desc       = &adma_desc;

    //
    // Filling Table
    //
    adma_table.desc[0].num_entries = 2;
    adma_table.desc[0].adma_list   = &adma_list[0];

    //
    // First entry of table
    //
    adma_table.desc[0].adma_list[0].buffer     = rdbuf1;
    adma_table.desc[0].adma_list[0].buffer_len = len1;
    adma_table.desc[0].adma_list[0].attrib     = ADMA2_ATTRIB_TRAN | ADMA2_ATTRIB_INT | ADMA2_ATTRIB_VALID;
    adma_table.desc[0].adma_list[0].link       = 0;

    //
    // Second entry of table
    //
    adma_table.desc[0].adma_list[1].buffer     = rdbuf2;
    adma_table.desc[0].adma_list[1].buffer_len = len2;
    adma_table.desc[0].adma_list[1].attrib     = ADMA2_ATTRIB_TRAN | ADMA2_ATTRIB_INT | ADMA2_ATTRIB_END | ADMA2_ATTRIB_VALID;
    adma_table.desc[0].adma_list[1].link       = 0;

    APP_PRINT("Read Data\n");

    //
    // Calling SendData API
    //
    GetData((void *)&adma_table, total_length, address, SDHC, SLOT);
    xSemaphoreTake(comp, portMAX_DELAY);
    APP_PRINT("Read Completed \n");
}

static void pio_or_sdma_write(uint32_t addr, uint32_t len)
{
    if (len > BUF1_LEN)
    {
        APP_PRINT("out of boundary of write buffer\n");
        return;
    }

    // Random data will cause bit-flip
    for (int i = 0; i < len; i++)
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

static int count = 0;
static int count_matched = 0;
static int count_failed  = 0;

static void check_if_rwbuf_match(uint8_t *rdbuf, uint8_t *wrbuf, uint32_t len)
{
    int i;
    for ( i = 0; i < len; i++ )
    {
        if (*(wrbuf + i) != *(rdbuf + i))
        {
            APP_PRINT("read and write buffer does not match from %d\n", i);
            count_failed++;
            break;
        }
    }

    if (i == len)
    {
        APP_PRINT("%d bytes Write and Read data matched\n", len);
        count_matched++;
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

static void set_clock_speed(struct sdio_bus_specific_info * businfo, uint32_t ui32Clock)
{
    if (ui32Clock > 48000000 || ui32Clock < 100000)
    {
        APP_PRINT("Setting clock speed is out of range\n");
        APP_PRINT("Use default clock speed\n");
    }
    else
    {
        businfo->clock = ui32Clock;
        APP_PRINT("set the clock speed to %dHz\n", ui32Clock);

    }
}

void app_sdmmc_blockrw_task(void *pvParameters)
{

    struct sdio_bus_specific_info businfo;

    APP_PRINT("sdmmc block read&write test\n");

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

    GetBusSpecificInformation(&businfo, SDHC, SLOT);

    vTaskDelay(pdMS_TO_TICKS(10));

//    SetMMCSDHighSpeed(SDHC, SLOT);

    while (count++ < TEST_COUNT)
    {
        //
        // PIO mode block Write & Read
        //

        // Single block read & write (CMD17 & CMD24), length is
        // is just one sector (block) size (512 bytes)
        // 1-wire data line
//        set_clock_speed(&businfo, 48000000);
        set_dma_mode(&businfo, NEX_XFER_PIO);
        set_bus_width(&businfo, SD_BUS_WIDTH1);
        SetBusSpecificInformation(&businfo, SDHC, SLOT);

        pio_or_sdma_write(TEST_ADDR, 512);
        pio_or_sdma_read(TEST_ADDR, 512);
        check_if_rwbuf_match(rdbuf1, wrbuf1, 512);

        // Single block read & write (CMD17 & CMD24), length is
        // is just one sector (block) size (512 bytes)
        // 4-wire data lines
        set_dma_mode(&businfo, NEX_XFER_PIO);
        set_bus_width(&businfo, SD_BUS_WIDTH4);
        SetBusSpecificInformation(&businfo, SDHC, SLOT);

        pio_or_sdma_write(TEST_ADDR, 512);
        pio_or_sdma_read(TEST_ADDR, 512);
        check_if_rwbuf_match(rdbuf1, wrbuf1, 512);

        // Multiple blocks read & write (CMD18 & CMD25)
        // length is not multiple of sector size (512 bytes)
        set_dma_mode(&businfo, NEX_XFER_PIO);
        set_bus_width(&businfo, SD_BUS_WIDTH1);
        SetBusSpecificInformation(&businfo, SDHC, SLOT);

        pio_or_sdma_write(TEST_ADDR, BUF1_LEN);
        pio_or_sdma_read(TEST_ADDR, BUF1_LEN);
        check_if_rwbuf_match(rdbuf1, wrbuf1, BUF1_LEN);

        // Multiple blocks read & write (CMD18 & CMD25)
        // length is not multiple of sector size (512 bytes)
        set_dma_mode(&businfo, NEX_XFER_PIO);
        set_bus_width(&businfo, SD_BUS_WIDTH4);
        SetBusSpecificInformation(&businfo, SDHC, SLOT);

        pio_or_sdma_write(TEST_ADDR, BUF1_LEN);
        pio_or_sdma_read(TEST_ADDR, BUF1_LEN);
        check_if_rwbuf_match(rdbuf1, wrbuf1, BUF1_LEN);

#if 0   // Fail
        // Single block read & write (CMD17 & CMD24), length is
        // is less than one sector (block) size (512 bytes)
        set_dma_mode(&businfo, NEX_XFER_PIO);
        SetBusSpecificInformation(&businfo, SDHC, SLOT);

        pio_or_sdma_write(TEST_ADDR, 100);
        pio_or_sdma_read(TEST_ADDR, 100);
        check_if_rwbuf_match(rdbuf1, wrbuf1, 100);

        // Multiple blocks read & write (CMD18 & CMD25)
        // length is not multiple of sector size (512 bytes)
        set_dma_mode(&businfo, NEX_XFER_PIO);
        SetBusSpecificInformation(&businfo, SDHC, SLOT);

        pio_or_sdma_write(TEST_ADDR, 512 + 256);
        pio_or_sdma_read(TEST_ADDR, 512 + 256);
        check_if_rwbuf_match(rdbuf1, wrbuf1, 512 + 256);
#endif
        //
        // SDMA mode block Write & Read
        //

        // Single block read & write (CMD17 & CMD24), length is
        // is just one sector (block) size (512 bytes)
        // 1-wire data line
        set_dma_mode(&businfo, NEX_XFER_DMA);
        set_bus_width(&businfo, SD_BUS_WIDTH1);
        SetBusSpecificInformation(&businfo, SDHC, SLOT);

        pio_or_sdma_write(TEST_ADDR, 512);
        pio_or_sdma_read(TEST_ADDR, 512);
        check_if_rwbuf_match(rdbuf1, wrbuf1, 512);

        // Single block read & write (CMD17 & CMD24), length is
        // is just one sector (block) size (512 bytes)
        // 4-wire data lines
        set_dma_mode(&businfo, NEX_XFER_DMA);
        set_bus_width(&businfo, SD_BUS_WIDTH4);
        SetBusSpecificInformation(&businfo, SDHC, SLOT);

        pio_or_sdma_write(TEST_ADDR, 512);
        pio_or_sdma_read(TEST_ADDR, 512);
        check_if_rwbuf_match(rdbuf1, wrbuf1, 512);

        // Multiple blocks read & write (CMD18 & CMD25)
        // length is multiple of sector size (512 bytes)
        set_dma_mode(&businfo, NEX_XFER_DMA);
        set_bus_width(&businfo, SD_BUS_WIDTH1);
        SetBusSpecificInformation(&businfo, SDHC, SLOT);

        pio_or_sdma_write(TEST_ADDR, BUF1_LEN);
        pio_or_sdma_read(TEST_ADDR, BUF1_LEN);
        check_if_rwbuf_match(rdbuf1, wrbuf1, BUF1_LEN);

        // Multiple blocks read & write (CMD18 & CMD25)
        // length is multiple of sector size (512 bytes)
        set_dma_mode(&businfo, NEX_XFER_DMA);
        set_bus_width(&businfo, SD_BUS_WIDTH4);
        SetBusSpecificInformation(&businfo, SDHC, SLOT);

        pio_or_sdma_write(TEST_ADDR, BUF1_LEN);
        pio_or_sdma_read(TEST_ADDR, BUF1_LEN);
        check_if_rwbuf_match(rdbuf1, wrbuf1, BUF1_LEN);

#if 0   // fail
        // Multiple blocks read & write (CMD18 & CMD25)
        // length is not multiple of sector size (512 bytes)
        set_dma_mode(&businfo, NEX_XFER_DMA);
        SetBusSpecificInformation(&businfo, SDHC, SLOT);

        pio_or_sdma_write(200, 512 + 256);
        pio_or_sdma_read(200, 512 + 256);
        check_if_rwbuf_match(rdbuf1, wrbuf1, 512 + 256);
#endif

        //
        // ADMA2 mode block Write & Read - bus width 1
        //
        set_dma_mode(&businfo, NEX_XFER_ADMA2);
        set_bus_width(&businfo, SD_BUS_WIDTH1);
        SetBusSpecificInformation(&businfo, SDHC, SLOT);

        adma2_write(TEST_ADDR);
        adma2_read(TEST_ADDR);
        check_if_rwbuf_match(rdbuf1, wrbuf1, BUF1_LEN);
        check_if_rwbuf_match(rdbuf2, wrbuf2, BUF2_LEN);

        //
        // ADMA2 mode block Write & Read - bus width 4
        //
        set_dma_mode(&businfo, NEX_XFER_ADMA2);
        set_bus_width(&businfo, SD_BUS_WIDTH4);
        SetBusSpecificInformation(&businfo, SDHC, SLOT);

        adma2_write(TEST_ADDR);
        adma2_read(TEST_ADDR);
        check_if_rwbuf_match(rdbuf1, wrbuf1, BUF1_LEN);
        check_if_rwbuf_match(rdbuf2, wrbuf2, BUF2_LEN);

        // check the heap_4 memory status
        APP_PRINT("%d -> heap_4 free heap size is : %d\n", count, xPortGetFreeHeapSize());
        APP_PRINT("%d -> heap_4 minimum free heap size is : %d\n", count, xPortGetMinimumEverFreeHeapSize());

    } // End of while

    APP_PRINT("total test: %d, matched: %d, failed: %d\n", count, count_matched, count_failed);

    //
    // set back the default dma mode
    //
    set_dma_mode(&businfo, NEX_XFER_DMA);
    SetBusSpecificInformation(&businfo, SDHC, SLOT);

_suspend:
    vTaskSuspend(NULL);
}

