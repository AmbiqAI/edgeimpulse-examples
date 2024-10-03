//*****************************************************************************
//
//! @file wifi_source_task.c
//!
//! @brief Task for synthetic data flow through the RS9116 WiFi module
//!
//*****************************************************************************

//*****************************************************************************
//
// ${copyright}
//
// This is part of revision ${version} of the AmbiqSuite Development Package.
//
//*****************************************************************************

#include <string.h>
#include <stdio.h>

#include "complex_test_cases.h"
#include "wifi_source_task.h"

#include "am_mcu_apollo.h"
#include "am_util.h"
#include "am_bsp.h"

#include "funky_trunc.h"
#include "file_writer_task.h"

#include "FreeRTOS.h"

extern volatile bool g_bPlaybackTaskComplete;

#define DCACHE_SIZE     64*1024

//*****************************************************************************
//
// Wifi Source task handle.
//
//*****************************************************************************
TaskHandle_t wifi_source_task_handle;

//*****************************************************************************
//
// Macro definitions
//
//*****************************************************************************

#define WIFI_SOURCE_TASK_HOST_BUS_WIDTH         AM_HAL_HOST_BUS_WIDTH_4
#define WIFI_SOURCE_TASK_HOST_UHS_MODE          AM_HAL_HOST_UHS_SDR50
#define WIFI_SOURCE_TASK_HOST_BUS_VOLTAGE       AM_HAL_HOST_BUS_VOLTAGE_1_8
#define WIFI_SOURCE_TASK_HOST_XFER_MODE         AM_HAL_HOST_XFER_ADMA
#define WIFI_SOURCE_TASK_HOST_ASYNC
// 48M not working for me yet on Apollo5 EB
//#define WIFI_SOURCE_TASK_HOST_CLOCK             48000000
#define WIFI_SOURCE_TASK_HOST_CLOCK             24000000
#define WIFI_SOURCE_TASK_CARD_PWR_POLICY        AM_HAL_CARD_PWR_CTRL_SDHC_OFF
#define WIFI_SOURCE_TASK_CARD_PWR_CTRL_FUNC     NULL
#define WIFI_SOURCE_TASK_CARD_FUNC_NUM          1   // function for the buffer we're using

#if SDIO_WIFI_INTERFACE == 0
#define WIFI_SOURCE_TASK_HOST_INDEX             AM_HAL_SDHC_CARD_HOST
#else
#define WIFI_SOURCE_TASK_HOST_INDEX             AM_HAL_SDHC_CARD_HOST1
#endif

//SDIO DMA timeout
#define SDIO1_DMA_TIMEOUT      (0x100UL)

// Addresses in the SDIO card MMIO space we can (ab)use
#define RSI_PING_BUFFER_ADDR 0x18000
#define RSI_PONG_BUFFER_ADDR 0x19000

#define RSI_BLOCK_SIZE           256
#define SDIO_TEST_BUF_LEN        RSI_BLOCK_SIZE*128
#if (SDIO_TEST_BUF_LEN * 2 > WIFI_BUFFER_LEN)
#error "WIFI_BUFFER_LEN must be larger than SDIO_TEST_BUF_LEN"
#endif
#define SDIO_TEST_ADDR           RSI_PONG_BUFFER_ADDR - SDIO_TEST_BUF_LEN

//*****************************************************************************
//
// Global variables and templates
//
//*****************************************************************************
volatile bool g_bWifiSourceTaskComplete = false;

am_hal_card_host_t *g_pSdhcHost = NULL;
am_hal_card_t g_sDevice;
bool g_bAsyncReadIsDone = false;
bool g_bAsyncWriteIsDone = false;

//*****************************************************************************
//
// Variables from sdio_rs9116_emmc_multiple_instances_test_cases
//
//*****************************************************************************
SemaphoreHandle_t g_semSdioWrite = NULL;
SemaphoreHandle_t g_semSdioRead = NULL;

