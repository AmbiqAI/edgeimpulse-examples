//*****************************************************************************
//
//! @file am_devices_mb85rq4ml.h
//!
//! @brief Fujitsu 512K SPI FRAM driver.
//!
//! @addtogroup mb85rq4ml MB85RQ4ML SPI FRAM driver
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

#ifndef AM_DEVICES_MB85RQ4ML_H
#define AM_DEVICES_MB85RQ4ML_H

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
#define AM_DEVICES_MB85RQ4ML_WRITE_STATUS       0x01
#define AM_DEVICES_MB85RQ4ML_WRITE              0x02
#define AM_DEVICES_MB85RQ4ML_READ               0x03
#define AM_DEVICES_MB85RQ4ML_WRITE_DISABLE      0x04
#define AM_DEVICES_MB85RQ4ML_READ_STATUS        0x05
#define AM_DEVICES_MB85RQ4ML_WRITE_ENABLE       0x06
#define AM_DEVICES_MB85RQ4ML_FAST_READ          0x0B
#define AM_DEVICES_MB85RQ4ML_READ_DEVICE_ID     0x9F
//! @}

//*****************************************************************************
//
//! @name Global definitions for the status register
//! @{
//
//*****************************************************************************
#define AM_DEVICES_MB85RQ4ML_WPEN        0x80        // Write pending status
#define AM_DEVICES_MB85RQ4ML_WEL         0x02        // Write enable latch
//! @}

//*****************************************************************************
//
//! Global definitions for the device id.
//
//*****************************************************************************
#define AM_DEVICES_MB85RQ4ML_ID         0x85297F04  //0x047F2985

//*****************************************************************************
//
// Global type definitions.
//
//*****************************************************************************
typedef enum
{
    AM_DEVICES_MB85RQ4ML_STATUS_SUCCESS,
    AM_DEVICES_MB85RQ4ML_STATUS_ERROR
} am_devices_mb85rq4ml_status_t;

typedef struct
{
    uint32_t ui32ClockFreq;
    uint32_t *pNBTxnBuf;
    uint32_t ui32NBTxnBufLength;
} am_devices_mb85rq4ml_config_t;

#define AM_DEVICES_MB85RQ4ML_CMD_WREN   AM_DEVICES_MB85RQ4ML_WRITE_ENABLE
#define AM_DEVICES_MB85RQ4ML_CMD_WRDI   AM_DEVICES_MB85RQ4ML_WRITE_DISABLE

#define AM_DEVICES_MB85RQ4ML_MAX_DEVICE_NUM    1

//*****************************************************************************
//
// External function definitions.
//
//*****************************************************************************

//*****************************************************************************
//
//! @brief Initialize the mb85rq4ml driver.
//!
//! @param ui32Module     - IOM Module#
//! @param pDevConfig
//! @param ppHandle
//! @param ppIomHandle
//!
//! @note This function should be called before any other am_devices_mb85rq4ml
//! functions. It is used to set tell the other functions how to communicate
//! with the external SPI FRAM hardware.
//!
//! @return Status.
//
//*****************************************************************************
extern uint32_t am_devices_mb85rq4ml_init(uint32_t ui32Module,
                                          am_devices_mb85rq4ml_config_t *pDevConfig,
                                          void **ppHandle,
                                          void **ppIomHandle);

//*****************************************************************************
//
//! @brief De-Initialize the mb85rq4ml driver.
//!
//! @param pHandle     - Pointer to Device driver handle
//!
//! This function reverses the initialization
//!
//! @return Status.
//
//*****************************************************************************
extern uint32_t am_devices_mb85rq4ml_term(void *pHandle);

//*****************************************************************************
//
//! @brief Reads the ID of the external FRAM and returns the value.
//!
//! @param pHandle   = Pointer to driver handle
//! @param pDeviceID - Pointer to the return buffer for the Device ID.
//!
//! This function reads the device ID register of the external FRAM, and returns
//! the result as an 32-bit unsigned integer value.
//!
//! @return 32-bit status
//
//*****************************************************************************
extern uint32_t am_devices_mb85rq4ml_read_id(void *pHandle,
                                             uint32_t *pDeviceID);

//*****************************************************************************
//
//! @brief Reads the current status of the external FRAM
//!
//! @param pHandle      - Pointer to driver handle
//! @param pStatus      - Status of FRAM is returned here
//!
//! This function reads the status register of the external FRAM, and returns
//! the result as an 8-bit unsigned integer value. The processor will block
//! during the data transfer process, but will return as soon as the status
//! register had been read.
//!
//! Macro definitions for interpreting the contents of the status register are
//! included in the header file.
//!
//! @return call status from am_devices_mb85rq4ml_status_t;
//
//*****************************************************************************
extern uint32_t am_devices_mb85rq4ml_status_get(void *pHandle,
                                                uint32_t *pStatus);

//*****************************************************************************
//
//! @brief Sends a specific command to the device (blocking).
//!
//! @param pHandle - pointer to driver handle
//! @param ui32Cmd
//!     - AM_DEVICES_CMD_WREN
//!     - AM_DEVICES_CMD_WRDI
//!
//! @return 32-bit status
//
//*****************************************************************************
extern uint32_t am_devices_mb85rq4ml_command_send(void *pHandle,
                                                  uint32_t ui32Cmd);

