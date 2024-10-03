//*****************************************************************************
//
//! @file am_devices_st7301.h
//!
//! @brief sitronix TFT display driver.
//!
//! @addtogroup st7301 ST7301 - Sitronix TFT display driver
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

#ifndef AM_DEVICES_ST7301_H
#define AM_DEVICES_ST7301_H

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
#define AM_DEVICES_ST7301_4SPI_INPUT_DATA_SEL           0xE4
#define AM_DEVICES_ST7301_MTP_LOAD_CTRL                 0xEB
#define AM_DEVICES_ST7301_BOOSTER_ENABLE                0xD1
#define AM_DEVICES_ST7301_GATE_VOL_CTRL                 0xC0
#define AM_DEVICES_ST7301_FRAME_RATE_CTRL               0xB2
#define AM_DEVICES_ST7301_UPDATE_PERIOD_GATE_EQ_CTRL    0xB4
#define AM_DEVICES_ST7301_DUTY_SETTING                  0xB0
#define AM_DEVICES_ST7301_SLEEP_OUT                     0x11
#define AM_DEVICES_ST7301_OSC_ENABLE                    0xC7
#define AM_DEVICES_ST7301_VCOMH_VOL_CTRL                0xCB
#define AM_DEVICES_ST7301_MEM_DATA_ACC_CTRL             0x36
#define AM_DEVICES_ST7301_DATA_FORMAT_SEL               0x3A
#define AM_DEVICES_ST7301_DESTRESS_PERIOD_GATE_EQ_CTRL  0xB5
#define AM_DEVICES_ST7301_SOURCE_SETTING                0xB9
#define AM_DEVICES_ST7301_PANEL_SETTING                 0xB8
#define AM_DEVICES_ST7301_COLUMN_ADDR_SETTING           0x2A
#define AM_DEVICES_ST7301_ROW_ADDR_SETTING              0x2B
#define AM_DEVICES_ST7301_TEARING_EFFECT_LINE_ON        0x35
#define AM_DEVICES_ST7301_SET_TEAR_SCANLINE             0x44
#define AM_DEVICES_ST7301_HIGH_POWER_MODE_ON            0x38
#define AM_DEVICES_ST7301_DISPLAY_OFF                   0x28
#define AM_DEVICES_ST7301_DISPLAY_ON                    0x29
#define AM_DEVICES_ST7301_INVERSION_ON                  0x21
#define AM_DEVICES_ST7301_LOW_POWER_MODE_ON             0x39
#define AM_DEVICES_ST7301_MEMORY_WRITE                  0x2C
#define AM_DEVICES_ST7301_MEMORY_WRITE_CONTINUE         0x3C
#define AM_DEVICES_ST7301_MEMORY_READ                   0x2E
#define AM_DEVICES_ST7301_MEMORY_READ_CONTINUE          0x3E
#define AM_DEVICES_ST7301_READ_ID                       0x04
//! @}

//*****************************************************************************
//
//! @name Global type definitions.
//! @{
//
//*****************************************************************************
typedef enum
{
    AM_DEVICES_ST7301_STATUS_SUCCESS,
    AM_DEVICES_ST7301_STATUS_ERROR
} am_devices_st7301_status_t;

typedef struct
{
    uint32_t ui32ClockFreq;
    uint32_t *pNBTxnBuf;
    uint32_t ui32NBTxnBufLength;
} am_devices_st7301_config_t;
//! @}

#define AM_DEVICES_ST7301_MAX_DEVICE_NUM    1

//*****************************************************************************
//
// External function definitions.
//
//*****************************************************************************

//*****************************************************************************
//
//! @brief Reads the current status of the external display
//!
//! @param pHandle - Pointer to driver handle
//!
//! This function reads the device ID register of the external display, and returns
//! the result as an 32-bit unsigned integer value.
//!
//! @return 32-bit status
//
//*****************************************************************************
extern uint32_t am_devices_st7301_reset(void *pHandle);

//*****************************************************************************
//
//! @brief
//! @param pHandle - Pointer to driver handle
//! @return
//
//*****************************************************************************
extern uint32_t am_devices_st7301_display_off(void *pHandle);

//*****************************************************************************
//
//! @brief
//! @param pHandle - Pointer to driver handle
//! @return
//
//*****************************************************************************
extern uint32_t am_devices_st7301_display_on(void *pHandle);

//*****************************************************************************
//
//! @brief Programs the given range of flash addresses.
//!
//! @param pHandle - Pointer to driver handle
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
extern uint32_t am_devices_st7301_blocking_write(void *pHandle,
                                                 uint8_t *pui8TxBuffer,
                                                 uint32_t ui32NumBytes);

