//*****************************************************************************
//
//! @file am_widget_sdio.c
//!
//! @brief This widget allows test cases to exercise the Apollo4 SDIO module.
//!
//!
//!
//! @endverbatim
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
#include "am_mcu_apollo.h"
#include "am_widget_sdio.h"
#include "sdio_test_common.h"
#include "am_bsp.h"
#include "am_util.h"

//*****************************************************************************
//
// Macro definitions
//
//*****************************************************************************

//*****************************************************************************
//
// Global data
//
//*****************************************************************************
am_widget_emmc_device_info_t emmcs_info[EMMC_NUM] =
{
    {
        .manufacturer = "Toshiba",
        .manufacturerID = 0x11,
        .productName = {'0','A','G','4','0','0'},
        .productRevision = 0x02,
        .OEM = 0,
        .emmc_csize = 0xfff,
        .max_enh_size_mult = 0x1d8,
        .sector_count = 0x760000,
    },
    {
        .manufacturer = "BIWIN",
        .manufacturerID = 0xF4,
        .productName = {0x20,0x6E,0x69,0x77,0x69,0x42},
        .productRevision = 0x11,
        .OEM = 0x22,
        .emmc_csize = 0xfff,
        .max_enh_size_mult = 0xd8,
        .sector_count = 0x6c0000,
    },  
    {
        .manufacturer = "Kingston",
        .manufacturerID = 0x70,
        .productName = {'4','0','7','5','6','M'},
        .productRevision = 0x08,
        .OEM = 0x0,
        .emmc_csize = 0xfff,
        .max_enh_size_mult = 0x1c4,
        .sector_count = 0x710000,
    },
};

am_widget_sdio_config_t g_DUTs[AM_HAL_CARD_HOST_NUM];

am_widget_sdio_test_t   DUTs_setting[DUT_NUM] =
{
  {
      .eHost = AM_HAL_SDHC_CARD_HOST,
      .eXferMode = AM_WIDGET_HOST_XFER_ADMA_SYNC,
      .eClock = AM_WIDGET_SDIO_375K,
      .eBusWidth = AM_HAL_HOST_BUS_WIDTH_1,
      .eBusVoltage = AM_HAL_HOST_BUS_VOLTAGE_1_8,
      .eUHSMode = AM_HAL_HOST_UHS_SDR50,
      .eEraseType = AM_HAL_ERASE,
      .ui32StartBlock = 0,
      .ui32BlockCount = BLK_NUM,
      .ui32SectorCount = 0x100,
      .ui32RpmbCount = 0x800,
      .ui32RpmbBlkCnt = SDIO_EMMC_RPMB_BLK_NUM,
      .ui32IoVectorCnt = IO_VECTOR_NUM,
  },
  {
      .eHost = AM_HAL_SDHC_CARD_HOST1,
      .eXferMode = AM_WIDGET_HOST_XFER_ADMA_SYNC,
      .eClock = AM_WIDGET_SDIO_375K,
      .eBusWidth = AM_HAL_HOST_BUS_WIDTH_1,
      .eBusVoltage = AM_HAL_HOST_BUS_VOLTAGE_1_8,
      .eUHSMode = AM_HAL_HOST_UHS_SDR50,
      .eEraseType = AM_HAL_ERASE,
      .ui32StartBlock = 0,
      .ui32BlockCount = BLK_NUM,
      .ui32SectorCount = 0x100,
      .ui32RpmbCount = 0x800,
      .ui32RpmbBlkCnt = SDIO_EMMC_RPMB_BLK_NUM,
      .ui32IoVectorCnt = IO_VECTOR_NUM,
  },
};

char *dummy_key_hash =
{
    "AAAABBBBCCCCDDDDEEEEFFFFGGGGHHHH"
};

//*****************************************************************************
//
// Memory allocation.
//
//*****************************************************************************

//*****************************************************************************
//
// Interrupt handler for SDIO
//
//*****************************************************************************
void am_sdio0_isr(void)
{
    uint32_t ui32IntStatus;

    am_hal_sdhc_intr_status_get(g_DUTs[0].pSdhcHost->pHandle, &ui32IntStatus, true);
    am_hal_sdhc_intr_status_clear(g_DUTs[0].pSdhcHost->pHandle, ui32IntStatus);
    am_hal_sdhc_interrupt_service(g_DUTs[0].pSdhcHost->pHandle, ui32IntStatus);
}

void am_sdio1_isr(void)
{
    uint32_t ui32IntStatus;

    am_hal_sdhc_intr_status_get(g_DUTs[1].pSdhcHost->pHandle, &ui32IntStatus, true);
    am_hal_sdhc_intr_status_clear(g_DUTs[1].pSdhcHost->pHandle, ui32IntStatus);
    am_hal_sdhc_interrupt_service(g_DUTs[1].pSdhcHost->pHandle, ui32IntStatus);
}
//*****************************************************************************
//
// Widget Setup Function.
//
//*****************************************************************************
bool am_widget_sdio_setup(am_widget_sdio_config_t *pHandles,am_widget_sdio_test_t *pSDHCTestConfig)
{
    uint32_t    i = 10;
    uint8_t ui8TxRxDelays[2];

#if !defined(APOLLO5_FPGA) && defined(EMMC_TIMINGSCAN_ENABLE)
    am_widget_timing_scan_t *timingScan = pSDHCTestConfig->sTimingParam;
    if(timingScan == NULL)
    {
        am_util_stdio_printf("\nSDIO%d skip timing scan parameter for this test setup.\n", pSDHCTestConfig->eHost);
    }
    else
    {
        if(!timingScan->bvalid && timingScan->bcalibrate)
        {
            if(pSDHCTestConfig->eCardType == AM_HAL_CARD_TYPE_EMMC)
            {
                am_util_stdio_printf("\nStart SDIO:%d DDR timing scan. \nPlease wait for several minutes...\n", pSDHCTestConfig->eHost);
                uint32_t ui32Stat = am_hal_card_emmc_calibrate(pSDHCTestConfig->eHost,
                                                            pSDHCTestConfig->eUHSMode,
                                                            sdio_test_speeds[pSDHCTestConfig->eClock].speed,
                                                            pSDHCTestConfig->eBusWidth,
                                                            (uint8_t *)ui8WrBuf, 100, 8, ui8TxRxDelays);
                if(ui32Stat == AM_HAL_STATUS_SUCCESS)
                {
                    am_util_stdio_printf("\nSDIO%d TX delay - %d, RX Delay - %d\n", pSDHCTestConfig->eHost, ui8TxRxDelays[0], ui8TxRxDelays[1]);
                    timingScan->bvalid = true;
                    timingScan->ui32TxDelay = ui8TxRxDelays[0];
                    timingScan->ui32RxDelay = ui8TxRxDelays[1];
                }
                else
                {
                    am_util_stdio_printf("\nSDIO%d DDR timing scan failed\n", pSDHCTestConfig->eHost);
                    return false;
                }
            }
            else if ( pSDHCTestConfig->eCardType == AM_HAL_CARD_TYPE_SDSC ||
                      pSDHCTestConfig->eCardType == AM_HAL_CARD_TYPE_SDHC ||
                      pSDHCTestConfig->eCardType == AM_HAL_CARD_TYPE_SDXC )
            {
                am_util_stdio_printf("\nStart SDIO:%d DDR timing scan. \nPlease wait for several minutes...\n", pSDHCTestConfig->eHost);
                uint32_t ui32Stat = am_hal_sd_card_calibrate(pSDHCTestConfig->eHost,
                                                            pSDHCTestConfig->eUHSMode,
                                                            sdio_test_speeds[pSDHCTestConfig->eClock].speed,
                                                            pSDHCTestConfig->eBusWidth,
                                                            (uint8_t *)ui8RdBuf, 100, 8, ui8TxRxDelays,
                                                            am_widget_sd_card_power_config);
                if(ui32Stat == AM_HAL_STATUS_SUCCESS)
                {
                    am_util_stdio_printf("\nSDIO%d TX delay - %d, RX Delay - %d\n", pSDHCTestConfig->eHost, ui8TxRxDelays[0], ui8TxRxDelays[1]);
                    timingScan->bvalid = true;
                    timingScan->ui32TxDelay = ui8TxRxDelays[0];
                    timingScan->ui32RxDelay = ui8TxRxDelays[1];
                }
                else
                {
                    am_util_stdio_printf("\nSDIO%d DDR timing scan failed\n", pSDHCTestConfig->eHost);
                    return false;
                }
            }
        }
    }
#endif
    //
    // Get the SDHC card host instance
    //
    pHandles->pSdhcHost = am_hal_get_card_host(pSDHCTestConfig->eHost, true);

    if (pHandles->pSdhcHost == NULL)
    {
        am_util_stdio_printf("No such card host !!!\n");
        am_util_stdio_printf("SDIO Test Fail !!!\n");
        return false;
    }

    //
    // enable sdio pins
    //
    am_bsp_sdio_pins_enable(pHandles->pSdhcHost->ui32Module, AM_HAL_HOST_BUS_WIDTH_8);

    //
    // check if card is present
    //
    while (am_hal_card_host_find_card(pHandles->pSdhcHost, pHandles->pDevHandle) != AM_HAL_STATUS_SUCCESS)
    {
        am_util_stdio_printf("No card is present now\n");
        am_util_delay_ms(1000);
        am_util_stdio_printf("Checking if card is available again\n");
        i--;
        if(i == 0)
        {
          return false;
        }
    }

    if (pSDHCTestConfig->eCardType == AM_HAL_CARD_TYPE_EMMC)
    {
        if (!am_widget_emmc_card_init(pHandles, pSDHCTestConfig))
        {
            am_util_stdio_printf("\nemmc:%d card init failed\n", pSDHCTestConfig->eHost);
            return false;
        }
    }
    else if (pSDHCTestConfig->eCardType == AM_HAL_CARD_TYPE_SDIO)
    {
        //
        // reset sdio device
        //
        am_bsp_sdio_reset(pHandles->pSdhcHost->ui32Module);

        if (!am_widget_sdio_card_init(pHandles, pSDHCTestConfig))
        {
            am_util_stdio_printf("\nsdio:%d card init failed\n", pSDHCTestConfig->eHost);
            return false;
        }
    }
    else if ( pSDHCTestConfig->eCardType == AM_HAL_CARD_TYPE_SDSC ||
              pSDHCTestConfig->eCardType == AM_HAL_CARD_TYPE_SDHC ||
              pSDHCTestConfig->eCardType == AM_HAL_CARD_TYPE_SDXC  )
    {
        if (!am_widget_sd_card_init(pHandles, pSDHCTestConfig))
        {
            am_util_stdio_printf("\nsd card:%d card init failed\n", pSDHCTestConfig->eHost);
            return false;
        }
    }
    else
    {
        am_util_stdio_printf("\nno sdio device find\n");
        return false;
    }

    //
    // Set bus mode, width and clock speed for test
    //
    if(AM_HAL_STATUS_SUCCESS != am_hal_card_cfg_set(pHandles->pDevHandle, pSDHCTestConfig->eCardType,
                                                        pSDHCTestConfig->eBusWidth,
                                                        sdio_test_speeds[pSDHCTestConfig->eClock].speed,
                                                        pSDHCTestConfig->eBusVoltage,
                                                        pSDHCTestConfig->eUHSMode))
    {
        am_util_stdio_printf("card:%d config set failed\n", pSDHCTestConfig->eHost);
        return false;
    }

#if !defined(APOLLO5_FPGA) && defined(EMMC_TIMINGSCAN_ENABLE)
    if(timingScan)
    {
        if(timingScan->bvalid)
        {
            am_util_stdio_printf("\nFound SDIO TX delay - %d, RX Delay - %d\n", timingScan->ui32TxDelay, timingScan->ui32RxDelay);
            ui8TxRxDelays[0] = timingScan->ui32TxDelay;
            ui8TxRxDelays[1] = timingScan->ui32RxDelay;
            am_hal_card_host_set_txrx_delay(pHandles->pSdhcHost, ui8TxRxDelays);
        }
    }
#endif
    // Return the result.
    return true;

}

//*****************************************************************************
//
// Widget Cleanup Function.
//
//*****************************************************************************
bool am_widget_sdio_cleanup(am_widget_sdio_config_t *pHandles)
{
  uint32_t      ui32Status;

  ui32Status = am_hal_card_deinit(pHandles->pDevHandle);
  if(ui32Status != AM_HAL_STATUS_SUCCESS)
  {
    am_util_stdio_printf("card deinit fail\n");
    return false;
  }

  // Return the result.
  return true;
}

#ifdef COLLECT_BENCHMARKS
#define BENCHMARK_TEST_LOOP 10

uint32_t g_ui32BlkTestCnt = 0;
float fWriteBandwidthTotal = 0;
float fReadBandwidthTotal = 0;

uint32_t am_widget_sdio_cal_time(uint32_t ui32prev, uint32_t ui32curr)
{
  if(ui32prev > ui32curr)
  {
      return 0xFFFFFFFF - ui32prev + ui32curr;
  }
  else
  {
      return ui32curr - ui32prev;
  }
}

static bool am_widget_benchmark_write(am_widget_sdio_config_t *pHandles,am_widget_sdio_test_t *pSDHCTestConfig, 
                                      uint32_t ui32StartBlock, uint32_t ui32BlockCount, uint8_t *pui8WrBuf)
{
    uint32_t ui32TimerTickBefore = 0;
    uint32_t ui32TimerTickAfter = 0;
    uint32_t num_of_ms;
    uint32_t volume_data_written;
    float fBytesPerSecond = 0;
    float fWriteLoopBandwidth =0;
    uint32_t loop;
    uint32_t ui32Stat = AM_HAL_STATUS_SUCCESS;
    uint32_t ui32BufAddr = 0;
    am_hal_cachectrl_range_t sRange;

    am_widget_emmc_devices_config_t *eMMC_device_config = (am_widget_emmc_devices_config_t *)pSDHCTestConfig->device_config;

    for(loop = 0; loop < BENCHMARK_TEST_LOOP; loop++)
    {
        //
        // make sure dma memory test buffer do cache cleaning
        //
        ui32BufAddr = (uint32_t)pui8WrBuf;
        if ( (ui32BufAddr >= SSRAM_BASEADDR) || ((ui32BufAddr + MEM_TEST_SZ) >= SSRAM_BASEADDR) )
        {
            //
            // Clean dcache data before write.
            //
            if ( ui32BlockCount*512 > DCACHE_SIZE ) // if ssram buffer > 64K, clean all is efficient
            {
                am_hal_cachectrl_dcache_clean(NULL);
            }
            else
            {
                sRange.ui32StartAddr = (uint32_t)pui8WrBuf;
                sRange.ui32Size = ui32BlockCount*512;
                am_hal_cachectrl_dcache_clean(&sRange);
            }
        }

        ui32TimerTickBefore = am_hal_timer_read(TIMER_NUM);

        if(pSDHCTestConfig->eXferMode == AM_WIDGET_HOST_XFER_PIO_SYNC || pSDHCTestConfig->eXferMode == AM_WIDGET_HOST_XFER_ADMA_SYNC)
        {
            ui32Stat = am_hal_card_block_write_sync(pHandles->pDevHandle, ui32StartBlock, ui32BlockCount, pui8WrBuf);

            ui32TimerTickAfter = am_hal_timer_read(TIMER_NUM);

            am_util_debug_printf("Synchronous Writing %d blocks is done, Xfer Status %d\n", ui32Stat >> 16, ui32Stat & 0xffff);
            if((ui32Stat & 0xffff) != 0 )
            {
                am_util_stdio_printf("Failed to write card.\n");
                return ui32Stat;
            }
        }
        else if(pSDHCTestConfig->eXferMode == AM_WIDGET_HOST_XFER_ADMA_ASYNC)
        {
            eMMC_device_config->bAsyncWriteIsDone = false;
            ui32Stat = am_hal_card_block_write_async(pHandles->pDevHandle, ui32StartBlock, ui32BlockCount, pui8WrBuf);
            am_util_debug_printf("Asynchronous ADMA Write Start, Write Status = %d\n", ui32Stat);
            if(ui32Stat != 0)
            {
                am_util_stdio_printf("Failed to write card.\n");
                return ui32Stat;
            }

            //
            // wait until the async write is done
            //
            uint32_t i = 0;
            while (!eMMC_device_config->bAsyncWriteIsDone)
            {
                am_util_delay_ms(1);
                i++;
                if ( i == DELAY_MAX_COUNT )
                {
                    am_util_stdio_printf("Failed to write card, async write timeout.\n");
                    return false;
                }
            }

            ui32TimerTickAfter = am_hal_timer_read(TIMER_NUM);

            if( i < DELAY_MAX_COUNT)
            {
                am_util_debug_printf("asynchronous block write is done\n");
            }
        }

        num_of_ms = am_widget_sdio_cal_time(ui32TimerTickBefore, ui32TimerTickAfter);
        
        if (!(num_of_ms)) // need to round up if less than 1 full MS expired
        {
            num_of_ms++;
            am_util_stdio_printf("\n less than 1MS for async read \n");
        }
        volume_data_written = (ui32BlockCount * 512);
        fWriteLoopBandwidth = (float)volume_data_written / num_of_ms / TIME_DIVIDER;
        fBytesPerSecond += fWriteLoopBandwidth;
    }

    fBytesPerSecond = fBytesPerSecond/loop;
    fWriteBandwidthTotal += fBytesPerSecond;

    am_util_debug_printf("\nLoop:%d, Time spent %dms and volume of data is %d\n",loop,num_of_ms,volume_data_written);
    am_util_debug_printf("\nMegabytes per second for eMMC write is %f\n", fBytesPerSecond);
    am_util_debug_printf("bit width is %d bit\n\n", pSDHCTestConfig->eBusWidth);

    return AM_HAL_STATUS_SUCCESS;
}

