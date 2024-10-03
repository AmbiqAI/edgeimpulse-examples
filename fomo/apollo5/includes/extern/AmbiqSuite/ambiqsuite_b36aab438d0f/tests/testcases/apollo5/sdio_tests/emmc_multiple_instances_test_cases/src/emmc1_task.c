//*****************************************************************************
//
//! @file emmc1_task.c
//!
//! @brief emmc1 task.
//
//*****************************************************************************

//*****************************************************************************
//
// ${copyright}
//
// This is part of revision ${version} of the AmbiqSuite Development Package.
//
//*****************************************************************************

#include "unity.h"
#include "am_mcu_apollo.h"
#include "am_widget_sdio.h"
#include "am_util.h"
#include "sdio_test_common.h"
#include "emmc_multiple_instances_test_cases.h"
#include "task.h"
#include "FreeRTOS.h"

//*****************************************************************************
//
// Macro definitions
//
//*****************************************************************************
#define CONFIG_EMMC1_TEST_MODE    1

#define SDIO_HOST_INSTANCE1      AM_HAL_SDHC_CARD_HOST1
#define START_BLK                0
#define EMMC1_BLK_NUM_MAX        128
#define EMMC1_BLK_SIZE           512
#define EMMC1_BUF_LEN            EMMC1_BLK_NUM_MAX*EMMC1_BLK_SIZE

#define SPEED_INDEX_START       1
#define SPEED_INDEX_END         4

#define XFER_MODE_INDEX_START       AM_WIDGET_HOST_XFER_ADMA_SYNC
#define XFER_MODE_INDEX_END         AM_WIDGET_HOST_XFER_ADMA_ASYNC

//SDIO DMA timeout
#define SDIO_DMA_TIMEOUT      (0x1000UL)

// EMMC1 task delay
#define MAX_SDIO_TASK_DELAY    (6)

//*****************************************************************************
//
// Semaphores used to sync emmc task with emmc1 task.
//
//*****************************************************************************
SemaphoreHandle_t g_semEMMC1Write = NULL;
SemaphoreHandle_t g_semEMMC1Read = NULL;
SemaphoreHandle_t g_semEMMC1End ;

//*****************************************************************************
//
// Global Variables
//
//*****************************************************************************
static volatile bool bAsyncWriteIsDone = false;
static volatile bool bAsyncReadIsDone  = false;

static am_widget_sdio_test_t g_sEmmc1Cfg[TEST_CONFIG_NUM];
static am_widget_sdio_test_t EMMC_STRESS_TEST_Config =
{
    .eHost = SDIO_HOST_INSTANCE1,
    .eXferMode = AM_WIDGET_HOST_XFER_ADMA_ASYNC,
    .eClock = AM_WIDGET_SDIO_48M,
    .eBusWidth = AM_HAL_HOST_BUS_WIDTH_4,
    .eBusVoltage = AM_HAL_HOST_BUS_VOLTAGE_1_8,
    .eUHSMode = AM_HAL_HOST_UHS_SDR50,
};

//*****************************************************************************
//
// EMMC1 Buffer.
//
//*****************************************************************************
#ifdef SDIO_SSRAM_TEST
AM_SHARED_RW uint8_t ui8Emmc1RdBuf[EMMC1_BUF_LEN] __attribute__((aligned(32))); //algined 32 byte to match a cache line
AM_SHARED_RW uint8_t ui8Emmc1WrBuf[EMMC1_BUF_LEN] __attribute__((aligned(32))); //algined 32 byte to match a cache line
#else
static uint8_t ui8Emmc1RdBuf[EMMC1_BUF_LEN] ALIGN(12);
static uint8_t ui8Emmc1WrBuf[EMMC1_BUF_LEN] ALIGN(12);
#endif

