//*****************************************************************************
//
//! @file am_devices_adxl362.h
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

#ifndef AM_DEVICES_ADXL362_H
#define AM_DEVICES_ADXL362_H

#ifdef __cplusplus
extern "C"
{
#endif

//*****************************************************************************
//
// Defines
//
//*****************************************************************************
#define AM_DEVICES_ADXL_IS_X_AXIS(sample)   ((sample & 0x00000001) != 0)

typedef enum
{
    AM_DEVICES_ADXL362_400HZ,
    AM_DEVICES_ADXL362_200HZ,
    AM_DEVICES_ADXL362_100HZ,
}eADXL362SampleRate;

typedef enum
{
    AM_DEVICES_ADXL362_2G,
    AM_DEVICES_ADXL362_4G,
    AM_DEVICES_ADXL362_8G,
}eADXL362Range;

//*****************************************************************************
//
// Device structure used for communication.
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
    //
    uint32_t ui32Samples;

    //
    //! Sensor sample rate.
    //! Valid values are:
    //!    - AM_DEVICES_ADXL362_100HZ
    //!    - AM_DEVICES_ADXL362_200HZ
    //!    - AM_DEVICES_ADXL362_400HZ
    //
    eADXL362SampleRate ui32SampleRate;

    //
    //! Accelerometer measurement range.
    //! Valid values are:
    //!    - AM_DEVICES_ADXL362_2G
    //!    - AM_DEVICES_ADXL362_4G
    //!    - AM_DEVICES_ADXL362_8G
    //
    eADXL362Range ui32Range;

    //
    //! Half bandwidth mode?
    //
    bool bHalfBandwidth;

    //
    //! Sync mode?
    //
    bool bSyncMode;
}
am_devices_adxl362_t;

//*****************************************************************************
//
// Typedef to make sample/word conversion easier to deal with.
//
//*****************************************************************************
#define am_devices_adxl362_sample(n)                                          \
    union                                                                     \
    {                                                                         \
        uint32_t words[((3 * n) + 1) >> 1];                                   \
        int16_t samples[3 * n];                                               \
    }

//*****************************************************************************
//
// Macro for retrieving the sign-extended magnitude of a 16-bit ADXL sample.
//
//*****************************************************************************
#define AM_DEVICES_ADXL362_VALUE(x)    (x ? (((int16_t) ((x) << 2)) / 4) : 0)

//*****************************************************************************
//
//! Function pointer used for callbacks.
//
//*****************************************************************************
typedef void (*am_devices_adxl362_callback_t)(void);

//*****************************************************************************
//
// External function definitions
//
//*****************************************************************************

//*****************************************************************************
//
//! @brief Initialize the ADXL362.
//!
//! This function sends a few commands to initialize the ADXL for normal usage.
//!
//! @param psDevice - Pointer to ADXL362 device structure
//!
//! We stop short of setting measurement mode.
//! We set the more important control register values from device settings
//! in the am_devices_adxl62_t control structure
//!
//! @note You can recall this any time to reinitialize the device.
//!
//
//*****************************************************************************
extern void am_devices_adxl362_config(const am_devices_adxl362_t *psDevice);

//*****************************************************************************
//
//! @brief Reset the ADXL362.
//!
//! @param psDevice - Pointer to ADXL362 device structure
//!
//! Sends a reset command to the ADXL362 over SPI.
//
//*****************************************************************************
extern void am_devices_adxl362_reset(const am_devices_adxl362_t *psDevice);

//*****************************************************************************
//
//! @brief Set measurement mode for the ADXL362.
//!
//! @param psDevice - Pointer to ADXL362 device structure
//!
//! Send this command when you are ready to start filling the ADXL362 FIFO.
//
//*****************************************************************************
extern void am_devices_adxl362_measurement_mode_set(const am_devices_adxl362_t *psDevice);

