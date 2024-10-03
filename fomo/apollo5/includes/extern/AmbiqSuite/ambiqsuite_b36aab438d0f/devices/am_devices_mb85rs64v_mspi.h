//*****************************************************************************
//
//! @file am_devices_mb85rs64v_mspi.h
//!
//! @brief Fujitsu 64K SPI FRAM driver.
//!
//! @addtogroup mb85rs64v_mspi MB85RS64V MSPI FRAM Driver
//! @ingroup devices
//! @{
//
//**************************************************************************

//*****************************************************************************
//
// ${copyright}
//
// This is part of revision ${version} of the AmbiqSuite Development Package.
//
//*****************************************************************************

#ifndef AM_DEVICES_MB85RS64V_MSPI_H
#define AM_DEVICES_MB85RS64V_MSPI_H

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
#define AM_DEVICES_MB85RS64V_WRITE_ENABLE       0x06
#define AM_DEVICES_MB85RS64V_WRITE_DISABLE      0x04
#define AM_DEVICES_MB85RS64V_READ_STATUS        0x05
#define AM_DEVICES_MB85RS64V_WRITE_STATUS       0x01
#define AM_DEVICES_MB85RS64V_READ               0x03
#define AM_DEVICES_MB85RS64V_WRITE              0x02
#define AM_DEVICES_MB85RS64V_READ_DEVICE_ID     0x9F
//! @}

//*****************************************************************************
//
//! @name Global definitions for the status register
//! @{
//
//*****************************************************************************
#define AM_DEVICES_MB85RS64V_WPEN        0x80        // Write pending status
#define AM_DEVICES_MB85RS64V_WEL         0x02        // Write enable latch
//! @}

//*****************************************************************************
//
//! Global definitions for the device id.
//
//*****************************************************************************
#define AM_DEVICES_MB85RS64V_ID         0x02037F04  //0x047F0302
#define AM_DEVICES_MB85RS64V_SIZE       (8 * 1024)

//*****************************************************************************
//
//! Global definitions for the MSPI instance to use.
//
//*****************************************************************************
#define AM_DEVICES_MSPI_FRAM_MAX_DEVICE_NUM    1

//*****************************************************************************
//
// Global type definitions.
//
//*****************************************************************************
typedef enum
{
    AM_DEVICES_MB85RS64V_STATUS_SUCCESS,
    AM_DEVICES_MB85RS64V_STATUS_ERROR
} am_devices_mb85rs64v_status_t;

#define AM_DEVICES_MB85RS64V_CMD_WREN   AM_DEVICES_MB85RS64V_WRITE_ENABLE
#define AM_DEVICES_MB85RS64V_CMD_WRDI   AM_DEVICES_MB85RS64V_WRITE_DISABLE

typedef struct
{
    am_hal_mspi_device_e eDeviceConfig;
    am_hal_mspi_clock_e eClockFreq;
    uint32_t *pNBTxnBuf;
    uint32_t ui32NBTxnBufLength;
    uint32_t ui32ScramblingStartAddr;
    uint32_t ui32ScramblingEndAddr;
} am_devices_mspi_fram_config_t;

//*****************************************************************************
//
// External function definitions.
//
//*****************************************************************************

//*****************************************************************************
//
//! @brief Initialize the mb85rs64v driver.
//!
//! @param ui32Module     - IOM Module#
//! @param psMSPIConfig   - Pointer to config structure
//! @param ppHandle       - Pointer to driver handle pointer
//! @param ppMSPIHandle   - Pointer to MSPI driver handle pointer
//!
//! This function should be called before any other am_devices_mb85rs64v
//! functions. It is used to set tell the other functions how to communicate
//! with the external spiflash hardware.
//!
//! @return Status.
//
//*****************************************************************************
extern uint32_t am_devices_mb85rs64v_mspi_init(uint32_t ui32Module,
                                               am_devices_mspi_fram_config_t *psMSPIConfig,
                                               void **ppHandle,
                                               void **ppMSPIHandle);

//*****************************************************************************
//
//! @brief De-Initialize the mb85rs64v driver.
//!
//! @param pHandle     - Pointer to driver handle
//!
//! This function reverses the initialization
//!
//! @return Status.
//
//*****************************************************************************
extern uint32_t am_devices_mb85rs64v_mspi_deinit(void *pHandle);

