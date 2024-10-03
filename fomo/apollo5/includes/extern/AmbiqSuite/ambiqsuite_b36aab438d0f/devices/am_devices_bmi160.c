//*****************************************************************************
//
//! @file am_devices_bmi160.c
//!
//! @brief Driver to interface with the BMI160.
//!
//! These functions implement the BMI160 support routines for use on Ambiq
//! Micro MCUs.
//!
//! @addtogroup bmi160 BMI160 SPI Device Control for the BMI160 External Accel/Gyro
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
#include "am_devices_bmi160.h"
#include "am_util_delay.h"

//*****************************************************************************
//
//  Clear one or more bits.
//
//*****************************************************************************
void
am_devices_bmi160_reg_clear(const am_devices_bmi160_t *psDevice,
                            uint8_t ui8Address,
                            uint8_t ui8Mask)
{
    uint8_t ui8Temp;

    ui8Temp = am_devices_bmi160_reg_read(psDevice, ui8Address);
    ui8Temp &= ~ui8Mask;
    am_devices_bmi160_reg_write(psDevice, ui8Address, ui8Temp);
}

//*****************************************************************************
//
// Set one or more bits.
//
//*****************************************************************************
void
am_devices_bmi160_reg_set(const am_devices_bmi160_t *psDevice,
                          uint8_t ui8Address,
                          uint8_t ui8Mask)
{
    uint8_t ui8Temp;

    ui8Temp = am_devices_bmi160_reg_read(psDevice, ui8Address);
    ui8Temp |= ui8Mask;
    am_devices_bmi160_reg_write(psDevice, ui8Address, ui8Temp);
}

//*****************************************************************************
//
// Reads an internal register in the BMI160.
//
//*****************************************************************************
uint8_t
am_devices_bmi160_reg_read(const am_devices_bmi160_t *psDevice,
                           uint8_t ui8Register)
{
    uint8_t ui8Offset;
    am_hal_iom_buffer(1) sData;

    //
    // Build the SPI offset and the data buffer.
    //
    ui8Offset = ui8Register;

    //
    // If configured for SPI mode, use SPI read, otherwise use I2C read.
    //
    if ( psDevice->bMode == AM_DEVICES_BMI160_MODE_SPI )
    {
        //
        // turn on the read bit in the offset value
        //
        ui8Offset = ui8Register | 0x80;

        //
        // Send the read to the bus using the polled API.
        //
        am_hal_iom_spi_read(psDevice->ui32IOMModule, psDevice->ui32ChipSelect,
                            sData.words, 1, AM_HAL_IOM_OFFSET(ui8Offset));
    }
    else
    {

        //
        // Send the write to the bus using the polled API.
        //
        am_hal_iom_i2c_read(psDevice->ui32IOMModule, psDevice->ui32Address,
                            sData.words, 1, AM_HAL_IOM_OFFSET(ui8Offset));
    }

    //
    // Return the retrieved data.
    //
    return sData.bytes[0];
}

//*****************************************************************************
//
//  Writes an internal register in the BMI160.
//
//*****************************************************************************
void
am_devices_bmi160_reg_write(const am_devices_bmi160_t *psDevice,
                            uint8_t ui8Register, uint8_t ui8Value)
{
    uint8_t ui8Offset;
    am_hal_iom_buffer(1) sData;

    //
    // Build the offset and the data buffer.
    //
    ui8Offset = ui8Register;
    sData.bytes[0] = ui8Value;

    //
    // If configured for SPI mode, use SPI read, otherwise use I2C read.
    //
    if ( psDevice->bMode == AM_DEVICES_BMI160_MODE_SPI )
    {

        //
        // Send the write to the bus using the polled API.
        //
        am_hal_iom_spi_write(psDevice->ui32IOMModule, psDevice->ui32ChipSelect,
                             sData.words, 1, AM_HAL_IOM_OFFSET(ui8Offset));
    }
    else
    {

        //
        // Send the write to the bus using the polled API.
        //
        am_hal_iom_i2c_write(psDevice->ui32IOMModule, psDevice->ui32Address,
                             sData.words, 1, AM_HAL_IOM_OFFSET(ui8Offset));

    }

    //
    // Delay after write to guarantee we exceed the ~400us delay needed
    // when the device is in low power mode (default out of POR).
    //
    am_util_delay_ms(1);
}

