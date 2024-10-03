//*****************************************************************************
//
//! @file emmc0_task.c
//!
//! @brief emmc0 task.
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
#define CONFIG_EMMC_TEST_MODE     1

#define SDIO_HOST_INSTANCE0      AM_HAL_SDHC_CARD_HOST
#define START_BLK                0
#define EMMC0_BLK_NUM_MAX        128
#define EMMC0_BLK_SIZE           512
#define EMMC0_BUF_LEN            EMMC0_BLK_NUM_MAX*EMMC0_BLK_SIZE

#define SPEED_INDEX_START       1
#define SPEED_INDEX_END         4

#define XFER_MODE_INDEX_START       AM_WIDGET_HOST_XFER_ADMA_SYNC
#define XFER_MODE_INDEX_END         AM_WIDGET_HOST_XFER_ADMA_ASYNC

//SDIO DMA timeout
#define SDIO_DMA_TIMEOUT      (0x1000UL)

// EMMC0 task delay
#define MAX_SDIO_TASK_DELAY    (6)

#ifdef EMMC_GPIO_DEBUG
#define EMMC_DEBUG_GPIO         9
#endif

//*****************************************************************************
//
// Semaphores used to sync emmc task with emmc task1.
//
//*****************************************************************************
SemaphoreHandle_t g_semEMMC0Write = NULL;
SemaphoreHandle_t g_semEMMC0Read = NULL;
SemaphoreHandle_t g_semEMMC0Start = NULL;
extern SemaphoreHandle_t g_semEMMC1End;

//*****************************************************************************
//
// Global Variables
//
//*****************************************************************************
static volatile bool bAsyncWriteIsDone = false;
static volatile bool bAsyncReadIsDone  = false;

static am_widget_sdio_test_t g_sEmmcCfg[TEST_CONFIG_NUM];
static am_widget_sdio_test_t EMMC_STRESS_TEST_Config =
{
    .eHost = SDIO_HOST_INSTANCE0,
    .eXferMode = AM_WIDGET_HOST_XFER_ADMA_ASYNC,
    .eClock = AM_WIDGET_SDIO_48M,
    .eBusWidth = AM_HAL_HOST_BUS_WIDTH_4,
    .eBusVoltage = AM_HAL_HOST_BUS_VOLTAGE_1_8,
    .eUHSMode = AM_HAL_HOST_UHS_SDR50,
};


//*****************************************************************************
//
// EMMC0 Buffer.
//
//*****************************************************************************
#ifdef SDIO_SSRAM_TEST
AM_SHARED_RW uint8_t ui8Emmc0RdBuf[EMMC0_BUF_LEN] __attribute__((aligned(32))); //algined 32 byte to match a cache line
AM_SHARED_RW uint8_t ui8Emmc0WrBuf[EMMC0_BUF_LEN] __attribute__((aligned(32))); //algined 32 byte to match a cache line
#else
static uint8_t ui8Emmc0RdBuf[EMMC0_BUF_LEN] ALIGN(12);
static uint8_t ui8Emmc0WrBuf[EMMC0_BUF_LEN] ALIGN(12);
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
        xSemaphoreGiveFromISR(g_semEMMC0Read, NULL);
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
        xSemaphoreGiveFromISR(g_semEMMC0Write, NULL);
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
emmc0_test_cfg_init(void)
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
                    g_sEmmcCfg[l].eBusWidth = sdio_test_widths[i].width;
                    g_sEmmcCfg[l].eUHSMode = sdio_test_modes[k].mode;
                    g_sEmmcCfg[l].eClock = (am_widget_sdio_clock_e)j;
                    g_sEmmcCfg[l].eHost = SDIO_HOST_INSTANCE0;
                    g_sEmmcCfg[l].eXferMode = sdio_test_xfer_modes[m].mode;
                    g_sEmmcCfg[l].eBusVoltage = AM_HAL_HOST_BUS_VOLTAGE_1_8;
                    l++;
                }
            }
        }
    }
}

