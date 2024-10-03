/*----------------------------------------------------------------------------/
/  FatFs - Generic FAT Filesystem Module  R0.14                               /
/-----------------------------------------------------------------------------/
/
/ Copyright (C) 2020, ChaN, all right reserved.
/
/ FatFs module is an open source software. Redistribution and use of FatFs in
/ source and binary forms, with or without modification, are permitted provided
/ that the following condition is met:
/
/ 1. Redistributions of source code must retain the above copyright notice,
/    this condition and the following disclaimer.
/
/ This software is provided by the copyright holder and contributors "AS IS"
/ and any warranties related to this software are DISCLAIMED.
/ The copyright owner or contributors be NOT LIABLE for any damages caused
/ by use of this software.
/
/----------------------------------------------------------------------------*/

//*****************************************************************************
//
// ${copyright}
//
// This is part of revision ${version} of the AmbiqSuite Development Package.
//
//*****************************************************************************

#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>

#include "am_bsp.h"
#include "am_util.h"
#include "mmc_apollo4.h"

/*--------------------------------------------------------------------------

   Module Global variable

---------------------------------------------------------------------------*/

am_hal_card_t SDcard;
void am_hal_card_event_test_cb(am_hal_host_evt_t *pEvt);

am_device_card_config_t     g_SDcardCfg =
{
    .eHost                  = AM_HAL_SDHC_CARD_HOST,
    .eXferMode              = AM_HAL_HOST_XFER_ADMA,
    .ui32Clock              = 48000000,
    .eBusWidth              = AM_HAL_HOST_BUS_WIDTH_4,
    .eBusVoltage            = AM_HAL_HOST_BUS_VOLTAGE_3_3,
    .eUHSMode               = AM_HAL_HOST_UHS_SDR50,
    .bAsync                 = false,

    .eCardType              = AM_HAL_CARD_TYPE_SDHC,
    .eCardPwrCtrlPolicy     = AM_HAL_CARD_PWR_CTRL_NONE,
    .pCardPwrCtrlFunc       = NULL,
    .pfunCallback           = am_hal_card_event_test_cb,
    .ui32SectorCount        = 0x100,
};

am_device_card_hw_t   g_SDcardHw =
{
    .pSdhcHost = NULL,
    .pDevHandle = &SDcard,
    .dStat = STA_NOINIT,
    .bAsyncReadIsDone = false,
    .bAsyncWriteIsDone = false,
};

/*--------------------------------------------------------------------------

   Module Private Functions

---------------------------------------------------------------------------*/

//*****************************************************************************
//
// Interrupt handler for SDIO
//
//*****************************************************************************
void am_sdio_isr(void)
{
    uint32_t ui32IntStatus;

    am_hal_sdhc_intr_status_get(g_SDcardHw.pSdhcHost->pHandle, &ui32IntStatus, true);
    am_hal_sdhc_intr_status_clear(g_SDcardHw.pSdhcHost->pHandle, ui32IntStatus);
    am_hal_sdhc_interrupt_service(g_SDcardHw.pSdhcHost->pHandle, ui32IntStatus);
}

