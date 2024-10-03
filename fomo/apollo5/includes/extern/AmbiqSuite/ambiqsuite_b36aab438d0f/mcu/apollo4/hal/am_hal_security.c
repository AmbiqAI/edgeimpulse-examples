//*****************************************************************************
//
//  am_hal_security.c
//! @file
//!
//! @brief Functions for on-chip security features
//!
//! @addtogroup
//! @ingroup apollo4hal
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
// Maximum iterations for hardware CRC to finish
//
#define MAX_CRC_WAIT        100000

#define AM_HAL_SECURITY_LOCKSTAT_CUSTOMER       0x1
#define AM_HAL_SECURITY_LOCKSTAT_RECOVERY       0x40000000
// #### INTERNAL BEGIN ####
#define AM_HAL_SECURITY_LOCKSTAT_INFO1ACCESS    0x80000000
// #### INTERNAL END ####

//*****************************************************************************
//
// Globals
//
//*****************************************************************************

//
// Assign ptr variables to avoid an issue with GCC reading from location 0x0.
//
const volatile uint32_t *g_pFlash0 = (uint32_t*)(AM_HAL_SBL_ADDRESS + 0);

//*****************************************************************************
//
//! @brief  Get Device Security Info
//!
//! @param  pSecInfo -  Pointer to structure for returned security info
//!
//! This will retrieve the security information for the device
//!
//! @return Returns AM_HAL_STATUS_SUCCESS on success
//
//*****************************************************************************
uint32_t am_hal_security_get_info(am_hal_security_info_t *pSecInfo)
{
    uint32_t flash0;
    bool     bSbl;
    if (!pSecInfo)
    {
        return AM_HAL_STATUS_INVALID_ARG;
    }

    pSecInfo->info0Version = AM_REGVAL(0x42000040);
    pSecInfo->bInfo0Valid = MCUCTRL->SHADOWVALID_b.INFO0VALID;
    bSbl = MCUCTRL->BOOTLOADER_b.SECBOOTFEATURE;
    if ( bSbl )
    {
        // Check if we're running pre-SBLv2
        flash0 = *g_pFlash0;
        // Check if SBL is installed

        if ((flash0 >> 24) != 0xA3)
        {
            return AM_HAL_STATUS_FAIL;
        }
        else
        {
            // Use SBL jump table function
            uint32_t status;
            uint32_t sblVersion;
            uint32_t (*pFuncVersion)(uint32_t *) = (uint32_t (*)(uint32_t *))(AM_HAL_SBL_ADDRESS + 0x3D1);
            status = pFuncVersion(&sblVersion);
            if (status != AM_HAL_STATUS_SUCCESS)
            {
                return status;
            }
            pSecInfo->sblVersion = sblVersion & 0x7FFF;
            pSecInfo->sblVersionAddInfo = sblVersion >> 15;
        }
    }
    else
    {
        return AM_HAL_STATUS_FAIL;
    }
    return AM_HAL_STATUS_SUCCESS;
} // am_hal_security_get_info()

//*****************************************************************************
//
//! @brief  Set the key for specified lock
//!
//! @param  lockType - The lock type to be operated upon
//! @param  pKey -  Pointer to 128b key value
//!
//! This will program the lock registers for the specified lock and key
//!
//! @return Returns AM_HAL_STATUS_SUCCESS on success
//
//*****************************************************************************
uint32_t am_hal_security_set_key(am_hal_security_locktype_t lockType, am_hal_security_128bkey_t *pKey)
{
#ifndef AM_HAL_DISABLE_API_VALIDATION
    if (pKey == NULL)
    {
        return AM_HAL_STATUS_INVALID_ARG;
    }
    switch (lockType)
    {
        case AM_HAL_SECURITY_LOCKTYPE_CUSTOMER:
        case AM_HAL_SECURITY_LOCKTYPE_RECOVERY:
// #### INTERNAL BEGIN ####
        case AM_HAL_SECURITY_LOCKTYPE_INFO1ACCESS:
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
//! @brief  Get the current status of the specified lock
//!
//! @param  lockType - The lock type to be operated upon
//! @param  pbUnlockStatus -  Pointer to return variable with lock status
//!
//! This will get the lock status for specified lock - true implies unlocked
//! Note that except for customer lock, other locks are self-locking on status read
//!
//! @return Returns AM_HAL_STATUS_SUCCESS on success
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
    switch (lockType)
    {
        case AM_HAL_SECURITY_LOCKTYPE_CUSTOMER:
            unlockMask = AM_HAL_SECURITY_LOCKSTAT_CUSTOMER;
            break;
        case AM_HAL_SECURITY_LOCKTYPE_RECOVERY:
            unlockMask = AM_HAL_SECURITY_LOCKSTAT_RECOVERY;
            break;
// #### INTERNAL BEGIN ####
        case AM_HAL_SECURITY_LOCKTYPE_INFO1ACCESS:
            unlockMask = AM_HAL_SECURITY_LOCKSTAT_INFO1ACCESS;
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
//! @brief  Compute CRC32 for a specified payload
//!
//! @param  ui32StartAddr - The start address of the payload.
//! @param  ui32SizeBytes - The length of payload in bytes.
//! @param  pui32Crc      - Pointer to variable to return the computed CRC.
//!
//! This function uses the hardware engine to compute CRC32 on an arbitrary data
//! payload.  The payload can reside in any contiguous memory including external
//! memory.
//!
//! @return Returns AM_HAL_STATUS_SUCCESS on success
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

    // TODO - Validate that the memory location is valid
#endif // AM_HAL_DISABLE_API_VALIDATION

    //
    // Program the CRC engine to compute the crc
    //
    ui32CRC32                 = 0xFFFFFFFF;
    SECURITY->RESULT          = ui32CRC32;
    SECURITY->SRCADDR         = ui32StartAddr;
    SECURITY->LEN             = ui32SizeBytes;
    SECURITY->CTRL_b.FUNCTION = SECURITY_CTRL_FUNCTION_CRC32;

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
        *pui32Crc = SECURITY->RESULT;
    }

    return status;

} // am_hal_crc32()
