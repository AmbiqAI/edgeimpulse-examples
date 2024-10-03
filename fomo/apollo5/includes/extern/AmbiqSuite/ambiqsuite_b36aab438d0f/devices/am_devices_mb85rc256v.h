//*****************************************************************************
//
//! @file am_devices_mb85rc256v.h
//!
//! @brief Fujitsu 256K I2C FRAM driver.
//!
//! @addtogroup mb85rc256v MB85RC256V I2C FRAM driver
//! @ingroup devices
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

#ifndef AM_DEVICES_MB85RC256V_H
#define AM_DEVICES_MB85RC256V_H

#ifdef __cplusplus
extern "C"
{
#endif

//*****************************************************************************
//
//! @name Global definitions for the commands
//! @{
//
//*****************************************************************************
#define AM_DEVICES_MB85RC256V_SLAVE_ID          0x50
#define AM_DEVICES_MB85RC256V_RSVD_SLAVE_ID     0x7C
#define AM_DEVICES_MB85RC256V_ID                0x0010A500
#define AM_DEVICES_MB85RC64TA_ID                0x0058A300
#define AM_DEVICES_MB85RC1MT_ID                 0x0058A700
//! @}

//*****************************************************************************
//
// Global type definitions.
//
//*****************************************************************************
typedef enum
{
    AM_DEVICES_MB85RC256V_STATUS_SUCCESS,
    AM_DEVICES_MB85RC256V_STATUS_ERROR
} am_devices_mb85rc256v_status_t;

typedef struct
{
    uint32_t ui32ClockFreq;
    uint32_t *pNBTxnBuf;
    uint32_t ui32NBTxnBufLength;
} am_devices_mb85rc256v_config_t;

#define AM_DEVICES_MB85RC256V_MAX_DEVICE_NUM    1

//*****************************************************************************
//
// External function definitions.
//
//*****************************************************************************

//*****************************************************************************
//
//! @brief Initialize the mb85rc256v driver.
//!
//! @param ui32Module
//! @param pDevConfig
//! @param ppHandle
//! @param ppIomHandle
//!
//! @note This function should be called before any other am_devices_mb85rc256v
//! functions. It is used to set tell the other functions how to communicate
//! with the external spiflash hardware.
//!
//! @return Status.
//
//*****************************************************************************
extern uint32_t am_devices_mb85rc256v_init(uint32_t ui32Module,
                                           am_devices_mb85rc256v_config_t *pDevConfig,
                                           void **ppHandle,
                                           void **ppIomHandle);

//*****************************************************************************
//
//! @brief De-Initialize the mb85rc256v driver.
//!
//! @param pHandle     - pointer to handle
//!
//! This function reverses the initialization
//!
//! @return Status.
//
//*****************************************************************************
extern uint32_t am_devices_mb85rc256v_term(void *pHandle);

//*****************************************************************************
//
//! @brief Reads the ID of the external flash and returns the value.
//!
//! @param pHandle   - Pointer to device handle
//! @param pDeviceID - Pointer to the return buffer for the Device ID.
//!
//! This function reads the device ID register of the external flash, and returns
//! the result as an 32-bit unsigned integer value.
//!
//! @return 32-bit status
//
//*****************************************************************************
extern uint32_t am_devices_mb85rc256v_read_id(void *pHandle,
                                              uint32_t *pDeviceID);

//*****************************************************************************
//
//! @brief Programs the given range of flash addresses.
//!
//! @param pHandle - Device handle of the external flash
//! @param pui8TxBuffer - Buffer to write the external flash data from
//! @param ui32WriteAddress - Address to write to in the external flash
//! @param ui32NumBytes - Number of bytes to write to the external flash
//!
//! This function uses the data in the provided pui8TxBuffer and copies it to
//! the external flash at the address given by ui32WriteAddress. It will copy
//! exactly ui32NumBytes of data from the original pui8TxBuffer pointer. The
//! user is responsible for ensuring that they do not overflow the target flash
//! memory or underflow the pui8TxBuffer array
//
//! @return 32-bit status
//
//*****************************************************************************
extern uint32_t am_devices_mb85rc256v_blocking_write(void *pHandle,
                                                     uint8_t *pui8TxBuffer,
                                                     uint32_t ui32WriteAddress,
                                                     uint32_t ui32NumBytes);

//*****************************************************************************
//
//! @brief Programs the given range of flash addresses.
//!
//! @param pHandle - Device handle
//! @param pui8TxBuffer - Buffer to write the external flash data from
//! @param ui32WriteAddress - Address to write to in the external flash
//! @param ui32NumBytes - Number of bytes to write to the external flash
//! @param pfnCallback  - Called when write is complete
//! @param pCallbackCtxt - Argument passed to callback
//!
//! This function uses the data in the provided pui8TxBuffer and copies it to
//! the external flash at the address given by ui32WriteAddress. It will copy
//! exactly ui32NumBytes of data from the original pui8TxBuffer pointer. The
//! user is responsible for ensuring that they do not overflow the target flash
//! memory or underflow the pui8TxBuffer array
//!
//! @return 32-bit status
//
//*****************************************************************************
extern uint32_t am_devices_mb85rc256v_nonblocking_write(void *pHandle,
                                                        uint8_t *pui8TxBuffer,
                                                        uint32_t ui32WriteAddress,
                                                        uint32_t ui32NumBytes,
                                                        am_hal_iom_callback_t pfnCallback,
                                                        void *pCallbackCtxt);

//*****************************************************************************
//
//! @brief Reads the contents of the fram into a buffer.
//!
//! @param pHandle          - Device handle
//! @param pui8RxBuffer     - Buffer to store the received data from the flash
//! @param ui32ReadAddress  - Address of desired data in external flash
//! @param ui32NumBytes     - Number of bytes to read from external flash
//!
//! This function reads the external flash at the provided address and stores
//! the received data into the provided buffer location. This function will
//! only store ui32NumBytes worth of data.
//!
//! @return 32-bit status
//
//*****************************************************************************
extern uint32_t am_devices_mb85rc256v_blocking_read(void *pHandle,
                                                    uint8_t *pui8RxBuffer,
                                                    uint32_t ui32ReadAddress,
                                                    uint32_t ui32NumBytes);

//*****************************************************************************
//
//! @brief Reads the contents of the fram into a buffer.
//!
//! @param pHandle - Device handle
//! @param pui8RxBuffer - Buffer to store the received data from the flash
//! @param ui32ReadAddress - Address of desired data in external flash
//! @param ui32NumBytes - Number of bytes to read from external flash
//! @param pfnCallback  - Called when read is complete
//! @param pCallbackCtxt - Argument passed to callback
//!
//! This function reads the external flash at the provided address and stores
//! the received data into the provided buffer location. This function will
//! only store ui32NumBytes worth of data.
//!
//! @return 32-bit status
//
//*****************************************************************************
extern uint32_t am_devices_mb85rc256v_nonblocking_read(void *pHandle,
                                                       uint8_t *pui8RxBuffer,
                                                       uint32_t ui32ReadAddress,
                                                       uint32_t ui32NumBytes,
                                                       am_hal_iom_callback_t pfnCallback,
                                                       void *pCallbackCtxt);

#ifdef __cplusplus
}
#endif

#endif // AM_DEVICES_MB85RC256V_H

//*****************************************************************************
//
// End Doxygen group.
//! @}
//
//*****************************************************************************

