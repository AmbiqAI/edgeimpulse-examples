//*****************************************************************************
//
//! @file sdio_rs9116_wifi_interface_task.c
//!
//! @brief sdio rs9116 wifi interface task task.
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
#include "am_util.h"
#include "sdio_rs9116_emmc_multiple_instances_test_cases.h"
#include "emmc_task.h"
#include "task.h"
#include "FreeRTOS.h"

//*****************************************************************************
//
// Macro definitions
//
//*****************************************************************************

//SDIO DMA timeout
#define SDIO1_DMA_TIMEOUT      (0x1000UL)

#define MAX_SDIO1_TASK_DELAY     6
#define SDIO_TEST_BUF_LEN        RSI_BLOCK_SIZE*128
#define SDIO_TEST_ADDR           RSI_PONG_BUFFER_ADDR - SDIO_TEST_BUF_LEN

#define SDIO_RS9116_WIDTH_START_INDEX   1
#define SDIO_RS9116_WIDTH_END_INDEX     2

#define SDIO_RS9116_SPEED_START_INDEX   1
#define SDIO_RS9116_SPEED_END_INDEX     4

#if EMMC_USE_SDIO_HOST1
#define SDIO_WIFI_INSTANCE      AM_HAL_SDHC_CARD_HOST
#else
#define SDIO_WIFI_INSTANCE      AM_HAL_SDHC_CARD_HOST1
#endif

//*****************************************************************************
//
// Semaphores used to sync emmc task with emmc task.
//
//*****************************************************************************
SemaphoreHandle_t g_semSdioWrite = NULL;
SemaphoreHandle_t g_semSdioRead = NULL;
SemaphoreHandle_t g_semSdioEnd;

//*****************************************************************************
//
// RS9116 Buffer, need aligned by 4096 byte in SDMA mode
//
//*****************************************************************************
#ifdef SDIO_SSRAM_TEST
AM_SHARED_RW uint8_t ui8Rs9116RdBuf[SDIO_TEST_BUF_LEN] ALIGN(12);
AM_SHARED_RW uint8_t ui8Rs9116WrBuf[SDIO_TEST_BUF_LEN] ALIGN(12);
#else
static uint8_t ui8Rs9116RdBuf[SDIO_TEST_BUF_LEN] ALIGN(12);
static uint8_t ui8Rs9116WrBuf[SDIO_TEST_BUF_LEN] ALIGN(12);
#endif

