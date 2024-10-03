//*****************************************************************************
//
//! @file am_devices_w25q64fw.h
//!
//! @brief Windbond SPI Flash driver
//!
//! @addtogroup w25q64fw W25Q64FW SPI Flash Driver
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

#ifndef AM_DEVICES_W25Q64FW_H
#define AM_DEVICES_W25Q64FW_H

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
#define AM_DEVICES_W25Q64FW_WRITE              0x02
#define AM_DEVICES_W25Q64FW_READ               0x03
#define AM_DEVICES_W25Q64FW_WRITE_DISABLE      0x04
#define AM_DEVICES_W25Q64FW_READ_STATUS        0x05
#define AM_DEVICES_W25Q64FW_WRITE_ENABLE       0x06
#define AM_DEVICES_W25Q64FW_FAST_READ          0x0B
#define AM_DEVICES_W25Q64FW_BLOCK_UNLOCK       0x39
#define AM_DEVICES_W25Q64FW_ENABLE_RESET       0x66
#define AM_DEVICES_W25Q64FW_RESET_DEVICE       0x99
#define AM_DEVICES_W25Q64FW_BLOCK_ERASE        0xD8
#define AM_DEVICES_W25Q64FW_READ_DEVICE_ID     0x9F
//! @}

//*****************************************************************************
//
//! @name  Global definitions for the status register
//! @{
//
//*****************************************************************************
#define AM_DEVICES_W25Q64FW_WIP         0x01        // Work In Progress
#define AM_DEVICES_W25Q64FW_WEL         0x02        // Write enable latch
//! @}

//*****************************************************************************
//
//! Global definitions for the device id.
//
//*****************************************************************************
#define AM_DEVICES_W25Q64FW_ID         0x001760EF

//*****************************************************************************
//
//! @name  Device specific definitions for the flash size information
//! @{
//
//*****************************************************************************
#define AM_DEVICES_W25Q64FW_PAGE_SIZE       0x100    //256 bytes, minimum program unit
#define AM_DEVICES_W25Q64FW_SECTOR_SIZE     0x10000  //64K bytes
#define AM_DEVICES_MSPI_FLASH_MAX_BLOCKS    128      // Sectors within 3-byte address range.
//! @}
//*****************************************************************************
//
//! @name  Global type definitions.
//! @{
//
//*****************************************************************************
typedef enum
{
    AM_DEVICES_W25Q64FW_STATUS_SUCCESS,
    AM_DEVICES_W25Q64FW_STATUS_ERROR
} am_devices_w25q64fw_status_t;

typedef struct
{
    uint32_t ui32ClockFreq;
    uint32_t *pNBTxnBuf;
    uint32_t ui32NBTxnBufLength;
} am_devices_w25q64fw_config_t;
//! @}

#define AM_DEVICES_W25Q64FW_CMD_WREN   AM_DEVICES_W25Q64FW_WRITE_ENABLE
#define AM_DEVICES_W25Q64FW_CMD_WRDI   AM_DEVICES_W25Q64FW_WRITE_DISABLE

#define AM_DEVICES_W25Q64FW_MAX_DEVICE_NUM    1

#define AM_DEVICES_W25Q64FW_MAX_TIMEOUT   100

//*****************************************************************************
//
// External function definitions.
//
//*****************************************************************************
//*****************************************************************************
//
//! @brief Initialize the w25q64fw driver.
//!
//! @param ui32Module     - IOM Module#
//! @param pDevConfig
//! @param ppHandle
//! @param ppIomHandle
//!
//! This function should be called before any other am_devices_w25q64fw
//! functions. It is used to set tell the other functions how to communicate
//! with the external SPI FRAM hardware.
//!
//! @return Status.
//
//*****************************************************************************
extern uint32_t am_devices_w25q64fw_init(uint32_t ui32Module,
                                         am_devices_w25q64fw_config_t *pDevConfig,
                                         void **ppHandle,
                                         void **ppIomHandle);

//*****************************************************************************
//
//! @brief De-Initialize the w25q64fw driver.
//!
//! @param pHandle     -
//!
//! This function reverses the initialization
//!
//! @return Status.
//
//*****************************************************************************
uint32_t extern uint32_t am_devices_w25q64fw_term(void *pHandle);

//*****************************************************************************
//
//! @brief Resets the device
//!
//! @param pHandle
//!
//! This function resets the SPI Flash device.
//!
//! @return 32-bit status
//
//*****************************************************************************
extern uint32_t am_devices_w25q64fw_reset(void *pHandle);

//*****************************************************************************
//
//! @brief Reads the ID of the external FRAM and returns the value.
//!
//! @param pHandle   - Pointer to driver handle
//! @param pDeviceID - Pointer to the return buffer for the Device ID.
//!
//! This function reads the device ID register of the external FRAM, and returns
//! the result as an 32-bit unsigned integer value.
//!
//! @return 32-bit status
//
//*****************************************************************************
extern uint32_t am_devices_w25q64fw_read_id(void *pHandle,
                                            uint32_t *pDeviceID);

