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
#include "mmc_apollo5.h"

/*--------------------------------------------------------------------------

   Module Global variable

---------------------------------------------------------------------------*/

am_hal_card_t eMMC_FatFs[AM_REG_SDIO_NUM_MODULES];

am_device_emmc_config_t     g_sEMMC_config =
{
    .eHost                  = AM_HAL_SDHC_CARD_HOST,
    .eXferMode              = AM_HAL_HOST_XFER_ADMA,
    .ui32Clock              = 48000000,
    .eBusWidth              = AM_HAL_HOST_BUS_WIDTH_8,
    .eBusVoltage            = AM_HAL_HOST_BUS_VOLTAGE_1_8,
    .eUHSMode               = AM_HAL_HOST_UHS_SDR50,
    .bAsync                 = false,

    .eCardType              = AM_HAL_CARD_TYPE_EMMC,
    .eCardPwrCtrlPolicy     = AM_HAL_CARD_PWR_CTRL_NONE,
    .pCardPwrCtrlFunc       = NULL,
    .pfunCallback           = NULL,
    .ui32SectorCount        = 0x100,
};

am_device_emmc_hw_t   g_sEMMC_hw =
{
    .pSdhcHost = NULL,
    .pDevHandle = &eMMC_FatFs[0],
    .dStat = STA_NOINIT,
    .bAsyncReadIsDone = true,
    .bAsyncWriteIsDone = true,
};
/*--------------------------------------------------------------------------

   Module Private Functions

---------------------------------------------------------------------------*/


/*--------------------------------------------------------------------------

   Public Functions

---------------------------------------------------------------------------*/


// In this test, eMMC is always on the interface opposite of SDIO
#if SDIO_WIFI_INTERFACE == 1
void am_sdio0_isr(void)
#else
void am_sdio1_isr(void)
#endif
{
    uint32_t ui32IntStatus;

    am_hal_sdhc_intr_status_get(g_sEMMC_hw.pSdhcHost->pHandle, &ui32IntStatus, true);
    am_hal_sdhc_intr_status_clear(g_sEMMC_hw.pSdhcHost->pHandle, ui32IntStatus);
    am_hal_sdhc_interrupt_service(g_sEMMC_hw.pSdhcHost->pHandle, ui32IntStatus);
}

/*-----------------------------------------------------------------------*/
/* Initialize Disk Drive                                                 */
/*-----------------------------------------------------------------------*/