static bool am_widget_benchmark_read(am_widget_sdio_config_t *pHandles,am_widget_sdio_test_t *pSDHCTestConfig, 
                                     uint32_t ui32StartBlock, uint32_t ui32BlockCount, uint8_t *pui8RdBuf)
{
    uint32_t ui32TimerTickBefore = 0;
    uint32_t ui32TimerTickAfter = 0;
    uint32_t num_of_ms;
    uint32_t volume_data_read;
    float fBytesPerSecond = 0;
    float fReadLoopBandwidth =0;
    uint32_t loop;
    uint32_t ui32Stat = AM_HAL_STATUS_SUCCESS;
    uint32_t ui32BufAddr = 0;
    am_hal_cachectrl_range_t sRange;

    am_widget_emmc_devices_config_t *eMMC_device_config = (am_widget_emmc_devices_config_t *)pSDHCTestConfig->device_config;

    memset(pui8RdBuf, 0, BUF_LEN);

    ui32BufAddr = (uint32_t)pui8RdBuf;
    if ( (ui32BufAddr >= SSRAM_BASEADDR) || ((ui32BufAddr + MEM_TEST_SZ) >= SSRAM_BASEADDR) )
    {
        //
        // Clean dcache data before write.
        //
        if ( ui32BlockCount*512 > DCACHE_SIZE ) // if ssram buffer > 64K, clean all is efficient
        {
            am_hal_cachectrl_dcache_clean(NULL);
        }
        else
        {
            sRange.ui32StartAddr = (uint32_t)pui8RdBuf;
            sRange.ui32Size = ui32BlockCount*512;
            am_hal_cachectrl_dcache_clean(&sRange);
        }
    }

    for(loop = 0; loop < BENCHMARK_TEST_LOOP; loop++)
    {
        ui32TimerTickBefore = am_hal_timer_read(TIMER_NUM);

        if(pSDHCTestConfig->eXferMode == AM_WIDGET_HOST_XFER_PIO_SYNC || pSDHCTestConfig->eXferMode == AM_WIDGET_HOST_XFER_ADMA_SYNC)
        {
            ui32Stat = am_hal_card_block_read_sync(pHandles->pDevHandle, ui32StartBlock, ui32BlockCount, pui8RdBuf);

            ui32TimerTickAfter = am_hal_timer_read(TIMER_NUM);

            ui32BufAddr = (uint32_t)pui8RdBuf;
            if ( (ui32BufAddr >= SSRAM_BASEADDR) || ((ui32BufAddr + MEM_TEST_SZ) >= SSRAM_BASEADDR) )
            {
                //
                // Clean dcache data before write.
                //
                if ( ui32BlockCount*512 > DCACHE_SIZE ) // if ssram buffer > 64K, clean all is efficient
                {
                    am_hal_cachectrl_dcache_invalidate(NULL, true);
                }
                else
                {
                    sRange.ui32StartAddr = (uint32_t)pui8RdBuf;
                    sRange.ui32Size = ui32BlockCount*512;
                    am_hal_cachectrl_dcache_invalidate(&sRange, false);
                }
            }

            if((ui32Stat & 0xffff) != 0 )
            {
                am_util_stdio_printf("Failed to read card.\n");
                return ui32Stat;
            }
        }
        else if(pSDHCTestConfig->eXferMode == AM_WIDGET_HOST_XFER_ADMA_ASYNC)
        {
            eMMC_device_config->bAsyncReadIsDone = false;
            ui32Stat = am_hal_card_block_read_async(pHandles->pDevHandle, ui32StartBlock, ui32BlockCount, pui8RdBuf);
            if(ui32Stat != 0)
            {
                am_util_stdio_printf("Failed to read card.\n");
                return ui32Stat;
            }

            //
            // wait until the async read is done
            //
            uint32_t i = 0;
            while (!eMMC_device_config->bAsyncReadIsDone)
            {
                am_util_delay_ms(1);
                i++;
                if ( i == DELAY_MAX_COUNT )
                {
                    am_util_stdio_printf("Failed to read card, async read timeout\n");
                    return false;
                }
            }

            ui32TimerTickAfter = am_hal_timer_read(TIMER_NUM);

            if( i < DELAY_MAX_COUNT)
            {
                am_util_debug_printf("asynchronous block read is done\n");
            }
        }

        num_of_ms = am_widget_sdio_cal_time(ui32TimerTickBefore, ui32TimerTickAfter);
        
        if (!(num_of_ms)) // need to round up if less than 1 full MS expired
        {
            num_of_ms++;
            am_util_stdio_printf("\n less than 1MS for async read \n");
        }
        volume_data_read = (ui32BlockCount * 512);
        fReadLoopBandwidth = (float)volume_data_read / num_of_ms / TIME_DIVIDER;
        fBytesPerSecond += fReadLoopBandwidth;
    }

    fBytesPerSecond = fBytesPerSecond/loop;
    fReadBandwidthTotal += fBytesPerSecond;

    am_util_debug_printf("\nLoop:%d, Time spent %dms and volume of data is %d\n",loop,num_of_ms,volume_data_read);
    am_util_debug_printf("\nMegabytes per second for eMMC read is %f\n", fBytesPerSecond);
    am_util_debug_printf("bit width is %d bit\n\n", pSDHCTestConfig->eBusWidth);

    return AM_HAL_STATUS_SUCCESS;
}

bool am_widget_sdio_benchmark_test_write_read(am_widget_sdio_config_t *pHandles,am_widget_sdio_test_t *pSDHCTestConfig, uint8_t *pui8WrBuf, uint8_t *pui8RdBuf)
{
    am_hal_host_xfer_mode_e xfer_mode;
    uint32_t ui32Stat = AM_HAL_STATUS_SUCCESS;
    uint32_t ui32BlkCntLoop;
    float fWriteBandwidth= 0;
    float fReadBandwidth = 0;

    //
    // Set transfer mode
    //
    if(pSDHCTestConfig->eXferMode == AM_WIDGET_HOST_XFER_PIO_SYNC)
    {
        xfer_mode = AM_HAL_HOST_XFER_PIO;
    }
    else
    {
        xfer_mode = AM_HAL_HOST_XFER_ADMA;
    }
    am_hal_card_host_set_xfer_mode(pHandles->pSdhcHost, xfer_mode);

    //
    // Register callback regardless
    //
    if(pSDHCTestConfig->eHost == AM_HAL_SDHC_CARD_HOST)
    {
        am_hal_card_register_evt_callback(pHandles->pDevHandle, am_widget_host0_event_cb);
    }
    else if(pSDHCTestConfig->eHost == AM_HAL_SDHC_CARD_HOST1)
    {
        am_hal_card_register_evt_callback(pHandles->pDevHandle, am_widget_host1_event_cb);
    }

    am_util_stdio_printf("\n   bechmark test result:  %10s  |  %8s  |  %8s  |  %8s  |\n", "xfer mode", "width", "Wr MB/s", "Rd MB/s");

    for(uint32_t ui32BlockCount = 1; ui32BlockCount <= pSDHCTestConfig->ui32BlockCount; ui32BlockCount += pSDHCTestConfig->ui32BlockCount / 4 + 1)
    {
        ui32BlkCntLoop = 0;
        fWriteBandwidthTotal = 0;
        fReadBandwidthTotal = 0;

        for(uint32_t ui32StartBlock = pSDHCTestConfig->ui32StartBlock; ui32StartBlock < pSDHCTestConfig->ui32SectorCount; ui32StartBlock += pSDHCTestConfig->ui32SectorCount/2 - ui32BlockCount + 1)
        {
            am_util_debug_printf("\nstart_blk = %d, blk_cnt = %d\n",ui32StartBlock, ui32BlockCount);
            ui32BlkCntLoop ++;

            //
            // erase eMMC before write
            //
            ui32Stat = am_hal_card_block_erase(pHandles->pDevHandle, ui32StartBlock, ui32BlockCount, AM_HAL_ERASE, 1000);
            if(ui32Stat != 0x0)
            {
                am_util_stdio_printf("erase eMMC failed ui32Stat=%d\n", ui32Stat);
                return false;
            }

            ui32Stat = am_widget_benchmark_write(pHandles, pSDHCTestConfig, pSDHCTestConfig->ui32StartBlock, ui32BlockCount, pui8WrBuf);
            if(ui32Stat != AM_HAL_STATUS_SUCCESS)
            {
                am_util_stdio_printf("Failed to benchmark write ui32Stat=%d\n", ui32Stat);
                return ui32Stat;
            }

            ui32Stat = am_widget_benchmark_read(pHandles, pSDHCTestConfig, pSDHCTestConfig->ui32StartBlock, ui32BlockCount, pui8RdBuf);
            if(ui32Stat != AM_HAL_STATUS_SUCCESS)
            {
                am_util_stdio_printf("Failed to benchmark read ui32Stat=%d\n", ui32Stat);
                return ui32Stat;
            }

            if(!am_widget_check_data_match(pui8RdBuf, pui8WrBuf, 512*ui32BlockCount))
            {
                return false;
            }
        }

        fWriteBandwidth = fWriteBandwidthTotal/ui32BlkCntLoop;
        fReadBandwidth = fReadBandwidthTotal/ui32BlkCntLoop;

        am_util_stdio_printf("             %4dblocks:  %10s  |  %8s  |  %8.5f  |  %8.5f  |\n", ui32BlockCount, 
                                                    sdio_benchmark_xfer_modes[pSDHCTestConfig->eXferMode].string, 
                                                           sdio_test_widths[pSDHCTestConfig->eBusWidth/4].string,
                                                  fWriteBandwidth, fReadBandwidth);
    }

    return true;
}

static bool am_widget_sd_card_benchmark_write(am_widget_sdio_config_t *pHandles,am_widget_sdio_test_t *pSDHCTestConfig, 
                                      uint32_t ui32StartBlock, uint32_t ui32BlockCount, uint8_t *pui8WrBuf)
{
    uint32_t ui32TimerTickBefore = 0;
    uint32_t ui32TimerTickAfter = 0;
    uint32_t num_of_ms;
    uint32_t volume_data_written;
    float fBytesPerSecond = 0;
    float fWriteLoopBandwidth =0;
    uint32_t loop;
    uint32_t ui32Stat = AM_HAL_STATUS_SUCCESS;
    uint32_t ui32BufAddr = 0;
    am_hal_cachectrl_range_t sRange;

    am_widget_emmc_devices_config_t *sd_card_device_config = (am_widget_emmc_devices_config_t *)pSDHCTestConfig->device_config;

    for(loop = 0; loop < BENCHMARK_TEST_LOOP; loop++)
    {
        //
        // make sure dma memory test buffer do cache cleaning
        //
        ui32BufAddr = (uint32_t)pui8WrBuf;
        if ( (ui32BufAddr >= SSRAM_BASEADDR) || ((ui32BufAddr + MEM_TEST_SZ) >= SSRAM_BASEADDR) )
        {
            //
            // Clean dcache data before write.
            //
            if ( ui32BlockCount*512 > DCACHE_SIZE ) // if ssram buffer > 64K, clean all is efficient
            {
                am_hal_cachectrl_dcache_clean(NULL);
            }
            else
            {
                sRange.ui32StartAddr = (uint32_t)pui8WrBuf;
                sRange.ui32Size = ui32BlockCount*512;
                am_hal_cachectrl_dcache_clean(&sRange);
            }
        }

        ui32TimerTickBefore = am_hal_timer_read(TIMER_NUM);

        if(pSDHCTestConfig->eXferMode == AM_WIDGET_HOST_XFER_PIO_SYNC || pSDHCTestConfig->eXferMode == AM_WIDGET_HOST_XFER_ADMA_SYNC)
        {
            ui32Stat = am_hal_sd_card_block_write_sync(pHandles->pDevHandle, ui32StartBlock, ui32BlockCount, pui8WrBuf);

            ui32TimerTickAfter = am_hal_timer_read(TIMER_NUM);

            am_util_debug_printf("Synchronous Writing %d blocks is done, Xfer Status %d\n", ui32Stat >> 16, ui32Stat & 0xffff);
            if((ui32Stat & 0xffff) != 0 )
            {
                am_util_stdio_printf("Failed to write sd card.\n");
                return ui32Stat;
            }
        }
        else if(pSDHCTestConfig->eXferMode == AM_WIDGET_HOST_XFER_ADMA_ASYNC)
        {
            sd_card_device_config->bAsyncWriteIsDone = false;
            ui32Stat = am_hal_sd_card_block_write_async(pHandles->pDevHandle, ui32StartBlock, ui32BlockCount, pui8WrBuf);
            am_util_debug_printf("Asynchronous ADMA Write Start, Write Status = %d\n", ui32Stat);
            if(ui32Stat != 0)
            {
                am_util_stdio_printf("Failed to write sd card.\n");
                return ui32Stat;
            }

            //
            // wait until the async write is done
            //
            uint32_t i = 0;
            while (!sd_card_device_config->bAsyncWriteIsDone)
            {
                am_util_delay_ms(1);
                i++;
                if ( i == DELAY_MAX_COUNT )
                {
                    am_util_stdio_printf("Failed to write card, async write timeout.\n");
                    return false;
                }
            }

            ui32TimerTickAfter = am_hal_timer_read(TIMER_NUM);

            if( i < DELAY_MAX_COUNT)
            {
                am_util_debug_printf("asynchronous block write is done\n");
            }
        }

        num_of_ms = am_widget_sdio_cal_time(ui32TimerTickBefore, ui32TimerTickAfter);

        if (!(num_of_ms)) // need to round up if less than 1 full MS expired
        {
            num_of_ms++;
            am_util_stdio_printf("\n less than 1MS for async read \n");
        }
        volume_data_written = (ui32BlockCount * 512);
        fWriteLoopBandwidth = (float)volume_data_written / num_of_ms / TIME_DIVIDER;
        fBytesPerSecond += fWriteLoopBandwidth;
    }

    fBytesPerSecond = fBytesPerSecond/loop;
    fWriteBandwidthTotal += fBytesPerSecond;

    am_util_debug_printf("\nLoop:%d, Time spent %dms and volume of data is %d\n",loop,num_of_ms,volume_data_written);
    am_util_debug_printf("\nMegabytes per second for eMMC write is %f\n", fBytesPerSecond);
    am_util_debug_printf("sd card bit width is %d bit\n\n", pSDHCTestConfig->eBusWidth);

    return AM_HAL_STATUS_SUCCESS;
}

