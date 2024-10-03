//*****************************************************************************
//
//! @file am_devices_ak09911.h
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

#ifndef AM_DEVICES_AK09911_H
#define AM_DEVICES_AK09911_H

#ifdef __cplusplus
extern "C"
{
#endif

//*****************************************************************************
//
// AK09911 Registers.
//
//*****************************************************************************
#define AM_DEVICES_AK09911_WIA1             0x00
#define AM_DEVICES_AK09911_WIA2             0x01
#define AM_DEVICES_AK09911_INFO             0x02
#define AM_DEVICES_AK09911_ST1              0x10
#define AM_DEVICES_AK09911_OUT_X_L          0x11
#define AM_DEVICES_AK09911_OUT_X_H          0x12
#define AM_DEVICES_AK09911_OUT_Y_L          0x13
#define AM_DEVICES_AK09911_OUT_Y_H          0x14
#define AM_DEVICES_AK09911_OUT_Z_L          0x15
#define AM_DEVICES_AK09911_OUT_Z_H          0x16
#define AM_DEVICES_AK09911_CNTL0            0x17 // Dummy
#define AM_DEVICES_AK09911_ST2              0x18
#define AM_DEVICES_AK09911_CNTL1            0x30 //Dummy
#define AM_DEVICES_AK09911_CNTL2            0x31
#define AM_DEVICES_AK09911_CNTL3            0x32
#define AM_DEVICES_AK09911_TEST             0x33
#define AM_DEVICES_AK09911_ASAX             0x60
#define AM_DEVICES_AK09911_ASAY             0x61
#define AM_DEVICES_AK09911_ASAZ             0x62

//*****************************************************************************
//
//! Structure for holding information about the AK09911
//
//*****************************************************************************
typedef struct
{
    uint32_t ui32IOMModule;
    uint32_t ui32Address;
}
am_devices_ak09911_t;

//*****************************************************************************
//
//! Buffer type to make reading ak09911 samples easier.
//
//*****************************************************************************
#define am_devices_ak09911_sample(n)                                          \
    union                                                                     \
    {                                                                         \
        uint32_t words[((3 * n) + 1) >> 1];                                   \
        int16_t samples[3 * n];                                               \
    }

//*****************************************************************************
//
//! Useful type for handling data transfers to and from the AK09911
//
//*****************************************************************************
#define am_hal_ak09911_regs(n)              am_hal_iom_buffer(n)

//*****************************************************************************
//
// External function definitions.
//
//*****************************************************************************
//*****************************************************************************
//
//! @brief Configures the AK09911 for operation.
//!
//! @param psDevice  - pointer to a device structure describing the AK09911.
//!
//! This function performs a basic, default configuration for the AK09911.
//
//*****************************************************************************
extern void am_devices_ak09911_config(am_devices_ak09911_t *psDevice);

//*****************************************************************************
//
//! @brief Triggers the next sample from the AK09911
//!
//! @param psDevice - Pointer to a device structure describing the AK09911.
//
//*****************************************************************************
extern void am_devices_ak09911_sample_trigger(am_devices_ak09911_t *psDevice);

//*****************************************************************************
//
//! @brief Retrieves the most recent sample from the AK09911.
//!
//! @param psDevice is a pointer to a device structure describing the AK09911.
//! @param psData is the location where this function will place the data.
//! @param pfnCallback is an optional callback function.
//!
//! This function reads the Magnetometer sample registers in the AK09911, and
//! places the resulting samples into the caller-supplied buffer. If a callback
//! function is supplied, this function will use the am_hal_iom_i2c_write_nb()
//! call to perform the transfer, and the caller's callback function will be
//! called upon completion.
//!
//! @note This function will write exactly 8 bytes of data to the location
//! pointed to by psData. The caller must make sure that psData is large enough
//! to hold this data.
//
//*****************************************************************************
extern void am_devices_ak09911_sample_get(
                                         am_devices_ak09911_t *psDevice,
                                         uint32_t *psData,
                                         am_hal_iom_callback_t pfnCallback);

//*****************************************************************************
//
//! @brief Reads an internal register in the AK09911.
//!
//! @param psDevice       - Pointer to a device structure describing the AK09911.
//! @param ui8Register   - The address of the register to read.
//!
//! This function performs a read to an AK09911 register over the serial bus.
//!
//! @return  The value read from the register
//
//*****************************************************************************
extern uint8_t am_devices_ak09911_reg_read(am_devices_ak09911_t *psDevice,
                                         uint8_t ui8Register);

//*****************************************************************************
//
//! @brief Reads a block of internal registers in the AK09911.
//!
//! @param psDevice         - Pointer to a device structure describing the AK09911.
//! @param ui8StartRegister - The address of the first register to read.
//! @param pui32Values      - The byte-packed array where the read data will go.
//! @param ui32NumBytes     - The total number of 8-bit registers to read.
//! @param pfnCallback      - An optional callback function pointer.
//!
//! This function performs a read to a block of AK09911 registers over the
//! serial bus. If the \e pfnCallback parameter is nonzero, this function will
//! use the am_hal_iom_i2c_read_nb() function as the underlying interface, and
//! \e pfnCallback will be provided to the HAL as the IOM callback function.\n
//! Otherwise, the SPI read will be polled.
//
//*****************************************************************************
extern void am_devices_ak09911_reg_block_read(
                                         am_devices_ak09911_t *psDevice,
                                         uint8_t ui8StartRegister,
                                         uint32_t *pui32Values, uint32_t
                                         ui32NumBytes,
                                         am_hal_iom_callback_t
                                         pfnCallback);

//*****************************************************************************
//
//! @brief Writes an internal register in the AK09911.
//!
//! @param psDevice     - Pointer to a device structure describing the AK09911.
//! @param ui8Register  - The address of the register to write.
//! @param ui8Value    - The value to write to the register.
//!
//! This function performs a write to an AK09911 register over the serial bus.
//
//*****************************************************************************
extern void am_devices_ak09911_reg_write(am_devices_ak09911_t *psDevice,
                                         uint8_t ui8Register,
                                         uint8_t ui8Value);

//*****************************************************************************
//
//! @brief Writes a block of internal registers in the AK09911.
//!
//! @param psDevice          - Pointer to a device structure describing the AK09911.
//! @param ui8StartRegister  - The address of the first register to write.
//! @param pui32Values       - The byte-packed array of data to write.
//! @param ui32NumBytes      - The total number of registers to write.
//! @param pfnCallback       - An optional callback function pointer.
//!
//! This function performs a write to a block of AK09911 registers over the
//! serial bus. If the \e pfnCallback parameter is nonzero, this function will
//! use the am_hal_iom_i2c_write_nb() function as the underlying interface, and
//! \e pfnCallback will be provided to the HAL as the IOM callback function.\n
//! Otherwise, the spi write will be polled.
//
//*****************************************************************************
extern void am_devices_ak09911_reg_block_write(
                                         am_devices_ak09911_t *psDevice,
                                         uint8_t ui8StartRegister,
                                         uint32_t *pui32Values,
                                         uint32_t ui32NumBytes,
                                         am_hal_iom_callback_t pfnCallback);

//*****************************************************************************
//
//! @brief Get the device ID
//!
//! @param psDevice     = Pointer to a device structure describing the AK09911
//!
//! This function reads the device ID register and returns the result. The
//! AK09911 should return 0x3D when functioning correctly.
//!
//! @return ID value.
//
//*****************************************************************************
extern uint16_t am_devices_ak09911_device_id_get(am_devices_ak09911_t *psDevice);

//*****************************************************************************
//
//! @brief Reset the AK09911 mag
//!
//! @param psDevice -Pointer to a device structure describing the AK09911
//!
//! This function resets the AK09911 mag.
//
//*****************************************************************************
extern void am_devices_ak09911_reset(am_devices_ak09911_t *psDevice);

#ifdef __cplusplus
}
#endif

#endif // AM_DEVICES_AK09911_H

//*****************************************************************************
//
// End Doxygen group.
//! @}
//
//*****************************************************************************

