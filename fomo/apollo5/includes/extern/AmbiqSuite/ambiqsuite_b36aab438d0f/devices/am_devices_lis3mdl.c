//*****************************************************************************
//
//! @file am_devices_lis3mdl.c
//!
//! @brief Driver for the ST Microelectronics LIS3MDL magnetometer
//!
//! @addtogroup lis3mdl SPI Device Control for the LIS3MDL External Magnetometer
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

#include "am_mcu_apollo.h"
#include "am_devices_lis3mdl.h"

//*****************************************************************************
//
// Reset the LIS3MDL mag
//
//*****************************************************************************
void
am_devices_lis3mdl_reset(const am_devices_lis3mdl_t *psDevice)
{
    //
    // Reset the LIS3MDL Mag
    //
    am_devices_lis3mdl_reg_write(psDevice, AM_DEVICES_LIS3MDL_CTRL_REG2, 0x0C);
}

//*****************************************************************************
//
//  Get the device ID
//
//*****************************************************************************
uint8_t
am_devices_lis3mdl_device_id_get(const am_devices_lis3mdl_t *psDevice)
{
    //
    // Read the WHO_AM_I register and return the result.
    //
    return am_devices_lis3mdl_reg_read(psDevice, AM_DEVICES_LIS3MDL_WHO_AM_I);
}

//*****************************************************************************
//
//  Configures the LIS3MDL for operation.
//
//*****************************************************************************
void
am_devices_lis3mdl_config(const am_devices_lis3mdl_t *psDevice)
{
    am_hal_lis3mdl_regs(6) sRegBuffer;

    //
    // Reset the device
    //
    am_devices_lis3mdl_reset(psDevice);

    sRegBuffer.bytes[0] = 0x10;     // CTRL_REG1
    sRegBuffer.bytes[1] = 0x00;     // CTRL_REG2
    sRegBuffer.bytes[2] = 0x00;     // CTRL_REG3
    sRegBuffer.bytes[3] = 0x00;     // CTRL_REG4
    sRegBuffer.bytes[4] = 0x00;     // CTRL_REG5

    //
    // Write the control registers to the device as one solid block write.
    //
    am_devices_lis3mdl_reg_block_write(psDevice, AM_DEVICES_LIS3MDL_CTRL_REG1,
                                       sRegBuffer.words, 5, 0);

    //
    // Clear out any old data that might be in the mag.
    //
    am_devices_lis3mdl_sample_get(psDevice, sRegBuffer.words, 0);
}

//*****************************************************************************
//
//  Trigger a single conversion mode
//
//*****************************************************************************
void
am_devices_lis3mdl_trigger(const am_devices_lis3mdl_t *psDevice)
{
    am_devices_lis3mdl_reg_write(psDevice, AM_DEVICES_LIS3MDL_CTRL_REG3, 0x01);
}

//*****************************************************************************
//
//  Retrieves the most recent sample from the LIS3MDL.
//
//*****************************************************************************
void
am_devices_lis3mdl_sample_get(const am_devices_lis3mdl_t *psDevice, uint32_t *psData,
                              am_hal_iom_callback_t pfnCallback)
{
    //
    // Read the magnetometer registers as a single block.
    //
    am_devices_lis3mdl_reg_block_read(psDevice, AM_DEVICES_LIS3MDL_OUT_X_L,
                                      psData, 6, pfnCallback);
}

//*****************************************************************************
//
//  Reads an internal register in the LIS3MDL.
//
//*****************************************************************************
uint8_t
am_devices_lis3mdl_reg_read(const am_devices_lis3mdl_t *psDevice,
                            uint8_t ui8Register)
{
    uint8_t ui8Offset;
    am_hal_iom_buffer(1) sData;

    //
    // Build the SPI offset and the data buffer.
    //
    ui8Offset = 0x80 | ui8Register;

    //
    // Send the read to the bus using the polled API.
    //
    am_hal_iom_spi_read(psDevice->ui32IOMModule, psDevice->ui32ChipSelect,
                        sData.words, 1, AM_HAL_IOM_OFFSET(ui8Offset));

    //
    // Return the retrieved data.
    //
    return sData.bytes[0];
}