static bool am_widget_sd_card_benchmark_read(am_widget_sdio_config_t *pHandles,am_widget_sdio_test_t *pSDHCTestConfig, 
                                     uint32_t ui32StartBlock, uint32_t ui32BlockCount, uint8_t *pui8RdBuf)
{
    uint32_t ui32TimerTickBefore = 0;
    uint32_t ui32TimerTickAfter = 0;
    uint32_t num_of_ms;
    uint32_t volume_data_read;
    float fBytesPerSecond = 0;
    float fReadLoopBandwidth =0;
    uint32_t loop;
    uint32_t ui32Stat = AM_HAL_STATUS_SUCCESS;
    uint32_t ui32BufAddr = 0;
    am_hal_cachectrl_range_t sRange;

    am_widget_emmc_devices_config_t *sd_card_device_config = (am_widget_emmc_devices_config_t *)pSDHCTestConfig->device_config;

    memset(pui8RdBuf, 0, BUF_LEN);

    ui32BufAddr = (uint32_t)pui8RdBuf;
    if ( (ui32BufAddr >= SSRAM_BASEADDR) || ((ui32BufAddr + MEM_TEST_SZ) >= SSRAM_BASEADDR) )
    {
        //
        // Clean dcache data before write.
        //
        if ( ui32BlockCount*512 > DCACHE_SIZE ) // if ssram buffer > 64K, clean all is efficient
        {
            am_hal_cachectrl_dcache_clean(NULL);
        }
        else
        {
            sRange.ui32StartAddr = (uint32_t)pui8RdBuf;
            sRange.ui32Size = ui32BlockCount*512;
            am_hal_cachectrl_dcache_clean(&sRange);
        }
    }

    for(loop = 0; loop < BENCHMARK_TEST_LOOP; loop++)
    {
        ui32TimerTickBefore = am_hal_timer_read(TIMER_NUM);

        if(pSDHCTestConfig->eXferMode == AM_WIDGET_HOST_XFER_PIO_SYNC || pSDHCTestConfig->eXferMode == AM_WIDGET_HOST_XFER_ADMA_SYNC)
        {
            ui32Stat = am_hal_sd_card_block_read_sync(pHandles->pDevHandle, ui32StartBlock, ui32BlockCount, pui8RdBuf);

            ui32TimerTickAfter = am_hal_timer_read(TIMER_NUM);

            ui32BufAddr = (uint32_t)pui8RdBuf;
            if ( (ui32BufAddr >= SSRAM_BASEADDR) || ((ui32BufAddr + MEM_TEST_SZ) >= SSRAM_BASEADDR) )
            {
                //
                // Clean dcache data before write.
                //
                if ( ui32BlockCount*512 > DCACHE_SIZE ) // if ssram buffer > 64K, clean all is efficient
                {
                    am_hal_cachectrl_dcache_invalidate(NULL, true);
                }
                else
                {
                    sRange.ui32StartAddr = (uint32_t)pui8RdBuf;
                    sRange.ui32Size = ui32BlockCount*512;
                    am_hal_cachectrl_dcache_invalidate(&sRange, true);
                }
            }

            if((ui32Stat & 0xffff) != 0 )
            {
                am_util_stdio_printf("Failed to read sd card.\n");
                return ui32Stat;
            }
        }
        else if(pSDHCTestConfig->eXferMode == AM_WIDGET_HOST_XFER_ADMA_ASYNC)
        {
            sd_card_device_config->bAsyncReadIsDone = false;
            ui32Stat = am_hal_sd_card_block_read_async(pHandles->pDevHandle, ui32StartBlock, ui32BlockCount, pui8RdBuf);
            if(ui32Stat != 0)
            {
                am_util_stdio_printf("Failed to read sd card.\n");
                return ui32Stat;
            }

            //
            // wait until the async read is done
            //
            uint32_t i = 0;
            while (!sd_card_device_config->bAsyncReadIsDone)
            {
                am_util_delay_ms(1);
                i++;
                if ( i == DELAY_MAX_COUNT )
                {
                    am_util_stdio_printf("Failed to read sd card, async read timeout\n");
                    return false;
                }
            }

            ui32TimerTickAfter = am_hal_timer_read(TIMER_NUM);

            if( i < DELAY_MAX_COUNT)
            {
                am_util_debug_printf("asynchronous block read is done\n");
            }
        }

        num_of_ms = am_widget_sdio_cal_time(ui32TimerTickBefore, ui32TimerTickAfter);

        if (!(num_of_ms)) // need to round up if less than 1 full MS expired
        {
            num_of_ms++;
            am_util_stdio_printf("\n less than 1MS for async read \n");
        }
        volume_data_read = (ui32BlockCount * 512);
        fReadLoopBandwidth = (float)volume_data_read / num_of_ms / TIME_DIVIDER;
        fBytesPerSecond += fReadLoopBandwidth;
    }

    fBytesPerSecond = fBytesPerSecond/loop;
    fReadBandwidthTotal += fBytesPerSecond;

    am_util_debug_printf("\nLoop:%d, Time spent %dms and volume of data is %d\n",loop,num_of_ms,volume_data_read);
    am_util_debug_printf("\nMegabytes per second for eMMC read is %f\n", fBytesPerSecond);
    am_util_debug_printf("sd card bit width is %d bit\n\n", pSDHCTestConfig->eBusWidth);

    return AM_HAL_STATUS_SUCCESS;
}

bool am_widget_sd_card_benchmark_test_write_read(am_widget_sdio_config_t *pHandles,am_widget_sdio_test_t *pSDHCTestConfig, uint8_t *pui8WrBuf, uint8_t *pui8RdBuf)
{
    am_hal_host_xfer_mode_e xfer_mode;
    uint32_t ui32Stat = AM_HAL_STATUS_SUCCESS;
    uint32_t ui32BlkCntLoop;
    float fWriteBandwidth= 0;
    float fReadBandwidth = 0;

    //
    // Set transfer mode
    //
    if(pSDHCTestConfig->eXferMode == AM_WIDGET_HOST_XFER_PIO_SYNC)
    {
        xfer_mode = AM_HAL_HOST_XFER_PIO;
    }
    else
    {
        xfer_mode = AM_HAL_HOST_XFER_ADMA;
    }
    am_hal_card_host_set_xfer_mode(pHandles->pSdhcHost, xfer_mode);

    //
    // Register callback regardless
    //
    if(pSDHCTestConfig->eHost == AM_HAL_SDHC_CARD_HOST)
    {
        am_hal_card_register_evt_callback(pHandles->pDevHandle, am_widget_host0_event_cb);
    }
    else if(pSDHCTestConfig->eHost == AM_HAL_SDHC_CARD_HOST1)
    {
        am_hal_card_register_evt_callback(pHandles->pDevHandle, am_widget_host1_event_cb);
    }

    //
    // Set bus mode, width and clock speed for test
    //
    if(AM_HAL_STATUS_SUCCESS != am_hal_card_cfg_set(pHandles->pDevHandle, pSDHCTestConfig->eCardType,
                                                        pSDHCTestConfig->eBusWidth,
                                                        sdio_test_speeds[pSDHCTestConfig->eClock].speed,
                                                        pSDHCTestConfig->eBusVoltage,
                                                        pSDHCTestConfig->eUHSMode))
    {
        am_util_stdio_printf("\nsd card:%d config set failed\n", pSDHCTestConfig->eHost);
        return false;
    }

    am_util_stdio_printf("\n sd card bechmark test result:  %10s  |  %8s  |  %8s  |  %8s  |\n", "xfer mode", "width", "Wr MB/s", "Rd MB/s");

    for(uint32_t ui32BlockCount = 1; ui32BlockCount <= pSDHCTestConfig->ui32BlockCount; ui32BlockCount += pSDHCTestConfig->ui32BlockCount / 4 + 1)
    {
        ui32BlkCntLoop = 0;
        fWriteBandwidthTotal = 0;
        fReadBandwidthTotal = 0;

        for(uint32_t ui32StartBlock = pSDHCTestConfig->ui32StartBlock; ui32StartBlock < pSDHCTestConfig->ui32SectorCount; ui32StartBlock += pSDHCTestConfig->ui32SectorCount/2 - ui32BlockCount + 1)
        {
            am_util_debug_printf("\nstart_blk = %d, blk_cnt = %d\n",ui32StartBlock, ui32BlockCount);
            ui32BlkCntLoop ++;

            //
            // erase sd card before write
            //
            ui32Stat = am_hal_sd_card_block_erase(pHandles->pDevHandle, ui32StartBlock, ui32BlockCount, 10000);
            if(ui32Stat != 0x0)
            {
                am_util_stdio_printf("erase sd card failed ui32Stat=%d\n", ui32Stat);
                return false;
            }

            ui32Stat = am_widget_sd_card_benchmark_write(pHandles, pSDHCTestConfig, pSDHCTestConfig->ui32StartBlock, ui32BlockCount, pui8WrBuf);
            if(ui32Stat != AM_HAL_STATUS_SUCCESS)
            {
                am_util_stdio_printf("Failed to benchmark write ui32Stat=%d\n", ui32Stat);
                return ui32Stat;
            }

            ui32Stat = am_widget_sd_card_benchmark_read(pHandles, pSDHCTestConfig, pSDHCTestConfig->ui32StartBlock, ui32BlockCount, pui8RdBuf);
            if(ui32Stat != AM_HAL_STATUS_SUCCESS)
            {
                am_util_stdio_printf("Failed to benchmark read ui32Stat=%d\n", ui32Stat);
                return ui32Stat;
            }

            if(!am_widget_check_data_match(pui8RdBuf, pui8WrBuf, 512*ui32BlockCount))
            {
                return false;
            }
        }

        fWriteBandwidth = fWriteBandwidthTotal/ui32BlkCntLoop;
        fReadBandwidth = fReadBandwidthTotal/ui32BlkCntLoop;

        am_util_stdio_printf("                   %4dblocks:  %10s  |  %8s  |  %8.5f  |  %8.5f  |\n", ui32BlockCount, 
                                                    sdio_benchmark_xfer_modes[pSDHCTestConfig->eXferMode].string, 
                                                           sdio_test_widths[pSDHCTestConfig->eBusWidth/4].string,
                                                  fWriteBandwidth, fReadBandwidth);
    }

    return true;
}

#endif

bool am_widget_sdio_test_write_read(am_widget_sdio_config_t *pHandles,am_widget_sdio_test_t *pSDHCTestConfig, uint8_t *pui8WrBuf, uint8_t *pui8RdBuf)
{
    am_widget_emmc_devices_config_t *eMMC_device_config = (am_widget_emmc_devices_config_t *)pSDHCTestConfig->device_config;
    am_hal_host_xfer_mode_e xfer_mode;
    uint32_t ui32Stat = AM_HAL_STATUS_SUCCESS;
    uint32_t ui32BufAddr = 0;

    am_hal_cachectrl_range_t sRange;

    //
    // Set transfer mode
    //
    if(pSDHCTestConfig->eXferMode == AM_WIDGET_HOST_XFER_PIO_SYNC)
    {
        xfer_mode = AM_HAL_HOST_XFER_PIO;
    }
    else
    {
        xfer_mode = AM_HAL_HOST_XFER_ADMA;
    }
    am_hal_card_host_set_xfer_mode(pHandles->pSdhcHost, xfer_mode);

    //
    // Register callback regardless
    //
    if(pSDHCTestConfig->eHost == AM_HAL_SDHC_CARD_HOST)
    {
        am_hal_card_register_evt_callback(pHandles->pDevHandle, am_widget_host0_event_cb);
    }
    else if(pSDHCTestConfig->eHost == AM_HAL_SDHC_CARD_HOST1)
    {
        am_hal_card_register_evt_callback(pHandles->pDevHandle, am_widget_host1_event_cb);
    }

    for(uint32_t ui32BlockCount = 1; ui32BlockCount <= pSDHCTestConfig->ui32BlockCount; ui32BlockCount += pSDHCTestConfig->ui32BlockCount / 4 + 1)
    {
        for(uint32_t ui32StartBlock = pSDHCTestConfig->ui32StartBlock; ui32StartBlock < pSDHCTestConfig->ui32SectorCount; ui32StartBlock += pSDHCTestConfig->ui32SectorCount/2 - ui32BlockCount + 1)
        {
            am_util_stdio_printf("\nstart_blk = %d, blk_cnt = %d\n",ui32StartBlock, ui32BlockCount);

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

            //
            // erase eMMC before write
            //
            ui32Stat = am_hal_card_block_erase(pHandles->pDevHandle, ui32StartBlock, ui32BlockCount, AM_HAL_ERASE, 1000);
            am_util_stdio_printf("erase eMMC before write, Erase Status = %d\n", ui32Stat);
            if(ui32Stat != 0x0)
            {
                am_util_stdio_printf("erase eMMC failed\n");
                return false;
            }

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

            //
            // make sure dma memory test buffer do cache cleaning
            //
            ui32BufAddr = (uint32_t)pui8WrBuf;
            if ( (ui32BufAddr >= SSRAM_BASEADDR) || ((ui32BufAddr + MEM_TEST_SZ) >= SSRAM_BASEADDR) )
            {
                //
                // Clean dcache data before write.
                //
                if ( ui32BlockCount*512 > DCACHE_SIZE ) // if ssram buffer > 64K, clean all is efficient
                {
                    am_hal_cachectrl_dcache_clean(NULL);
                }
                else
                {
                    sRange.ui32StartAddr = (uint32_t)pui8WrBuf;
                    sRange.ui32Size = ui32BlockCount*512;
                    am_hal_cachectrl_dcache_clean(&sRange);
                }
            }

            //
            // write data to emmc flash
            //
            if(pSDHCTestConfig->eXferMode == AM_WIDGET_HOST_XFER_PIO_SYNC || pSDHCTestConfig->eXferMode == AM_WIDGET_HOST_XFER_ADMA_SYNC)
            {
                ui32Stat = am_hal_card_block_write_sync(pHandles->pDevHandle, ui32StartBlock, ui32BlockCount, pui8WrBuf);
                am_util_stdio_printf("Synchronous Writing %d blocks is done, Xfer Status %d\n", ui32Stat >> 16, ui32Stat & 0xffff);
                if((ui32Stat & 0xffff) != 0 )
                {
                    am_util_stdio_printf("Failed to write card.\n");
                    return false;
                }
            }
            else if(pSDHCTestConfig->eXferMode == AM_WIDGET_HOST_XFER_ADMA_ASYNC)
            {
                eMMC_device_config->bAsyncWriteIsDone = false;
                ui32Stat = am_hal_card_block_write_async(pHandles->pDevHandle, ui32StartBlock, ui32BlockCount, pui8WrBuf);
                am_util_stdio_printf("Asynchronous ADMA Write Start, Write Status = %d\n", ui32Stat);
                if(ui32Stat != 0)
                {
                    am_util_stdio_printf("Failed to write card.\n");
                    return false;                    
                }

                //
                // wait until the async write is done
                //
                uint32_t i = 0;
                while (!eMMC_device_config->bAsyncWriteIsDone)
                {
                    am_util_delay_ms(1);
                    i++;
                    if ( i == DELAY_MAX_COUNT )
                    {
                        am_util_stdio_printf("Failed to write card, async write timeout.\n");
                        return false;
                    }
                }

                if( i < DELAY_MAX_COUNT)
                {
                    am_util_stdio_printf("asynchronous block write is done\n");
                }
            }

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

            //
            // read back data from eMMC
            //
            memset(pui8RdBuf, 0, BUF_LEN);

            ui32BufAddr = (uint32_t)pui8RdBuf;
            if ( (ui32BufAddr >= SSRAM_BASEADDR) || ((ui32BufAddr + MEM_TEST_SZ) >= SSRAM_BASEADDR) )
            {
                //
                // Clean dcache data before write.
                //
                if ( ui32BlockCount*512 > DCACHE_SIZE ) // if ssram buffer > 64K, clean all is efficient
                {
                    am_hal_cachectrl_dcache_clean(NULL);
                }
                else
                {
                    sRange.ui32StartAddr = (uint32_t)pui8RdBuf;
                    sRange.ui32Size = ui32BlockCount*512;
                    am_hal_cachectrl_dcache_clean(&sRange);
                }
            }

            if(pSDHCTestConfig->eXferMode == AM_WIDGET_HOST_XFER_PIO_SYNC || pSDHCTestConfig->eXferMode == AM_WIDGET_HOST_XFER_ADMA_SYNC)
            {
                ui32Stat = am_hal_card_block_read_sync(pHandles->pDevHandle, ui32StartBlock, ui32BlockCount, pui8RdBuf);
                am_util_stdio_printf("Synchronous Reading %d blocks is done, Xfer Status %d\n", ui32Stat >> 16, ui32Stat & 0xffff);
                if((ui32Stat & 0xffff) != 0 )
                {
                    am_util_stdio_printf("Failed to read card.\n");
                    return false;
                }

                ui32BufAddr = (uint32_t)pui8RdBuf;
                if ( (ui32BufAddr >= SSRAM_BASEADDR) || ((ui32BufAddr + MEM_TEST_SZ) >= SSRAM_BASEADDR) )
                {
                    //
                    // Clean dcache data before write.
                    //
                    if ( ui32BlockCount*512 > DCACHE_SIZE ) // if ssram buffer > 64K, clean all is efficient
                    {
                        am_hal_cachectrl_dcache_invalidate(NULL, true);
                    }
                    else
                    {
                        sRange.ui32StartAddr = (uint32_t)pui8RdBuf;
                        sRange.ui32Size = ui32BlockCount*512;
                        am_hal_cachectrl_dcache_invalidate(&sRange, true);
                    }
                }

            }
            else if(pSDHCTestConfig->eXferMode == AM_WIDGET_HOST_XFER_ADMA_ASYNC)
            {
                eMMC_device_config->bAsyncReadIsDone = false;
                ui32Stat = am_hal_card_block_read_async(pHandles->pDevHandle, ui32StartBlock, ui32BlockCount, pui8RdBuf);
                am_util_stdio_printf("Asynchronous ADMA Read Start, Read Status = %d\n", ui32Stat);
                if(ui32Stat != 0)
                {
                    am_util_stdio_printf("Failed to read card.\n");
                    return false;                    
                }

                //
                // wait until the async read is done
                //
                uint32_t i = 0;
                while (!eMMC_device_config->bAsyncReadIsDone)
                {
                    am_util_delay_ms(1);
                    i++;
                    if ( i == DELAY_MAX_COUNT )
                    {
                        am_util_stdio_printf("Failed to read card, async read timeout\n");
                        return false;
                    }
                }

                if( i < DELAY_MAX_COUNT)
                {
                    am_util_stdio_printf("asynchronous block read is done\n");
                }
            }

            //
            // check if block data match or not
            //
            if(!am_widget_check_data_match(pui8RdBuf, pui8WrBuf, 512*ui32BlockCount))
            {
                return false;
            }
        }
    }
    return true;
}

