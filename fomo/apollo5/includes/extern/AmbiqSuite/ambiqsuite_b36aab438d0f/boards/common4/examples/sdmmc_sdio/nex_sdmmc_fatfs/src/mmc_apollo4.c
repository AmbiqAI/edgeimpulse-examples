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
#include "am_mcu_apollo.h"
#include "am_bsp.h"
#include "am_util.h"
#include "nex_api.h"
#include "diskio.h"
#include "mmc_apollo4.h"

static volatile BYTE hc_xc_card = 0;
static volatile DSTATUS Stat = STA_NOINIT;  /* Disk status */
static NEX_MMC_EXT_CSD ext_csd;

SemaphoreHandle_t comp_flag;

/*--------------------------------------------------------------------------

   Module Private Functions

---------------------------------------------------------------------------*/
//*****************************************************************************
//
// This callback function is called from 'nex_api_thread' task
//
//*****************************************************************************
static void UsrFunctionPtr(SD_CALL_BACK_INFO *info)
{
    xSemaphoreGive(comp_flag);
}

//
// PIO, SDMA, ADMA2 mode setting
//
typedef enum
{
    NEX_XFER_PIO,
    NEX_XFER_DMA,
    NEX_XFER_ADMA2,
} xfer_mode_e;

//*****************************************************************************
//
// This function enables/disables the DMA mode in the SDIO Host Controller.
//
//*****************************************************************************
static void set_dma_mode(struct sdio_bus_specific_info * businfo, xfer_mode_e mode)
{
    switch (mode)
    {
        case NEX_XFER_PIO:  //PIO
            businfo->dma_enable_disable = 0;    // = 0 for PIO and 1 for DMA and ADMA
            businfo->dma_type = 0;              // = 0 for PIO, 0 for DMA and 1 for ADMA
            APP_PRINT("PIO mode is set\n");
            break;
        case NEX_XFER_DMA:  //DMA
            businfo->dma_enable_disable = 1;
            businfo->dma_type = 0;
            APP_PRINT("SDMA mode is set\n");
            break;
        case NEX_XFER_ADMA2:  //ADMA2
            businfo->dma_enable_disable = 1;
            businfo->dma_type = 1;
            APP_PRINT("ADMA2 mode is set \n");
            break;
        default: //DMA - default
            businfo->dma_enable_disable = 1;
            businfo->dma_type = 0;
            APP_PRINT("default SDMA is set\n");
            break;
    }
}

//*****************************************************************************
//
// This function sets the bus width in the SDIO Host Controller.
//
//*****************************************************************************
static void set_bus_width(struct sdio_bus_specific_info * businfo, int bus_width)
{
    if ((businfo->bus_width != bus_width) && (bus_width >= 0) && (bus_width <= 6))
    {
        businfo->bus_width = bus_width;
        APP_PRINT("set the bus width %d\n", bus_width);
    }
    else
    {
        APP_PRINT("use the default bus width %d\n", businfo->bus_width);
    }
}

/*--------------------------------------------------------------------------

   Public Functions

---------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------*/
/* Initialize Disk Drive                                                 */
/*-----------------------------------------------------------------------*/

DSTATUS mmc_disk_initialize (void)
{
    BYTE i = 0;
    struct sdio_bus_specific_info businfo;

    while (IsCardInitialized(SDHC, SLOT) == SD_FALSE)
    {
        vTaskDelay(pdMS_TO_TICKS(10));
        i++;
        if ( i == 100 )
        {
            APP_PRINT("Error: Card Initialization Fail\n");
            Stat |= STA_NOINIT;
            return Stat;
        }
    }

    if (INVALID_CARD == CardType(SDHC, SLOT))
    {
      APP_PRINT("Error: Invalid Card!!!\n");
      Stat |= STA_NOINIT;
      return Stat;
    }
    else if ( (CardType(SDHC, SLOT) == MMC_MODE_eMMCHC) || (CardType(SDHC, SLOT) == MMC_MODE_SDHC) || (CardType(SDHC, SLOT) == MMC_MODE_MMCHC) )
    {
      hc_xc_card = 1;
      APP_PRINT("High Capacity Card!!!\n");
    }
    else if ( CardType(SDHC, SLOT) == MMC_MODE_SDXC )
    {
      hc_xc_card = 1;
      APP_PRINT("eXtended Capacity Card!!!\n");
    }

    comp_flag = xSemaphoreCreateBinary();
    if (NULL == comp_flag)
    {
      APP_PRINT("Error: Semaphore create failed\n");
    }

    RegisterISRCallBack(UsrFunctionPtr, SDHC, SLOT);

    GetBusSpecificInformation(&businfo, SDHC, SLOT);

    //
    // Set DMA mode. 4-wire data lines
    //
    set_dma_mode(&businfo, NEX_XFER_DMA);
    set_bus_width(&businfo, SD_BUS_WIDTH4);
    SetBusSpecificInformation(&businfo, SDHC, SLOT);

    Stat &= ~STA_NOINIT;

    return Stat;
}


