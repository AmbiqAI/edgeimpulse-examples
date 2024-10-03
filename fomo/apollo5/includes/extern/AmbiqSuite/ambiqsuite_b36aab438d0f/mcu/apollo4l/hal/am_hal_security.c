//*****************************************************************************
//
//! @file am_hal_security.c
//!
//! @brief Functions for on-chip security features
//!
//! @addtogroup security_4l Security Functionality
//! @ingroup apollo4l_hal
//! @{
//
//*****************************************************************************

//*****************************************************************************
//
// ${copyright}
//
// This is part of revision ${version} of the AmbiqSuite Development Package.
//
//*****************************************************************************
#include <stdint.h>
#include <stdbool.h>
#include "am_mcu_apollo.h"

//*****************************************************************************
//  Local defines.
//*****************************************************************************
//
//! Maximum iterations for hardware CRC to finish
//
#define MAX_CRC_WAIT        100000

#define AM_HAL_SECURITY_LOCKSTAT_CUSTOTP_PROG   0x00000001
#define AM_HAL_SECURITY_LOCKSTAT_CUSTOTP_READ   0x00000002
// #### INTERNAL BEGIN ####
#define AM_HAL_SECURITY_LOCKSTAT_INFO1ACCESS    0x80000000
#define AM_HAL_SECURITY_LOCKSTAT_AMBOTP_PROG    0x00000010
#define AM_HAL_SECURITY_LOCKSTAT_AMBOTP_READ    0x00000020
// #### INTERNAL END ####

//*****************************************************************************
//
// Globals
//
//*****************************************************************************

//*****************************************************************************
//
// Get Device Security Info
//
// This will retrieve the security information for the device
//
//*****************************************************************************
uint32_t am_hal_security_get_info(am_hal_security_info_t *pSecInfo)
{
    bool     bSbl;

    if ( !pSecInfo )
    {
        return AM_HAL_STATUS_INVALID_ARG;
    }

    am_hal_mram_info_read(0, AM_REG_INFO0_SECURITY_VERSION_O / 4, 1, &pSecInfo->info0Version);
    pSecInfo->bInfo0Valid = MCUCTRL->SHADOWVALID_b.INFO0VALID;
    bSbl = (MCUCTRL->BOOTLOADER_b.SECBOOTFEATURE == MCUCTRL_BOOTLOADER_SECBOOTFEATURE_ENABLED);
    if ((PWRCTRL->DEVPWRSTATUS_b.PWRSTCRYPTO == 0) || (CRYPTO->HOSTCCISIDLE_b.HOSTCCISIDLE == 0))
    {
        // Crypto is not accessible
        pSecInfo->lcs = AM_HAL_SECURITY_LCS_UNDEFINED;
    }
    else
    {
        pSecInfo->lcs = (am_hal_security_device_lcs_e)CRYPTO->LCSREG_b.LCSREG;
    }
    if ( bSbl )
    {
        // Use the field in INFO1 - Readable space
        am_hal_mram_info_read(1, AM_REG_INFO1_SBR_VERSION_0_O / 4, 1, &pSecInfo->sbrVersion);
        am_hal_mram_info_read(1, AM_REG_INFO1_SBL_VERSION_0_O / 4, 1, &pSecInfo->sblVersion);
        am_hal_mram_info_read(1, AM_REG_INFO1_SBL_VERSION_1_O / 4, 1, &pSecInfo->sblVersionAddInfo);
        am_hal_mram_info_read(1, AM_REG_INFO1_SBLOTA_O / 4, 1, &pSecInfo->sblStagingAddr);
    }
    else
    {
        pSecInfo->sbrVersion = 0xFFFFFFFF;
        pSecInfo->sblVersion = 0xFFFFFFFF;
        pSecInfo->sblVersionAddInfo = 0xFFFFFFFF;
        pSecInfo->sblStagingAddr = 0xFFFFFFFF;
    }

    return AM_HAL_STATUS_SUCCESS;

} // am_hal_security_get_info()

//*****************************************************************************
//
// Get Device Security SOCID
//
// This will retrieve the SOCID information for the device
//
//*****************************************************************************
uint32_t am_hal_security_get_socid(am_hal_security_socid_t *pSocId)
{
    return am_hal_mram_info_read(1, AM_REG_INFO1_SOCID0_O / 4, 8, pSocId->socid);
}