//
// Check erase data
//
bool am_widget_emmc_erase_check(am_hal_card_t *pCard, uint8_t *pui8RdBuf, uint32_t ui32Len)
{
    uint32_t i;
    uint32_t err_cnt =0;
    bool pass = true;

    //
    // get erase memory range
    //
    uint8_t ui8EraseData = am_hal_unstuff_bytes(pCard->ui32ExtCSD, 181, 1);
    if( ui8EraseData )
    {
        ui8EraseData = 0xFF;
    }
    else
    {
        ui8EraseData = 0;
    }

    for (i = 0; i < ui32Len; i++)
    {
        if (pui8RdBuf[i] != ui8EraseData)
        {
            am_util_stdio_printf("RdBuf[%d]=%x,ui8EraseData=%x\n", i, pui8RdBuf[i], ui8EraseData);
            pass = false;
            err_cnt ++;
            if(err_cnt > DATA_ERROR_CHECK_NUM)
            {
                am_util_stdio_printf("\n******************************************************************************\n");
                am_util_stdio_printf("Mismatched data count is larger than %d. Stopped printing data buffer, len=%d\n", DATA_ERROR_CHECK_NUM, ui32Len);
                am_util_stdio_printf("********************************************************************************\n");
                return pass;
            }
        }
    }

    if (pass)
    {
        am_util_stdio_printf("erase pass: data is 0x%x\n", ui8EraseData);
    }
    else
    {
        am_util_stdio_printf("erase fail: erase data not matched, len=%d\n", ui32Len);
    }

    return pass;
}

//
// Get the size of erase group
//
static uint32_t 
am_widget_get_emmc_erase_group(am_hal_card_t *pCard)
{
    uint32_t ui32EraseGroup, ui32EraseGroupDef = 0;

    ui32EraseGroupDef = am_hal_unstuff_bytes(pCard->ui32ExtCSD, 175, 1);
    if ( ui32EraseGroupDef )
    {
        uint32_t ui32EraseGprSize = am_hal_unstuff_bytes(pCard->ui32ExtCSD, 224, 1);
        ui32EraseGroup = 512*1024*ui32EraseGprSize/pCard->ui32BlkSize;
    }
    else
    {
        uint32_t ui32EraseGprSize = am_hal_unstuff_bits(pCard->ui32CSD, 37, 5);
        uint32_t ui32EraseGprMult = am_hal_unstuff_bits(pCard->ui32CSD, 42, 5);
        ui32EraseGroup = ( ui32EraseGprSize + 1 )*( ui32EraseGprMult + 1 );
    }

    return ui32EraseGroup;
}

static bool am_widget_emmc_erase(am_widget_sdio_config_t *pHandles, am_widget_sdio_test_t *pSDHCTestConfig,
                               uint32_t ui32StartBlock, uint32_t ui32BlockOffSet , uint32_t ui32BlockCount, 
                                           uint32_t ui32EraseBlock, uint8_t *pui8WrBuf, uint8_t *pui8RdBuf)
{
    uint32_t ui32Stat = AM_HAL_STATUS_SUCCESS;
    uint32_t ui32TimeOut = ERASE_TIME_OUT;
    uint32_t ui32BufAddr = 0;
    am_hal_cachectrl_range_t sRange;

    if(pSDHCTestConfig->eEraseType != AM_HAL_ERASE)
    {
        ui32TimeOut = SECURE_ERASE_TIME_OUT;
    }

    ui32Stat = am_hal_card_block_erase(pHandles->pDevHandle, ui32StartBlock + ui32BlockOffSet, ui32BlockCount, pSDHCTestConfig->eEraseType, ui32TimeOut);
    if(ui32Stat != 0x0)
    {
        am_util_stdio_printf("erase eMMC failed\n");
        return false;
    }

    memset(pui8RdBuf, 0x55, BUF_LEN);

    ui32BufAddr = (uint32_t)pui8RdBuf;
    if ( (ui32BufAddr >= SSRAM_BASEADDR) || ((ui32BufAddr + MEM_TEST_SZ) >= SSRAM_BASEADDR) )
    {
        //
        // Clean dcache data before write.
        //
        if ( ui32BlockCount*512 > DCACHE_SIZE ) // if ssram buffer > 64K, clean all is efficient
        {
            am_hal_cachectrl_dcache_clean(NULL);
        }
        else
        {
            sRange.ui32StartAddr = (uint32_t)pui8RdBuf;
            sRange.ui32Size = ui32BlockCount*512;
            am_hal_cachectrl_dcache_clean(&sRange);
        }
    }

    //
    // If erase start block is the multiple of ui32EraseBlock, usually the ui32EraseBlock is 1024
    // SDIO host erase the whole ui32EraseBlock, erase 1024 blocks: ui32StartBlock+1024
    // else will erase the 2 consecutive blocks, erase 2048 blocks: ui32StartBlock-1024, ui32StartBlock+1024
    //
    if ((ui32StartBlock + ui32BlockOffSet) % ui32EraseBlock == 0)
    {
        ui32Stat = am_hal_card_block_read_sync(pHandles->pDevHandle, ui32StartBlock + ui32BlockOffSet, ui32BlockCount, pui8RdBuf);
        am_util_stdio_printf("Cheking erase region: start block %d, block count is: %d\n", ui32StartBlock + ui32BlockOffSet + ui32EraseBlock, ui32Stat >> 16);
        if((ui32Stat & 0xffff) != 0 )
        {
            am_util_stdio_printf("Failed to read card.\n");
            return false;
        }

        ui32BufAddr = (uint32_t)pui8RdBuf;
        if ( (ui32BufAddr >= SSRAM_BASEADDR) || ((ui32BufAddr + MEM_TEST_SZ) >= SSRAM_BASEADDR) )
        {
            //
            // Clean dcache data before write.
            //
            if ( ui32BlockCount*512 > DCACHE_SIZE ) // if ssram buffer > 64K, clean all is efficient
            {
                am_hal_cachectrl_dcache_invalidate(NULL, true);
            }
            else
            {
                sRange.ui32StartAddr = (uint32_t)pui8RdBuf;
                sRange.ui32Size = ui32BlockCount*512;
                am_hal_cachectrl_dcache_invalidate(&sRange, false);
            }
        }

        if(am_widget_emmc_erase_check(pHandles->pDevHandle, pui8RdBuf, 512*ui32BlockCount) != true)
        {
            return false;
        }

        if ( ui32BlockOffSet < 2*ui32EraseBlock)
        {
            memset(pui8RdBuf, 0, BUF_LEN);

            ui32BufAddr = (uint32_t)pui8RdBuf;
            if ( (ui32BufAddr >= SSRAM_BASEADDR) || ((ui32BufAddr + MEM_TEST_SZ) >= SSRAM_BASEADDR) )
            {
                //
                // Clean dcache data before write.
                //
                if ( ui32BlockCount*512 > DCACHE_SIZE ) // if ssram buffer > 64K, clean all is efficient
                {
                    am_hal_cachectrl_dcache_clean(NULL);
                }
                else
                {
                    sRange.ui32StartAddr = (uint32_t)pui8RdBuf;
                    sRange.ui32Size = ui32BlockCount*512;
                    am_hal_cachectrl_dcache_clean(&sRange);
                }
            }

            //
            // Check the blocks that not erase
            //
            ui32Stat = am_hal_card_block_read_sync(pHandles->pDevHandle, ui32StartBlock + ui32BlockOffSet + ui32EraseBlock, ERASE_TEST_START_BLK_OFFSET, pui8RdBuf);
            am_util_stdio_printf("Cheking not erase region: start block %d, block count is: %d\n", ui32StartBlock + ui32BlockOffSet + ui32EraseBlock, ui32Stat >> 16);
            if((ui32Stat & 0xffff) != 0 )
            {
                am_util_stdio_printf("Failed to read card.\n");
                return false;
            }

            ui32BufAddr = (uint32_t)pui8RdBuf;
            if ( (ui32BufAddr >= SSRAM_BASEADDR) || ((ui32BufAddr + MEM_TEST_SZ) >= SSRAM_BASEADDR) )
            {
                //
                // Clean dcache data before write.
                //
                if ( ui32BlockCount*512 > DCACHE_SIZE ) // if ssram buffer > 64K, clean all is efficient
                {
                    am_hal_cachectrl_dcache_invalidate(NULL, true);
                }
                else
                {
                    sRange.ui32StartAddr = (uint32_t)pui8RdBuf;
                    sRange.ui32Size = ui32BlockCount*512;
                    am_hal_cachectrl_dcache_invalidate(&sRange, false);
                }
            }

            if(!am_widget_check_data_match(pui8RdBuf, pui8WrBuf, 512*ERASE_TEST_START_BLK_OFFSET))
            {
                return false;
            }
        }
    }
    else
    {
        //
        //Check the start of the current group data
        //
        uint32_t ui32StartBlkOffSet = ( ui32StartBlock + ui32BlockOffSet ) % ui32EraseBlock;
        ui32Stat = am_hal_card_block_read_sync(pHandles->pDevHandle, ui32StartBlock + ui32BlockOffSet - ui32StartBlkOffSet, ui32BlockCount, pui8RdBuf);
        am_util_stdio_printf("Checking erase region: start block %d, block count is: %d\n", ui32StartBlock + ui32BlockOffSet - ui32StartBlkOffSet, ui32Stat >> 16);
        if((ui32Stat & 0xffff) != 0 )
        {
            am_util_stdio_printf("Failed to read card.\n");
            return false;
        }

        ui32BufAddr = (uint32_t)pui8RdBuf;
        if ( (ui32BufAddr >= SSRAM_BASEADDR) || ((ui32BufAddr + MEM_TEST_SZ) >= SSRAM_BASEADDR) )
        {
            //
            // Clean dcache data before write.
            //
            if ( ui32BlockCount*512 > DCACHE_SIZE ) // if ssram buffer > 64K, clean all is efficient
            {
                am_hal_cachectrl_dcache_invalidate(NULL, true);
            }
            else
            {
                sRange.ui32StartAddr = (uint32_t)pui8RdBuf;
                sRange.ui32Size = ui32BlockCount*512;
                am_hal_cachectrl_dcache_invalidate(&sRange, false);
            }
        }

        if(am_widget_emmc_erase_check(pHandles->pDevHandle, pui8RdBuf, 512*ui32BlockCount) != true)
        {
            return false;
        }

        //
        // Check the erase data in current erase group and the last group
        //
        ui32Stat = am_hal_card_block_read_sync(pHandles->pDevHandle, ui32StartBlock + ui32BlockOffSet, ui32BlockCount, pui8RdBuf);
        am_util_stdio_printf("Checking erase region: start block %d, block count is: %d\n", ui32StartBlock + ui32BlockOffSet, ui32Stat >> 16);
        if((ui32Stat & 0xffff) != 0 )
        {
            am_util_stdio_printf("Failed to read card.\n");
            return false;
        }

        ui32BufAddr = (uint32_t)pui8RdBuf;
        if ( (ui32BufAddr >= SSRAM_BASEADDR) || ((ui32BufAddr + MEM_TEST_SZ) >= SSRAM_BASEADDR) )
        {
            //
            // Clean dcache data before write.
            //
            if ( ui32BlockCount*512 > DCACHE_SIZE ) // if ssram buffer > 64K, clean all is efficient
            {
                am_hal_cachectrl_dcache_invalidate(NULL, true);
            }
            else
            {
                sRange.ui32StartAddr = (uint32_t)pui8RdBuf;
                sRange.ui32Size = ui32BlockCount*512;
                am_hal_cachectrl_dcache_invalidate(&sRange, false);
            }
        }

        if(am_widget_emmc_erase_check(pHandles->pDevHandle, pui8RdBuf, 512*ui32BlockCount) != true)
        {
            return false;
        }

        memset(pui8RdBuf, 0x55, BUF_LEN);

        ui32BufAddr = (uint32_t)pui8RdBuf;
        if ( (ui32BufAddr >= SSRAM_BASEADDR) || ((ui32BufAddr + MEM_TEST_SZ) >= SSRAM_BASEADDR) )
        {
            //
            // Clean dcache data before write.
            //
            if ( ui32BlockCount*512 > DCACHE_SIZE ) // if ssram buffer > 64K, clean all is efficient
            {
                am_hal_cachectrl_dcache_clean(NULL);
            }
            else
            {
                sRange.ui32StartAddr = (uint32_t)pui8RdBuf;
                sRange.ui32Size = ui32BlockCount*512;
                am_hal_cachectrl_dcache_clean(&sRange);
            }
        }

        //
        // Check the ERASE_TEST_START_BLK_OFFSET blocks data in current erase group
        //
        ui32Stat = am_hal_card_block_read_sync(pHandles->pDevHandle, ui32StartBlock + ui32BlockOffSet + ui32EraseBlock, ERASE_TEST_START_BLK_OFFSET, pui8RdBuf);
        am_util_stdio_printf("Checking erase region: start block %d, block count is: %d\n", ui32StartBlock + ui32BlockOffSet + ui32EraseBlock, ui32Stat >> 16);
        if((ui32Stat & 0xffff) != 0 )
        {
            am_util_stdio_printf("Failed to read card.\n");
            return false;
        }

        ui32BufAddr = (uint32_t)pui8RdBuf;
        if ( (ui32BufAddr >= SSRAM_BASEADDR) || ((ui32BufAddr + MEM_TEST_SZ) >= SSRAM_BASEADDR) )
        {
            //
            // Clean dcache data before write.
            //
            if ( ui32BlockCount*512 > DCACHE_SIZE ) // if ssram buffer > 64K, clean all is efficient
            {
                am_hal_cachectrl_dcache_invalidate(NULL, true);
            }
            else
            {
                sRange.ui32StartAddr = (uint32_t)pui8RdBuf;
                sRange.ui32Size = ui32BlockCount*512;
                am_hal_cachectrl_dcache_invalidate(&sRange, false);
            }
        }

        if(am_widget_emmc_erase_check(pHandles->pDevHandle, pui8RdBuf, 512*ERASE_TEST_START_BLK_OFFSET) != true)
        {
            return false;
        }

        if ( ui32BlockOffSet < 2*ui32EraseBlock)
        {
            memset(pui8RdBuf, 0, BUF_LEN);

            ui32BufAddr = (uint32_t)pui8RdBuf;
            if ( (ui32BufAddr >= SSRAM_BASEADDR) || ((ui32BufAddr + MEM_TEST_SZ) >= SSRAM_BASEADDR) )
            {
                //
                // Clean dcache data before write.
                //
                if ( ui32BlockCount*512 > DCACHE_SIZE ) // if ssram buffer > 64K, clean all is efficient
                {
                    am_hal_cachectrl_dcache_clean(NULL);
                }
                else
                {
                    sRange.ui32StartAddr = (uint32_t)pui8RdBuf;
                    sRange.ui32Size = ui32BlockCount*512;
                    am_hal_cachectrl_dcache_clean(&sRange);
                }
            }

            //
            // Check the blocks that not erase
            //
            ui32Stat = am_hal_card_block_read_sync(pHandles->pDevHandle, ui32StartBlock + ui32BlockOffSet + 2*ui32EraseBlock, ERASE_TEST_START_BLK_OFFSET, pui8RdBuf);
            am_util_stdio_printf("Cheking not erase region: start block %d, block count is: %d\n", ui32StartBlock + ui32BlockOffSet + 2*ui32EraseBlock, ui32Stat >> 16);
            if((ui32Stat & 0xffff) != 0 )
            {
                am_util_stdio_printf("Failed to read card.\n");
                return false;
            }

            ui32BufAddr = (uint32_t)pui8RdBuf;
            if ( (ui32BufAddr >= SSRAM_BASEADDR) || ((ui32BufAddr + MEM_TEST_SZ) >= SSRAM_BASEADDR) )
            {
                //
                // Clean dcache data before write.
                //
                if ( ui32BlockCount*512 > DCACHE_SIZE ) // if ssram buffer > 64K, clean all is efficient
                {
                    am_hal_cachectrl_dcache_invalidate(NULL, true);
                }
                else
                {
                    sRange.ui32StartAddr = (uint32_t)pui8RdBuf;
                    sRange.ui32Size = ui32BlockCount*512;
                    am_hal_cachectrl_dcache_invalidate(&sRange, false);
                }
            }

            if(!am_widget_check_data_match(pui8RdBuf, pui8WrBuf, 512*ERASE_TEST_START_BLK_OFFSET))
            {
                return false;
            }
        }
    }

    return true;
}