//*****************************************************************************
//
// SDIO interrupt callback.
//
//*****************************************************************************
static void am_hal_card_event_test_cb(am_hal_host_evt_t *pEvt)
{
    am_hal_card_host_t *pHost = (am_hal_card_host_t *)pEvt->pCtx;
    am_hal_cachectrl_range_t sRange;
    uint32_t ui32BufLen = 0;
    uint32_t ui32BufAddr = 0;

    if (AM_HAL_EVT_XFER_COMPLETE == pEvt->eType &&
        pHost->AsyncCmdData.dir == AM_HAL_DATA_DIR_READ)
    {
        xSemaphoreGiveFromISR(g_semEMMC1Read, NULL);
        am_util_debug_printf("Last Read Xfered block %d\n", pEvt->ui32BlkCnt);

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
        xSemaphoreGiveFromISR(g_semEMMC1Write, NULL);
        am_util_debug_printf("Last Write Xfered block %d\n", pEvt->ui32BlkCnt);
    }

    if (AM_HAL_EVT_DAT_ERR == pEvt->eType)
    {
        am_util_stdio_printf("Data error type %d\n", pHost->AsyncCmdData.eDataError);
    }
}

//*****************************************************************************
//
// EMMC0 task configurations init.
//
//*****************************************************************************
void
emmc1_test_cfg_init(void)
{
    uint32_t i, j, k, m, l = 0;
    for(m = XFER_MODE_INDEX_START; m <= XFER_MODE_INDEX_END; m++)
    {
        for (i = 1; i < WIDTH_END_INDEX; i++)
        {
            for (j = SPEED_INDEX_START; j < SPEED_INDEX_END; j++)
            {
                for (k = MODE_START_INDEX; k < MODE_END_INDEX; k++)
                {
                    g_sEmmc1Cfg[l].eBusWidth = sdio_test_widths[i].width;
                    g_sEmmc1Cfg[l].eUHSMode = sdio_test_modes[k].mode;
                    g_sEmmc1Cfg[l].eClock = (am_widget_sdio_clock_e)j;
                    g_sEmmc1Cfg[l].eHost = SDIO_HOST_INSTANCE1;
                    g_sEmmc1Cfg[l].eXferMode = sdio_test_xfer_modes[m].mode;
                    g_sEmmc1Cfg[l].eBusVoltage = AM_HAL_HOST_BUS_VOLTAGE_1_8;
                    l++;
                }
            }
        }
    }
}

bool
emmc1_config_init(uint32_t ui32index)
{
    am_widget_sdio_config_t *pHandles = (am_widget_sdio_config_t *)(&g_DUTs[DUTs_setting[SDIO_HOST_INSTANCE1].eHost]);

    am_util_debug_printf("Emmc1 Stress Test Mode Config\n");

#if CONFIG_EMMC1_TEST_MODE
    DUTs_setting[SDIO_HOST_INSTANCE1].eClock = g_sEmmc1Cfg[ui32index].eClock;
    DUTs_setting[SDIO_HOST_INSTANCE1].eBusWidth = g_sEmmc1Cfg[ui32index].eBusWidth;
    DUTs_setting[SDIO_HOST_INSTANCE1].eUHSMode = g_sEmmc1Cfg[ui32index].eUHSMode;
    DUTs_setting[SDIO_HOST_INSTANCE1].eXferMode = g_sEmmc1Cfg[ui32index].eXferMode;
#else
    DUTs_setting[SDIO_HOST_INSTANCE1].eClock = EMMC_STRESS_TEST_Config.eClock;
    DUTs_setting[SDIO_HOST_INSTANCE1].eBusWidth = EMMC_STRESS_TEST_Config.eBusWidth;
    DUTs_setting[SDIO_HOST_INSTANCE1].eUHSMode = EMMC_STRESS_TEST_Config.eUHSMode;
    DUTs_setting[SDIO_HOST_INSTANCE1].eXferMode = EMMC_STRESS_TEST_Config.eXferMode;
#endif

    DUTs_setting[SDIO_HOST_INSTANCE1].sTimingParam = am_widget_findTimingParam((am_widget_timing_scan_hashtable_t *)&eMMC_timing_table[SDIO_HOST_INSTANCE1], &DUTs_setting[SDIO_HOST_INSTANCE1]);

    if(DUTs_setting[SDIO_HOST_INSTANCE1].eUHSMode == AM_HAL_HOST_UHS_DDR50 && DUTs_setting[SDIO_HOST_INSTANCE1].eClock != AM_WIDGET_SDIO_48M)
    {
        DUTs_setting[SDIO_HOST_INSTANCE1].eClock = EMMC_STRESS_TEST_Config.eClock;
    }

    //
    // initialize the test read and write buffers
    //
    am_widget_prepare_data_pattern((ui32index % SDIO_TEST_PATTERN_NUMBER), ui8Emmc1WrBuf, EMMC1_BUF_LEN);

    am_util_debug_printf("SDIO host: %d\n",DUTs_setting[SDIO_HOST_INSTANCE1].eHost);
    am_util_debug_printf("Bus Speed Mode = %s\n", sdio_test_modes[DUTs_setting[SDIO_HOST_INSTANCE1].eUHSMode/2-1].string);
    am_util_debug_printf("%s, clock = %s, bit width = %d\n", sdio_test_xfer_modes[DUTs_setting[SDIO_HOST_INSTANCE1].eXferMode].string,
                                                            sdio_test_speeds[0+DUTs_setting[SDIO_HOST_INSTANCE1].eClock].string, 
                                                            DUTs_setting[SDIO_HOST_INSTANCE1].eBusWidth);

    TEST_ASSERT_TRUE(am_widget_sdio_setup(&g_DUTs[DUTs_setting[SDIO_HOST_INSTANCE1].eHost], &DUTs_setting[SDIO_HOST_INSTANCE1]));

    //
    // Set transfer mode as AMDA mode
    //
    am_hal_card_host_set_xfer_mode(pHandles->pSdhcHost, AM_HAL_HOST_XFER_ADMA);

    //
    // async read & write, card insert & remove needs a callback function
    //
    am_hal_card_register_evt_callback(pHandles->pDevHandle, am_hal_card_event_test_cb);

    return true;
}

