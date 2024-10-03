//*****************************************************************************
//
//! @file am_devices_l3gd20h.c
//!
//! @brief Driver to interface with the L3GD20H
//!
//! These functions implement the L3GD20H support routines for use on Ambiq
//! Micro MCUs.
//!
//! @addtogroup L3GD20H SPI Device Control for the L3GD20H External Gyro
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

#include "am_bsp.h"
#include "am_devices_l3gd20h.h"

//*****************************************************************************
//
//  Reset the L3GD20H gyro
//
//*****************************************************************************
void
am_devices_l3gd20h_reset(am_devices_l3gd20h_t *psDevice)
{
    //
    // Reset the device.
    //
    am_devices_l3gd20h_reg_write(psDevice, AM_DEVICES_L3GD20H_LOW_ODR, 0x04);
}

//*****************************************************************************
//
//  Get the device ID
//
//*****************************************************************************
uint8_t
am_devices_l3gd20h_device_id_get(am_devices_l3gd20h_t *psDevice)
{
    //
    // Read the WHO_AM_I register and return the result.
    //
    return am_devices_l3gd20h_reg_read(psDevice, AM_DEVICES_L3GD20H_WHO_AM_I);
}

//*****************************************************************************
//
//  Initialize the L3GD20H
//
//*****************************************************************************
void
am_devices_l3gd20h_config(am_devices_l3gd20h_t *psDevice)
{
    am_hal_iom_buffer(4) sCommand;

    //
    // Reset the device.
    //
    am_devices_l3gd20h_reset(psDevice);

    //
    // Default initialization:
    // (Default filters)
    // (Little-Endian, 245dps, 4-wire SPI)
    // (FIFO Disabled)
    //
    sCommand.bytes[0] = 0x00;   // CTRL2
    sCommand.bytes[1] = 0x00;   // CTRL3
    sCommand.bytes[2] = 0x00;   // CTRL4
    sCommand.bytes[3] = 0x00;   // CTRL5

    //
    // If fifo size is greater than one configure fifo
    //
    if (psDevice->ui32Samples > 1)
    {
        //
        // Calculate the correct FIFO control word. The lower 5 bits are the fifo
        // threshold, and the upper three are the fifo operating mode. Take the
        // user-supplied threshold, and put the fifo in "dynamic stream" mode.
        //
        am_devices_l3gd20h_reg_write(psDevice,
                                     AM_DEVICES_L3GD20H_FIFO_CTRL,
                                     (0xC0 | (psDevice->ui32Samples & 0x1F)));

        sCommand.bytes[1] = 0x04;   // INT2 on FIFO Threshold
        sCommand.bytes[3] = 0x40;   // Enable FIFO
    }
    else
    {
        sCommand.bytes[1] = 0x08;   // INT2 on data-ready
    }

    //
    // Write the register values to the block of registers starting with CTRL2.
    //
    am_devices_l3gd20h_reg_block_write(psDevice, AM_DEVICES_L3GD20H_CTRL2,
                                       sCommand.words, 4, 0);

    //
    // Write CTRL1 to enable axis and start the sampling process.
    //
    am_devices_l3gd20h_reg_write(psDevice, AM_DEVICES_L3GD20H_CTRL1, 0x0F);
}

//*****************************************************************************
//
//  Read a series of gyroscope samples from the L3GD20H.
//
//*****************************************************************************
void
am_devices_l3gd20h_sample_read(am_devices_l3gd20h_t *psDevice,
                               uint32_t *pui32Data,
                               am_hal_iom_callback_t pfnCallback)
{
    //
    // Start a block read from the beginning of the L3GD20H's FIFO read
    // location. The L3GD20H should handle the address increment and wrap
    // automatically.
    //
    am_devices_l3gd20h_reg_block_read(psDevice, AM_DEVICES_L3GD20H_OUT_X_L,
                                      pui32Data,
                                      (psDevice->ui32Samples * 6),
                                      pfnCallback);
}

//*****************************************************************************
//
//  Get the state of the control registers from the L3GD20H Gyro.
//
//*****************************************************************************
void
am_devices_l3gd20h_ctrl_reg_state_get(am_devices_l3gd20h_t *psDevice,
                                      uint32_t *pui32Return)
{
    //
    // Read the control registers from the device as one solid block.
    //
    am_devices_l3gd20h_reg_block_read(psDevice, AM_DEVICES_L3GD20H_CTRL1,
                                      pui32Return, 5, 0);
}

//*****************************************************************************
//
//  Reads an internal register in the L3GD20H.
//
//*****************************************************************************
uint8_t
am_devices_l3gd20h_reg_read(am_devices_l3gd20h_t *psDevice,
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
//  Reads a block of internal registers in the L3GD20H.
//
//*****************************************************************************
void
am_devices_l3gd20h_reg_block_read(am_devices_l3gd20h_t *psDevice,
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
    if (pfnCallback)
    {
        //
        // If so, use a non-blocking call with a callback.
        //
        am_hal_iom_spi_read_nb(psDevice->ui32IOMModule,
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
//  Writes an internal register in the L3GD20H.
//
//*****************************************************************************
void
am_devices_l3gd20h_reg_write(am_devices_l3gd20h_t *psDevice,
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
//  Writes a block of internal registers in the L3GD20H.
//
//*****************************************************************************
void
am_devices_l3gd20h_reg_block_write(am_devices_l3gd20h_t *psDevice,
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
    if (pfnCallback)
    {
        //
        // If so, use a non-blocking call with a callback.
        //
        am_hal_iom_spi_write_nb(psDevice->ui32IOMModule,
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

