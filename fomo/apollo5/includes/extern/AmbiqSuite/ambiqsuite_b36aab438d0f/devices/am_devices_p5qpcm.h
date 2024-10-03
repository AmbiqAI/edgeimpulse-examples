//*****************************************************************************
//
//! @file am_devices_p5qpcm.h
//!
//! @brief Driver for the Micron P5Q PCM SPI Flash.
//!
//! @addtogroup p5qpcm P5Q PCM SPI Flash Driver
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

#ifndef AM_DEVICES_P5QPCM_H
#define AM_DEVICES_P5QPCM_H

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
#define AM_DEVICES_P5QPCM_WREN            0x06        // Write enable
#define AM_DEVICES_P5QPCM_WRDI            0x04        // Write disable
#define AM_DEVICES_P5QPCM_RDID            0x9E        // Read Identification
#define AM_DEVICES_P5QPCM_RDRSR           0x05        // Read status register
#define AM_DEVICES_P5QPCM_WRSR            0x01        // Write status register
#define AM_DEVICES_P5QPCM_READ            0x03        // Read data bytes
#define AM_DEVICES_P5QPCM_PP              0x22        // Page program (bit-alterable)
#define AM_DEVICES_P5QPCM_SE              0xD8        // Sector Erase
#define AM_DEVICES_P5QPCM_BE              0xC7        // Bulk Erase
//! @}

//*****************************************************************************
//
//! @name Global definitions for the flash status register
//! @{
//
//*****************************************************************************
#define AM_DEVICES_P5QPCM_SRWD           0x80        // Status register write protect
#define AM_DEVICES_P5QPCM_BP3            0x40        // Block protect 3
#define AM_DEVICES_P5QPCM_TB             0x20        // Top/Bottom bit
#define AM_DEVICES_P5QPCM_BP2            0x10        // Block protect 2
#define AM_DEVICES_P5QPCM_BP1            0x08        // Block protect 1
#define AM_DEVICES_P5QPCM_BP0            0x04        // Block protect 0
#define AM_DEVICES_P5QPCM_WEL            0x02        // Write enable latch
#define AM_DEVICES_P5QPCM_WIP            0x01        // Write in progress

//! @}
//*****************************************************************************
//
//! @name Function pointers for SPI write and read.
//! @{
//
//*****************************************************************************
typedef bool (*am_devices_p5qpcm_write_t)(uint32_t ui32Module,
                                          uint32_t ui32ChipSelect,
                                          uint32_t *pui32Data,
                                          uint32_t ui32NumBytes,
                                          uint32_t ui32Options);

typedef bool (*am_devices_p5qpcm_read_t)(uint32_t ui32Module,
                                         uint32_t ui32ChipSelect,
                                         uint32_t *pui32Data,
                                         uint32_t ui32NumBytes,
                                         uint32_t ui32Options);

//! @}
//*****************************************************************************
//
//! @name Device structure used for communication.
//! @{
//
//*****************************************************************************
typedef struct
{
    //
    // Module number to use for IOM access.
    //
    uint32_t ui32IOMModule;

    //
    // Chip Select number to use for IOM access.
    //
    uint32_t ui32ChipSelect;
}
am_devices_p5qpcm_t;
//! @}

//*****************************************************************************
//
// External function definitions.
//
//*****************************************************************************
//*****************************************************************************
//
//! @brief
//!
//! @param psIOMSettings
//! @param pfnWriteFunc
//! @param pfnReadFunc
//
//*****************************************************************************
extern void am_devices_p5qpcm_init(am_hal_iom_spi_device_t *psIOMSettings,
                                   am_devices_p5qpcm_write_t pfnWriteFunc,
                                   am_devices_p5qpcm_read_t pfnReadFunc);

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
extern uint8_t am_devices_p5qpcm_status(void);

//*****************************************************************************
//
//! @brief Reads the ID register for the external flash
//!
//! This function reads the ID register of the external flash, and returns the
//! result as a 32-bit unsigned integer value. The processor will block during
//! the data transfer process, but will return as soon as the ID register had
//! been read. The ID contents for this flash only contains 24 bits of data, so
//! the result will be stored in the lower 24 bits of the return value.
//!
//! @return 32-bit ID register contents
//
//*****************************************************************************
extern uint32_t am_devices_p5qpcm_id(void);

//*****************************************************************************
//
//! @brief Reads the contents of the external flash into a buffer.
//!
//! @param pui8RxBuffer - Buffer to store the received data from the flash
//! @param ui32ReadAddress - Address of desired data in external flash
//! @param ui32NumBytes - Number of bytes to read from external flash
//!
//! This function reads the external flash at the provided address and stores
//! the received data into the provided buffer location. This function will
//! only store ui32NumBytes worth of data.
//
//*****************************************************************************
extern void am_devices_p5qpcm_read(uint8_t *pui8RxBuffer,
                                   uint32_t ui32ReadAddress,
                                   uint32_t ui32NumBytes);

//*****************************************************************************
//
//! @brief Programs the given range of flash addresses.
//!
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
//*****************************************************************************
extern void am_devices_p5qpcm_write(uint8_t *pui8TxBuffer,
                                    uint32_t ui32WriteAddress,
                                    uint32_t ui32NumBytes);

//*****************************************************************************
//
//! @brief Erases the entire contents of the external flash
//!
//! This function uses the "Bulk Erase" instruction to erase the entire
//! contents of the external flash.
//
//*****************************************************************************
extern void am_devices_p5qpcm_mass_erase(void);

//*****************************************************************************
//
//! @brief Erases the contents of a single sector of flash
//!
//! @param ui32SectorAddress - Address to erase in the external flash
//!
//! This function erases a single sector of the external flash as specified by
//! ui32EraseAddress. The entire sector where ui32EraseAddress will
//! be erased.
//
//*****************************************************************************
extern void am_devices_p5qpcm_sector_erase(uint32_t ui32SectorAddress);

#ifdef __cplusplus
}
#endif

#endif // AM_DEVICES_P5QPCM_H

//*****************************************************************************
//
// End Doxygen group.
//! @}
//
//*****************************************************************************

