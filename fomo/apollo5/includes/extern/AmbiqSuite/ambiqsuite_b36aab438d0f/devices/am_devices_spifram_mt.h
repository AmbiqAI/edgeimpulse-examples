//*****************************************************************************
//
//! @file am_devices_spifram_mt.h
//!
//! @brief Generic spifram driver.
//!
//! @addtogroup spifram_mt SPI MT FRAM Driver
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

#ifndef AM_DEVICES_SPIFRAM_MT_H
#define AM_DEVICES_SPIFRAM_MT_H

//*****************************************************************************
//
//! @name Global definitions for fram commands
//! @{
//
//*****************************************************************************

#define AM_DEVICES_SPIFRAM_WRSR        0x01      /* Write Status Register */
#define AM_DEVICES_SPIFRAM_WRITE       0x02      /* Write Memory */
#define AM_DEVICES_SPIFRAM_READ        0x03      /* Read Memory */
#define AM_DEVICES_SPIFRAM_WRDI        0x04      /* Reset Write Enable Latch */
#define AM_DEVICES_SPIFRAM_RDRSR       0x05      /* Read Status Register */
#define AM_DEVICES_SPIFRAM_WREN        0x06      /* Write Enable Latch */
#define AM_DEVICES_SPIFRAM_RDID        0x9F      /* Read Device ID */
//! @}

//*****************************************************************************
//
//! @name  Global definitions for the fram status register
//! @{
//
//*****************************************************************************
#define AM_DEVICES_SPIFRAM_WEL         0x02        // Write enable latch
#define AM_DEVICES_SPIFRAM_BP0         0x04        // Block Protect 0
#define AM_DEVICES_SPIFRAM_BP1         0x08        // Block Protect 0
#define AM_DEVICES_SPIFRAM_WPEN        0x80        // Status Register Write Protect
//! @}

//*****************************************************************************
//
//! @name  FRAM status register mask bits.
//! @{
//
//*****************************************************************************
#define AM_DEVICES_SPIFRAM_STAT_WPEN    0x80
#define AM_DEVICES_SPIFRAM_STAT_RSVD    0x70
#define AM_DEVICES_SPIFRAM_STAT_BP1     0x08
#define AM_DEVICES_SPIFRAM_STAT_BP0     0x04
#define AM_DEVICES_SPIFRAM_STAT_WEL     0x02
#define AM_DEVICES_SPIFRAM_STAT_ZERO    0x01
//! @}

#ifdef __cplusplus
extern "C"
{
#endif

//*****************************************************************************
//
// External function definitions.
//
//*****************************************************************************
//*****************************************************************************
//
//! @brief Reads the ID register for the external fram
//!
//! @param ui32IOMModule
//! @param ui32ChipSelect
//! @param manufacturerID
//! @param productID
//!
//! This function reads the ID register of the external fram, and returns the
//! result as a 32-bit unsigned integer value. The processor will block during
//! the data transfer process, but will return as soon as the ID register had
//! been read. The ID contents for this fram only contains 24 bits of data, so
//! the result will be stored in the lower 24 bits of the return value.
//!
//! @return 32-bit ID register contents
//
//*****************************************************************************
extern uint32_t am_devices_spifram_mt_id(uint32_t ui32IOMModule,
                                         uint32_t ui32ChipSelect,
                                         uint8_t *manufacturerID,
                                         uint16_t *productID);

//*****************************************************************************
//
//! @brief Reads the contents of the external fram into a buffer.
//!
//! @param ui32IOMModule
//! @param ui32ChipSelect
//! @param pui8RxBuffer - Buffer to store the received data from the fram
//! @param ui32ReadAddress - Address of desired data in external fram
//! @param ui32NumBytes - Number of bytes to read from external fram
//! @param mode
//! @param pfnCallback
//!
//! This function reads the external fram at the provided address and stores
//! the received data into the provided buffer location. This function will
//! only store ui32NumBytes worth of data.
//
//*****************************************************************************
extern void am_devices_spifram_mt_read_mode(uint32_t ui32IOMModule,
                                            uint32_t ui32ChipSelect,
                                            uint8_t *pui8RxBuffer,
                                            uint32_t ui32ReadAddress,
                                            uint32_t ui32NumBytes,
                                            AM_HAL_IOM_MODE_E mode,
                                            am_hal_iom_callback_t pfnCallback);

//*****************************************************************************
//
//! @brief Programs the given range of fram addresses.
//!
//! @param ui32IOMModule
//! @param ui32ChipSelect
//! @param pui8TxBuffer - Buffer to write the external fram data from
//! @param ui32WriteAddress - Address to write to in the external fram
//! @param ui32NumBytes - Number of bytes to write to the external fram
//! @param mode
//! @param pfnCallback
//!
//! This function uses the data in the provided pui8TxBuffer and copies it to
//! the external fram at the address given by ui32WriteAddress. It will copy
//! exactly ui32NumBytes of data from the original pui8TxBuffer pointer. The
//! user is responsible for ensuring that they do not overflow the target fram
//! memory or underflow the pui8TxBuffer array
//
//*****************************************************************************
extern void am_devices_spifram_mt_write_mode(uint32_t ui32IOMModule,
                                             uint32_t ui32ChipSelect,
                                             uint8_t *pui8TxBuffer,
                                             uint32_t ui32WriteAddress,
                                             uint32_t ui32NumBytes,
                                             AM_HAL_IOM_MODE_E mode,
                                             am_hal_iom_callback_t pfnCallback);
#ifdef __cplusplus
}
#endif

#endif // AM_DEVICES_SPIFRAM_MT_H

//*****************************************************************************
//
// End Doxygen group.
//! @}
//
//*****************************************************************************

