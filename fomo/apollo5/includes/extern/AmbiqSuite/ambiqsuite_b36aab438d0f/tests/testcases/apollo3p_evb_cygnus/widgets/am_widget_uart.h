//*****************************************************************************
//
//! @file am_widget_uart.h
//!
//! @brief
//!
//! @addtogroup hal Hardware Abstraction Layer (HAL)
//! @addtogroup ctimer Counter/Timer (CTIMER)
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
#ifndef AM_WIDGET_UART_H
#define AM_WIDGET_UART_H

#ifdef __cplusplus
extern "C"
{
#endif

extern void *UART;

typedef struct
{
    uint32_t length;

    bool txblocking;
    bool rxblocking;

    uint32_t txbuffer;
    uint32_t rxbuffer;

    uint32_t result;
}
am_widget_uart_loopback_config_t;

uint32_t am_widget_uart_loopback(am_widget_uart_loopback_config_t *psConfig);

#ifdef __cplusplus
}
#endif

#endif // AM_WIDGET_UART_H

//*****************************************************************************
//
// End Doxygen group.
//! @}
//
//*****************************************************************************