//*****************************************************************************
//
// RS9116 buffer in XIPMM PSRAM
//
//*****************************************************************************
uint8_t *ui8Rs9116RdBuf = (uint8_t *)WIFI_BUFFER_ADDRESS;
uint8_t *ui8Rs9116WrBuf = (uint8_t *)(WIFI_BUFFER_ADDRESS + SDIO_TEST_BUF_LEN);

//*****************************************************************************
//
// SDIO ISR
//
//*****************************************************************************
#if SDIO_WIFI_INTERFACE == 0
void am_sdio0_isr(void)
#else
void am_sdio1_isr(void)
#endif
{
    uint32_t ui32IntStatus;

    am_hal_sdhc_intr_status_get(g_pSdhcHost->pHandle, &ui32IntStatus, true);
    am_hal_sdhc_intr_status_clear(g_pSdhcHost->pHandle, ui32IntStatus);
    am_hal_sdhc_interrupt_service(g_pSdhcHost->pHandle, ui32IntStatus);
}

//*****************************************************************************
//
// SDIO interrupt callbacks.
//
//*****************************************************************************

static void wifi_source_host_card_event_test_cb(am_hal_host_evt_t *pEvt)
{
    am_hal_card_host_t *pHost = (am_hal_card_host_t *)pEvt->pCtx;
    am_hal_cachectrl_range_t sRange;
    uint32_t ui32BufLen = 0;
    uint32_t ui32BufAddr = 0;

    if (AM_HAL_EVT_XFER_COMPLETE == pEvt->eType &&
        pHost->AsyncCmdData.dir == AM_HAL_DATA_DIR_READ)
    {
        xSemaphoreGiveFromISR(g_semSdioRead, NULL);
        g_bAsyncReadIsDone = true;
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
        g_bAsyncWriteIsDone = true;
        am_util_debug_printf("Last Write Xfered block %d\n", pEvt->ui32BlkCnt);
    }

    if (AM_HAL_EVT_SDMA_DONE == pEvt->eType &&
        pHost->AsyncCmdData.dir == AM_HAL_DATA_DIR_READ)
    {
        am_util_debug_printf("SDMA Read Xfered block %d\n", pEvt->ui32BlkCnt);
    }

    if (AM_HAL_EVT_SDMA_DONE == pEvt->eType &&
        pHost->AsyncCmdData.dir == AM_HAL_DATA_DIR_WRITE)
    {
        am_util_debug_printf("SDMA Write Xfered block %d\n", pEvt->ui32BlkCnt);
    }


    if (AM_HAL_EVT_DAT_ERR == pEvt->eType)
    {
        am_util_stdio_printf("Data error type %d\n", pHost->AsyncCmdData.eDataError);
    }
}

static bool
sdio_rs9116_multi_blocks_write(am_hal_card_t *pCard, uint32_t ui32Addr, uint32_t ui32Len, uint8_t *ui8WrBuf)
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
    ui32BufAddr = (uint32_t)ui8WrBuf;
    if ( ui32BufAddr >= SSRAM_BASEADDR )
    {
        //
        // Clean dcache data before write.
        //
        if ( SDIO_TEST_BUF_LEN >= DCACHE_SIZE ) // if ssram buffer > 64K, clean all is efficient
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


#ifdef WIFI_SOURCE_TASK_HOST_ASYNC
    am_util_debug_printf("Async write setup blocks %d len %d\n", no_of_blocks, ui32Len);
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
#else
    ui32Status = am_hal_sdio_card_multi_bytes_write_sync(pCard, 1, ui32MemAddr, (uint8_t *)ui8WrBuf, no_of_blocks, ui32Len, true);
#endif
    if( (ui32Status & 0xFFFF) != AM_HAL_STATUS_SUCCESS )
    {
        am_util_stdio_printf("sdio_rs9116_write FAIL. Status=0x%x\n", ui32Status);
        return false;
    }
    else
    {
        am_util_debug_printf("sdio_rs9116_write address 0x%x\n", ui32MemAddr);
    }

    return true;
}

