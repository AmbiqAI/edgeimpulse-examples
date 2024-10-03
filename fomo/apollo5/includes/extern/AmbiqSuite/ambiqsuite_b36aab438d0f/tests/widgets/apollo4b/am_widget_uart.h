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
    uint8_t  uart;
    uint32_t length;

    bool txblocking;
    bool rxblocking;

    uint32_t txbufferSize;
    uint32_t rxbufferSize;

    uint32_t txBytesTransferred;
    uint32_t rxBytesTransferred;

    bool txQueueEnable;
    bool rxQueueEnable;

    uint8_t pui8QueueTx[256];
    uint8_t pui8QueueRx[256];

    uint8_t pui8SpaceTx[256];
    uint8_t pui8SpaceRx[256];

    uint32_t result;
}
am_widget_uart_loopback_config_t;

extern void am_widget_uart_loopback_nonblocking_init(am_widget_uart_loopback_config_t *psConfigloopback,
                                                     uint8_t enableQueue, uint32_t length);
extern uint32_t am_widget_uart_loopback(am_widget_uart_loopback_config_t *psConfig);
extern void am_widget_uart_loopback_blocking_init(am_widget_uart_loopback_config_t *psConfigloopback,
                                                  uint8_t enableQueue, uint32_t length);

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
