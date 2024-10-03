//*****************************************************************************
//
//! @file emmc_raw_block_read_write.c
//!
//! @brief emmc raw block read and write example.
//!
//! Purpose: This example demonstrates how to blocking PIO and DMA read & write
//!          APIs with eMMC device.
//!
//! Additional Information:
//! To enable debug printing, add the following project-level macro definitions.
//!
//! AM_DEBUG_PRINTF
//!
//! When defined, debug messages will be sent over ITM/SWO at 1M Baud.
//!
//! Note that when these macros are defined, the device will never achieve deep
//! sleep, only normal sleep, due to the ITM (and thus the HFRC) being enabled.
//
//*****************************************************************************

//*****************************************************************************
//
// ${copyright}
//
// This is part of revision ${version} of the AmbiqSuite Development Package.
//
//*****************************************************************************

//*****************************************************************************
//
// This application has a large number of common include files. For
// convenience, we'll collect them all together in a single header and include
// that everywhere.
//
//*****************************************************************************
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>

#include "mspi_dma_xip_parallel_stress_test_cases.h"

#if  ( EMMC_TASK_ENABLE == 1 )

#define ENABLE_SDIO_POWER_SAVE
#define ENABLE_SDIO_ADMA_ASYNC
#define MAX_EMMC_TASK_DELAY    (300)

#define START_BLK 3000
#define BLK_NUM 10
#define BUF_LEN 512*BLK_NUM

//SDIO DMA timeout
#define SDIO_DMA_TIMEOUT      (0x1000UL)

//*****************************************************************************
//
// Semaphores used to sync emmc task with emmc task.
//
//*****************************************************************************
SemaphoreHandle_t g_semEMMCWrite = NULL;
SemaphoreHandle_t g_semEMMCRead = NULL;

TaskHandle_t emmc_task_handle;

AM_SHARED_RW uint8_t ui8RdBuf[BUF_LEN] __attribute__((aligned(32))); //algined 32 byte to match a cache line
AM_SHARED_RW uint8_t ui8WrBuf[BUF_LEN] __attribute__((aligned(32))); //algined 32 byte to match a cache line

volatile bool bAsyncWriteIsDone = false;
volatile bool bAsyncReadIsDone  = false;

void am_hal_card_event_test_cb(am_hal_host_evt_t *pEvt)
{
    am_hal_card_host_t *pHost = (am_hal_card_host_t *)pEvt->pCtx;
    am_hal_cachectrl_range_t sRange;
    uint32_t ui32BufLen = 0;

    if (AM_HAL_EVT_XFER_COMPLETE == pEvt->eType &&
        pHost->AsyncCmdData.dir == AM_HAL_DATA_DIR_READ)
    {
        xSemaphoreGiveFromISR(g_semEMMCRead, NULL);
        //am_util_debug_printf("Last Read Xfered block %d\n", pEvt->ui32BlkCnt);

        //
        // Flush and invalidate cache, if ssram buffer > 64K, clean all is efficient
        //
        if ( (uint32_t)(pHost->AsyncCmdData.pui8Buf) >= SSRAM_BASEADDR )
        {
            ui32BufLen = pEvt->ui32BlkCnt*512;
            if ( ui32BufLen > DCACHE_SIZE )
            {
                am_hal_cachectrl_dcache_invalidate(NULL, true);
            }
            else
            {
                sRange.ui32StartAddr = (uint32_t)(pHost->AsyncCmdData.pui8Buf);
                sRange.ui32Size = ui32BufLen;
                am_hal_cachectrl_dcache_invalidate(&sRange, false);
            }
        }
    }

    if (AM_HAL_EVT_XFER_COMPLETE == pEvt->eType &&
        pHost->AsyncCmdData.dir == AM_HAL_DATA_DIR_WRITE)
    {
        xSemaphoreGiveFromISR(g_semEMMCWrite, NULL);
        //am_util_debug_printf("Last Write Xfered block %d\n", pEvt->ui32BlkCnt);
    }

    if (AM_HAL_EVT_SDMA_DONE == pEvt->eType &&
        pHost->AsyncCmdData.dir == AM_HAL_DATA_DIR_READ)
    {
        //am_util_debug_printf("SDMA Read Xfered block %d\n", pEvt->ui32BlkCnt);
    }

    if (AM_HAL_EVT_SDMA_DONE == pEvt->eType &&
        pHost->AsyncCmdData.dir == AM_HAL_DATA_DIR_WRITE)
    {
        //am_util_debug_printf("SDMA Write Xfered block %d\n", pEvt->ui32BlkCnt);
    }

    if (AM_HAL_EVT_DAT_ERR == pEvt->eType)
    {
        //am_util_debug_printf("Data error type %d\n", pHost->AsyncCmdData.eDataError);
    }

    if (AM_HAL_EVT_CARD_PRESENT == pEvt->eType)
    {
        //am_util_debug_printf("A card is inserted\n");
    }
}


