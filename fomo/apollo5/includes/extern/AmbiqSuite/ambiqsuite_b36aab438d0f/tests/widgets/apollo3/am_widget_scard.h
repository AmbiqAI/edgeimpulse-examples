//*****************************************************************************
//
//! @file am_widget_scard.h
//!
//! @brief
//!
//! @addtogroup hal Hardware Abstraction Layer (HAL)
//! @addtogroup smart card controller (SCC)
//! @ingroup hal
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
#ifndef AM_WIDGET_SCARD_H
#define AM_WIDGET_SCARD_H

#ifdef __cplusplus
extern "C"
{
#endif


typedef struct
{
    am_hal_scard_config_t         SCARDConfig;            // SCARD configuration.
    uint32_t                      ui32Module;             // SCARD instance number.
}
am_widget_scard_config_t;

typedef struct
{
    uint32_t length;

    bool txblocking;
    bool rxblocking;

    uint32_t txbuffer;
    uint32_t rxbuffer;

    uint32_t result;

    char* string;
}am_widget_scard_test_config_t;

extern void *SCARD_Handle;

extern uint32_t am_widget_scard_test_setup(am_widget_scard_config_t *pTestCfg, void **ppWidget, char *pErrStr);
extern uint32_t am_widget_scard_test_cleanup(void *pWidget, char *pErrStr);
extern uint32_t am_widget_scard_get_atr(uint8_t *pui8ATR, uint8_t *pui8Len);
extern uint32_t am_widget_scard_pps_exchange(am_hal_scard_pps_t stPPS);
extern uint32_t am_widget_scard_recv_data(uint8_t* pui8DataBuf);
extern uint32_t am_widget_scard_send_data(uint8_t* pui8DataBuf);


#ifdef __cplusplus
}
#endif

#endif // AM_WIDGET_SCARD_H

//*****************************************************************************
//
// End Doxygen group.
//! @}
//
//*****************************************************************************
