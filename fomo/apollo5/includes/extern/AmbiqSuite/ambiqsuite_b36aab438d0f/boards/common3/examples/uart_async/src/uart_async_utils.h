//*****************************************************************************
//
//! @file uart_async_utils.h
//!
//! @brief Functions to setup and manage the uart asynchronous driver
//!
//! @addtogroup peripheral_examples Peripheral Examples
//
//! @defgroup uart_async UART Asynchronous Example
//! @ingroup peripheral_examples
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

#ifndef RTT_UART_ASYNC_UTILS_H
#define RTT_UART_ASYNC_UTILS_H

#include "am_mcu_apollo.h"
#include "am_bsp.h"


#define UART_ID                            (0)
#define TIMER_NUM       0               ///< Timer number used in the example
#define MAX_UART_PACKET_SIZE            (2048)
#define UART_RX_TIMEOUT_MS              (5)
#define NUM_UARTS_USED 1

//
//! enumerate uart names
//
typedef enum
{
#if defined(AM_BSP_GPIO_UART_TX) && defined(AM_BSP_GPIO_UART_RX)
    eUART0,
#endif
#if defined(AM_BSP_GPIO_UART1_TX) && defined(AM_BSP_GPIO_UART1_RX)
    eUART1,
#endif
    eMAX_UARTS,
    euidX32 = 0x70000000,  // force this to 32bit value on all compilers
}
uart_id_e;

//
//! define data used per uart instance
//
typedef struct
{
    void *pvUART;
    uint32_t ui32ISrErrorCount;
    am_hal_uart_status_t e32Status;

    //
    //! Saved buffer sizes for the uart
    //
    uint32_t ui32TxBuffSize;
    uint32_t ui32RxBuffSize;

    //
    //! in this example, all uart buffers are the same size,
    //! this is not needed and can be modified
    //
    uint8_t pui8UARTTXBuffer[MAX_UART_PACKET_SIZE];
    uint8_t pui8UARTRXBuffer[MAX_UART_PACKET_SIZE];

    uart_id_e uartId;
    volatile bool bRxComplete;

}
uart_async_local_vars_t;

uint32_t init_uart(uart_id_e uartId, uart_async_local_vars_t **ppUartLocalVar);

#endif //RTT_UART_ASYNC_UTILS_H
//*****************************************************************************
//! @}
//*****************************************************************************
