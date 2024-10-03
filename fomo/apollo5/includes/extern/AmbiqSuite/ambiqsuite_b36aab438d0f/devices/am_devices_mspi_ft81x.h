//*****************************************************************************
//
//! @file am_devices_mspi_ft81x.h
//!
//! @brief MSPI FT81X display driver.
//!
//! @addtogroup mspi_ft81x FT81X MSPI Display Driver
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

#ifndef AM_DEVICES_FT81X_H
#define AM_DEVICES_FT81X_H

#ifdef __cplusplus
extern "C"
{
#endif

#define ROW_NUM                 390//390//316//180
#define COLUMN_NUM              390//390//200//390//316//180//120//(120*3)

#define ROW_START       0
#define COL_START       0
#define ROW_ADJ         0
#define COL_ADJ         6

//*****************************************************************************
//
//! @name Global definitions for the commands
//! @{
//
//*****************************************************************************
#define AM_DEVICES_FT81X_DISPLAY_RAM_BASE       0x00000000

#define AM_DEVICES_FT81X_DEVICE_ID_ADDR         0x000C0000
#define AM_DEVICES_FT81X_DEVICE_ID_VAL          0x00011208

#define AM_DEVICES_FT81X_RAM_DL_BASE            0x00300000
#define AM_DEVICES_FT81X_REG_BASE               0x00302000
#define AM_DEVICES_FT81X_REG_ID                 0x00302000
#define AM_DEVICES_FT81X_REG_ID_VAL             0x0000007C
#define AM_DEVICES_FX81X_REG_HCYCLE             0x0030202C
#define AM_DEVICES_FT81X_REG_HOFFSET            0x00302030
#define AM_DEVICES_FT81X_REG_HSIZE              0x00302034
#define AM_DEVICES_FT81X_REG_HSYNC0             0x00302038
#define AM_DEVICES_FT81X_REG_HSYNC1             0x0030203C
#define AM_DEVICES_FT81X_REG_VCYCLE             0x00302040
#define AM_DEVICES_FT81X_REG_VOFFSET            0x00302044
#define AM_DEVICES_FT81X_REG_VSIZE              0x00302048
#define AM_DEVICES_FT81X_REG_VSYNC0             0x0030204C
#define AM_DEVICES_FT81X_REG_VSYNC1             0x00302050
#define AM_DEVICES_FT81X_REG_DLSWAP             0x00302054
#define AM_DEVICES_FT81X_REG_SWIZZLE            0x00302064
#define AM_DEVICES_FT81X_REG_CSPREAD            0x00302068
#define AM_DEVICES_FT81X_REG_PCLK_POL           0x0030206C
#define AM_DEVICES_FT81X_REG_PCLK               0x00302070
#define AM_DEVICES_FT81X_REG_GPIO_DIR           0x00302090
#define AM_DEVICES_FT81X_REG_GPIO               0x00302094
#define AM_DEVICES_FT81X_REG_PWM_DUTY           0x003020D4
#define AM_DEVICES_FT81X_REG_SPI_WIDTH          0x00302188

#define AM_DEVICES_FT81X_CLEAR_COLOR_RGB(r, b, g) (0x02000000 | (r << 16) | (b << 8) | g)
//! @}

//*****************************************************************************
//
// Global type definitions.
//
//*****************************************************************************
  typedef enum
  {
    AM_DEVICES_FT81X_STATUS_SUCCESS,
    AM_DEVICES_FT81X_STATUS_ERROR
  } am_devices_ft81x_status_t;

  typedef struct
  {
      am_hal_mspi_device_e eDeviceConfig;
      am_hal_mspi_clock_e eClockFreq;
      uint32_t *pNBTxnBuf;
      uint32_t ui32NBTxnBufLength;
      uint32_t ui32ScramblingStartAddr;
      uint32_t ui32ScramblingEndAddr;
  } am_devices_mspi_ft81x_config_t;

#define AM_DEVICES_MSPI_FT81X_MAX_DEVICE_NUM    1

//*****************************************************************************
//
// External function definitions.
//
//*****************************************************************************

//*****************************************************************************
//
//! @brief
//!
//! @param pHandle
//! @param ui32Addr
//! @param pData
//! @param ui32NumBytes
//!
//! @return
//
//*****************************************************************************
extern uint32_t am_devices_ft81x_command_write(void *pHandle,
                                                 uint32_t ui32Addr,
                                                 uint8_t *pData,
                                                 uint32_t ui32NumBytes);
//*****************************************************************************
//
//! @brief
//!
//! @param pHandle
//! @param ui32Addr
//! @param pData
//! @param ui32NumBytes
//!
//! @return
//
//*****************************************************************************
extern uint32_t am_devices_ft81x_command_read(void *pHandle,
                                                uint32_t ui32Addr,
                                                uint8_t *pData,
                                                uint32_t ui32NumBytes);

//extern uint32_t am_devices_ft81x_device_id(void *pHandle);

//*****************************************************************************
//
//! @brief Programs the given range of flash addresses.
//!
//! @param pHandle           - Pointer to device driver handle
//! @param ui32WriteAddress  - Address to write to in the external flash
//! @param pui8TxBuffer      - Buffer to write the external flash data from
//! @param ui32NumBytes      - Number of bytes to write to the external flash
//! @param bWaitForCompletion - When true, this function will block and wait
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
extern uint32_t am_devices_ft81x_nonblocking_write(void *pHandle,
                                                     uint32_t ui32WriteAddress,
                                                     uint8_t *pui8TxBuffer,
                                                     uint32_t ui32NumBytes,
                                                     bool bWaitForCompletion);

  //*****************************************************************************
//
//! @brief Programs the given range of flash addresses.
//!
//! @param pHandle              - Pointer to driver handle
//! @param ui32WriteAddress     - Address to write to in the external flash
//! @param pui8TxBuffer         - Buffer to write the external flash data from
//! @param ui32NumBytes         - Number of bytes to write to the external flash
//! @param ui32PauseCondition
//! @param ui32StatusSetClr
//! @param pfnCallback          - Callback function
//! @param pCallbackCtxt        - Callback context
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
extern uint32_t am_devices_ft81x_nonblocking_write_adv(void *pHandle,
                                                         uint32_t ui32WriteAddress,
                                                         uint8_t *pui8TxBuffer,
                                                         uint32_t ui32NumBytes,
                                                         uint32_t ui32PauseCondition,
                                                         uint32_t ui32StatusSetClr,
                                                         am_hal_mspi_callback_t pfnCallback,
                                                         void *pCallbackCtxt);

//*****************************************************************************
//
//! @brief Reads the contents of the fram into a buffer.
//!
//! @param pHandle          - Pointer to device driver handle
//! @param ui32ReadAddress  - Address of desired data in external flash
//! @param pui8RxBuffer     - Buffer to store the received data from the flash
//! @param ui32NumBytes     - Number of bytes to read from external flash
//! @param bWaitForCompletion - When true this function will block
//!
//! This function reads the external display at the provided address and stores
//! the received data into the provided buffer location. This function will
//! only store ui32NumBytes worth of data.
//!
//! @return 32-bit status
//
//*****************************************************************************
extern uint32_t am_devices_ft81x_nonblocking_read(void *pHandle,
                                                    uint32_t ui32ReadAddress,
                                                    uint8_t *pui8RxBuffer,
                                                    uint32_t ui32NumBytes,
                                                    bool bWaitForCompletion);
//*****************************************************************************
//
//! @brief Initialize the FT81X driver.
//!
//! @param ui32Module     - MSPI module ID.
//! @param psMspiSettings - MSPI device structure describing the target spiflash.
//! @param ppHandle       - pointer to driver handle.
//! @param ppMspiHandle   - pointer to MSPI handle.
//!
//! This function should be called before any other am_devices_ft81x
//! functions. It is used to set tell the other functions how to communicate
//! with the TFT display hardware.
//!
//! @return Status.
//
//*****************************************************************************
extern uint32_t am_devices_mspi_ft81x_init(uint32_t ui32Module, am_devices_mspi_ft81x_config_t *psMspiSettings, void **ppHandle, void **ppMspiHandle);

  //*****************************************************************************
//
//! @brief De-Initialize the FT81X driver.
//!
//! @param pHandle     - Pointer to driver handle
//!
//! This function reverses the initialization
//!
//! @return Status.
//
//*****************************************************************************
extern uint32_t am_devices_ft81x_term(void *pHandle);

#ifdef __cplusplus
}
#endif

#endif // AM_DEVICES_FT81X_H

//*****************************************************************************
//
// End Doxygen group.
//! @}
//
//*****************************************************************************

