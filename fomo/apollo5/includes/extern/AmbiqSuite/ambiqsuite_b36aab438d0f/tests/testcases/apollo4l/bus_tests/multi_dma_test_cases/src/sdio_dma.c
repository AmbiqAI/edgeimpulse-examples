//*****************************************************************************
//
//! @file sdio_dma.c
//!
//! @brief emmc block read and write example.
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

#include "multi_dma.h"

//#define FMC_BOARD_EMMC_TEST
#define TEST_POWER_SAVING
//#define EMMC_TASK_DEBUG_LOG

#define ALIGN(x) __attribute__((aligned(1 << x)))

#define START_BLK 3000
#define BLK_NUM 4
#define BUF_LEN 512*BLK_NUM

am_hal_card_t eMMCard;

volatile uint8_t ui8RdBuf[BUF_LEN] ALIGN(12);
volatile uint8_t ui8WrBuf[BUF_LEN] ALIGN(12);

volatile bool bAsyncWriteIsDone = true;
volatile bool bAsyncReadIsDone  = true;

void am_hal_card_event_test_cb(am_hal_host_evt_t *pEvt)
{
    am_hal_card_host_t *pHost = (am_hal_card_host_t *)pEvt->pCtx;

    if (AM_HAL_EVT_XFER_COMPLETE == pEvt->eType &&
        pHost->AsyncCmdData.dir == AM_HAL_DATA_DIR_READ)
    {
        bAsyncReadIsDone = true;
        am_hal_gpio_output_toggle(DEBUG_PIN_SDIO);
        //am_util_stdio_printf("Last Read Xfered block %d\n", pEvt->ui32BlkCnt);
    }

    if (AM_HAL_EVT_XFER_COMPLETE == pEvt->eType &&
        pHost->AsyncCmdData.dir == AM_HAL_DATA_DIR_WRITE)
    {
        bAsyncWriteIsDone = true;
        am_hal_gpio_output_toggle(DEBUG_PIN_SDIO);
        //am_util_stdio_printf("Last Write Xfered block %d\n", pEvt->ui32BlkCnt);
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
            am_util_stdio_printf("EMMC error: pui8RdBuf[%d] = %d and pui8WrBuf[%d] = %d\n", i, pui8RdBuf[i], i, pui8WrBuf[i]);
            break;
        }
    }

    if (i == ui32Len)
    {
        am_util_stdio_printf("EMMC\n");
    }

}

am_hal_card_host_t *pSdhcCardHost = NULL;

void am_sdio_isr(void)
{
    uint32_t ui32IntStatus;

    am_hal_sdhc_intr_status_get(pSdhcCardHost->pHandle, &ui32IntStatus, true);
    am_hal_sdhc_intr_status_clear(pSdhcCardHost->pHandle, ui32IntStatus);
    am_hal_sdhc_interrupt_service(pSdhcCardHost->pHandle, ui32IntStatus);
}


bool
sdio_init()
{
    //
    // Configure SDIO PINs.
    //
    am_bsp_sdio_pins_enable(AM_HAL_HOST_BUS_WIDTH_8);

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
        return false;
        //vTaskSuspend(NULL);
    }

    am_util_stdio_printf("card host is found\n");

    //
    // check if card is present
    //
    while (am_hal_card_host_find_card(pSdhcCardHost, &eMMCard) != AM_HAL_STATUS_SUCCESS)
    {
        am_util_stdio_printf("No card is present now\n");
        am_util_stdio_printf("Checking if card is available again\n");
        return false;
    }

    while (am_hal_card_init(&eMMCard, AM_HAL_CARD_TYPE_EMMC, NULL, AM_HAL_CARD_PWR_CTRL_SDHC_OFF) == AM_HAL_STATUS_FAIL)
    {
        am_util_stdio_printf("card and host is not ready, try again\n");
        return false;
    }

    //
    // set the card type to eMMC by using 48MHz and 8-bit mode for read and write
    //
    if( AM_HAL_STATUS_SUCCESS != am_hal_card_cfg_set(&eMMCard, AM_HAL_CARD_TYPE_EMMC,
        AM_HAL_HOST_BUS_WIDTH_8, 96000000, AM_HAL_HOST_BUS_VOLTAGE_1_8,
        AM_HAL_HOST_UHS_NONE))
    {
        return false;
    }
    //
    // async read & write, card insert & remove needs a callback function
    //
    am_util_stdio_printf("========  Test multiple block async write and read  ========\n");
    am_hal_card_register_evt_callback(&eMMCard, am_hal_card_event_test_cb);
    NVIC_EnableIRQ(SDIO_IRQn);
    return true;
}

bool
sdio_write()
{
    //
    // write 512 bytes to emmc flash
    //
    if(bAsyncWriteIsDone == true)
    {
        bAsyncWriteIsDone = false;
        am_hal_card_block_write_async(&eMMCard, START_BLK, 1, (uint8_t *)ui8WrBuf);
    }
    else
    {
        //am_util_stdio_printf("sdio is busy.\n");
    }

    //am_hal_card_pwrctrl_sleep(&eMMCard);
    return true;
}

bool
sdio_read()
{
    //am_hal_card_pwrctrl_wakeup(&eMMCard);
    //
    // read back the first block of emmc flash
    //
    if(bAsyncReadIsDone == true)
    {
        bAsyncReadIsDone = false;
        memset((void *)ui8RdBuf, 0x0, BUF_LEN);

        am_hal_card_block_read_async(&eMMCard, START_BLK, 1, (uint8_t *)ui8RdBuf);
    }
    else
    {
        //am_util_stdio_printf("sdio is busy.\n");
    }

    //
    // check if block data match or not
    //
    //check_if_data_match((uint8_t *)ui8RdBuf, (uint8_t *)ui8WrBuf, 512);
    return true;
}

