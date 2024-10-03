//*****************************************************************************
//
//! @file am_devices_adxl363.h
//!
//! @brief Driver to interface with the ADXL363
//!
//! These functions implement the ADXL363 support routines for use on Ambiq
//! Micro MCUs.
//!
//! @addtogroup adxl363 ADXL363 SPI Driver
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

#ifndef AM_DEVICES_ADXL363_H
#define AM_DEVICES_ADXL363_H

#ifdef __cplusplus
extern "C"
{
#endif

#define ADXL363_DEV_ID0 0xAD
#define ADXL363_DEV_ID1 0x1D
#define DEVICE_ID_SIZE 2

//*****************************************************************************
//
// Defines
//
//*****************************************************************************
#define AM_DEVICES_ADXL_IS_X_AXIS(sample)   ((sample & 0x00000001) != 0)
//
//!
//
typedef enum
{
    AM_DEVICES_ADXL363_400HZ,
    AM_DEVICES_ADXL363_200HZ,
    AM_DEVICES_ADXL363_100HZ,
}
eADXL363SampleRate;

//
//! Enum definitions for accelerometer sensitivity setting
//
typedef enum
{
    AM_DEVICES_ADXL363_2G,
    AM_DEVICES_ADXL363_4G,
    AM_DEVICES_ADXL363_8G,
}
eADXL363Range;

//
//! used to return accelerometer data from pressed data
//
typedef struct
{
    //
    //! this is in units of G(gravity) * 1000
    //!
    int32_t     i32vectorsGx1000[3];
    //
    //! when using xyz data, adter processing the three inputs this will be
    //! 0x00010101
    //
    uint32_t    ui32ErrorCode ;
}
ADXL363Vector_t;

//*****************************************************************************
//
//! Function pointer for SPI write.
//
//*****************************************************************************
typedef bool (*am_devices_adxl363_write_t)(uint32_t ui32Module,
                                           uint32_t ui32ChipSelect,
                                           uint32_t *pui32Data,
                                           uint32_t ui32NumBytes,
                                           uint32_t ui32Options);

//*****************************************************************************
//
//! Function pointer for SPI read.
//
//*****************************************************************************
typedef bool (*am_devices_adxl363_read_t)(uint32_t ui32Module,
                                          uint32_t ui32ChipSelect,
                                          uint32_t *pui32Data,
                                          uint32_t ui32NumBytes,
                                          uint32_t ui32Options);

//*****************************************************************************
//
//! Device structure used for communication.
//
//*****************************************************************************
typedef struct
{

    void *pHandle ;
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
    //! Sensor sample rate.\n
    //! Valid values are:
    //!    - AM_DEVICES_ADXL363_100HZ
    //!    - AM_DEVICES_ADXL363_200HZ
    //!    - AM_DEVICES_ADXL363_400HZ
    //
    eADXL363SampleRate ui32SampleRate;

    //
    //! Accelerometer measurement range.
    //! Valid values are:\n
    //!    - AM_DEVICES_ADXL363_2G
    //!    - AM_DEVICES_ADXL363_4G
    //!    - AM_DEVICES_ADXL363_8G
    //
    eADXL363Range eGForceRange;

    //
    //! Half bandwidth mode
    //
    bool bHalfBandwidth;

    //
    //! Sync mode, the micro supplies the sync on int2 pin
    //
    bool bSyncMode;

    //
    //! int line indicates data is ready
    //
    volatile bool dataReady ;
    //
    //! non blocking operation is in progress
    //
    volatile bool nonBlockBusy ;
    //
    //! fifo read is in progress
    //
    volatile bool readInProgress ;

    //
    //! when true, the callback has run
    //
    volatile bool adxlCallback ;
}
am_devices_adxl363_t;

//*****************************************************************************
//
//! macro to make sample/word conversion easier to deal with.
//
//*****************************************************************************
#define am_hal_iom_byte_buffer(n)                                             \
    union                                                                     \
    {                                                                         \
        uint64_t llwords[((n + 7) >> 3)];                                     \
        uint32_t words[((n + 3) >> 2)];                                       \
        uint8_t bytes[n];                                                     \
    }

//
//! return status enum for this module
//
typedef enum
{
    AM_DEVICES_ADXL363_SUCCESS,
    AM_DEVICES_ADXL363_ERROR

}
am_devices_adxl363_stauts_e ;

//*****************************************************************************
//
// !Function pointer used for callbacks.
//
//*****************************************************************************
typedef void (*am_devices_adxl363_callback_t)(void);

//*****************************************************************************
//
// External function definitions
//
//*****************************************************************************

//*****************************************************************************
//
//! @brief Initialize the ADXL363 driver
//!
//! This function sends will initialize the SPI driver.
//!
//! @param psDevice - Pointer to ADXL363 device structure
//!
//! @return status  - generic or interface specific status.
//
//*****************************************************************************
extern uint32_t am_devices_adxl363_init(am_devices_adxl363_t *psDevice) ;

//*****************************************************************************
//
//! @brief Initialize the ADXL363.
//!
//! This function sets the internal registers in the ADXL363
//!
//! @param psDevice - Pointer to ADXL363 device structure
//!
//! The more important control register values from device settings are set
//! in the am_devices_adxl63_t control structure
//!
//! @note You can recall this any time to reinitialize the device.
//! @par
//! @note Recommend to call device reset function before this
//! @par
//! @note Recommend to call device start function after this (to device)
//!
//! @return status      - generic or interface specific status.
//
//*****************************************************************************
extern uint32_t am_devices_adxl363_config(am_devices_adxl363_t *psDevice);

//*****************************************************************************
//
//! @brief Reset the ADXL363.
//!
//! @param psDevice     - Pointer to ADXL363 device structure
//!
//! Sends a reset command to the ADXL363 over SPI.
//!
//! @return status      - generic or interface specific status.
//
//*****************************************************************************
extern uint32_t am_devices_adxl363_reset(const am_devices_adxl363_t *psDevice);

//*****************************************************************************
//
//! @brief Set measurement mode for the ADXL363.
//!
//! @param psDevice - Pointer to ADXL363 device structure
//!
//! Send this command to start the ADXL363 to begin filling its fifo.
//!
//! @return status      - generic or interface specific status.
//
//*****************************************************************************
extern uint32_t am_devices_adxl363_measurement_mode_set(const am_devices_adxl363_t *psDevice);

//*****************************************************************************
//
//! @brief this function will place the ADXL363 in standby mode
//!
//! @param psDevice - Pointer to ADXL363 device structure
//!
//! This function will read the two byte adc conversion register
//! result.
//!
//! @return status      - generic or interface specific status.
//
//*****************************************************************************
extern uint32_t am_devices_adxl363_measurement_mode_standby(const am_devices_adxl363_t *psDevice) ;

//*****************************************************************************
//
//! @brief read adc registers from ADXL363
//!
//! @param psDevice - Pointer to ADXL363 device structure
//! @param pui32Return - Pointer for returned data
//!
//! This function will read the two byte adc conversion register
//! result.
//!
//! @return status      - generic or interface specific status.
//
//*****************************************************************************
extern uint32_t am_devices_adxl363_read_adc(const am_devices_adxl363_t *psDevice,
                                            uint32_t *pui32Return) ;

//*****************************************************************************
//
//! @brief Get FIFO depth for the ADXL363.
//!
//! @param psDevice - Pointer to ADXL363 device structure
//! @param pui32Return - Pointer to returned data
//!
//! This function issues a read FIFO depth command and blocks waiting on the
//! result.
//!
//! @return status      - generic or interface specific status.
//
//*****************************************************************************
extern uint32_t am_devices_adxl363_fifo_depth_get(const am_devices_adxl363_t *psDevice,
                                                  uint32_t *pui32Return);

//*****************************************************************************
//
//! @brief Get the state of the control registers from the ADXL363.
//!
//! @param psDevice - Pointer to ADXL363 device structure.
//! @param pui32Return - Pointer to buffer to hold register values.
//!
//! Get the state of the control registers from the ADXL363 and returns the 16
//! bytes. This function blocks.
//!
//! @return status      - generic or interface specific status.
//
//*****************************************************************************
extern uint32_t am_devices_adxl363_ctrl_reg_state_get(const am_devices_adxl363_t *psDevice,
                                                      uint32_t *pui32Return);

//*****************************************************************************
//
//! @brief Get the device ID.
//!
//! @param psDevice - Pointer to ADXL363 device structure
//! @param pui32Return - Pointer to buffer to receive device ID
//!
//! Gets the device ID and returns the 4 bytes. This function blocks.
//!
//! @return status      - generic or interface specific status.
//
//*****************************************************************************
extern uint32_t am_devices_adxl363_device_id_get(const am_devices_adxl363_t *psDevice,
                                             uint32_t *pui32Return);

//*****************************************************************************
//
//! @brief Write to ADXL62 control registers
//!
//! @param psDevice      - Pointer to ADXL363 device structure
//! @param pui8RegValues - Pointer to input array of command bytes
//! @param ui8StartReg   - Starting register in ADXL2363
//! @param ui32Number    - Number of bytes to write to ADXL363
//!
//! This function sends a number of bytes to the ADXL363 control registers.
//! One can use this blocking function to modify any of the ADXL363 registers
//! after they have been configured.
//!
//! @return status       - generic or interface specific status.
//
//*****************************************************************************
extern uint32_t am_devices_adxl363_reg_write(const am_devices_adxl363_t *psDevice,
                                         uint8_t *pui8RegValues,
                                         uint8_t ui8StartReg,
                                         uint32_t ui32Number);

//*****************************************************************************
//
//! @brief Read the state of one or more ADXL363 control registers
//!
//! Get the state of the control registers from the ADXL363
//!
//! @param psDevice - Pointer to ADXL363 device structure
//! @param pui8Return pointer to ui8 array to receive values
//! @param ui8StartReg register number (address) for first read
//! @param ui32Number number of registers to read must be >0 and <16
//!
//! Use this blocking function to read the state of one or more ADXL363 control
//! registers.
//!
//! @return status      - generic or interface specific status.
//
//*****************************************************************************
extern uint32_t am_devices_adxl363_ctrl_reg_read(
                                 const am_devices_adxl363_t *psDevice,
                                 uint8_t *pui8Return,
                                 uint8_t ui8StartReg,
                                 uint32_t ui32Number) ;

//*****************************************************************************
//
//! @brief Get a sample from the ADXL363
//!
//! @param psDevice - Pointer to ADXL363 device structure
//! @param ui32NumSamples - number of samples to read
//! @param pui32ReturnBuffer - pointer to sample buffer (ui32 aligned).
//! @param pfnCallback - pointer to function to call when the data returns
//! @param callbackCtxt - pointer to data the callback function can access
//!
//! This non blocking routine launches the FIFO read command via the IOM and
//! then returns. The function pointed to by pfnCallback is called when the
//! data has been written to pui32ReturnBuffer.
//!
//! If pfnCallback is NULL then polling is used and this function blocks.
//!
//! @return status      - generic or interface specific status.
//
//*****************************************************************************
extern uint32_t am_devices_adxl363_sample_get(const am_devices_adxl363_t *psDevice,
                                              uint32_t ui32NumSamples,
                                              uint32_t *pui32ReturnBuffer,
                                              am_hal_iom_callback_t pfnCallback,
                                              void *callbackCtxt);

//*****************************************************************************
//
//! @brief decode an array of 16bit fifo sample inputs from the ADXL363
//!
//! @param pui8InputBuff  buffer containing raw samples from ADXL363 fifo read
//! @param ui32InputBytes size of the sample in bytes
//! @param outNrmlVectors array of processed output vectors
//! @param eGforceRange   the g-force seeting used in the accelerometer
//!
//! @return number of failed samples for this call
//
//*****************************************************************************
extern uint32_t
am_devices_adxl363_extract_data(uint8_t *pui8InputBuff,
                                uint32_t ui32InputBytes,
                                ADXL363Vector_t *outNrmlVectors,
                                eADXL363Range eGforceRange);

#ifdef __cplusplus
}
#endif

#endif // AM_DEVICES_ADXL363_H

//*****************************************************************************
//
// End Doxygen group.
//! @}
//
//*****************************************************************************

