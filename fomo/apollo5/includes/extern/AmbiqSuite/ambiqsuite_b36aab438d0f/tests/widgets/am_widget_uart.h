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

typedef enum
{
  AM_WIDGET_UART_DATA_ALL_ZEROS,
  AM_WIDGET_UART_DATA_ALL_ONES,
  AM_WIDGET_UART_DATA_COUNTING,
  AM_WIDGET_UART_DATA_RANDOM
} am_widget_uart_data_t;


typedef struct
{
  am_widget_uart_data_t         Data;
  uint32_t                      Size;
  uint32_t                      ui32BaudRate;
  uint32_t                      ui32FlowCtrl;
  bool                          useFifo;
  bool                          useBuffers;
} am_widget_uart_config_t;

extern uint32_t am_widget_uart_setup(am_widget_uart_config_t *pUARTTestConfig, void **ppWidget, char *pErrStr);
extern bool am_widget_uart_test_polling_char(void *pWidget, char *pErrStr);
extern bool am_widget_uart_test_polling_string(void *pWidget, char *pErrStr);
extern bool am_widget_uart_test_buffered_char(void *pWidget, char *pErrStr);
extern bool am_widget_uart_test_buffered_string(void *pWidget, char *pErrStr);
extern bool am_widget_uart_test_power(void);
extern bool am_widget_uart_test_status(void);
extern uint32_t am_widget_uart_cleanup(void *pWidget, char *pErrStr);
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