static bool am_widget_emmc_trim_erase(am_widget_sdio_config_t *pHandles, am_widget_sdio_test_t *pSDHCTestConfig,
                            uint32_t ui32StartBlk, uint32_t ui32BlkCnt, uint8_t *pui8WrBuf, uint8_t *pui8RdBuf)
{
    uint32_t ui32Stat = AM_HAL_STATUS_SUCCESS;
    uint32_t ui32TrimeEraseOffSet = 10;
    uint32_t ui32TimeOut = ERASE_TIME_OUT;
    uint32_t ui32BufAddr = 0;
    am_hal_cachectrl_range_t sRange;

    if ( ui32TrimeEraseOffSet > ERASE_BLK_NUM )
    {
        return false;
    }

    if ( pSDHCTestConfig->eEraseType == AM_HAL_SECURE_TRIM2 )
    {
        ui32TimeOut = SECURE_ERASE_TIME_OUT;
    }

    ui32Stat = am_hal_card_block_erase(pHandles->pDevHandle, ui32StartBlk, ui32BlkCnt - ui32TrimeEraseOffSet, pSDHCTestConfig->eEraseType, ui32TimeOut);
    if(ui32Stat != 0x0)
    {
        am_util_stdio_printf("erase eMMC failed\n");
        return false;
    }

    if ( pSDHCTestConfig->eEraseType == AM_HAL_DISCARD )
    {
        am_util_stdio_printf("Skip blocks check when erase type is discard\n");
        return true;
    }

    //
    // Check erased blocks
    //
    if ( pSDHCTestConfig->eEraseType != AM_HAL_SECURE_TRIM1 )
    {
        memset(pui8RdBuf, 0x55, BUF_LEN);
 
        ui32BufAddr = (uint32_t)pui8RdBuf;
        if ( (ui32BufAddr >= SSRAM_BASEADDR) || ((ui32BufAddr + MEM_TEST_SZ) >= SSRAM_BASEADDR) )
        {
            //
            // Clean dcache data before write.
            //
            if ( ui32BlkCnt*512 > DCACHE_SIZE ) // if ssram buffer > 64K, clean all is efficient
            {
                am_hal_cachectrl_dcache_clean(NULL);
            }
            else
            {
                sRange.ui32StartAddr = (uint32_t)pui8RdBuf;
                sRange.ui32Size = ui32BlkCnt*512;
                am_hal_cachectrl_dcache_clean(&sRange);
            }
        }

        ui32Stat = am_hal_card_block_read_sync(pHandles->pDevHandle, ui32StartBlk, ui32BlkCnt - ui32TrimeEraseOffSet, pui8RdBuf);
        am_util_stdio_printf("Cheking erase region: start block %d, block count is: %d\n", ui32StartBlk, ui32Stat >> 16);
        if((ui32Stat & 0xffff) != 0 )
        {
            am_util_stdio_printf("Failed to read card.\n");
            return false;
        }

        ui32BufAddr = (uint32_t)pui8RdBuf;
        if ( (ui32BufAddr >= SSRAM_BASEADDR) || ((ui32BufAddr + MEM_TEST_SZ) >= SSRAM_BASEADDR) )
        {
            //
            // Clean dcache data before write.
            //
            if ( ui32BlkCnt*512 > DCACHE_SIZE ) // if ssram buffer > 64K, clean all is efficient
            {
                am_hal_cachectrl_dcache_invalidate(NULL, true);
            }
            else
            {
                sRange.ui32StartAddr = (uint32_t)pui8RdBuf;
                sRange.ui32Size = ui32BlkCnt*512;
                am_hal_cachectrl_dcache_invalidate(&sRange, false);
            }
        }

        if(am_widget_emmc_erase_check(pHandles->pDevHandle, pui8RdBuf, 512*(ui32BlkCnt - ui32TrimeEraseOffSet)) != true)
        {
            return false;
        }
    }

    //
    // Check the marked blocks in trim step1
    //
    if ( pSDHCTestConfig->eEraseType == AM_HAL_SECURE_TRIM1 )
    {
        memset(pui8RdBuf, 0x00, BUF_LEN);

        ui32BufAddr = (uint32_t)pui8RdBuf;
        if ( (ui32BufAddr >= SSRAM_BASEADDR) || ((ui32BufAddr + MEM_TEST_SZ) >= SSRAM_BASEADDR) )
        {
            //
            // Clean dcache data before write.
            //
            if ( ui32BlkCnt*512 > DCACHE_SIZE ) // if ssram buffer > 64K, clean all is efficient
            {
                am_hal_cachectrl_dcache_clean(NULL);
            }
            else
            {
                sRange.ui32StartAddr = (uint32_t)pui8RdBuf;
                sRange.ui32Size = ui32BlkCnt*512;
                am_hal_cachectrl_dcache_clean(&sRange);
            }
        }

        ui32Stat = am_hal_card_block_read_sync(pHandles->pDevHandle, ui32StartBlk, ui32BlkCnt - ui32TrimeEraseOffSet, pui8RdBuf);
        am_util_stdio_printf("Cheking erase region: start block %d, block count is: %d\n", ui32StartBlk, ui32Stat >> 16);
        if((ui32Stat & 0xffff) != 0 )
        {
            am_util_stdio_printf("Failed to read card.\n");
            return false;
        }

        ui32BufAddr = (uint32_t)pui8RdBuf;
        if ( (ui32BufAddr >= SSRAM_BASEADDR) || ((ui32BufAddr + MEM_TEST_SZ) >= SSRAM_BASEADDR) )
        {
            //
            // Clean dcache data before write.
            //
            if ( ui32BlkCnt*512 > DCACHE_SIZE ) // if ssram buffer > 64K, clean all is efficient
            {
                am_hal_cachectrl_dcache_invalidate(NULL, true);
            }
            else
            {
                sRange.ui32StartAddr = (uint32_t)pui8RdBuf;
                sRange.ui32Size = ui32BlkCnt*512;
                am_hal_cachectrl_dcache_invalidate(&sRange, false);
            }
        }

        if(am_widget_check_data_match( pui8RdBuf, pui8WrBuf, 512*(ui32BlkCnt - ui32TrimeEraseOffSet)) != true)
        {
            return false;
        }
    }

    //
    // Check not erased blocks 
    //
    ui32Stat = am_hal_card_block_read_sync(pHandles->pDevHandle, ui32StartBlk + ui32BlkCnt - ui32TrimeEraseOffSet, ui32TrimeEraseOffSet, pui8RdBuf);
    am_util_stdio_printf("Cheking not erase region: start block %d, block count is: %d\n", ui32StartBlk + ui32BlkCnt - ui32TrimeEraseOffSet, ui32Stat >> 16);
    if((ui32Stat & 0xffff) != 0 )
    {
        am_util_stdio_printf("Failed to read card.\n");
        return false;
    }

    ui32BufAddr = (uint32_t)pui8RdBuf;
    if ( (ui32BufAddr >= SSRAM_BASEADDR) || ((ui32BufAddr + MEM_TEST_SZ) >= SSRAM_BASEADDR) )
    {
        //
        // Clean dcache data before write.
        //
        if ( ui32BlkCnt*512 > DCACHE_SIZE ) // if ssram buffer > 64K, clean all is efficient
        {
            am_hal_cachectrl_dcache_invalidate(NULL, true);
        }
        else
        {
            sRange.ui32StartAddr = (uint32_t)pui8RdBuf;
            sRange.ui32Size = ui32BlkCnt*512;
            am_hal_cachectrl_dcache_invalidate(&sRange, false);
        }
    }

    //
    // Check if block data match with write buffer
    //
    if(!am_widget_check_data_match(pui8RdBuf, &pui8WrBuf[(ui32BlkCnt-ui32TrimeEraseOffSet)*512], 512*ui32TrimeEraseOffSet))
    {
        return false;
    }

    return true;
}

bool am_widget_emmc_test_erase(am_widget_sdio_config_t *pHandles, am_widget_sdio_test_t *pSDHCTestConfig, uint8_t *pui8WrBuf, uint8_t *pui8RdBuf)
{
    uint32_t ui32Stat = AM_HAL_STATUS_SUCCESS;
    uint32_t ui32BlockCount, ui32EraseBlock = 0;
    uint32_t ui32BufAddr = 0;
    am_hal_cachectrl_range_t sRange;

    ui32BlockCount = ERASE_BLK_NUM;

    ui32EraseBlock = am_widget_get_emmc_erase_group(pHandles->pDevHandle);
    am_util_debug_printf("Erase group is %d\n", ui32EraseBlock);

    //
    // Set transfer mode
    //
    am_hal_card_host_set_xfer_mode(pHandles->pSdhcHost, AM_HAL_HOST_XFER_ADMA);

    for(uint32_t ui32StartBlock = pSDHCTestConfig->ui32StartBlock; ui32StartBlock < pSDHCTestConfig->ui32SectorCount; ui32StartBlock += pSDHCTestConfig->ui32SectorCount/2 - 4*ui32EraseBlock - ERASE_TEST_START_BLK_OFFSET)
    {
        am_util_stdio_printf("\nstart_blk = %d, blk_cnt = %d\n",ui32StartBlock, ui32BlockCount);

        //
        //  Write data to emmc flash and check the data before erase
        //
        for (uint32_t ui32BlockOffSet = 0; ui32BlockOffSet < 4*ui32EraseBlock; ui32BlockOffSet += ui32EraseBlock)
        {
            if ( pSDHCTestConfig->eEraseType != AM_HAL_SECURE_TRIM2 )
            {
                //
                // make sure dma memory test buffer do cache cleaning
                //
                ui32BufAddr = (uint32_t)pui8WrBuf;
                if ( (ui32BufAddr >= SSRAM_BASEADDR) || ((ui32BufAddr + MEM_TEST_SZ) >= SSRAM_BASEADDR) )
                {
                    //
                    // Clean dcache data before write.
                    //
                    if ( ui32BlockCount*512 > DCACHE_SIZE ) // if ssram buffer > 64K, clean all is efficient
                    {
                        am_hal_cachectrl_dcache_clean(NULL);
                    }
                    else
                    {
                        sRange.ui32StartAddr = (uint32_t)pui8WrBuf;
                        sRange.ui32Size = ui32BlockCount*512;
                        am_hal_cachectrl_dcache_clean(&sRange);
                    }
                }

                ui32Stat = am_hal_card_block_write_sync(pHandles->pDevHandle, ui32StartBlock + ui32BlockOffSet, ui32BlockCount, pui8WrBuf);
                am_util_stdio_printf("Synchronous Writing %d blocks is done, start block %d, Xfer Status %d\n", ui32Stat >> 16, ui32StartBlock + ui32BlockOffSet, ui32Stat & 0xffff);
                if((ui32Stat & 0xffff) != 0 )
                {
                    am_util_stdio_printf("Failed to write card.\n");
                    return false;
                }

                memset(pui8RdBuf, 0, BUF_LEN);

                ui32BufAddr = (uint32_t)pui8RdBuf;
                if ( (ui32BufAddr >= SSRAM_BASEADDR) || ((ui32BufAddr + MEM_TEST_SZ) >= SSRAM_BASEADDR) )
                {
                    //
                    // Clean dcache data before write.
                    //
                    if ( ui32BlockCount*512 > DCACHE_SIZE ) // if ssram buffer > 64K, clean all is efficient
                    {
                        am_hal_cachectrl_dcache_clean(NULL);
                    }
                    else
                    {
                        sRange.ui32StartAddr = (uint32_t)pui8RdBuf;
                        sRange.ui32Size = ui32BlockCount*512;
                        am_hal_cachectrl_dcache_clean(&sRange);
                    }
                }

                ui32Stat = am_hal_card_block_read_sync(pHandles->pDevHandle, ui32StartBlock + ui32BlockOffSet, ui32BlockCount, pui8RdBuf);
                am_util_stdio_printf("Synchronous Reading %d blocks is done, start block %d, Xfer Status %d\n", ui32Stat >> 16, ui32StartBlock + ui32BlockOffSet, ui32Stat & 0xffff);
                if((ui32Stat & 0xffff) != 0 )
                {
                    am_util_stdio_printf("Failed to read card.\n");
                    return false;
                }

                ui32BufAddr = (uint32_t)pui8RdBuf;
                if ( (ui32BufAddr >= SSRAM_BASEADDR) || ((ui32BufAddr + MEM_TEST_SZ) >= SSRAM_BASEADDR) )
                {
                    //
                    // Clean dcache data before write.
                    //
                    if ( ui32BlockCount*512 > DCACHE_SIZE ) // if ssram buffer > 64K, clean all is efficient
                    {
                        am_hal_cachectrl_dcache_invalidate(NULL, true);
                    }
                    else
                    {
                        sRange.ui32StartAddr = (uint32_t)pui8RdBuf;
                        sRange.ui32Size = ui32BlockCount*512;
                        am_hal_cachectrl_dcache_invalidate(&sRange, false);
                    }
                }

                //
                // check if block data match or not
                //
                if(!am_widget_check_data_match(pui8RdBuf, pui8WrBuf, 512*ui32BlockCount))
                {
                    return false;
                }
            }
        }

        for (uint32_t ui32BlockOffSet = 0; ui32BlockOffSet < 3*ui32EraseBlock; ui32BlockOffSet += ui32EraseBlock)
        {
            if (pSDHCTestConfig->eEraseType == AM_HAL_ERASE || pSDHCTestConfig->eEraseType == AM_HAL_SECURE_ERASE)
            {
                ui32Stat = am_widget_emmc_erase(pHandles, pSDHCTestConfig, ui32StartBlock , ui32BlockOffSet, ui32BlockCount, ui32EraseBlock, pui8WrBuf, pui8RdBuf);
            }
            else
            {
                ui32Stat = am_widget_emmc_trim_erase(pHandles, pSDHCTestConfig, ui32StartBlock + ui32BlockOffSet, ui32BlockCount, pui8WrBuf, pui8RdBuf);
            }
        }
    }

    return ui32Stat;
}

