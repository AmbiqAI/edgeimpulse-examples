//*****************************************************************************
//
//! @file am_devices_mspi_flash.h
//!
//! @brief General Multibit SPI Flash driver.
//!
//! @addtogroup mspi_flash Generic MSPI FLASH Driver
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

#ifndef AM_DEVICES_MSPI_FLASH_H
#define AM_DEVICES_MSPI_FLASH_H

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
#define AM_DEVICES_MSPI_FLASH_WRITE_STATUS      0x01
#define AM_DEVICES_MSPI_FLASH_PAGE_PROGRAM      0x02
#define AM_DEVICES_MSPI_FLASH_READ              0x03
#define AM_DEVICES_MSPI_FLASH_WRITE_DISABLE     0x04
#define AM_DEVICES_MSPI_FLASH_READ_STATUS       0x05
#define AM_DEVICES_MSPI_FLASH_WRITE_ENABLE      0x06
#define AM_DEVICES_MSPI_FLASH_FAST_READ         0x0B
#define AM_DEVICES_MSPI_FLASH_READ_4B           0x13
#define AM_DEVICES_MSPI_FLASH_SUBSECTOR_ERASE   0x20
#define AM_DEVICES_MSPI_FLASH_DUAL_READ         0x3B
#define AM_DEVICES_MSPI_FLASH_DUAL_IO_READ      0xBB
#define AM_DEVICES_MSPI_FLASH_WRITE_ENHVOL_CFG  0x61
#define AM_DEVICES_MSPI_FLASH_RESET_ENABLE      0x66
#define AM_DEVICES_MSPI_FLASH_QUAD_READ         0x6B
#define AM_DEVICES_MSPI_FLASH_WRITE_VOL_CFG     0x81
#define AM_DEVICES_MSPI_FLASH_RESET_MEMORY      0x99
#define AM_DEVICES_MSPI_FLASH_READ_ID           0x9F
#define AM_DEVICES_MSPI_FLASH_ENTER_4B          0xB7
#define AM_DEVICES_MSPI_FLASH_BULK_ERASE        0xC7
#define AM_DEVICES_MSPI_FLASH_SECTOR_ERASE      0xD8
#define AM_DEVICES_MSPI_FLASH_EXIT_4B           0xE9
#define AM_DEVICES_MSPI_FLASH_QUAD_IO_READ      0xEB
#define AM_DEVICES_MSPI_FLASH_READ_QUAD_4B      0xEC

//! @}

//*****************************************************************************
//
//! @name Global definitions for the flash status register
//! @{
//
//*****************************************************************************
#define AM_DEVICES_MSPI_FLASH_WEL         0x00000002        // Write enable latch
#define AM_DEVICES_MSPI_FLASH_WIP         0x00000001        // Write in progress

//
// The following definitions are typically specific to a multibit spi flash device.
// They should be tailored by the example or testcase (i.e., defined in the project).
//
//#define MICRON_N25Q256A
//#define CYPRESS_S25FS064S
//#define MACRONIX_MX25U12835F
//#define ADESTO_ATXP032
//! @}

#if defined (MICRON_N25Q256A)
//*****************************************************************************
//
//! @name Device specific identification.
//! @{
//
//*****************************************************************************
#define AM_DEVICES_MSPI_FLASH_ID            0x0019BA20
#define AM_DEVICES_MSPI_FLASH_ID_MASK       0x00FFFFFF
//! @}

//*****************************************************************************
//
//! @name  Device specific definitions for flash commands
//! @{
//
//*****************************************************************************
// None.
//*****************************************************************************
//! @}
//
//! @name  Device specific definitions for the Configuration register(s)
//! @{
//
//*****************************************************************************
#define AM_DEVICES_MSPI_FLASH_DUMMY_CLOCKS(n)     (((uint8_t)(n) << 4) & 0xF0)
#define AM_DEVICES_MSPI_FLASH_XIP(n)              (((uint8_t)(n) << 3) & 0x08)
#define AM_DEVICES_MSPI_FLASH_WRAP(n)             (((uint8_t)(n)) & 0x03)
#define AM_DEVICES_MSPI_FLASH_QUAD_MODE           (0x0A)
#define AM_DEVICES_MSPI_FLASH_DUAL_MODE           (0x8A)
#define AM_DEVICES_MSPI_FLASH_SERIAL_MODE         (0xCA)
//! @}

