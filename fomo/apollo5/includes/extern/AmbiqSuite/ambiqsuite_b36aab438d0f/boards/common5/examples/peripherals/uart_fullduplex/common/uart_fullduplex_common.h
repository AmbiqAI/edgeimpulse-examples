//*****************************************************************************
//
//! @file uart_fullduplex_common.h
//!
//! @brief uart fullduplex common definitions.
//!
//
//*****************************************************************************
//*****************************************************************************
//
// ${copyright}
//
// This is part of revision ${version} of the AmbiqSuite Development Package.
//
//*****************************************************************************
#include "am_mcu_apollo.h"
#include "am_bsp.h"
#include "am_util.h"

#define TEST_UART_TX_MODULE     3
#define TEST_UART_RX_MODULE     0

#define CUST_UART0RX_PIN        62
#define CUST_UART0RTS_PIN       58

#define UART_DMA_ENABLED        1

#define UART_QUEUE_SIZE         2048
#define UART_BUFFER_SIZE        2048

//#define UART_TEST_PACKET_SIZE   200
//#define UART_TEST_PACKET_SIZE   1024
#define UART_TEST_PACKET_SIZE   2048

//#define UART_TEST_PACKET_CNT    10
#define UART_TEST_PACKET_CNT    50

#define UART_SLAVE_READY_IN_PIN 5
#define UART_SLAVE_READY_OUT_PIN 5

typedef struct
{
    uint8_t  uart_tx;
    uint8_t  uart_rx;
    uint32_t length;

    bool txblocking;
    bool rxblocking;

    uint32_t txbufferSize;
    uint32_t rxbufferSize;

    uint32_t txBytesTransferred;
    uint32_t rxBytesTransferred;

    bool txQueueEnable;
    bool rxQueueEnable;

    uint8_t pui8QueueTx[UART_QUEUE_SIZE];
    uint8_t pui8QueueRx[UART_QUEUE_SIZE];

    uint8_t *pui8SpaceTx;
    uint8_t *pui8SpaceRx;

    uint32_t result;
}
am_uart_fifo_config_t;

void common_setup(void);
void common_teardown(void);
void uart_flush_fifo(void);
int16_t uart_run_fullduplex_test(void);
void uart_pin_config(void);
void uart_init(void);