//*****************************************************************************
//
//! @brief Programs the given range of FRAM addresses.
//!
//! @param pHandle              - Pointer to Device handle
//! @param pui8TxBuffer         - Buffer to write the external FRAM data from
//! @param ui32WriteAddress     - Address to write to in the external FRAM
//! @param ui32NumBytes         - Number of bytes to write to the external FRAM
//!
//! This function uses the data in the provided pui8TxBuffer and copies it to
//! the external FRAM at the address given by ui32WriteAddress. It will copy
//! exactly ui32NumBytes of data from the original pui8TxBuffer pointer. The
//! user is responsible for ensuring that they do not overflow the target FRAM
//! memory or underflow the pui8TxBuffer array
//!
//! @return 32-bit status
//
//*****************************************************************************
extern uint32_t am_devices_mb85rq4ml_blocking_write(void *pHandle,
                                                    uint8_t *pui8TxBuffer,
                                                    uint32_t ui32WriteAddress,
                                                    uint32_t ui32NumBytes);

//*****************************************************************************
//
//! @brief Programs the given range of FRAM addresses.
//!
//! @param pHandle      - Pointer to device handle
//! @param pui8TxBuffer - Buffer to write the external FRAM data from
//! @param ui32WriteAddress - Address to write to in the external FRAM
//! @param ui32NumBytes - Number of bytes to write to the external FRAM
//! @param pfnCallback  - Callback function
//! @param pCallbackCtxt - Value passed to callback function
//!
//! This function uses the data in the provided pui8TxBuffer and copies it to
//! the external FRAM at the address given by ui32WriteAddress. It will copy
//! exactly ui32NumBytes of data from the original pui8TxBuffer pointer. The
//! user is responsible for ensuring that they do not overflow the target FRAM
//! memory or underflow the pui8TxBuffer array
//!
//! @return 32-bit status
//
//*****************************************************************************
extern uint32_t am_devices_mb85rq4ml_nonblocking_write(void *pHandle,
                                                       uint8_t *pui8TxBuffer,
                                                       uint32_t ui32WriteAddress,
                                                       uint32_t ui32NumBytes,
                                                       am_hal_iom_callback_t pfnCallback,
                                                       void *pCallbackCtxt);

//*****************************************************************************
//
//! @brief Programs the given range of FRAM addresses.
//!
//! @param pHandle      - Pointer to device handle
//! @param pui8TxBuffer - Buffer to write the external FRAM data from
//! @param ui32WriteAddress - Address to write to in the external FRAM
//! @param ui32NumBytes - Number of bytes to write to the external FRAM
//! @param ui32PauseCondition
//! @param ui32StatusSetClr
//! @param pfnCallback  - Callback function
//! @param pCallbackCtxt - Value passed to callback function
//!
//! This function uses the data in the provided pui8TxBuffer and copies it to
//! the external FRAM at the address given by ui32WriteAddress. It will copy
//! exactly ui32NumBytes of data from the original pui8TxBuffer pointer. The
//! user is responsible for ensuring that they do not overflow the target FRAM
//! memory or underflow the pui8TxBuffer array
//!
//! @return 32-bit status
//
//*****************************************************************************
extern uint32_t am_devices_mb85rq4ml_nonblocking_write_adv(void *pHandle,
                                       uint8_t *pui8TxBuffer,
                                       uint32_t ui32WriteAddress,
                                       uint32_t ui32NumBytes,
                                       uint32_t ui32PauseCondition,
                                       uint32_t ui32StatusSetClr,
                                       am_hal_iom_callback_t pfnCallback,
                                       void *pCallbackCtxt);

//*****************************************************************************
//
//! @brief Reads the contents of the fram into a buffer.
//!
//! @param pHandle          - Pointer to driver handle
//! @param pui8RxBuffer     - Buffer to store the received data from the FRAM
//! @param ui32ReadAddress - Address of desired data in external FRAM
//! @param ui32NumBytes     - Number of bytes to read from external FRAM
//!
//! This function reads the external FRAM at the provided address and stores
//! the received data into the provided buffer location. This function will
//! only store ui32NumBytes worth of data.
//!
//! @return 32-bit status
//
//*****************************************************************************
extern uint32_t am_devices_mb85rq4ml_blocking_read(void *pHandle,
                                                   uint8_t *pui8RxBuffer,
                                                   uint32_t ui32ReadAddress,
                                                   uint32_t ui32NumBytes);

//*****************************************************************************
//
//! @brief Reads the contents of the fram into a buffer.
//!
//! @param pHandle
//! @param pui8RxBuffer - Buffer to store the received data from the FRAM
//! @param ui32ReadAddress - Address of desired data in external FRAM
//! @param ui32NumBytes - Number of bytes to read from external FRAM
//! @param pfnCallback
//! @param pCallbackCtxt
//!
//! This function reads the external FRAM at the provided address and stores
//! the received data into the provided buffer location. This function will
//! only store ui32NumBytes worth of data.
//!
//! @return 32-bit status
//
//*****************************************************************************
extern uint32_t am_devices_mb85rq4ml_nonblocking_read(void *pHandle,
                                                      uint8_t *pui8RxBuffer,
                                                      uint32_t ui32ReadAddress,
                                                      uint32_t ui32NumBytes,
                                                      am_hal_iom_callback_t pfnCallback,
                                                      void *pCallbackCtxt);

#ifdef __cplusplus
}
#endif

#endif // AM_DEVICES_MB85RQ4ML_H

//*****************************************************************************
//
// End Doxygen group.
//! @}
//
//*****************************************************************************