//*****************************************************************************
//
//! @brief Get FIFO depth for the ADXL362.
//!
//! @param psDevice     - Pointer to ADXL362 device structure
//! @param pui32Return  - Pointer to longword, will hold two byte value read
//!                       from IOM
//!
//! This function issues a read FIFO depth command and blocks waiting on the
//! result.
//
//*****************************************************************************
extern void am_devices_adxl362_fifo_depth_get(const am_devices_adxl362_t *psDevice,
                                              uint32_t *pui32Return);

//*****************************************************************************
//
//! @brief Get a sample from the ADXL362
//!
//! @param psDevice          - Pointer to ADXL362 device structure
//! @param ui32NumSamples    - Number of samples to read
//! @param pui32ReturnBuffer - Pointer to sample buffer (ui32 aligned).
//! @param pfnCallback       - Pointer to function to call when the data returns
//!
//! This non blocking routine launches the FIFO read command via the IOM and
//! then returns. The function pointed to by pfnCallback is called when the
//! data has been written to pui32ReturnBuffer.
//!
//! @note If pfnCallback is NULL then polling is used and this function blocks.
//!
//! @note Ensure pui32ReturnBuffer points to a buffer large enough to hold all
//! the returned data
//
//*****************************************************************************
extern void am_devices_adxl362_sample_get(const am_devices_adxl362_t *psDevice,
                                          uint32_t ui32NumSamples,
                                          uint32_t *pui32ReturnBuffer,
                                          am_devices_adxl362_callback_t pfnCallback);

//*****************************************************************************
//
//! @brief Get the state of the control registers from the ADXL362.
//!
//! @param psDevice     - Pointer to ADXL362 device structure.
//! @param pui32Return  - Pointer to buffer to hold register values.
//!
//! @note pui32Return must point to a buffer large enough to hold the returned
//!       data
//! @par
//! Get the state of the control registers from the ADXL362 and returns the 16
//! bytes.
//!
//! @note This function blocks.
//
//*****************************************************************************
extern void am_devices_adxl362_ctrl_reg_state_get(const am_devices_adxl362_t *psDevice,
                                                  uint32_t *pui32Return);

//*****************************************************************************
//
//! @brief Get the device ID.
//!
//! @param psDevice     - Pointer to ADXL362 device structure
//! @param pui32Return  - Pointer to buffer to receive device ID
//!
//! Gets the device ID and returns the 4 bytes.
//! @note This function blocks.
//
//*****************************************************************************
extern void am_devices_adxl362_device_id_get(const am_devices_adxl362_t *psDevice,
                                             uint32_t *pui32Return);

//*****************************************************************************
//
//! @brief Write to ADXL62 control registers
//!
//! @param psDevice      - Pointer to ADXL362 device structure
//! @param pui8RegValues - Pointer to input array of command bytes
//! @param ui8StartReg   - Starting register in ADXL2362
//! @param ui32Number    - Number of bytes to write to ADXL362
//!
//! This function sends a number of bytes to the ADXL362 control registers.
//! One can use this blocking function to modify any of the ADXL362 registers
//! after they have been configured.
//
//*****************************************************************************
extern void am_devices_adxl362_reg_write(const am_devices_adxl362_t *psDevice,
                                         uint8_t *pui8RegValues,
                                         uint8_t ui8StartReg,
                                         uint32_t ui32Number);

//*****************************************************************************
//
//! @brief Read the state of one or more ADXL362 control registers
//!
//! Get the state of the control registers from the ADXL362
//!
//! @param psDevice     - Pointer to ADXL362 device structure
//! @param pui8Return   - Pinter to ui8 array to receive values
//! @param ui8StartReg  - Register number (address) for first read
//! @param ui32Number   - Number of registers to read must be >0 and <16
//!
//! Use this blocking function to read the state of one or more ADXL362 control
//! registers.
//
//*****************************************************************************
extern void am_devices_adxl362_ctrl_reg_read(const am_devices_adxl362_t *psDevice,
                                             uint8_t *pui8Return,
                                             uint8_t ui8StartReg,
                                             uint32_t ui32Number);

#ifdef __cplusplus
}
#endif

#endif // AM_DEVICES_ADXL362_H

//*****************************************************************************
//
// End Doxygen group.
//! @}
//
//*****************************************************************************

