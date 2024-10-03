//*****************************************************************************
//
//! @file am_devices_amx8x5.h
//!
//! @brief Driver to interface with the AMx8x5 RTC.
//!
//! These functions implement the AMx8x5 support routines for use on Ambiq
//! Micro MCUs.
//!
//! @addtogroup amx8x5 AMx8x5 RTC Device Driver
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

#ifndef AM_DEVICES_AMX8X5_H
#define AM_DEVICES_AMX8X5_H

#ifdef __cplusplus
extern "C"
{
#endif

//*****************************************************************************
//
//! @name Register Defines.
//! @{
//
//*****************************************************************************
#define AM_DEVICES_AMX8X5_HUNDREDTHS                0x00
#define AM_DEVICES_AMX8X5_YEARS                     0x06
#define AM_DEVICES_AMX8X5_ALARM_HUNDRS              0x08
#define AM_DEVICES_AMX8X5_STATUS                    0x0F
#define AM_DEVICES_AMX8X5_CONTROL_1                 0x10
#define AM_DEVICES_AMX8X5_CONTROL_2                 0x11
#define AM_DEVICES_AMX8X5_INT_MASK                  0x12
#define AM_DEVICES_AMX8X5_SQW                       0x13
#define AM_DEVICES_AMX8X5_CAL_XT                    0x14
#define AM_DEVICES_AMX8X5_CAL_RC_HI                 0x15
#define AM_DEVICES_AMX8X5_CAL_RC_LOW                0x16
#define AM_DEVICES_AMX8X5_SLEEP_CTRL                0x17
#define AM_DEVICES_AMX8X5_TIMER_CTRL                0x18
#define AM_DEVICES_AMX8X5_TIMER                     0x19
#define AM_DEVICES_AMX8X5_TIMER_INITIAL             0x1A
#define AM_DEVICES_AMX8X5_WDT                       0x1B
#define AM_DEVICES_AMX8X5_OSC_CONTROL               0x1C
#define AM_DEVICES_AMX8X5_OSC_STATUS                0x1D
#define AM_DEVICES_AMX8X5_CONFIG_KEY                0x1F
#define AM_DEVICES_AMX8X5_ACAL_FLT                  0x26
#define AM_DEVICES_AMX8X5_EXTENDED_ADDR             0x3F
#define AM_DEVICES_AMX8X5_RAM_START                 0x40
//! @}

//! Keys.
#define AM_DEVICES_AMX8X5_CONFIG_KEY_VAL            0xA1

//! @{
//! Modes
#define AM_DEVICES_AMX8X5_12HR_MODE                 0x01
#define AM_DEVICES_AMX8X5_24HR_MODE                 0x02
//! @}

//*****************************************************************************
//
// Device structure used for communication.
//
//*****************************************************************************

//! @{
//! defines to be used in am_devices_amx8x5_t structure.
#define AM_DEVICES_AMX8X5_MODE_SPI                  true
#define AM_DEVICES_AMX8X5_MODE_I2C                  false
//! @}

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
}
am_devices_amx8x5_t;

//*****************************************************************************
//
//! Structure used for time keeping.
//
//*****************************************************************************
typedef struct
{
    uint8_t ui8Hundredth;
    uint8_t ui8Second;
    uint8_t ui8Minute;
    uint8_t ui8Hour;
    uint8_t ui8Date;
    uint8_t ui8Weekday;
    uint8_t ui8Month;
    uint8_t ui8Year;
    uint8_t ui8Century;
    uint8_t ui8Mode;
} am_devices_amx8x5_time_t;

extern am_devices_amx8x5_time_t g_psTimeRegs;

//*****************************************************************************
//
// External function definitions
//
//*****************************************************************************

//*****************************************************************************
//
//! @brief Clear one or more bits.
//!
//! @param psDevice is a pointer to a device structure describing the AMx8x5.
//! @param ui8Address - RTC address.
//! @param ui8Mask - Bits to clear.
//!
//! This function clears one or more bits in the selected register, selected by
//! 1's in the mask.
//
//*****************************************************************************
extern void am_devices_amx8x5_reg_clear(am_devices_amx8x5_t *psDevice,
                                        uint8_t ui8Address, uint8_t ui8Mask);

//*****************************************************************************
//
//! @brief Set one or more bits.
//!
//! @param psDevice is a pointer to a device structure describing the AMx8x5.
//! @param ui8Address - RTC address.
//! @param ui8Mask - Bits to set.
//!
//! This function sets one or more bits in the selected register, selected by
//! 1's in the mask.
//
//*****************************************************************************
extern void am_devices_amx8x5_reg_set(am_devices_amx8x5_t *psDevice,
                                      uint8_t ui8Address, uint8_t ui8Mask);

