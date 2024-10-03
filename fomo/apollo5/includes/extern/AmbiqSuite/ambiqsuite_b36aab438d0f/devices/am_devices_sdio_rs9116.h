//*****************************************************************************
//
//! @file am_devices_sdio_rs9116.h
//!
//! @brief SiliconLabs RS9116 Wi-Fi device driver.
//!
//! @addtogroup RS9116 SDIO Wi-Fi Driver
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
#ifndef AM_DEVICES_SDIO_RS9116_H
#define AM_DEVICES_SDIO_RS9116_H

#ifdef __cplusplus
extern "C"
{
#endif

//*****************************************************************************
//
// Global definitions
//
//*****************************************************************************
extern am_hal_card_host_t *pSdhcCardHost;
extern am_hal_card_t       RS9116SdioCard;
extern uint32_t            ui32SdioModule;
extern uint8_t             sdio_init_done;

//*****************************************************************************
//
// Global type definitions.
//
//*****************************************************************************
typedef enum
{
    AM_DEVICES_SDIO_RS9116_STATUS_SUCCESS = 0,
    AM_DEVICES_SDIO_RS9116_STATUS_ERROR  = -1,
} am_devices_sdio_rs9116_status_t;

//*****************************************************************************
//
// External function definitions.
//
//*****************************************************************************
extern int16_t rsi_sdio_write_multiple(uint8_t *tx_data, uint32_t Addr, uint32_t no_of_blocks);
extern int8_t rsi_sdio_read_multiple(uint8_t *read_buff, uint32_t Addr, uint32_t no_of_blocks);
extern int8_t sdio_reg_writeb(uint32_t Addr, uint8_t *dBuf);
extern int8_t sdio_reg_readb(uint32_t Addr, uint8_t *dBuf);
extern int16_t rsi_sdio_readb(uint32_t addr, uint16_t len, uint8_t *dBuf);
extern int16_t rsi_sdio_writeb(uint32_t addr, uint16_t len, uint8_t *dBuf);
extern int32_t rsi_mcu_sdio_init(void);
extern int32_t rsi_sdio_apply_timing(uint8_t ui8TxRxDelays[2]);
extern int32_t rsi_sdio_timing_scan(am_hal_host_inst_index_e eIndex,
                           am_hal_host_uhs_mode_e eUHSMode,
                           uint32_t ui32Clock,
                           am_hal_host_bus_width_e eBusWidth,
                           uint8_t *ui8CalibBuf,
                           uint32_t ui32StartAddr,
                           uint32_t ui32BlockCnt,
                           am_hal_host_bus_voltage_e eIoVoltage,
                           uint8_t ui8TxRxDelays[2],
                           am_hal_sdio_card_reset_func pSdioCardReset);

#ifdef __cplusplus
}
#endif

#endif // AM_DEVICES_SDIO_RS9116_H

//*****************************************************************************
// End Doxygen group.
//! @}
//
//*****************************************************************************

