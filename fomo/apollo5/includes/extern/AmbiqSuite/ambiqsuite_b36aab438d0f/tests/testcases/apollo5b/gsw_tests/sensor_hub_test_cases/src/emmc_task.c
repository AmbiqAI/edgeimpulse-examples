//*****************************************************************************
//
//! @file emmc_task.c
//!
//! @brief emmc task
//
//*****************************************************************************

//*****************************************************************************
//
// ${copyright}
//
// This is part of revision ${version} of the AmbiqSuite Development Package.
//
//*****************************************************************************
#include "sensor_hub_test_cases.h"

//*****************************************************************************
//
// EMMC task handle.
//
//*****************************************************************************
TaskHandle_t EmmcTaskHandle;

//*****************************************************************************
//
// Global variables and templates 
//
//*****************************************************************************
volatile bool g_bEmmcTaskComplete = false;

#define TEST_POWER_SAVING
//#define EMMC_TASK_DEBUG_LOG
#define SDIO_INSTANCE 0

#define DCACHE_SIZE     64*1024

AM_SHARED_RW uint8_t g_pui8RxSensorDataBuf[MSG_BUF_BYTES] __attribute__((aligned(32))); // data received from Process Sensors task
AM_SHARED_RW uint8_t g_pui8RxDataVerifyBuf[MSG_BUF_BYTES] __attribute__((aligned(32))); // read back from eMMC 

volatile bool bAsyncWriteIsDone = false;
volatile bool bAsyncReadIsDone  = false;

am_hal_card_host_t *pSdhcCardHost = NULL;

extern bool g_bTestEnd;

void am_hal_card_event_test_cb(am_hal_host_evt_t *pEvt)
{
    am_hal_card_host_t *pHost = (am_hal_card_host_t *)pEvt->pCtx;
	am_hal_cachectrl_range_t sRange;
    uint32_t ui32BufLen = 0;
    uint32_t ui32BufAddr = 0;

    if (AM_HAL_EVT_XFER_COMPLETE == pEvt->eType &&
        pHost->AsyncCmdData.dir == AM_HAL_DATA_DIR_READ)
    {
        bAsyncReadIsDone = true;
        am_util_stdio_printf("Last Read Xfered block %d\n", pEvt->ui32BlkCnt);
			
	    //
        // Flush and invalidate cache, if ssram buffer > 64K, clean all is efficient
        //
        ui32BufAddr= (uint32_t)(pHost->AsyncCmdData.pui8Buf);
        if ( ui32BufAddr >= SSRAM_BASEADDR )
        {
            ui32BufLen = pEvt->ui32BlkCnt*512;
            if ( ui32BufLen > DCACHE_SIZE )
            {
                am_hal_cachectrl_dcache_invalidate(NULL, true);
            }
            else
            {
                sRange.ui32StartAddr = ui32BufAddr;
                sRange.ui32Size = ui32BufLen;
                am_hal_cachectrl_dcache_invalidate(&sRange, false);
            }
        }
    }

    if (AM_HAL_EVT_XFER_COMPLETE == pEvt->eType &&
        pHost->AsyncCmdData.dir == AM_HAL_DATA_DIR_WRITE)
    {
        bAsyncWriteIsDone = true;
        am_util_stdio_printf("Last Write Xfered block %d\n", pEvt->ui32BlkCnt);
    }

    if (AM_HAL_EVT_SDMA_DONE == pEvt->eType &&
        pHost->AsyncCmdData.dir == AM_HAL_DATA_DIR_READ)
    {
        am_util_stdio_printf("SDMA Read Xfered block %d\n", pEvt->ui32BlkCnt);
    }

    if (AM_HAL_EVT_SDMA_DONE == pEvt->eType &&
        pHost->AsyncCmdData.dir == AM_HAL_DATA_DIR_WRITE)
    {
        am_util_stdio_printf("SDMA Write Xfered block %d\n", pEvt->ui32BlkCnt);
    }

    if (AM_HAL_EVT_DAT_ERR == pEvt->eType)
    {
        am_util_stdio_printf("Data error type %d\n", pHost->AsyncCmdData.eDataError);
    }

    if (AM_HAL_EVT_CARD_PRESENT == pEvt->eType)
    {
        am_util_stdio_printf("A card is inserted\n");
    }
}


void check_if_data_match(uint8_t *pui8RdBuf, uint8_t *pui8WrBuf, uint32_t ui32Len)
{
    uint32_t i;
    for (i = 0; i < ui32Len; i++)
    {
        if (pui8RdBuf[i] != pui8WrBuf[i])
        {
            TEST_ASSERT_TRUE(false);
            am_util_stdio_printf("EMMC error: pui8RdBuf[%d] = %d and pui8WrBuf[%d] = %d\n", i, pui8RdBuf[i], i, pui8WrBuf[i]);
            break;
        }
    }

    if (i == ui32Len)
    {
        am_util_stdio_printf("EMMC\n");
    }

}