//*****************************************************************************
//
//! @brief Reads the ID of the external flash and returns the value.
//!
//! @param pHandle      - Pointer to driver handle
//!
//! This function reads the device ID register of the external flash, and returns
//! the result as an 32-bit unsigned integer value.
//!
//! @return 32-bit status
//
//*****************************************************************************
extern uint32_t am_devices_mb85rs64v_mspi_read_id(void *pHandle);

//*****************************************************************************
//
//! @brief Reads the current status of the external flash
//!
//! This function reads the status register of the external flash, and returns
//! the result as an 8-bit unsigned integer value. The processor will block
//! during the data transfer process, but will return as soon as the status
//! register had been read.
//!
//! Macro definitions for interpreting the contents of the status register are
//! included in the header file.
//!
//! @return 8-bit status register contents
//
//*****************************************************************************
extern uint32_t am_devices_mb85rs64v_mspi_status_get(void *pHandle, uint32_t *pStatus);

//*****************************************************************************
//
//! @brief Sends a specific command to the device (blocking).
//!
//! @param pHandle      - Pointer to driver handle
//! @param ui32Cmd
//!         - AM_DEVICES_CMD_WREN
//!         - AM_DEVICES_CMD_WRDI
//!
//! @return 32-bit status
//
//*****************************************************************************
extern uint32_t am_devices_mb85rs64v_mspi_command_send(void *pHandle, uint32_t ui32Cmd);

//*****************************************************************************
//
//! @brief Programs the given range of flash addresses.
//!
//! @param pHandle      - Pointer to driver handle
//! @param pui8TxBuffer - Buffer to write the external flash data from
//! @param ui32WriteAddress - Address to write to in the external flash
//! @param ui32NumBytes - Number of bytes to write to the external flash
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
extern uint32_t am_devices_mb85rs64v_mspi_blocking_write(void *pHandle,
                                                    uint8_t *pui8TxBuffer,
                                                    uint32_t ui32WriteAddress,
                                                    uint32_t ui32NumBytes);

//*****************************************************************************
//
//! @brief Programs the given range of flash addresses.
//!
//! @param pHandle      - Pointer to driver handle
//! @param pui8TxBuffer - Buffer to write the external flash data from
//! @param ui32WriteAddress - Address to write to in the external flash
//! @param ui32NumBytes - Number of bytes to write to the external flash
//! @param bWaitForCompletion - Block and wait for transfer completetion
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
extern uint32_t am_devices_mb85rs64v_mspi_write(void *pHandle, uint8_t *pui8TxBuffer,
                                            uint32_t ui32WriteAddress,
                                            uint32_t ui32NumBytes,
                                            bool bWaitForCompletion);

//*****************************************************************************
//
//! @brief Reads the contents of the fram into a buffer.
//!
//! @param pHandle      - Pointer to driver handle
//! @param pui8RxBuffer - Buffer to store the received data from the flash
//! @param ui32ReadAddress - Address of desired data in external flash
//! @param ui32NumBytes - Number of bytes to read from external flash
//!
//! This function reads the external flash at the provided address and stores
//! the received data into the provided buffer location. This function will
//! only store ui32NumBytes worth of data.
//!
//! @return 32-bit status
//
//*****************************************************************************
extern uint32_t am_devices_mb85rs64v_mspi_blocking_read(void *pHandle, uint8_t *pui8RxBuffer,
                                                   uint32_t ui32ReadAddress,
                                                   uint32_t ui32NumBytes);

//*****************************************************************************
//
//! @brief Reads the contents of the fram into a buffer.
//!
//! @param pHandle      - Pointer to driver handle
//! @param pui8RxBuffer - Buffer to store the received data from the flash
//! @param ui32ReadAddress - Address of desired data in external flash
//! @param ui32NumBytes - Number of bytes to read from external flash
//! @param bWaitForCompletion - Block and wait for transfer completetion
//!
//! This function reads the external flash at the provided address and stores
//! the received data into the provided buffer location. This function will
//! only store ui32NumBytes worth of data.
//!
//! @return 32-bit status
//
//*****************************************************************************
extern uint32_t am_devices_mb85rs64v_mspi_read(void *pHandle, uint8_t *pui8RxBuffer,
                                                  uint32_t ui32ReadAddress,
                                                  uint32_t ui32NumBytes,
                                                  bool bWaitForCompletion);

#ifdef __cplusplus
}
#endif

#endif // AM_DEVICES_MB85RS64V_MSPI_H

//*****************************************************************************
//
// End Doxygen group.
//! @}
//
//*****************************************************************************