//*****************************************************************************
//
// Set the key for specified lock
//
// This will program the lock registers for the specified lock and key
//
//*****************************************************************************
uint32_t am_hal_security_set_key(am_hal_security_locktype_t lockType, am_hal_security_128bkey_t *pKey)
{
#ifndef AM_HAL_DISABLE_API_VALIDATION
    if ( pKey == NULL )
    {
        return AM_HAL_STATUS_INVALID_ARG;
    }

    switch (lockType)
    {
        case AM_HAL_SECURITY_LOCKTYPE_CUSTOTP_PROG:
        case AM_HAL_SECURITY_LOCKTYPE_CUSTOTP_READ:
// #### INTERNAL BEGIN ####
        case AM_HAL_SECURITY_LOCKTYPE_INFO1ACCESS:
        case AM_HAL_SECURITY_LOCKTYPE_AMBOTP_PROG:
        case AM_HAL_SECURITY_LOCKTYPE_AMBOTP_READ:
// #### INTERNAL END ####
            break;
        default:
            return AM_HAL_STATUS_INVALID_ARG;
    }
#endif // AM_HAL_DISABLE_API_VALIDATION

    SECURITY->LOCKCTRL = lockType;
    SECURITY->KEY0 = pKey->keys.key0;
    SECURITY->KEY1 = pKey->keys.key1;
    SECURITY->KEY2 = pKey->keys.key2;
    SECURITY->KEY3 = pKey->keys.key3;

    return AM_HAL_STATUS_SUCCESS;

} // am_hal_security_set_key()

//*****************************************************************************
//
// Get the current status of the specified lock
//
// This will get the lock status for specified lock - true implies unlocked
// Note that except for customer lock, other locks are self-locking on status read
//
//*****************************************************************************
uint32_t am_hal_security_get_lock_status(am_hal_security_locktype_t lockType, bool *pbUnlockStatus)
{
    uint32_t unlockMask;
#ifndef AM_HAL_DISABLE_API_VALIDATION
    if (pbUnlockStatus == NULL)
    {
        return AM_HAL_STATUS_INVALID_ARG;
    }
#endif // AM_HAL_DISABLE_API_VALIDATION

    switch ( lockType )
    {
        case AM_HAL_SECURITY_LOCKTYPE_CUSTOTP_PROG:
            unlockMask = AM_HAL_SECURITY_LOCKSTAT_CUSTOTP_PROG;
            break;
        case AM_HAL_SECURITY_LOCKTYPE_CUSTOTP_READ:
            unlockMask = AM_HAL_SECURITY_LOCKSTAT_CUSTOTP_READ;
            break;
// #### INTERNAL BEGIN ####
        case AM_HAL_SECURITY_LOCKTYPE_INFO1ACCESS:
            unlockMask = AM_HAL_SECURITY_LOCKSTAT_INFO1ACCESS;
            break;
        case AM_HAL_SECURITY_LOCKTYPE_AMBOTP_PROG:
            unlockMask = AM_HAL_SECURITY_LOCKSTAT_AMBOTP_PROG;
            break;
        case AM_HAL_SECURITY_LOCKTYPE_AMBOTP_READ:
            unlockMask = AM_HAL_SECURITY_LOCKSTAT_AMBOTP_READ;
            break;
// #### INTERNAL END ####
        default:
            return AM_HAL_STATUS_INVALID_ARG;
    }
    *pbUnlockStatus = SECURITY->LOCKSTAT & unlockMask;

    return AM_HAL_STATUS_SUCCESS;

} // am_hal_security_get_lock_status()

//*****************************************************************************
//
// Compute CRC32 for a specified payload
//
// This function uses the hardware engine to compute CRC32 on an arbitrary data
// payload.  The payload can reside in any contiguous memory including external
// memory.
//
//*****************************************************************************
uint32_t
am_hal_crc32(uint32_t ui32StartAddr, uint32_t ui32SizeBytes, uint32_t *pui32Crc)
{
    uint32_t status, ui32CRC32;

#ifndef AM_HAL_DISABLE_API_VALIDATION
    if (pui32Crc == NULL)
    {
        return AM_HAL_STATUS_INVALID_ARG;
    }

    //
    // Make sure size is multiple of 4 bytes
    //
    if (ui32SizeBytes & 0x3)
    {
        return AM_HAL_STATUS_INVALID_ARG;
    }

#endif // AM_HAL_DISABLE_API_VALIDATION

    //
    // Program the CRC engine to compute the crc
    //
    ui32CRC32                 = 0xFFFFFFFF;
    SECURITY->RESULT          = ui32CRC32;
    SECURITY->SRCADDR         = ui32StartAddr;
    SECURITY->LEN             = ui32SizeBytes;
    SECURITY->CTRL_b.FUNCTION = SECURITY_CTRL_FUNCTION_CRC32;

    if ((ui32StartAddr + ui32SizeBytes) >= (SRAM_BASEADDR + TCM_MAX_SIZE))
    {
        // Need to ensure memory writes have been flushed before starting hardware
        am_hal_sysctrl_bus_write_flush();
    }
    //
    // Start the CRC
    //
    SECURITY->CTRL_b.ENABLE = 1;

    //
    // Wait for CRC to finish
    //
    status = am_hal_delay_us_status_change(MAX_CRC_WAIT,
        (uint32_t)&SECURITY->CTRL, SECURITY_CTRL_ENABLE_Msk, 0);

    if (status == AM_HAL_STATUS_SUCCESS)
    {
        if (SECURITY->CTRL_b.CRCERROR)
        {
            status = AM_HAL_STATUS_HW_ERR;
        }
        *pui32Crc = SECURITY->RESULT;
    }

    return status;

} // am_hal_crc32()

//*****************************************************************************
//
// End Doxygen group.
//! @}
//
//*****************************************************************************
