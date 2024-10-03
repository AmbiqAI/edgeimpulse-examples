//*****************************************************************************
//
//! @file am_devices_rm67162.c
//!
//! @brief Generic Raydium OLED display driver.
//!
//! @addtogroup rm67162 RM67162 Display Driver
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

#ifndef AM_DEVICES_RM67162_H
#define AM_DEVICES_RM67162_H

#ifdef __cplusplus
extern "C"
{
#endif
//*****************************************************************************
//
//! @name Global definitions for the commands
//! @{
//
//*****************************************************************************
#define AM_DEVICES_RM67162_4SPI_INPUT_DATA_SEL           0xE4
#define AM_DEVICES_RM67162_MTP_LOAD_CTRL                 0xEB
#define AM_DEVICES_RM67162_BOOSTER_ENABLE                0xD1
#define AM_DEVICES_RM67162_GATE_VOL_CTRL                 0xC0
#define AM_DEVICES_RM67162_FRAME_RATE_CTRL               0xB2
#define AM_DEVICES_RM67162_UPDATE_PERIOD_GATE_EQ_CTRL    0xB4
#define AM_DEVICES_RM67162_DUTY_SETTING                  0xB0
#define AM_DEVICES_RM67162_SLEEP_OUT                     0x11
#define AM_DEVICES_RM67162_OSC_ENABLE                    0xC7
#define AM_DEVICES_RM67162_VCOMH_VOL_CTRL                0xCB
#define AM_DEVICES_RM67162_SCAN_MODE                     0x36
#define AM_DEVICES_RM67162_DATA_FORMAT_SEL               0x3A
#define AM_DEVICES_RM67162_DESTRESS_PERIOD_GATE_EQ_CTRL  0xB5
#define AM_DEVICES_RM67162_SOURCE_SETTING                0xB9
#define AM_DEVICES_RM67162_PANEL_SETTING                 0xB8
#define AM_DEVICES_RM67162_COLUMN_ADDR_SETTING           0x2A
#define AM_DEVICES_RM67162_ROW_ADDR_SETTING              0x2B
#define AM_DEVICES_RM67162_TEARING_EFFECT_LINE_ON        0x35
#define AM_DEVICES_RM67162_SET_TEAR_SCANLINE             0x44
#define AM_DEVICES_RM67162_HIGH_POWER_MODE_ON            0x38
#define AM_DEVICES_RM67162_DISPLAY_OFF                   0x28
#define AM_DEVICES_RM67162_DISPLAY_ON                    0x29
#define AM_DEVICES_RM67162_INVERSION_ON                  0x21
#define AM_DEVICES_RM67162_LOW_POWER_MODE_ON             0x39
#define AM_DEVICES_RM67162_MEMORY_WRITE                  0x2C
#define AM_DEVICES_RM67162_MEMORY_WRITE_CONTINUE         0x3C
#define AM_DEVICES_RM67162_MEMORY_READ                   0x2E
#define AM_DEVICES_RM67162_MEMORY_READ_CONTINUE          0x3E
#define AM_DEVICES_RM67162_READ_ID                       0x04
#define AM_DEVICES_RM67162_SET_DSPI_MODE                 0xC4
#define AM_DEVICES_RM67162_SET_WRITE_DISPLAY_CTRL        0x53

//! @}

//*****************************************************************************
//
// Global type definitions.
//
//*****************************************************************************
typedef enum
{
    AM_DEVICES_RM67162_STATUS_SUCCESS,
    AM_DEVICES_RM67162_STATUS_ERROR
} am_devices_rm67162_status_t;

#define AM_DEVICES_RM67162_SPI_WRAM                     0x80
#define AM_DEVICES_RM67162_DSPI_WRAM                    0x81

#define AM_DEVICES_RM67162_COLOR_MODE_8BIT              0x72
#define AM_DEVICES_RM67162_COLOR_MODE_3BIT              0x71
#define AM_DEVICES_RM67162_COLOR_MODE_16BIT             0x75
#define AM_DEVICES_RM67162_COLOR_MODE_24BIT             0x77

#define AM_DEVICES_RM67162_SCAN_MODE_0                  0x40
#define AM_DEVICES_RM67162_SCAN_MODE_90                 0x70
#define AM_DEVICES_RM67162_SCAN_MODE_180                0x10
#define AM_DEVICES_RM67162_SCAN_MODE_270                0x00

typedef struct
{
    uint8_t bus_mode;
    uint8_t color_mode;
    uint8_t scan_mode;

    uint32_t max_row;
    uint32_t max_col;
    uint32_t row_offset;
    uint32_t col_offset;
} am_devices_rm67162_graphic_conf_t;

//*****************************************************************************
//
// External function definitions.
//
//*****************************************************************************
//*****************************************************************************
//
//! @brief Reads the current status of the external display
//!
//! This function reads the device ID register of the external display, and returns
//! the result as an 32-bit unsigned integer value.
//!
//! @return 32-bit status
//
//*****************************************************************************
extern uint32_t am_devices_rm67162_reset(void);

//*****************************************************************************
//
//! @brief
//! @return
//
//*****************************************************************************
extern uint32_t am_devices_rm67162_display_off(void);

//*****************************************************************************
//
//! @brief
//! @return
//
//*****************************************************************************
extern uint32_t am_devices_rm67162_display_on(void);

//*****************************************************************************
//
//! @brief Programs the given range of flash addresses.
//!
//! @param pui8TxBuffer - Buffer to write the external flash data from
//! @param ui32NumBytes - Number of bytes to write to the external flash
//!
//! This function uses the data in the provided pui8TxBuffer and copies it to
//! the external flash at the address given by ui32WriteAddress. It will copy
//! exactly ui32NumBytes of data from the original pui8TxBuffer pointer. The
//! user is responsible for ensuring that they do not overflow the target flash
//! memory or underflow the pui8TxBuffer array
//!
//! @return 32-bit status
//
//*****************************************************************************
extern uint32_t am_devices_rm67162_blocking_write(uint8_t *pui8TxBuffer,
                                                  uint32_t ui32NumBytes);

//*****************************************************************************
//
//! @brief Programs the given range of flash addresses.
//!
//! @param pui8TxBuffer - Buffer to write the external flash data from
//! @param ui32NumBytes - Number of bytes to write to the external flash
//! @param bContinue
//! @param pfnCallback
//! @param pCallbackCtxt - Device number of the external flash
//!
//! This function uses the data in the provided pui8TxBuffer and copies it to
//! the external flash at the address given by ui32WriteAddress. It will copy
//! exactly ui32NumBytes of data from the original pui8TxBuffer pointer. The
//! user is responsible for ensuring that they do not overflow the target flash
//! memory or underflow the pui8TxBuffer array
//!
//! @return 32-bit status
//
//*****************************************************************************
extern uint32_t am_devices_rm67162_nonblocking_write(uint8_t *pui8TxBuffer,
                                                     uint32_t ui32NumBytes,
                                                     bool bContinue,
                                                     am_hal_iom_callback_t pfnCallback,
                                                     void *pCallbackCtxt);

//*****************************************************************************
//
//! @brief Programs the given range of flash addresses.
//!
//! @param pui8TxBuffer - Buffer to write the external flash data from
//! @param ui32NumBytes - Number of bytes to write to the external flash
//! @param bContinue
//! @param ui32PauseCondition
//! @param ui32StatusSetClr
//! @param pfnCallback
//! @param pCallbackCtxt
//!
//! This function uses the data in the provided pui8TxBuffer and copies it to
//! the external flash at the address given by ui32WriteAddress. It will copy
//! exactly ui32NumBytes of data from the original pui8TxBuffer pointer. The
//! user is responsible for ensuring that they do not overflow the target flash
//! memory or underflow the pui8TxBuffer array
//!
//! @return 32-bit status
//
//*****************************************************************************
extern uint32_t am_devices_rm67162_nonblocking_write_adv(uint8_t *pui8TxBuffer, uint32_t ui32NumBytes, bool bContinue, uint32_t ui32PauseCondition, uint32_t ui32StatusSetClr, am_hal_iom_callback_t pfnCallback, void *pCallbackCtxt);

//*****************************************************************************
//
//! @brief Reads the contents of the fram into a buffer.
//!
//! @param pui8RxBuffer - Buffer to store the received data from the flash
//! @param ui32NumBytes - Number of bytes to read from external flash
//!
//! This function reads the external flash at the provided address and stores
//! the received data into the provided buffer location. This function will
//! only store ui32NumBytes worth of data.
//!
//! @return 32-bit status
//
//*****************************************************************************
extern uint32_t am_devices_rm67162_blocking_read(uint8_t *pui8RxBuffer, uint32_t ui32NumBytes);

//*****************************************************************************
//
//! @brief Reads the contents of the fram into a buffer.
//!
//! @param pui8RxBuffer     - Buffer to store the received data from the flash
//! @param ui32NumBytes     - Number of bytes to read from external flash
//! @param pfnCallback      - Callback fcn
//! @param pCallbackCtxt    - Passed to callback function
//!
//! This function reads the external flash at the provided address and stores
//! the received data into the provided buffer location. This function will
//! only store ui32NumBytes worth of data.
//
//! @return 32-bit status
//
//*****************************************************************************
extern uint32_t am_devices_rm67162_nonblocking_read(uint8_t *pui8RxBuffer, uint32_t ui32NumBytes, am_hal_iom_callback_t pfnCallback, void *pCallbackCtxt);

//*****************************************************************************
//
//! @brief
//! @param pdata
//! @return
//
//*****************************************************************************
extern uint32_t am_devices_rm67162_read_id(uint32_t *pdata);

//*****************************************************************************
//
//! @brief Initialize the rm67162 driver.
//!
//! @param ui32Module     - IOM module ID.
//! @param psIOMSettings  - IOM device structure describing the target spiflash.
//! @param psGraphic_conf  -
//!
//! This function should be called before any other am_devices_rm67162
//! functions. It is used to set tell the other functions how to communicate
//! with the OLED display hardware.
//!
//! @return Status.
//
//*****************************************************************************
extern uint32_t am_devices_rm67162_init(uint32_t ui32Module,
                                        am_hal_iom_config_t *psIOMSettings,
                                        am_devices_rm67162_graphic_conf_t *psGraphic_conf);

//*****************************************************************************
//
//! @brief De-Initialize the rm67162 driver.
//!
//! @param ui32Module     - IOM Module#
//!
//! This function reverses the initialization
//!
//! @return Status.
//
//*****************************************************************************
extern uint32_t am_devices_rm67162_term(uint32_t ui32Module);

//*****************************************************************************
//
//! @brief Generic Command Write function.
//!
//! @param bHiPrio
//! @param ui32Instr
//! @param bdata
//!
//! @return
//
//*****************************************************************************
extern uint32_t am_devices_rm67162_command_write(bool bHiPrio, uint32_t ui32Instr, bool bdata);

//*****************************************************************************
//
//! @brief
//! @param bHiPrio
//! @param startRow
//! @param startCol
//! @param endRow
//! @param endCol
//! @return
//
//*****************************************************************************
extern uint32_t am_devices_rm67162_set_transfer_window(bool bHiPrio, uint32_t startRow, uint32_t startCol, uint32_t endRow, uint32_t endCol);

//*****************************************************************************
//
//! @brief
//! @param x
//! @param y
//! @return
//
//*****************************************************************************
extern uint32_t am_devices_rm67162_setting_address(uint32_t x, uint32_t y);

#ifdef __cplusplus
}
#endif

#endif // AM_DEVICES_RM67162_H

//*****************************************************************************
//
// End Doxygen group.
//! @}
//
//*****************************************************************************

