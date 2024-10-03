//*****************************************************************************
//
//! amotas_main.c
//! @file
//!
//! @brief This file provides the main application for the AMOTA service.
//!
//
//*****************************************************************************

//*****************************************************************************
//
// ${copyright}
//
// This is part of revision ${version} of the AmbiqSuite Development Package.
//
//*****************************************************************************
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include "amotas_api.h"
#include "amota_profile_config.h"
#include "am_logger.h"
#include "rpc_common.h"

/*
 * GLOBAL VARIABLE
 ****************************************************************************************
 */
static am_util_multiboot_flash_info_t *g_pFlash = &g_intFlash;

amotasFlashOp_t amotasFlash = {
    .bufferIndex = 0,
};

// Temporary scratch buffer used to read from flash
uint32_t amotasTmpBuf[AMOTA_PACKET_SIZE / 4];
static uint32_t sblOtaStorageAddr = AMOTA_INVALID_SBL_STOR_ADDR;

amotasCb_t amotasCb;

/*
 * LOCAL FUNCTION DEFINITIONS
 ****************************************************************************************
 */

// Erases the flash based on ui32Addr & ui32NumBytes
static void erase_flash(uint32_t ui32Addr, uint32_t ui32NumBytes)
{
    // Erase the image
    while ( ui32NumBytes )
    {
        g_pFlash->flash_erase_sector(ui32Addr);
        if ( ui32NumBytes > g_pFlash->flashSectorSize )
        {
            ui32NumBytes -= g_pFlash->flashSectorSize;
            ui32Addr += g_pFlash->flashSectorSize;
        }
        else
        {
            break;
        }
    }
}

//*****************************************************************************
//
// Set Firmware Address
//
//return true if success, otherwise false
//*****************************************************************************
static bool amotas_set_fw_addr(void)
{
    bool bResult = false;

    amotasCb.newFwFlashInfo.addr = 0;
    amotasCb.newFwFlashInfo.offset = 0;

    //
    // Check storage type
    //
    if ( amotasCb.fwHeader.storageType == AMOTA_FW_STORAGE_INTERNAL )
    {
        // storage in internal flash
        uint32_t storeAddr = (AMOTA_INT_FLASH_OTA_ADDRESS + AM_HAL_FLASH_PAGE_SIZE - 1) & ~(AM_HAL_FLASH_PAGE_SIZE - 1);
        uint32_t maxSize = AMOTA_INT_FLASH_OTA_MAX_SIZE & ~(AM_HAL_FLASH_PAGE_SIZE - 1);

#if  !defined(AM_PART_APOLLO4B) && !defined(AM_PART_APOLLO4P)// There is no easy way to get the information about the main image
        uint32_t ui32CurLinkAddr;
        uint32_t ui32CurLen;
        // Get information about current main image
        if (am_util_multiboot_get_main_image_info(&ui32CurLinkAddr, &ui32CurLen) == false)
        {
            return false;
        }
        // Length in flash page size multiples
        ui32CurLen = (ui32CurLen + (AM_HAL_FLASH_PAGE_SIZE - 1)) & ~(AM_HAL_FLASH_PAGE_SIZE - 1) ;
        // Make sure the OTA area does not overwrite the main image
        if (!((storeAddr + maxSize) <= ui32CurLinkAddr) &&
            !(storeAddr >= (ui32CurLinkAddr + ui32CurLen)))
        {
            am_error_printf(true, "OTA memory overlaps with main firmware\r\n");
            return false;
        }
#endif
        // Check to make sure the incoming image will fit in the space allocated for OTA
        if (amotasCb.fwHeader.fwLength > maxSize)
        {
            am_error_printf(true, "not enough OTA space allocated = %d bytes, Desired = %d bytes",
                maxSize, amotasCb.fwHeader.fwLength);

            return false;
        }

        g_pFlash = &g_intFlash;

        amotasCb.newFwFlashInfo.addr = storeAddr;

        am_info_printf(true, "storeAddr:0x%x\r\n", storeAddr);

        bResult = true;
    }
    else if ( amotasCb.fwHeader.storageType == AMOTA_FW_STORAGE_EXTERNAL )
    {
        //storage in external flash

#if (AMOTAS_SUPPORT_EXT_FLASH == 1)
        //
        // update target address information
        //
        g_pFlash = &g_extFlash;

        if (g_pFlash->flash_read_page &&
            g_pFlash->flash_write_page &&
            g_pFlash->flash_erase_sector &&
            (amotasCb.fwHeader.fwLength <= AMOTA_EXT_FLASH_OTA_MAX_SIZE))
        {
            amotasCb.newFwFlashInfo.addr = AMOTA_EXT_FLASH_OTA_ADDRESS;
            bResult = true;
        }
        else
#endif
        {
            bResult = false;
        }
    }
    else
    {
        // reserved state
        bResult = false;
    }

    if (bResult == true)
    {
        //
        // Initialize the flash device.
        //
        if (FLASH_OPERATE(g_pFlash, flash_init) == 0)
        {
            if (FLASH_OPERATE(g_pFlash, flash_enable) != 0)
            {
                FLASH_OPERATE(g_pFlash, flash_deinit);
                bResult = false;
            }
            //
            // Erase necessary sectors in the flash according to length of the image.
            //
            erase_flash(amotasCb.newFwFlashInfo.addr, amotasCb.fwHeader.fwLength);

            FLASH_OPERATE(g_pFlash, flash_disable);
        }
        else
        {
            bResult = false;
        }
    }
    return bResult;
}