//*****************************************************************************
//
//! @brief Programs the given range of flash addresses.
//!
//! @param pHandle - Pointer to driver handle
//! @param pui8TxBuffer - Buffer to write the external flash data from
//! @param ui32NumBytes - Number of bytes to write to the external flash
//! @param bContinue
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
extern uint32_t am_devices_st7301_nonblocking_write(void *pHandle,
                                                    uint8_t *pui8TxBuffer,
                                                    uint32_t ui32NumBytes,
                                                    bool bContinue,
                                                    am_hal_iom_callback_t pfnCallback,
                                                    void *pCallbackCtxt);

//*****************************************************************************
//
//! @brief Programs the given range of flash addresses.
//!
//! @param pHandle - Pointer to driver handle
//! @param pui8TxBuffer
//! @param ui32NumBytes
//! @param bContinue
//! @param ui32Instr
//! @param ui32InstrLen
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
extern uint32_t am_devices_st7301_nonblocking_write_adv(void *pHandle,
                                                        uint8_t *pui8TxBuffer,
                                                        uint32_t ui32NumBytes,
                                                        bool bContinue,
                                                        uint32_t ui32Instr,
                                                        uint32_t ui32InstrLen,
                                                        uint32_t ui32PauseCondition,
                                                        uint32_t ui32StatusSetClr,
                                                        am_hal_iom_callback_t pfnCallback,
                                                        void *pCallbackCtxt);

//*****************************************************************************
//
//! @brief Reads the contents of the fram into a buffer.
//!
//! @param pHandle       - Pointer to driver handle
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
extern uint32_t am_devices_st7301_blocking_read(void *pHandle,
                                                uint8_t *pui8RxBuffer,
                                                uint32_t ui32NumBytes);

//*****************************************************************************
//
//! @brief Reads the contents of the fram into a buffer.
//!
//! @param pHandle
//! @param pui8RxBuffer - Buffer to store the received data from the flash
//! @param ui32NumBytes - Number of bytes to read from external flash
//! @param pfnCallback
//! @param pCallbackCtxt
//!
//! This function reads the external flash at the provided address and stores
//! the received data into the provided buffer location. This function will
//! only store ui32NumBytes worth of data.
//!
//! @return 32-bit status
//
//*****************************************************************************
extern uint32_t am_devices_st7301_nonblocking_read(void *pHandle,
                                                   uint8_t *pui8RxBuffer,
                                                   uint32_t ui32NumBytes,
                                                   am_hal_iom_callback_t pfnCallback,
                                                   void *pCallbackCtxt);

//*****************************************************************************
//
//! @brief
//!
//! @param pHandle
//! @param pdata
//!
//! @return
//
//*****************************************************************************
extern uint32_t am_devices_st7301_read_id(void *pHandle,
                                          uint32_t *pdata);

//*****************************************************************************
//
//! @brief Initialize the st7301 driver.
//!
//! @param ui32Module     - IOM module ID.
//! @param pDevConfig
//! @param ppHandle
//! @param ppIomHandle
//!
//! This function should be called before any other am_devices_st7301
//! functions. It is used to set tell the other functions how to communicate
//! with the TFT display hardware.
//!
//! @return Status.
//
//*****************************************************************************
extern uint32_t am_devices_st7301_init(uint32_t ui32Module,
                                       am_devices_st7301_config_t *pDevConfig,
                                       void **ppHandle,
                                       void **ppIomHandle);

//*****************************************************************************
//
//! @brief De-Initialize the st7301 driver.
//!
//! @param pHandle     -
//!
//! This function reverses the initialization
//!
//! @return Status.
//
//*****************************************************************************
extern uint32_t am_devices_st7301_term(void *pHandle);

//*****************************************************************************
//
//! @brief Generic Command Write function.
//!
//! @param pHandle
//! @param bHiPrio
//! @param ui32Instr
//! @param bdata
//!
//! @return
//
//*****************************************************************************
extern uint32_t am_devices_st7301_command_write(void *pHandle,
                                                bool bHiPrio,
                                                uint32_t ui32Instr,
                                                bool bdata);
//*****************************************************************************
//
//! @brief
//! @param pHandle - Pointer to driver handle
//! @param bHiPrio
//! @param startRow
//! @param startCol
//! @param endRow
//! @param endCol
//! @return
//
//*****************************************************************************
extern uint32_t am_devices_st7301_set_transfer_window(void *pHandle,
                                                      bool bHiPrio,
                                                      uint32_t startRow,
                                                      uint32_t startCol,
                                                      uint32_t endRow,
                                                      uint32_t endCol);

#ifdef __cplusplus
}
#endif

#endif // AM_DEVICES_ST7301_H

//*****************************************************************************
//
// End Doxygen group.
//! @}
//
//*****************************************************************************