//*****************************************************************************
//
//  Reads a block of internal registers in the LIS3MDL.
//
//*****************************************************************************
void
am_devices_lis3mdl_reg_block_read(const am_devices_lis3mdl_t *psDevice,
                                  uint8_t ui8StartRegister,
                                  uint32_t *pui32Values,
                                  uint32_t ui32NumBytes,
                                  am_hal_iom_callback_t pfnCallback)
{
    uint8_t ui8Offset;

    //
    // Build the SPI offset for writing a block of registers from the
    // user-supplied start point.
    //
    ui8Offset = 0xC0 | ui8StartRegister;

    //
    // Check to see if the callback pointer is valid.
    //
    if ( pfnCallback )
    {
        //
        // If so, use a non-blocking call with a callback.
        //
        am_hal_iom_queue_spi_read(psDevice->ui32IOMModule,
                               psDevice->ui32ChipSelect,
                               pui32Values, ui32NumBytes,
                               AM_HAL_IOM_OFFSET(ui8Offset),
                               pfnCallback);
    }
    else
    {
        //
        // Otherwise, use a polled call.
        //
        am_hal_iom_spi_read(psDevice->ui32IOMModule,
                            psDevice->ui32ChipSelect,
                            pui32Values, ui32NumBytes,
                            AM_HAL_IOM_OFFSET(ui8Offset));
    }
}

//*****************************************************************************
//
//  Writes an internal register in the LIS3MDL.
//
//*****************************************************************************
void
am_devices_lis3mdl_reg_write(const am_devices_lis3mdl_t *psDevice,
                             uint8_t ui8Register, uint8_t ui8Value)
{
    uint8_t ui8Offset;
    am_hal_iom_buffer(1) sData;

    //
    // Build the SPI offset and the data buffer.
    //
    ui8Offset = ui8Register;
    sData.bytes[0] = ui8Value;

    //
    // Send the write to the bus using the polled API.
    //
    am_hal_iom_spi_write(psDevice->ui32IOMModule, psDevice->ui32ChipSelect,
                         sData.words, 1, AM_HAL_IOM_OFFSET(ui8Offset));
}

//*****************************************************************************
//
//  Writes a block of internal registers in the LIS3MDL.
//
//*****************************************************************************
void
am_devices_lis3mdl_reg_block_write(const am_devices_lis3mdl_t *psDevice,
                                   uint8_t ui8StartRegister,
                                   uint32_t *pui32Values,
                                   uint32_t ui32NumBytes,
                                   am_hal_iom_callback_t pfnCallback)
{
    uint8_t ui8Offset;

    //
    // Build the SPI offset for writing a block of registers from the
    // user-supplied start point.
    //
    ui8Offset = 0x40 | ui8StartRegister;

    //
    // Check to see if the callback pointer is valid.
    //
    if ( pfnCallback )
    {
        //
        // If so, use a non-blocking call with a callback.
        //
        am_hal_iom_queue_spi_write(psDevice->ui32IOMModule,
                                   psDevice->ui32ChipSelect,
                                   pui32Values, ui32NumBytes,
                                   AM_HAL_IOM_OFFSET(ui8Offset),
                                   pfnCallback);
    }
    else
    {
        //
        // Otherwise, use a polled call.
        //
        am_hal_iom_spi_write(psDevice->ui32IOMModule,
                             psDevice->ui32ChipSelect,
                             pui32Values, ui32NumBytes,
                             AM_HAL_IOM_OFFSET(ui8Offset));
    }
}

//*****************************************************************************
//
// End Doxygen group.
//! @}
//
//*****************************************************************************