static int verify_flash_content(uint32_t flashAddr, uint32_t *pSram, uint32_t len, am_util_multiboot_flash_info_t *pFlash)
{
    // read back and check
    uint32_t  offset = 0;
    uint32_t  remaining = len;
    int       ret = 0;
    while (remaining)
    {
        uint32_t tmpSize =
            (remaining > AMOTA_PACKET_SIZE) ? AMOTA_PACKET_SIZE : remaining;
        pFlash->flash_read_page((uint32_t)amotasTmpBuf, (uint32_t *)(flashAddr + offset), tmpSize);

        ret = memcmp(amotasTmpBuf, (uint8_t*)((uint32_t)pSram + offset), tmpSize);

        if ( ret != 0 )
        {
            // there is write failure happened.
            am_error_printf(true, "flash write verify failed. address 0x%x. length %d", flashAddr, len);
            break;
        }
        offset += tmpSize;
        remaining -= tmpSize;
    }
    return ret;
}

//*****************************************************************************
//
// Write to Flash
//
//return true if success, otherwise false
//*****************************************************************************
static bool amotas_write2flash(uint16_t len, uint8_t *buf, uint32_t addr, bool lastPktFlag)
{
    uint16_t ui16BytesRemaining = len;
    uint32_t ui32TargetAddress = 0;
    uint8_t ui8PageCount = 0;
    bool bResult = true;
    uint16_t i;

    addr -= amotasFlash.bufferIndex;
    //
    // Check the target flash address to ensure we do not operation the wrong address
    // make sure to write to page boundary
    //
#if defined(AM_PART_APOLLO4B) || defined(AM_PART_APOLLO4P)
    if (((!((amotasCb.fwHeader.imageId == AMOTA_IMAGE_ID_SBL) &&
        (amotasCb.newFwFlashInfo.offset < AMOTA_ENCRYPTED_SBL_SIZE))) &&
        ((uint32_t)amotasCb.newFwFlashInfo.addr > addr)) ||
        (addr & (g_pFlash->flashPageSize - 1)))
    {
        //
        // application is trying to write to wrong address
        //
        return false;
    }
#else
    if (((uint32_t)amotasCb.newFwFlashInfo.addr > addr) ||
        (addr & (g_pFlash->flashPageSize - 1)))
    {
        //
        // application is trying to write to wrong address
        //
        return false;
    }
#endif

    FLASH_OPERATE(g_pFlash, flash_enable);

    while (ui16BytesRemaining)
    {
        uint16_t ui16Bytes2write = g_pFlash->flashPageSize - amotasFlash.bufferIndex;
        if (ui16Bytes2write > ui16BytesRemaining)
        {
            ui16Bytes2write = ui16BytesRemaining;
        }
        // move data into buffer
        for ( i = 0; i < ui16Bytes2write; i++ )
        {
            // avoid using memcpy
            amotasFlash.writeBuffer[amotasFlash.bufferIndex++] = buf[i];
        }
        ui16BytesRemaining -= ui16Bytes2write;
        buf += ui16Bytes2write;

        //
        // Write to flash when there is data more than 1 page size
        // For last fragment write even if it is less than one page
        //
        if (lastPktFlag || (amotasFlash.bufferIndex == g_pFlash->flashPageSize))
        {
            ui32TargetAddress = (addr + ui8PageCount*g_pFlash->flashPageSize);
            // Always write whole pages
            if ((g_pFlash->flash_write_page(ui32TargetAddress, (uint32_t *)amotasFlash.writeBuffer, g_pFlash->flashPageSize) != 0)
                || (verify_flash_content(ui32TargetAddress, (uint32_t *)amotasFlash.writeBuffer, amotasFlash.bufferIndex, g_pFlash) != 0))
            {
                bResult = false;
                am_error_printf(true, "write to flash failed\r\n");
                break;
            }

            am_debug_printf(true, "Flash write succeeded to address 0x%x. length %d, lastPktFlag:%d", ui32TargetAddress, amotasFlash.bufferIndex, lastPktFlag);

            ui8PageCount++;
            amotasFlash.bufferIndex = 0;
            bResult = true;
        }
    }
    FLASH_OPERATE(g_pFlash, flash_disable);

    //
    // If we get here, operations are done correctly
    //
    return bResult;
}