DSTATUS mmc_disk_initialize (void)
{
    if ( g_sEMMC_hw.dStat == STA_NOINIT )
    {

        //
        // Get the SDHC card host instance
        //
        g_sEMMC_hw.pSdhcHost = am_hal_get_card_host(g_sEMMC_config.eHost, true);

        if (g_sEMMC_hw.pSdhcHost == NULL)
        {
            am_util_stdio_printf("No such card host !!!\n");
            am_util_stdio_printf("SDIO Test Fail !!!\n");
            g_sEMMC_hw.dStat |= STA_NOINIT;
            return g_sEMMC_hw.dStat;
        }

        //
        // enable sdio pins
        //
        am_bsp_sdio_pins_enable(g_sEMMC_hw.pSdhcHost->ui32Module, AM_HAL_HOST_BUS_WIDTH_8);

        //
        // check if card is present
        //
        uint32_t ui32Retrials = 10;
        while (am_hal_card_host_find_card(g_sEMMC_hw.pSdhcHost, g_sEMMC_hw.pDevHandle) != AM_HAL_STATUS_SUCCESS)
        {
            am_util_stdio_printf("No card is present now\n");
            am_util_delay_ms(1000);
            am_util_stdio_printf("Checking if card is available again\n");
            ui32Retrials--;
            if(ui32Retrials == 0)
            {
                g_sEMMC_hw.dStat |= STA_NOINIT;
                return g_sEMMC_hw.dStat;
            }
        }

        ui32Retrials = 10;
        while (am_hal_card_init(g_sEMMC_hw.pDevHandle, g_sEMMC_config.eCardType, g_sEMMC_config.pCardPwrCtrlFunc, g_sEMMC_config.eCardPwrCtrlPolicy) != AM_HAL_STATUS_SUCCESS)
        {
            am_util_delay_ms(1000);
            am_util_stdio_printf("card and host is not ready, try again\n");
            ui32Retrials--;
            if(ui32Retrials == 0)
            {
                g_sEMMC_hw.dStat |= STA_NOINIT;
                return g_sEMMC_hw.dStat;
            }
        }

        am_util_debug_printf("eMMC SN: 0x%x\n", am_hal_card_get_cid_field(g_sEMMC_hw.pDevHandle, 16, 32));

        g_sEMMC_config.ui32SectorCount = am_hal_card_get_ext_csd_field(g_sEMMC_hw.pDevHandle, 212, 4);
        if ( g_sEMMC_config.ui32SectorCount == 0 )
        {
            am_util_debug_printf("Fail to get the sector counter of eMMC\n");
            g_sEMMC_hw.dStat |= STA_NOINIT;
            return g_sEMMC_hw.dStat;
        }
        else
        {
            am_util_debug_printf("eMMC EXT CSD Sector Count is: 0x%x\n", g_sEMMC_config.ui32SectorCount);
        }

        if(AM_HAL_STATUS_SUCCESS != am_hal_card_cfg_set(g_sEMMC_hw.pDevHandle, g_sEMMC_config.eCardType,
                                                        g_sEMMC_config.eBusWidth,
                                                        g_sEMMC_config.ui32Clock,
                                                        g_sEMMC_config.eBusVoltage,
                                                        g_sEMMC_config.eUHSMode))
        {
            am_util_stdio_printf("card config set failed\n");
            g_sEMMC_hw.dStat |= STA_NOINIT;
            return g_sEMMC_hw.dStat;
        }

        am_hal_card_host_set_xfer_mode(g_sEMMC_hw.pSdhcHost, g_sEMMC_config.eXferMode);

        am_hal_card_register_evt_callback(g_sEMMC_hw.pDevHandle, g_sEMMC_config.pfunCallback);

#ifdef ENABLE_SDIO_POWER_SAVE
        //
        // power down SDIO peripheral
        //
        if ( am_hal_card_pwrctrl_sleep(g_sEMMC_hw.pDevHandle) != AM_HAL_STATUS_SUCCESS )
        {
            am_util_debug_printf("Fail to power down SDIO peripheral\n");
            g_sEMMC_hw.dStat |= STA_NOINIT;
            return g_sEMMC_hw.dStat;
        }
#endif
        g_sEMMC_hw.dStat &= ~STA_NOINIT;
    }

    return g_sEMMC_hw.dStat;
}



/*-----------------------------------------------------------------------*/
/* Get Disk Status                                                       */
/*-----------------------------------------------------------------------*/

DSTATUS mmc_disk_status (void)
{
    return g_sEMMC_hw.dStat;
}



/*-----------------------------------------------------------------------*/
/* Read Sector(s)                                                        */
/*-----------------------------------------------------------------------*/