/*-----------------------------------------------------------------------*/
/* Get Disk Status                                                       */
/*-----------------------------------------------------------------------*/

DSTATUS mmc_disk_status (void)
{
    return Stat;
}


/*-----------------------------------------------------------------------*/
/* Read Sector(s)                                                        */
/*-----------------------------------------------------------------------*/

DRESULT mmc_disk_read (
    BYTE *buff,         /* Pointer to the data buffer to store read data */
    LBA_t sector,       /* Start sector number (LBA) */
    UINT count          /* Sector count (1..128) */
)
{
    DWORD rd_length, rd_addr;

    if (!count)
    {
        return RES_PARERR;
    }
    if (Stat & STA_NOINIT)
    {
        return RES_NOTRDY;
    }

    APP_PRINT("Read Data\n");
    APP_PRINT("count = %d  sector= %d\n", count, sector);

    rd_length = count * 512;
    if (hc_xc_card == 1)
    {
        rd_addr = sector;
    }
    else
    {
        rd_addr = sector * 512;
    }

    //
    // Calling GetData API
    //
    if ( GetData(buff, rd_length, rd_addr, SDHC, SLOT) != SD_SUCCESS )
    {
        APP_PRINT("Fail to read the device\n");
        return RES_ERROR;
    }
    xSemaphoreTake(comp_flag, portMAX_DELAY);
    APP_PRINT("Read Completed \n");

    return RES_OK;
}


/*-----------------------------------------------------------------------*/
/* Write Sector(s)                                                       */
/*-----------------------------------------------------------------------*/

DRESULT mmc_disk_write (
    const BYTE *buff,   /* Pointer to the data to be written */
    LBA_t sector,       /* Start sector number (LBA) */
    UINT count          /* Sector count (1..128) */
)
{
    BYTE * wr_buf;
    DWORD wr_length, wr_addr;

    if (!count)
    {
        return RES_PARERR;
    }

    if (Stat & STA_NOINIT)
    {
        return RES_NOTRDY;
    }

    APP_PRINT("Write Data\n");
    APP_PRINT("count = %d  sector= %d\n", count, sector);

    wr_buf = (BYTE *)buff;
    wr_length = count * 512;
    if (hc_xc_card == 1)
    {
      wr_addr = sector;
    }
    else
    {
      wr_addr = sector * 512;
    }

    //
    // Calling SendData API
    //
    if ( SendData(wr_buf, wr_length, wr_addr, SDHC, SLOT) != SD_SUCCESS )
    {
       APP_PRINT("Fail to write the device\n");
       return RES_ERROR;
    }
    xSemaphoreTake(comp_flag, portMAX_DELAY);
    APP_PRINT("Write Completed \n");

    return RES_OK;
}

/*-----------------------------------------------------------------------*/
/* Miscellaneous Functions                                               */
/*-----------------------------------------------------------------------*/

DRESULT mmc_disk_ioctl (
    BYTE cmd,       /* Control code */
    void *buff      /* Buffer to send/receive control data */
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

        if (Stat & STA_NOINIT)
        {
            return RES_NOTRDY;
        }

        memset(&ext_csd, 0, sizeof(NEX_MMC_EXT_CSD));
        GetCardSpecificInfo_ExtCSD(&ext_csd, SDHC, SLOT);

        switch (cmd)
        {
            /* Make sure that no pending write process */
            case CTRL_SYNC :
              res = RES_OK;
              break;

            /* Get number of sectors on the disk (DWORD) */
            case GET_SECTOR_COUNT :
              *(DWORD*)buff = ext_csd.sec_count;
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