static bool
sdio_rs9116_multi_blocks_read(am_hal_card_t *pCard, uint32_t ui32Addr, uint32_t ui32Len, uint8_t *ui8RdBuf)
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

#ifdef WIFI_SOURCE_TASK_HOST_ASYNC
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
		else
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
#else
    ui32Status = am_hal_sdio_card_multi_bytes_read_sync(pCard, 1, ui32MemAddr, (uint8_t *)(ui8RdBuf), no_of_blocks, ui32BlkSize, true);
#endif

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

bool wifi_sdio_setup(void) {
    //
    // Get the SDHC card host instance
    //
    g_pSdhcHost = am_hal_get_card_host(WIFI_SOURCE_TASK_HOST_INDEX, true);

    if (g_pSdhcHost == NULL)
    {
        am_util_stdio_printf("No such card host !!!\n");
        return false;
    }

    //
    // enable sdio pins
    //
    am_bsp_sdio_pins_enable(g_pSdhcHost->ui32Module, AM_HAL_HOST_BUS_WIDTH_8);

    //
    // check if card is present
    //
    uint32_t ui32Retries = 10;
    while (am_hal_card_host_find_card(g_pSdhcHost, &g_sDevice) != AM_HAL_STATUS_SUCCESS)
    {
        am_util_stdio_printf("No card is present now\n");
        am_util_delay_ms(1000);
        am_util_stdio_printf("Checking if card is available again\n");
        ui32Retries--;
        if(ui32Retries == 0)
        {
          return false;
        }
    }

    //
    // reset sdio device
    //
    am_bsp_sdio_reset(g_pSdhcHost->ui32Module);


    // init SDIO device
    ui32Retries = 10;
    while (am_hal_card_init(&g_sDevice, AM_HAL_CARD_TYPE_UNKNOWN,
                            WIFI_SOURCE_TASK_CARD_PWR_CTRL_FUNC,
                            WIFI_SOURCE_TASK_CARD_PWR_POLICY) != AM_HAL_STATUS_SUCCESS)
    {
        am_util_delay_ms(1000);
        am_util_stdio_printf("card and host is not ready, try again\n");
        ui32Retries--;
        if(ui32Retries == 0)
        {
            return false;
        }
    }

    uint32_t ui32Stat;
    // rs9116 specific function init
    ui32Stat = am_hal_sdio_card_func_enable(&g_sDevice, WIFI_SOURCE_TASK_CARD_FUNC_NUM);
    if(ui32Stat != AM_HAL_STATUS_SUCCESS)
    {
        am_util_stdio_printf("Enable function failed. Status=0x%x\n", ui32Stat);
    }

    //
    // Enable SDIO card function interrupt
    //
    ui32Stat = am_hal_sdio_card_func_interrupt_enable(&g_sDevice, WIFI_SOURCE_TASK_CARD_FUNC_NUM);
    if(ui32Stat != AM_HAL_STATUS_SUCCESS)
    {
        am_util_stdio_printf("Enable function interrupt failed. Status=0x%x\n", ui32Stat);
    }

    //
    // Get SDIO card function block size
    //
    uint32_t ui32BlkSize = 0;
    ui32Stat = am_hal_sdio_card_get_block_size(&g_sDevice, WIFI_SOURCE_TASK_CARD_FUNC_NUM, &ui32BlkSize);
    if(ui32Stat != AM_HAL_STATUS_SUCCESS)
    {
        am_util_stdio_printf("Fail to get SDIO card block size. Status=0x%x\n", ui32Stat);
    }

    //
    // Set SDIO card function block size
    //
    ui32BlkSize = RSI_BLOCK_SIZE;
    ui32Stat = am_hal_sdio_card_set_block_size(&g_sDevice, WIFI_SOURCE_TASK_CARD_FUNC_NUM, ui32BlkSize);
    if(ui32Stat != AM_HAL_STATUS_SUCCESS)
    {
        am_util_stdio_printf("Fail to set SDIO card block size. Status=0x%x\n", ui32Stat);
    }

    //
    // Get SDIO card function block size again
    //
    ui32Stat = am_hal_sdio_card_get_block_size(&g_sDevice, WIFI_SOURCE_TASK_CARD_FUNC_NUM, &ui32BlkSize);
    if(ui32Stat != AM_HAL_STATUS_SUCCESS)
    {
        am_util_stdio_printf("Fail to check SDIO card block size. Status=0x%x\n", ui32Stat);
        return false;
    }

    //
    // Set bus mode, width and clock speed for test
    //
    if(AM_HAL_STATUS_SUCCESS != am_hal_card_cfg_set(&g_sDevice, AM_HAL_CARD_TYPE_SDIO,
                                                        WIFI_SOURCE_TASK_HOST_BUS_WIDTH,
                                                        WIFI_SOURCE_TASK_HOST_CLOCK,
                                                        WIFI_SOURCE_TASK_HOST_BUS_VOLTAGE,
                                                        WIFI_SOURCE_TASK_HOST_UHS_MODE))
    {
        am_util_stdio_printf("card config set failed\n");
        return false;
    }

    am_hal_card_host_set_xfer_mode(g_pSdhcHost, WIFI_SOURCE_TASK_HOST_XFER_MODE);

    // We made it! Now hook the callback
    am_hal_card_register_evt_callback(&g_sDevice, wifi_source_host_card_event_test_cb);

    return true;
}

