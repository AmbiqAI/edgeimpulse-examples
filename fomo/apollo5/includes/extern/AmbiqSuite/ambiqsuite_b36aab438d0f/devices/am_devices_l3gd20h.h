//*****************************************************************************
//
//! @file am_devices_l3gd20h.h
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

#ifndef AM_DEVICES_L3GD20H_H
#define AM_DEVICES_L3GD20H_H

#ifdef __cplusplus
extern "C"
{
#endif

//*****************************************************************************
//
//! @name Defines
//! @{
//
//*****************************************************************************
#define AM_DEVICES_L3GD20H_WHO_AM_I         0x0F
#define AM_DEVICES_L3GD20H_CTRL1            0x20
#define AM_DEVICES_L3GD20H_CTRL2            0x21
#define AM_DEVICES_L3GD20H_CTRL3            0x22
#define AM_DEVICES_L3GD20H_CTRL4            0x23
#define AM_DEVICES_L3GD20H_CTRL5            0x24
#define AM_DEVICES_L3GD20H_REFERENCE        0x25
#define AM_DEVICES_L3GD20H_OUT_TEMP         0x26
#define AM_DEVICES_L3GD20H_STATUS           0x27
#define AM_DEVICES_L3GD20H_OUT_X_L          0x28
#define AM_DEVICES_L3GD20H_OUT_X_H          0x29
#define AM_DEVICES_L3GD20H_OUT_Y_L          0x2A
#define AM_DEVICES_L3GD20H_OUT_Y_H          0x2B
#define AM_DEVICES_L3GD20H_OUT_Z_L          0x2C
#define AM_DEVICES_L3GD20H_OUT_Z_H          0x2D
#define AM_DEVICES_L3GD20H_FIFO_CTRL        0x2E
#define AM_DEVICES_L3GD20H_FIFO_SRC         0x2F
#define AM_DEVICES_L3GD20H_IG_CFG           0x30
#define AM_DEVICES_L3GD20H_IG_SRC           0x31
#define AM_DEVICES_L3GD20H_IG_THS_XH        0x32
#define AM_DEVICES_L3GD20H_IG_THS_XL        0x33
#define AM_DEVICES_L3GD20H_IG_THS_YH        0x34
#define AM_DEVICES_L3GD20H_IG_THS_YL        0x35
#define AM_DEVICES_L3GD20H_IG_THS_ZH        0x36
#define AM_DEVICES_L3GD20H_IG_THS_ZL        0x37
#define AM_DEVICES_L3GD20H_IG_DURATION      0x38
#define AM_DEVICES_L3GD20H_LOW_ODR          0x39
//! @}  Defines

//*****************************************************************************
//
//! Device structure used for communication.
//
//*****************************************************************************
typedef struct
{
    //
    //! Module number to use for IOM access.
    //
    uint32_t ui32IOMModule;

    //
    //! Chip Select number to use for IOM access.
    //
    uint32_t ui32ChipSelect;

    //
    //! Number of samples to collect before interrupt.  1 sample consists of
    //! 6 bytes (2 bytes/axis)
    uint32_t ui32Samples;
}
am_devices_l3gd20h_t;

//*****************************************************************************
//
//! Buffer type to make reading l3gd20h samples easier.
//
//*****************************************************************************
#define am_devices_l3gd20h_sample(n)                                          \
    union                                                                     \
    {                                                                         \
        uint32_t words[((3 * n) + 1) >> 1];                                   \
        int16_t samples[3 * n];                                               \
    }

//*****************************************************************************
//
// External function definitions
//
//*****************************************************************************

//*****************************************************************************
//
//! @brief Writes a block of internal registers in the L3GD20H.
//!
//! @param psDevice         - Pointer to a device structure describing the L3GD20H.
//! @param ui8StartRegister - Address of the first register to write.
//! @param pui32Values      - Byte-packed array of data to write.
//! @param ui32NumBytes     - The total number of registers to write.
//! @param pfnCallback      - Optional callback function pointer.
//!
//! This function performs a write to a block of L3GD20H registers over the
//! serial bus. If the \e pfnCallback parameter is nonzero, this function will
//! use the am_hal_iom_spi_write_nb() function as the underlying interface, and
//! \e pfnCallback will be provided to the HAL as the IOM callback function.
//! Otherwise, the spi write will be polled.
//
//*****************************************************************************
extern void am_devices_l3gd20h_reg_block_write(
                                         am_devices_l3gd20h_t *psDevice,
                                         uint8_t ui8StartRegister,
                                         uint32_t *pui32Values,
                                         uint32_t ui32NumBytes,
                                         am_hal_iom_callback_t pfnCallback);

//*****************************************************************************
//
//! @brief Writes an internal register in the L3GD20H.
//!
//! @param psDevice      - Pointer to a device structure describing the L3GD20H.
//! @param ui8Register   - Address of the register to write.
//! @param ui8Value      - The value to write to the register.
//!
//! This function performs a write to an L3GD20H register over the serial bus.
//
//*****************************************************************************
extern void am_devices_l3gd20h_reg_write(am_devices_l3gd20h_t *psDevice,
                                         uint8_t ui8Register, uint8_t ui8Value);

//*****************************************************************************
//
//! @brief Reads a block of internal registers in the L3GD20H.
//!
//! @param psDevice         - Pointer to a device structure describing the L3GD20H.
//! @param ui8StartRegister - Address of the first register to read.
//! @param pui32Values      - Byte-packed array where the read data will go.
//! @param ui32NumBytes     - Total number of registers to read.
//! @param pfnCallback      - Optional callback function pointer.
//!
//! This function performs a read to a block of L3GD20H registers over the
//! serial bus. If the \e pfnCallback parameter is nonzero, this function will
//! use the am_hal_iom_spi_read_nb() function as the underlying interface, and
//! \e pfnCallback will be provided to the HAL as the IOM callback function.
//! Otherwise, the spi read will be polled.
//
//*****************************************************************************
extern void am_devices_l3gd20h_reg_block_read(
                                         am_devices_l3gd20h_t *psDevice,
                                         uint8_t ui8StartRegister,
                                         uint32_t *pui32Values,
                                         uint32_t ui32NumBytes,
                                         am_hal_iom_callback_t pfnCallback);

//*****************************************************************************
//
//! @brief Reads an internal register in the L3GD20H.
//!
//! @param psDevice is a pointer to a device structure describing the L3GD20H.
//! @param ui8Register is the address of the register to read.
//!
//! This function performs a read to an L3GD20H register over the serial bus.
//!
//! @return Retrieved Data
//
//*****************************************************************************
extern uint8_t am_devices_l3gd20h_reg_read(
                                         am_devices_l3gd20h_t *psDevice,
                                         uint8_t ui8Register);

//*****************************************************************************
//
//! @brief Read a series of gyroscope samples from the L3GD20H.
//!
//! @param psDevice     - Pointer to a device structure describing the L3GD20H
//! @param pui32Data    - Buffer where samples will be placed
//! @param pfnCallback  - Optional callback to be called on completion.
//!
//! This sample retrieves a set of gyroscope data from the L3GD20H. Each
//! triplet is 6 bytes long (2 bytes per axis). They are placed into the
//! caller-supplied array in a byte-packed format.
//!
//! If the \e pfnCallback parameter is non-null, this function will use the
//! am_hal_iom_spi_read_nb() function as the transport mechanism. The
//! caller-supplied callback function will be passed to the IOM HAL, and it
//! will be called from the IOM interrupt handler when the last byte is
//! received. If \e pfnCallback is not provided, this function will use a
//! polled API instead.
//
//*****************************************************************************
extern void am_devices_l3gd20h_sample_read(
                                         am_devices_l3gd20h_t *psDevice,
                                         uint32_t *pui32Data,
                                         am_hal_iom_callback_t pfnCallback);

//*****************************************************************************
//
//! @brief Get the state of the control registers from the L3GD20H Gyro.
//!
//! @param psDevice     - Pointer to a device structure describing the L3GD20H
//! @param pui32Return  - Buffer where the control register values will be
//! written.
//!
//! Gets the state of the control registers from the L3GD20H Gyro and returns
//! the 5 bytes.
//!
//! @note The register values will be retrieved in a byte-packed format. The
//! caller must ensure that the memory location references by the
//! \e pui32Return pointer has enough space for all 5 bytes.
//
//*****************************************************************************
extern void am_devices_l3gd20h_ctrl_reg_state_get(
                                         am_devices_l3gd20h_t *psDevice,
                                         uint32_t *pui32Return);

//*****************************************************************************
//
//! @brief Initialize the L3GD20H
//!
//! @param psDevice  - Pointer to a device structure describing the L3GD20H
//!
//! This function sends a few commands to initialize the GYRO for normal usage
//! depending on the number of sample to be collected.  If samples is greater
//! than one, the GYRO is configured to use the FIFO in data-stream mode where
//! the interrupts occurs on the threshold level. If only one sample is
//! requested, the GYRO is configured to use the interrupt as a data-ready
//! line.
//!
//! @note This can be called any time to reinitialize the device.
//
//*****************************************************************************
extern void am_devices_l3gd20h_config(am_devices_l3gd20h_t *psDevice);

//*****************************************************************************
//
//! @brief Get the device ID
//!
//! @param psDevice - Pointer to a device structure describing the L3GD20H
//!
//! This function reads the device ID register and returns the result. The
//! L3GD20H should return 0xD7 when functioning correctly.
//!
//! @return ID value.
//
//*****************************************************************************
extern uint8_t am_devices_l3gd20h_device_id_get(
                                         am_devices_l3gd20h_t *psDevice);

//*****************************************************************************
//
//! @brief Reset the L3GD20H gyro
//!
//! @param psDevice  - Pointer to a device structure describing the L3GD20H
//!
//! This function resets the L3GD20H gryo.
//
//*****************************************************************************
extern void am_devices_l3gd20h_reset(am_devices_l3gd20h_t *psDevice);

#ifdef __cplusplus
}
#endif

#endif // AM_DEVICES_L3GD20H_H

//*****************************************************************************
//
// End Doxygen group.
//! @}
//
//*****************************************************************************

