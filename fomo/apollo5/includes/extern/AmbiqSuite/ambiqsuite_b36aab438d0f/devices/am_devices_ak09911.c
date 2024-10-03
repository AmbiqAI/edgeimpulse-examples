//*****************************************************************************
//
//! @file am_devices_ak09911.c
//!
//! @brief Driver for the ST Microelectronics AK09911 magnetometer
//!
//! @addtogroup ak09911 SPI Device Control for the AK09911 External Magnetometer
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
#include "am_devices_ak09911.h"
#include "am_util.h"

//*****************************************************************************
//
//  Reset the AK09911 mag
//
//*****************************************************************************
void
am_devices_ak09911_reset(am_devices_ak09911_t *psDevice)
{
    //
    // Reset the AK09911 Mag.
    //
    am_devices_ak09911_reg_write(psDevice, AM_DEVICES_AK09911_CNTL3, 0x01);
}

//*****************************************************************************
//
//  Get the device ID
//
//*****************************************************************************
uint16_t
am_devices_ak09911_device_id_get(am_devices_ak09911_t *psDevice)
{
    //
    // Read the WHO_AM_I register and return the result.
    //
    return (am_devices_ak09911_reg_read(psDevice, AM_DEVICES_AK09911_WIA2) << 8) |
                  am_devices_ak09911_reg_read(psDevice, AM_DEVICES_AK09911_WIA1);
}

//*****************************************************************************
//
//  Configures the AK09911 for operation.
//
//*****************************************************************************
void
am_devices_ak09911_config(am_devices_ak09911_t *psDevice)
{
    am_hal_ak09911_regs(6) sRegBuffer;

    //
    // Reset the device.
    //
    am_devices_ak09911_reset(psDevice);
    am_util_delay_ms(1);

    sRegBuffer.bytes[0] = 0x1;     // CTRL_REG2 (continuous mode 0)

    //
    // Write the control registers to the device as one solid block write.
    //
    am_devices_ak09911_reg_write(psDevice, AM_DEVICES_AK09911_CNTL2,
                                 sRegBuffer.bytes[0]);

    //
    // Clear out any old data that might be in the mag.
    //
    am_devices_ak09911_sample_get(psDevice, sRegBuffer.words, 0);
}

//*****************************************************************************
//
// @brief Triggers the next sample from the AK09911
//
//*****************************************************************************
void
am_devices_ak09911_sample_trigger(am_devices_ak09911_t *psDevice)
{
    am_devices_ak09911_reg_write(psDevice, AM_DEVICES_AK09911_CNTL2, 0x1);
}

//*****************************************************************************
//
//  Retrieves the most recent sample from the AK09911.
//
//*****************************************************************************
void
am_devices_ak09911_sample_get(am_devices_ak09911_t *psDevice, uint32_t *psData,
                              am_hal_iom_callback_t pfnCallback)
{
    uint8_t ui8Value;

    am_devices_ak09911_reg_write(psDevice, AM_DEVICES_AK09911_CNTL2, 0x1);

    while (1)
    {
        ui8Value = am_devices_ak09911_reg_read(psDevice, AM_DEVICES_AK09911_ST1);

        if (ui8Value & 0x1)
        {
            break;
        }

        if (ui8Value & 0x2)
        {
            am_devices_ak09911_reg_read(psDevice, AM_DEVICES_AK09911_ST2);
        }
    }

    //
    // Read the magnetometer registers as a single block.
    //
    am_devices_ak09911_reg_block_read(psDevice, AM_DEVICES_AK09911_OUT_X_L,
                                      psData, 6, pfnCallback);
}

//*****************************************************************************
//
//  Reads an internal register in the AK09911.
//
//*****************************************************************************
uint8_t
am_devices_ak09911_reg_read(am_devices_ak09911_t *psDevice,
                            uint8_t ui8Register)
{
    uint8_t ui8Offset;
    am_hal_iom_buffer(1) sData;

    //
    // Build the data buffer.
    //
    ui8Offset = ui8Register;

    //
    // Send the read to the bus using the polled API.
    //
    am_hal_iom_i2c_read(psDevice->ui32IOMModule, psDevice->ui32Address,
                        sData.words, 1, AM_HAL_IOM_OFFSET(ui8Offset));

    //
    // Return the retrieved data.
    //
    return sData.bytes[0];
}

//*****************************************************************************
//
//  Reads a block of internal registers in the AK09911.
//
//*****************************************************************************
void
am_devices_ak09911_reg_block_read(am_devices_ak09911_t *psDevice,
                                  uint8_t ui8StartRegister,
                                  uint32_t *pui32Values,
                                  uint32_t ui32NumBytes,
                                  am_hal_iom_callback_t pfnCallback)
{
    uint8_t ui8Offset;
    ui8Offset = ui8StartRegister;

    //
    // Check to see if the callback pointer is valid.
    //
    if ( pfnCallback )
    {
        //
        // If so, use a non-blocking call with a callback.
        //
        am_hal_iom_i2c_read_nb(psDevice->ui32IOMModule,
                               psDevice->ui32Address,
                               pui32Values, ui32NumBytes,
                               AM_HAL_IOM_OFFSET(ui8Offset),
                               pfnCallback);
    }
    else
    {
        //
        // Otherwise, use a polled call.
        //
        am_hal_iom_i2c_read(psDevice->ui32IOMModule,
                            psDevice->ui32Address,
                            pui32Values, ui32NumBytes,
                            AM_HAL_IOM_OFFSET(ui8Offset));
    }
}

//*****************************************************************************
//
//  Writes an internal register in the AK09911.
//
//*****************************************************************************
void
am_devices_ak09911_reg_write(am_devices_ak09911_t *psDevice,
                             uint8_t ui8Register, uint8_t ui8Value)
{
    uint8_t ui8Offset;
    am_hal_iom_buffer(1) sData;

    //
    // Build the data buffer.
    //
    ui8Offset = ui8Register;
    sData.bytes[0] = ui8Value;

    //
    // Send the write to the bus using the polled API.
    //
    am_hal_iom_i2c_write(psDevice->ui32IOMModule, psDevice->ui32Address,
                         sData.words, 1, AM_HAL_IOM_OFFSET(ui8Offset));
}

//*****************************************************************************
//
//  Writes a block of internal registers in the AK09911.
//
//*****************************************************************************
void
am_devices_ak09911_reg_block_write(am_devices_ak09911_t *psDevice,
                                   uint8_t ui8StartRegister,
                                   uint32_t *pui32Values,
                                   uint32_t ui32NumBytes,
                                   am_hal_iom_callback_t pfnCallback)
{
    uint8_t ui8Offset;
    ui8Offset = ui8StartRegister;

    //
    // Check to see if the callback pointer is valid.
    //
    if ( pfnCallback )
    {
        //
        // If so, use a non-blocking call with a callback.
        //
        am_hal_iom_i2c_write_nb(psDevice->ui32IOMModule,
                                psDevice->ui32Address,
                                pui32Values, ui32NumBytes,
                                AM_HAL_IOM_OFFSET(ui8Offset),
                                pfnCallback);
    }
    else
    {
        //
        // Otherwise, use a polled call.
        //
        am_hal_iom_i2c_write(psDevice->ui32IOMModule,
                             psDevice->ui32Address,
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