DRESULT mmc_disk_read (
    BYTE *buff,            /* Pointer to the data buffer to store read data */
    LBA_t sector,          /* Start sector number (LBA) */
    UINT count            /* Sector count (1..128) */
)
{
    DWORD ui32Status;

    if ( !count )
    {
        return RES_PARERR;
    }

    if (g_sEMMC_hw.dStat & STA_NOINIT)
    {
        return RES_NOTRDY;
    }

#ifdef ENABLE_SDIO_POWER_SAVE
    //
    // power up SDIO peripheral
    //
    if ( am_hal_card_pwrctrl_wakeup(g_sEMMC_hw.pDevHandle) != AM_HAL_STATUS_SUCCESS )
    {
        am_util_debug_printf("Fail to power up SDIO peripheral\n");
        return RES_ERROR;
    }
#endif

    //am_util_debug_printf("Read, count = %d  sector= %d\n", count, sector);


    //
    // Calling Read API
    //

    if(!g_sEMMC_config.bAsync)
    {
        ui32Status = am_hal_card_block_read_sync(g_sEMMC_hw.pDevHandle, sector, count, (uint8_t *)buff);
        if ( ui32Status == 0 )
        {
            am_util_debug_printf("Fail to call read API. number of bytes read = %d\n", ui32Status);
            return RES_ERROR;
        }
    }
    else
    {
        g_sEMMC_hw.bAsyncReadIsDone = false;
        ui32Status = am_hal_card_block_read_async(g_sEMMC_hw.pDevHandle, sector, count, (uint8_t *)buff);
        if ( ui32Status != AM_HAL_STATUS_SUCCESS )
        {
            am_util_debug_printf("Fail to call read API. Read Status = %d\n", ui32Status);
            return RES_ERROR;
        }

        //
        // wait until the async read is done
        //
        uint32_t ui32CounterTimeout = 0;
        while (!g_sEMMC_hw.bAsyncReadIsDone)
        {
            am_util_delay_us(100); //!  bumped up from 1ms
            ui32CounterTimeout++;
            if ( ui32CounterTimeout == 5000 ) //! bumped up from 1000
            {
                am_util_debug_printf("Read Timeout\n");
                return RES_ERROR;
            }
        }
    }

#ifdef ENABLE_SDIO_POWER_SAVE
    //
    // power down SDIO peripheral
    //
    if ( am_hal_card_pwrctrl_sleep(g_sEMMC_hw.pDevHandle) != AM_HAL_STATUS_SUCCESS )
    {
        am_util_debug_printf("Fail to power down SDIO peripheral\n");
        return RES_ERROR;
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
    DWORD ui32Status;

    if ( !count )
    {
        return RES_PARERR;
    }

    if ( g_sEMMC_hw.dStat & STA_NOINIT )
    {
        return RES_NOTRDY;
    }

#ifdef ENABLE_SDIO_POWER_SAVE
    //
    // power up SDIO peripheral
    //
    if ( am_hal_card_pwrctrl_wakeup(g_sEMMC_hw.pDevHandle) != AM_HAL_STATUS_SUCCESS )
    {
        am_util_debug_printf("Fail to power up SDIO peripheral\n");
        return RES_ERROR;
    }
#endif

    //am_util_debug_printf("Write, count = %d  sector= %d\n", count, sector);

    wr_buf = (BYTE *)buff;

    //
    // Calling Write API

    if(!g_sEMMC_config.bAsync)
    {
        ui32Status = am_hal_card_block_write_sync(g_sEMMC_hw.pDevHandle, sector, count, (uint8_t *)wr_buf);
        if ( ui32Status == 0 )
        {
            am_util_debug_printf("Fail to call write API.  Number of bytes written is = %d\n", ui32Status);
            return RES_ERROR;
        }
    }
    else
    {
        g_sEMMC_hw.bAsyncWriteIsDone = false;
        ui32Status = am_hal_card_block_write_async(g_sEMMC_hw.pDevHandle, sector, count, (uint8_t *)wr_buf);
        if ( ui32Status != AM_HAL_STATUS_SUCCESS )
        {
            am_util_debug_printf("Fail to call write API. Write Status = %d\n", ui32Status);
            return RES_ERROR;
        }

        //
        // wait until the async write is done
        //
        uint32_t ui32CounterTimeout = 0;
        while (!g_sEMMC_hw.bAsyncWriteIsDone)
        {
            am_util_delay_us(100);  // was 1ms
            ui32CounterTimeout++;
            if ( ui32CounterTimeout == 1000 )
            {
                am_util_debug_printf("Write Timeout\n");
                return RES_ERROR;
            }
        }
    }


#ifdef ENABLE_SDIO_POWER_SAVE
    //
    // power down SDIO peripheral
    //
    if ( am_hal_card_pwrctrl_sleep(g_sEMMC_hw.pDevHandle) != AM_HAL_STATUS_SUCCESS )
    {
        am_util_debug_printf("Fail to power down SDIO peripheral\n");
        return RES_ERROR;
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

    if (g_sEMMC_hw.dStat & STA_NOINIT)
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
            *(DWORD*)buff = g_sEMMC_config.ui32SectorCount;
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