//*****************************************************************************
//
// Card event call back function
//
//*****************************************************************************
void am_hal_card_event_test_cb(am_hal_host_evt_t *pEvt)
{
    am_hal_card_host_t *pHost = (am_hal_card_host_t *)pEvt->pCtx;

    if (AM_HAL_EVT_XFER_COMPLETE == pEvt->eType &&
        pHost->AsyncCmdData.dir == AM_HAL_DATA_DIR_READ)
    {
        g_SDcardHw.bAsyncReadIsDone = true;
        am_util_debug_printf("Last Read Xfered block %d\n", pEvt->ui32BlkCnt);
    }

    if (AM_HAL_EVT_XFER_COMPLETE == pEvt->eType &&
        pHost->AsyncCmdData.dir == AM_HAL_DATA_DIR_WRITE)
    {
        g_SDcardHw.bAsyncWriteIsDone = true;
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

    if (AM_HAL_EVT_CARD_PRESENT == pEvt->eType)
    {
        am_util_debug_printf("A card is inserted\n");
    }
}

//
// Register SD Card power cycle function for card power on/off/reset,
// Usually used for switching to 3.3V signaling in sd card initialization
//
uint32_t sd_card_power_config(am_hal_card_pwr_e eCardPwr)
{
    am_hal_gpio_pinconfig(AM_BSP_GPIO_SD_POWER_CTRL, am_hal_gpio_pincfg_output);

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

/*--------------------------------------------------------------------------

   Public Functions

---------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------*/
/* Initialize Disk Drive                                                 */
/*-----------------------------------------------------------------------*/

DSTATUS mmc_disk_initialize (void)
{
    DWORD i = 0;

    if ( g_SDcardHw.dStat == STA_NOINIT )
    {
        //
        // Get the SDHC card host instance
        //
        g_SDcardHw.pSdhcHost = am_hal_get_card_host(g_SDcardCfg.eHost, true);

        if (g_SDcardHw.pSdhcHost == NULL)
        {
            am_util_debug_printf("Error: No card host found!\n");
            g_SDcardHw.dStat |= STA_NOINIT;
            return g_SDcardHw.dStat;
        }

        am_util_debug_printf("card host is found\n");

        //
        // check if card is present
        //
        while (am_hal_card_host_find_card(g_SDcardHw.pSdhcHost, g_SDcardHw.pDevHandle) != AM_HAL_STATUS_SUCCESS)
        {
            am_util_debug_printf("No card is present now\n");
            am_util_delay_ms(10);
            am_util_debug_printf("Checking if card is available again\n");
            i++;
            if ( i ==  100 )
            {
                g_SDcardHw.dStat |= STA_NOINIT;
                return g_SDcardHw.dStat;
            }
        }

        //
        // Initialize the card
        //
        i = 0;
        while (am_hal_card_init(g_SDcardHw.pDevHandle, AM_HAL_CARD_TYPE_SDHC, sd_card_power_config, AM_HAL_CARD_PWR_CTRL_SDHC_OFF) != AM_HAL_STATUS_SUCCESS)
        {
            am_util_delay_ms(10);
            am_util_debug_printf("card and host is not ready, try again\n");
            i++;
            if ( i ==  100 )
            {
                g_SDcardHw.dStat |= STA_NOINIT;
                return g_SDcardHw.dStat;
            }
        }

        //
        // Set ADMA transfer mode
        //
        am_hal_card_host_set_xfer_mode(g_SDcardHw.pSdhcHost, AM_HAL_HOST_XFER_ADMA);

        //
        // async read & write, card insert & remove needs a callback function
        //
        if ( g_SDcardCfg.bAsync )
        {
            am_hal_card_register_evt_callback(g_SDcardHw.pDevHandle, g_SDcardCfg.pfunCallback);
        }

        //
        // Get the sector count from SD card
        //
        g_SDcardCfg.ui32SectorCount = am_hal_sd_card_get_block_count(g_SDcardHw.pDevHandle);

        if ( g_SDcardCfg.ui32SectorCount == 0 )
        {
            am_util_debug_printf("Fail to get the sector counter of Card\n");
            g_SDcardHw.dStat |= STA_NOINIT;
            return g_SDcardHw.dStat;
        }
        else
        {
            am_util_debug_printf("Card EXT CSD Sector Count is: 0x%x\n", g_SDcardCfg.ui32SectorCount);
        }

        // Configure the SDIO host
        //
        if ( AM_HAL_STATUS_SUCCESS != am_hal_card_cfg_set(g_SDcardHw.pDevHandle, g_SDcardCfg.eCardType,
                                                        g_SDcardCfg.eBusWidth,
                                                        g_SDcardCfg.ui32Clock,
                                                        g_SDcardCfg.eBusVoltage,
                                                        g_SDcardCfg.eUHSMode) )
        {
            am_util_debug_printf("Fail to configure the SDIO host\n");
            g_SDcardHw.dStat |= STA_NOINIT;
            return g_SDcardHw.dStat;
        }

#ifdef ENABLE_SDIO_POWER_SAVE
        //
        // power down SDIO peripheral
        //
        if ( am_hal_card_pwrctrl_sleep(g_SDcardHw.pDevHandle) != AM_HAL_STATUS_SUCCESS )
        {
            am_util_debug_printf("Fail to power down SDIO peripheral\n");
            g_SDcardHw.dStat |= STA_NOINIT;
            return g_SDcardHw.dStat;
        }
        else
        {
            am_util_debug_printf("Power down SDIO peripheral\n");
        }
#endif

        g_SDcardHw.dStat &= ~STA_NOINIT;
    }

    return g_SDcardHw.dStat;
}


/*-----------------------------------------------------------------------*/
/* Get Disk Status                                                       */
/*-----------------------------------------------------------------------*/

DSTATUS mmc_disk_status (void)
{
    return g_SDcardHw.dStat;
}


/*-----------------------------------------------------------------------*/
/* Read Sector(s)                                                        */
/*-----------------------------------------------------------------------*/

DRESULT mmc_disk_read (
    BYTE *buff,            /* Pointer to the data buffer to store read data */
    LBA_t sector,                /* Start sector number (LBA) */
    UINT count            /* Sector count (1..128) */
)
{
    DWORD i = 0;
    DWORD ui32Status;

    if ( !count )
    {
        return RES_PARERR;
    }

    if (g_SDcardHw.dStat & STA_NOINIT)
    {
        return RES_NOTRDY;
    }

#ifdef ENABLE_SDIO_POWER_SAVE
    //
    // power up SDIO peripheral
    //
    if ( am_hal_card_pwrctrl_wakeup(g_SDcardHw.pDevHandle) != AM_HAL_STATUS_SUCCESS )
    {
        am_util_debug_printf("Fail to power up SDIO peripheral\n");
        return RES_ERROR;
    }
    else
    {
        am_util_debug_printf("Power up SDIO peripheral\n");
    }
#endif

    am_util_debug_printf("Read Data\n");
    am_util_debug_printf("count = %d  sector= %d\n", count, sector);

    //
    // Calling Read API
    //
    if ( !g_SDcardCfg.bAsync )
    {
        ui32Status = am_hal_sd_card_block_read_sync(g_SDcardHw.pDevHandle, sector, count, (uint8_t *)buff);
        if ( (ui32Status & 0xFFFF) != AM_HAL_STATUS_SUCCESS )
        {
            am_util_debug_printf("Fail to call read API. number of bytes read = %d\n", ui32Status);
            return RES_ERROR;
        }
    }
    else
    {
        g_SDcardHw.bAsyncReadIsDone = false;

        ui32Status = am_hal_sd_card_block_read_async(g_SDcardHw.pDevHandle, sector, count, (uint8_t *)buff);
        if ( ui32Status != AM_HAL_STATUS_SUCCESS )
        {
            am_util_debug_printf("Fail to call read API. Read Status = %d\n", ui32Status);
            return RES_ERROR;
        }

        //
        // wait until the async read is done
        //
        while (!g_SDcardHw.bAsyncReadIsDone)
        {
            am_util_delay_ms(1);
            i++;
            if ( i == 1000 )
            {
                am_util_debug_printf("Read Timeout\n");
                return RES_ERROR;
            }
        }

    }

    am_util_debug_printf("Read Completed \n");

#ifdef ENABLE_SDIO_POWER_SAVE
    //
    // power down SDIO peripheral
    //
    if ( am_hal_card_pwrctrl_sleep(g_SDcardHw.pDevHandle) != AM_HAL_STATUS_SUCCESS )
    {
        am_util_debug_printf("Fail to power down SDIO peripheral\n");
        return RES_ERROR;
    }
    else
    {
        am_util_debug_printf("Power down SDIO peripheral\n");
    }
#endif

    return RES_OK;
}


/*-----------------------------------------------------------------------*/
/* Write Sector(s)                                                       */
/*-----------------------------------------------------------------------*/

DRESULT mmc_disk_write (
    const BYTE *buff,    /* Pointer to the data to be written */
    LBA_t sector,        /* Start sector number (LBA) */
    UINT count        /* Sector count (1..128) */
)
{
    BYTE * wr_buf;
    DWORD i = 0;
    DWORD ui32Status;

    if ( !count )
    {
        return RES_PARERR;
    }

    if ( g_SDcardHw.dStat & STA_NOINIT )
    {
        return RES_NOTRDY;
    }

#ifdef ENABLE_SDIO_POWER_SAVE
    //
    // power up SDIO peripheral
    //
    if ( am_hal_card_pwrctrl_wakeup(g_SDcardHw.pDevHandle) != AM_HAL_STATUS_SUCCESS )
    {
        am_util_debug_printf("Fail to power up SDIO peripheral\n");
        return RES_ERROR;
    }
    else
    {
        am_util_debug_printf("Power up SDIO peripheral\n");
    }
#endif

    am_util_debug_printf("Write Data\n");
    am_util_debug_printf("count = %d  sector= %d\n", count, sector);

    wr_buf = (BYTE *)buff;

    //
    // Calling Write API
    //
    if ( !g_SDcardCfg.bAsync )
    {
        ui32Status = am_hal_sd_card_block_write_sync(g_SDcardHw.pDevHandle, sector, count, (uint8_t *)wr_buf);
        if ( (ui32Status & 0xFFFF) != AM_HAL_STATUS_SUCCESS )
        {
            am_util_debug_printf("Fail to call read API. number of bytes read = %d\n", ui32Status);
            return RES_ERROR;
        }
    }
    else
    {
        g_SDcardHw.bAsyncWriteIsDone = false;

        ui32Status = am_hal_sd_card_block_write_async(g_SDcardHw.pDevHandle, sector, count, (uint8_t *)wr_buf);
        if ( ui32Status != AM_HAL_STATUS_SUCCESS )
        {
            am_util_debug_printf("Fail to call write API. Write Status = %d\n", ui32Status);
            return RES_ERROR;
        }

        //
        // wait until the async write is done
        //
        while (!g_SDcardHw.bAsyncWriteIsDone)
        {
            am_util_delay_ms(1);
            i++;
            if ( i == 1000 )
            {
                am_util_debug_printf("Write Timeout\n");
                return RES_ERROR;
            }
        }
    }
    am_util_debug_printf("Write Completed \n");

#ifdef ENABLE_SDIO_POWER_SAVE
    //
    // power down SDIO peripheral
    //
    if ( am_hal_card_pwrctrl_sleep(g_SDcardHw.pDevHandle) != AM_HAL_STATUS_SUCCESS )
    {
        am_util_debug_printf("Fail to power down SDIO peripheral\n");
        return RES_ERROR;
    }
    else
    {
        am_util_debug_printf("Power down SDIO peripheral\n");
    }
#endif

    return RES_OK;
}

/*-----------------------------------------------------------------------*/
/* Miscellaneous Functions                                               */
/*-----------------------------------------------------------------------*/

DRESULT mmc_disk_ioctl (
    BYTE cmd,        /* Control code */
    void *buff        /* Buffer to send/receive control data */
)
{
       DRESULT res = RES_ERROR;

#if FF_USE_TRIM
    LBA_t *range;
    DWORD st, ed;
#endif
#if _USE_ISDIO
    SDIO_CTRL *sdi;
    BYTE rc, *bp;
    UINT dc;
#endif

    if (g_SDcardHw.dStat & STA_NOINIT)
    {
        return RES_NOTRDY;
    }

    switch (cmd)
    {
        /* Make sure that no pending write process */
        case CTRL_SYNC :
            res = RES_OK;
            break;

        /* Get number of sectors on the disk (DWORD) */
        case GET_SECTOR_COUNT :
            *(DWORD*)buff = g_SDcardCfg.ui32SectorCount;
            res = RES_OK;
            break;

        /* Get erase block size in unit of sector (DWORD) */
        case GET_BLOCK_SIZE :
            *(DWORD*)buff = 1;
            res = RES_OK;
            break;

        default:
            res = RES_PARERR;
    }

    return res;
}