//*****************************************************************************
//
//! @brief Reads an internal register in the AMx8x5.
//!
//! @param psDevice is a pointer to a device structure describing the AMx8x5.
//! @param ui8Register is the address of the register to read.
//!
//! This function performs a read to an AMx8x5 register over the serial bus.
//!
//! @return return the retrieved byte
//
//*****************************************************************************
extern uint8_t am_devices_amx8x5_reg_read(am_devices_amx8x5_t *psDevice,
                            uint8_t ui8Register);

//*****************************************************************************
//
//! @brief Reads a block of internal registers in the AMx8x5.
//!
//! @param psDevice          - Pointer to a device structure describing the AMx8x5.
//! @param ui8StartRegister - The address of the first register to read.
//! @param pui32Values       - The byte-packed array where the read data will go.
//! @param ui32NumBytes      - The total number of registers to read.
//! @param pfnCallback       - An optional callback function pointer.
//!
//! This function performs a read to a block of AMx8x5 registers over the
//! serial bus. If the \e pfnCallback parameter is nonzero, this function will
//! use the am_hal_iom_spi_read_nb() function as the underlying interface, and
//! \e pfnCallback will be provided to the HAL as the IOM callback function.\n
//! Otherwise, the spi read will be polled.
//
//*****************************************************************************
extern void am_devices_amx8x5_reg_block_read(am_devices_amx8x5_t *psDevice,
                                  uint8_t ui8StartRegister,
                                  uint32_t *pui32Values,
                                  uint32_t ui32NumBytes,
                                  am_hal_iom_callback_t pfnCallback);

//*****************************************************************************
//
//! @brief Writes an internal register in the AMx8x5.
//!
//! @param psDevice is a pointer to a device structure describing the AMx8x5.
//! @param ui8Register is the address of the register to write.
//! @param ui8Value is the value to write to the register.
//!
//! This function performs a write to an AMx8x5 register over the serial bus.
//
//*****************************************************************************
extern void am_devices_amx8x5_reg_write(am_devices_amx8x5_t *psDevice,
                             uint8_t ui8Register, uint8_t ui8Value);

//*****************************************************************************
//
//! @brief Writes a block of internal registers in the AMx8x5.
//!
//! @param psDevice          - Pointer to a device structure describing the AMx8x5.
//! @param ui8StartRegister - The address of the first register to write.
//! @param pui32Values       - The byte-packed array of data to write.
//! @param ui32NumBytes      - Total number of registers to write.
//! @param pfnCallback       - An optional callback function pointer.
//!
//! This function performs a write to a block of AMx8x5 registers over the
//! serial bus. If the \e pfnCallback parameter is nonzero, this function will
//! use the am_hal_iom_spi_write_nb() function as the underlying interface, and
//! \e pfnCallback will be provided to the HAL as the IOM callback function.\n
//! Otherwise, the spi write will be polled.
//
//*****************************************************************************
extern void am_devices_amx8x5_reg_block_write(am_devices_amx8x5_t *psDevice,
                                   uint8_t ui8StartRegister,
                                   uint32_t *pui32Values,
                                   uint32_t ui32NumBytes,
                                   am_hal_iom_callback_t pfnCallback);

//*****************************************************************************
//
//! @brief Reset the AMx8x5.
//!
//! @param psDevice is a pointer to a device structure describing the AMx8x5.
//!
//! This function performs a reset to AMx8x5.
//
//*****************************************************************************
extern void am_devices_amx8x5_reset(am_devices_amx8x5_t *psDevice);

//*****************************************************************************
//
//! @brief Get the time.
//!
//! @param psDevice is a pointer to a device structure describing the AMx8x5.
//!
//! This function loads the g_psTimeRegs structure with the time from the
//! AMX8XX.
//
//*****************************************************************************
extern void am_devices_amx8x5_time_get(am_devices_amx8x5_t *psDevice);

//*****************************************************************************
//
//! @brief Set the time in the counters.
//!
//! @param psDevice is a pointer to a device structure describing the AMx8x5.
//! @param ui8Protect:  0 => leave counters writable
//!                     1 => leave counters unwritable
//!
//! This function loads the AMX8XX counter registers with the current
//! g_psTimeRegs structure values.
//
//*****************************************************************************
extern void am_devices_amx8x5_time_set(am_devices_amx8x5_t *psDevice,
                                       uint8_t ui8Protect);

//*****************************************************************************
//
//! @brief Set the calibration.
//!
//! @param psDevice is a pointer to a device structure describing the AMx8x5.
//! @param  ui8Mode:
//!        - 0 => calibrate the XT oscillator
//!        - 1 => calibrate the RC oscillator
//! @param   iAdjust: Adjustment in ppm.
//!     - Adjustment limits are:
//!             - ui8Mode = 0 => (-610 to +242)
//!             - ui8Mode = 1 => (-65536 to +65520)
//!      - An iAdjust value of zero resets the selected oscillator calibration
//!         value to 0.
//!
//! This function loads the AMX8XX counter registers with the current
//! g_psTimeRegs structure values.
//
//*****************************************************************************
extern void am_devices_amx8x5_cal_set(am_devices_amx8x5_t *psDevice,
                                      uint8_t ui8Mode, int32_t iAdjust);