//*****************************************************************************
//
//! @name  Device specific definitions for the flash size information
//! @{
//
//*****************************************************************************
#define AM_DEVICES_MSPI_FLASH_PAGE_SIZE       0x100    //256 bytes, minimum program unit
#define AM_DEVICES_MSPI_FLASH_SUBSECTOR_SIZE  0x1000   //4K bytes
#define AM_DEVICES_MSPI_FLASH_SECTOR_SIZE     0x10000  //64K bytes
#define AM_DEVICES_MSPI_FLASH_MAX_SECTORS     256      // Sectors within 3-byte address range.
//! @}
#endif

#if defined (CYPRESS_S25FS064S)
//*****************************************************************************
//
//! @name Device specific identification.
//! @{
//
//*****************************************************************************
#define AM_DEVICES_MSPI_FLASH_ID        0x00170201
#define AM_DEVICES_MSPI_FLASH_ID_MASK   0x00FFFFFF
//! @}

//*****************************************************************************
//
//! @name Device specific definitions for flash commands
//! @{
//
//*****************************************************************************
#define AM_DEVICES_MSPI_FLASH_READ_STATUS2              0x07
#define AM_DEVICES_MSPI_FLASH_WRITE_ANY_REG             0x71
#define AM_DEVICES_MSPI_FLASH_EVAL_ERASE_STATUS         0xD0
//! @}

//*****************************************************************************
//
//! @name Device specific definitions for the Configuration register(s)
//! @{
//
//*****************************************************************************
#define AM_DEVICES_MSPI_FLASH_QUAD_MODE           (0x02)
#define AM_DEVICES_MSPI_FLASH_DUAL_MODE           (0x00)
#define AM_DEVICES_MSPI_FLASH_SERIAL_MODE         (0x00)
#define AM_DEVICES_MSPI_FLASH_CR1V                (0x00020080)
#define AM_DEVICES_MSPI_FLASH_CR1V_VALUE(n)       (((uint32_t)(n) << 24) & 0xFF000000)
#define AM_DEVICES_MSPI_FLASH_CR2V                (0x00030080)
#define AM_DEVICES_MSPI_FLASH_CR2V_VALUE(n)       (((uint32_t)(n) << 24) & 0xFF000000)
#define AM_DEVICES_MSPI_FLASH_ADDR_QUAD_EXT_DELAY (2)
#define AM_DEVICES_MSPI_FLASH_ADDR_DUAL_EXT_DELAY (4)
#define AM_DEVICES_MSPI_FLASH_CR3V                (0x00040080)
#define AM_DEVICES_MSPI_FLASH_CR4V                (0x00050080)
#define AM_DEVICES_MSPI_FLASH_CR3V_VALUE          (((uint32_t)(0x08) << 24) & 0xFF000000)
#define AM_DEVICES_MSPI_FLASH_CR4V_VALUE          (((uint32_t)(0x10) << 24) & 0xFF000000)
#define AM_DEVICES_MSPI_FLASH_ERASE_SUCCESS       (0x04)
//! @}

//*****************************************************************************
//
//! @name Device specific definitions for the flash size information
//! @{
//
//*****************************************************************************
#define AM_DEVICES_MSPI_FLASH_PAGE_SIZE       0x100    //256 bytes, minimum program unit
#define AM_DEVICES_MSPI_FLASH_SUBSECTOR_SIZE  0x1000   //4K bytes
#define AM_DEVICES_MSPI_FLASH_SECTOR_SIZE     0x10000  //64K bytes.
#define AM_DEVICES_MSPI_FLASH_MAX_SECTORS     128      // Sectors within 3-byte address range.
//! @}
#endif

#if defined (MACRONIX_MX25U12835F)
//*****************************************************************************
//
//! @name Device specific identification.
//! @{
//
//*****************************************************************************
#define AM_DEVICES_MSPI_FLASH_ID        0x003825C2
#define AM_DEVICES_MSPI_FLASH_ID_MASK   0x00FFFFFF
//! @}

//*****************************************************************************
//
//! @name Device specific definitions for flash commands
//! @{
//
//*****************************************************************************
#define AM_DEVICES_MSPI_FLASH_READ_CONFIG               0x15
#define AM_DEVICES_MSPI_FLASH_ENABLE_QPI_MODE           0x35
#define AM_DEVICES_MSPI_FLASH_DISABLE_QPI_MODE          0xF5
//! @}

