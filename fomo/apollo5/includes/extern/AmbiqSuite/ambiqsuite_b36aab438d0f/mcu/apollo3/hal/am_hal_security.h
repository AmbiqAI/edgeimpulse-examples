//*****************************************************************************
//
//! @file am_hal_security.h
//!
//! @brief Functions for on-chip security features
//!
//! @addtogroup security Security - On-Chip Security Functionality
//! @ingroup apollo3_hal
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

#ifndef AM_HAL_SECURITY_H
#define AM_HAL_SECURITY_H

//
//! @brief Location of SBL install address for this device
//!
//! @note Important Note:
//!  Some caution should be observed when using AM_HAL_SBL_ADDRESS as an address.
//!   GCC considers use of this address to be a NULL pointer.  When compiled with
//!   high optimization (-O3) and used to read the location with, for example,
//!   code such as *((volatile uint32_t *)(AM_HAL_SBL_ADDRESS)), GCC will insert
//!   an instruction it calls "UDF" (undefined), op-code 0xDEFF, which will cause
//!   a fault on execution to trap the "invalid" null-ptr usage.
//!  This does not appear to be an issue with IAR and Keil ARM5.
//!  It is likely an issue with Keil ARM6.
//
#define AM_HAL_SBL_ADDRESS                  0x00000000

typedef struct
{
    bool     bInfo0Valid;
    uint32_t info0Version;
    uint32_t sblVersion;
    uint32_t sblVersionAddInfo;
} am_hal_security_info_t;

//! LOCK Definitions
typedef enum
{
    AM_HAL_SECURITY_LOCKTYPE_CUSTOMER       = 0x1,
    AM_HAL_SECURITY_LOCKTYPE_RECOVERY       = 0x9D,
// #### INTERNAL BEGIN ####
    AM_HAL_SECURITY_LOCKTYPE_INFO1ACCESS    = 0x9E,
// #### INTERNAL END ####
} am_hal_security_locktype_t;

typedef union
{
    uint32_t keyword[4];
    struct
    {
        uint32_t key0;
        uint32_t key1;
        uint32_t key2;
        uint32_t key3;
    } keys;
} am_hal_security_128bkey_t;

#ifdef __cplusplus
extern "C"
{
#endif

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
uint32_t am_hal_security_get_info(am_hal_security_info_t *pSecInfo);

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
uint32_t am_hal_security_set_key(am_hal_security_locktype_t lockType, am_hal_security_128bkey_t *pKey);

//*****************************************************************************
//
//! @brief  Get the current status of the specified lock
//!
//! @param  lockType - The lock type to be operated upon
//! @param  pbUnlockStatus -  Pointer to return variable with lock status
//!
//! This will get the lock status for specified lock
//! Note that except for customer lock, other locks are self-locking on status read
//!
//! @return Returns AM_HAL_STATUS_SUCCESS on success
//
//*****************************************************************************
uint32_t am_hal_security_get_lock_status(am_hal_security_locktype_t lockType, bool *pbUnlockStatus);

//*****************************************************************************
//
//! @brief  Initialize CRC32 engine
//!
//! This will initialize the hardware engine to compute CRC32 on an arbitrary data payload
//!
//! @return Returns AM_HAL_STATUS_SUCCESS on success
//
//*****************************************************************************
uint32_t am_hal_crc32_init(void);

//*****************************************************************************
//
//! @brief  Accumulate CRC32 for a specified payload
//!
//! @param  ui32StartAddr - The start address of the payload
//! @param  ui32SizeBytes - The length of payload in bytes
//! @param  pui32Crc      - Pointer to accumulated CRC
//!
//! This will use the hardware engine to compute CRC32 on an arbitrary data payload
//!
//! @return Returns AM_HAL_STATUS_SUCCESS on success
//
//*****************************************************************************
uint32_t am_hal_crc32_accum(uint32_t ui32StartAddr, uint32_t ui32SizeBytes, uint32_t *pui32Crc) ;

//*****************************************************************************
//
//! @brief  Compute CRC32 for a specified payload
//!
//! @param  ui32StartAddr - The start address of the payload
//! @param  ui32SizeBytes - The length of payload in bytes
//! @param  pui32Crc      - Pointer to return computed CRC
//!
//! This will use the hardware engine to compute CRC32 on an arbitrary data payload
//!
//! @return Returns AM_HAL_STATUS_SUCCESS on success
//
//*****************************************************************************
uint32_t am_hal_crc32(uint32_t ui32StartAddr, uint32_t ui32SizeBytes, uint32_t *pui32Crc) ;

//*****************************************************************************
//
//! @brief  Helper function to Perform exit operations for a secondary bootloader
//!
//! @param  pImage - The address of the image to give control to
//!
//! This function does the necessary security operations while exiting from a
//! a secondary bootloader program. If still open, it locks the info0 key region,
//! as well as further updates to the flash protection register.
//! It also checks if it needs to halt to honor a debugger request.
//! If an image address is specified, control is transferred to the same on exit.
//!
//! @return Returns AM_HAL_STATUS_SUCCESS on success, if no image address specified
//! If an image address is provided, a successful execution results in transfer to
//! the image - and this function does not return.
//
//*****************************************************************************
uint32_t am_hal_bootloader_exit(uint32_t *pImage);

#ifdef __cplusplus
}
#endif

#endif // AM_HAL_SECURITY_H
//*****************************************************************************
//
// End Doxygen group.
//! @}
//
//*****************************************************************************
