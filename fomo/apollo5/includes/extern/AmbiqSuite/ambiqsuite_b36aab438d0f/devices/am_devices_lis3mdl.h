//*****************************************************************************
//
//! @file am_devices_lis3mdl.h
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

#ifndef AM_DEVICES_LIS3MDL_H
#define AM_DEVICES_LIS3MDL_H

#ifdef __cplusplus
extern "C"
{
#endif

//*****************************************************************************
//
//! @name LIS3MDL Registers.
//! @{
//
//*****************************************************************************
#define AM_DEVICES_LIS3MDL_WHO_AM_I         0x0F
#define AM_DEVICES_LIS3MDL_CTRL_REG1        0x20
#define AM_DEVICES_LIS3MDL_CTRL_REG2        0x21
#define AM_DEVICES_LIS3MDL_CTRL_REG3        0x22
#define AM_DEVICES_LIS3MDL_CTRL_REG4        0x23
#define AM_DEVICES_LIS3MDL_CTRL_REG5        0x24
#define AM_DEVICES_LIS3MDL_STATUS_REG       0x27
#define AM_DEVICES_LIS3MDL_OUT_X_L          0x28
#define AM_DEVICES_LIS3MDL_OUT_X_H          0x29
#define AM_DEVICES_LIS3MDL_OUT_Y_L          0x2A
#define AM_DEVICES_LIS3MDL_OUT_Y_H          0x2B
#define AM_DEVICES_LIS3MDL_OUT_Z_L          0x2C
#define AM_DEVICES_LIS3MDL_OUT_Z_H          0x2D
#define AM_DEVICES_LIS3MDL_TEMP_OUT_L       0x2E
#define AM_DEVICES_LIS3MDL_TEMP_OUT_H       0x2F
#define AM_DEVICES_LIS3MDL_INT_CFG          0x30
#define AM_DEVICES_LIS3MDL_INT_SRC          0x31
#define AM_DEVICES_LIS3MDL_INT_THS_L        0x32
#define AM_DEVICES_LIS3MDL_INT_THS_H        0x33

//! @}

//*****************************************************************************
//
//! Structure for holding information about the LIS3MDL
//
//*****************************************************************************
typedef struct
{
    uint32_t ui32IOMModule;
    uint32_t ui32ChipSelect;
}
am_devices_lis3mdl_t;

//*****************************************************************************
//
//! Buffer type to make reading lis3mdl samples easier.
//
//*****************************************************************************
#define am_devices_lis3mdl_sample(n)                                          \
    union                                                                     \
    {                                                                         \
        uint32_t words[((3 * n) + 1) >> 1];                                   \
        int16_t samples[3 * n];                                               \
    }

//*****************************************************************************
//
//! Useful types for handling data transfers to and from the LIS3MDL
//
//*****************************************************************************
#define am_hal_lis3mdl_regs(n)              am_hal_iom_buffer(n)

//*****************************************************************************
//
// External function definitions.
//
//*****************************************************************************

//*****************************************************************************
//
//! @brief Configures the LIS3MDL for operation.
//!
//! @param psDevice - Pointer to a device structure describing the LIS3MDL.
//!
//! This function performs a basic, default configuration for the LIS3MDL.
//
//*****************************************************************************
extern void am_devices_lis3mdl_config(const am_devices_lis3mdl_t *psDevice);

//*****************************************************************************
//
//! @brief Trigger a single conversion mode
//!
//! @param psDevice - Pointer to a device structure describing the LIS3MDL.
//!
//! This function triggers a single-conversion mode read at the current
//! configuration  Sampling frequency must be between 0.625 and 80 hz
//
//*****************************************************************************
extern void am_devices_lis3mdl_trigger(const am_devices_lis3mdl_t *psDevice);

//*****************************************************************************
//
//! @brief Retrieves the most recent sample from the LIS3MDL.
//!
//! @param psDevice - Pointer to a device structure describing the LIS3MDL.
//! @param psData   - The location where this function will place the data.
//! @param pfnCallback - Optional callback function.
//!
//! This function reads the Magnetometer sample registers in the LIS3MDL, and
//! places the resulting samples into the caller-supplied buffer. If a callback
//! function is supplied, this function will use the am_hal_iom_queue_spi_read()
//! call to perform the transfer, and the caller's callback function will be
//! called upon completion.
//!
//! @note This function will write exactly 8 bytes of data to the location
//! pointed to by psData. The caller must make sure that psData is large enough
//! to hold this data.
//
//*****************************************************************************
extern void am_devices_lis3mdl_sample_get(
                                         const am_devices_lis3mdl_t *psDevice,
                                         uint32_t *psData,
                                         am_hal_iom_callback_t pfnCallback);

//*****************************************************************************
//
//! @brief Reads an internal register in the LIS3MDL.
//!
//! @param psDevice - Pointer to a device structure describing the LIS3MDL.
//! @param ui8Register - Address of the register to read.
//!
//! This function performs a read to an LIS3MDL register over the serial bus.
//!
//! @return The register data that was read
//
//*****************************************************************************
extern uint8_t am_devices_lis3mdl_reg_read(const am_devices_lis3mdl_t *psDevice,
                                         uint8_t ui8Register);

//*****************************************************************************
//
//! @brief Reads a block of internal registers in the LIS3MDL.
//!
//! @param psDevice - Pointer to a device structure describing the LIS3MDL.
//! @param ui8StartRegister - Address of the first register to read.
//! @param pui32Values - Byte-packed array where the read data will go.
//! @param ui32NumBytes - The total number of 8-bit registers to read.
//! @param pfnCallback - Optional callback function pointer.
//!
//! This function performs a read to a block of LIS3MDL registers over the
//! serial bus. If the \e pfnCallback parameter is nonzero, this function will
//! use the am_hal_iom_queue_spi_read() function as the underlying interface, and
//! \e pfnCallback will be provided to the HAL as the IOM callback function.
//! Otherwise, the SPI read will be polled.
//
//*****************************************************************************
extern void am_devices_lis3mdl_reg_block_read(
                                         const am_devices_lis3mdl_t *psDevice,
                                         uint8_t ui8StartRegister,
                                         uint32_t *pui32Values,
                                         uint32_t ui32NumBytes,
                                         am_hal_iom_callback_t pfnCallback);

//*****************************************************************************
//
//! @brief Writes an internal register in the LIS3MDL.
//!
//! @param psDevice - Pointer to a device structure describing the LIS3MDL.
//! @param ui8Register - Address of the register to write.
//! @param ui8Value - The value to write to the register.
//!
//! This function performs a write to an LIS3MDL register over the serial bus.
//
//*****************************************************************************
extern void am_devices_lis3mdl_reg_write(const am_devices_lis3mdl_t *psDevice,
                                         uint8_t ui8Register,
                                         uint8_t ui8Value);

//*****************************************************************************
//
//! @brief Writes a block of internal registers in the LIS3MDL.
//!
//! @param psDevice - Pointer to a device structure describing the LIS3MDL.
//! @param ui8StartRegister - Address of the first register to write.
//! @param pui32Values - Byte-packed array of data to write.
//! @param ui32NumBytes - The total number of registers to write.
//! @param pfnCallback - Optional callback function pointer.
//!
//! This function performs a write to a block of LIS3MDL registers over the
//! serial bus. If the \e pfnCallback parameter is nonzero, this function will
//! use the am_hal_iom_queue_spi_write() function as the underlying interface, and
//! \e pfnCallback will be provided to the HAL as the IOM callback function.
//! Otherwise, the spi write will be polled.
//
//*****************************************************************************
extern void am_devices_lis3mdl_reg_block_write(
                                         const am_devices_lis3mdl_t *psDevice,
                                         uint8_t ui8StartRegister,
                                         uint32_t *pui32Values,
                                         uint32_t ui32NumBytes,
                                         am_hal_iom_callback_t pfnCallback);

//*****************************************************************************
//
//! @brief Get the device ID
//!
//! @param psDevice - Pointer to a device structure describing the LIS3MDL
//!
//! This function reads the device ID register and returns the result. The
//! LIS3MDL should return 0x3D when functioning correctly.
//
//*****************************************************************************
extern uint8_t am_devices_lis3mdl_device_id_get(
                                         const am_devices_lis3mdl_t *psDevice);

//*****************************************************************************
//
//! @brief Reset the LIS3MDL mag
//!
//! @param psDevice - Pointer to a device structure describing the LIS3MDL
//!
//! This function resets the LIS3MDL gryo.
//
//*****************************************************************************
extern void am_devices_lis3mdl_reset(const am_devices_lis3mdl_t *psDevice);

#ifdef __cplusplus
}
#endif

#endif // AM_DEVICES_LIS3MDL_H

//*****************************************************************************
//
// End Doxygen group.
//! @}
//
//*****************************************************************************

