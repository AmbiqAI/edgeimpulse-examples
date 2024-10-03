//*****************************************************************************
//
//! @file am_devices_adxl364.h
//!
//! @brief Driver to interface with the ADXL364
//!
//! These functions implement the ADXL364 support routines for use on Ambiq
//! Micro MCUs.
//!
//! @addtogroup adxl364 ADXL364 SPI Driver
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

#ifndef AM_DEVICES_ADXL364_H
#define AM_DEVICES_ADXL364_H

#ifdef __cplusplus
extern "C"
{
#endif

//*****************************************************************************
//
// Defines
//
//*****************************************************************************
#define AM_DEVICES_ADXL_IS_X_AXIS(sample)   ((sample&0x000000001)!=0)

//*****************************************************************************
//
//! Function pointer for SPI write
//
//*****************************************************************************
typedef bool (*am_devices_adxl364_write_t)(uint32_t ui32Module,
                                           uint32_t ui32ChipSelect,
                                           uint32_t *pui32Data,
                                           uint32_t ui32NumBytes,
                                           uint32_t ui32Options);

//*****************************************************************************
//
//! Function pointer for SPI read.
//
//*****************************************************************************
typedef bool (*am_devices_adxl364_read_t)(uint32_t ui32Module,
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
    //
    //! Module number to use for IOM access.
    //
    uint32_t ui32IOMModule;

    //
    //! Chip Select number to use for IOM access.
    //
    uint32_t ui32ChipSelect;
}
am_devices_adxl364_t;

//*****************************************************************************
//
// External function definitions
//
//*****************************************************************************

//*****************************************************************************
//
//! @brief Initialize the ADXL364 driver.
//!
//! @param psIOMSettings - pointer to am_hal_iom_spi_device_t struct
//! @param pfnWriteFunc  - pointer to write function
//! @param pfnReadFunc   - pointer to read function
//!
//! This function sets up the interface between the IO master and the ADXL.
//! @par
//! @details communicate with the ADXL over the IO master SPI bus.
//
//*****************************************************************************

extern void am_devices_adxl364_driver_init(am_hal_iom_spi_device_t *psIOMSettings,
                                           am_devices_adxl364_write_t pfnWriteFunc,
                                           am_devices_adxl364_read_t pfnReadFunc);

//*****************************************************************************
//
//! @brief Initialize the ADXL364.
//!
//! @details This function sends a few commands to initialize the ADXL for normal usage.
//!     - This stops short of setting measurement mode.
//!     - The sample rate is set to 400 Samples/second.
//!     - The FIFO high water mark is set at 210 samples or 420 bytes.
//!
//! @note You can recall this any time to reinitialize the device.
//!
//! @param bSyncMode true --> use ctimer to trigger samples
//! @param ui32ClockFreqHz CPU core clock in Hertz
//!
//! @return Zero if successful.  (always returns zero)
//
//*****************************************************************************

extern int  am_devices_adxl364_init(bool bSyncMode, uint32_t ui32ClockFreqHz);

//*****************************************************************************
//
//! @brief Reset the ADXL364
//!
//! Sends a reset command to the ADXL364 over SPI.
//
//*****************************************************************************
extern void am_devices_adxl364_reset(void);

//*****************************************************************************
//
//! @brief Set measurement mode for the ADXL364
//!
//! @return Zero if successful.
//
//*****************************************************************************
extern int  am_devices_adxl364_measurement_mode_set(void);

//*****************************************************************************
//
//! @brief Get FIFO depth for the ADXL364
//!
//! @param p        - pointer to buffer where data is saved
//!                      - allocate at least 4 bytes, lower two bytes used
//!
//! Called from am_devices_3dof  task
//!
//! @return Zero if successful.
//
//*****************************************************************************
extern int  am_devices_adxl364_fifo_depth_get(uint32_t * p);
//*****************************************************************************
//
//! @brief Get a sample from the ADXL364
//!
//! @param Number   - Number of samples
//!     -- For now, Number of samples has to be less than or equal to 32
//! @param p        - pointer to buffer where data is saved
//!
//! @return Zero if successful.  (always returns zero)
//
//*****************************************************************************
extern int  am_devices_adxl364_sample_get(int Number, uint32_t * p);

//*****************************************************************************
//
//! @brief Triggers an IOM command to read a set of samples from the ADXL
//!
//! @param Number
//!
//! @details This function will start the SPI transfer, but doesn't wait for the
//! transfer to complete. Instead, the data will be transferred to the IOM
//! fifo, where they can be retrieved later. The IOM will issue a "fifo
//! threshold" interrupt when the fifo grows beyond the configured size, at
//! which point the caller will need to read the accumulated data into a
//! buffer. After the last byte is transferred, the IOM will issue a "command
//! complete" interrupt, and the bus may be used for other transfers.
//!
//! @return Zero if successful.
//
//*****************************************************************************
extern int  am_devices_adxl364_sample_get_nonblocking(int Number);

//*****************************************************************************
//
//! @brief Get the state of the control registers from  the ADXL364
//!
//! @param p - pointer return buffer need to be at least 21 bytes long
//!     - returns 21 bytes starting at 0x19 into p
//!
//! @details Called from am_devices_3dof  task
//!
//! @return Zero if successful.
//
//*****************************************************************************
extern int  am_devices_adxl364_ctrl_reg_state_get(uint32_t * p);

//*****************************************************************************
//
//! @brief Get the device ID
//!
//! @param p - pointer return buffer need to be at least 4 bytes long
//!          - returns 4 byte device id into p
//!
//! @details Called from am_devices_3dof  task
//
//*****************************************************************************
extern void am_devices_adxl364_device_id_get(uint32_t * p);

#ifdef __cplusplus
}
#endif

#endif // AM_DEVICES_ADXL364_H

//*****************************************************************************
//
// End Doxygen group.
//! @}
//
//*****************************************************************************