//*****************************************************************************
//
// Reads a block of internal registers in the BMI160.
//
//
// This function performs a read to a block of BMI160 registers over the
// serial bus. If the  pfnCallback parameter is nonzero, this function will
// use the am_hal_iom_queue_spi_read() function as the underlying interface, and
//  pfnCallback will be provided to the HAL as the IOM callback function.
// Otherwise, the SPI read will be polled.
//
//*****************************************************************************
void
am_devices_bmi160_reg_block_read(const am_devices_bmi160_t *psDevice,
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
    ui8Offset = ui8StartRegister;

    //
    // If configured for SPI mode, use SPI read, otherwise use I2C read.
    //
    if ( psDevice->bMode == AM_DEVICES_BMI160_MODE_SPI )
    {
        //
        // Make sure R/W bit is set for reading
        //
        ui8Offset = ui8StartRegister | 0x80;

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
    else
    {
        //
        // Check to see if the callback pointer is valid.
        //
        if ( pfnCallback )
        {
            //
            // If so, use a non-blocking call with a callback.
            //
            am_hal_iom_queue_i2c_read(psDevice->ui32IOMModule,
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
}

//*****************************************************************************
//
//  Writes a block of internal registers in the BMI160.
//
//
//*****************************************************************************
void
am_devices_bmi160_reg_block_write(const am_devices_bmi160_t *psDevice,
                                  uint8_t ui8StartRegister,
                                  uint32_t *pui32Values,
                                  uint32_t ui32NumBytes,
                                  am_hal_iom_callback_t pfnCallback)
{
    uint8_t ui8Offset;

    //
    // Build the offset for writing a block of registers from the
    // user-supplied start point.
    //
    ui8Offset = ui8StartRegister;

    //
    // If configured for SPI mode, use SPI read, otherwise use I2C read.
    //
    if ( psDevice->bMode == AM_DEVICES_BMI160_MODE_SPI )
    {
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
    else
    {
        //
        // Check to see if the callback pointer is valid.
        //
        if ( pfnCallback )
        {
            //
            // If so, use a non-blocking call with a callback.
            //
            am_hal_iom_queue_i2c_write(psDevice->ui32IOMModule,
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
}

//*****************************************************************************
//
// Soft Reset the BMI160
//
//*****************************************************************************
void
am_devices_bmi160_reset(const am_devices_bmi160_t *psDevice)
{
    //
    // Issue a soft reset.
    //
    am_devices_bmi160_reg_write(psDevice, AM_DEVICES_BMI160_CMD, 0xB6);
}

//*****************************************************************************
//
// enable the BMI160.
//
//*****************************************************************************
void
am_devices_bmi160_enable(const am_devices_bmi160_t *psDevice,
                         int data_rate,
                         bool enable_accel,
                         bool enable_gyro)
{
    if ( data_rate > 0 )
    {
        if ( enable_accel )
        {
            am_devices_bmi160_reg_write(psDevice, AM_DEVICES_BMI160_CMD,
                                        AM_DEVICES_BMI160_ACCEL_MODE_NORMAL);
        }

        if ( enable_gyro )
        {
            am_devices_bmi160_reg_write(psDevice, AM_DEVICES_BMI160_CMD,
                                        AM_DEVICES_BMI160_GYRO_MODE_NORMAL);
        }

        uint8_t acc_regval = 0;
        uint8_t gyro_regval = 0;

        if ( data_rate >= 100 )
        {
            acc_regval = BMI160_ACCEL_OUTPUT_DATA_RATE_100HZ;
            gyro_regval = BMI160_GYRO_OUTPUT_DATA_RATE_100HZ;
        }
        else if ( data_rate >= 50 )
        {
            acc_regval = BMI160_ACCEL_OUTPUT_DATA_RATE_50HZ;
            gyro_regval = BMI160_GYRO_OUTPUT_DATA_RATE_50HZ;
        }
        else
        {
            acc_regval = BMI160_ACCEL_OUTPUT_DATA_RATE_25HZ;
            gyro_regval = BMI160_GYRO_OUTPUT_DATA_RATE_25HZ;
        };

        am_devices_bmi160_reg_write(
            psDevice,
            AM_DEVICES_BMI160_GYR_CONF,
            gyro_regval);

        am_devices_bmi160_reg_write(
            psDevice,
            AM_DEVICES_BMI160_ACC_CONF,
            acc_regval);

        //
        // Set the Accel range to 16g
        //
        am_devices_bmi160_reg_write(psDevice,
                                    AM_DEVICES_BMI160_ACC_RANGE, 0x0C);

        //
        // Map INT1 to the data-ready interrupt
        //
        am_devices_bmi160_reg_write(psDevice,
                                    AM_DEVICES_BMI160_INT_MAP_1, 0x80);

        //
        // Enable INT 1 as data-ready
        //
        am_devices_bmi160_reg_write(psDevice,
                                    AM_DEVICES_BMI160_INT_EN_1, 0x10);
    }
    else
    {
        am_devices_bmi160_reg_write(psDevice, AM_DEVICES_BMI160_CMD,
                                    AM_DEVICES_BMI160_ACCEL_MODE_SUSPEND);
        am_devices_bmi160_reg_write(psDevice, AM_DEVICES_BMI160_CMD,
                                    AM_DEVICES_BMI160_GYRO_MODE_SUSPEND);
    }
}

//*****************************************************************************
//
// Initializes the BMI160.
//
//*****************************************************************************
void
am_devices_bmi160_config(const am_devices_bmi160_t *psDevice)
{
    uint8_t ui8Status = 0;
    uint8_t ui8Attempts = 50;

    //
    // Reset the BMI160 sensor
    //
    am_devices_bmi160_reset(psDevice);

    //
    // Put accel and gyro in normal mode.
    //
    while ( ui8Status != 0x14 && ui8Attempts-- )
    {
        //
        // Place accel & gyro in normal mode and verify status
        //
        am_devices_bmi160_reg_write(psDevice, AM_DEVICES_BMI160_CMD,
                                    AM_DEVICES_BMI160_ACCEL_MODE_NORMAL);
        am_devices_bmi160_reg_write(psDevice, AM_DEVICES_BMI160_CMD,
                                    AM_DEVICES_BMI160_GYRO_MODE_NORMAL);
        am_util_delay_ms(1);
        ui8Status = am_devices_bmi160_reg_read(psDevice,
                                               AM_DEVICES_BMI160_PMU_STATUS);
    }

    //
    // BMI160 not in correct power mode
    //
    if ( !ui8Attempts )
    {
        return;
    }

    //
    // Read status register to clear it.
    //
    ui8Status = am_devices_bmi160_reg_read(psDevice,
                                           AM_DEVICES_BMI160_ERR_REG);
    //
    // Set the Accel range to 16g
    //
    am_devices_bmi160_reg_write(psDevice,
                                AM_DEVICES_BMI160_ACC_RANGE, 0x0C);

    //
    // Enable INT 1 output as active high
    //
    am_devices_bmi160_reg_write(psDevice,
                                AM_DEVICES_BMI160_INT_OUT_CTRL, 0x0A);
    //
    // Latch interrupts
    //
    am_devices_bmi160_reg_write(psDevice,
                                AM_DEVICES_BMI160_INT_LATCH, 0x0F);

    //
    // Map data-ready to INT 1
    //
    if ( psDevice->ui32Samples > 1 )
    {
        //
        // Enable accel & gyro axis in the FIFO
        //
        am_devices_bmi160_reg_write(psDevice,
                                    AM_DEVICES_BMI160_FIFO_CFG_1, 0xC0);

        //
        // Map INT1 to the fifo watermark interrupt
        am_devices_bmi160_reg_write(psDevice,
                                    AM_DEVICES_BMI160_INT_MAP_1, 0x40);

        //
        // Set the FIFO watermark
        //
        am_devices_bmi160_reg_write(psDevice,
                                    AM_DEVICES_BMI160_FIFO_CFG_0,
                                    (psDevice->ui32Samples & 0xFF) * 3);

        ui8Status = am_devices_bmi160_reg_read(psDevice,
                                           AM_DEVICES_BMI160_FIFO_CFG_0);

        //
        // Enable INT 1 as FIFO watermark
        //
        am_devices_bmi160_reg_write(psDevice,
                                    AM_DEVICES_BMI160_INT_EN_1, 0x40);

        //
        // FIFO Flush
        //
        am_devices_bmi160_reg_write(psDevice, AM_DEVICES_BMI160_CMD, 0xB0);

    }
    else
    {
        //
        // Map INT1 to the data-ready interrupt
        //
        am_devices_bmi160_reg_write(psDevice,
                                    AM_DEVICES_BMI160_INT_MAP_1, 0x80);

        //
        // Enable INT 1 as data-ready
        //
        am_devices_bmi160_reg_write(psDevice,
                                    AM_DEVICES_BMI160_INT_EN_1, 0x10);
    }
}

//*****************************************************************************
//
// Gets the BMI160 samples.
//
//
//*****************************************************************************
void
am_devices_bmi160_sample_get(const am_devices_bmi160_t *psDevice,
                             uint32_t ui32NumSamples,
                             uint32_t *pui32ReturnBuffer,
                             am_devices_bmi160_callback_t pfnCallback)
{
    //
    // Read the registers as a single block.
    //
    if ( psDevice->ui32Samples > 1 )
    {
        am_devices_bmi160_reg_block_read(psDevice, AM_DEVICES_BMI160_FIFO_DATA,
                                        pui32ReturnBuffer,
                                        (ui32NumSamples * 12),
                                        pfnCallback);
    }
    else
    {
        am_devices_bmi160_reg_block_read(psDevice,
                                        AM_DEVICES_BMI160_DATA_GYR_X_L,
                                        pui32ReturnBuffer,
                                        12,
                                        pfnCallback);
    }
}

//*****************************************************************************
//
// Get the device ID.
//
//*****************************************************************************
uint8_t
am_devices_bmi160_device_id_get(const am_devices_bmi160_t *psDevice)
{
    //
    // Return the device ID.
    //
    return am_devices_bmi160_reg_read(psDevice,
                                            AM_DEVICES_BMI160_CHIPID);
}

//*****************************************************************************
//
// Enable/Disable an interrupt on any motion.
//
//*****************************************************************************
void
am_devices_bmi160_enable_any_motion(const am_devices_bmi160_t *psDevice, bool bEnable)
{
    uint32_t ui32RegVal = 0;
    uint32_t ui32RegValArray[3];

    if ( bEnable )
    {
        //
        // Map Any Motion to Int1
        //
        ui32RegVal = AM_DEVICES_BMI160_SET_BITSLICE(
            ui32RegVal, AM_DEVICES_BMI160_USER_INTR_MAP_0_INTR1_ANY_MOTION, 1);
        am_devices_bmi160_reg_write(
            psDevice,
            AM_DEVICES_BMI160_USER_INTR_MAP_0_INTR1_ANY_MOTION__REG,
            (uint8_t)ui32RegVal);

        //
        // Configure registers for any-motion interrupt.
        // Duration in samples.
        //
        ui32RegValArray[0] = AM_DEVICES_BMI160_SET_BITSLICE(
            0, AM_DEVICES_BMI160_USER_INTR_MOTION_0_INTR_ANY_MOTION_DURN, 1);

        //
        // Threshold in LSB's.
        //
        ui32RegValArray[1] = AM_DEVICES_BMI160_SET_BITSLICE(
            0, AM_DEVICES_BMI160_USER_INTR_MOTION_1_INTR_ANY_MOTION_THRES, 1);

        //
        // Enable any motion.
        //
        ui32RegValArray[2] = AM_DEVICES_BMI160_SET_BITSLICE(
            0,
            AM_DEVICES_BMI160_USER_INTR_ENABLE_0_ANY_MOTION_X_ENABLE,
            1);
        ui32RegValArray[2] = AM_DEVICES_BMI160_SET_BITSLICE(
            ui32RegValArray[2],
            AM_DEVICES_BMI160_USER_INTR_ENABLE_0_ANY_MOTION_Y_ENABLE,
            1);
        ui32RegValArray[2] = AM_DEVICES_BMI160_SET_BITSLICE(
            ui32RegValArray[2],
            AM_DEVICES_BMI160_USER_INTR_ENABLE_0_ANY_MOTION_Z_ENABLE,
            1);

        //ui32RegVal = ACCEL_MODE_LOWPOWER;
        //am_devices_bmi160_reg_write(psDevice, AM_DEVICES_BMI160_CMD,
        //                            &ui32RegVal, 1);

        ui32RegVal = AM_DEVICES_BMI160_GYRO_MODE_SUSPEND;
        am_devices_bmi160_reg_write(psDevice,
                                    AM_DEVICES_BMI160_CMD,
                                    (uint8_t)ui32RegVal);

        //
        // Enable it Data Ready.
        // Not sure what to do with read value??? STATUS_OK does not exist.
        //
        ui32RegVal = am_devices_bmi160_reg_read(psDevice,
                                                AM_DEVICES_BMI160_INT_EN_1);

        ui32RegVal = AM_DEVICES_BMI160_SET_BITSLICE(
            ui32RegVal,
            AM_DEVICES_BMI160_USER_INTR_ENABLE_1_DATA_RDY_ENABLE,
            0);
        am_devices_bmi160_reg_write(psDevice, AM_DEVICES_BMI160_INT_EN_1,
                                    (uint8_t)ui32RegVal);

    }
    else
    {
        ui32RegValArray[0] = 0;
        ui32RegValArray[1] = 0;
        ui32RegValArray[2] = 0;
    }

    //
    // Write the registers.
    //
    ui32RegVal = 0;
    am_devices_bmi160_reg_write(psDevice, AM_DEVICES_BMI160_INT_MOTION_0,
                                (uint8_t)ui32RegValArray[0]);
    am_devices_bmi160_reg_write(psDevice, AM_DEVICES_BMI160_INT_MOTION_1,
                                (uint8_t)ui32RegValArray[1]);
    am_devices_bmi160_reg_write(psDevice, AM_DEVICES_BMI160_INT_MOTION_2,
                                (uint8_t)ui32RegVal);
    am_devices_bmi160_reg_write(psDevice, AM_DEVICES_BMI160_INT_MOTION_3,
                                (uint8_t)ui32RegVal);
    am_devices_bmi160_reg_write(psDevice, AM_DEVICES_BMI160_INT_EN_0,
                                (uint8_t)ui32RegValArray[2]);
}

//*****************************************************************************
//
// End Doxygen group.
//! @}
//
//*****************************************************************************