//*****************************************************************************
//
// Update OTA information with Firmware Information.
//
//*****************************************************************************
static void amotas_update_ota(void)
{
    uint8_t magic;
    if (amotasCb.fwHeader.imageId == AMOTA_IMAGE_ID_SBL)
    {
        magic = AM_IMAGE_MAGIC_SBL;
    }
    else
    {
        magic = amotasCb.metaData.magicNum;
    }

    // Set OTAPOINTER
    am_hal_ota_add(AM_HAL_MRAM_PROGRAM_KEY, magic, (uint32_t *)amotasCb.newFwFlashInfo.addr);
}

static void amotas_init_ota(void)
{
    am_hal_otadesc_t *pOtaDesc = (am_hal_otadesc_t *)(OTA_POINTER_LOCATION & ~(AM_HAL_FLASH_PAGE_SIZE - 1));
    // Initialize OTA descriptor - This should ideally be initiated through a separate command
    // to facilitate multiple image upgrade in a single reboot
    // Will need change in the AMOTA app to do so
    am_hal_ota_init(AM_HAL_MRAM_PROGRAM_KEY, pOtaDesc);
}


/*
 * EXPORTED FUNCTION DEFINITIONS
 ****************************************************************************************
 */

void amota_init(void)
{
    memset(&amotasCb, 0, sizeof(amotasCb));

    amotas_init_ota();
}

void amota_packet_header_handle(uint8_t *header_data)
{
    bool    bResult         = false;
    bool    resumeTransfer  = false;
    uint8_t *p_heaser_data  = header_data;

    BSTREAM_TO_UINT8(resumeTransfer, p_heaser_data);
    BSTREAM_TO_UINT32(amotasCb.fwHeader.fwLength, p_heaser_data);
    BSTREAM_TO_UINT32(amotasCb.fwHeader.storageType, p_heaser_data);
    BSTREAM_TO_UINT32(amotasCb.fwHeader.imageId, p_heaser_data);

    am_info_printf(true, "resumeTransfer:%d, imageId:%d, fwLength:0x%x\r\n", resumeTransfer, amotasCb.fwHeader.imageId, amotasCb.fwHeader.fwLength);

    //get the SBL OTA storage address if the image is for SBL
    //the address can be 0x8000 or 0x10000 based on current SBL running address
    if (amotasCb.fwHeader.imageId == AMOTA_IMAGE_ID_SBL)
    {
        am_hal_security_info_t pSecInfo;
        am_hal_security_get_info(&pSecInfo);
        sblOtaStorageAddr = pSecInfo.sblStagingAddr;
        am_info_printf(true, "get sblOtaStorageAddr: 0x%x", sblOtaStorageAddr);
    }
    if (resumeTransfer)
    {
        am_info_printf(true, "OTA process start from offset = 0x%x", amotasCb.newFwFlashInfo.offset);
        am_info_printf(true, "beginning of flash addr = 0x%x", amotasCb.newFwFlashInfo.addr);
    }
    else
    {
        am_info_printf(true, "OTA process start from beginning\r\n");
        amotasFlash.bufferIndex = 0;
        bResult = amotas_set_fw_addr();

        if ( bResult == false )
        {
            am_error_printf(true, "set fw addr fail\r\n");
        }
    }
}

void amota_reset_start(void)
{
    am_info_printf(true, "amotas reset board\r\n");
    am_util_delay_ms(10);

    am_hal_reset_control(AM_HAL_RESET_CONTROL_SWPOR, 0);
}

void amota_update_flag(void)
{
    amotas_update_ota();

    FLASH_OPERATE(g_pFlash, flash_deinit);
    g_pFlash = &g_intFlash;
}

void amota_write2flash(uint16_t len, uint8_t *buf, bool last_pkt_flag)
{
    bool bResult = false;

    if (amotasCb.fwHeader.imageId == AMOTA_IMAGE_ID_SBL)
    {
        if ( amotasCb.newFwFlashInfo.offset < AMOTA_ENCRYPTED_SBL_SIZE )
        {
            bResult = amotas_write2flash(len, buf, sblOtaStorageAddr + amotasCb.newFwFlashInfo.offset, last_pkt_flag);
        }
        else
        {
            bResult = amotas_write2flash(len, buf, amotasCb.newFwFlashInfo.addr + amotasCb.newFwFlashInfo.offset - AMOTA_ENCRYPTED_SBL_SIZE, last_pkt_flag);
        }
    }
    else
    {
        if(amotasCb.newFwFlashInfo.offset == 0)
        {
            memcpy(&amotasCb.metaData, buf, sizeof(amotaMetadataInfo_t));
        }

        bResult = amotas_write2flash(len, buf, amotasCb.newFwFlashInfo.addr + amotasCb.newFwFlashInfo.offset, last_pkt_flag);
    }

    if ( bResult )
    {
        amotasCb.newFwFlashInfo.offset += len;
    }
    else
    {
        am_error_printf(true, "write 2 flash fail, len:%d\r\n", len);
    }
}
