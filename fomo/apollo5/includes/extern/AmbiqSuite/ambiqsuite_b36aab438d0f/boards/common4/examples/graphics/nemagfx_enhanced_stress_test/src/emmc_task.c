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

#include "nemagfx_enhanced_stress_test.h"

#if  ( EMMC_TASK_ENABLE == 1 )

//#define FMC_BOARD_EMMC_TEST
#define TEST_POWER_SAVING
//#define EMMC_TASK_DEBUG_LOG

#define ALIGN(x) __attribute__((aligned(1 << x)))

#define START_BLK 3000
#define BLK_NUM 4
#define BUF_LEN 512*BLK_NUM

TaskHandle_t emmc_task_handle;

volatile uint8_t ui8RdBuf[BUF_LEN] ALIGN(12);
volatile uint8_t ui8WrBuf[BUF_LEN] ALIGN(12);

volatile bool bAsyncWriteIsDone = false;
volatile bool bAsyncReadIsDone  = false;

void am_hal_card_event_test_cb(am_hal_host_evt_t *pEvt)
{
    am_hal_card_host_t *pHost = (am_hal_card_host_t *)pEvt->pCtx;

    if (AM_HAL_EVT_XFER_COMPLETE == pEvt->eType &&
        pHost->AsyncCmdData.dir == AM_HAL_DATA_DIR_READ)
    {
        bAsyncReadIsDone = true;
        am_util_stdio_printf("Last Read Xfered block %d\n", pEvt->ui32BlkCnt);
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

void am_sdio_isr(void)
{
    uint32_t ui32IntStatus;

    am_hal_sdhc_intr_status_get(pSdhcCardHost->pHandle, &ui32IntStatus, true);
    am_hal_sdhc_intr_status_clear(pSdhcCardHost->pHandle, ui32IntStatus);
    am_hal_sdhc_interrupt_service(pSdhcCardHost->pHandle, ui32IntStatus);
}


void EmmcTask(void *pvParameters)
{
    //
    // Configure SDIO PINs.
    //
    am_bsp_sdio_pins_enable(SDIO_BUS_WIDTH);

#ifdef FMC_BOARD_EMMC_TEST
    //
    // FPGA level shift control
    //
    am_hal_gpio_pinconfig(76, am_hal_gpio_pincfg_output);
    am_hal_gpio_output_clear(76);
#endif


#if 0
    //
    // Enable below code if we need to find a proper TX/RX delay settings
    // you need to #undef SDIO_DLY in the 'am_hal_sdhc.c'
    //
    uint8_t ui8TxRxDelays[2];
    am_hal_card_emmc_ddr50_calibrate(SDIO_BUS_WIDTH,
        (uint8_t *)ui8WrBuf, START_BLK, 2, ui8TxRxDelays);

    am_util_stdio_printf("SDIO TX delay - %d, RX Delay - %d\n", ui8TxRxDelays[0], ui8TxRxDelays[1]);
#endif

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
        SDIO_BUS_WIDTH, 48000000, AM_HAL_HOST_BUS_VOLTAGE_1_8,
        AM_HAL_HOST_UHS_SDR50);


    for ( ; ; )
    {

#if 0
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

        //
        // write 512 bytes to emmc flash
        //
        am_hal_card_block_write_sync(&eMMCard, START_BLK, 1, (uint8_t *)ui8WrBuf);

#ifdef TEST_POWER_SAVING

        //
        // Test the power saving feature
        //
#ifdef  EMMC_TASK_DEBUG_LOG
        am_util_stdio_printf("\nCard power saving\n");
#endif
        am_hal_card_pwrctrl_sleep(&eMMCard);
        vTaskDelay(1000);
        am_hal_card_pwrctrl_wakeup(&eMMCard);

#ifdef  EMMC_TASK_DEBUG_LOG
        am_util_stdio_printf("\nCard power wakeup\n");
#endif
#endif

#ifdef  EMMC_TASK_DEBUG_LOG
        am_util_stdio_printf("\nRead 512 bytes block\n");
#endif
        //
        // read back the first block of emmc flash
        //
        memset((void *)ui8RdBuf, 0x0, BUF_LEN);


        am_hal_card_block_read_sync(&eMMCard, START_BLK, 1, (uint8_t *)ui8RdBuf);

        //
        // check if block data match or not
        //
        if (check_if_data_match((uint8_t *)ui8RdBuf, (uint8_t *)ui8WrBuf, 512) == 0)
        {
            am_util_stdio_printf("{TASK] : EMMC\n");
        }


#if 0
        am_hal_card_cfg_set(&eMMCard, AM_HAL_CARD_TYPE_EMMC,
            SDIO_BUS_WIDTH, 48000000, AM_HAL_HOST_BUS_VOLTAGE_1_8,
            AM_HAL_HOST_UHS_DDR50);

        am_util_debug_printf("\n======== Test multiple block sync write and read ========\n");

        uint32_t ui32Status;
        ui32Status = am_hal_card_block_write_sync(&eMMCard, START_BLK, BLK_NUM, (uint8_t *)ui8WrBuf);
        am_util_stdio_printf("Synchronous Writing %d blocks is done, Xfer Status %d\n", ui32Status >> 16, ui32Status & 0xffff);

#ifdef TEST_POWER_SAVING

        //
        // Test the power saving feature
        //
        am_util_stdio_printf("\nCard power saving\n");
        am_hal_card_pwrctrl_sleep(&eMMCard);
        vTaskDelay(10);
        am_util_stdio_printf("\nCard power wakeup\n");
        am_hal_card_pwrctrl_wakeup(&eMMCard);
#endif

        //
        // read back the first block of emmc flash
        //
        memset((void *)ui8RdBuf, 0x0, BUF_LEN);
        ui32Status = am_hal_card_block_read_sync(&eMMCard, START_BLK, BLK_NUM, (uint8_t *)ui8RdBuf);
        am_util_stdio_printf("Synchronous Reading %d blocks is done, Xfer Status %d\n", ui32Status >> 16, ui32Status & 0xffff);

        //
        // check if block data match or not
        //
        check_if_data_match((uint8_t *)ui8RdBuf, (uint8_t *)ui8WrBuf, BUF_LEN);

        am_util_stdio_printf("============================================================\n\n");

#endif

#if 0
        // am_hal_card_host_set_xfer_mode(pSdhcCardHost, AM_HAL_HOST_XFER_ADMA);

        am_hal_card_cfg_set(&eMMCard, AM_HAL_CARD_TYPE_EMMC,
            SDIO_BUS_WIDTH, 48000000, AM_HAL_HOST_BUS_VOLTAGE_1_8,
            AM_HAL_HOST_UHS_NONE);

        //
        // async read & write, card insert & remove needs a callback function
        //
        am_util_debug_printf("========  Test multiple block async write and read  ========\n");
        am_hal_card_register_evt_callback(&eMMCard, am_hal_card_event_test_cb);

        //
        // Write 'BLK_NUM' blocks to the eMMC flash
        //
        bAsyncWriteIsDone = false;
        am_hal_card_block_write_async(&eMMCard, START_BLK, BLK_NUM, (uint8_t *)ui8WrBuf);

        //
        // wait until the async write is done
        //
        while (!bAsyncWriteIsDone)
        {
            vTaskDelay(1000);
            am_util_stdio_printf("waiting the asynchronous block write to complete\n");
        }

        am_util_stdio_printf("asynchronous block write is done\n");


#ifdef TEST_POWER_SAVING
        //
        // Test the power saving feature
        //

        am_util_stdio_printf("\nCard power saving\n");
        am_hal_card_pwrctrl_sleep(&eMMCard);
        vTaskDelay(10);
        am_util_stdio_printf("\nCard power wakeup\n");
        am_hal_card_pwrctrl_wakeup(&eMMCard);

#endif
        //
        // Read 'BLK_NUM' blocks to the eMMC flash
        //
        bAsyncReadIsDone = false;
        memset((void *)ui8RdBuf, 0x0, BUF_LEN);
        am_hal_card_block_read_async(&eMMCard, START_BLK, BLK_NUM, (uint8_t *)ui8RdBuf);

        //
        // wait until the async read is done
        //
        while (!bAsyncReadIsDone)
        {
            vTaskDelay(1000);
            am_util_stdio_printf("waiting the asynchronous block read to complete\n");
        }

        check_if_data_match((uint8_t *)ui8RdBuf, (uint8_t *)ui8WrBuf, BUF_LEN);

        am_util_stdio_printf("============================================================\n\n");

#endif

    } // End of for loop

}

#endif  //( EMMC_TASK_ENABLE == 1 )