//*****************************************************************************
//
// Widget Test Execution Functions
//
//*****************************************************************************

bool am_widget_check_data_match(uint8_t *pui8RdBuf, uint8_t *pui8WrBuf, uint32_t ui32Len)
{
    uint32_t i;
    uint32_t err_cnt =0;
    bool pass = true;
    for (i = 0; i < ui32Len; i++)
    {
        if (pui8RdBuf[i] != pui8WrBuf[i])
        {
            am_util_stdio_printf("RdBuf[%d]=%x,WrBuf[%d]=%x\n", i, pui8RdBuf[i], i, pui8WrBuf[i]);
            pass = false;
            err_cnt ++;
            if(err_cnt > DATA_ERROR_CHECK_NUM)
            {
                am_util_stdio_printf("\n******************************************************************************\n");
                am_util_stdio_printf("Mismatched data count is larger than %d. Stopped printing data buffer, len=%d\n", DATA_ERROR_CHECK_NUM, ui32Len);
                am_util_stdio_printf("********************************************************************************\n");
                return pass;
            }
        }
    }

    if (pass)
    {
        am_util_debug_printf("data matched\n");
    }
    else
    {
        am_util_stdio_printf("check data fail: data not matched, len=%d\n", ui32Len);
    }

    return pass;
}

void am_widget_prepare_testdata(uint8_t *pui8WrBuf, uint32_t ui32Len, uint32_t ui32Seed)
{
    srand(ui32Seed);
    for(uint32_t i = 0;i < ui32Len;++i)
    {
        pui8WrBuf[i] = rand();
    }
}

uint32_t am_widget_get_emmc_identifier(am_hal_card_t *pCard)
{
    uint32_t index = 0xFF;
    am_widget_emmc_device_info_t emmc_info;

#ifndef AM_HAL_DISABLE_API_VALIDATION
    if ( !pCard || !pCard->bCidValid )
    {
        return 0xFFFFFFFF;
    }
#endif // AM_HAL_DISABLE_API_VALIDATION
    emmc_info.manufacturerID = am_hal_unstuff_bits(pCard->ui32CID, 120, 8);
    for(uint32_t i = 0; i < 6; i++)
    {
        emmc_info.productName[i] = am_hal_unstuff_bits(pCard->ui32CID, 56 + i*8, 8);
    }
    emmc_info.productRevision = am_hal_unstuff_bits(pCard->ui32CID, 48, 8);
    emmc_info.OEM = am_hal_unstuff_bits(pCard->ui32CID, 104, 8);

    for(uint32_t i = 0; i < EMMC_NUM; i++)
    {
       if(emmc_info.manufacturerID == emmcs_info[i].manufacturerID &&
          emmc_info.OEM == emmcs_info[i].OEM &&
          emmc_info.productRevision == emmcs_info[i].productRevision)
        {
            index = i;
            for(uint32_t j = 0; j < 6; j++)
            {
                if(emmc_info.productName[j] != emmcs_info[i].productName[j])
                {
                    index = 0xFF;
                    break;
                }
            }
        }
    }
    return index;
}

static uint32_t am_widget_calhash(uint8_t *str)
{
    uint32_t hash = 5381;
    uint32_t c;

    while (c = *str++)
    {
        hash = ((hash << 5) + hash) + c; /* hash * 33 + c */
    }

    return hash;
}

void am_widget_setupTimingScanTable(am_widget_timing_scan_hashtable_t *table)
{
    uint8_t keystr[6];
    uint32_t key;
    am_widget_timing_scan_t value;
    value.bcalibrate = true;
    value.bvalid = false;

    am_util_stdio_sprintf((char *)keystr, "%d,%d,%d", AM_HAL_HOST_UHS_DDR50, AM_WIDGET_SDIO_48M, AM_HAL_HOST_BUS_WIDTH_4);
    key = am_widget_calhash(keystr);
    table[0].key = key;
    table[0].value = value;

    am_util_stdio_sprintf((char *)keystr, "%d,%d,%d", AM_HAL_HOST_UHS_DDR50, AM_WIDGET_SDIO_48M, AM_HAL_HOST_BUS_WIDTH_8);
    key = am_widget_calhash(keystr);
    table[1].key = key;
    table[1].value = value;

    am_util_stdio_sprintf((char *)keystr, "%d,%d,%d", AM_HAL_HOST_UHS_SDR50, AM_WIDGET_SDIO_48M, AM_HAL_HOST_BUS_WIDTH_8);
    key = am_widget_calhash(keystr);
    table[2].key = key;
    table[2].value = value;    

    table[3].key = 0x0;
}

am_widget_timing_scan_t *am_widget_findTimingParam(am_widget_timing_scan_hashtable_t *table, am_widget_sdio_test_t *pSDHCTestConfig)
{
    uint8_t keystr[6];
    am_util_stdio_sprintf((char *)keystr, "%d,%d,%d", pSDHCTestConfig->eUHSMode, pSDHCTestConfig->eClock, pSDHCTestConfig->eBusWidth);
    for(uint32_t i = 0; i < SCAN_NUM; i++)
    {
        if(table[i].key == am_widget_calhash(keystr))
        {
            am_util_stdio_printf("found key:0x%x\n",table[i].key);
            return &(table[i].value);
        }
    }
    return NULL;
}

//for emmc only
void am_widget_host0_event_cb(am_hal_host_evt_t *pEvt)
{
    am_hal_card_host_t *pHost = (am_hal_card_host_t *)pEvt->pCtx;
    am_widget_emmc_devices_config_t *pDevConfig = (am_widget_emmc_devices_config_t *)DUTs_setting[0].device_config;
    am_hal_cachectrl_range_t sRange;
    uint32_t ui32BufLen = 0;
    uint32_t ui32BufAddr = 0;

    if (AM_HAL_EVT_XFER_COMPLETE == pEvt->eType &&
        pHost->AsyncCmdData.dir == AM_HAL_DATA_DIR_READ)
    {
        pDevConfig->bAsyncReadIsDone = true;
        am_util_debug_printf("Host0 Last Read Xfered block %d\n", pEvt->ui32BlkCnt);

        //
        // Flush and invalidate cache, if ssram buffer > 64K, clean all is efficient
        //
        ui32BufAddr= (uint32_t)(pHost->AsyncCmdData.pui8Buf);
        if ( (ui32BufAddr >= SSRAM_BASEADDR) || ((ui32BufAddr + MEM_TEST_SZ) >= SSRAM_BASEADDR) )
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
        pDevConfig->bAsyncWriteIsDone = true;
        am_util_debug_printf("Host0 Last Write Xfered block %d\n", pEvt->ui32BlkCnt);
    }
}

//for emmc only
void am_widget_host1_event_cb(am_hal_host_evt_t *pEvt)
{
    am_hal_card_host_t *pHost = (am_hal_card_host_t *)pEvt->pCtx;
    am_widget_emmc_devices_config_t *pDevConfig = (am_widget_emmc_devices_config_t *)DUTs_setting[1].device_config;
    am_hal_cachectrl_range_t sRange;
    uint32_t ui32BufLen = 0;
    uint32_t ui32BufAddr = 0;

    if (AM_HAL_EVT_XFER_COMPLETE == pEvt->eType &&
        pHost->AsyncCmdData.dir == AM_HAL_DATA_DIR_READ)
    {
        pDevConfig->bAsyncReadIsDone = true;
        am_util_debug_printf("Host1 Last Read Xfered block %d\n", pEvt->ui32BlkCnt);

        //
        // Flush and invalidate cache, if ssram buffer > 64K, clean all is efficient
        //
        ui32BufAddr= (uint32_t)(pHost->AsyncCmdData.pui8Buf);
        if ( (ui32BufAddr >= SSRAM_BASEADDR) || ((ui32BufAddr + MEM_TEST_SZ) >= SSRAM_BASEADDR) )
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
        pDevConfig->bAsyncWriteIsDone = true;
        am_util_debug_printf("Host1 Last Write Xfered block %d\n", pEvt->ui32BlkCnt);
    }
}

//
// Init the emmc
//
bool am_widget_emmc_card_init(am_widget_sdio_config_t *pHandles,am_widget_sdio_test_t *pSDHCTestConfig)
{
    uint32_t i = 10;
    am_widget_emmc_devices_config_t *pDevConfig = (am_widget_emmc_devices_config_t *)pSDHCTestConfig->device_config;

    while (am_hal_card_init(pHandles->pDevHandle, pSDHCTestConfig->eCardType, pDevConfig->pCardPwrCtrlFunc, pDevConfig->eCardPwrCtrlPolicy) != AM_HAL_STATUS_SUCCESS)
    {
        am_util_delay_ms(1000);
        am_util_stdio_printf("emmc:%d is not ready, try again\n", pSDHCTestConfig->eHost);
        i--;
        if(i == 0)
        {
          return false;
        }
    }
    return true;
}

//*****************************************************************************
//
// Widget prepare test data pattern.
//
//*****************************************************************************
bool am_widget_prepare_data_pattern(uint32_t pattern_index, uint8_t* buff, uint32_t len)
{
    uint32_t *pui32TxPtr = (uint32_t*)buff;
    uint8_t  *pui8TxPtr  = (uint8_t*)buff;

    switch ( pattern_index )
    {
        case 0:
            // 0x5555AAAA
            for (uint32_t i = 0; i < len / 4; i++)
            {
               pui32TxPtr[i] = (0x5555AAAA);
            }
            break;
        case 1:
            // 0xFFFF0000
            for (uint32_t i = 0; i < len / 4; i++)
            {
               pui32TxPtr[i] = (0xFFFF0000);
            }
            break;
        case 2:
            // walking
            for (uint32_t i = 0; i < len; i++)
            {
               pui8TxPtr[i] = 0x01 << (i % 8);
            }
            break;
        case 3:
            // incremental from 1
            for (uint32_t i = 0; i < len; i++)
            {
               pui8TxPtr[i] = ((i + 1) & 0xFF);
            }
            break;
        case 4:
            // decremental from 0xff
            for ( uint32_t i = 0; i < len; i++ )
            {
                // decrement starting from 0xff
                pui8TxPtr[i] = (0xff - i) & 0xFF;
            }
            break;
        default:
            // incremental from 1
            for (uint32_t i = 0; i < len; i++)
            {
               pui8TxPtr[i] = ((i ) & 0xFF);
            }
            break;
    }
    return true;
}

//*****************************************************************************
//
// Rs9116 read bytes/blocks fron SDIO card in pio/sync/async mode.
//
//*****************************************************************************
bool am_widget_sdio_rsi_read(am_hal_card_t *pCard, am_widget_sdio_test_t *pSDHCTestConfig, uint32_t ui32Addr, uint32_t ui32Len, uint8_t *ui8RdBuf)
{
    uint32_t ui32Status;
    uint32_t ui32MemAddr;
    uint32_t ui32BlkSize;
    uint32_t no_of_blocks = 1;
    uint32_t ui32BufAddr = 0;

    am_hal_cachectrl_range_t sRange;
    am_widget_sdio_devices_config_t *sdio_device_config = (am_widget_sdio_devices_config_t *)pSDHCTestConfig->device_config;

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
        no_of_blocks = 0;
    }

    ui32MemAddr = (ui32Addr & 0xFFFF) | (1 << 16);

    if ( (pSDHCTestConfig->eXferMode == AM_WIDGET_HOST_XFER_ADMA_ASYNC) || (pSDHCTestConfig->eXferMode == AM_WIDGET_HOST_XFER_SDMA_ASYNC) )
    {
        sdio_device_config->bAsyncReadIsDone = false;
        ui32Status = am_hal_sdio_card_multi_bytes_read_async(pCard, 1, ui32MemAddr, (uint8_t *)(ui8RdBuf), no_of_blocks, ui32BlkSize, true);

        //
        // wait until the async read is done
        //
        uint32_t i = 0;
        while (!sdio_device_config->bAsyncReadIsDone)
        {
            am_util_delay_ms(1);
            i++;
            if ( i == DELAY_MAX_COUNT )
            {
                am_util_stdio_printf("Failed to read card, async read timeout\n");
                break;
            }
        }

        if( i < DELAY_MAX_COUNT)
        {
            am_util_debug_printf("asynchronous block read is done\n");
        }

        am_util_debug_printf("Asynchronous ADMA Read End, Read Status = %d\n", ui32Status);

        if(ui32Status != 0)
        {
            am_util_stdio_printf("Failed to read card.\n");
        }
    }
    else
    {
        ui32Status = am_hal_sdio_card_multi_bytes_read_sync(pCard, 1, ui32MemAddr, (uint8_t *)(ui8RdBuf), no_of_blocks, ui32BlkSize, true);

        if ( pSDHCTestConfig->eXferMode != AM_WIDGET_HOST_XFER_PIO_SYNC )
        {
            ui32BufAddr = (uint32_t)ui8RdBuf;
            if ( ui32BufAddr >= SSRAM_BASEADDR )
            {
                //
                // Clean dcache data before write.
                //
                if ( ui32Len > DCACHE_SIZE ) // if ssram buffer > 64K, clean all is efficient
                {
                    am_hal_cachectrl_dcache_invalidate(NULL, true);
                }
                else
                {
                    sRange.ui32StartAddr = ui32BufAddr;
                    sRange.ui32Size = ui32Len;
                    am_hal_cachectrl_dcache_invalidate(&sRange, false);
                }
            }
        }
    }

    if( (ui32Status & 0xFFFF) != AM_HAL_STATUS_SUCCESS )
    {
        am_util_stdio_printf("rsi_mem_rd FAIL. Status=0x%x\n", ui32Status);
        return false;
    }
    else
    {
        am_util_debug_printf("rsi_mem_rd address 0x%x\n", ui32MemAddr);
    }

    return true;
}

//*****************************************************************************
//
// Rs9116 write bytes/blocks fron SDIO card in pio/sync/async mode.
//
//*****************************************************************************
bool am_widget_sdio_rsi_write(am_hal_card_t *pCard, am_widget_sdio_test_t *pSDHCTestConfig, uint32_t ui32Addr, uint32_t ui32Len, uint8_t *ui8WrBuf)
{
    uint32_t ui32Status;
    uint32_t ui32MemAddr;
    uint32_t ui32BlkSize = 0;
    uint32_t no_of_blocks = 1;
    uint32_t ui32BufAddr = 0;

    am_hal_cachectrl_range_t sRange;

    am_widget_sdio_devices_config_t *sdio_device_config = (am_widget_sdio_devices_config_t *)pSDHCTestConfig->device_config;

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
        ui32BlkSize = RSI_BLOCK_SIZE;
    }
    else
    {
        ui32BlkSize = ui32Len;
        no_of_blocks = 0;
    }

    ui32MemAddr = (ui32Addr & 0xFFFF) | (1 << 16);

    //
    // make sure dma memory test buffer do cache cleaning
    //
    if ( pSDHCTestConfig->eXferMode != AM_WIDGET_HOST_XFER_PIO_SYNC )
    {
        ui32BufAddr = (uint32_t)ui8WrBuf;
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

    if ( (pSDHCTestConfig->eXferMode == AM_WIDGET_HOST_XFER_ADMA_ASYNC) || (pSDHCTestConfig->eXferMode == AM_WIDGET_HOST_XFER_SDMA_ASYNC) )
    {
        sdio_device_config->bAsyncWriteIsDone = false;
        ui32Status = am_hal_sdio_card_multi_bytes_write_async(pCard, 1, ui32MemAddr, (uint8_t *)ui8WrBuf, no_of_blocks, ui32BlkSize, true);
        am_util_debug_printf("Asynchronous Write Start, Write Status = %d\n", ui32Status);
        if(ui32Status != 0)
        {
            am_util_stdio_printf("Failed to write card.\n");
        }

        //
        // wait until the async write is done
        //
        uint32_t i = 0;
        while (!sdio_device_config->bAsyncWriteIsDone)
        {
            am_util_delay_ms(1);
            i++;
            if ( i == DELAY_MAX_COUNT )
            {
                am_util_stdio_printf("Failed to write card, async write timeout.\n");
                break;
            }
        }

        if( i < DELAY_MAX_COUNT)
        {
            am_util_debug_printf("asynchronous block write is done\n");
        }
    }
    else
    {
        ui32Status = am_hal_sdio_card_multi_bytes_write_sync(pCard, 1, ui32MemAddr, (uint8_t *)ui8WrBuf, no_of_blocks, ui32BlkSize, true);
    }

    if( (ui32Status & 0xFFFF) != AM_HAL_STATUS_SUCCESS )
    {
        am_util_stdio_printf("rsi_mem_wr FAIL. Status=0x%x\n", ui32Status);
        return false;
    }
    else
    {
        am_util_debug_printf("rsi_mem_wr address 0x%x\n", ui32MemAddr);
    }

    return true;
}