//*****************************************************************************
//
//! @name Device specific definitions for the Configuration register(s)
//! @{
//
//*****************************************************************************
#define AM_DEVICES_MSPI_FLASH_STATUS              (0x00)
#define AM_DEVICES_MSPI_FLASH_CONFIG              (0x07)
//! @}

//*****************************************************************************
//
//! @name Device specific definitions for the flash size information
//! @{
//
//*****************************************************************************
#define AM_DEVICES_MSPI_FLASH_PAGE_SIZE       0x100    //256 bytes, minimum program unit
#define AM_DEVICES_MSPI_FLASH_SECTOR_SIZE     0x1000   //4K bytes
#define AM_DEVICES_MSPI_FLASH_BLOCK_SIZE      0x10000  //64K bytes.
#define AM_DEVICES_MSPI_FLASH_MAX_SECTORS     4096     // Sectors within 3-byte address range.
#define AM_DEVICES_MSPI_FLASH_MAX_BLOCKS      256
//! @}
#endif

#if defined (ADESTO_ATXP032)
//*****************************************************************************
//
//! @name Device specific identification.
//! @{
//
//*****************************************************************************
#define AM_DEVICES_MSPI_FLASH_ID        0x0043A700
#define AM_DEVICES_MSPI_FLASH_ID_MASK   0x00FFFFFF
//! @}
//*****************************************************************************
//
//! @name Device specific definitions for flash commands
//! @{
//
//*****************************************************************************
#define AM_DEVICES_ATXP032_ENTER_QUAD_MODE      0x38
#define AM_DEVICES_ATXP032_UNPROTECT_SECTOR     0x39
#define AM_DEVICES_ATXP032_WRITE_STATUS_CTRL    0x71
#define AM_DEVICES_ATXP032_ENTER_OCTAL_MODE     0xE8
#define AM_DEVICES_ATXP032_RETURN_TO_SPI_MODE   0xFF
//! @}
//*****************************************************************************
//
//! @name Device specific definitions for the flash size information
//! @{
//
//*****************************************************************************
#define AM_DEVICES_MSPI_FLASH_PAGE_SIZE       0x100    //256 bytes, minimum program unit
//#define AM_DEVICES_MSPI_FLASH_SECTOR_SIZE     0x10000   //64K bytes
#define AM_DEVICES_MSPI_FLASH_SECTOR_SIZE     0x1000   //4K bytes
#define AM_DEVICES_MSPI_FLASH_MAX_BLOCKS      256
#define AM_DEVICES_MSPI_FLASH_MAX_SECTORS     256      // Sectors within 4-byte address range.
//! @}

//*****************************************************************************
//
//! @name Global definitions for the flash status register
//! @{
//
//*****************************************************************************
#define AM_DEVICES_ATXP032_RSTE        0x00000010        // Reset enable
#define AM_DEVICES_ATXP032_WEL         0x00000002        // Write enable latch
#define AM_DEVICES_ATXP032_WIP         0x00000001        // Operation in progress
//! @}

#endif

//*****************************************************************************
//
//! @name Global definitions for the MSPI instance to use.
//! @{
//
//*****************************************************************************
#define AM_DEVICES_MSPI_FLASH_MSPI_INSTANCE     0

#define AM_DEVICES_MSPI_FLASH_MAX_DEVICE_NUM    1
//! @}

//*****************************************************************************
//
// Global type definitions.
//
//*****************************************************************************
typedef enum
{
    AM_DEVICES_MSPI_FLASH_STATUS_SUCCESS,
    AM_DEVICES_MSPI_FLASH_STATUS_ERROR
} am_devices_mspi_flash_status_t;

typedef struct
{
    am_hal_mspi_device_e eDeviceConfig;
    am_hal_mspi_clock_e eClockFreq;
    am_hal_mspi_xipmixed_mode_e eMixedMode;
    uint32_t *pNBTxnBuf;
    uint32_t ui32NBTxnBufLength;
    uint32_t ui32ScramblingStartAddr;
    uint32_t ui32ScramblingEndAddr;
} am_devices_mspi_flash_config_t;

