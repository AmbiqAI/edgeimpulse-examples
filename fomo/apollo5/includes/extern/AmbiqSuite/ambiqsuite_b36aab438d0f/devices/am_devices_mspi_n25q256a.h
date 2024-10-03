//*****************************************************************************
//
//! @file am_devices_mspi_n25q256a.h
//!
//! @brief Micron Serial NOR SPI Flash driver.
//!
//! @addtogroup mspi_n25q256a N25Q256A MSPI Display Driver
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

#ifndef AM_DEVICES_MSPI_N25Q256A_H
#define AM_DEVICES_MSPI_N25Q256A_H

#ifdef __cplusplus
extern "C"
{
#endif

//*****************************************************************************
//
//! @name Global definitions for flash commands
//! @{
//
//*****************************************************************************
#define AM_DEVICES_N25Q256A_RESET_ENABLE        0x66
#define AM_DEVICES_N25Q256A_RESET_MEMORY        0x99
#define AM_DEVICES_N25Q256A_READ_ID             0x9F
#define AM_DEVICES_N25Q256A_WRITE_ENABLE        0x06
#define AM_DEVICES_N25Q256A_WRITE_DISABLE       0x04
#define AM_DEVICES_N25Q256A_READ_STATUS         0x05
#define AM_DEVICES_N25Q256A_WRITE_STATUS        0x01
#define AM_DEVICES_N25Q256A_WRITE_VOL_CFG       0x81
#define AM_DEVICES_N25Q256A_WRITE_ENHVOL_CFG    0x61
#define AM_DEVICES_N25Q256A_PAGE_PROGRAM        0x02
#define AM_DEVICES_N25Q256A_SUBSECTOR_ERASE     0x20
#define AM_DEVICES_N25Q256A_SECTOR_ERASE        0xD8
#define AM_DEVICES_N25Q256A_BULK_ERASE          0xC7
#define AM_DEVICES_N25Q256A_ENTER_4B            0xB7
#define AM_DEVICES_N25Q256A_EXIT_4B             0xE9
#define AM_DEVICES_N25Q256A_READ                0x03
#define AM_DEVICES_N25Q256A_FAST_READ           0x0B
#define AM_DEVICES_N25Q256A_READ_4B             0x13
#define AM_DEVICES_N25Q256A_READ_QUAD           0xEB
#define AM_DEVICES_N25Q256A_READ_QUAD_4B        0xEC
//! @}

//*****************************************************************************
//
//! @name Global definitions for the flash status register
//! @{
//
//*****************************************************************************
#define AM_DEVICES_N25Q256A_WEL         0x00000002        // Write enable latch
#define AM_DEVICES_N25Q256A_WIP         0x00000001        // Write in progress
//! @}

//*****************************************************************************
//
//! @name Device specific identification.
//! @{
//
//*****************************************************************************
#define AM_DEVICES_N25Q256A_ID            0x0019BA20
#define AM_DEVICES_N25Q256A_ID_MASK       0x00FFFFFF
//! @}

//*****************************************************************************
//
//! @name Global definitions for the Volatile Configuration register
//! @{
//
//*****************************************************************************
#define AM_DEVICES_N25Q256A_DUMMY_CLOCKS(n)     (((uint8_t)(n) << 4) & 0xF0)
#define AM_DEVICES_N25Q256A_XIP(n)              (((uint8_t)(n) << 3) & 0x08)
#define AM_DEVICES_N25Q256A_WRAP(n)             (((uint8_t)(n)) & 0x03)
//! @}

//*****************************************************************************
//
//! @name Global definitions for the flash size information
//! @{
//
//*****************************************************************************
#define AM_DEVICES_N25Q256A_PAGE_SIZE       0x100    //256 bytes, minimum program unit
#define AM_DEVICES_N25Q256A_SUBSECTOR_SIZE  0x1000   //4096 bytes
#define AM_DEVICES_N25Q256A_SECTOR_SIZE     0x01000  //4096 bytes.  Reduced to avoid CQ overflow.
#define AM_DEVICES_N25Q256A_MAX_SECTORS     256      // Sectors within 3-byte address range.
//! @}

//*****************************************************************************
//
//! @name Global definitions for the Enhanced Volatile Configuration register
//! @{
//
//*****************************************************************************
#define AM_DEVICES_N25Q256A_QUAD_MODE           (0x0A)
#define AM_DEVICES_N25Q256A_DUAL_MODE           (0x8A)
#define AM_DEVICES_N25Q256A_SERIAL_MODE         (0xCA)
//! @}

//*****************************************************************************
//
//! @name Global definitions for the MSPI instance to use.
//! @{
//
//*****************************************************************************
#define AM_DEVICES_N25Q256A_MSPI_INSTANCE       0

#define AM_DEVICES_MSPI_N25Q256A_MAX_DEVICE_NUM 1
//! @}

//*****************************************************************************
//
// Global type definitions.
//
//*****************************************************************************
typedef enum
{
  AM_DEVICES_N25Q256A_STATUS_SUCCESS,
  AM_DEVICES_N25Q256A_STATUS_ERROR
} am_devices_n25q256a_status_t;

typedef struct
{
    am_hal_mspi_device_e eDeviceConfig;
    am_hal_mspi_clock_e eClockFreq;
    uint32_t *pNBTxnBuf;
    uint32_t ui32NBTxnBufLength;
    uint32_t ui32ScramblingStartAddr;
    uint32_t ui32ScramblingEndAddr;
} am_devices_mspi_n25q256a_config_t;

//*****************************************************************************
//
// External function definitions.
//
//*****************************************************************************
//*****************************************************************************
//
//! @brief Initialize the n25q256a driver.
//!
//! @param ui32Module
//! @param psMSPISettings
//! @param ppHandle        - Pointer to device driver handle
//! @param ppMspiHandle    - Pointer to MSPI driver handle
//!
//! This function should be called before any other am_devices_spiflash
//! functions. It is used to set tell the other functions how to communicate
//! with the external spiflash hardware.
//!
//! The \e pfnWriteFunc and \e pfnReadFunc variables may be used to provide
//! alternate implementations of SPI write and read functions respectively. If
//! they are left set to 0, the default functions am_hal_iom_spi_write() and
//! am_hal_iom_spi_read() will be used.
//!
//! @return None.
//
//*****************************************************************************
extern uint32_t am_devices_n25q256a_init(uint32_t ui32Module,
                                         am_devices_mspi_n25q256a_config_t *psMSPISettings,
                                         void **ppHandle,
                                         void **ppMspiHandle);

//*****************************************************************************
//
//! @brief De-Initialization the n25q256a driver.
//!
//! @param pHandle        - Pointer to device driver handle
//!
//! This function should be called before any other am_devices_spiflash
//! functions. It is used to set tell the other functions how to communicate
//! with the external spiflash hardware.
//!
//! The \e pfnWriteFunc and \e pfnReadFunc variables may be used to provide
//! alternate implementations of SPI write and read functions respectively. If
//! they are left set to 0, the default functions am_hal_iom_spi_write() and
//! am_hal_iom_spi_read() will be used.
//!
//! @return None.
//
//*****************************************************************************
extern uint32_t am_devices_n25q256a_deinit(void *pHandle);

//*****************************************************************************
//
//! @brief Reads the ID of the external flash and returns the value.
//!
//! @param pHandle        - Pointer to device driver handle
//!
//! This function reads the device ID register of the external flash, and returns
//! the result as an 32-bit unsigned integer value.
//!
//! @return 32-bit status
//
//*****************************************************************************
extern uint32_t am_devices_n25q256a_id(void *pHandle);

//*****************************************************************************
//
//! @brief Reads the current status of the external flash
//!
//! @param pHandle        - Pointer to device driver handle
//!
//! This function reads the device ID register of the external flash, and returns
//! the result as an 32-bit unsigned integer value.
//!
//! @return 32-bit status
//
//*****************************************************************************
extern uint32_t am_devices_n25q256a_reset(void *pHandle);

//*****************************************************************************
//
//! @brief Reads the current status of the external flash
//!
//! @param pHandle        - Pointer to device driver handle
//! @param pStatus         - Device status returned here
//!
//! This function reads the status register of the external flash, and returns
//! the result as an 8-bit unsigned integer value. The processor will block
//! during the data transfer process, but will return as soon as the status
//! register had been read.
//!
//! Macro definitions for interpreting the contents of the status register are
//! included in the header file.
//!
//! @return 32bit call status
//
//*****************************************************************************
extern uint32_t am_devices_n25q256a_status(void *pHandle, uint32_t *pStatus);

//*****************************************************************************
//
//! @brief
//!
//! @param pHandle
//! @param pui8RxBuffer
//! @param ui32ReadAddress
//! @param ui32NumBytes
//! @param ui32PauseCondition
//! @param ui32StatusSetClr
//! @param pfnCallback
//! @param pCallbackCtxt
//!
//! @return
//
//*****************************************************************************
extern uint32_t am_devices_n25q256a_read_adv(void *pHandle, uint8_t *pui8RxBuffer,
                               uint32_t ui32ReadAddress,
                               uint32_t ui32NumBytes,
                               uint32_t ui32PauseCondition,
                               uint32_t ui32StatusSetClr,
                               am_hal_mspi_callback_t pfnCallback,
                               void *pCallbackCtxt);

//*****************************************************************************
//
//! @brief Reads the contents of the external flash into a buffer.
//!
//! @param pHandle           - Pointer to driver handle
//! @param pui8RxBuffer      - Buffer to store the received data from the flash
//! @param ui32ReadAddress   - Address of desired data in external flash
//! @param ui32NumBytes      - Number of bytes to read from external flash
//! @param bWaitForCompletion
//!
//! This function reads the external flash at the provided address and stores
//! the received data into the provided buffer location. This function will
//! only store ui32NumBytes worth of data.
//!
//! @return 32-bit status
//
//*****************************************************************************
extern uint32_t am_devices_n25q256a_read(void *pHandle, uint8_t *pui8RxBuffer,
                         uint32_t ui32ReadAddress,
                         uint32_t ui32NumBytes,
                         bool bWaitForCompletion);

//*****************************************************************************
//
//! @brief Reads the contents of the external flash into a buffer.
//!
//! @param pHandle              - Pointer to driver handle
//! @param pui8RxBuffer         - Buffer to store the received data from the flash
//! @param ui32ReadAddress      - Address of desired data in external flash
//! @param ui32NumBytes         - Number of bytes to read from external flash
//! @param bWaitForCompletion
//!
//! This function reads the external flash at the provided address and stores
//! the received data into the provided buffer location. This function will
//! only store ui32NumBytes worth of data.
//
//! @return 32-bit status
//
//*****************************************************************************
extern uint32_t am_devices_n25q256a_read_hiprio(void *pHandle, uint8_t *pui8RxBuffer,
                        uint32_t ui32ReadAddress,
                        uint32_t ui32NumBytes,
                        bool bWaitForCompletion);

// ##### INTERNAL BEGIN #####
//*****************************************************************************
//
//! @brief
//!
//! @param pHandle
//! @param pui8RxBuffer
//! @param ui32ReadAddress
//! @param ui32NumBytes
//!
//! @return
//
//*****************************************************************************
extern uint32_t am_devices_n25q256a_read_callback(void *pHandle, uint8_t *pui8RxBuffer,
                           uint32_t ui32ReadAddress,
                           uint32_t ui32NumBytes);
// ##### INTERNAL END #####

//*****************************************************************************
//
//! @brief Programs the given range of flash addresses.
//!
//! @param pHandle              - Pointer to driver handle
//! @param pui8TxBuffer         - Buffer to write the external flash data from
//! @param ui32WriteAddress     - Address to write to in the external flash
//! @param ui32NumBytes         - Number of bytes to write to the external flash
//! @param bWaitForCompletion
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
extern uint32_t am_devices_n25q256a_write(void *pHandle,
                                          uint8_t *pui8TxBuffer,
                                          uint32_t ui32WriteAddress,
                                          uint32_t ui32NumBytes,
                                          bool bWaitForCompletion);

//*****************************************************************************
//
//! @brief Erases the entire contents of the external flash
//!
//! @param pHandle              - Pointer to driver handle
//!
//! This function uses the "Bulk Erase" instruction to erase the entire
//! contents of the external flash.
//!
//! @return 32-bit status
//
//*****************************************************************************
extern uint32_t am_devices_n25q256a_mass_erase(void *pHandle);

//*****************************************************************************
//
//! @brief Erases the contents of a single sector of flash
//!
//! @param pHandle              - Pointer to driver handle
//! @param ui32SectorAddress - Address to erase in the external flash
//!
//! This function erases a single sector of the external flash as specified by
//! ui32EraseAddress. The entire sector where ui32EraseAddress will
//! be erased.
//!
//! @return 32-bit status
//
//*****************************************************************************
extern uint32_t am_devices_n25q256a_sector_erase(void *pHandle, uint32_t ui32SectorAddress);

//*****************************************************************************
//
//! @brief Sets up the MSPI and external FLASH into XIP mode.
//!
//! @param pHandle              - Pointer to driver handle
//!
//! This function sets the external psram device and the MSPI into XIP mode.
//!
//! @return 32-bit status
//
//*****************************************************************************
extern uint32_t am_devices_n25q256a_enable_xip(void *pHandle);

//*****************************************************************************
//
//! @brief Removes the MSPI and external FLASH from XIP mode.
//!
//! @param pHandle              - Pointer to driver handle
//!
//! This function removes the external device and the MSPI from XIP mode.
//!
//! @return 32-bit status
//
//*****************************************************************************
extern uint32_t am_devices_n25q256a_disable_xip(void *pHandle);

//*****************************************************************************
//
//! @brief Sets up the MSPI and external FLASH into scrambling mode.
//!
//! @param pHandle              - Pointer to driver handle
//!
//! This function sets the external psram device and the MSPI into scrambling mode.
//!
//! @return 32-bit status
//
//*****************************************************************************
extern uint32_t am_devices_n25q256a_enable_scrambling(void *pHandle);

//*****************************************************************************
//
//! @brief Removes the MSPI and external FLASH from scrambling mode.
//!
//! @param pHandle              - Pointer to driver handle
//!
//! This function removes the external device and the MSPI from scrambling mode.
//!
//! @return 32-bit status
//
//*****************************************************************************
extern uint32_t am_devices_n25q256a_disable_scrambling(void *pHandle);

#ifdef __cplusplus
}
#endif

#endif // AM_DEVICES_MSPI_N25Q256A_H

//*****************************************************************************
//
// End Doxygen group.
//! @}
//
//*****************************************************************************