int check_if_data_match(uint8_t *pui8RdBuf, uint8_t *pui8WrBuf, uint32_t ui32Len)
{
    uint32_t i;
    for (i = 0; i < ui32Len; i++)
    {
        if (pui8RdBuf[i] != pui8WrBuf[i])
        {
            am_util_stdio_printf("EMMC error: pui8RdBuf[%d] = %d and pui8WrBuf[%d] = %d\n", i, pui8RdBuf[i], i, pui8WrBuf[i]);
            break;
        }
    }

    if (i == ui32Len)
    {
        return 0;
    }
    return -1;
}

am_hal_card_host_t *pSdhcCardHost = NULL;

void am_sdio0_isr(void)
{
    uint32_t ui32IntStatus;

    am_hal_sdhc_intr_status_get(pSdhcCardHost->pHandle, &ui32IntStatus, true);
    am_hal_sdhc_intr_status_clear(pSdhcCardHost->pHandle, ui32IntStatus);
    am_hal_sdhc_interrupt_service(pSdhcCardHost->pHandle, ui32IntStatus);
}



void EmmcTask(void *pvParameters)
{
    uint32_t ui32Stat = AM_HAL_STATUS_SUCCESS;
    uint32_t randomDelay;
    am_hal_cachectrl_range_t sRange;

    //
    // initialize the test read and write buffers
    //
    for (int i = 0; i < BUF_LEN; i++)
    {
        ui8WrBuf[i] = i % 256;
        ui8RdBuf[i] = 0x0;
    }

    //
    // Get the uderlying SDHC card host instance
    //
    pSdhcCardHost = am_hal_get_card_host(AM_HAL_SDHC_CARD_HOST, true);

    if (pSdhcCardHost == NULL)
    {
        am_util_stdio_printf("No such card host and stop\n");
        vTaskSuspend(NULL);
    }

    am_util_stdio_printf("card host is found\n");

    //
    // Configure SDIO PINs.
    //
    am_bsp_sdio_pins_enable(AM_BSP_SDIO_INSTANCE, AM_BSP_SDIO_BUS_WIDTH);
    
    am_hal_card_t eMMCard;

    //
    // check if card is present
    //
    while (am_hal_card_host_find_card(pSdhcCardHost, &eMMCard) != AM_HAL_STATUS_SUCCESS)
    {
        am_util_stdio_printf("No card is present now\n");
        vTaskDelay(1000);
        am_util_stdio_printf("Checking if card is available again\n");
    }

    while (am_hal_card_init(&eMMCard, AM_HAL_CARD_TYPE_EMMC, NULL, AM_HAL_CARD_PWR_CTRL_SDHC_OFF) == AM_HAL_STATUS_FAIL)
    {
        vTaskDelay(1000);
        am_util_stdio_printf("card and host is not ready, try again\n");
    }

    //
    // set the card type to eMMC by using 48MHz and 8-bit mode for read and write
    //
    am_hal_card_cfg_set(&eMMCard, AM_HAL_CARD_TYPE_EMMC,
        SDIO_BUS_WIDTH, 48000000, 
        AM_HAL_HOST_BUS_VOLTAGE_1_8,
        AM_HAL_HOST_UHS_SDR50);

#ifdef ENABLE_SDIO_ADMA_ASYNC
        am_hal_card_host_set_xfer_mode(pSdhcCardHost, AM_HAL_HOST_XFER_ADMA);

        //
        // async read & write, card insert & remove needs a callback function
        //
        am_util_debug_printf("========  Test multiple block async write and read  ========\n");
        am_hal_card_register_evt_callback(&eMMCard, am_hal_card_event_test_cb);
#endif

#ifdef ENABLE_SDIO_POWER_SAVE
        //
        // power down SDIO peripheral
        //
        if ( am_hal_card_pwrctrl_sleep(&eMMCard) != AM_HAL_STATUS_SUCCESS )
        {
            am_util_stdio_printf("Fail to power down SDIO peripheral\n");
            vErrorHandler();
        }
#endif

    while(1)
    {
#ifdef ENABLE_SDIO_POWER_SAVE
        //
        // power up SDIO peripheral
        //
        if ( am_hal_card_pwrctrl_wakeup(&eMMCard) != AM_HAL_STATUS_SUCCESS )
        {
            am_util_stdio_printf("Fail to power up SDIO peripheral\n");
            vErrorHandler();
        }
#endif

        //
        // erase eMMC before write
        //
        ui32Stat = am_hal_card_block_erase(&eMMCard, START_BLK, BLK_NUM, AM_HAL_ERASE, 1000);
        if(ui32Stat != 0x0)
        {
            am_util_stdio_printf("erase eMMC failed\n");
            vErrorHandler();
        }

        //
        // Clean dcache data before write.
        //
        if ( BUF_LEN > DCACHE_SIZE ) // if ssram buffer > 64K, clean all is efficient
        {
            am_hal_cachectrl_dcache_clean(NULL);
        }
        else
        {
            sRange.ui32StartAddr = (uint32_t)ui8WrBuf;
            sRange.ui32Size = BUF_LEN;
            am_hal_cachectrl_dcache_clean(&sRange);
        }

#ifdef ENABLE_SDIO_POWER_SAVE
        //
        // power down SDIO peripheral
        //
        ui32Stat = am_hal_card_pwrctrl_sleep(&eMMCard);
        if(ui32Stat != AM_HAL_STATUS_SUCCESS)
        {
            am_util_stdio_printf("Failed to power down card\n");
            vErrorHandler();
        }
#endif
        //
        // Task Delay Set.
        //
        srand(xTaskGetTickCount());
        randomDelay = rand() % MAX_EMMC_TASK_DELAY;
        vTaskDelay(randomDelay);

#ifdef ENABLE_SDIO_POWER_SAVE
        //
        // power up SDIO peripheral
        //
        ui32Stat = am_hal_card_pwrctrl_wakeup(&eMMCard);
        if(ui32Stat != AM_HAL_STATUS_SUCCESS)
        {
            am_util_stdio_printf("Failed to power up card.\n");
            vErrorHandler();
        }
#endif

#ifdef ENABLE_SDIO_ADMA_ASYNC
        //
        // Write 'BLK_NUM' blocks to the eMMC flash
        //
        bAsyncWriteIsDone = false;
        ui32Stat = am_hal_card_block_write_async(&eMMCard, START_BLK, BLK_NUM, (uint8_t *)ui8WrBuf);
        if((ui32Stat & 0xffff) != 0 )
        {
            am_util_stdio_printf("Failed to write card.\n");
            vErrorHandler();
        }

        //
        // wait until the async write is done
        //
        BaseType_t ret = xSemaphoreTake( g_semEMMCWrite, SDIO_DMA_TIMEOUT );
        if ( ret == pdFALSE )
        {
            am_util_stdio_printf("sdio DMA timeout!\n");
            vErrorHandler();
        }
        am_util_debug_printf("asynchronous block write is done\n");
#else
        //
        // write BLK_NUM blocks to emmc flash
        //
        ui32Stat = am_hal_card_block_write_sync(&eMMCard, START_BLK, BLK_NUM, (uint8_t *)ui8WrBuf);
        if((ui32Stat & 0xffff) != 0 )
        {
            am_util_stdio_printf("Failed to write card.\n");
            vErrorHandler();
        }
#endif

#ifdef ENABLE_SDIO_POWER_SAVE
        //
        // power down SDIO peripheral
        //
        if ( am_hal_card_pwrctrl_sleep(&eMMCard) != AM_HAL_STATUS_SUCCESS )
        {
            am_util_stdio_printf("Fail to power down SDIO peripheral\n");
            vErrorHandler();
        }
#endif

        //
        // read back the first block of emmc flash
        //
        memset((void *)ui8RdBuf, 0x0, BUF_LEN);

        //
        // Clean dcache data if ssram buffer acessed by other master
        //
        if ( BUF_LEN > DCACHE_SIZE ) // if ssram buffer > 64K, clean all is efficient
        {
            am_hal_cachectrl_dcache_clean(NULL);
        }
        else
        {
            sRange.ui32StartAddr = (uint32_t)ui8RdBuf;
            sRange.ui32Size = BUF_LEN;
            am_hal_cachectrl_dcache_clean(&sRange);
        }

#ifdef ENABLE_SDIO_POWER_SAVE
        //
        // power up SDIO peripheral
        //
        if ( am_hal_card_pwrctrl_wakeup(&eMMCard) != AM_HAL_STATUS_SUCCESS )
        {
            am_util_stdio_printf("Fail to power up SDIO peripheral\n");
            vErrorHandler();
        }
#endif

#ifdef ENABLE_SDIO_ADMA_ASYNC
        //
        // Read 'BLK_NUM' blocks to the eMMC flash
        //
        bAsyncReadIsDone = false;
        am_hal_card_block_read_async(&eMMCard, START_BLK, BLK_NUM, (uint8_t *)ui8RdBuf);

        //
        // wait until the async read is done
        //
        ret = xSemaphoreTake( g_semEMMCRead, SDIO_DMA_TIMEOUT );
        if ( ret == pdFALSE )
        {
            am_util_stdio_printf("SDIO DMA timeout!\n");
            vErrorHandler();
        }
#else
        am_hal_card_block_read_sync(&eMMCard, START_BLK, BLK_NUM, (uint8_t *)ui8RdBuf);

        //
        // Flush and invalidate cache, if ssram buffer > 64K, clean all is efficient
        //
        if ( BUF_LEN > DCACHE_SIZE )
        {
            am_hal_cachectrl_dcache_invalidate(NULL, true);
        }
        else
        {
            sRange.ui32StartAddr = (uint32_t)ui8RdBuf;
            sRange.ui32Size = BUF_LEN;
            am_hal_cachectrl_dcache_invalidate(&sRange, false);
        }
#endif

        //
        // check if block data match or not
        //
        if (check_if_data_match((uint8_t *)ui8RdBuf, (uint8_t *)ui8WrBuf, BUF_LEN) == 0)
        {
            am_util_stdio_printf("[TASK] : EMMC\n");
        }
        else
        {
            am_util_stdio_printf("EMMC Task Data Mismatch!\n");
        }

#ifdef ENABLE_SDIO_POWER_SAVE
        //
        // power down SDIO peripheral
        //
        if ( am_hal_card_pwrctrl_sleep(&eMMCard) != AM_HAL_STATUS_SUCCESS )
        {
            am_util_stdio_printf("Fail to power down SDIO peripheral\n");
            vErrorHandler();
        }

        srand(xTaskGetTickCount());
        randomDelay = rand() % MAX_EMMC_TASK_DELAY;
        vTaskDelay(randomDelay);
#endif

    } // End of for loop

}

#endif  //( EMMC_TASK_ENABLE == 1 )