//*****************************************************************************
//
// External function definitions.
//
//*****************************************************************************
//*****************************************************************************
//
//! @brief Initialize the mspi_flash driver.
//!
//! @param ui32Module
//! @param psMSPISettings
//! @param ppHandle
//! @param ppMspiHandle
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
//! @return status.
//
//*****************************************************************************
extern uint32_t am_devices_mspi_flash_init(uint32_t ui32Module,
                                           am_devices_mspi_flash_config_t *psMSPISettings,
                                           void **ppHandle, void **ppMspiHandle);

//*****************************************************************************
//
//! @brief De-Initialization the mspi_flash driver.
//!
//! @param pHandle - Pointer to driver handle
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
//! @return status
//
//*****************************************************************************
extern uint32_t am_devices_mspi_flash_deinit(void *pHandle);

//*****************************************************************************
//
//! @brief Reads the ID of the external flash and returns the value.
//!
//! @param pHandle - Pointer driver handle
//!
//! This function reads the device ID register of the external flash, and returns
//! the result as an 32-bit unsigned integer value.
//!
//! @return 32-bit status
//
//*****************************************************************************
extern uint32_t am_devices_mspi_flash_id(void *pHandle);

//*****************************************************************************
//
//! @brief This will reset the external flash device
//!
//! @param pHandle - Pointer to driver handle
//!
//! This will reset the flash device
//!
//! @return 32-bit status
//
//*****************************************************************************
extern uint32_t am_devices_mspi_flash_reset(void *pHandle);

//*****************************************************************************
//
//! @brief Reads the current status of the external flash
//!
//! This function reads the status register of the external flash, and returns
//! the result as an 8-bit unsigned integer value. The processor will block
//! during the data transfer process, but will return as soon as the status
//! register had been read.
//!
//! @param pHandle  - Pointer to driver handle
//! @param pStatus  - Status is returned here
//!
//! Macro definitions for interpreting the contents of the status register are
//! included in the header file.
//!
//! @return return status
//
//*****************************************************************************
extern uint32_t am_devices_mspi_flash_status(void *pHandle, uint32_t *pStatus);

//*****************************************************************************
//
//! @brief Reads the contents of the external flash into a buffer.
//!
//! @param pHandle      - Pointer to driver handle
//! @param pui8RxBuffer - Read data is saved here
//! @param ui32ReadAddress - Read address in device
//! @param ui32NumBytes  - Number of bytes to read
//! @param bWaitForCompletion  - when true this is a blocking call
//!
//! This function reads the external flash at the provided address and stores
//! the received data into the provided buffer location. This function will
//! only store ui32NumBytes worth of data.
//!
//! @return 32-bit status
//
//*****************************************************************************
extern uint32_t am_devices_mspi_flash_read(void *pHandle, uint8_t *pui8RxBuffer,
                                           uint32_t ui32ReadAddress,
                                           uint32_t ui32NumBytes,
                                           bool bWaitForCompletion);

//*****************************************************************************
//
//! @brief Programs the given range of flash addresses.
//!
//! @param pHandle          - Pointer to driver handle
//! @param pui8TxBuffer     - Buffer to write the external flash data from
//! @param ui32WriteAddress - Address to write to in the external flas
//! @param ui32NumBytes     - Number of bytes to write to the external flash
//! @param bWaitForCompletion
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
extern uint32_t am_devices_mspi_flash_write(void *pHandle, uint8_t *pui8TxBuffer,
                                            uint32_t ui32WriteAddress,
                                            uint32_t ui32NumBytes,
                                            bool bWaitForCompletion);

//*****************************************************************************
//
//! @brief Erases the entire contents of the external flash
//!
//! @param pHandle - Pointer to driver handle
//!
//! This function uses the "Bulk Erase" instruction to erase the entire
//! contents of the external flash.
//!
//! @return 32-bit status
//
//*****************************************************************************
extern uint32_t am_devices_mspi_flash_mass_erase(void *pHandle);

//*****************************************************************************
//
//! @brief Erases the contents of a single sector of flash
//!
//! @param pHandle           - Pointer to driver handle
//! @param ui32SectorAddress - Address to erase in the external flash
//!
//! This function erases a single sector of the external flash as specified by
//! ui32EraseAddress. The entire sector where ui32EraseAddress will
//! be erased.
//!
//! @return 32-bit status
//
//*****************************************************************************
extern uint32_t am_devices_mspi_flash_sector_erase(void *pHandle, uint32_t ui32SectorAddress);

