//*****************************************************************************
//
//! @file am_devices_mspi_psram_aps6408l.h
//!
//! @brief Micron Serial SPI PSRAM driver.
//!
//! @addtogroup mspi_psram_aps64048l APS6408L MSPI PSRAM Driver
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

#ifndef AM_DEVICES_MSPI_PSRAM_APS6408L_H
#define AM_DEVICES_MSPI_PSRAM_APS6408L_H

#ifdef __cplusplus
extern "C"
{
#endif

//*****************************************************************************
//
//! @name Global definitions for psram commands
//! @{
//
//*****************************************************************************
#define AM_DEVICES_MSPI_PSRAM_APS6408L_SYNC_READ        0x00
#define AM_DEVICES_MSPI_PSRAM_APS6408L_SYNC_WRITE       0x80
#define AM_DEVICES_MSPI_PSRAM_APS6408L_SYNC_READ_LB     0x20
#define AM_DEVICES_MSPI_PSRAM_APS6408L_SYNC_WRITE_LB    0xA0
#define AM_DEVICES_MSPI_PSRAM_APS6408L_MODE_REG_READ    0x40
#define AM_DEVICES_MSPI_PSRAM_APS6408L_MODE_REG_WRITE   0xC0
#define AM_DEVICES_MSPI_PSRAM_APS6408L_GLOBAL_RESET     0xFF

//! @}

//*****************************************************************************
//
//! @name  Device specific identification.
//! @{
//
//*****************************************************************************
#define AM_DEVICES_MPSI_PSRAM_APS6408L_PAGE_SIZE        1024
//! @}

//*****************************************************************************
//
//! @name  Global type definitions.
//! @{
//
//*****************************************************************************
typedef enum
{
    AM_DEVICES_MSPI_PSRAM_STATUS_SUCCESS,
    AM_DEVICES_MSPI_PSRAM_STATUS_ERROR
} am_devices_mspi_psram_status_t;
//! @}

//*****************************************************************************
//
// External function definitions.
//
//*****************************************************************************
extern volatile uint32_t g_MSPIInterruptStatus;

//*****************************************************************************
//
//! @brief Initialize the mspi_psram driver.
//!
//! @param ui32Module     - Module number
//! @param psMSPISettings - MSPI device structure describing the target spi psram.
//! @param pHandle        - MSPI handler which needs to be return
//!
//! This function should be called before any other am_devices_mspi_psram
//! functions. It is used to set tell the other functions how to communicate
//! with the external psram hardware.
//!
//! @return status.
//
//*****************************************************************************
extern uint32_t am_devices_mspi_psram_aps6408l_init(uint32_t ui32Module,
                                           am_hal_mspi_dev_config_t *psMSPISettings,
                                           void **pHandle);

//*****************************************************************************
//
//! @brief
//! @param psMSPISettings
//! @return
//
//*****************************************************************************
extern uint32_t am_devices_mspi_psram_aps6408l_deinit(am_hal_mspi_dev_config_t *psMSPISettings);

//*****************************************************************************
//
//! @brief Reads the ID of the external psram and returns the value.
//!
//! This function reads the device ID register of the external psram, compares
//! the value to known good value, and returns success/failure.
//!
//! @return 32-bit status
//
//*****************************************************************************
extern uint32_t am_devices_mspi_psram_aps6408l_id(void);

//*****************************************************************************
//
//! @brief  Reset the PSRAM
//!
//! @return
//
//*****************************************************************************
extern uint32_t am_devices_mspi_psram_aps6408l_reset(void);

//*****************************************************************************
//
//! @brief
//! @param pui8RxBuffer
//! @param ui32ReadAddress
//! @param ui32NumBytes
//! @param ui32PauseCondition
//! @param ui32StatusSetClr
//! @param pfnCallback
//! @param pCallbackCtxt
//! @return
//
//*****************************************************************************
extern uint32_t am_devices_mspi_psram_aps6408l_read(uint8_t *pui8RxBuffer,
                                           uint32_t ui32ReadAddress,
                                           uint32_t ui32NumBytes,
                                           uint32_t ui32PauseCondition,
                                           uint32_t ui32StatusSetClr,
                                           am_hal_mspi_callback_t pfnCallback,
                                           void *pCallbackCtxt);

//*****************************************************************************
//
//! @brief
//! @param puiTxBuffer
//! @param ui32WriteAddress
//! @param ui32NumBytes
//! @param ui32PauseCondition
//! @param ui32StatusSetClr
//! @param pfnCallback
//! @param pCallbackCtxt
//! @return
//
//*****************************************************************************
extern uint32_t am_devices_mspi_psram_aps6408l_write(uint8_t *puiTxBuffer,
                                           uint32_t ui32WriteAddress,
                                           uint32_t ui32NumBytes,
                                           uint32_t ui32PauseCondition,
                                           uint32_t ui32StatusSetClr,
                                           am_hal_mspi_callback_t pfnCallback,
                                           void *pCallbackCtxt);

#if 0
extern uint32_t am_devices_mspi_psram_enable_xip(void);

extern uint32_t am_devices_mspi_psram_disable_xip(void);

extern uint32_t am_devices_mspi_psram_enable_scrambling(void);

extern uint32_t am_devices_mspi_psram_disable_scrambling(void);

extern uint32_t am_devices_mspi_psram_read_hiprio(uint8_t *pui8RxBuffer,
                           uint32_t ui32ReadAddress,
                           uint32_t ui32NumBytes,
                           am_hal_mspi_callback_t pfnCallback,
                           void *pCallbackCtxt);
extern uint32_t am_devices_mspi_psram_nonblocking_read(uint8_t *pui8RxBuffer,
                           uint32_t ui32ReadAddress,
                           uint32_t ui32NumBytes,
                           am_hal_mspi_callback_t pfnCallback,
                           void *pCallbackCtxt);

extern uint32_t am_devices_mspi_psram_write_hiprio(uint8_t *pui8TxBuffer,
                           uint32_t ui32WriteAddress,
                           uint32_t ui32NumBytes,
                           am_hal_mspi_callback_t pfnCallback,
                           void *pCallbackCtxt);
extern uint32_t am_devices_mspi_psram_nonblocking_write(uint8_t *pui8TxBuffer,
                           uint32_t ui32WriteAddress,
                           uint32_t ui32NumBytes,
                           am_hal_mspi_callback_t pfnCallback,
                           void *pCallbackCtxt);
#endif

#ifdef __cplusplus
}
#endif

#endif // AM_DEVICES_MSPI_PSRAM_APS6408L_H

//*****************************************************************************
//
// End Doxygen group.
//! @}
//
//*****************************************************************************