//*****************************************************************************
//
// SDIO interrupt callback.
//
//*****************************************************************************
static void host0_card_event_test_cb(am_hal_host_evt_t *pEvt)
{
    am_hal_card_host_t *pHost = (am_hal_card_host_t *)pEvt->pCtx;
    am_widget_sdio_devices_config_t *pDevConfig = (am_widget_sdio_devices_config_t *)DUTs_setting[0].device_config;
    am_hal_cachectrl_range_t sRange;
    uint32_t ui32BufLen = 0;
    uint32_t ui32BufAddr = 0;

    if (AM_HAL_EVT_XFER_COMPLETE == pEvt->eType &&
        pHost->AsyncCmdData.dir == AM_HAL_DATA_DIR_READ)
    {
        xSemaphoreGiveFromISR(g_semSdioRead, NULL);
        pDevConfig->bAsyncReadIsDone = true;
        am_util_debug_printf("Last Read Xfered block %d\n", pEvt->ui32BlkCnt);

        //
        // Flush and invalidate cache, if ssram buffer > 64K, clean all is efficient
        //
        ui32BufAddr= (uint32_t)(pHost->AsyncCmdData.pui8Buf);
        if ( ui32BufAddr >= SSRAM_BASEADDR )
        {
            ui32BufLen = pHost->AsyncCmdData.ui32BlkCnt * pHost->AsyncCmdData.ui32BlkSize;
            if ( ui32BufLen == 0)
            {
                ui32BufLen = pHost->AsyncCmdData.ui32BlkSize;
            }

            if ( ui32BufLen >= DCACHE_SIZE )
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
        xSemaphoreGiveFromISR(g_semSdioWrite, NULL);
        pDevConfig->bAsyncWriteIsDone = true;
        am_util_debug_printf("Last Write Xfered block %d\n", pEvt->ui32BlkCnt);
    }

   if (AM_HAL_EVT_DAT_ERR == pEvt->eType)
    {
        am_util_stdio_printf("Data error type %d\n", pHost->AsyncCmdData.eDataError);
    }
}

static void host1_card_event_test_cb(am_hal_host_evt_t *pEvt)
{
    am_hal_card_host_t *pHost = (am_hal_card_host_t *)pEvt->pCtx;
    am_widget_sdio_devices_config_t *pDevConfig = (am_widget_sdio_devices_config_t *)DUTs_setting[1].device_config;
    am_hal_cachectrl_range_t sRange;
    uint32_t ui32BufLen = 0;
    uint32_t ui32BufAddr = 0;

    if (AM_HAL_EVT_XFER_COMPLETE == pEvt->eType &&
        pHost->AsyncCmdData.dir == AM_HAL_DATA_DIR_READ)
    {
        pDevConfig->bAsyncReadIsDone = true;
        xSemaphoreGiveFromISR(g_semSdioRead, NULL);
        am_util_debug_printf("Last Read Xfered block %d\n", pEvt->ui32BlkCnt);

        //
        // Flush and invalidate cache, if ssram buffer > 64K, clean all is efficient
        //
        ui32BufAddr= (uint32_t)(pHost->AsyncCmdData.pui8Buf);
        if ( ui32BufAddr >= SSRAM_BASEADDR )
        {
            ui32BufLen = pHost->AsyncCmdData.ui32BlkCnt * pHost->AsyncCmdData.ui32BlkSize;
            if ( ui32BufLen == 0)
            {
                ui32BufLen = pHost->AsyncCmdData.ui32BlkSize;
            }

            if ( ui32BufLen >= DCACHE_SIZE )
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
        pDevConfig->bAsyncWriteIsDone = true;
        xSemaphoreGiveFromISR(g_semSdioWrite, NULL);
        am_util_debug_printf("Last Write Xfered block %d\n", pEvt->ui32BlkCnt);
    }

    if (AM_HAL_EVT_DAT_ERR == pEvt->eType)
    {
        am_util_stdio_printf("Data error type %d\n", pHost->AsyncCmdData.eDataError);
    }
}

static bool
sdio_rs9116_multi_blocks_write(am_hal_card_t *pCard, am_widget_sdio_test_t *pSDHCTestConfig, uint32_t ui32Addr, uint32_t ui32Len, uint8_t *ui8WrBuf)
{
    uint32_t ui32Status;
    uint32_t ui32MemAddr;
    uint32_t no_of_blocks = 1;
    uint32_t ui32BufAddr = 0;

    am_hal_cachectrl_range_t sRange;

    uint8_t data = ((ui32Addr & 0xff000000) >> 24);
    ui32Status = am_hal_sdio_card_byte_write(pCard, 0, 0xfb, data);
    if(ui32Status != AM_HAL_STATUS_SUCCESS)
    {
        am_util_stdio_printf("Fail to write SDIO reg at addr 0xfb\n", ui32Status);
        return false;
    }

    data = ((ui32Addr & 0x00ff0000) >> 16);
    ui32Status = am_hal_sdio_card_byte_write(pCard, 0, 0xfa, data);
    if(ui32Status != AM_HAL_STATUS_SUCCESS)
    {
        am_util_stdio_printf("Fail to write SDIO reg at addr 0xfa\n", ui32Status);
        return false;
    }

    if( ui32Len > RSI_BLOCK_SIZE )
    {
        // Calculate number of blocks
        no_of_blocks = (ui32Len / RSI_BLOCK_SIZE) ;

        if (ui32Len % RSI_BLOCK_SIZE)
        {
            no_of_blocks = no_of_blocks + 1;
        }
        ui32Len = RSI_BLOCK_SIZE;
    }

    ui32MemAddr = (ui32Addr & 0xFFFF) | (1 << 16);

    //
    // make sure dma memory test buffer do cache cleaning
    //
    if ( pSDHCTestConfig->eXferMode != AM_WIDGET_HOST_XFER_PIO_SYNC)
    {
        ui32BufAddr = (uint32_t)ui8WrBuf;
        if ( ui32BufAddr >= SSRAM_BASEADDR )
        {
            //
            // Clean dcache data before write.
            //
            if ( SDIO_TEST_BUF_LEN > DCACHE_SIZE ) // if ssram buffer > 64K, clean all is efficient
            {
                am_hal_cachectrl_dcache_clean(NULL);
            }
            else
            {
                sRange.ui32StartAddr = ui32BufAddr;
                sRange.ui32Size = SDIO_TEST_BUF_LEN;
                am_hal_cachectrl_dcache_clean(&sRange);
            }
        }
    }

    if ( (pSDHCTestConfig->eXferMode == AM_WIDGET_HOST_XFER_ADMA_ASYNC) || (pSDHCTestConfig->eXferMode == AM_WIDGET_HOST_XFER_SDMA_ASYNC) )
    {
        ui32Status = am_hal_sdio_card_multi_bytes_write_async(pCard, 1, ui32MemAddr, (uint8_t *)ui8WrBuf, no_of_blocks, ui32Len, true);
        am_util_debug_printf("Asynchronous Write Start, Write Status = %d\n", ui32Status);
        if(ui32Status != 0)
        {
            am_util_stdio_printf("Failed to write card.\n");
        }

        //
        // wait until the async write is done
        //
        BaseType_t ret = xSemaphoreTake( g_semSdioWrite, SDIO1_DMA_TIMEOUT );
        TEST_ASSERT_TRUE(ret != pdFALSE);
        if ( ret == pdFALSE )
        {
            am_util_stdio_printf("sdio1 DMA timeout!\n");
        }
    }
    else
    {
        ui32Status = am_hal_sdio_card_multi_bytes_write_sync(pCard, 1, ui32MemAddr, (uint8_t *)ui8WrBuf, no_of_blocks, ui32Len, true);
    }

    if( (ui32Status & 0xFFFF) != AM_HAL_STATUS_SUCCESS )
    {
        am_util_stdio_printf("sdio_rs9116_write FAIL. Status=0x%x\n", ui32Status);
        return false;
    }
    else
    {
        am_util_debug_printf("sdio_rs9116_write address 0x%x\n", ui32Addr);
    }

    return true;
}

static bool
sdio_rs9116_multi_blocks_read(am_hal_card_t *pCard, am_widget_sdio_test_t *pSDHCTestConfig, uint32_t ui32Addr, uint32_t ui32Len, uint8_t *ui8RdBuf)
{
    uint32_t ui32Status;
    uint32_t ui32MemAddr;
    uint32_t ui32BlkSize;
    uint32_t no_of_blocks = 1;
    uint32_t ui32BufAddr = 0;

    am_hal_cachectrl_range_t sRange;

    uint8_t data = ((ui32Addr & 0xff000000) >> 24);
    ui32Status = am_hal_sdio_card_byte_write(pCard, 0, 0xfb, data);
    if(ui32Status != AM_HAL_STATUS_SUCCESS)
    {
        am_util_stdio_printf("Fail to write SDIO reg at addr 0xfb\n", ui32Status);
        return false;
    }

    data = ((ui32Addr & 0x00ff0000) >> 16);
    ui32Status = am_hal_sdio_card_byte_write(pCard, 0, 0xfa, data);
    if(ui32Status != AM_HAL_STATUS_SUCCESS)
    {
        am_util_stdio_printf("Fail to write SDIO reg at addr 0xfa\n", ui32Status);
        return false;
    }

    if( ui32Len > RSI_BLOCK_SIZE )
    {
        // Calculate number of blocks
        no_of_blocks = (ui32Len / RSI_BLOCK_SIZE) ;

        if(ui32Len % RSI_BLOCK_SIZE)
        {
            no_of_blocks = no_of_blocks + 1;
        }
        ui32BlkSize = RSI_BLOCK_SIZE;
    }
    else
    {
        ui32BlkSize = ui32Len;
    }

    ui32MemAddr = (ui32Addr & 0xFFFF) | (1 << 16);

    if ( (pSDHCTestConfig->eXferMode == AM_WIDGET_HOST_XFER_ADMA_ASYNC) || (pSDHCTestConfig->eXferMode == AM_WIDGET_HOST_XFER_SDMA_ASYNC) )
    {
        ui32Status = am_hal_sdio_card_multi_bytes_read_async(pCard, 1, ui32MemAddr, (uint8_t *)(ui8RdBuf), no_of_blocks, ui32BlkSize, true);
        am_util_debug_printf("Asynchronous Read Start, Read Status = %d\n", ui32Status);

        //
        // wait until the async read is done
        //
        BaseType_t ret = xSemaphoreTake( g_semSdioRead, SDIO1_DMA_TIMEOUT );
        TEST_ASSERT_TRUE(ret != pdFALSE);
        if ( ret == pdFALSE )
        {
            am_util_stdio_printf("sdio1 DMA timeout!\n");
        }
    }
    else
    {
        ui32Status = am_hal_sdio_card_multi_bytes_read_sync(pCard, 1, ui32MemAddr, (uint8_t *)(ui8RdBuf), no_of_blocks, ui32BlkSize, true);

        if ( pSDHCTestConfig->eXferMode != AM_WIDGET_HOST_XFER_PIO_SYNC)
        {
            ui32BufAddr = (uint32_t)ui8Rs9116RdBuf;
            if ( ui32BufAddr >= SSRAM_BASEADDR )
            {
                //
                // Clean dcache data before write.
                //
                if ( SDIO_TEST_BUF_LEN > DCACHE_SIZE ) // if ssram buffer > 64K, clean all is efficient
                {
                    am_hal_cachectrl_dcache_invalidate(NULL, true);
                }
                else
                {
                    sRange.ui32StartAddr = ui32BufAddr;
                    sRange.ui32Size = SDIO_TEST_BUF_LEN;
                    am_hal_cachectrl_dcache_invalidate(&sRange, false);
                }
            }
        }
    }

    if( (ui32Status & 0xFFFF) != AM_HAL_STATUS_SUCCESS )
    {
        am_util_stdio_printf("sdio_rs9116_multi_blocks_read FAIL. Status=0x%x\n", ui32Status);
        return false;
    }
    else
    {
        am_util_debug_printf("sdio_rs9116_multi_blocks_read address 0x%x\n", ui32MemAddr);
    }

    return true;
}

static bool sdio_rs9116_1_2_bytes_stress_test(void)
{
    am_hal_host_xfer_mode_e xfer_mode;
    uint32_t mode_index = MODE_START_INDEX;
    uint32_t DUT_index = SDIO_WIFI_INSTANCE;
    uint32_t ui32Len;
    bool  testPassed = true;

    am_widget_sdio_config_t *pHandles = &g_DUTs[DUTs_setting[DUT_index].eHost];
    am_widget_sdio_test_t *pSDHCTestConfig = &DUTs_setting[DUT_index];

    for (uint32_t xfermode = RSI_XFER_MODE_START_INDEX; xfermode < RSI_XFER_MODE_END_INDEX; xfermode++)
    {
      DUTs_setting[DUT_index].eXferMode = sdio_test_xfer_modes[xfermode].mode;
      am_util_debug_printf("\n=============== %s Sdio RS9116 1_2 Bytes Write & Read Test  ================ \n", sdio_test_xfer_modes[xfermode].string);

        for(uint32_t width_index = SDIO_RS9116_WIDTH_START_INDEX; width_index < SDIO_RS9116_WIDTH_END_INDEX; width_index++)
        {
          for(uint32_t speed_index = SDIO_RS9116_SPEED_START_INDEX; speed_index < SDIO_RS9116_SPEED_END_INDEX; speed_index++)
          {
            //
            // RevA silicon limitation:https://ambiqmicro.atlassian.net/browse/CYSV-98
            //
            if ( speed_index >= AM_WIDGET_SDIO_12M )
            {
                am_util_stdio_printf("skipping when sdio clock = %s in sio rs9116 1_2 Bytes\n", sdio_test_speeds[speed_index].string);
                continue;
            }

            DUTs_setting[DUT_index].eClock = (am_widget_sdio_clock_e) speed_index;
            DUTs_setting[DUT_index].eBusWidth = sdio_test_widths[width_index].width;
            DUTs_setting[DUT_index].eUHSMode = sdio_test_modes[mode_index].mode;
            DUTs_setting[DUT_index].sTimingParam = am_widget_findTimingParam((am_widget_timing_scan_hashtable_t *)&rs9116_timing_table[DUT_index], &DUTs_setting[DUT_index]);

            am_util_debug_printf("\n eXferMode = %d \n", DUTs_setting[DUT_index].eXferMode);
            am_util_debug_printf("=============== Starting 1_2 bytes  test for DUT host index: %d ===============\n",DUTs_setting[DUT_index].eHost);
            am_util_stdio_printf("sdio wifi 1_2 bytes test:%s clock = %s, bit width = %s\n", sdio_test_xfer_modes[xfermode].string, sdio_test_speeds[speed_index].string, sdio_test_widths[width_index].string);

            TEST_ASSERT_TRUE(am_widget_sdio_setup(&g_DUTs[DUTs_setting[DUT_index].eHost], &DUTs_setting[DUT_index]));

            //
            // Set transfer mode
            //
            if(DUTs_setting[DUT_index].eXferMode  == AM_WIDGET_HOST_XFER_PIO_SYNC)
            {
                xfer_mode = AM_HAL_HOST_XFER_PIO;
            }
            else if (DUTs_setting[DUT_index].eXferMode  == AM_WIDGET_HOST_XFER_ADMA_SYNC || DUTs_setting[DUT_index].eXferMode  == AM_WIDGET_HOST_XFER_ADMA_ASYNC)
            {
                xfer_mode = AM_HAL_HOST_XFER_ADMA;
            }
            else
            {
                xfer_mode = AM_HAL_HOST_XFER_SDMA;
            }

            am_hal_card_host_set_xfer_mode(pHandles->pSdhcHost, xfer_mode);

            if(pSDHCTestConfig->eHost == AM_HAL_SDHC_CARD_HOST)
            {
                am_hal_card_register_evt_callback(pHandles->pDevHandle, host0_card_event_test_cb);
            }
            else if(pSDHCTestConfig->eHost == AM_HAL_SDHC_CARD_HOST1)
            {
                am_hal_card_register_evt_callback(pHandles->pDevHandle, host1_card_event_test_cb);
            }

            for (uint32_t test_len = 1; test_len < 3; test_len++ )
            {
                uint32_t ui32BlockCount = 1;
                ui32Len = RSI_BLOCK_SIZE*ui32BlockCount;

                for (int i = 0; i < 256; i++)
                {
                    ui8Rs9116WrBuf[i] = 0xFF-i;
                    ui8Rs9116RdBuf[i] = 0x0;
                    if(test_len ==2)
                    {
                        ui8Rs9116WrBuf[i] = i & 0xFF;
                    }
                }

                //
                // write a block with 1 or 2 byte count
                //
                ui32Len = test_len;
                for (int addr = 0; addr < RSI_BLOCK_SIZE; addr+=test_len)
                {
                  TEST_ASSERT_TRUE(am_widget_sdio_rsi_write(pHandles->pDevHandle, pSDHCTestConfig, RSI_PING_BUFFER_ADDR + addr, ui32Len, (uint8_t *)(&ui8Rs9116WrBuf[addr])));
                }

                ui32Len = RSI_BLOCK_SIZE;
                for (int i = 0; i < ui32Len; i ++)
                {
                    ui8Rs9116RdBuf[i] = 0;
                }

                TEST_ASSERT_TRUE(am_widget_sdio_rsi_read(pHandles->pDevHandle, pSDHCTestConfig, RSI_PING_BUFFER_ADDR, ui32Len, (uint8_t *)ui8Rs9116RdBuf));

                //
                // check if block data match or not after block read
                //
                testPassed = am_widget_check_data_match(ui8Rs9116RdBuf, ui8Rs9116WrBuf, ui32Len);
                if (!testPassed)
                {
                    am_util_stdio_printf("Sdio rs9116 1_2 bytes test at %s, speed=%s, width=%s, len=%d, failed on SDIO #%d\n", sdio_test_xfer_modes[xfermode].string, sdio_test_speeds[speed_index].string, sdio_test_widths[width_index].string, ui32Len, DUT_index);
                }

                TEST_ASSERT_TRUE(testPassed);

                for (uint32_t uiReadLen = 1; uiReadLen < 3; uiReadLen++)
                {
                    for (int i = 0; i < uiReadLen; i ++)
                    {
                        ui8Rs9116RdBuf[i] = 0;
                    }

                    for (int addr = 0; addr < RSI_BLOCK_SIZE; addr+=uiReadLen)
                    {
                        TEST_ASSERT_TRUE(am_widget_sdio_rsi_read(pHandles->pDevHandle, pSDHCTestConfig, RSI_PING_BUFFER_ADDR + addr, uiReadLen, (uint8_t *)(&ui8Rs9116RdBuf[addr])));
                    }

                    //
                    // check if block data match or not
                    //
                    testPassed = am_widget_check_data_match(ui8Rs9116RdBuf, ui8Rs9116WrBuf, uiReadLen);
                    if (testPassed)
                    {
                        am_util_debug_printf("sdio rs9116 1_2 bytes test at %s, speed=%s, width=%s, rd_len=%d, wr_len=%d, passed on SDIO #%d\n",sdio_test_xfer_modes[xfermode].string, sdio_test_speeds[speed_index].string, sdio_test_widths[width_index].string, uiReadLen, test_len, DUT_index);
                    }
                    TEST_ASSERT_TRUE(testPassed);
                }
            }
          }
        }
    }
    return testPassed;
}

static bool sdio_rs9116_multiple_bytes_stress_test(void)
{
    am_hal_host_xfer_mode_e xfer_mode;
    uint32_t mode_index = MODE_START_INDEX;
    uint32_t DUT_index = SDIO_WIFI_INSTANCE;
    uint32_t ui32Len;
    uint32_t ui32Count = 0;
    bool  testPassed = true;
    uint32_t ui32BufAddr = 0;

    am_hal_cachectrl_range_t sRange;

    am_widget_sdio_config_t *pHandles = &g_DUTs[DUTs_setting[DUT_index].eHost];
    am_widget_sdio_test_t *pSDHCTestConfig = &DUTs_setting[DUT_index];

    for (uint32_t xfermode = RSI_XFER_MODE_START_INDEX; xfermode < RSI_XFER_MODE_END_INDEX; xfermode++)
    {
      DUTs_setting[DUT_index].eXferMode = sdio_test_xfer_modes[xfermode].mode;
      am_util_debug_printf("\n=============== %s Sdio Multiple bytes Write & Read Test  ================ \n", sdio_test_xfer_modes[xfermode].string);

        for(uint32_t width_index = SDIO_RS9116_WIDTH_START_INDEX; width_index < SDIO_RS9116_WIDTH_END_INDEX; width_index++)
        {
          for(uint32_t speed_index = SDIO_RS9116_SPEED_START_INDEX; speed_index < SDIO_RS9116_SPEED_END_INDEX; speed_index++)
          {
            DUTs_setting[DUT_index].eClock = (am_widget_sdio_clock_e) speed_index;
            DUTs_setting[DUT_index].eBusWidth = sdio_test_widths[width_index].width;
            DUTs_setting[DUT_index].eUHSMode = sdio_test_modes[mode_index].mode;
            DUTs_setting[DUT_index].sTimingParam = am_widget_findTimingParam((am_widget_timing_scan_hashtable_t *)&rs9116_timing_table[DUT_index], &DUTs_setting[DUT_index]);
            am_util_stdio_printf("sdio wifi multiple bytes test:%s clock = %s, bit width = %s\n", sdio_test_xfer_modes[xfermode].string, sdio_test_speeds[speed_index].string, sdio_test_widths[width_index].string);

            TEST_ASSERT_TRUE(am_widget_sdio_setup(&g_DUTs[DUTs_setting[DUT_index].eHost], &DUTs_setting[DUT_index]));

            //
            // Set transfer mode
            //
            if(DUTs_setting[DUT_index].eXferMode  == AM_WIDGET_HOST_XFER_PIO_SYNC)
            {
                xfer_mode = AM_HAL_HOST_XFER_PIO;
            }
            else if (DUTs_setting[DUT_index].eXferMode  == AM_WIDGET_HOST_XFER_ADMA_SYNC || DUTs_setting[DUT_index].eXferMode  == AM_WIDGET_HOST_XFER_ADMA_ASYNC)
            {
                xfer_mode = AM_HAL_HOST_XFER_ADMA;
            }
            else
            {
                xfer_mode = AM_HAL_HOST_XFER_SDMA;
            }

            am_hal_card_host_set_xfer_mode(pHandles->pSdhcHost, xfer_mode);

            if(pSDHCTestConfig->eHost == AM_HAL_SDHC_CARD_HOST)
            {
                am_hal_card_register_evt_callback(pHandles->pDevHandle, host0_card_event_test_cb);
            }
            else if(pSDHCTestConfig->eHost == AM_HAL_SDHC_CARD_HOST1)
            {
                am_hal_card_register_evt_callback(pHandles->pDevHandle, host1_card_event_test_cb);
            }

            xSemaphoreTake(g_semEMMCStart, SDIO1_DMA_TIMEOUT);

            for (ui32Len = SDIO_BYTES_TEST_START_NUM; ui32Len < RSI_BLOCK_SIZE; ui32Len += SDIO_TEST_INCREACE_LENGTH )
            {
                TEST_ASSERT_TRUE(am_widget_prepare_data_pattern( (ui32Count % SDIO_TEST_PATTERN_NUMBER), ui8Rs9116WrBuf, ui32Len));

                TEST_ASSERT_TRUE(am_widget_sdio_rsi_write(pHandles->pDevHandle, pSDHCTestConfig, RSI_PING_BUFFER_ADDR, ui32Len, (uint8_t *)ui8Rs9116WrBuf));

                memset((void*)ui8Rs9116RdBuf, 0, ui32Len);

                if ( pSDHCTestConfig->eXferMode != AM_WIDGET_HOST_XFER_PIO_SYNC )
                {
                    ui32BufAddr = (uint32_t)ui8Rs9116RdBuf;
                    if ( ui32BufAddr >= SSRAM_BASEADDR )
                    {
                        //
                        // Clean dcache data before write.
                        //
                        if ( ui32Len > DCACHE_SIZE ) // if ssram buffer > 64K, clean all is efficient
                        {
                            am_hal_cachectrl_dcache_clean(NULL);
                        }
                        else
                        {
                            sRange.ui32StartAddr = ui32BufAddr;
                            sRange.ui32Size = ui32Len;
                            am_hal_cachectrl_dcache_clean(&sRange);
                        }
                    }
                }

                TEST_ASSERT_TRUE(am_widget_sdio_rsi_read(pHandles->pDevHandle, pSDHCTestConfig, RSI_PING_BUFFER_ADDR, ui32Len, (uint8_t *)ui8Rs9116RdBuf));

                //
                // check if block data match or not
                //
                testPassed = am_widget_check_data_match(ui8Rs9116RdBuf, ui8Rs9116WrBuf, ui32Len);
                if (testPassed)
                {
                    am_util_debug_printf("sdio wifi multiple stress test :%s clock = %s, bit width = %s\n", sdio_test_xfer_modes[xfermode].string, sdio_test_speeds[speed_index].string, sdio_test_widths[width_index].string);
                }

                ui32Count ++;

                TEST_ASSERT_TRUE(testPassed);
            }
            xSemaphoreGive(g_semSdioEnd);
          }
        }
    }
    return testPassed;
}

static bool
sdio_rs9116_multiple_blocks_stress_test(void)
{
    uint32_t randomDelay;
    uint32_t mode_index = 0;
    uint32_t DUT_index = SDIO_WIFI_INSTANCE;
    uint32_t ui32Len;
    uint32_t ui32BufAddr = 0;

    am_hal_cachectrl_range_t sRange;
    am_hal_host_xfer_mode_e xfer_mode;

    am_widget_sdio_config_t *pHandles = &g_DUTs[DUTs_setting[DUT_index].eHost];
    am_widget_sdio_test_t *pSDHCTestConfig = &DUTs_setting[DUT_index];

    for (uint32_t xfermode = RSI_XFER_MODE_START_INDEX; xfermode < RSI_XFER_MODE_END_INDEX; xfermode++)
    {
        DUTs_setting[DUT_index].eXferMode = sdio_test_xfer_modes[xfermode].mode;
        am_util_debug_printf("\n=============== %s Sdio Multiple blocks Write & Read Test  ================ \n", sdio_test_xfer_modes[xfermode].string);

        for(uint32_t width_index = SDIO_RS9116_WIDTH_START_INDEX; width_index < SDIO_RS9116_WIDTH_END_INDEX; width_index++)
        {
            for(uint32_t speed_index = SDIO_RS9116_SPEED_START_INDEX; speed_index < SDIO_RS9116_SPEED_END_INDEX; speed_index++)
            {
                DUTs_setting[DUT_index].eClock = (am_widget_sdio_clock_e) speed_index;
                DUTs_setting[DUT_index].eBusWidth = sdio_test_widths[width_index].width;
                DUTs_setting[DUT_index].eUHSMode = sdio_test_modes[mode_index].mode;
                DUTs_setting[DUT_index].sTimingParam = am_widget_findTimingParam((am_widget_timing_scan_hashtable_t *)&rs9116_timing_table[DUT_index], &DUTs_setting[DUT_index]);

                am_util_debug_printf("=============== Starting Multiple Blocks test for DUT host index: %d ===============\n",DUTs_setting[DUT_index].eHost);
                am_util_stdio_printf("sdio wifi :%s clock = %s, bit width = %s\n", sdio_test_xfer_modes[xfermode].string, sdio_test_speeds[speed_index].string, sdio_test_widths[width_index].string);

                TEST_ASSERT_TRUE(am_widget_sdio_setup(&g_DUTs[DUTs_setting[DUT_index].eHost], &DUTs_setting[DUT_index]));

                //
                // Set transfer mode
                //
                if(DUTs_setting[DUT_index].eXferMode  == AM_WIDGET_HOST_XFER_PIO_SYNC)
                {
                    xfer_mode = AM_HAL_HOST_XFER_PIO;
                }
                else if (DUTs_setting[DUT_index].eXferMode  == AM_WIDGET_HOST_XFER_ADMA_SYNC || DUTs_setting[DUT_index].eXferMode  == AM_WIDGET_HOST_XFER_ADMA_ASYNC)
                {
                    xfer_mode = AM_HAL_HOST_XFER_ADMA;
                }
                else
                {
                    xfer_mode = AM_HAL_HOST_XFER_SDMA;
                }

                am_hal_card_host_set_xfer_mode(pHandles->pSdhcHost, xfer_mode);

                if(pSDHCTestConfig->eHost == AM_HAL_SDHC_CARD_HOST)
                {
                    am_hal_card_register_evt_callback(pHandles->pDevHandle, host0_card_event_test_cb);
                }
                else if(pSDHCTestConfig->eHost == AM_HAL_SDHC_CARD_HOST1)
                {
                    am_hal_card_register_evt_callback(pHandles->pDevHandle, host1_card_event_test_cb);
                }

                for (uint32_t ui32Cnt = 0; ui32Cnt < 5; ui32Cnt ++ )
                {
                    ui32Len = SDIO_TEST_BUF_LEN;

                    am_util_debug_printf("sdio multiple blocks test len:%d\n" , ui32Len);

                    TEST_ASSERT_TRUE(am_widget_prepare_data_pattern( (ui32Cnt % 5), ui8Rs9116WrBuf, ui32Len));

                    xSemaphoreTake(g_semEMMCStart, SDIO1_DMA_TIMEOUT);

                    TEST_ASSERT_TRUE(sdio_rs9116_multi_blocks_write(pHandles->pDevHandle, pSDHCTestConfig, SDIO_TEST_ADDR, ui32Len, (uint8_t *)ui8Rs9116WrBuf));

                    memset((void*)ui8Rs9116RdBuf, 0, SDIO_TEST_BUF_LEN);

                    if ( pSDHCTestConfig->eXferMode != AM_WIDGET_HOST_XFER_PIO_SYNC)
                    {
                        ui32BufAddr = (uint32_t)ui8Rs9116RdBuf;
                        if ( ui32BufAddr >= SSRAM_BASEADDR )
                        {
                            //
                            // Clean dcache data before write.
                            //
                            if ( SDIO_TEST_BUF_LEN > DCACHE_SIZE ) // if ssram buffer > 64K, clean all is efficient
                            {
                                am_hal_cachectrl_dcache_clean(NULL);
                            }
                            else
                            {
                                sRange.ui32StartAddr = ui32BufAddr;
                                sRange.ui32Size = SDIO_TEST_BUF_LEN;
                                am_hal_cachectrl_dcache_clean(&sRange);
                            }
                        }
                    }

                    //
                    // Task Delay Set.
                    //
                    srand(xTaskGetTickCount());
                    randomDelay = rand() % MAX_SDIO1_TASK_DELAY;
                    vTaskDelay(randomDelay);

                    TEST_ASSERT_TRUE(sdio_rs9116_multi_blocks_read(pHandles->pDevHandle, pSDHCTestConfig, SDIO_TEST_ADDR, ui32Len, (uint8_t *)ui8Rs9116RdBuf));

                    //
                    // check if block data match or not
                    //
                    TEST_ASSERT_TRUE(am_widget_check_data_match(ui8Rs9116RdBuf, ui8Rs9116WrBuf, ui32Len));
                }
                xSemaphoreGive(g_semSdioEnd);
            }
        }
    }
    return true;
}

//*****************************************************************************
//
// Sdio RS116 WiFi interface task.
//
//*****************************************************************************
void Sdio_RS116_WiFi_interface_Task(void *pvParameters)
{
    am_util_stdio_printf("\nSdio RS116 WiFi interface task start\n");

    g_semSdioEnd = xSemaphoreCreateBinary();
    TEST_ASSERT_TRUE(g_semSdioEnd != NULL);

    TEST_ASSERT_TRUE(sdio_rs9116_multiple_blocks_stress_test());
    TEST_ASSERT_TRUE(sdio_rs9116_1_2_bytes_stress_test());
    TEST_ASSERT_TRUE(sdio_rs9116_multiple_bytes_stress_test());

    vSemaphoreDelete(g_semSdioWrite);
    vSemaphoreDelete(g_semSdioRead);
    vSemaphoreDelete(g_semSdioEnd);

    TEST_ASSERT_TRUE(am_widget_sdio_cleanup(&g_DUTs[DUTs_setting[SDIO_WIFI_INSTANCE].eHost]));
    am_util_stdio_printf("\nSdio RS116 WiFi interface task end!\n");

    vTaskDelete(NULL);
    while (1);
}

