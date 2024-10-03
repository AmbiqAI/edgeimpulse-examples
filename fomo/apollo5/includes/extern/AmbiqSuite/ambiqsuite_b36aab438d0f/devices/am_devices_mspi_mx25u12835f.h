//*****************************************************************************
//
//! @file am_devices_mspi_mx25u12835f.h
//!
//! @brief General Multibit SPI Mx25u12835f driver.
//!
//! @addtogroup mspi_mx25u12835f MX25U12835F MSPI Display Driver
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

#ifndef AM_DEVICES_MSPI_MX25U12835F_H
#define AM_DEVICES_MSPI_MX25U12835F_H

#ifdef __cplusplus
extern "C"
{
#endif

//*****************************************************************************
//
//! @name Global definitions for mx25u12835f commands
//! @{
//
//*****************************************************************************
#define AM_DEVICES_MSPI_MX25U12835F_WRITE_STATUS      0x01
#define AM_DEVICES_MSPI_MX25U12835F_PAGE_PROGRAM      0x02
#define AM_DEVICES_MSPI_MX25U12835F_READ              0x03
#define AM_DEVICES_MSPI_MX25U12835F_WRITE_DISABLE     0x04
#define AM_DEVICES_MSPI_MX25U12835F_READ_STATUS       0x05
#define AM_DEVICES_MSPI_MX25U12835F_WRITE_ENABLE      0x06
#define AM_DEVICES_MSPI_MX25U12835F_FAST_READ         0x0B
#define AM_DEVICES_MSPI_MX25U12835F_READ_4B           0x13
#define AM_DEVICES_MSPI_MX25U12835F_SUBSECTOR_ERASE   0x20
#define AM_DEVICES_MSPI_MX25U12835F_DUAL_READ         0x3B
#define AM_DEVICES_MSPI_MX25U12835F_DUAL_IO_READ      0xBB
#define AM_DEVICES_MSPI_MX25U12835F_WRITE_ENHVOL_CFG  0x61
#define AM_DEVICES_MSPI_MX25U12835F_RESET_ENABLE      0x66
#define AM_DEVICES_MSPI_MX25U12835F_QUAD_READ         0x6B
#define AM_DEVICES_MSPI_MX25U12835F_WRITE_VOL_CFG     0x81
#define AM_DEVICES_MSPI_MX25U12835F_RESET_MEMORY      0x99
#define AM_DEVICES_MSPI_MX25U12835F_READ_ID           0x9F
#define AM_DEVICES_MSPI_MX25U12835F_ENTER_4B          0xB7
#define AM_DEVICES_MSPI_MX25U12835F_BULK_ERASE        0xC7
#define AM_DEVICES_MSPI_MX25U12835F_SECTOR_ERASE      0xD8
#define AM_DEVICES_MSPI_MX25U12835F_EXIT_4B           0xE9
#define AM_DEVICES_MSPI_MX25U12835F_QUAD_IO_READ      0xEB
#define AM_DEVICES_MSPI_MX25U12835F_READ_QUAD_4B      0xEC

//! @}

//*****************************************************************************
//
//! @name Global definitions for the mx25u12835f status register
//! @{
//
//*****************************************************************************
#define AM_DEVICES_MSPI_MX25U12835F_WEL         0x00000002        // Write enable latch
#define AM_DEVICES_MSPI_MX25U12835F_WIP         0x00000001        // Write in progress
//! @}

//*****************************************************************************
//
//! @name Device specific identification.
//! @{
//
//*****************************************************************************
#define AM_DEVICES_MSPI_MX25U12835F_ID        0x003825C2
#define AM_DEVICES_MSPI_MX25U12835F_ID_MASK   0x00FFFFFF
//! @}

//*****************************************************************************
//
//! @name Device specific definitions for mx25u12835f commands
//! @{
//
//*****************************************************************************
#define AM_DEVICES_MSPI_MX25U12835F_READ_CONFIG               0x15
#define AM_DEVICES_MSPI_MX25U12835F_ENABLE_QPI_MODE           0x35
#define AM_DEVICES_MSPI_MX25U12835F_DISABLE_QPI_MODE          0xF5

//! @}
//*****************************************************************************
//
//! @name Device specific definitions for the Configuration register(s)
//! @{
//
//*****************************************************************************
#define AM_DEVICES_MSPI_MX25U12835F_STATUS              (0x00)
#define AM_DEVICES_MSPI_MX25U12835F_CONFIG              (0x07)
//! @}

//*****************************************************************************
//
//! @name Device specific definitions for the mx25u12835f size information
//! @{
//
//*****************************************************************************
#define AM_DEVICES_MSPI_MX25U12835F_PAGE_SIZE       0x100    //256 bytes, minimum program unit
#define AM_DEVICES_MSPI_MX25U12835F_SECTOR_SIZE     0x1000   //4K bytes
#define AM_DEVICES_MSPI_MX25U12835F_BLOCK_SIZE      0x10000  //64K bytes.
#define AM_DEVICES_MSPI_MX25U12835F_MAX_SECTORS     4096     // Sectors within 3-byte address range.
#define AM_DEVICES_MSPI_MX25U12835F_MAX_BLOCKS      256
//! @}

//*****************************************************************************
//
//! @name Global definitions for the MSPI instance to use.
//! @{
//
//*****************************************************************************
#define AM_DEVICES_MSPI_MX25U12835F_MSPI_INSTANCE     0

#define AM_DEVICES_MSPI_MX25U12835F_MAX_DEVICE_NUM    1
//! @}

//*****************************************************************************
//
// Global type definitions.
//
//*****************************************************************************
typedef enum
{
    AM_DEVICES_MSPI_MX25U12835F_STATUS_SUCCESS,
    AM_DEVICES_MSPI_MX25U12835F_STATUS_ERROR
} am_devices_mspi_mx25u12835f_status_t;

typedef struct
{
    am_hal_mspi_device_e eDeviceConfig;
    am_hal_mspi_clock_e eClockFreq;
    uint32_t *pNBTxnBuf;
    uint32_t ui32NBTxnBufLength;
    uint32_t ui32ScramblingStartAddr;
    uint32_t ui32ScramblingEndAddr;
} am_devices_mspi_mx25u12835f_config_t;

//*****************************************************************************
//
// External function definitions.
//
//*****************************************************************************
//*****************************************************************************
//
//! @brief Initialize the mspi_mx25u12835f driver.
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
//! @return status
//
//*****************************************************************************
extern uint32_t am_devices_mspi_mx25u12835f_init(uint32_t ui32Module,
                                           am_devices_mspi_mx25u12835f_config_t *psMSPISettings,
                                           void **ppHandle, void **ppMspiHandle);

//*****************************************************************************
//
//! @brief De-Initialization the mspi_mx25u12835f driver.
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
extern uint32_t am_devices_mspi_mx25u12835f_deinit(void *pHandle);

//*****************************************************************************
//
//! @brief Reads the ID of the external mx25u12835f and returns the value.
//!
//! @param pHandle - Pointer to driver handle.
//!
//! This function reads the device ID register of the external mx25u12835f, and returns
//! the result as an 32-bit unsigned integer value.
//!
//! @return 32-bit status
//
//*****************************************************************************
extern uint32_t am_devices_mspi_mx25u12835f_id(void *pHandle);

//*****************************************************************************
//
//! @brief Reads the current status of the external mx25u12835f
//!
//! @param pHandle - Pointer to driver handle.
//!
//! This function reads the device ID register of the external mx25u12835f, and returns
//! the result as an 32-bit unsigned integer value.
//!
//! @return 32-bit status
//
//*****************************************************************************
extern uint32_t am_devices_mspi_mx25u12835f_reset(void *pHandle);

//*****************************************************************************
//
//! @brief Reads the current status of the external mx25u12835f
//!
//! @param pHandle - Pointer to driver handle.
//! @param pStatus - device status returned here
//! This function reads the status register of the external mx25u12835f, and returns
//! the result as an 8-bit unsigned integer value. The processor will block
//! during the data transfer process, but will return as soon as the status
//! register had been read.
//!
//! Macro definitions for interpreting the contents of the status register are
//! included in the header file.
//!
//! @return call status
//
//*****************************************************************************
extern uint32_t am_devices_mspi_mx25u12835f_status(void *pHandle, uint32_t *pStatus);

//*****************************************************************************
//
//! @brief Reads the contents of the external mx25u12835f into a buffer.
//!
//! @param pHandle         - Pointer to driver handle
//! @param pui8RxBuffer    - Data is saved here
//! @param ui32ReadAddress - Address of desired data in external mx25u12835f
//! @param ui32NumBytes    - Number of bytes to read from external mx25u12835f
//! @param bWaitForCompletion
//!
//! This function reads the external mx25u12835f at the provided address and stores
//! the received data into the provided buffer location. This function will
//! only store ui32NumBytes worth of data.
//!
//! @return 32-bit status
//
//*****************************************************************************
extern uint32_t am_devices_mspi_mx25u12835f_read(void *pHandle, uint8_t *pui8RxBuffer,
                                           uint32_t ui32ReadAddress,
                                           uint32_t ui32NumBytes,
                                           bool bWaitForCompletion);

//*****************************************************************************
//
//! @brief Programs the given range of mx25u12835f addresses.
//!
//! @param pHandle           - Pointer to driver handle
//! @param pui8TxBuffer      - Buffer to write the external mx25u12835f data from
//! @param ui32WriteAddress  - Address to write to in the external mx25u12835f
//! @param ui32NumBytes      - Number of bytes to write to the external mx25u12835f
//! @param bWaitForCompletion
//!
//! This function uses the data in the provided pui8TxBuffer and copies it to
//! the external mx25u12835f at the address given by ui32WriteAddress. It will copy
//! exactly ui32NumBytes of data from the original pui8TxBuffer pointer. The
//! user is responsible for ensuring that they do not overflow the target mx25u12835f
//! memory or underflow the pui8TxBuffer array
//!
//! @return 32-bit status
//
//*****************************************************************************
extern uint32_t am_devices_mspi_mx25u12835f_write(void *pHandle,
                                            uint8_t *pui8TxBuffer,
                                            uint32_t ui32WriteAddress,
                                            uint32_t ui32NumBytes,
                                            bool bWaitForCompletion);

//*****************************************************************************
//
//! @brief Erases the entire contents of the external mx25u12835f
//!
//! @param pHandle - Pointer to driver handle
//!
//! This function uses the "Bulk Erase" instruction to erase the entire
//! contents of the external mx25u12835f.
//!
//! @return 32-bit status
//
//*****************************************************************************
extern uint32_t am_devices_mspi_mx25u12835f_mass_erase(void *pHandle);

//*****************************************************************************
//
//! @brief Erases the contents of a single sector of mx25u12835f
//!
//! @param pHandle           - Pointer to driver handle
//! @param ui32SectorAddress - Address to erase in the external mx25u12835f
//!
//! This function erases a single sector of the external mx25u12835f as specified by
//! ui32EraseAddress. The entire sector where ui32EraseAddress will
//! be erased.
//!
//! @return 32-bit status
//
//*****************************************************************************
extern uint32_t am_devices_mspi_mx25u12835f_sector_erase(void *pHandle, uint32_t ui32SectorAddress);

//*****************************************************************************
//
//! @brief Sets up the MSPI and external MX25U12835F into XIP mode.
//!
//! @param pHandle - Pointer to driver handle
//!
//! This function sets the external psram device and the MSPI into XIP mode.
//!
//! @return 32-bit status
//
//*****************************************************************************
extern uint32_t am_devices_mspi_mx25u12835f_enable_xip(void *pHandle);

//*****************************************************************************
//
//! @brief Removes the MSPI and external MX25U12835F from XIP mode.
//!
//! @param pHandle - Pointer to driver handle
//!
//! This function removes the external device and the MSPI from XIP mode.
//!
//! @return 32-bit status
//
//*****************************************************************************
extern uint32_t am_devices_mspi_mx25u12835f_disable_xip(void *pHandle);

//*****************************************************************************
//
//! @brief Sets up the MSPI and external MX25U12835F into scrambling mode.
//!
//! @param pHandle - Pointer to driver handle
//!
//! This function sets the external psram device and the MSPI into scrambling mode.
//!
//! @return 32-bit status
//
//*****************************************************************************
extern uint32_t am_devices_mspi_mx25u12835f_enable_scrambling(void *pHandle);

//*****************************************************************************
//
//! @brief Removes the MSPI and external MX25U12835F from scrambling mode.
//!
//! @param pHandle - Pointer to driver handle
//!
//! This function removes the external device and the MSPI from scrambling mode.
//!
//! @return 32-bit status
//
//*****************************************************************************
extern uint32_t am_devices_mspi_mx25u12835f_disable_scrambling(void *pHandle);

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
extern uint32_t
am_devices_mspi_mx25u12835f_read_adv(void *pHandle, uint8_t *pui8RxBuffer,
                               uint32_t ui32ReadAddress,
                               uint32_t ui32NumBytes,
                               uint32_t ui32PauseCondition,
                               uint32_t ui32StatusSetClr,
                               am_hal_mspi_callback_t pfnCallback,
                               void *pCallbackCtxt);

//*****************************************************************************
//
//! @brief Reads the contents of the external mx25u12835f into a buffer.
//!
//! @param pHandle            - Pointer to driver handle
//! @param pui8RxBuffer       - Buffer to store the received data from the mx25u12835f
//! @param ui32ReadAddress    - Address of desired data in external mx25u12835f
//! @param ui32NumBytes       - Number of bytes to read from external mx25u12835f
//! @param bWaitForCompletion
//!
//! This function reads the external mx25u12835f at the provided address and stores
//! the received data into the provided buffer location. This function will
//! only store ui32NumBytes worth of data.
//!
//! @return 32-bit status
//
//*****************************************************************************
extern uint32_t
am_devices_mspi_mx25u12835f_read_hiprio(void *pHandle, uint8_t *pui8RxBuffer,
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
//! @return staus
//
//*****************************************************************************
extern uint32_t
am_devices_mspi_mx25u12835f_read_callback(void *pHandle, uint8_t *pui8RxBuffer,
                           uint32_t ui32ReadAddress,
                           uint32_t ui32NumBytes);
// ##### INTERNAL END #####

#ifdef __cplusplus
}
#endif

#endif // AM_DEVICES_MSPI_MX25U12835F_H

//*****************************************************************************
//
// End Doxygen group.
//! @}
//
//*****************************************************************************