static bool
sdio_emmc1_stress_test(void)
{
    uint32_t ui32Stat = AM_HAL_STATUS_SUCCESS;
    uint32_t randomDelay;
    uint32_t ui32BufAddr = 0;

    am_hal_cachectrl_range_t sRange;

    am_widget_sdio_config_t *pHandles = (am_widget_sdio_config_t *)(&g_DUTs[DUTs_setting[SDIO_HOST_INSTANCE1].eHost]);
    am_widget_sdio_test_t *pSDHCTestConfig = (am_widget_sdio_test_t *)(&DUTs_setting[SDIO_HOST_INSTANCE1]);

    for (uint32_t mode_index = 0; mode_index < TEST_CONFIG_NUM; mode_index++)
    {
        //
        // Emmc1 different mode set
        //
        emmc1_config_init(mode_index);
        am_util_stdio_printf("emmc1 mode:%d\n", mode_index);

        pSDHCTestConfig->ui32BlockCount = EMMC1_BLK_NUM_MAX;
        for(pSDHCTestConfig->ui32StartBlock = START_BLK; pSDHCTestConfig->ui32StartBlock < pSDHCTestConfig->ui32SectorCount; pSDHCTestConfig->ui32StartBlock += pSDHCTestConfig->ui32SectorCount/2 - pSDHCTestConfig->ui32BlockCount)
        {
            am_util_debug_printf("sdio host%d write read test\n", pSDHCTestConfig->eHost);
            am_util_debug_printf("start_blk = %d, blk_cnt = %d\n",pSDHCTestConfig->ui32StartBlock, pSDHCTestConfig->ui32BlockCount);

#ifdef ENABLE_SDIO_POWER_SAVE
            //
            // power down SDIO peripheral
            //
            ui32Stat = am_hal_card_pwrctrl_sleep(pHandles->pDevHandle);
            if(ui32Stat != AM_HAL_STATUS_SUCCESS)
            {
                am_util_stdio_printf("Failed to power down card\n");
                return false;
            }

            //
            // power up SDIO peripheral
            //
            ui32Stat = am_hal_card_pwrctrl_wakeup(pHandles->pDevHandle);
            if(ui32Stat != AM_HAL_STATUS_SUCCESS)
            {
                am_util_stdio_printf("Failed to power up card.\n");
                return false;
            }
#endif
            //
            // erase eMMC before write
            //
            ui32Stat = am_hal_card_block_erase(pHandles->pDevHandle, pSDHCTestConfig->ui32StartBlock, pSDHCTestConfig->ui32BlockCount, AM_HAL_ERASE, 100);
            am_util_debug_printf("erase eMMC before write, Erase Status = %d\n", ui32Stat);
            if(ui32Stat != 0x0)
            {
                am_util_stdio_printf("erase eMMC failed\n");
                return false;
            }
            //
            //  Task Delay Set.
            //
            srand(xTaskGetTickCount());
            randomDelay = rand() % MAX_SDIO_TASK_DELAY;
            vTaskDelay(randomDelay);

#ifdef ENABLE_SDIO_POWER_SAVE
            //
            // power down SDIO peripheral
            //
            ui32Stat = am_hal_card_pwrctrl_sleep(pHandles->pDevHandle);
            if(ui32Stat != AM_HAL_STATUS_SUCCESS)
            {
                am_util_stdio_printf("Failed to power down card\n");
                return false;
            }

            //
            // power up SDIO peripheral
            //
            ui32Stat = am_hal_card_pwrctrl_wakeup(pHandles->pDevHandle);
            if(ui32Stat != AM_HAL_STATUS_SUCCESS)
            {
                am_util_stdio_printf("Failed to power up card.\n");
                return false;
            }
#endif

            xSemaphoreTake( g_semEMMC0Start, SDIO_DMA_TIMEOUT );

            //
            // make sure dma memory test buffer do cache cleaning
            //
            ui32BufAddr = (uint32_t)ui8Emmc1WrBuf;
            if ( ui32BufAddr >= SSRAM_BASEADDR )
            {
                //
                // Clean dcache data before write.
                //
                if ( pSDHCTestConfig->ui32BlockCount*512 > DCACHE_SIZE ) // if ssram buffer > 64K, clean all is efficient
                {
                    am_hal_cachectrl_dcache_clean(NULL);
                }
                else
                {
                    sRange.ui32StartAddr = ui32BufAddr;
                    sRange.ui32Size = pSDHCTestConfig->ui32BlockCount*512;
                    am_hal_cachectrl_dcache_clean(&sRange);
                }
            }

            if(pSDHCTestConfig->eXferMode == AM_WIDGET_HOST_XFER_ADMA_ASYNC)
            {
                ui32Stat = am_hal_card_block_write_async(pHandles->pDevHandle, pSDHCTestConfig->ui32StartBlock, pSDHCTestConfig->ui32BlockCount, (uint8_t *)ui8Emmc1WrBuf);
                am_util_debug_printf("Asynchronous ADMA Write Start, Write Status = %d\n", ui32Stat);
                TEST_ASSERT_EQUAL_HEX32(0x0, ui32Stat);
                if(ui32Stat != 0)
                {
                    am_util_stdio_printf("Failed to write card.\n");
                    return false;
                }

                //
                // wait until the async write is done
                //
                BaseType_t ret = xSemaphoreTake( g_semEMMC1Write, SDIO_DMA_TIMEOUT );
                TEST_ASSERT_TRUE(ret != pdFALSE);
                if ( ret == pdFALSE )
                {
                
                    am_util_stdio_printf("sdio DMA timeout!\n");
                }
            }
            else
            {
                ui32Stat = am_hal_card_block_write_sync(pHandles->pDevHandle, pSDHCTestConfig->ui32StartBlock, pSDHCTestConfig->ui32BlockCount, (uint8_t *)ui8Emmc1WrBuf);
                am_util_debug_printf("Synchronous Write Start, Write Status = %d\n", ui32Stat);
                if((ui32Stat & 0xffff) != 0 )
                {
                    am_util_stdio_printf("Failed to write card.\n");
                    return false;
                }
            }

#ifdef ENABLE_SDIO_POWER_SAVE
            //
            // power down SDIO peripheral
            //
            ui32Stat = am_hal_card_pwrctrl_sleep(pHandles->pDevHandle);
            if(ui32Stat != AM_HAL_STATUS_SUCCESS)
            {
                am_util_stdio_printf("Failed to power down card\n");
                return false;
            }

            //
            // power up SDIO peripheral
            //
            ui32Stat = am_hal_card_pwrctrl_wakeup(pHandles->pDevHandle);
            if(ui32Stat != AM_HAL_STATUS_SUCCESS)
            {
                am_util_stdio_printf("Failed to power up card.\n");
                return false;
            }
#endif

            //
            // read back data from eMMC
            //
            memset((void*)ui8Emmc1RdBuf, 0, EMMC1_BUF_LEN);

            ui32BufAddr = (uint32_t)ui8Emmc1RdBuf;
            if ( ui32BufAddr >= SSRAM_BASEADDR )
            {
                //
                // Clean dcache data before write.
                //
                if ( pSDHCTestConfig->ui32BlockCount*512 > DCACHE_SIZE ) // if ssram buffer > 64K, clean all is efficient
                {
                    am_hal_cachectrl_dcache_clean(NULL);
                }
                else
                {
                    sRange.ui32StartAddr = ui32BufAddr;
                    sRange.ui32Size = pSDHCTestConfig->ui32BlockCount*512;
                    am_hal_cachectrl_dcache_clean(&sRange);
                }
            }

            if (pSDHCTestConfig->eXferMode == AM_WIDGET_HOST_XFER_ADMA_ASYNC)
            {
                ui32Stat = am_hal_card_block_read_async(pHandles->pDevHandle, pSDHCTestConfig->ui32StartBlock, pSDHCTestConfig->ui32BlockCount, (uint8_t *)ui8Emmc1RdBuf);
                am_util_debug_printf("Asynchronous ADMA Read Start, Read Status = %d\n", ui32Stat);
                TEST_ASSERT_EQUAL_HEX32(0x0, ui32Stat);
                if(ui32Stat != 0)
                {
                    am_util_stdio_printf("Failed to read card.\n");
                    return false;
                }

                //
                // wait until the async read is done
                //
                BaseType_t ret = xSemaphoreTake( g_semEMMC1Read, SDIO_DMA_TIMEOUT );
                TEST_ASSERT_TRUE(ret != pdFALSE);
                if ( ret == pdFALSE )
                {
                    am_util_stdio_printf("sdio DMA timeout!\n");
                }
            }
            else 
            {
                ui32Stat = am_hal_card_block_read_sync(pHandles->pDevHandle, pSDHCTestConfig->ui32StartBlock, pSDHCTestConfig->ui32BlockCount, (uint8_t *)ui8Emmc1RdBuf);
                am_util_debug_printf("Synchronous Read Start, Read Status = %d\n", ui32Stat);
                if((ui32Stat & 0xffff) != 0 )
                {
                    am_util_stdio_printf("Failed to read card.\n");
                    return false;
                }
 
                ui32BufAddr = (uint32_t)ui8Emmc1RdBuf;
                if ( ui32BufAddr >= SSRAM_BASEADDR )
                {
                    //
                    // Clean dcache data before write.
                    //
                    if ( pSDHCTestConfig->ui32BlockCount*512 > DCACHE_SIZE ) // if ssram buffer > 64K, clean all is efficient
                    {
                        am_hal_cachectrl_dcache_invalidate(NULL, true);
                    }
                    else
                    {
                        sRange.ui32StartAddr = ui32BufAddr;
                        sRange.ui32Size = pSDHCTestConfig->ui32BlockCount*512;
                        am_hal_cachectrl_dcache_invalidate(&sRange, false);
                    }
                }
            }

            //
            // check if block data match or not
            //
            TEST_ASSERT_TRUE(am_widget_check_data_match((uint8_t *)ui8Emmc1RdBuf, (uint8_t *)ui8Emmc1WrBuf, 512*pSDHCTestConfig->ui32BlockCount));
        }

        xSemaphoreGive(g_semEMMC1End);
    }
    
    return true;
}

//*****************************************************************************
//
// EMMC1 task.
//
//*****************************************************************************
void Emmc1Task(void *pvParameters)
{
    am_util_stdio_printf("\nEmmc1 task start\n");

    g_semEMMC1End = xSemaphoreCreateBinary();
    TEST_ASSERT_TRUE(g_semEMMC1End != NULL);

    TEST_ASSERT_TRUE(sdio_emmc1_stress_test());

    vSemaphoreDelete(g_semEMMC1Write);
    vSemaphoreDelete(g_semEMMC1Read);
    vSemaphoreDelete(g_semEMMC1End);

    TEST_ASSERT_TRUE(am_widget_sdio_cleanup(&g_DUTs[DUTs_setting[SDIO_HOST_INSTANCE1].eHost]));
    am_util_stdio_printf("\nEmmc1: task end!\n");

    vTaskDelete(NULL);
    while (1);
}

