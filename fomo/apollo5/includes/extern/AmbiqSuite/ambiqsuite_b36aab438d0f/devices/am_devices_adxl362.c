//*****************************************************************************
//
//! @file am_devices_adxl362.c
//!
//! @brief Driver to interface with the ADXL362
//!
//! These functions implement the ADXL362 support routines for use on Ambiq
//! Micro MCUs.
//!
//! @addtogroup adxl362 ADXL362 SPI Driver
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

#include "am_bsp.h"
#include "am_util.h"
#include "am_devices_adxl362.h"

//*****************************************************************************
//
// Initialize the ADXL362.
//
//*****************************************************************************
void
am_devices_adxl362_config(const am_devices_adxl362_t *psDevice)
{
    am_hal_iom_buffer(15) sCommand;

    //
    // Reset the adxl362
    //
    am_devices_adxl362_reset(psDevice);

    //
    // Setup default register state command string to send to ADXL362.
    //
    sCommand.bytes[14] = 0x13;  // reg #2C FILTER_CTL 100Hz sample rate
    sCommand.bytes[13] = 0x02;  // reg #2B INTMAP2    Interrupt on FIFO ready
    sCommand.bytes[12] = 0x04;  // reg #2A INTMAP1    Interrupt on water mark
    sCommand.bytes[11] = 0x86;  // reg #29 FIFO_SAMPLES == 130x3 (0x186) NOTE: AH = 1
    sCommand.bytes[10] = 0x0A;  // reg #28 FIFO_CTRL   AH=1  FIFO MODE = stream mode
    sCommand.bytes[9]  = 0x00;  // reg #27 ACT_INACT_CTL
    sCommand.bytes[8]  = 0x02;  // reg #26 TIME_INACT_H
    sCommand.bytes[7]  = 0x00;  // reg #25 TIME_INACT_L
    sCommand.bytes[6]  = 0x00;  // reg #24 THRESHOLD_INACT_H
    sCommand.bytes[5]  = 0x80;  // reg #23 THRESHOLD_INACT_L
    sCommand.bytes[4]  = 0x20;  // reg #22 TIME_ACT
    sCommand.bytes[3]  = 0x02;  // reg #21 THRESHOLD_ACT_H
    sCommand.bytes[2]  = 0x00;  // reg #20 THRESHOLD_ACT_L
    sCommand.bytes[1]  = 0x20;  // register address of THRESHOLD_ACT_L
    sCommand.bytes[0]  = 0x0A;  // SPI WRITE command for the ADXL362

    //
    // Handle optional RANGE setting
    //
    switch (psDevice->ui32Range)
    {
        case AM_DEVICES_ADXL362_2G:
            sCommand.bytes[14] = 0x00;  // reg +/- 2g
        break;

        case AM_DEVICES_ADXL362_4G:
            sCommand.bytes[14] = 0x40;  // reg +/- 4g
        break;

        case AM_DEVICES_ADXL362_8G:
            sCommand.bytes[14] = 0x80;  // reg +/- 8g
        break;

        default:
        break;
    }

    //
    // Handle optional sample rate selections
    //
    switch (psDevice->ui32SampleRate)
    {
        case AM_DEVICES_ADXL362_400HZ:
            sCommand.bytes[14] |= 0x05; //  400Hz
        break;

        case AM_DEVICES_ADXL362_200HZ:
            sCommand.bytes[14] |= 0x04; //  200Hz
        break;

        case AM_DEVICES_ADXL362_100HZ:
            sCommand.bytes[14] |= 0x03; //  100Hz
        break;

        default:
        break;
    }

    //
    // Handle optional half bandwidth reduction mode
    //
    if (psDevice->bHalfBandwidth)
    {
        sCommand.bytes[14] &= ~0x10;
    }

    //
    // Handle optional synchronous sampling mode.
    //
    if (psDevice->bSyncMode)
    {
        sCommand.bytes[14] = 0x09;  // reg #2C FILTER_CTL   synchronous sample mode
        sCommand.bytes[13] = 0x00;  // reg #2B INTMAP2    Not available in sync mode
    }

    //
    // Over ride the high water mark with the one supplied.
    //
    sCommand.bytes[11]  =  psDevice->ui32Samples & 0x0000000ff;
    sCommand.bytes[10] |= (psDevice->ui32Samples & 0x000000100) ? 0x00000008 : 0;

    //
    // Use polled IOM send routine to load the command registers.
    //
    am_hal_iom_spi_write(psDevice->ui32IOMModule,
                         psDevice->ui32ChipSelect, sCommand.words, 15,
                         AM_HAL_IOM_RAW);
}