//*****************************************************************************
//
// Perform initial setup for the wifi source task
//
//*****************************************************************************
void
WifiSourceTaskSetup(void)
{
    am_util_stdio_printf("Wifi Source task: setup\r\n");
    g_semSdioWrite = xSemaphoreCreateBinary();
    g_semSdioRead = xSemaphoreCreateBinary();
    TEST_ASSERT_TRUE(wifi_sdio_setup());
}

//*****************************************************************************
//
// WifiSourceTask - Pass data through the RS9116 module
//
//*****************************************************************************
void
WifiSourceTask(void *pvParameters)
{
    uint32_t ui32Len;
    uint32_t ui32Remaining;
    uint32_t ui32MP3Index = 0;
    uint32_t file_index = 0;
    uint32_t ui32InvalidBytes = 0;
    uint32_t ui32InvalidTransfers = 0;
	  am_hal_cachectrl_range_t sRange;

    am_util_stdio_printf("\r\nRunning Wifi Source Task\r\n");

    while(!g_bPlaybackTaskComplete) {

        while(1) {
					  //
            // Clean dcache data before write.
            //
            if ( ui32Len > DCACHE_SIZE ) // if ssram buffer > 64K, clean all is efficient
            {
                am_hal_cachectrl_dcache_clean(NULL);
            }
            else
            {
                sRange.ui32StartAddr = (uint32_t)ui8Rs9116WrBuf;
                sRange.ui32Size = ui32Len;
                am_hal_cachectrl_dcache_clean(&sRange);
            }
            ui32Len = SDIO_TEST_BUF_LEN;
            memmove(ui8Rs9116WrBuf, funky_trunc_mp3, funky_trunc_mp3_len);
            TEST_ASSERT_TRUE_MESSAGE(sdio_rs9116_multi_blocks_write(&g_sDevice, SDIO_TEST_ADDR, ui32Len, (uint8_t *)ui8Rs9116WrBuf), "SDIO write failed");
						
						
        //
        // Clean dcache data if ssram buffer acessed by other master
        //
        if ( ui32Len > DCACHE_SIZE ) // if ssram buffer > 64K, clean all is efficient
        {
            am_hal_cachectrl_dcache_clean(NULL);
        }
        else
        {
            sRange.ui32StartAddr = (uint32_t)ui8Rs9116RdBuf;
            sRange.ui32Size = ui32Len;
            am_hal_cachectrl_dcache_clean(&sRange);
        }
            
            memset((void*)ui8Rs9116RdBuf, 0, SDIO_TEST_BUF_LEN);
            TEST_ASSERT_TRUE_MESSAGE(sdio_rs9116_multi_blocks_read(&g_sDevice, SDIO_TEST_ADDR, ui32Len, (uint8_t *)ui8Rs9116RdBuf), "SDIO readback failed");
				
        //
        // Flush and invalidate cache, if ssram buffer > 64K, clean all is efficient
        //
        if ( ui32Len > DCACHE_SIZE )
        {
            am_hal_cachectrl_dcache_invalidate(NULL, true);
        }
        else
        {
            sRange.ui32StartAddr = (uint32_t)ui8Rs9116RdBuf;
            sRange.ui32Size = ui32Len;
            am_hal_cachectrl_dcache_invalidate(&sRange, false);
        }				

            // compare the buffers
            ui32InvalidBytes = 0;
            for(uint32_t i=0; i<(SDIO_TEST_BUF_LEN / 4); i++) {
                if(*(uint32_t *)(ui8Rs9116RdBuf + i) != *(uint32_t *)(ui8Rs9116WrBuf + i)) {
                    am_util_debug_printf("SDIO expected %08x got %08x\n", *(uint32_t *)(ui8Rs9116WrBuf + i), *(uint32_t *)(ui8Rs9116RdBuf + i));
                    ui32InvalidBytes++;
                }
            }
            if(ui32InvalidBytes == 0) {
                break;
            } else {
                ui32InvalidTransfers++;
                TEST_ASSERT_TRUE_MESSAGE(false, "SDIO readback mismatch!");
            }
        }
        while (!g_bFileWriterTaskComplete)
        {
            xSemaphoreTake(g_xFileWriterBufMutex, portMAX_DELAY);
            if(g_sFileWriterMetadata.bytes <= 0) {
                if(ui32MP3Index == 0) {
                    // Starting at the beginning of a file, so choose a new filename
                    sprintf(g_sFileWriterMetadata.filename, "fnky%04lx.mp3", file_index);
                    g_sFileWriterMetadata.op = FILE_WRITER_CREATE;
                } else {
                    if(g_sFileWriterMetadata.op == FILE_WRITER_CREATE) {
                        g_sFileWriterMetadata.op = FILE_WRITER_APPEND;
                    }
                }
                ui32Remaining = funky_trunc_mp3_len - ui32MP3Index;
                g_sFileWriterMetadata.bytes = ui32Remaining >= DECODED_BUFFER_SIZE ? DECODED_BUFFER_SIZE: ui32Remaining;
                // copy the static file data into the buffer
                memmove(g_sFileWriterMetadata.buffer,
                                    ui8Rs9116RdBuf + ui32MP3Index,
                                    g_sFileWriterMetadata.bytes);
                if(ui32Remaining <= DECODED_BUFFER_SIZE) {
                    // We transferred our last buffer for this file, close it out
                    ui32MP3Index = 0;
                    g_sFileWriterMetadata.op = FILE_WRITER_CLOSE;
                    file_index = (file_index + 1) & 0xFFFF; // next filename, but wrap at 65536 files
                    xSemaphoreGive(g_xFileWriterBufMutex);
                    break;
                }
                else
                {
                    ui32MP3Index += DECODED_BUFFER_SIZE;
                }
            }
            xSemaphoreGive(g_xFileWriterBufMutex);

        }
        am_util_stdio_printf("WifiSourceTask      - transferred %d bytes to FileWriterTask, invalid transfers: %d\n", funky_trunc_mp3_len, ui32InvalidTransfers);
    }
    am_util_stdio_printf("WifiSourceTask      - Suspending Wifi Source Task\r\n");
    // Indicate the Source Task loops are complete
    g_bWifiSourceTaskComplete = true;
    // Suspend and delete the task
    vSemaphoreDelete(g_semSdioWrite);
    vSemaphoreDelete(g_semSdioRead);
    vTaskDelete(NULL);
}
