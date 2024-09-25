//*****************************************************************************
//
//! @file emmc_scatter_raw_read_write.c
//!
//! @brief emmc scatter raw read and write example.
//!
//! Purpose: This example demonstrates how to use ADMA based scatter IO APIs
//!          to read/write eMMC device.
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
// Copyright (c) 2024, Ambiq Micro, Inc.
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// 1. Redistributions of source code must retain the above copyright notice,
// this list of conditions and the following disclaimer.
//
// 2. Redistributions in binary form must reproduce the above copyright
// notice, this list of conditions and the following disclaimer in the
// documentation and/or other materials provided with the distribution.
//
// 3. Neither the name of the copyright holder nor the names of its
// contributors may be used to endorse or promote products derived from this
// software without specific prior written permission.
//
// Third party software included in this distribution is subject to the
// additional license terms as defined in the /docs/licenses directory.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
// ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
// LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
// CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
// SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
// INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
// CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
// ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
// POSSIBILITY OF SUCH DAMAGE.
//
// This is part of revision release_a5b_sdk2-748191cd0 of the AmbiqSuite Development Package.
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
#include "am_mcu_apollo.h"
#include "am_bsp.h"
#include "am_util.h"

#define CARD_INFOR_TEST
#define TEST_SSRAM
#define TEST_SCATTER_ADDRESS_SYNC_WRITE
#define TEST_SCATTER_ADDRESS_SYNC_READ
#define TEST_SCATTER_ADDRESS_ASYNC_WRITE
#define TEST_SCATTER_ADDRESS_ASYNC_READ
#define TEST_SCATTER_ADDRESS_SYNC_WRITE_READ
#define TEST_SCATTER_ADDRESS_ASYNC_WRITE_READ

#define SDIO_TEST_MODULE    0
#if SDIO_TEST_MODULE
#define SDIO_HOST   AM_HAL_SDHC_CARD_HOST1
#else
#define SDIO_HOST   AM_HAL_SDHC_CARD_HOST
#endif

#define DELAY_MAX_COUNT             0x100000

#define ALIGN(x) __attribute__((aligned(1 << x)))

#define IO_VECTOR_CNT   4
#define BLK_OFFSET      20
#define START_BLK       100
#define BLK_NUM         IO_VECTOR_CNT*8
#define BUF_LEN         512*BLK_NUM

#define DCACHE_SIZE 64*1024

uint8_t ui8RdBuf[BUF_LEN] ALIGN(12);
uint8_t ui8WrBuf[BUF_LEN] ALIGN(12);

uint8_t ui8Buf[BLK_NUM / IO_VECTOR_CNT * 512] ALIGN(12);

#if defined(TEST_SCATTER_ADDRESS_SYNC_WRITE_READ) || defined(TEST_SCATTER_ADDRESS_ASYNC_WRITE_READ)
uint8_t ui8ReadBuf[BLK_NUM / IO_VECTOR_CNT * 512] ALIGN(12);
am_hal_card_iovec_t IoRdVec[IO_VECTOR_CNT];
#endif

AM_SHARED_RW uint8_t ui8RdBufSSRAM[BUF_LEN] __attribute__((aligned(32)));   //algined 32 byte to match a cache line
AM_SHARED_RW uint8_t ui8WrBufSSRAM[BUF_LEN] __attribute__((aligned(32)));   //algined 32 byte to match a cache line

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
        bAsyncReadIsDone = true;
        am_util_stdio_printf("Last Read Xfered block %d\n", pEvt->ui32BlkCnt);

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
        bAsyncWriteIsDone = true;
        am_util_stdio_printf("Last Write Xfered block %d\n", pEvt->ui32BlkCnt);
    }

    if (AM_HAL_EVT_CARD_PRESENT == pEvt->eType)
    {
        am_util_stdio_printf("A card is inserted\n");
    }
}

bool check_if_data_match(uint8_t *pui8RdBuf, uint8_t *pui8WrBuf, uint32_t ui32Len)
{
    uint32_t i;
    for (i = 0; i < ui32Len; i++)
    {
        if (pui8RdBuf[i] != pui8WrBuf[i])
        {
            am_util_stdio_printf("pui8RdBuf[%d] = %d and pui8WrBuf[%d] = %d\n", i, pui8RdBuf[i], i, pui8WrBuf[i]);
            return false;
        }
    }

    if (i == ui32Len)
    {
        return true;
    }
    else
    {
        return false;
    }
}

am_hal_card_host_t *pSdhcCardHost = NULL;

