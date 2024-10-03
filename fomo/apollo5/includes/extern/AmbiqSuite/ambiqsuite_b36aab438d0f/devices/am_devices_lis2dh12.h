//*****************************************************************************
//
//! @file am_devices_lis2dh12.h
//!
//! @brief Driver to interface with the LIS2DH12
//!
//! These functions implement the LIS2DH12 support routines for use on Ambiq
//! Micro MCUs.
//!
//! @addtogroup lis2dh12 SPI Device Control for the LIS2DH12 External Accelerometer
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

#ifndef AM_DEVICES_LIS2DH12_H
#define AM_DEVICES_LIS2DH12_H

#ifdef __cplusplus
extern "C"
{
#endif

//*****************************************************************************
//
//! @name LIS2DH12 Registers.
//! @{
//
//*****************************************************************************
#define AM_DEVICES_LIS2DH12_MAX_SAMPLE_SIZE   32
#define AM_DEVICES_LIS2DH12_MAX_SAMPLE_BYTES  192
//! @}

//*****************************************************************************
//
//! @name LIS2DH12 Registers.
//! @{
//
//*****************************************************************************
#define AM_DEVICES_LIS2DH12_STATUS_REG_AUX    0x07
#define AM_DEVICES_LIS2DH12_OUT_TEMP_L        0x0C
#define AM_DEVICES_LIS2DH12_OUT_TEMP_H        0x0D
#define AM_DEVICES_LIS2DH12_INT_COUNTER_REG   0x0E
#define AM_DEVICES_LIS2DH12_WHO_AM_I          0x0F
#define AM_DEVICES_LIS2DH12_CTRL_REG1         0x20
#define AM_DEVICES_LIS2DH12_CTRL_REG2         0x21
#define AM_DEVICES_LIS2DH12_CTRL_REG3         0x22
#define AM_DEVICES_LIS2DH12_CTRL_REG4         0x23
#define AM_DEVICES_LIS2DH12_CTRL_REG5         0x24
#define AM_DEVICES_LIS2DH12_CTRL_REG6         0x25
#define AM_DEVICES_LIS2DH12_CTRL_REG6         0x25
#define AM_DEVICES_LIS2DH12_REF_DCAPTURE      0x26
#define AM_DEVICES_LIS2DH12_STATUS_REG        0x27
#define AM_DEVICES_LIS2DH12_OUT_X_L           0x28
#define AM_DEVICES_LIS2DH12_OUT_X_H           0x29
#define AM_DEVICES_LIS2DH12_OUT_Y_L           0x2A
#define AM_DEVICES_LIS2DH12_OUT_Y_H           0x2B
#define AM_DEVICES_LIS2DH12_OUT_Z_L           0x2C
#define AM_DEVICES_LIS2DH12_OUT_Z_H           0x2D
#define AM_DEVICES_LIS2DH12_FIFO_CTRL_REG     0x2E
#define AM_DEVICES_LIS2DH12_SRC_REG           0x2F
#define AM_DEVICES_LIS2DH12_INT1_CFG          0x30
#define AM_DEVICES_LIS2DH12_INT1_SRC          0x31
#define AM_DEVICES_LIS2DH12_INT1_THS          0x32
#define AM_DEVICES_LIS2DH12_INT1_DURATION     0x33
#define AM_DEVICES_LIS2DH12_INT2_CFG          0x34
#define AM_DEVICES_LIS2DH12_INT2_SRC          0x35
#define AM_DEVICES_LIS2DH12_INT2_THS          0x36
#define AM_DEVICES_LIS2DH12_INT2_DURATION     0x37
#define AM_DEVICES_LIS2DH12_CLICK_CFG         0x38
#define AM_DEVICES_LIS2DH12_CLICK_SRC         0x39
#define AM_DEVICES_LIS2DH12_CLICK_THS         0x3A
#define AM_DEVICES_LIS2DH12_TIME_LIMIT        0x3B
#define AM_DEVICES_LIS2DH12_TIME_LATENCY      0x3C
#define AM_DEVICES_LIS2DH12_TIME_WINDOW       0x3D
#define AM_DEVICES_LIS2DH12_ACT_THS           0x3E
#define AM_DEVICES_LIS2DH12_ACT_DUR           0x3F
//! @}

//*****************************************************************************
//
//! Structure for holding information about the LIS2DH12
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
am_devices_lis2dh12_t;

//*****************************************************************************
//
//! Buffer type to make reading lis2dh12 samples easier.
//
//*****************************************************************************
#define am_devices_lis2dh12_sample(n)                                         \
    union                                                                     \
    {                                                                         \
        uint32_t words[((3 * n) + 1) >> 1];                                   \
        int16_t samples[3 * n];                                               \
    }

//*****************************************************************************
//
//! Useful types for handling data transfers to and from the LIS2DH12
//
//*****************************************************************************
#define am_hal_lis2dh12_regs(n)              am_hal_iom_buffer(n)

//*****************************************************************************
//
// External function definitions.
//
//*****************************************************************************

//*****************************************************************************
//
//! @brief Configures the LIS2DH12 for operation.
//!
//! @param psDevice - Pointer to a device structure describing the LIS2DH12.
//!
//! This function performs a basic, default configuration for the LIS2DH12.
//
//*****************************************************************************
extern void am_devices_lis2dh12_config(am_devices_lis2dh12_t *psDevice);

//*****************************************************************************
//
//! @brief Retrieves the most recent sample from the LIS2DH12.
//!
//! @param psDevice - Pointer to a device structure describing the LIS2DH12.
//! @param psData is the location where this function will place the data.
//! @param pfnCallback - Optional callback function.
//!
//! This function reads the Magnetometer sample registers in the LIS2DH12, and
//! places the resulting samples into the caller-supplied buffer. If a callback
//! function is supplied, this function will use the am_hal_iom_spi_write_nb()
//! call to perform the transfer, and the caller's callback function will be
//! called upon completion.
//!
//! @note This function will write exactly 8 bytes of data to the location
//! pointed to by psData. The caller must make sure that psData is large enough
//! to hold this data.
//
//*****************************************************************************
extern void am_devices_lis2dh12_sample_get(am_devices_lis2dh12_t *psDevice,
                                          uint32_t *psData,
                                          am_hal_iom_callback_t pfnCallback);

//*****************************************************************************
//
//! @brief Reads an internal register in the LIS2DH12.
//!
//! @param psDevice - Pointer to a device structure describing the LIS2DH12.
//! @param ui8Register - The Address of the register to read.
//!
//! This function performs a read to an LIS2DH12 register over the serial bus.
//!
//! @return The data read
//
//*****************************************************************************
extern uint8_t am_devices_lis2dh12_reg_read(am_devices_lis2dh12_t *psDevice,
                                           uint8_t ui8Register);

//*****************************************************************************
//
//! @brief Reads a block of internal registers in the LIS2DH12.
//!
//! @param psDevice - Pointer to a device structure describing the LIS2DH12.
//! @param ui8StartRegister  - The Address of the first register to read.
//! @param pui32Values       - Byte-packed array where the read data will go.
//! @param ui32NumBytes      - Total number of 8-bit registers to read.
//! @param pfnCallback       - Optional callback function pointer.
//!
//! This function performs a read to a block of LIS2DH12 registers over the
//! serial bus. If the \e pfnCallback parameter is nonzero, this function will
//! use the am_hal_iom_spi_read_nb() function as the underlying interface, and
//! \e pfnCallback will be provided to the HAL as the IOM callback function.
//! Otherwise, the spi read will be polled.
//
//*****************************************************************************
extern void am_devices_lis2dh12_reg_block_read(am_devices_lis2dh12_t *psDevice,
                              uint8_t ui8StartRegister,
                              uint32_t *pui32Values,
                              uint32_t ui32NumBytes,
                              am_hal_iom_callback_t pfnCallback);

//*****************************************************************************
//
//! @brief Writes an internal register in the LIS2DH12.
//!
//! @param psDevice - Pointer to a device structure describing the LIS2DH12.
//! @param ui8Register - The Address of the register to write.
//! @param ui8Value    - The value to write to the register.
//!
//! This function performs a write to an LIS2DH12 register over the serial bus.
//
//*****************************************************************************
extern void am_devices_lis2dh12_reg_write(am_devices_lis2dh12_t *psDevice,
                                         uint8_t ui8Register, uint8_t ui8Value);

//*****************************************************************************
//
//! @brief Writes a block of internal registers in the LIS2DH12.
//!
//! @param psDevice - Pointer to a device structure describing the LIS2DH12.
//! @param ui8StartRegister - The Address of the first register to write.
//! @param pui32Values - Byte-packed array of data to write.
//! @param ui32NumBytes - Total number of registers to write.
//! @param pfnCallback - Optional callback function pointer.
//!
//! This function performs a write to a block of LIS2DH12 registers over the
//! serial bus. If the \e pfnCallback parameter is nonzero, this function will
//! use the am_hal_iom_spi_write_nb() function as the underlying interface, and
//! \e pfnCallback will be provided to the HAL as the IOM callback function.
//! Otherwise, the spi write will be polled.
//
//*****************************************************************************
extern void am_devices_lis2dh12_reg_block_write(
                                         am_devices_lis2dh12_t *psDevice,
                                         uint8_t ui8StartRegister,
                                         uint32_t *pui32Values,
                                         uint32_t ui32NumBytes,
                                         am_hal_iom_callback_t pfnCallback);

//*****************************************************************************
//
//! @brief Get the device ID
//!
//! @param psDevice is a pointer to a device structure describing the LIS2DH12
//!
//! This function reads the device ID register and returns the result. The
//! LIS2DH12 should return 0x33 when functioning correctly.
//!
//! @return ID value.
//
//*****************************************************************************
extern uint8_t am_devices_lis2dh12_device_id_get(
                                         am_devices_lis2dh12_t *psDevice);

#ifdef __cplusplus
}
#endif

#endif // AM_DEVICES_LIS2DH12_H

//*****************************************************************************
//
// End Doxygen group.
//! @}
//
//*****************************************************************************

