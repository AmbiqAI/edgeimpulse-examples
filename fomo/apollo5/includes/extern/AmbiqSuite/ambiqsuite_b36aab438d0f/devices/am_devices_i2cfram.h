//*****************************************************************************
//
//! @file am_devices_i2cfram.h
//!
//! @brief Generic I2C FRAM driver.
//!
//! @addtogroup i2cfram I2C FRAM Driver
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

#ifndef AM_DEVICES_I2CFRAM_H
#define AM_DEVICES_I2CFRAM_H

#ifdef __cplusplus
extern "C"
{
#endif

//*****************************************************************************
//
//! Device structure used for communication.
//
//*****************************************************************************
typedef struct
{
    //*************************************************************************
    //! @name Parameters supplied by application.
    //! @{
    //*************************************************************************

    //
    //! Module number to use for IOM access.
    //
    uint32_t ui32IOMModule;

    //
    //! Bus Address to use for IOM I2C access.
    //
    uint32_t ui32BusAddress;
    //! @}

} am_devices_i2cfram_t;

//*****************************************************************************
//
// External function definitions.
//
//*****************************************************************************
//*****************************************************************************
//
//! @brief Initialize the i2cfram driver.
//!
//! @param psIOMSettings - IOM device structure describing the target spifram.
//!
//! @note This function should be called before any other am_devices_i2cfram
//! functions. It is used to set tell the other functions how to communicate
//! with the external i2cfram hardware.
//
//*****************************************************************************
extern void am_devices_i2cfram_init(am_devices_i2cfram_t *psIOMSettings);

//*****************************************************************************
//
//! @brief read and return the ID register, manfID, productID from external FRAM
//!
//! @param manufacturerID  read from device returned here
//! @param productID       read from device returned here
//!
//! @return   ID register
//
//*****************************************************************************
extern uint32_t am_devices_i2cfram_id(
    uint16_t *manufacturerID,
    uint16_t *productID);

//*****************************************************************************
//
//! @brief Reads the contents of the external fram into a buffer.
//!
//! @param pui8RxBuffer     - Buffer to store the received data from the fram
//! @param ui32ReadAddress  - Address of desired data in external fram
//! @param ui32NumBytes     - Number of bytes to read from external fram
//!
//! This function reads the external fram at the provided address and stores
//! the received data into the provided buffer location. This function will
//! only store ui32NumBytes worth of data.
//
//*****************************************************************************
extern void am_devices_i2cfram_read(uint8_t *pui8RxBuffer,
                                     uint32_t ui32ReadAddress,
                                     uint32_t ui32NumBytes);

//*****************************************************************************
//
//! @brief Programs the given range of fram addresses.
//!
//! @param pui8TxBuffer - Buffer to write the external fram data from
//! @param ui32WriteAddress - Address to write to in the external fram
//! @param ui32NumBytes - Number of bytes to write to the external fram
//!
//! This function uses the data in the provided pui8TxBuffer and copies it to
//! the external fram at the address given by ui32WriteAddress. It will copy
//! exactly ui32NumBytes of data from the original pui8TxBuffer pointer. The
//! user is responsible for ensuring that they do not overflow the target fram
//! memory or underflow the pui8TxBuffer array
//
//*****************************************************************************
extern void am_devices_i2cfram_write(uint8_t *pui8TxBuffer,
                                      uint32_t ui32WriteAddress,
                                      uint32_t ui32NumBytes);

//##### INTERNAL BEGIN #####
//*****************************************************************************
//
//! @brief Reads the contents of the external fram into a buffer.
//!
//! @param pui8RxBuffer    - Buffer to store the received data from the fram
//! @param ui32ReadAddress - Address of desired data in external fram
//! @param ui32NumBytes    - Number of bytes to read from external fram
//! @param mode            - IOM Mode
//! @param pfnCallback     - Function called when all data has been read
//!
//! This function reads the external fram at the provided address and stores
//! the received data into the provided buffer location. This function will
//! only store ui32NumBytes worth of data.
//
//*****************************************************************************
extern void am_devices_i2cfram_read_mode(uint8_t *pui8RxBuffer,
                uint32_t ui32ReadAddress,
                uint32_t ui32NumBytes,
                AM_HAL_IOM_MODE_E mode,
                am_hal_iom_callback_t pfnCallback);

//*****************************************************************************
//
//! @brief Programs the given range of fram addresses.
//!
//! @param pui8TxBuffer - Buffer to write the external fram data from
//! @param ui32WriteAddress - Address to write to in the external fram
//! @param ui32NumBytes - Number of bytes to write to the external fram
//! @param mode            - IOM Mode
//! @param pfnCallback     - Function called when all data has been copied
//!
//! This function uses the data in the provided pui8TxBuffer and copies it to
//! the external FRAM at the address given by ui32WriteAddress. It will copy
//! exactly ui32NumBytes of data from the original pui8TxBuffer pointer. The
//! user is responsible for ensuring that they do not overflow the target FRAM
//! memory or underflow the pui8TxBuffer array
//
//*****************************************************************************
extern void am_devices_i2cfram_write_mode(uint8_t *pui8TxBuffer,
                uint32_t ui32WriteAddress,
                uint32_t ui32NumBytes,
                AM_HAL_IOM_MODE_E mode,
                am_hal_iom_callback_t pfnCallback);

//##### INTERNAL END #####

#ifdef __cplusplus
}
#endif

#endif // AM_DEVICES_I2CFRAM_H

//*****************************************************************************
//
// End Doxygen group.
//! @}
//
//*****************************************************************************