//
// Take over the interrupt handler for whichever SDIO we're using.
//
#define emmc_sdio_isr                                                          \
    am_sdio_isr1(SDIO_TEST_MODULE)
#define am_sdio_isr1(n)                                                        \
    am_sdio_isr(n)
#define am_sdio_isr(n)                                                         \
    am_sdio ## n ## _isr

//*****************************************************************************
//
// SDIO ISRs.
//
//*****************************************************************************
void emmc_sdio_isr(void)
{
    uint32_t ui32IntStatus;

    am_hal_sdhc_intr_status_get(pSdhcCardHost->pHandle, &ui32IntStatus, true);
    am_hal_sdhc_intr_status_clear(pSdhcCardHost->pHandle, ui32IntStatus);
    am_hal_sdhc_interrupt_service(pSdhcCardHost->pHandle, ui32IntStatus);
}

int
main(void)
{
    am_hal_cachectrl_range_t sRange;

    //
    // Configure the board for low power.
    //
    am_bsp_low_power_init();

    //
    //  Enable the I-Cache and D-Cache.
    //
    am_hal_cachectrl_icache_enable();
    am_hal_cachectrl_dcache_enable(true);

    //
    // Configure SDIO PINs.
    //
    am_bsp_sdio_pins_enable(SDIO_TEST_MODULE, AM_HAL_HOST_BUS_WIDTH_8);

    //
    // Global interrupt enable
    //
    am_hal_interrupt_master_enable();

    //
    // Enable printing to the console.
    //
    am_bsp_itm_printf_enable();

    am_util_stdio_printf("\nApollo5 eMMC:%d scatter raw block read write example\n", SDIO_TEST_MODULE);

    //
    // initialize the test read and write buffers
    //
    for (int i = 0; i < BUF_LEN; i++)
    {
        ui8WrBufSSRAM[i] = i % 256;
        ui8WrBuf[i] = ~(i % 256);
        ui8RdBuf[i] = 0x0;
    }

    //
    // Get the uderlying SDHC card host instance
    //
    pSdhcCardHost = am_hal_get_card_host(SDIO_HOST, true);

    if (pSdhcCardHost == NULL)
    {
        am_util_stdio_printf("No such card host and stop\n");
        while(1);
    }
    am_util_stdio_printf("card host is found\n");

    am_hal_card_t eMMCard;

    am_hal_card_iovec_t IoVec[IO_VECTOR_CNT];

    //
    // check if card is present
    //
    while (am_hal_card_host_find_card(pSdhcCardHost, &eMMCard) != AM_HAL_STATUS_SUCCESS)
    {
        am_util_stdio_printf("No card is present now\n");
        am_util_delay_ms(1000);
        am_util_stdio_printf("Checking if card is available again\n");
    }

    while (am_hal_card_init(&eMMCard, AM_HAL_CARD_TYPE_EMMC, NULL, AM_HAL_CARD_PWR_CTRL_SDHC_OFF) != AM_HAL_STATUS_SUCCESS)
    {
        am_util_delay_ms(1000);
        am_util_stdio_printf("card init failed, try again\n");
    }

    //
    // 48MHz, 8-bit SDR mode for read and write
    //
    while (am_hal_card_cfg_set(&eMMCard, AM_HAL_CARD_TYPE_EMMC,
        AM_HAL_HOST_BUS_WIDTH_8, 48000000, AM_HAL_HOST_BUS_VOLTAGE_1_8,
        AM_HAL_HOST_UHS_NONE) != AM_HAL_STATUS_SUCCESS)
    {
        am_util_delay_ms(1000);
        am_util_stdio_printf("setting SDR50 failed\n");
    }

    //
    // Set transfer mode
    //
    am_hal_card_host_set_xfer_mode(pSdhcCardHost, AM_HAL_HOST_XFER_ADMA);

#ifdef CARD_INFOR_TEST
    //
    // Test the CID, CSD and EXT CSD parse function
    //
    am_util_stdio_printf("\n======== Test the CID, CSD and EXT CSD parse function ========\n");
    uint32_t emmc_psn;
    emmc_psn = am_hal_card_get_cid_field(&eMMCard, 16, 32);
    am_util_stdio_printf("Product Serial Number : 0x%x\n", emmc_psn);

    uint32_t emmc_csize;
    emmc_csize = am_hal_card_get_csd_field(&eMMCard,  62, 12);
    am_util_stdio_printf("Product CSD Size : 0x%x\n", emmc_csize);

    uint32_t max_enh_size_mult;
    uint32_t sec_count;
    max_enh_size_mult = am_hal_card_get_ext_csd_field(&eMMCard, 157, 3);
    sec_count = am_hal_card_get_ext_csd_field(&eMMCard, 212, 4);
    am_util_stdio_printf("Product EXT CSD Max Enh Size Multi : 0x%x\n", max_enh_size_mult);
    am_util_stdio_printf("Product EXT CSD Sector Count : 0x%x\n", sec_count);

    am_util_stdio_printf("============================================================\n\n");
#endif

#ifdef TEST_SSRAM
    //
    // write BLK_NUM*512 bytes to emmc flash
    //
    am_util_stdio_printf("\n======== Test multiple block sync write and read in SSRAM ========\n");
    am_hal_card_block_erase(&eMMCard, START_BLK, BLK_NUM, AM_HAL_ERASE, 100);

    for ( int i = 0; i < IO_VECTOR_CNT; i++ )
    {
        IoVec[i].pIovBase = ui8WrBufSSRAM + i * (BLK_NUM / IO_VECTOR_CNT * 512);
        IoVec[i].ui32IovLen = BLK_NUM / IO_VECTOR_CNT * 512;
    }

    //
    // Clean dcache data before write.
    //
    am_hal_cachectrl_dcache_clean(NULL);

    am_hal_emmc_card_scatter_write_sync(&eMMCard, START_BLK, IoVec, IO_VECTOR_CNT);

    memset((void *)ui8RdBufSSRAM, 0x0, BUF_LEN);

    am_hal_cachectrl_dcache_clean(NULL);

    for ( int i = 0; i < IO_VECTOR_CNT; i++ )
    {
        IoVec[i].pIovBase = ui8RdBufSSRAM + i * (BLK_NUM / IO_VECTOR_CNT * 512);
        IoVec[i].ui32IovLen = BLK_NUM / IO_VECTOR_CNT * 512;
    }

    am_hal_emmc_card_scatter_read_sync(&eMMCard, START_BLK, IoVec, IO_VECTOR_CNT);

    am_hal_cachectrl_dcache_invalidate(NULL, true);

    //
    // check if block data match or not
    //
    if (check_if_data_match((uint8_t *)ui8RdBufSSRAM, (uint8_t *)ui8WrBufSSRAM, BUF_LEN))
    {
        am_util_stdio_printf("data matched in 0x%x\n", ui8RdBufSSRAM);
    }
    else
    {
        am_util_stdio_printf("data mismatch in SSRAM\n");
        return false;
    }
    am_util_stdio_printf("============================================================\n\n");
#endif

#ifdef TEST_SCATTER_ADDRESS_SYNC_WRITE
    for ( int i = 0; i < BLK_NUM / IO_VECTOR_CNT * 512; i++)
    {
        ui8Buf[i] = rand() % 256;
    }

    am_util_stdio_printf("\n======== Test multiple block sync write with scatter buffer address========\n");
    am_hal_card_block_erase(&eMMCard, START_BLK, BLK_NUM, AM_HAL_ERASE, 100);

    for ( int i = 0; i < IO_VECTOR_CNT; i++ )
    {
        if ( i == 1 )
        {
            IoVec[i].pIovBase = ui8WrBuf;
        }
        else if ( i == 2 )
        {
            IoVec[i].pIovBase = ui8Buf;
        }
        else
        {
            IoVec[i].pIovBase = ui8WrBufSSRAM + i * (BLK_NUM / IO_VECTOR_CNT * 512);
        }

        IoVec[i].ui32IovLen = BLK_NUM / IO_VECTOR_CNT * 512;
    }

    //
    // Clean dcache data before write.
    //
    if ( BLK_NUM / IO_VECTOR_CNT * 512 > DCACHE_SIZE ) // if ssram buffer > 64K, clean all is efficient
    {
        am_hal_cachectrl_dcache_clean(NULL);
    }
    else
    {
        sRange.ui32StartAddr = (uint32_t)ui8WrBufSSRAM;
        sRange.ui32Size = BLK_NUM / IO_VECTOR_CNT * 512;
        am_hal_cachectrl_dcache_clean(&sRange);
    }

    am_hal_emmc_card_scatter_write_sync(&eMMCard, START_BLK, IoVec, IO_VECTOR_CNT);

    memset((void *)ui8RdBufSSRAM, 0x0, BUF_LEN);

    //
    // Clean dcache data if ssram buffer acessed by other master
    //
    if ( BUF_LEN > DCACHE_SIZE ) // if ssram buffer > 64K, clean all is efficient
    {
        am_hal_cachectrl_dcache_clean(NULL);
    }
    else
    {
        sRange.ui32StartAddr = (uint32_t)ui8RdBufSSRAM;
        sRange.ui32Size = BUF_LEN;
        am_hal_cachectrl_dcache_clean(&sRange);
    }

    am_hal_card_block_read_sync(&eMMCard, START_BLK, BLK_NUM, (uint8_t *)ui8RdBufSSRAM);

    //
    // Flush and invalidate cache, if ssram buffer > 64K, clean all is efficient
    //
    if ( BUF_LEN > DCACHE_SIZE )
    {
        am_hal_cachectrl_dcache_invalidate(NULL, true);
    }
    else
    {
        sRange.ui32StartAddr = (uint32_t)ui8RdBufSSRAM;
        sRange.ui32Size = BUF_LEN;
        am_hal_cachectrl_dcache_invalidate(&sRange, false);
    }

    //
    // check if data match or not in scatter buffer address
    //
    for ( int i = 0; i < IO_VECTOR_CNT; i++ )
    {
        if (check_if_data_match((uint8_t *)(ui8RdBufSSRAM + i * (BLK_NUM / IO_VECTOR_CNT * 512)), (uint8_t *)(IoVec[i].pIovBase), BLK_NUM / IO_VECTOR_CNT * 512))
        {
            am_util_stdio_printf("data matched, IovBase address = 0x%x, IovLen = %d\n", IoVec[i].pIovBase, IoVec[i].ui32IovLen);
        }
        else
        {
            am_util_stdio_printf("data mismatch in address: %x\n", IoVec[i].pIovBase);
            break;
        }
    }

    am_util_stdio_printf("============================================================\n\n");
#endif

#ifdef TEST_SCATTER_ADDRESS_SYNC_READ
    am_util_stdio_printf("\n======== Test multiple block sync read with scatter buffer address========\n");
    am_hal_card_block_erase(&eMMCard, START_BLK + BLK_OFFSET, BLK_NUM, AM_HAL_ERASE, 100);

    //
    // Clean dcache data before write.
    //
    if ( BUF_LEN > DCACHE_SIZE ) // if ssram buffer > 64K, clean all is efficient
    {
        am_hal_cachectrl_dcache_clean(NULL);
    }
    else
    {
        sRange.ui32StartAddr = (uint32_t)ui8WrBufSSRAM;
        sRange.ui32Size = BUF_LEN;
        am_hal_cachectrl_dcache_clean(&sRange);
    }

    am_hal_card_block_write_sync(&eMMCard, START_BLK + BLK_OFFSET, BLK_NUM, (uint8_t *)ui8WrBufSSRAM);

    for ( int i = 0; i < IO_VECTOR_CNT; i++ )
    {
        if ( i == 1 )
        {
            IoVec[i].pIovBase = ui8RdBuf;
        }
        else if ( i == 2 )
        {
            IoVec[i].pIovBase = ui8Buf;
        }
        else
        {
            IoVec[i].pIovBase = ui8RdBufSSRAM + i * (BLK_NUM / IO_VECTOR_CNT * 512);
        }

        IoVec[i].ui32IovLen = BLK_NUM / IO_VECTOR_CNT * 512;
    }

    memset((void *)ui8RdBufSSRAM, 0x0, BUF_LEN);
    memset((void *)ui8Buf, 0x0, IoVec[2].ui32IovLen);

    //
    // Clean dcache data if ssram buffer acessed by other master
    //
    if ( BUF_LEN > DCACHE_SIZE ) // if ssram buffer > 64K, clean all is efficient
    {
        am_hal_cachectrl_dcache_clean(NULL);
    }
    else
    {
        sRange.ui32StartAddr = (uint32_t)ui8RdBufSSRAM;
        sRange.ui32Size = BUF_LEN;
        am_hal_cachectrl_dcache_clean(&sRange);
    }

    am_hal_emmc_card_scatter_read_sync(&eMMCard, START_BLK + BLK_OFFSET, IoVec, IO_VECTOR_CNT);

    //
    // Flush and invalidate cache, if ssram buffer > 64K, clean all is efficient
    //
    if ( BUF_LEN > DCACHE_SIZE )
    {
        am_hal_cachectrl_dcache_invalidate(NULL, true);
    }
    else
    {
        sRange.ui32StartAddr = (uint32_t)ui8RdBufSSRAM;
        sRange.ui32Size = BUF_LEN;
        am_hal_cachectrl_dcache_invalidate(&sRange, false);
    }

    //
    // check data match or not in scatter buffer address
    //
    for ( int i = 0; i < IO_VECTOR_CNT; i++ )
    {
        if (check_if_data_match((uint8_t *)(IoVec[i].pIovBase), (uint8_t *)(ui8WrBufSSRAM + i * (BLK_NUM / IO_VECTOR_CNT * 512)), BLK_NUM / IO_VECTOR_CNT * 512))
        {
            am_util_stdio_printf("data matched, IovBase address = 0x%x, IovLen = %d\n", IoVec[i].pIovBase, IoVec[i].ui32IovLen);
        }
        else
        {
            am_util_stdio_printf("data mismatch in address: %x\n", IoVec[i].pIovBase);
            break;
        }
    }
#endif

#ifdef TEST_SCATTER_ADDRESS_SYNC_WRITE_READ
    for ( int i = 0; i < BLK_NUM / IO_VECTOR_CNT * 512; i++)
    {
        ui8Buf[i] = rand() % 256;
    }

    am_util_stdio_printf("\n======== Test multiple block sync write read with scatter buffer address========\n");
    am_hal_card_block_erase(&eMMCard, START_BLK, BLK_NUM, AM_HAL_ERASE, 100);

    for ( int i = 0; i < IO_VECTOR_CNT; i++ )
    {
        if ( i == 1 )
        {
            IoVec[i].pIovBase = ui8WrBuf;
        }
        else if ( i == 2 )
        {
            IoVec[i].pIovBase = ui8Buf;
        }
        else
        {
            IoVec[i].pIovBase = ui8WrBufSSRAM + i * (BLK_NUM / IO_VECTOR_CNT * 512);
        }

        IoVec[i].ui32IovLen = BLK_NUM / IO_VECTOR_CNT * 512;
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
        sRange.ui32StartAddr = (uint32_t)ui8WrBufSSRAM;
        sRange.ui32Size = BUF_LEN;
        am_hal_cachectrl_dcache_clean(&sRange);
    }

    am_hal_emmc_card_scatter_write_sync(&eMMCard, START_BLK, IoVec, IO_VECTOR_CNT);

    for ( int i = 0; i < IO_VECTOR_CNT; i++ )
    {
        if ( i == 1 )
        {
            IoRdVec[i].pIovBase = ui8RdBuf;
        }
        else if ( i == 2 )
        {
            IoRdVec[i].pIovBase = ui8ReadBuf;
        }
        else
        {
            IoRdVec[i].pIovBase = ui8RdBufSSRAM + i * (BLK_NUM / IO_VECTOR_CNT * 512);
        }

        IoRdVec[i].ui32IovLen = BLK_NUM / IO_VECTOR_CNT * 512;
    }

    memset((void *)ui8RdBufSSRAM, 0x0, BUF_LEN);
    memset((void *)ui8ReadBuf, 0x0, IoVec[2].ui32IovLen);

    //
    // Clean dcache data if ssram buffer acessed by other master
    //
    if ( BUF_LEN > DCACHE_SIZE ) // if ssram buffer > 64K, clean all is efficient
    {
        am_hal_cachectrl_dcache_clean(NULL);
    }
    else
    {
        sRange.ui32StartAddr = (uint32_t)ui8RdBufSSRAM;
        sRange.ui32Size = BUF_LEN;
        am_hal_cachectrl_dcache_clean(&sRange);
    }

    am_hal_emmc_card_scatter_read_sync(&eMMCard, START_BLK, IoRdVec, IO_VECTOR_CNT);

    //
    // Flush and invalidate cache, if ssram buffer > 64K, clean all is efficient
    //
    if ( BUF_LEN > DCACHE_SIZE )
    {
        am_hal_cachectrl_dcache_invalidate(NULL, true);
    }
    else
    {
        sRange.ui32StartAddr = (uint32_t)ui8RdBufSSRAM;
        sRange.ui32Size = BUF_LEN;
        am_hal_cachectrl_dcache_invalidate(&sRange, false);
    }

    //
    // check if data match or not in scatter buffer address
    //
    for ( int i = 0; i < IO_VECTOR_CNT; i++ )
    {
        if (check_if_data_match((uint8_t *)(IoRdVec[i].pIovBase), (uint8_t *)(IoVec[i].pIovBase), BLK_NUM / IO_VECTOR_CNT * 512))
        {
            am_util_stdio_printf("data matched, IovBase address = 0x%x, IovLen = %d\n", IoVec[i].pIovBase, IoVec[i].ui32IovLen);
        }
        else
        {
            am_util_stdio_printf("data mismatch in address: %x\n", IoVec[i].pIovBase);
            break;
        }
    }

    am_util_stdio_printf("============================================================\n\n");
#endif

#ifdef TEST_SCATTER_ADDRESS_ASYNC_WRITE
    //
    // async read & write, card insert & remove needs a callback function
    //
    for ( int i = 0; i < BLK_NUM / IO_VECTOR_CNT * 512; i++)
    {
        ui8Buf[i] = rand() % 256;
    }

    am_util_stdio_printf("\n======== Test multiple block async write with scatter buffer address========\n");

    am_hal_card_register_evt_callback(&eMMCard, am_hal_card_event_test_cb);

    am_hal_card_block_erase(&eMMCard, START_BLK, BLK_NUM, AM_HAL_ERASE, 100);

    //
    // Write 'BLK_NUM' blocks to the eMMC flash
    //
    bAsyncWriteIsDone = false;

    for ( int i = 0; i < IO_VECTOR_CNT; i++ )
    {
        if ( i == 1 )
        {
            IoVec[i].pIovBase = ui8WrBuf;
        }
        else if ( i == 2 )
        {
            IoVec[i].pIovBase = ui8Buf;
        }
        else
        {
            IoVec[i].pIovBase = ui8WrBufSSRAM + i * (BLK_NUM / IO_VECTOR_CNT * 512);
        }

        IoVec[i].ui32IovLen = BLK_NUM / IO_VECTOR_CNT * 512;
    }

    //
    // Clean dcache data before write.
    //
    if ( BLK_NUM / IO_VECTOR_CNT * 512 > DCACHE_SIZE ) // if ssram buffer > 64K, clean all is efficient
    {
        am_hal_cachectrl_dcache_clean(NULL);
    }
    else
    {
        sRange.ui32StartAddr = (uint32_t)ui8WrBufSSRAM;
        sRange.ui32Size = BUF_LEN;
        am_hal_cachectrl_dcache_clean(&sRange);
    }

    am_hal_emmc_card_scatter_write_async(&eMMCard, START_BLK, IoVec, IO_VECTOR_CNT);

    //
    // wait until the async write is done
    //
    while (!bAsyncWriteIsDone)
    {
        am_util_delay_ms(1000);
        am_util_stdio_printf("waiting scatter asynchronous write to complete\n");
    }

    am_util_stdio_printf("scatter asynchronous write is done\n");

    //
    // Read 'BLK_NUM' blocks to the eMMC flash
    //
    bAsyncReadIsDone = false;
    memset((void *)ui8RdBufSSRAM, 0x0, BUF_LEN);

    //
    // Clean dcache data if ssram buffer acessed by other master
    //
    if ( BUF_LEN > DCACHE_SIZE ) // if ssram buffer > 64K, clean all is efficient
    {
        am_hal_cachectrl_dcache_clean(NULL);
    }
    else
    {
        sRange.ui32StartAddr = (uint32_t)ui8RdBufSSRAM;
        sRange.ui32Size = BUF_LEN;
        am_hal_cachectrl_dcache_clean(&sRange);
    }

    am_hal_card_block_read_async(&eMMCard, START_BLK, BLK_NUM, (uint8_t *)ui8RdBufSSRAM);

    //
    // wait until the async read is done
    //
    while (!bAsyncReadIsDone)
    {
        am_util_delay_ms(1000);
        am_util_stdio_printf("waiting the asynchronous block read to complete\n");
    }

    //
    // check if data match or not in scatter buffer address
    //
    for ( int i = 0; i < IO_VECTOR_CNT; i++ )
    {
        if (check_if_data_match((uint8_t *)(ui8RdBufSSRAM + i * (BLK_NUM / IO_VECTOR_CNT * 512)), (uint8_t *)(IoVec[i].pIovBase), BLK_NUM / IO_VECTOR_CNT * 512))
        {
            am_util_stdio_printf("data matched, IovBase address = 0x%x, IovLen = %d\n", IoVec[i].pIovBase, IoVec[i].ui32IovLen);
        }
        else
        {
            am_util_stdio_printf("data mismatch at address: %x in Async mode\n", IoVec[i].pIovBase);
            break;
        }
    }
    am_util_stdio_printf("============================================================\n\n");
#endif

#ifdef TEST_SCATTER_ADDRESS_ASYNC_READ
    am_util_stdio_printf("\n======== Test multiple block async read with scatter buffer address========\n");

    am_hal_card_block_erase(&eMMCard, START_BLK + BLK_OFFSET, BLK_NUM, AM_HAL_ERASE, 100);

    //
    // Clean dcache data before write.
    //
    if ( BUF_LEN > DCACHE_SIZE ) // if ssram buffer > 64K, clean all is efficient
    {
        am_hal_cachectrl_dcache_clean(NULL);
    }
    else
    {
        sRange.ui32StartAddr = (uint32_t)ui8WrBufSSRAM;
        sRange.ui32Size = BUF_LEN;
        am_hal_cachectrl_dcache_clean(&sRange);
    }

    //
    // Write 'BLK_NUM' blocks to the eMMC flash
    //
    bAsyncWriteIsDone = false;
    am_hal_card_block_write_async(&eMMCard, START_BLK + BLK_OFFSET, BLK_NUM, (uint8_t *)ui8WrBufSSRAM);

    //
    // wait until the async write is done
    //
    while (!bAsyncWriteIsDone)
    {
        am_util_delay_ms(1000);
        am_util_stdio_printf("waiting the asynchronous block write to complete\n");
    }

    am_util_stdio_printf("asynchronous block write is done\n");

    bAsyncReadIsDone = false;

    for ( int i = 0; i < IO_VECTOR_CNT; i++ )
    {
        if ( i == 1 )
        {
            IoVec[i].pIovBase = ui8RdBuf;
        }
        else if ( i == 2 )
        {
            IoVec[i].pIovBase = ui8Buf;
        }
        else
        {
            IoVec[i].pIovBase = ui8RdBufSSRAM + i * (BLK_NUM / IO_VECTOR_CNT * 512);
        }

        IoVec[i].ui32IovLen = BLK_NUM / IO_VECTOR_CNT * 512;
    }

    memset((void *)ui8RdBufSSRAM, 0x0, BUF_LEN);
    memset((void *)ui8Buf, 0x0, IoVec[2].ui32IovLen);

    //
    // Clean dcache data if ssram buffer acessed by other master
    //
    if ( BUF_LEN > DCACHE_SIZE ) // if ssram buffer > 64K, clean all is efficient
    {
        am_hal_cachectrl_dcache_clean(NULL);
    }
    else
    {
        sRange.ui32StartAddr = (uint32_t)ui8RdBufSSRAM;
        sRange.ui32Size = BUF_LEN;
        am_hal_cachectrl_dcache_clean(&sRange);
    }

    am_hal_emmc_card_scatter_read_async(&eMMCard, START_BLK + BLK_OFFSET, IoVec, IO_VECTOR_CNT);

    //
    // Flush and invalidate cache, if ssram buffer > 64K, clean all is efficient
    //
    if ( BUF_LEN > DCACHE_SIZE )
    {
        am_hal_cachectrl_dcache_invalidate(NULL, true);
    }
    else
    {
        sRange.ui32StartAddr = (uint32_t)ui8RdBufSSRAM;
        sRange.ui32Size = BUF_LEN;
        am_hal_cachectrl_dcache_invalidate(&sRange, false);
    }

    //
    // wait until the async read is done
    //
    while (!bAsyncReadIsDone)
    {
        am_util_delay_ms(1000);
        am_util_stdio_printf("waiting scatter asynchronous read to complete\n");
    }

    //
    // check if data match or not in scatter buffer address
    //
    for ( int i = 0; i < IO_VECTOR_CNT; i++ )
    {
        if (check_if_data_match((uint8_t *)(IoVec[i].pIovBase), (uint8_t *)(ui8WrBufSSRAM + i * (BLK_NUM / IO_VECTOR_CNT * 512)), BLK_NUM / IO_VECTOR_CNT * 512))
        {
            am_util_stdio_printf("data matched, IovBase address = 0x%x, IovLen = %d\n", IoVec[i].pIovBase, IoVec[i].ui32IovLen);
        }
        else
        {
            am_util_stdio_printf("data mismatch at address: %x in Async mode\n", IoVec[i].pIovBase);
            break;
        }
    }

    am_util_stdio_printf("============================================================\n\n");
#endif

#ifdef TEST_SCATTER_ADDRESS_ASYNC_WRITE_READ

    for ( int i = 0; i < BLK_NUM / IO_VECTOR_CNT * 512; i++)
    {
        ui8Buf[i] = rand() % 256;
    }

    am_util_stdio_printf("\n======== Test multiple block async write read with scatter buffer address========\n");

    am_hal_card_block_erase(&eMMCard, START_BLK, BLK_NUM, AM_HAL_ERASE, 100);

    //
    // Write 'BLK_NUM' blocks to the eMMC flash
    //
    bAsyncWriteIsDone = false;

    for ( int i = 0; i < IO_VECTOR_CNT; i++ )
    {
        if ( i == 1 )
        {
            IoVec[i].pIovBase = ui8WrBuf;
        }
        else if ( i == 2 )
        {
            IoVec[i].pIovBase = ui8Buf;
        }
        else
        {
            IoVec[i].pIovBase = ui8WrBufSSRAM + i * (BLK_NUM / IO_VECTOR_CNT * 512);
        }

        IoVec[i].ui32IovLen = BLK_NUM / IO_VECTOR_CNT * 512;
    }

    //
    // Clean dcache data before write.
    //
    if ( BLK_NUM / IO_VECTOR_CNT * 512 > DCACHE_SIZE ) // if ssram buffer > 64K, clean all is efficient
    {
        am_hal_cachectrl_dcache_clean(NULL);
    }
    else
    {
        sRange.ui32StartAddr = (uint32_t)ui8WrBufSSRAM;
        sRange.ui32Size = BUF_LEN;
        am_hal_cachectrl_dcache_clean(&sRange);
    }

    am_hal_emmc_card_scatter_write_async(&eMMCard, START_BLK, IoVec, IO_VECTOR_CNT);

    //
    // wait until the async write is done
    //
    while (!bAsyncWriteIsDone)
    {
        am_util_delay_ms(1000);
        am_util_stdio_printf("waiting the scatter asynchronous write to complete\n");
    }

    am_util_stdio_printf("scatter asynchronous block write is done\n");

    bAsyncReadIsDone = false;

    for ( int i = 0; i < IO_VECTOR_CNT; i++ )
    {
        if ( i == 1 )
        {
            IoRdVec[i].pIovBase = ui8RdBuf;
        }
        else if ( i == 2 )
        {
            IoRdVec[i].pIovBase = ui8ReadBuf;
        }
        else
        {
            IoRdVec[i].pIovBase = ui8RdBufSSRAM + i * (BLK_NUM / IO_VECTOR_CNT * 512);
        }

        IoRdVec[i].ui32IovLen = BLK_NUM / IO_VECTOR_CNT * 512;
    }

    memset((void *)ui8RdBufSSRAM, 0x0, BUF_LEN);
    memset((void *)ui8ReadBuf, 0x0, IoRdVec[2].ui32IovLen);

    //
    // Clean dcache data if ssram buffer acessed by other master
    //
    if ( BUF_LEN > DCACHE_SIZE ) // if ssram buffer > 64K, clean all is efficient
    {
        am_hal_cachectrl_dcache_clean(NULL);
    }
    else
    {
        sRange.ui32StartAddr = (uint32_t)ui8RdBufSSRAM;
        sRange.ui32Size = BUF_LEN;
        am_hal_cachectrl_dcache_clean(&sRange);
    }

    am_hal_emmc_card_scatter_read_async(&eMMCard, START_BLK, IoRdVec, IO_VECTOR_CNT);

    //
    // Flush and invalidate cache, if ssram buffer > 64K, clean all is efficient
    //
    if ( BUF_LEN > DCACHE_SIZE )
    {
        am_hal_cachectrl_dcache_invalidate(NULL, true);
    }
    else
    {
        sRange.ui32StartAddr = (uint32_t)ui8RdBufSSRAM;
        sRange.ui32Size = BUF_LEN;
        am_hal_cachectrl_dcache_invalidate(&sRange, false);
    }

    //
    // wait until the async read is done
    //
    while (!bAsyncReadIsDone)
    {
        am_util_delay_ms(1000);
        am_util_stdio_printf("waiting the scatter asynchronous read to complete\n");
    }

    am_util_stdio_printf("scatter asynchronous read is done\n");

    //
    // check if data match or not in scatter buffer address
    //
    for ( int i = 0; i < IO_VECTOR_CNT; i++ )
    {
        if (check_if_data_match((uint8_t *)(IoRdVec[i].pIovBase), (uint8_t *)(IoVec[i].pIovBase), BLK_NUM / IO_VECTOR_CNT * 512))
        {
            am_util_stdio_printf("data matched, IovBase address = 0x%x, IovLen = %d\n", IoVec[i].pIovBase, IoVec[i].ui32IovLen);
        }
        else
        {
            am_util_stdio_printf("data mismatch in address: %x\n", IoVec[i].pIovBase);
            break;
        }
    }

    am_util_stdio_printf("============================================================\n\n");
#endif

    //
    // End banner.
    //
    am_util_stdio_printf("\nApollo5 eMMC:%d scatter raw block read write example complete\n", SDIO_TEST_MODULE);

    while (1);
}

