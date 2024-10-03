//*****************************************************************************
//
//! @file am_widget_sdio.h
//
//*****************************************************************************

//*****************************************************************************
//
// ${copyright}
//
// This is part of revision ${version} of the AmbiqSuite Development Package.
//
//*****************************************************************************
#ifndef AM_WIDGET_SDIO_H
#define AM_WIDGET_SDIO_H

#ifdef __cplusplus
extern "C"
{
#endif

typedef enum
{
    AM_WIDGET_SDIO_SUCCESS,
    AM_WIDGET_SDIO_ERROR
} am_widget_sdio_status_t;  

extern am_hal_card_host_t *pSdhcCardHost;
extern am_hal_card_t eMMCard;
extern char *dummy_key_hash;

extern volatile bool bAsyncWriteIsDone;
extern volatile bool bAsyncReadIsDone;

extern bool am_widget_check_data_match(uint8_t *pui8RdBuf, uint8_t *pui8WrBuf, uint32_t ui32Len);
extern bool am_widget_prepare_data_pattern(uint32_t pattern_index, uint8_t* buff, uint32_t len);
extern bool am_widget_check_key(void);

extern bool am_widget_sdio_setup(void);
extern bool am_widget_sdio_cleanup(void);

extern void am_hal_card_event_test_cb(am_hal_host_evt_t *pEvt);
extern uint32_t am_widget_sdio_cal_time(uint32_t ui32prev, uint32_t ui32curr);
extern uint32_t am_widget_sd_card_power_config(am_hal_card_pwr_e eCardPwr);

#ifdef __cplusplus
}
#endif

#endif // AM_WIDGET_SDIO_H

//*****************************************************************************
//
// End Doxygen group.
//! @}
//
//*****************************************************************************