bool am_widget_sdio_rsi_single_write(am_hal_card_t *pCard, am_widget_sdio_test_t *pSDHCTestConfig, uint32_t ui32Addr, uint8_t ui8Data)
{
    uint32_t ui32Status;
    uint32_t ui32MemAddr;

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

    ui32MemAddr = (ui32Addr & 0xFFFF) | (1 << 16);

    ui32Status = am_hal_sdio_card_byte_write(pCard, 1, ui32MemAddr, ui8Data);

    if( (ui32Status & 0xFFFF) != AM_HAL_STATUS_SUCCESS )
    {
        am_util_stdio_printf("rsi_mem_wr FAIL. Status=0x%x\n", ui32Status);
        return false;
    }
    else
    {
        am_util_debug_printf("rsi_mem_wr address 0x%x\n", ui32MemAddr);
    }

    return true;
}

bool am_widget_sdio_rsi_single_read(am_hal_card_t *pCard, am_widget_sdio_test_t *pSDHCTestConfig, uint32_t ui32Addr, uint8_t *pui8Data)
{
    uint32_t ui32Status;
    uint32_t ui32MemAddr;

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

    ui32MemAddr = (ui32Addr & 0xFFFF) | (1 << 16);

    ui32Status = am_hal_sdio_card_byte_read(pCard, 1, ui32MemAddr, pui8Data);

    if( (ui32Status & 0xFFFF) != AM_HAL_STATUS_SUCCESS )
    {
        am_util_stdio_printf("rsi_mem_rd FAIL. Status=0x%x\n", ui32Status);
        return false;
    }
    else
    {
        am_util_debug_printf("rsi_mem_rd address 0x%x\n", ui32MemAddr);
    }

    return true;
}

//*****************************************************************************
//
// SDIO Device RS9116 init.
//
//*****************************************************************************
uint32_t am_widget_sdio_rs9116_function_init(am_widget_sdio_config_t *pHandles)
{
    uint32_t ui32FuncNum = 1;
    uint32_t ui32Stat = AM_HAL_STATUS_SUCCESS;

    //
    // Enable SDIO card function
    //
    ui32Stat = am_hal_sdio_card_func_enable(pHandles->pDevHandle, ui32FuncNum);
    if(ui32Stat != AM_HAL_STATUS_SUCCESS)
    {
        am_util_stdio_printf("Enable function failed. Status=0x%x\n", ui32Stat);
    }

    //
    // Enable SDIO card function interrupt
    //
    ui32Stat = am_hal_sdio_card_func_interrupt_enable(pHandles->pDevHandle, ui32FuncNum);
    if(ui32Stat != AM_HAL_STATUS_SUCCESS)
    {
        am_util_stdio_printf("Enable function interrupt failed. Status=0x%x\n", ui32Stat);
    }

    //
    // Get SDIO card function block size
    //
    uint32_t ui32BlkSize = 0;
    ui32Stat = am_hal_sdio_card_get_block_size(pHandles->pDevHandle, ui32FuncNum, &ui32BlkSize);
    if(ui32Stat != AM_HAL_STATUS_SUCCESS)
    {
        am_util_stdio_printf("Fail to get SDIO card block size. Status=0x%x\n", ui32Stat);
    }

    //
    // Set SDIO card function block size
    //
    ui32BlkSize = RSI_BLOCK_SIZE;
    ui32Stat = am_hal_sdio_card_set_block_size(pHandles->pDevHandle, ui32FuncNum, ui32BlkSize);
    if(ui32Stat != AM_HAL_STATUS_SUCCESS)
    {
        am_util_stdio_printf("Fail to set SDIO card block size. Status=0x%x\n", ui32Stat);
    }

    //
    // Get SDIO card function block size again
    //
    ui32Stat = am_hal_sdio_card_get_block_size(pHandles->pDevHandle, ui32FuncNum, &ui32BlkSize);
    if(ui32Stat != AM_HAL_STATUS_SUCCESS)
    {
        am_util_stdio_printf("Fail to check SDIO card block size. Status=0x%x\n", ui32Stat);
    }

    return ui32Stat;
}

//
// Init the wifi card
//
bool am_widget_sdio_card_init(am_widget_sdio_config_t *pHandles,am_widget_sdio_test_t *pSDHCTestConfig)
{
    uint32_t i = 10;
    uint32_t ui32Stat = AM_HAL_STATUS_SUCCESS;
    am_widget_sdio_devices_config_t *pDevConfig = (am_widget_sdio_devices_config_t *)pSDHCTestConfig->device_config;

    while (am_hal_card_init(pHandles->pDevHandle, pSDHCTestConfig->eCardType, pDevConfig->pCardPwrCtrlFunc, pDevConfig->eCardPwrCtrlPolicy) != AM_HAL_STATUS_SUCCESS)
    {
        am_util_delay_ms(1000);
        am_util_stdio_printf("sdio:%d and host is not ready, try again\n", pSDHCTestConfig->eHost);
        i--;
        if(i == 0)
        {
          return false;
        }
    }

    ui32Stat = am_widget_sdio_rs9116_function_init(pHandles);
    if(ui32Stat != AM_HAL_STATUS_SUCCESS)
    {
        am_util_stdio_printf("Fail to set RS9116 function. Status=0x%x\n", ui32Stat);
    }

    return true;
}

//
// Register SD Card power cycle function for card power on/off/reset
//
uint32_t am_widget_sd_card_power_config(am_hal_card_pwr_e eCardPwr)
{
    am_hal_gpio_pinconfig(AM_BSP_GPIO_SD_POWER_CTRL, am_hal_gpio_pincfg_output);
    am_hal_gpio_pinconfig(AM_BSP_GPIO_SD_LEVEL_SHIFT_SEL, am_hal_gpio_pincfg_output);

    if ( eCardPwr == AM_HAL_CARD_PWR_CYCLE )
    {
        //
        // SD Card power cycle or power on
        //
        am_hal_gpio_output_clear(AM_BSP_GPIO_SD_POWER_CTRL);
        am_util_delay_ms(20);
        am_hal_gpio_output_set(AM_BSP_GPIO_SD_POWER_CTRL);

        //
        // wait until the power supply is stable
        //
        am_util_delay_ms(20);

#ifdef SD_CARD_BOARD_SUPPORT_1_8_V
        am_hal_gpio_output_clear(AM_BSP_GPIO_SD_LEVEL_SHIFT_SEL);
#endif
    }
    else if ( eCardPwr == AM_HAL_CARD_PWR_OFF )
    {
        am_hal_gpio_output_clear(AM_BSP_GPIO_SD_POWER_CTRL);

#ifdef SD_CARD_BOARD_SUPPORT_1_8_V
        am_hal_gpio_output_clear(AM_BSP_GPIO_SD_LEVEL_SHIFT_SEL);
#endif
    }
    else if ( eCardPwr == AM_HAL_CARD_PWR_SWITCH )
    {
        //
        // set level shifter to 1.8V
        //
        am_hal_gpio_output_set(AM_BSP_GPIO_SD_LEVEL_SHIFT_SEL);
        am_util_delay_ms(20);
    }

    return true;
}

//
// Init the SD Card
//
bool am_widget_sd_card_init(am_widget_sdio_config_t *pHandles,am_widget_sdio_test_t *pSDHCTestConfig)
{
    uint32_t i = 10;
    am_widget_emmc_devices_config_t *pDevConfig = (am_widget_emmc_devices_config_t *)pSDHCTestConfig->device_config;

    while (am_hal_card_init(pHandles->pDevHandle, pSDHCTestConfig->eCardType, am_widget_sd_card_power_config, pDevConfig->eCardPwrCtrlPolicy) != AM_HAL_STATUS_SUCCESS)
    {
        am_util_delay_ms(1000);
        am_util_stdio_printf("sd card:%d is not ready, try again\n", pSDHCTestConfig->eHost);
        i--;
        if(i == 0)
        {
          return false;
        }
    }
    return true;
}

bool am_widget_sd_card_test_write_read(am_widget_sdio_config_t *pHandles,am_widget_sdio_test_t *pSDHCTestConfig, uint8_t *pui8WrBuf, uint8_t *pui8RdBuf)
{
    am_widget_emmc_devices_config_t *sd_card_device_config = (am_widget_emmc_devices_config_t *)pSDHCTestConfig->device_config;
    am_hal_host_xfer_mode_e xfer_mode;
    uint32_t ui32Stat = AM_HAL_STATUS_SUCCESS;
    uint32_t ui32BufAddr = 0;
    am_hal_cachectrl_range_t sRange;

    //
    // Set transfer mode
    //
    if(pSDHCTestConfig->eXferMode == AM_WIDGET_HOST_XFER_PIO_SYNC)
    {
        xfer_mode = AM_HAL_HOST_XFER_PIO;
    }
    else
    {
        xfer_mode = AM_HAL_HOST_XFER_ADMA;
    }
    am_hal_card_host_set_xfer_mode(pHandles->pSdhcHost, xfer_mode);

    //
    // Register callback regardless
    //
    if(pSDHCTestConfig->eHost == AM_HAL_SDHC_CARD_HOST)
    {
        am_hal_card_register_evt_callback(pHandles->pDevHandle, am_widget_host0_event_cb);
    }
    else if(pSDHCTestConfig->eHost == AM_HAL_SDHC_CARD_HOST1)
    {
        am_hal_card_register_evt_callback(pHandles->pDevHandle, am_widget_host1_event_cb);
    }

    //
    // Set bus mode, width and clock speed for test
    //
    if(AM_HAL_STATUS_SUCCESS != am_hal_card_cfg_set(pHandles->pDevHandle, pSDHCTestConfig->eCardType,
                                                        pSDHCTestConfig->eBusWidth,
                                                        sdio_test_speeds[pSDHCTestConfig->eClock].speed,
                                                        pSDHCTestConfig->eBusVoltage,
                                                        pSDHCTestConfig->eUHSMode))
    {
        am_util_stdio_printf("\nsd card:%d config set failed\n", pSDHCTestConfig->eHost);
        return false;
    }

    for(uint32_t ui32BlockCount = 1; ui32BlockCount <= pSDHCTestConfig->ui32BlockCount; ui32BlockCount += pSDHCTestConfig->ui32BlockCount / 4 + 1)
    {
        for(uint32_t ui32StartBlock = pSDHCTestConfig->ui32StartBlock; ui32StartBlock < pSDHCTestConfig->ui32SectorCount; ui32StartBlock += pSDHCTestConfig->ui32SectorCount/2 - ui32BlockCount + 1)
        {
            am_util_stdio_printf("\nstart_blk = %d, blk_cnt = %d\n",ui32StartBlock, ui32BlockCount);

            //
            // erase sd card before write
            //
            ui32Stat = am_hal_sd_card_block_erase(pHandles->pDevHandle, ui32StartBlock, ui32BlockCount, 10000);
            am_util_stdio_printf("erase SD Card before write, Erase Status = %d\n", ui32Stat);
            if(ui32Stat != 0x0)
            {
                am_util_stdio_printf("erase sd card failed\n");
                return false;
            }

            //
            // make sure dma memory test buffer do cache cleaning
            //
            ui32BufAddr = (uint32_t)pui8WrBuf;
            if ( (ui32BufAddr >= SSRAM_BASEADDR) || ((ui32BufAddr + MEM_TEST_SZ) >= SSRAM_BASEADDR) )
            {
                //
                // Clean dcache data before write.
                //
                if ( ui32BlockCount*512 > DCACHE_SIZE ) // if ssram buffer > 64K, clean all is efficient
                {
                    am_hal_cachectrl_dcache_clean(NULL);
                }
                else
                {
                    sRange.ui32StartAddr = (uint32_t)pui8WrBuf;
                    sRange.ui32Size = ui32BlockCount*512;
                    am_hal_cachectrl_dcache_clean(&sRange);
                }
            }

            //
            // write data to sd card
            //
            if(pSDHCTestConfig->eXferMode == AM_WIDGET_HOST_XFER_PIO_SYNC || pSDHCTestConfig->eXferMode == AM_WIDGET_HOST_XFER_ADMA_SYNC)
            {
                ui32Stat = am_hal_sd_card_block_write_sync(pHandles->pDevHandle, ui32StartBlock, ui32BlockCount, pui8WrBuf);
                am_util_stdio_printf("Synchronous Writing %d blocks is done, Xfer Status %d\n", ui32Stat >> 16, ui32Stat & 0xffff);
                if((ui32Stat & 0xffff) != 0 )
                {
                    am_util_stdio_printf("Failed to write sd card.\n");
                    return false;
                }
            }
            else if(pSDHCTestConfig->eXferMode == AM_WIDGET_HOST_XFER_ADMA_ASYNC)
            {
                sd_card_device_config->bAsyncWriteIsDone = false;
                ui32Stat = am_hal_sd_card_block_write_async(pHandles->pDevHandle, ui32StartBlock, ui32BlockCount, pui8WrBuf);
                am_util_stdio_printf("Asynchronous ADMA Write Start, Write Status = %d\n", ui32Stat);
                if(ui32Stat != 0)
                {
                    am_util_stdio_printf("Failed to write sd card.\n");
                    return false;                    
                }

                //
                // wait until the async write is done
                //
                uint32_t i = 0;
                while (!sd_card_device_config->bAsyncWriteIsDone)
                {
                    am_util_delay_ms(1);
                    i++;
                    if ( i == DELAY_MAX_COUNT )
                    {
                        am_util_stdio_printf("Failed to write card, async write timeout.\n");
                        return false;
                    }
                }

                if( i < DELAY_MAX_COUNT)
                {
                    am_util_stdio_printf("asynchronous block write is done\n");
                }
            }

            //
            // read back data from sd card
            //
            memset(pui8RdBuf, 0, BUF_LEN);

            ui32BufAddr = (uint32_t)pui8RdBuf;
            if ( (ui32BufAddr >= SSRAM_BASEADDR) || ((ui32BufAddr + MEM_TEST_SZ) >= SSRAM_BASEADDR) )
            {
                //
                // Clean dcache data before write.
                //
                if ( ui32BlockCount*512 > DCACHE_SIZE ) // if ssram buffer > 64K, clean all is efficient
                {
                    am_hal_cachectrl_dcache_clean(NULL);
                }
                else
                {
                    sRange.ui32StartAddr = (uint32_t)pui8RdBuf;
                    sRange.ui32Size = ui32BlockCount*512;
                    am_hal_cachectrl_dcache_clean(&sRange);
                }
            }

            if(pSDHCTestConfig->eXferMode == AM_WIDGET_HOST_XFER_PIO_SYNC || pSDHCTestConfig->eXferMode == AM_WIDGET_HOST_XFER_ADMA_SYNC)
            {
                ui32Stat = am_hal_sd_card_block_read_sync(pHandles->pDevHandle, ui32StartBlock, ui32BlockCount, pui8RdBuf);
                am_util_stdio_printf("Synchronous Reading %d blocks is done, Xfer Status %d\n", ui32Stat >> 16, ui32Stat & 0xffff);
                if((ui32Stat & 0xffff) != 0 )
                {
                    am_util_stdio_printf("Failed to read sd card.\n");
                    return false;
                }

                ui32BufAddr = (uint32_t)pui8RdBuf;
                if ( (ui32BufAddr >= SSRAM_BASEADDR) || ((ui32BufAddr + MEM_TEST_SZ) >= SSRAM_BASEADDR) )
                {
                    //
                    // Clean dcache data before write.
                    //
                    if ( ui32BlockCount*512 > DCACHE_SIZE ) // if ssram buffer > 64K, clean all is efficient
                    {
                        am_hal_cachectrl_dcache_invalidate(NULL, true);
                    }
                    else
                    {
                        sRange.ui32StartAddr = (uint32_t)pui8RdBuf;
                        sRange.ui32Size = ui32BlockCount*512;
                        am_hal_cachectrl_dcache_invalidate(&sRange, true);
                    }
                }                
            }
            else if(pSDHCTestConfig->eXferMode == AM_WIDGET_HOST_XFER_ADMA_ASYNC)
            {
                sd_card_device_config->bAsyncReadIsDone = false;
                ui32Stat = am_hal_sd_card_block_read_async(pHandles->pDevHandle, ui32StartBlock, ui32BlockCount, pui8RdBuf);
                am_util_stdio_printf("Asynchronous ADMA Read Start, Read Status = %d\n", ui32Stat);
                if(ui32Stat != 0)
                {
                    am_util_stdio_printf("Failed to read sd card.\n");
                    return false;                    
                }

                //
                // wait until the async read is done
                //
                uint32_t i = 0;
                while (!sd_card_device_config->bAsyncReadIsDone)
                {
                    am_util_delay_ms(1);
                    i++;
                    if ( i == DELAY_MAX_COUNT )
                    {
                        am_util_stdio_printf("Failed to read card, async read timeout\n");
                        return false;
                    }
                }

                if( i < DELAY_MAX_COUNT)
                {
                    am_util_stdio_printf("asynchronous block read is done\n");
                }
            }

            //
            // check if block data match or not
            //
            if(!am_widget_check_data_match(pui8RdBuf, pui8WrBuf, 512*ui32BlockCount))
            {
                return false;
            }
        }
    }
    return true;
}