//*****************************************************************************
//
//! @brief Set the alarm value.
//!
//! @param psDevice is a pointer to a device structure describing the AMx8x5.
//! @param ui8Repeat - the alarm repeat interval
//!        - 0 => disable alarm
//!        - 1 => once per year
//!        - 2 => once per month
//!        - 3 => once per week
//!        - 4 => once per day
//!        - 5 => once per hour
//!        - 6 => once per minute
//!        - 7 => once per second
//!        - 8 => once per 10th of a second
//!        - 9 => once per 100th of a second
//!        - NOTE: year and century are not used
//!        - NOTE: mode must match current 12/24 selection
//! @param ui8IntMode - define the interrupt mode
//!        - 0 => level interrupt
//!        - 1 => pulse of 1/8192s (XT) or 1/128 s (RC)
//!        - 2 => pulse of 1/64 s
//!        - 3 => pulse of 1/4 s
//! @param ui8Pin - pin on which to generate the interrupt
//!        - 0 => internal flag only
//!        - 1 => FOUT/nIRQ
//!        - 2 => PSW/nIRQ2
//!
//! This function sets the alarm value and configures the correct pin (if
//! necessary).
//
//*****************************************************************************
extern void am_devices_amx8x5_alarm_set(am_devices_amx8x5_t *psDevice,
                                        uint8_t ui8Repeat, uint8_t ui8IntMode,
                                        uint8_t ui8Pin);

//*****************************************************************************
//
//! @brief Configure and set the countdown.
//!
//! @param psDevice     - Pointer to a device structure describing the AMx8x5.
//! @param ui8Range:
//!         - 0 => iPeriod in us
//!         - 1 => iPeriod in seconds
//! @param iPeriod      - IPeriod of the countdown timer.
//! @param ui8Repeat    - Configure the interrupt output type:
//!        - 0 => generate a single level interrupt
//!        - 1 => generate a repeated pulsed interrupt,
//!             - 1/4096 s (XT mode),
//!             - 1/128 s  (RC mode)
//!                  - (ui8Range must be 0)
//!        - 2 => generate a single pulsed interrupt,
//!             - 1/4096 s (XT mode),
//!             - 1/128 s  (RC mode)
//!                  - (ui8Range must be 0)
//!        - 3 => generate a repeated pulsed interrupt, 1/128 s (ui8Range must be 0)
//!        - 4 => generate a single pulsed interrupt, 1/128 s (ui8Range must be 0)
//!        - 5 => generate a repeated pulsed interrupt, 1/64 s (ui8Range must be 1)
//!        - 6 => generate a single pulsed interrupt, 1/64 s (ui8Range must be 1)
//! @param ui8Pin       - Select the pin to generate a countdown interrupt:
//!        - 0 => disable the countdown timer
//!        - 1 => generate an interrupt on nTIRQ only, asserted low
//!        - 2 => generate an interrupt on FOUT/nIRQ and nTIRQ, both asserted low
//!        - 3 => generate an interrupt on PSW/nIRQ2 and nTIRQ, both asserted low
//!        - 4 => generate an interrupt on CLKOUT/nIRQ3 and nTIRQ, both asserted low
//!        - 5 => generate an interrupt on CLKOUT/nIRQ3 (asserted high) and nTIRQ
//!        (asserted low)
//!
//! This function configures and sets the countdown.
//
//*****************************************************************************
extern void am_devices_amx8x5_countdown_set(am_devices_amx8x5_t *psDevice,
                                            uint8_t ui8Range, int32_t iPeriod,
                                            uint8_t ui8Repeat, uint8_t ui8Pin);

//*****************************************************************************
//
//! @brief Select an oscillator mode.
//!
//! @param psDevice - Pointer to a device structure describing the AMx8x5.
//! @param ui8OSC   - The oscillator to select
//!        - 0 => 32 KHz XT oscillator, no automatic oscillator switching
//!        - 1 => 32 KHz XT oscillator, automatic oscillator switching to RC on
//!        switch to battery power
//!        - 2 => 128 Hz RC oscillator
//!
//! This function sets the desired oscillator.
//!
//! @return 1 for error
//
//*****************************************************************************
extern uint32_t am_devices_amx8x5_osc_sel(am_devices_amx8x5_t *psDevice,
                                          uint8_t ui8OSC);