void am_sdio0_isr(void)
{
    uint32_t ui32IntStatus;

    am_hal_sdhc_intr_status_get(pSdhcCardHost->pHandle, &ui32IntStatus, true);
    am_hal_sdhc_intr_status_clear(pSdhcCardHost->pHandle, ui32IntStatus);
    am_hal_sdhc_interrupt_service(pSdhcCardHost->pHandle, ui32IntStatus);
}



void EmmcTask(void *pvParameters)
{
    uint32_t ui32Status;
    size_t xReceivedBytes;
    uint32_t ui32BufAddr = 0;
    am_hal_cachectrl_range_t sRange;
    //
    // Configure SDIO PINs.
    //
    am_bsp_sdio_pins_enable(SDIO_INSTANCE, AM_HAL_HOST_BUS_WIDTH_8);

    //
    // Get the uderlying SDHC card host instance
    //
    pSdhcCardHost = am_hal_get_card_host(AM_HAL_SDHC_CARD_HOST, true);

    if (pSdhcCardHost == NULL)
    {
        TEST_ASSERT_TRUE(false);
        am_util_stdio_printf("\nNo such card host and stop\n");
        vTaskDelete(NULL);
    }

    am_util_stdio_printf("\ncard host is found\n");

    am_hal_card_t eMMCard;

    //
    // check if card is present
    //
    if (am_hal_card_host_find_card(pSdhcCardHost, &eMMCard) != AM_HAL_STATUS_SUCCESS)
    {
        TEST_ASSERT_TRUE(false);
        am_util_stdio_printf("\nNo card is present now\n");
        vTaskDelete(NULL);
    }

    if (am_hal_card_init(&eMMCard, AM_HAL_CARD_TYPE_EMMC, NULL, AM_HAL_CARD_PWR_CTRL_SDHC_OFF) == AM_HAL_STATUS_FAIL)
    {
        TEST_ASSERT_TRUE(false);
        am_util_stdio_printf("\ncard and host is not ready.\n");
        vTaskDelete(NULL);
    }

    //
    // set the card type to eMMC by using 48MHz and 8-bit mode for read and write
    //
    ui32Status = am_hal_card_cfg_set(&eMMCard, AM_HAL_CARD_TYPE_EMMC,
        AM_HAL_HOST_BUS_WIDTH_8, 48000000, AM_HAL_HOST_BUS_VOLTAGE_1_8,
        AM_HAL_HOST_UHS_DDR50);
    if (ui32Status != AM_HAL_STATUS_SUCCESS)
    {
        TEST_ASSERT_TRUE(false);
        am_util_stdio_printf("\nam_hal_card_cfg_set FAILED with AM_HAL_STATUS=%d\n", ui32Status);
        vTaskDelete(NULL);
    }

    uint32_t start_block = 0;
    uint32_t end_block = start_block + MSG_BUF_BLOCKS;
    uint32_t transfer_count = 0;

    //
    // Initialize the test read and write buffers
    //
    for(int i = 0; i < MSG_BUF_BYTES; i++)
    {
#if (PROCESS_SENSORS_TASK_ENABLE == 1)
        g_pui8RxSensorDataBuf[i] = 0x5A;
#else
        g_pui8RxSensorDataBuf[i] = i % 128;
#endif
        g_pui8RxDataVerifyBuf[i] = 0x0;
    }

    uint32_t i = 0;

    for ( ; ; )
    {

#if (PROCESS_SENSORS_TASK_ENABLE == 1)
        xReceivedBytes = xMessageBufferReceive(g_xSensorDataMessageBuffer, g_pui8RxSensorDataBuf, MSG_BUF_BYTES, portMAX_DELAY);
        am_util_stdio_printf("\n%d Bytes Received in Message Buffer\n, Iteration: %d, Unity Test Failures: %d\r\n", xReceivedBytes, i++, Unity.CurrentTestFailed);
#else
        am_util_stdio_printf("\nIteration: %d, Unity Test Failures: %d\r\n", i++, Unity.CurrentTestFailed);
        vTaskDelay(500);
#endif
        am_hal_gpio_output_set(GPIO_EMMC_TASK);

        //am_util_stdio_printf("\neMMC card wake up!\n");
        am_hal_card_pwrctrl_wakeup(&eMMCard);

        am_util_stdio_printf("\nWriting %d blocks, start_block=%d\n", MSG_BUF_BLOCKS, start_block);
			
        //
        // make sure dma memory test buffer do cache cleaning
        //
        ui32BufAddr = (uint32_t)g_pui8RxSensorDataBuf;
        if ( ui32BufAddr >= SSRAM_BASEADDR )
        {
            //
            // Clean dcache data before write.
            //
            if ( MSG_BUF_BLOCKS*512 >= DCACHE_SIZE ) // if ssram buffer > 64K, clean all is efficient
            {
                am_hal_cachectrl_dcache_clean(NULL);
            }
            else
            {
                sRange.ui32StartAddr = ui32BufAddr;
                sRange.ui32Size = MSG_BUF_BLOCKS*512;
                am_hal_cachectrl_dcache_clean(&sRange);
            }
        }

        //
        // write message buffer bytes to emmc flash
        //
        ui32Status = am_hal_card_block_write_sync(&eMMCard, start_block, MSG_BUF_BLOCKS, (uint8_t *)g_pui8RxSensorDataBuf);
        if((ui32Status & 0xFFFF) != AM_HAL_STATUS_SUCCESS)
        {
            TEST_ASSERT_TRUE(false);
            am_util_stdio_printf("\nam_hal_card_block_write_sync FAILED with AM_HAL_STATUS=%d\n", ui32Status);
        }



        ui32BufAddr = (uint32_t)g_pui8RxDataVerifyBuf;
        if ( ui32BufAddr >= SSRAM_BASEADDR )
        {
            //
            // Clean dcache data before write.
            //
            if ( MSG_BUF_BLOCKS*512 >= DCACHE_SIZE ) // if ssram buffer > 64K, clean all is efficient
            {
                am_hal_cachectrl_dcache_clean(NULL);
            }
            else
            {
                sRange.ui32StartAddr = ui32BufAddr;
                sRange.ui32Size = MSG_BUF_BLOCKS*512;
                am_hal_cachectrl_dcache_clean(&sRange);
            }
        }
				
				
        am_util_stdio_printf("Read back %d blocks, start_block=%d\n", MSG_BUF_BLOCKS, start_block);

        ui32Status = am_hal_card_block_read_sync(&eMMCard, start_block, MSG_BUF_BLOCKS, (uint8_t *)g_pui8RxDataVerifyBuf);
        if((ui32Status & 0xFFFF) != AM_HAL_STATUS_SUCCESS)
        {
            TEST_ASSERT_TRUE(false);
            am_util_stdio_printf("\nam_hal_card_block_read_sync FAILED with AM_HAL_STATUS=%d\n", ui32Status);
        }

        //am_util_stdio_printf("eMMC card power down...\n");
        am_hal_card_pwrctrl_sleep(&eMMCard);

        ui32BufAddr = (uint32_t)g_pui8RxDataVerifyBuf;
        if ( ui32BufAddr >= SSRAM_BASEADDR )
        {
            //
            // Clean dcache data before write.
            //
            if ( MSG_BUF_BLOCKS*512 >= DCACHE_SIZE ) // if ssram buffer > 64K, clean all is efficient
            {
                am_hal_cachectrl_dcache_invalidate(NULL, true);
            }
            else
            {
                sRange.ui32StartAddr = ui32BufAddr;
                sRange.ui32Size = MSG_BUF_BLOCKS*512;
                am_hal_cachectrl_dcache_invalidate(&sRange, false);
            }
        }				
				
        //
        // check if block data match or not
        //
        check_if_data_match((uint8_t *)g_pui8RxDataVerifyBuf, (uint8_t *)g_pui8RxSensorDataBuf, MSG_BUF_BYTES);

        //
        // Clear the Verify buffer
        //
        for (int i = 0; i < MSG_BUF_BYTES; i++)
        {
            g_pui8RxDataVerifyBuf[i] = 0;
        }

        start_block += MSG_BUF_BLOCKS;
        end_block = start_block + MSG_BUF_BLOCKS;
        if (end_block > eMMCard.ui32MaxBlks)
        {
            start_block = 0;
        } 

        //if(transfer_count++ > EMMC_XFER_LIM)
        //{
        //    g_bTestEnd = true;
        //    break;
        //}
        

        am_hal_gpio_output_clear(GPIO_EMMC_TASK);
    } // End of for loop

    g_bEmmcTaskComplete = true;

    am_util_stdio_printf("\neMMC task end!\n");
    //
    // Deinit the emmc.
    //
    ui32Status = am_hal_card_deinit(&eMMCard);
    TEST_ASSERT_TRUE(AM_HAL_STATUS_SUCCESS == ui32Status);
    if (AM_HAL_STATUS_SUCCESS != ui32Status)
    {
        am_util_stdio_printf("Failed to deinit eMMC correctly!\n");
    }
    vTaskDelete(NULL);
    while (1);
}