//*****************************************************************************
//
//! @brief Sets up the MSPI and external FLASH into XIP mode.
//!
//! @param pHandle  - Pointer to driver handle
//!
//! This function sets the external psram device and the MSPI into XIP mode.
//!
//! @return 32-bit status
//
//*****************************************************************************
extern uint32_t am_devices_mspi_flash_enable_xip(void *pHandle);

//*****************************************************************************
//
//! @brief Removes the MSPI and external FLASH from XIP mode.
//!
//! @param pHandle - Pointer to driver handle
//!
//! This function removes the external device and the MSPI from XIP mode.
//!
//! @return 32-bit status
//
//*****************************************************************************
extern uint32_t am_devices_mspi_flash_disable_xip(void *pHandle);

//*****************************************************************************
//
//! @brief Sets up the MSPI and external FLASH into scrambling mode.
//!
//! @param pHandle  - Pointer to driver handle
//!
//! This function sets the external psram device and the MSPI into scrambling mode.
//!
//! @return 32-bit status
//
//*****************************************************************************
extern uint32_t am_devices_mspi_flash_enable_scrambling(void *pHandle);

//*****************************************************************************
//
//! @brief Removes the MSPI and external FLASH from scrambling mode.
//!
//! @param pHandle  - Pointer to driver handle
//!
//! This function removes the external device and the MSPI from scrambling mode.
//!
//! @return 32-bit status
//
//*****************************************************************************
extern uint32_t am_devices_mspi_flash_disable_scrambling(void *pHandle);

//*****************************************************************************
//
//! @brief
//! @param pHandle
//! @param pui8RxBuffer
//! @param ui32ReadAddress
//! @param ui32NumBytes
//! @param ui32PauseCondition
//! @param ui32StatusSetClr
//! @param pfnCallback
//! @param pCallbackCtxt
//! @return
//
//*****************************************************************************
extern uint32_t am_devices_mspi_flash_read_adv(void *pHandle,
                               uint8_t *pui8RxBuffer,
                               uint32_t ui32ReadAddress,
                               uint32_t ui32NumBytes,
                               uint32_t ui32PauseCondition,
                               uint32_t ui32StatusSetClr,
                               am_hal_mspi_callback_t pfnCallback,
                               void *pCallbackCtxt);

// ****************************************************************************//
//
//! @brief
//!
//! @param pHandle
//! @param pMSPISettings
//!
//! @return
//
// ****************************************************************************
extern am_devices_mspi_flash_config_t am_devices_mspi_flash_mode_switch(
                                 void *pHandle,
                                 am_devices_mspi_flash_config_t *pMSPISettings);

// ****************************************************************************//
//
//! @brief
//!
//! @param pHandle
//! @param ui32Mode
//
// ****************************************************************************//
extern void am_devices_mspi_mixmode_switch(void *pHandle, uint32_t ui32Mode);

//*****************************************************************************
//
//! @brief Reads the contents of the external flash into a buffer.
//!
//! @param pHandle          - Pointer to device driver
//! @param pui8RxBuffer     - Read data is saved here
//! @param ui32ReadAddress  - Read address in device
//! @param ui32NumBytes     - Number of bytes to read
//! @param bWaitForCompletion  - When true this call blocks
//!
//! This function reads the external flash at the provided address and stores
//! the received data into the provided buffer location. This function will
//! only store ui32NumBytes worth of data.
//!
//! @return 32-bit status
//
//*****************************************************************************
extern uint32_t
am_devices_mspi_flash_read_hiprio(void *pHandle, uint8_t *pui8RxBuffer,
                           uint32_t ui32ReadAddress,
                           uint32_t ui32NumBytes,
                           bool bWaitForCompletion);

// ##### INTERNAL BEGIN #####
//*****************************************************************************
//
//! @brief
//! @param pHandle
//! @param pui8RxBuffer
//! @param ui32ReadAddress
//! @param ui32NumBytes
//! @return
//
//*****************************************************************************
extern uint32_t
am_devices_mspi_flash_read_callback(void *pHandle, uint8_t *pui8RxBuffer,
                           uint32_t ui32ReadAddress,
                           uint32_t ui32NumBytes);
// ##### INTERNAL END #####

#ifdef __cplusplus
}
#endif

#endif // AM_DEVICES_MSPI_FLASH_H

//*****************************************************************************
//
// End Doxygen group.
//! @}
//
//*****************************************************************************