//*****************************************************************************
//
// Reset the ADXL362.
//
//*****************************************************************************
void
am_devices_adxl362_reset(const am_devices_adxl362_t *psDevice)
{
    am_hal_iom_buffer(3) sCommand;

    //
    // Use polled IOM send routine to reset the ADXL362.
    //
    sCommand.bytes[2] = 0x52; // R for reset is a required parameter
    sCommand.bytes[1] = 0x1F; // register SOFT_RESET
    sCommand.bytes[0] = 0x0A; // SPI WRITE

    //
    // Write to the ADXL362 via the IOM.
    //
    am_hal_iom_spi_write(psDevice->ui32IOMModule,
                         psDevice->ui32ChipSelect, sCommand.words, 3,
                         AM_HAL_IOM_RAW);

    //
    // Add some delay to wait for the ADXL362 to finish its reset.
    //
    am_util_delay_ms(50);
}

//*****************************************************************************
//
// brief Set measurement mode for the ADXL362.
//
//*****************************************************************************
void
am_devices_adxl362_measurement_mode_set(const am_devices_adxl362_t *psDevice)
{
    am_hal_iom_buffer(3) sCommand;

    //
    // Use polled IOM send routine.
    //
    sCommand.bytes[2] = 0x02; // reg #2D POWER_CTL  set measurement mode
    sCommand.bytes[1] = 0x2D; // register address of POWER_CTL
    sCommand.bytes[0] = 0x0A; // SPI WRITE command for the ADXL362

    //
    // Write to the IOM.
    //
    am_hal_iom_spi_write(psDevice->ui32IOMModule,
                         psDevice->ui32ChipSelect, sCommand.words, 3,
                         AM_HAL_IOM_RAW);
}

//*****************************************************************************
//
//  Get FIFO depth for the ADXL362.
//
//*****************************************************************************
void
am_devices_adxl362_fifo_depth_get(const am_devices_adxl362_t *psDevice,
                                  uint32_t *pui32Return)
{
    am_hal_iom_buffer(2) sCommand;

    //
    // Use polled IOM send routine.
    //
    sCommand.bytes[1] = 0x0C; // register FIFO ENTRIES LOW
    sCommand.bytes[0] = 0x0B; // READ

    //
    // Write to the IOM.
    //
    am_hal_iom_spi_write(psDevice->ui32IOMModule,
                         psDevice->ui32ChipSelect, sCommand.words, 2,
                         AM_HAL_IOM_CS_LOW | AM_HAL_IOM_RAW);

    //
    // Read from the IOM.
    //
    am_hal_iom_spi_read(psDevice->ui32IOMModule,
                        psDevice->ui32ChipSelect, pui32Return, 2,
                        AM_HAL_IOM_RAW);

    //
    // Set the pointer.
    //
    *pui32Return &= 0x0000ffff;
}

//*****************************************************************************
//
//  Get a sample from the ADXL362
//
//*****************************************************************************
void
am_devices_adxl362_sample_get(const am_devices_adxl362_t *psDevice,
                              uint32_t ui32NumSamples,
                              uint32_t *pui32ReturnBuffer,
                              am_devices_adxl362_callback_t pfnCallback)
{
    if (pfnCallback)
    {
        //
        // Start a non-blocking read from the ADXL. Pass the caller's callback
        // straight through to the IOM HAL.
        //
        am_hal_iom_spi_read_nb(psDevice->ui32IOMModule,
                               psDevice->ui32ChipSelect,
                               pui32ReturnBuffer,
                               (ui32NumSamples * 6),
                               AM_HAL_IOM_OFFSET(0xD),
                               pfnCallback);
    }
    else
    {
        //
        // If the caller didn't provide a callback, fall back to the polling
        // method. This consumes significantly more power and processor time.
        //
        am_hal_iom_spi_read(psDevice->ui32IOMModule,
                            psDevice->ui32ChipSelect,
                            pui32ReturnBuffer,
                            (ui32NumSamples * 6),
                            AM_HAL_IOM_OFFSET(0xD));
    }
}