//*****************************************************************************
//
//! @brief Reads the current status of the external FRAM
//!
//! @param pHandle   - Pointer to driver handle
//! @param pStatus
//!
//! This function reads the status register of the external FRAM, and returns
//! the result as an 8-bit unsigned integer value. The processor will block
//! during the data transfer process, but will return as soon as the status
//! register had been read.
//!
//! @note Macro definitions for interpreting the contents of the status register are
//! included in the header file.
//!
//! @return 8-bit status register contents
//
//*****************************************************************************
extern uint32_t am_devices_w25q64fw_status_get(void *pHandle,
                                               uint32_t *pStatus);

//*****************************************************************************
//
//! @brief Sends a specific command to the device (blocking).
//!
//! @param pHandle
//! @param ui32Cmd
//!     - AM_DEVICES_CMD_WREN
//!     - AM_DEVICES_CMD_WRDI
//!
//! @return 32-bit status
//
//*****************************************************************************
extern uint32_t am_devices_w25q64fw_command_send(void *pHandle,
                                                 uint32_t ui32Cmd);

//*****************************************************************************
//
//! @brief Erases the contents of a single sector of flash
//!
//! @param pHandle           -
//! @param ui32SectorAddress - Address to erase in the external flash
//!
//! This function erases a single sector of the external flash as specified by
//! ui32EraseAddress. The entire sector where ui32EraseAddress will
//! be erased.
//!
//! @return 32-bit status
//
//*****************************************************************************
extern uint32_t am_devcies_w25q64fw_sector_erase(void *pHandle,
                                                 uint32_t ui32SectorAddress);

//*****************************************************************************
//
//! @brief Programs the given range of FRAM addresses.
//!
//! @param pHandle - Device number of the external FRAM
//! @param pui8TxBuffer - Buffer to write the external FRAM data from
//! @param ui32WriteAddress - Address to write to in the external FRAM
//! @param ui32NumBytes - Number of bytes to write to the external FRAM
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
extern uint32_t am_devices_w25q64fw_blocking_write(void *pHandle,
                                                   uint8_t *pui8TxBuffer,
                                                   uint32_t ui32WriteAddress,
                                                   uint32_t ui32NumBytes);

//*****************************************************************************
//
//! @brief Programs the given range of FRAM addresses.
//!
//! @param pHandle
//! @param pui8TxBuffer - Buffer to write the external FRAM data from
//! @param ui32WriteAddress - Address to write to in the external FRAM
//! @param ui32NumBytes - Number of bytes to write to the external FRAM
//! @param pfnCallback
//! @param pCallbackCtxt
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
extern uint32_t am_devices_w25q64fw_nonblocking_write(void *pHandle,
                                                      uint8_t *pui8TxBuffer,
                                                      uint32_t ui32WriteAddress,
                                                      uint32_t ui32NumBytes,
                                                      am_hal_iom_callback_t pfnCallback,
                                                      void *pCallbackCtxt);

//*****************************************************************************
//
//! @brief Programs the given range of FRAM addresses.
//!
//! @param pHandle - Device number of the external FRAM
//! @param pui8TxBuffer - Buffer to write the external FRAM data from
//! @param ui32WriteAddress - Address to write to in the external FRAM
//! @param ui32NumBytes - Number of bytes to write to the external FRAM
//! @param ui32PauseCondition
//! @param ui32StatusSetClr
//! @param pfnCallback
//! @param pCallbackCtxt
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
extern uint32_t am_devices_w25q64fw_nonblocking_write_adv(void *pHandle,
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
//! @param pHandle
//! @param pui8RxBuffer - Buffer to store the received data from the FRAM
//! @param ui32ReadAddress - Address of desired data in external FRAM
//! @param ui32NumBytes - Number of bytes to read from external FRAM
//!
//! This function reads the external FRAM at the provided address and stores
//! the received data into the provided buffer location. This function will
//! only store ui32NumBytes worth of data.
//!
//! @return 32-bit status
//
//*****************************************************************************
extern uint32_t am_devices_w25q64fw_blocking_read(void *pHandle,
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
extern uint32_t am_devices_w25q64fw_nonblocking_read(void *pHandle,
                                                      uint8_t *pui8RxBuffer,
                                                      uint32_t ui32ReadAddress,
                                                      uint32_t ui32NumBytes,
                                                      am_hal_iom_callback_t pfnCallback,
                                                      void *pCallbackCtxt);

#ifdef __cplusplus
}
#endif

#endif // AM_DEVICES_W25Q64FW_H

//*****************************************************************************
//
// End Doxygen group.
//! @}
//
//*****************************************************************************