bool
emmc0_config_init(uint32_t ui32index)
{
    am_widget_sdio_config_t *pHandles = (am_widget_sdio_config_t *)(&g_DUTs[DUTs_setting[SDIO_HOST_INSTANCE0].eHost]);

    am_util_debug_printf("Emmc0 Stress Test Mode Config\n");

#if CONFIG_EMMC_TEST_MODE
    DUTs_setting[SDIO_HOST_INSTANCE0].eClock = g_sEmmcCfg[ui32index].eClock;
    DUTs_setting[SDIO_HOST_INSTANCE0].eBusWidth = g_sEmmcCfg[ui32index].eBusWidth;
    DUTs_setting[SDIO_HOST_INSTANCE0].eUHSMode = g_sEmmcCfg[ui32index].eUHSMode;
    DUTs_setting[SDIO_HOST_INSTANCE0].eXferMode = g_sEmmcCfg[ui32index].eXferMode;
#else
    DUTs_setting[SDIO_HOST_INSTANCE0].eClock = EMMC_STRESS_TEST_Config.eClock;
    DUTs_setting[SDIO_HOST_INSTANCE0].eBusWidth = EMMC_STRESS_TEST_Config.eBusWidth;
    DUTs_setting[SDIO_HOST_INSTANCE0].eUHSMode = EMMC_STRESS_TEST_Config.eUHSMode;
    DUTs_setting[SDIO_HOST_INSTANCE0].eXferMode = EMMC_STRESS_TEST_Config.eXferMode;
#endif

    DUTs_setting[SDIO_HOST_INSTANCE0].sTimingParam = am_widget_findTimingParam((am_widget_timing_scan_hashtable_t *)&eMMC_timing_table[SDIO_HOST_INSTANCE0], &DUTs_setting[SDIO_HOST_INSTANCE0]);

    if(DUTs_setting[SDIO_HOST_INSTANCE0].eUHSMode == AM_HAL_HOST_UHS_DDR50 && DUTs_setting[SDIO_HOST_INSTANCE0].eClock != AM_WIDGET_SDIO_48M)
    {
        DUTs_setting[SDIO_HOST_INSTANCE0].eClock = EMMC_STRESS_TEST_Config.eClock;
    }

    //
    // initialize the test read and write buffers
    //

    am_widget_prepare_data_pattern((ui32index % SDIO_TEST_PATTERN_NUMBER), ui8Emmc0WrBuf, EMMC0_BUF_LEN);

    am_util_debug_printf("SDIO host: %d\n",DUTs_setting[SDIO_HOST_INSTANCE0].eHost);
    am_util_debug_printf("Bus Speed Mode = %s\n", sdio_test_modes[DUTs_setting[SDIO_HOST_INSTANCE0].eUHSMode/2-1].string);
    am_util_debug_printf("%s clock = %s, bit width = %d\n", sdio_test_xfer_modes[DUTs_setting[SDIO_HOST_INSTANCE0].eXferMode].string, 
                                                            sdio_test_speeds[0+DUTs_setting[SDIO_HOST_INSTANCE0].eClock].string, 
                                                            DUTs_setting[SDIO_HOST_INSTANCE0].eBusWidth);

    TEST_ASSERT_TRUE(am_widget_sdio_setup(&g_DUTs[DUTs_setting[SDIO_HOST_INSTANCE0].eHost], &DUTs_setting[SDIO_HOST_INSTANCE0]));

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
sdio_emmc0_stress_test(void)
{
    uint32_t randomDelay;
    uint32_t ui32Stat = AM_HAL_STATUS_SUCCESS;
    uint32_t ui32BufAddr = 0;

    am_hal_cachectrl_range_t sRange;

    am_widget_sdio_config_t *pHandles = (am_widget_sdio_config_t *)(&g_DUTs[DUTs_setting[SDIO_HOST_INSTANCE0].eHost]);
    am_widget_sdio_test_t *pSDHCTestConfig = (am_widget_sdio_test_t *)(&DUTs_setting[SDIO_HOST_INSTANCE0]);

#ifdef EMMC_GPIO_DEBUG
    am_hal_gpio_pinconfig(EMMC_DEBUG_GPIO, am_hal_gpio_pincfg_output);
#endif

    for (uint32_t mode_index = 0; mode_index < TEST_CONFIG_NUM; mode_index++)
    {
        //
        // Emmc1 different mode set
        //
        emmc0_config_init(mode_index);
        am_util_stdio_printf("emmc0 mode:%d\n", mode_index);

        pSDHCTestConfig->ui32BlockCount = EMMC0_BLK_NUM_MAX;
        for(pSDHCTestConfig->ui32StartBlock = START_BLK; pSDHCTestConfig->ui32StartBlock < pSDHCTestConfig->ui32SectorCount; pSDHCTestConfig->ui32StartBlock += pSDHCTestConfig->ui32SectorCount/2 - pSDHCTestConfig->ui32BlockCount)
        {
#ifdef EMMC_GPIO_DEBUG
            am_hal_gpio_state_write(EMMC_DEBUG_GPIO, AM_HAL_GPIO_OUTPUT_SET);
#endif
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
            // Task Delay Set.
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
            //
            // emmc0 write start semaphore
            //
            xSemaphoreGive(g_semEMMC0Start);

            //
            // make sure dma memory test buffer do cache cleaning
            //
            ui32BufAddr = (uint32_t)ui8Emmc0WrBuf;
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
                ui32Stat = am_hal_card_block_write_async(pHandles->pDevHandle, pSDHCTestConfig->ui32StartBlock, pSDHCTestConfig->ui32BlockCount, (uint8_t *)ui8Emmc0WrBuf);
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
                BaseType_t ret = xSemaphoreTake( g_semEMMC0Write, SDIO_DMA_TIMEOUT );
                TEST_ASSERT_TRUE(ret != pdFALSE);
                if ( ret == pdFALSE )
                {
                
                    am_util_stdio_printf("sdio DMA timeout!\n");
                }
            }
            else
            {
                ui32Stat = am_hal_card_block_write_sync(pHandles->pDevHandle, pSDHCTestConfig->ui32StartBlock, pSDHCTestConfig->ui32BlockCount, (uint8_t *)ui8Emmc0WrBuf);
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
            memset((void*)ui8Emmc0RdBuf, 0, EMMC0_BUF_LEN);

            ui32BufAddr = (uint32_t)ui8Emmc0RdBuf;
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
                ui32Stat = am_hal_card_block_read_async(pHandles->pDevHandle, pSDHCTestConfig->ui32StartBlock, pSDHCTestConfig->ui32BlockCount, (uint8_t *)ui8Emmc0RdBuf);
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
                BaseType_t ret = xSemaphoreTake( g_semEMMC0Read, SDIO_DMA_TIMEOUT );
                TEST_ASSERT_TRUE(ret != pdFALSE);
                if ( ret == pdFALSE )
                {
                    am_util_stdio_printf("sdio DMA timeout!\n");
                }
            }
            else 
            {
                ui32Stat = am_hal_card_block_read_sync(pHandles->pDevHandle, pSDHCTestConfig->ui32StartBlock, pSDHCTestConfig->ui32BlockCount, (uint8_t *)ui8Emmc0RdBuf);
                am_util_debug_printf("Synchronous Read Start, Read Status = %d\n", ui32Stat);
                if((ui32Stat & 0xffff) != 0 )
                {
                    am_util_stdio_printf("Failed to read card.\n");
                    return false;
                }

                ui32BufAddr = (uint32_t)ui8Emmc0RdBuf;
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
            TEST_ASSERT_TRUE(am_widget_check_data_match((uint8_t *)ui8Emmc0RdBuf, (uint8_t *)ui8Emmc0WrBuf, 512*pSDHCTestConfig->ui32BlockCount));

            //
            // wait for emmc1 test case read finish
            //
            xSemaphoreTake( g_semEMMC1End, SDIO_DMA_TIMEOUT );
        }
    }
    return true;
}

//*****************************************************************************
//
// EMMC0 task.
//
//*****************************************************************************
void Emmc0Task(void *pvParameters)
{
    am_util_stdio_printf("\nEmmc0 task start\n");

    g_semEMMC0Start = xSemaphoreCreateBinary();
    TEST_ASSERT_TRUE(g_semEMMC0Start != NULL);

    TEST_ASSERT_TRUE(sdio_emmc0_stress_test());

    vSemaphoreDelete(g_semEMMC0Write);
    vSemaphoreDelete(g_semEMMC0Read);
    vSemaphoreDelete(g_semEMMC0Start);

    TEST_ASSERT_TRUE(am_widget_sdio_cleanup(&g_DUTs[DUTs_setting[SDIO_HOST_INSTANCE0].eHost]));
    am_util_stdio_printf("\nEmmc0: task end!\n");

    vTaskDelete(NULL);
    while (1);
}