//*****************************************************************************
//
//! @brief Configure and enable the square wave output.
//!
//! @param psDevice - Pointer to a device structure describing the AMx8x5.
//! @param ui8SQFS  - Square wave output select (0 to 31)
//! @param ui8Pin   - Output pin for SQW (may be ORed) in addition to CLKOUT
//!        - 0 => disable SQW
//!        - 1 => FOUT
//!        - 2 => PSW/nIRQ2
//!
//! This function configures and enables the square wave output.
//
//*****************************************************************************
extern void am_devices_amx8x5_sqw_set(am_devices_amx8x5_t *psDevice,
                                      uint8_t ui8SQFS, uint8_t ui8Pin);

//*****************************************************************************
//
//! @brief Set up sleep mode (AM18x5 only).
//!
//! @param psDevice     - Pointer to a device structure describing the AMx8x5.
//! @param ui8Timeout   - Minimum timeout period in 7.8 ms periods (0 to 7)
//! @param ui8Mode      - Sleep mode (nRST modes not available in AM08xx)
//!        - 0 => nRST is pulled low in sleep mode
//!        - 1 => PSW/nIRQ2 is pulled high on a sleep
//!        - 2 => nRST pulled low and PSW/nIRQ2 pulled high on sleep
//!
//! This function sets up sleep mode. This is available on the AM18x5 only.
//!
//! @return returned value of the attempted sleep command:
//!        - 0 => sleep request accepted, sleep mode will be initiated in
//!        ui8Timeout seconds
//!        - 1 => illegal input values
//!        - 2 => sleep request declined, interrupt is currently pending
//!        - 3 => sleep request declined, no sleep trigger interrupt enabled
//
//*****************************************************************************
extern uint32_t am_devices_amx8x5_sleep_set(am_devices_amx8x5_t *psDevice,
                                           uint8_t ui8Timeout, uint8_t ui8Mode);

//*****************************************************************************
//
//! @brief Set up the watchdog timer.
//!
//! @param psDevice     - Pointer to a device structure describing the AMx8x5.
//! @param ui8Period    - Timeout period in ms (65 to 124,000)
//! @param ui8Pin       - Pin to generate the watchdog signal
//!        - 0 => disable WDT
//!        - 1 => generate an interrupt on FOUT/nIRQ
//!        - 2 => generate an interrupt on PSW/nIRQ2
//!        - 3 => generate a reset on nRST (AM18xx only)
//!
//! This function sets up sleep mode. This is available on the AM18x5 only.
//
//*****************************************************************************
extern void am_devices_amx8x5_watchdog_set(am_devices_amx8x5_t *psDevice,
                                           uint32_t ui8Period, uint8_t ui8Pin);

//*****************************************************************************
//
//! @brief Set up autocalibration.
//!
//! @param psDevice     - Pointer to a device structure describing the AMx8x5.
//! @param ui8Period    - The repeat period for autocalibration.
//!        - 0 => disable autocalibration
//!        - 1 => execute a single autocalibration cycle
//!        - 2 => execute a cycle every 1024 seconds (~17 minutes)
//!        - 3 => execute a cycle every 512 seconds (~8.5 minutes)
//!
//! This function sets up autocalibration.
//
//*****************************************************************************
extern void am_devices_amx8x5_autocal_set(am_devices_amx8x5_t *psDevice,
                                          uint8_t ui8Period);

//*****************************************************************************
//
//! @brief Gets the extension address for the AMx8x5..
//!
//! @param psDevice   - Pointer to a device structure describing the AMx8x5.
//! @param ui8Address - The address.
//!
//! This function returns the extension address.
//!
//! @return the extension address
//
//*****************************************************************************
extern uint8_t am_devices_amx8x5_ext_address_get(am_devices_amx8x5_t *psDevice,
                                                 uint8_t ui8Address);

//*****************************************************************************
//
//! @brief Read a byte from the local AMX8X5 RAM.
//!
//! @param psDevice     - Pointer to a device structure describing the AMx8x5.
//! @param ui8Address   - RTC RAM address.
//!
//! This function reads a byte from the local AMX8X5 RAM.
//!
//! @return the Value at the desired address.
//
//*****************************************************************************
extern uint8_t am_devices_amx8x5_ram_read(am_devices_amx8x5_t *psDevice,
                                          uint8_t ui8Address);

//*****************************************************************************
//
//! @brief Write a byte to the local AMX8X5 RAM.
//!
//! @param psDevice     - Pointer to a device structure describing the AMx8x5.
//! @param ui8Address   - RTC RAM address.
//! @param ui8Data       - Value to be written.
//!
//! This function writes a byte to the local AMX8X5 RAM.
//
//*****************************************************************************
extern void am_devices_amx8x5_ram_write(am_devices_amx8x5_t *psDevice,
                                        uint8_t ui8Address, uint8_t ui8Data);

#ifdef __cplusplus
}
#endif

#endif // AM_DEVICES_AMX8X5_H

//*****************************************************************************
//
// End Doxygen group.
//! @}
//
//*****************************************************************************