//*****************************************************************************
//
//  Get the state of the control registers from the ADXL362.
//
//*****************************************************************************
void
am_devices_adxl362_ctrl_reg_state_get(const am_devices_adxl362_t *psDevice,
                                      uint32_t *pui32Return)
{
    am_hal_iom_buffer(2) sCommand;

    //
    // Use polled IOM send routine.
    //
    sCommand.bytes[1] = 0x20;
    sCommand.bytes[0] = 0x0B;

    //
    // Write to the IOM.
    //
    am_hal_iom_spi_write(psDevice->ui32IOMModule,
                         psDevice->ui32ChipSelect, sCommand.words, 2,
                         AM_HAL_IOM_CS_LOW | AM_HAL_IOM_RAW);

    //
    // Read from the IOM.
    //
    am_hal_iom_spi_read(psDevice->ui32IOMModule,
                        psDevice->ui32ChipSelect, pui32Return, 16,
                        AM_HAL_IOM_RAW);
}

//*****************************************************************************
//
//  Get the device ID.
//
//*****************************************************************************
void
am_devices_adxl362_device_id_get(const am_devices_adxl362_t *psDevice,
                                 uint32_t *pui32Return)
{
    am_hal_iom_buffer(2) sCommand;

    //
    // Use polled IOM send routine.
    //
    sCommand.bytes[1] = 0x00; // beginning of Device ID
    sCommand.bytes[0] = 0x0B;

    //
    // Write to the IOM.
    //
    am_hal_iom_spi_write(psDevice->ui32IOMModule,
                         psDevice->ui32ChipSelect, sCommand.words, 2,
                         AM_HAL_IOM_CS_LOW | AM_HAL_IOM_RAW);

    //
    // Read from the IOM.
    //
    am_hal_iom_spi_read(psDevice->ui32IOMModule,
                        psDevice->ui32ChipSelect, pui32Return, 4,
                        AM_HAL_IOM_RAW);
}

//*****************************************************************************
//
//  Write to ADXL62 control registers
//
//*****************************************************************************
void
am_devices_adxl362_reg_write(const am_devices_adxl362_t *psDevice,
                             uint8_t *pui8RegValues,
                             uint8_t ui8StartReg,
                             uint32_t ui32Number)
{
    int i;
    am_hal_iom_buffer(32) sCommand;

    //
    // We can't accept a register write of more than 32 bytes.
    //
    if (ui32Number > 30 || ui32Number < 1)
    {
        while (1);
    }

    //
    // Set up for an ADXL362 register write command string
    //
    sCommand.bytes[0] = 0x0A; // SPI WRITE command for the ADXL362
    sCommand.bytes[1] = ui8StartReg; // register address of THRESHOLD_ACT_L

    for (i = 0; i < ui32Number; i++)
    {
        //
        // Copy bytes to known word aligned buffer
        //
        sCommand.bytes[i + 2] = pui8RegValues[i];
    }

    //
    // Use polled IOM send routine to send the register values
    //
    am_hal_iom_spi_write(psDevice->ui32IOMModule,
                         psDevice->ui32ChipSelect, sCommand.words,
                         ui32Number + 2, AM_HAL_IOM_RAW);
}

//*****************************************************************************
//
//  Read the state of one or more ADXL362 control registers
//
//*****************************************************************************
void
am_devices_adxl362_ctrl_reg_read(const am_devices_adxl362_t *psDevice,
                                 uint8_t *pui8Return,
                                 uint8_t ui8StartReg,
                                 uint32_t ui32Number)
{
    int i;
    am_hal_iom_buffer(2) sCommand;
    am_hal_iom_buffer(16) sResult;

    //
    // Use polled IOM send routine.
    //
    sCommand.bytes[1] = ui8StartReg;
    sCommand.bytes[0] = 0x0B;

    //
    // Write to the IOM.
    //
    am_hal_iom_spi_write(psDevice->ui32IOMModule,
                         psDevice->ui32ChipSelect, sCommand.words, 2,
                         AM_HAL_IOM_CS_LOW | AM_HAL_IOM_RAW);

    //
    // Read from the IOM.
    //
    am_hal_iom_spi_read(psDevice->ui32IOMModule,
                        psDevice->ui32ChipSelect, sResult.words,
                        ui32Number, AM_HAL_IOM_RAW);

    //
    // Copy it back to the caller
    //
    for (i = 0; i < ui32Number; i++)
    {
        *pui8Return++ = sResult.bytes[i];
    }
}
//*****************************************************************************
//
// End Doxygen group.
//! @}
//
//*****************************************************************************

