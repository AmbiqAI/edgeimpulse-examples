//*****************************************************************************
//
//! @file am_devices_bmi160.h
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

#ifndef AM_DEVICES_BMI160_H
#define AM_DEVICES_BMI160_H

#ifdef __cplusplus
extern "C"
{
#endif

//*****************************************************************************
//
//! Global to track the IOM state.
//
//*****************************************************************************
extern volatile bool g_bBMIComplete;

//*****************************************************************************
//
// BIT SLICE GET AND SET FUNCTIONS
//
//*****************************************************************************
#define AM_DEVICES_BMI160_GET_BITSLICE(regvar, bitname)\
        ((regvar & bitname##__MSK) >> bitname##__POS)

#define AM_DEVICES_BMI160_SET_BITSLICE(regvar, bitname, val)\
    (((regvar) & ~bitname##__MSK) |              \
     (((val) << bitname##__POS) & bitname##__MSK))

//*****************************************************************************
//
// BMI160 Registers.
//
//*****************************************************************************
#define AM_DEVICES_BMI160_CHIPID                    0x00
#define AM_DEVICES_BMI160_ERR_REG                   0x02
#define AM_DEVICES_BMI160_PMU_STATUS                0x03
#define AM_DEVICES_BMI160_DATA_MAG_X_L              0x04
#define AM_DEVICES_BMI160_DATA_MAG_X_H              0x05
#define AM_DEVICES_BMI160_DATA_MAG_Y_L              0x06
#define AM_DEVICES_BMI160_DATA_MAG_Y_H              0x07
#define AM_DEVICES_BMI160_DATA_MAG_Z_L              0x08
#define AM_DEVICES_BMI160_DATA_MAG_Z_H              0x09
#define AM_DEVICES_BMI160_DATA_RHALL_L              0x0A
#define AM_DEVICES_BMI160_DATA_RHALL_H              0x0B
#define AM_DEVICES_BMI160_DATA_GYR_X_L              0x0C
#define AM_DEVICES_BMI160_DATA_GYR_X_H              0x0D
#define AM_DEVICES_BMI160_DATA_GYR_Y_L              0x0E
#define AM_DEVICES_BMI160_DATA_GYR_Y_H              0x0F
#define AM_DEVICES_BMI160_DATA_GYR_Z_L              0x10
#define AM_DEVICES_BMI160_DATA_GYR_Z_H              0x11
#define AM_DEVICES_BMI160_DATA_ACC_X_L              0x12
#define AM_DEVICES_BMI160_DATA_ACC_X_H              0x13
#define AM_DEVICES_BMI160_DATA_ACC_Y_L              0x14
#define AM_DEVICES_BMI160_DATA_ACC_Y_H              0x15
#define AM_DEVICES_BMI160_DATA_ACC_Z_L              0x16
#define AM_DEVICES_BMI160_DATA_ACC_Z_H              0x17
#define AM_DEVICES_BMI160_SENSORTIME_0              0x18
#define AM_DEVICES_BMI160_SENSORTIME_1              0x19
#define AM_DEVICES_BMI160_SENSORTIME_2              0x1A
#define AM_DEVICES_BMI160_STATUS                    0x1B
#define AM_DEVICES_BMI160_INT_STATUS_0              0x1C
#define AM_DEVICES_BMI160_INT_STATUS_1              0x1D
#define AM_DEVICES_BMI160_INT_STATUS_2              0x1E
#define AM_DEVICES_BMI160_INT_STATUS_3              0x1F
#define AM_DEVICES_BMI160_TEMP_L                    0x20
#define AM_DEVICES_BMI160_TEMP_H                    0x21
#define AM_DEVICES_BMI160_FIFO_LEN_L                0x22
#define AM_DEVICES_BMI160_FIFO_LEN_H                0x23
#define AM_DEVICES_BMI160_FIFO_DATA                 0x24
#define AM_DEVICES_BMI160_ACC_CONF                  0x40
#define AM_DEVICES_BMI160_ACC_RANGE                 0x41
#define AM_DEVICES_BMI160_GYR_CONF                  0x42
#define AM_DEVICES_BMI160_GYR_RANGE                 0x43
#define AM_DEVICES_BMI160_MAG_CONF                  0x44
#define AM_DEVICES_BMI160_FIFO_DOWNS                0x45
#define AM_DEVICES_BMI160_FIFO_CFG_0                0x46
#define AM_DEVICES_BMI160_FIFO_CFG_1                0x47
#define AM_DEVICES_BMI160_MAG_IF_0                  0x4B
#define AM_DEVICES_BMI160_MAG_IF_1                  0x4C
#define AM_DEVICES_BMI160_MAG_IF_2                  0x4D
#define AM_DEVICES_BMI160_MAG_IF_3                  0x4E
#define AM_DEVICES_BMI160_MAG_IF_4                  0x4F
#define AM_DEVICES_BMI160_INT_EN_0                  0x50
#define AM_DEVICES_BMI160_INT_EN_1                  0x51
#define AM_DEVICES_BMI160_INT_EN_2                  0x52
#define AM_DEVICES_BMI160_INT_OUT_CTRL              0x53
#define AM_DEVICES_BMI160_INT_LATCH                 0x54
#define AM_DEVICES_BMI160_INT_MAP_0                 0x55
#define AM_DEVICES_BMI160_INT_MAP_1                 0x56
#define AM_DEVICES_BMI160_INT_MAP_2                 0x57
#define AM_DEVICES_BMI160_INT_DATA_0                0x58
#define AM_DEVICES_BMI160_INT_DATA_1                0x59
#define AM_DEVICES_BMI160_INT_LOWHIGH_0             0x5A
#define AM_DEVICES_BMI160_INT_LOWHIGH_1             0x5B
#define AM_DEVICES_BMI160_INT_LOWHIGH_2             0x5C
#define AM_DEVICES_BMI160_INT_LOWHIGH_3             0x5D
#define AM_DEVICES_BMI160_INT_LOWHIGH_4             0x5E
#define AM_DEVICES_BMI160_INT_MOTION_0              0x5F
#define AM_DEVICES_BMI160_INT_MOTION_1              0x60
#define AM_DEVICES_BMI160_INT_MOTION_2              0x61
#define AM_DEVICES_BMI160_INT_MOTION_3              0x62
#define AM_DEVICES_BMI160_INT_TAP_0                 0x63
#define AM_DEVICES_BMI160_INT_TAP_1                 0x64
#define AM_DEVICES_BMI160_INT_ORIENT_0              0x65
#define AM_DEVICES_BMI160_INT_ORIENT_1              0x66
#define AM_DEVICES_BMI160_INT_FLAT_0                0x67
#define AM_DEVICES_BMI160_INT_FLAT_1                0x68
#define AM_DEVICES_BMI160_FOC_CONF                  0x69
#define AM_DEVICES_BMI160_CONF                      0x6A
#define AM_DEVICES_BMI160_IF_CONF                   0x6B
#define AM_DEVICES_BMI160_PMU_TRIG                  0x6C
#define AM_DEVICES_BMI160_SELF_TEST                 0x6D
#define AM_DEVICES_BMI160_NV_CONF                   0x70
#define AM_DEVICES_BMI160_OFFSET_0                  0x71
#define AM_DEVICES_BMI160_OFFSET_1                  0x72
#define AM_DEVICES_BMI160_OFFSET_2                  0x73
#define AM_DEVICES_BMI160_OFFSET_3                  0x74
#define AM_DEVICES_BMI160_OFFSET_4                  0x75
#define AM_DEVICES_BMI160_OFFSET_5                  0x76
#define AM_DEVICES_BMI160_OFFSET_6                  0x77
#define AM_DEVICES_BMI160_STEP_CNT_0                0x78
#define AM_DEVICES_BMI160_STEP_CNT_1                0x79
#define AM_DEVICES_BMI160_STEP_CONF_0               0x7A
#define AM_DEVICES_BMI160_STEP_CONF_1               0x7B
#define AM_DEVICES_BMI160_CMD                       0x7E

//*****************************************************************************
//! HIGH_G THRESHOLD LENGTH, POSITION AND MASK
//*****************************************************************************
// Int_LowHigh_4 Description - Reg Addr --> 0x5e, Bit --> 0...7
#define AM_DEVICES_BMI160_USER_INTR_LOWHIGH_4_INTR_HIGH_THRES__POS          0
#define AM_DEVICES_BMI160_USER_INTR_LOWHIGH_4_INTR_HIGH_THRES__LEN          8
#define AM_DEVICES_BMI160_USER_INTR_LOWHIGH_4_INTR_HIGH_THRES__MSK          0xFF
#define AM_DEVICES_BMI160_USER_INTR_LOWHIGH_4_INTR_HIGH_THRES__REG          \
        AM_DEVICES_BMI160_INT_MOTION_0

//*****************************************************************************
// ANY MOTION DURATION LENGTH, POSITION AND MASK
//*****************************************************************************
// Int_Motion_0 Description - Reg Addr --> 0x5f, Bit --> 0...1
#define AM_DEVICES_BMI160_USER_INTR_MOTION_0_INTR_ANY_MOTION_DURN__POS      0
#define AM_DEVICES_BMI160_USER_INTR_MOTION_0_INTR_ANY_MOTION_DURN__LEN      2
#define AM_DEVICES_BMI160_USER_INTR_MOTION_0_INTR_ANY_MOTION_DURN__MSK      0x03
#define AM_DEVICES_BMI160_USER_INTR_MOTION_0_INTR_ANY_MOTION_DURN__REG      \
        AM_DEVICES_BMI160_INT_MOTION_0

//*****************************************************************************
// ANY MOTION THRESHOLD LENGTH, POSITION AND MASK
//*****************************************************************************
// Int_Motion_1 Description - Reg Addr --> 0x60, Bit --> 0...7
#define AM_DEVICES_BMI160_USER_INTR_MOTION_1_INTR_ANY_MOTION_THRES__POS     0
#define AM_DEVICES_BMI160_USER_INTR_MOTION_1_INTR_ANY_MOTION_THRES__LEN     8
#define AM_DEVICES_BMI160_USER_INTR_MOTION_1_INTR_ANY_MOTION_THRES__MSK     0xFF
#define AM_DEVICES_BMI160_USER_INTR_MOTION_1_INTR_ANY_MOTION_THRES__REG     \
        AM_DEVICES_BMI160_INT_MOTION_1

//*****************************************************************************
// INTERRUPT MAPPIONG OF ANY MOTION_G LENGTH, POSITION AND MASK
//*****************************************************************************
// Int_Map_0 Description - Reg Addr --> 0x55, Bit -->2
#define AM_DEVICES_BMI160_USER_INTR_MAP_0_INTR1_ANY_MOTION__POS             2
#define AM_DEVICES_BMI160_USER_INTR_MAP_0_INTR1_ANY_MOTION__LEN             1
#define AM_DEVICES_BMI160_USER_INTR_MAP_0_INTR1_ANY_MOTION__MSK             0x04
#define AM_DEVICES_BMI160_USER_INTR_MAP_0_INTR1_ANY_MOTION__REG             \
        AM_DEVICES_BMI160_INT_MAP_0

//*****************************************************************************
// DATA READY ENABLE LENGTH, POSITION AND MASK
//*****************************************************************************
// Int_En_1 Description - Reg Addr --> 0x51, Bit -->4
#define AM_DEVICES_BMI160_USER_INTR_ENABLE_1_DATA_RDY_ENABLE__POS           4
#define AM_DEVICES_BMI160_USER_INTR_ENABLE_1_DATA_RDY_ENABLE__LEN           1
#define AM_DEVICES_BMI160_USER_INTR_ENABLE_1_DATA_RDY_ENABLE__MSK           0x10
#define AM_DEVICES_BMI160_USER_INTR_ENABLE_1_DATA_RDY_ENABLE__REG           \
        AM_DEVICES_BMI160_INT_EN_1

//*****************************************************************************
// ANY MOTION XYZ AXIS ENABLE LENGTH, POSITION AND MASK
//*****************************************************************************
// Int_En_0 Description - Reg Addr --> 0x50, Bit -->0
#define AM_DEVICES_BMI160_USER_INTR_ENABLE_0_ANY_MOTION_X_ENABLE__POS       0
#define AM_DEVICES_BMI160_USER_INTR_ENABLE_0_ANY_MOTION_X_ENABLE__LEN       1
#define AM_DEVICES_BMI160_USER_INTR_ENABLE_0_ANY_MOTION_X_ENABLE__MSK       0x01
#define AM_DEVICES_BMI160_USER_INTR_ENABLE_0_ANY_MOTION_X_ENABLE__REG       \
        AM_DEVICES_BMI160_INT_EN_0

// Int_En_0 Description - Reg Addr --> 0x50, Bit -->1
#define AM_DEVICES_BMI160_USER_INTR_ENABLE_0_ANY_MOTION_Y_ENABLE__POS       1
#define AM_DEVICES_BMI160_USER_INTR_ENABLE_0_ANY_MOTION_Y_ENABLE__LEN       1
#define AM_DEVICES_BMI160_USER_INTR_ENABLE_0_ANY_MOTION_Y_ENABLE__MSK       0x02
#define AM_DEVICES_BMI160_USER_INTR_ENABLE_0_ANY_MOTION_Y_ENABLE__REG       \
        AM_DEVICES_BMI160_INT_EN_0

// Int_En_0 Description - Reg Addr --> 0x50, Bit -->2
#define AM_DEVICES_BMI160_USER_INTR_ENABLE_0_ANY_MOTION_Z_ENABLE__POS       2
#define AM_DEVICES_BMI160_USER_INTR_ENABLE_0_ANY_MOTION_Z_ENABLE__LEN       1
#define AM_DEVICES_BMI160_USER_INTR_ENABLE_0_ANY_MOTION_Z_ENABLE__MSK       0x04
#define AM_DEVICES_BMI160_USER_INTR_ENABLE_0_ANY_MOTION_Z_ENABLE__REG       \
        AM_DEVICES_BMI160_INT_EN_0

//*****************************************************************************
// ACCEL POWER MODE
//*****************************************************************************
#define AM_DEVICES_BMI160_ACCEL_MODE_NORMAL         0x11
#define AM_DEVICES_BMI160_ACCEL_MODE_LOWPOWER       0X12
#define AM_DEVICES_BMI160_ACCEL_MODE_SUSPEND        0X10
//*****************************************************************************
// GYRO POWER MODE
//*****************************************************************************
#define AM_DEVICES_BMI160_GYRO_MODE_SUSPEND         0x14
#define AM_DEVICES_BMI160_GYRO_MODE_NORMAL          0x15
#define AM_DEVICES_BMI160_GYRO_MODE_FASTSTARTUP     0x17
//*****************************************************************************
// MAG POWER MODE
//*****************************************************************************
#define AM_DEVICES_BMI160_MAG_MODE_SUSPEND          0x18
#define AM_DEVICES_BMI160_MAG_MODE_NORMAL           0x19
#define AM_DEVICES_BMI160_MAG_MODE_LOWPOWER         0x1A

//*****************************************************************************
//
//! @name ACCEL ODR
//!
//! @{
//
//*****************************************************************************
#define BMI160_ACCEL_OUTPUT_DATA_RATE_RESERVED       0x00
#define BMI160_ACCEL_OUTPUT_DATA_RATE_0_78HZ         0x01
#define BMI160_ACCEL_OUTPUT_DATA_RATE_1_56HZ         0x02
#define BMI160_ACCEL_OUTPUT_DATA_RATE_3_12HZ         0x03
#define BMI160_ACCEL_OUTPUT_DATA_RATE_6_25HZ         0x04
#define BMI160_ACCEL_OUTPUT_DATA_RATE_12_5HZ         0x05
#define BMI160_ACCEL_OUTPUT_DATA_RATE_25HZ           0x06
#define BMI160_ACCEL_OUTPUT_DATA_RATE_50HZ           0x07
#define BMI160_ACCEL_OUTPUT_DATA_RATE_100HZ          0x08
#define BMI160_ACCEL_OUTPUT_DATA_RATE_200HZ          0x09
#define BMI160_ACCEL_OUTPUT_DATA_RATE_400HZ          0x0A
#define BMI160_ACCEL_OUTPUT_DATA_RATE_800HZ          0x0B
#define BMI160_ACCEL_OUTPUT_DATA_RATE_1600HZ         0x0C
#define BMI160_ACCEL_OUTPUT_DATA_RATE_RESERVED0      0x0D
#define BMI160_ACCEL_OUTPUT_DATA_RATE_RESERVED1      0x0E
#define BMI160_ACCEL_OUTPUT_DATA_RATE_RESERVED2      0x0F
//! @}

//*****************************************************************************
//
//! @name GYRO ODR
//!
//! @{
//
//*****************************************************************************
#define BMI160_GYRO_OUTPUT_DATA_RATE_RESERVED       0x00
#define BMI160_GYRO_OUTPUT_DATA_RATE_25HZ           0x06
#define BMI160_GYRO_OUTPUT_DATA_RATE_50HZ           0x07
#define BMI160_GYRO_OUTPUT_DATA_RATE_100HZ          0x08
#define BMI160_GYRO_OUTPUT_DATA_RATE_200HZ          0x09
#define BMI160_GYRO_OUTPUT_DATA_RATE_400HZ          0x0A
#define BMI160_GYRO_OUTPUT_DATA_RATE_800HZ          0x0B
#define BMI160_GYRO_OUTPUT_DATA_RATE_1600HZ         0x0C
#define BMI160_GYRO_OUTPUT_DATA_RATE_3200HZ         0x0D
//! @}

//*****************************************************************************
//
// Device structure used for communication.
//
//*****************************************************************************

// defines to be used in am_devices_bmi160_t structure.
#define AM_DEVICES_BMI160_MODE_SPI                  true
#define AM_DEVICES_BMI160_MODE_I2C                  false

//
//!
//
typedef struct
{
    //
    //! SPI or I2C mode.
    //
    bool bMode;

    //
    //! Module number to use for IOM access.
    //
    uint32_t ui32IOMModule;

    //
    //! Chip Select number to use for IOM access. (unused in I2C mode).
    //
    uint32_t ui32ChipSelect;

    //
    //! Address for I2C communication. (unused in SPI mode)
    //
    uint32_t ui32Address;

    //
    //! Number of samples to collect before interrupt.
    //
    uint32_t ui32Samples;
}
am_devices_bmi160_t;

//*****************************************************************************
//
//! Function pointer used for callbacks.
//
//*****************************************************************************
typedef void (*am_devices_bmi160_callback_t)(void);

//*****************************************************************************
//
//! Typedef to make sample/word conversion easier to deal with.
//
//*****************************************************************************
#define bmi160_sample_buffer(n)                                               \
    union                                                                     \
    {                                                                         \
        uint32_t words[((6 * n) + 1) >> 1];                                   \
        int16_t samples[6 * n];                                               \
    }

//*****************************************************************************
//
// External function definitions
//
//*****************************************************************************

//*****************************************************************************
//
//! @brief Clear one or more bits.
//!
//! @param psDevice     - Pointer to a device structure describing the BMI160.
//! @param ui8Address   - Reg address.
//! @param ui8Mask      - Bits to clear.
//!
//! This function clears one or more bits in the selected register, selected by
//! 1's in the mask.
//
//*****************************************************************************
extern void am_devices_bmi160_reg_clear(const am_devices_bmi160_t *psDevice,
                                        uint8_t ui8Address, uint8_t ui8Mask);

//*****************************************************************************
//
//! @brief Set one or more bits.
//!
//! @param psDevice     - Pointer to a device structure describing the BMI160.
//! @param ui8Address   - RTC address.
//! @param ui8Mask      - Bits to set.
//!
//! This function sets one or more bits in the selected register, selected by
//! 1's in the mask.
//
//*****************************************************************************
extern void am_devices_bmi160_reg_set(const am_devices_bmi160_t *psDevice,
                                      uint8_t ui8Address, uint8_t ui8Mask);

//*****************************************************************************
//
//! @brief Reads an internal register in the BMI160.
//!
//! @param psDevice     - Pointer to a device structure describing the BMI160.
//! @param ui8Register - The address of the register to read.
//!
//! This function performs a read to an BMI160 register over the serial bus.
//!
//! @return retrieved byte
//
//*****************************************************************************
extern uint8_t am_devices_bmi160_reg_read(const am_devices_bmi160_t *psDevice,
                            uint8_t ui8Register);

//*****************************************************************************
//
//! @brief Reads a block of internal registers in the BMI160.
//!
//! @param psDevice          - Pointer to a device structure describing the BMI160.
//! @param ui8StartRegister - The address of the first register to read.
//! @param pui32Values       - The byte-packed array where the read data will go.
//! @param ui32NumBytes      - The total number of registers to read.
//! @param pfnCallback       - An optional callback function pointer.
//!
//! This function performs a read to a block of BMI160 registers over the
//! serial bus. If the \e pfnCallback parameter is nonzero, this function will
//! use the am_hal_iom_queue_spi_read() function as the underlying interface, and
//! \e pfnCallback will be provided to the HAL as the IOM callback function.\n
//! Otherwise, the SPI read will be polled.
//
//*****************************************************************************
extern void am_devices_bmi160_reg_block_read(const am_devices_bmi160_t *psDevice,
                                  uint8_t ui8StartRegister,
                                  uint32_t *pui32Values,
                                  uint32_t ui32NumBytes,
                                  am_hal_iom_callback_t pfnCallback);

//*****************************************************************************
//
//! @brief Writes an internal register in the BMI160.
//!
//! @param psDevice is a pointer to a device structure describing the BMI160.
//! @param ui8Register is the address of the register to write.
//! @param ui8Value is the value to write to the register.
//!
//! This function performs a write to an BMI160 register over the serial bus.
//
//*****************************************************************************
extern void am_devices_bmi160_reg_write(const am_devices_bmi160_t *psDevice,
                             uint8_t ui8Register, uint8_t ui8Value);

//*****************************************************************************
//
//! @brief Writes a block of internal registers in the BMI160.
//!
//! @param psDevice          - Pointer to a device structure describing the BMI160.
//! @param ui8StartRegister  - The address of the first register to write.
//! @param pui32Values       - The byte-packed array of data to write.
//! @param ui32NumBytes      - The total number of registers to write.
//! @param pfnCallback       - An optional callback function pointer.
//!
//! This function performs a write to a block of BMI160 registers over the
//! serial bus. If the \e pfnCallback parameter is nonzero, this function will
//! use the am_hal_iom_queue_spi_write() function as the underlying interface, and
//! \e pfnCallback will be provided to the HAL as the IOM callback function.\n
//! Otherwise, the SPI write will be polled.
//
//*****************************************************************************
extern void am_devices_bmi160_reg_block_write(const am_devices_bmi160_t *psDevice,
                                   uint8_t ui8StartRegister,
                                   uint32_t *pui32Values,
                                   uint32_t ui32NumBytes,
                                   am_hal_iom_callback_t pfnCallback);

//*****************************************************************************
//
//! @brief Gets the BMI160 samples.
//!
//! @param psDevice          - Pointer to a device structure describing the BMI160.
//! @param ui32NumSamples    - The number of samples to get.
//! @param pui32ReturnBuffer - Pointer to the buffer to store return data.
//! @param pfnCallback - Optional callback.
//!
//! This function gets the BMI160 samples.
//
//*****************************************************************************
extern void am_devices_bmi160_sample_get(const am_devices_bmi160_t *psDevice,
                                          uint32_t ui32NumSamples,
                                          uint32_t *pui32ReturnBuffer,
                                          am_devices_bmi160_callback_t pfnCallback);

//*****************************************************************************
//
//! @brief Soft Reset the BMI160
//!
//! @param psDevice - Pointer to a device structure describing the BMI160.
//!
//! This function forces a soft reset command to the BMI160.
//
//*****************************************************************************
extern void am_devices_bmi160_reset(const am_devices_bmi160_t *psDevice);

//*****************************************************************************
//
//! @brief Initializes the BMI160.
//!
//! @param psDevice - Pointer to a device structure describing the BMI160.
//!
//! This function initializes the BMI160.
//
//*****************************************************************************
extern void am_devices_bmi160_config(const am_devices_bmi160_t *psDevice);

//*****************************************************************************
//
//! @brief Get the device ID.
//!
//! @param psDevice - Pointer to BMI160 device structure
//!
//! Gets the device ID and returns the 4 bytes. This function blocks.
//!
//! @return Device ID byte.
//
//*****************************************************************************
extern uint8_t am_devices_bmi160_device_id_get(const am_devices_bmi160_t *psDevice);

//*****************************************************************************
//
//! @brief enable the BMI160.
//!
//! @param psDevice     - Pointer to a device structure describing the BMI160.
//! @param data_rate    - The output sample rate for accel and/or gyro.
//! @param enable_accel - Set to true to enable the accel.
//! @param enable_gyro  - Set to true to enable the gyro.
//!
//! This function initializes the BMI160.
//
//*****************************************************************************
extern void am_devices_bmi160_enable(const am_devices_bmi160_t *psDevice,
                                     int data_rate,
                                     bool enable_accel,
                                     bool enable_gyro);

//*****************************************************************************
//
//! @brief Enable/Disable an interrupt on any motion.
//!
//! @param psDevice - Pointer to BMI160 device structure
//! @param bEnable  - If true enable any motion interrupt else disable it.
//!
//! Enables an interrupt on any motion if bEnable is true, otherwise the
//! interrupt is disabled.
//
//*****************************************************************************
extern void am_devices_bmi160_enable_any_motion(const am_devices_bmi160_t *psDevice, bool bEnable);

#ifdef __cplusplus
}
#endif

#endif // AM_DEVICES_BMI160_H

//*****************************************************************************
//
// End Doxygen group.
//! @}
//
//*****************************************************************************