#ifdef EMMC_RPMB_FEATURE
bool am_widget_set_key(am_hal_card_t *pCard)
{
    uint32_t ui32Status;

    //
    // Switch to rpmb partition
    //
    if( am_hal_card_get_ext_csd_field(pCard, MMC_EXT_REGS_PARTITON_CONFIG, 1) != AM_DEVICES_EMMC_RPMB_ACCESS )
    {
        if ( am_devices_emmc_rpmb_partition_switch(pCard, AM_DEVICES_EMMC_RPMB_ACCESS) != AM_DEVICES_EMMC_RPMB_STATUS_SUCCESS )
        {
            am_util_stdio_printf("eMMC switch partition failed\n");
            return false;
        }
    }

    //
    // Set 256bit key to emmc
    //
    ui32Status = am_devices_emmc_rpmb_set_key(pCard, (uint8_t *)dummy_key_hash);
    if (ui32Status != AM_DEVICES_EMMC_RPMB_STATUS_SUCCESS)
    {
        am_util_stdio_printf("eMMC set key failed, Stat: %d\n", ui32Status);
        return false;
    }
    else
    {
        am_util_stdio_printf("eMMC set key successful \n");
    }
    return true;
}

//*****************************************************************************
//
// Check secure key in rpm mode
//
//*****************************************************************************
bool am_widget_check_key(am_hal_card_t *pCard)
{
    uint32_t ui32Writecnt;
    uint32_t ui32Status;

    //
    // Switch to rpmb partition
    //
    if( am_hal_card_get_ext_csd_field(pCard, MMC_EXT_REGS_PARTITON_CONFIG, 1) != AM_DEVICES_EMMC_RPMB_ACCESS )
    {
        if ( am_devices_emmc_rpmb_partition_switch(pCard, AM_DEVICES_EMMC_RPMB_ACCESS) != AM_DEVICES_EMMC_RPMB_STATUS_SUCCESS )
        {
            am_util_stdio_printf("eMMC switch partition failed\n");
            return false;
        }
    }

    //
    // Set counter
    //
    ui32Status= am_devices_emmc_rpmb_get_counter(pCard, &ui32Writecnt, (uint8_t *)dummy_key_hash);
    if ( ui32Status == AM_DEVICES_EMMC_RPMB_STATUS_KEY_NOT_PROGRAMMED_ERROR)
    {
        ui32Status = am_widget_set_key(pCard);
        if( ui32Status != true)
        {
            return false;
        } 
    }
    else if (ui32Status == AM_DEVICES_EMMC_RPMB_STATUS_SUCCESS )
    {
        am_util_stdio_printf("eMMC writer counter: %d\n", ui32Writecnt);
    }
    else
    {
        am_util_stdio_printf("eMMC failed to get write counter, Stat:%d\n", ui32Status);
        return false;
    }

    return true;
}

bool am_widget_rpmb_partition_switch(am_hal_card_t *pCard, am_devices_emmc_partiton_access_e ePartionMode)
{
    if ( am_devices_emmc_rpmb_partition_switch(pCard, ePartionMode) != AM_HAL_STATUS_SUCCESS)
    {
        return false;
    }

    return true;
}

bool am_widget_rpmb_test_write_read(am_widget_sdio_config_t *pHandles, am_widget_sdio_test_t *pSDHCTestConfig, uint8_t *pui8WrBuf, uint8_t *pui8RdBuf)
{
    uint32_t testPassed;
    uint32_t ui32BufAddr = 0;

    am_hal_cachectrl_range_t sRange;

    //
    // Start rpmb write & read test
    //
    for(uint32_t ui32BlkCnt = 1; ui32BlkCnt <= pSDHCTestConfig->ui32RpmbBlkCnt; ui32BlkCnt += pSDHCTestConfig->ui32RpmbBlkCnt/ 2 + 1)
    {
        for(uint32_t ui32StartBlock = pSDHCTestConfig->ui32StartBlock; ui32StartBlock < pSDHCTestConfig->ui32RpmbCount; ui32StartBlock += pSDHCTestConfig->ui32RpmbCount/4 - ui32BlkCnt)
        {
            am_util_stdio_printf("ui32StartBlock = %d, ui32BlkCnt = %d\n",ui32StartBlock, ui32BlkCnt);

            //
            // make sure dma memory test buffer do cache cleaning
            //
            ui32BufAddr = (uint32_t)pui8WrBuf;
            if ( (ui32BufAddr >= SSRAM_BASEADDR) || ((ui32BufAddr + MEM_TEST_SZ) >= SSRAM_BASEADDR) )
            {
                //
                // Clean dcache data before write.
                //
                if ( ui32BlkCnt*512 > DCACHE_SIZE ) // if ssram buffer > 64K, clean all is efficient
                {
                    am_hal_cachectrl_dcache_clean(NULL);
                }
                else
                {
                    sRange.ui32StartAddr = (uint32_t)pui8WrBuf;
                    sRange.ui32Size = ui32BlkCnt*512;
                    am_hal_cachectrl_dcache_clean(&sRange);
                }
            }

            TEST_ASSERT_FALSE(am_devices_emmc_rpmb_write(pHandles->pDevHandle, (uint8_t *)pui8WrBuf, ui32StartBlock, ui32BlkCnt, (uint8_t *)dummy_key_hash));

            memset((void *)ui8RdBuf, 0x0, ui32BlkCnt*512);

            ui32BufAddr = (uint32_t)pui8RdBuf;
            if ( (ui32BufAddr >= SSRAM_BASEADDR) || ((ui32BufAddr + MEM_TEST_SZ) >= SSRAM_BASEADDR) )
            {
                //
                // Clean dcache data before write.
                //
                if ( ui32BlkCnt*512 > DCACHE_SIZE ) // if ssram buffer > 64K, clean all is efficient
                {
                    am_hal_cachectrl_dcache_clean(NULL);
                }
                else
                {
                    sRange.ui32StartAddr = (uint32_t)pui8RdBuf;
                    sRange.ui32Size = ui32BlkCnt*512;
                    am_hal_cachectrl_dcache_clean(&sRange);
                }
            }

            TEST_ASSERT_FALSE(am_devices_emmc_rpmb_read(pHandles->pDevHandle, (uint8_t *)pui8RdBuf, ui32StartBlock, ui32BlkCnt, (uint8_t *)dummy_key_hash));

            ui32BufAddr = (uint32_t)pui8RdBuf;
            if ( (ui32BufAddr >= SSRAM_BASEADDR) || ((ui32BufAddr + MEM_TEST_SZ) >= SSRAM_BASEADDR) )
            {
                //
                // Clean dcache data before write.
                //
                if ( ui32BlkCnt*512 > DCACHE_SIZE ) // if ssram buffer > 64K, clean all is efficient
                {
                    am_hal_cachectrl_dcache_invalidate(NULL, true);
                }
                else
                {
                    sRange.ui32StartAddr = (uint32_t)pui8RdBuf;
                    sRange.ui32Size = ui32BlkCnt*512;
                    am_hal_cachectrl_dcache_invalidate(&sRange, true);
                }
            }

            //
            // check if block data match or not
            //
            testPassed = am_widget_check_data_match((uint8_t *)pui8RdBuf, (uint8_t *)pui8WrBuf, AM_DEVICES_EMMC_RPMB_DATA_SIZE * ui32BlkCnt);
            if (!testPassed)
            {
                return false;
            }
        }
    }

    return true;
}
#endif

#ifdef EMMC_SCATTER_TEST
bool am_widget_emmc_scatter_test_write_read(am_widget_sdio_config_t *pHandles,am_widget_sdio_test_t *pSDHCTestConfig,
                            am_hal_card_iovec_t *pWriteVec, am_hal_card_iovec_t *pReadVec, uint32_t ui32IoVectorCount)
{
    am_widget_emmc_devices_config_t *eMMC_device_config = (am_widget_emmc_devices_config_t *)pSDHCTestConfig->device_config;
    uint32_t ui32Stat = AM_HAL_STATUS_SUCCESS;

    //
    // Set ADMA mode in scatter test
    //
    am_hal_card_host_set_xfer_mode(pHandles->pSdhcHost, AM_HAL_HOST_XFER_ADMA);

    //
    // Register callback regardless
    //
    if(pSDHCTestConfig->eHost == AM_HAL_SDHC_CARD_HOST)
    {
        am_hal_card_register_evt_callback(pHandles->pDevHandle, am_widget_host0_event_cb);
    }
    else if(pSDHCTestConfig->eHost == AM_HAL_SDHC_CARD_HOST1)
    {
        am_hal_card_register_evt_callback(pHandles->pDevHandle, am_widget_host1_event_cb);
    }

    for(uint32_t ui32StartBlock = pSDHCTestConfig->ui32StartBlock; ui32StartBlock < pSDHCTestConfig->ui32SectorCount; 
    ui32StartBlock += pSDHCTestConfig->ui32SectorCount/2 - ui32IoVectorCount*pSDHCTestConfig->ui32IoVectorCnt + 1)
    {
        am_util_stdio_printf("\nstart_blk = %d, iovector_blk = %d\n",ui32StartBlock, ui32IoVectorCount);

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

        //
        // erase eMMC before write
        //
        ui32Stat = am_hal_card_block_erase(pHandles->pDevHandle, ui32StartBlock, ui32IoVectorCount*pSDHCTestConfig->ui32IoVectorCnt, AM_HAL_ERASE, 1000);
        am_util_stdio_printf("erase eMMC before write, Erase Status = %d\n", ui32Stat);
        if(ui32Stat != 0x0)
        {
            am_util_stdio_printf("erase eMMC failed\n");
            return false;
        }

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

        am_hal_cachectrl_dcache_clean(NULL);

        //
        // scatter write data to emmc flash
        //
        if( pSDHCTestConfig->eXferMode == AM_WIDGET_HOST_XFER_ADMA_SYNC )
        {
            ui32Stat = am_hal_emmc_card_scatter_write_sync(pHandles->pDevHandle, ui32StartBlock, pWriteVec, pSDHCTestConfig->ui32IoVectorCnt);
            am_util_stdio_printf("Scatter Synchronous Writing %d blocks is done, Xfer Status %d\n", ui32Stat >> 16, ui32Stat & 0xffff);
            if((ui32Stat & 0xffff) != 0 )
            {
                am_util_stdio_printf("Failed to write card.\n");
                return false;
            }
        }
        else if(pSDHCTestConfig->eXferMode == AM_WIDGET_HOST_XFER_ADMA_ASYNC)
        {
            eMMC_device_config->bAsyncWriteIsDone = false;
            ui32Stat = am_hal_emmc_card_scatter_write_async(pHandles->pDevHandle, ui32StartBlock, pWriteVec, pSDHCTestConfig->ui32IoVectorCnt);
            am_util_stdio_printf("Scatter Asynchronous ADMA Write Start, Write Status = %d\n", ui32Stat);
            if(ui32Stat != 0)
            {
                am_util_stdio_printf("Failed to write card.\n");
                return false;                    
            }

            //
            // wait until the async write is done
            //
            uint32_t i = 0;
            while (!eMMC_device_config->bAsyncWriteIsDone)
            {
                am_util_delay_ms(1);
                i++;
                if ( i == DELAY_MAX_COUNT )
                {
                    am_util_stdio_printf("Failed to write card, scatter async write timeout.\n");
                    return false;
                }
            }

            if( i < DELAY_MAX_COUNT)
            {
                am_util_stdio_printf("Scatter asynchronous block write is done\n");
            }
        }

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

        //
        // read back data from eMMC
        //
        if( pSDHCTestConfig->eXferMode == AM_WIDGET_HOST_XFER_ADMA_SYNC )
        {
            ui32Stat = am_hal_emmc_card_scatter_read_sync(pHandles->pDevHandle, ui32StartBlock, pReadVec, pSDHCTestConfig->ui32IoVectorCnt);
            am_util_stdio_printf("Scatter Synchronous Reading %d blocks is done, Xfer Status %d\n", ui32Stat >> 16, ui32Stat & 0xffff);
            if((ui32Stat & 0xffff) != 0 )
            {
                am_util_stdio_printf("Failed to read card.\n");
                return false;
            }
        }
        else if(pSDHCTestConfig->eXferMode == AM_WIDGET_HOST_XFER_ADMA_ASYNC)
        {
            eMMC_device_config->bAsyncReadIsDone = false;
            ui32Stat = am_hal_emmc_card_scatter_read_async(pHandles->pDevHandle, ui32StartBlock, pReadVec, pSDHCTestConfig->ui32IoVectorCnt);
            am_util_stdio_printf("Scatter Asynchronous ADMA Read Start, Read Status = %d\n", ui32Stat);
            if(ui32Stat != 0)
            {
                am_util_stdio_printf("Failed to read card.\n");
                return false;                    
            }

            //
            // wait until the async read is done
            //
            uint32_t i = 0;
            while (!eMMC_device_config->bAsyncReadIsDone)
            {
                am_util_delay_ms(1);
                i++;
                if ( i == DELAY_MAX_COUNT )
                {
                    am_util_stdio_printf("Failed to read card, scatter async read timeout\n");
                    return false;
                }
            }

            if( i < DELAY_MAX_COUNT)
            {
                am_util_stdio_printf("scatter asynchronous block read is done\n");
            }
        }

        am_hal_cachectrl_dcache_invalidate(NULL, true);

        //
        // check if block data match or not
        //
        for ( int i = 0; i < pSDHCTestConfig->ui32IoVectorCnt; i++ )
        {
            if (am_widget_check_data_match((uint8_t *)(pReadVec[i].pIovBase), (uint8_t *)(pWriteVec[i].pIovBase), pReadVec[i].ui32IovLen))
            {
                am_util_stdio_printf("data matched, IovBase address:0x%x, IovLen:%d\n",pWriteVec[i].pIovBase, pWriteVec[i].ui32IovLen);
            }
            else
            {
                am_util_stdio_printf("data mismatch, write address:0x%x, read address:0x%x\n",pWriteVec[i].pIovBase, pReadVec[i].pIovBase);
                return false;
            }
        }
    }

    return true;
}

#endif