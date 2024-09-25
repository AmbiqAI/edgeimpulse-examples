//*****************************************************************************
//
//! @file uart_fullduplex.c
//!
//! @brief This example demonstrates how apollo5 UART work in fullduplex mode.
//! To run this example, user needs two apollo5b_eb_revb boards and define
//! UART_FULLDUPLEX_HOST and UART_FULLDUPLEX_SLAVE respectively for the host and
//! the slave, then power on the host first, then the slave.
//!
//! @verbatim
//! Pin connections for the UART Master board to the UART Slave board.
//!
//!     HOST (uart_fullduplex_host)             SLAVE (uart_fullduplex)
//!     --------------------                    ----------------
//!     GPIO[60]  UART0 TX                      GPIO[63]  UART3 RX
//!     GPIO[63]  UART3 RX                      GPIO[60]  UART0 TX
//!     GPIO[58]  UART3 RTS                     GPIO[59]  UART0 CTS
//!     GPIO[59]  UART0 CTS                     GPIO[58]  UART3 RTS
//!     GPIO[5]  Slave Ready INT (host)         GPIO[5]  Slave Ready INT (slave)
//!     GND                                     GND
//!
//
//*****************************************************************************

//*****************************************************************************
//
// Copyright (c) 2024, Ambiq Micro, Inc.
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// 1. Redistributions of source code must retain the above copyright notice,
// this list of conditions and the following disclaimer.
//
// 2. Redistributions in binary form must reproduce the above copyright
// notice, this list of conditions and the following disclaimer in the
// documentation and/or other materials provided with the distribution.
//
// 3. Neither the name of the copyright holder nor the names of its
// contributors may be used to endorse or promote products derived from this
// software without specific prior written permission.
//
// Third party software included in this distribution is subject to the
// additional license terms as defined in the /docs/licenses directory.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
// ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
// LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
// CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
// SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
// INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
// CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
// ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
// POSSIBILITY OF SUCH DAMAGE.
//
// This is part of revision release_a5b_sdk2-748191cd0 of the AmbiqSuite Development Package.
//
//*****************************************************************************

#include "am_mcu_apollo.h"
#include "am_bsp.h"
#include "am_util.h"

#define TEST_UART_TX_MODULE     0
#define TEST_UART_RX_MODULE     3

#define CUST_UART0TX_PIN        60
#define CUST_UART0CTS_PIN       59

#define UART_FULLDUPLEX_HOST    1
#define UART_FULLDUPLEX_SLAVE   0

#define UART_DMA_ENABLED        1

#define UART_QUEUE_SIZE         2048
#define UART_BUFFER_SIZE        2048

//#define UART_TEST_PACKET_SIZE   200
//#define UART_TEST_PACKET_SIZE   1024
#define UART_TEST_PACKET_SIZE   2048

//#define UART_TEST_PACKET_CNT    10
#define UART_TEST_PACKET_CNT    50

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

AM_SHARED_RW uint8_t ui8TxBuffer[UART_BUFFER_SIZE] __attribute__((aligned(32)));
AM_SHARED_RW uint8_t ui8RxBuffer[UART_BUFFER_SIZE] __attribute__((aligned(32)));

am_uart_fifo_config_t g_sUartFifoCfg =
{
    .uart_tx       = TEST_UART_TX_MODULE,
    .uart_rx       = TEST_UART_RX_MODULE,
    .length     = UART_TEST_PACKET_SIZE,

    .txblocking = 0,
    .rxblocking = 0,

    .txbufferSize = UART_BUFFER_SIZE,
    .rxbufferSize = UART_BUFFER_SIZE,

    .txBytesTransferred = 0,
    .rxBytesTransferred = 0,

    .txQueueEnable = 1,
    .rxQueueEnable = 1,

    .pui8SpaceTx = ui8TxBuffer,
    .pui8SpaceRx = ui8RxBuffer,
};

//*****************************************************************************
//
// State variables for the UART.
//
//*****************************************************************************
void *g_hUART_TX;
void *g_hUART_RX;

volatile bool g_bUartTxDone = false;
volatile bool g_bUartRxDone = false;

uint32_t g_ui32ErrorStatus = 0;
uint16_t g_ui16TestCount = 0;

#if UART_FULLDUPLEX_HOST
volatile bool g_bSlaveReady = false;
#endif

//*****************************************************************************
//
// UART Configuration
//
//*****************************************************************************
am_hal_uart_config_t sUartConfig =
{
    // Tested with 2 boards.
#if defined(AM_PART_APOLLO5B)
    .ui32BaudRate = 3000000,
#else
    .ui32BaudRate = 2900000,
#endif
    //.ui32BaudRate = 115200 * 5,
    //.ui32BaudRate = 115200,

    .eDataBits = AM_HAL_UART_DATA_BITS_8,
    .eParity = AM_HAL_UART_PARITY_NONE,
    .eStopBits = AM_HAL_UART_TWO_STOP_BITS,
    .eFlowControl = AM_HAL_UART_FLOW_CTRL_RTS_CTS,
    //.eTXFifoLevel = AM_HAL_UART_FIFO_LEVEL_8,
    .eTXFifoLevel = AM_HAL_UART_FIFO_LEVEL_16,
    //.eTXFifoLevel = AM_HAL_UART_FIFO_LEVEL_24,
    //.eRXFifoLevel = AM_HAL_UART_FIFO_LEVEL_8
    .eRXFifoLevel = AM_HAL_UART_FIFO_LEVEL_16
    //.eRXFifoLevel = AM_HAL_UART_FIFO_LEVEL_24
};

#if UART_FULLDUPLEX_HOST
#define UART_SLAVE_READY_IN_PIN 5
#endif

#if UART_FULLDUPLEX_SLAVE
#define UART_SLAVE_READY_OUT_PIN 5
#endif

#if UART_FULLDUPLEX_HOST
//*****************************************************************************
//
// Interrupt handler for the GPIO pins.
//
//*****************************************************************************
void
am_gpio0_001f_isr(void)
{
    am_hal_gpio_mask_t IntStatus;
    uint32_t    ui32IntStatus;

    am_hal_gpio_interrupt_status_get(AM_HAL_GPIO_INT_CHANNEL_0,
                                     false,
                                     &IntStatus);
    am_hal_gpio_interrupt_irq_status_get(GPIO0_001F_IRQn, false, &ui32IntStatus);
    am_hal_gpio_interrupt_irq_clear(GPIO0_001F_IRQn, ui32IntStatus);
    am_hal_gpio_interrupt_service(GPIO0_001F_IRQn, ui32IntStatus);
}

// ISR callback for the slave ready int
static void slave_rdy_handler(void *pArg)
{
    g_bSlaveReady = true;
}
#endif

bool data_handler(uint8_t* pData, uint32_t length)
{
    uint8_t checksum = 0;
    // confirm checksum
    for ( uint32_t i = 0; i < (length - 1); i++ )
    {
        checksum += pData[i];
    }

    if ( checksum != pData[length - 1] )
    {
        // checksum error
        am_util_stdio_printf("\nRX data not matched at round %d test\n", g_ui16TestCount);
        return 1;
    }
    else
    {
        // checksum pass
        am_util_stdio_printf("# \n");
        return 0;
    }
}

//*****************************************************************************
//
// UART interrupt handler.
//
//*****************************************************************************
void uart_isr_service(uint8_t module)
{
    //
    // Service the FIFOs as necessary, and clear the interrupts.
    //
    uint32_t ui32Status;

    if ( (TEST_UART_TX_MODULE != TEST_UART_RX_MODULE) && (TEST_UART_RX_MODULE == module) )
    {
        am_hal_uart_interrupt_status_get(g_hUART_RX, &ui32Status, true);
        am_hal_uart_interrupt_clear(g_hUART_RX, ui32Status);
        am_hal_uart_interrupt_service(g_hUART_RX, ui32Status);
    }
    else
    {
        am_hal_uart_interrupt_status_get(g_hUART_TX, &ui32Status, true);
        am_hal_uart_interrupt_clear(g_hUART_TX, ui32Status);
        am_hal_uart_interrupt_service(g_hUART_TX, ui32Status);
    }
}

void am_uart_isr(void)
{
    uart_isr_service(0);
}

void am_uart1_isr(void)
{
    uart_isr_service(1);
}

void am_uart2_isr(void)
{
    uart_isr_service(2);
}

void am_uart3_isr(void)
{
    uart_isr_service(3);
}

void
tx_complete_callback(uint32_t ui32ErrorStatus, void *pvContext)
{
    g_bUartTxDone = true;
}

void
rx_complete_callback(uint32_t ui32ErrorStatus, void *pvContext)
{
    g_bUartRxDone = true;
#if UART_DMA_ENABLED
    am_hal_cachectrl_range_t sRange;
    sRange.ui32StartAddr = (uint32_t)g_sUartFifoCfg.pui8SpaceRx;
    sRange.ui32Size = UART_TEST_PACKET_SIZE;
    am_hal_cachectrl_dcache_invalidate(&sRange, false);
#endif
}

//*****************************************************************************
//
// Main function.
//
//*****************************************************************************
int main(void)
{
    am_hal_uart_transfer_t  sTransactionTx;
#if (UART_DMA_ENABLED == 0)
    am_hal_uart_transfer_t  sTransactionRx;
#else
    am_hal_cachectrl_range_t sRange;
#endif
    //
    // Configure the board for low power operation.
    //
    am_bsp_low_power_init();

    //
    // Enable HFADJ
    //
    am_hal_clkgen_control(AM_HAL_CLKGEN_CONTROL_HFADJ_ENABLE, 0);

    //
    // Enable the ITM print interface.
    //
    am_bsp_itm_printf_enable();

    //
    // Clear the terminal and print the banner.
    //
    am_util_stdio_terminal_clear();

#if UART_FULLDUPLEX_HOST
    am_util_stdio_printf("UART FIFO Full-duplex Host Example\n");

    uint32_t IntNum = UART_SLAVE_READY_IN_PIN;
    am_hal_gpio_pinconfig(IntNum, am_hal_gpio_pincfg_input);

    uint32_t ui32IntStatus = (1 << IntNum);
    am_hal_gpio_interrupt_irq_clear(GPIO0_001F_IRQn, ui32IntStatus);
    am_hal_gpio_interrupt_register(AM_HAL_GPIO_INT_CHANNEL_0, IntNum, slave_rdy_handler, NULL);
    am_hal_gpio_interrupt_control(AM_HAL_GPIO_INT_CHANNEL_0,
                                  AM_HAL_GPIO_INT_CTRL_INDV_ENABLE,
                                  (void *)&IntNum);
    NVIC_EnableIRQ(GPIO0_001F_IRQn);
    am_hal_interrupt_master_enable();

    while(g_bSlaveReady == false);
    g_bSlaveReady = false;
#elif UART_FULLDUPLEX_SLAVE
    am_util_stdio_printf("UART FIFO Full-duplex Slave Example\n");

    am_hal_gpio_pinconfig(UART_SLAVE_READY_OUT_PIN, am_hal_gpio_pincfg_output);

    am_hal_gpio_output_set(UART_SLAVE_READY_OUT_PIN);
    am_hal_gpio_output_clear(UART_SLAVE_READY_OUT_PIN);
#endif
    am_util_stdio_printf("Make sure UART_FULLDUPLEX_HOST and UART_FULLDUPLEX_SLAVE are correctly defined for this board before start.\n");
    am_util_stdio_printf("And switch the IOX mode to 10 if use apollo5b_eb_revb\n");

#if defined(CUST_UART0TX_PIN) && defined(CUST_UART0CTS_PIN)
    if (g_sUartFifoCfg.uart_tx == 0)
    {
        am_hal_gpio_pinconfig( CUST_UART0TX_PIN, g_AM_BSP_GPIO_UART0_TX);
        am_hal_gpio_pinconfig( CUST_UART0CTS_PIN, g_AM_BSP_GPIO_UART0_CTS);
        am_util_stdio_printf("TX: %d\tCTS: %d\n", CUST_UART0TX_PIN, CUST_UART0CTS_PIN);
    }
#elif defined(AM_BSP_GPIO_UART0_TX) && defined(AM_BSP_GPIO_UART0_CTS)
    if (g_sUartFifoCfg.uart_tx == 0)
    {
        am_hal_gpio_pinconfig( AM_BSP_GPIO_UART0_TX, g_AM_BSP_GPIO_UART0_TX);
        am_hal_gpio_pinconfig( AM_BSP_GPIO_UART0_CTS, g_AM_BSP_GPIO_UART0_CTS);
        am_util_stdio_printf("TX: %d\tCTS: %d\n", AM_BSP_GPIO_UART0_TX, AM_BSP_GPIO_UART0_CTS);
    }
#endif
#if defined(AM_BSP_GPIO_UART1_TX) && defined(AM_BSP_GPIO_UART1_CTS)
    else if (g_sUartFifoCfg.uart_tx == 1)
    {
        am_hal_gpio_pinconfig( AM_BSP_GPIO_UART1_TX, g_AM_BSP_GPIO_UART1_TX);
        am_hal_gpio_pinconfig( AM_BSP_GPIO_UART1_CTS, g_AM_BSP_GPIO_UART1_CTS);
        am_util_stdio_printf("TX: %d\tCTS: %d\n", AM_BSP_GPIO_UART1_TX, AM_BSP_GPIO_UART1_CTS);
    }
#endif
#if defined(AM_BSP_GPIO_UART2_TX) && defined(AM_BSP_GPIO_UART2_CTS)
    else if (g_sUartFifoCfg.uart_tx == 2)
    {
        am_hal_gpio_pinconfig( AM_BSP_GPIO_UART2_TX, g_AM_BSP_GPIO_UART2_TX);
        am_hal_gpio_pinconfig( AM_BSP_GPIO_UART2_CTS, g_AM_BSP_GPIO_UART2_CTS);
        am_util_stdio_printf("TX: %d\tCTS: %d\n", AM_BSP_GPIO_UART2_TX, AM_BSP_GPIO_UART2_CTS);
    }
#endif
#if defined(AM_BSP_GPIO_UART3_TX) && defined(AM_BSP_GPIO_UART3_CTS)
    else if (g_sUartFifoCfg.uart_tx == 3)
    {
        am_hal_gpio_pinconfig( AM_BSP_GPIO_UART3_TX, g_AM_BSP_GPIO_UART3_TX);
        am_hal_gpio_pinconfig( AM_BSP_GPIO_UART3_CTS, g_AM_BSP_GPIO_UART3_CTS);
        am_util_stdio_printf("TX: %d\tCTS: %d\n", AM_BSP_GPIO_UART3_TX, AM_BSP_GPIO_UART3_CTS);
    }
#endif

#if defined(AM_BSP_GPIO_UART0_RX) && defined(AM_BSP_GPIO_UART0_RTS)
    if (g_sUartFifoCfg.uart_rx == 0)
    {
        am_hal_gpio_pinconfig( AM_BSP_GPIO_UART0_RX, g_AM_BSP_GPIO_UART0_RX);
        am_hal_gpio_pinconfig( AM_BSP_GPIO_UART0_RTS, g_AM_BSP_GPIO_UART0_RTS);
        am_util_stdio_printf("RX: %d\tRTS: %d\n", AM_BSP_GPIO_UART0_RX, AM_BSP_GPIO_UART0_RTS);
    }
#endif
#if defined(AM_BSP_GPIO_UART1_RX) && defined(AM_BSP_GPIO_UART1_RTS)
    else if (g_sUartFifoCfg.uart_rx == 1)
    {
        am_hal_gpio_pinconfig( AM_BSP_GPIO_UART1_RX, g_AM_BSP_GPIO_UART1_RX);
        am_hal_gpio_pinconfig( AM_BSP_GPIO_UART1_RTS, g_AM_BSP_GPIO_UART1_RTS);
        am_util_stdio_printf("RX: %d\tRTS: %d\n", AM_BSP_GPIO_UART1_RX, AM_BSP_GPIO_UART1_RTS);
    }
#endif
#if defined(AM_BSP_GPIO_UART2_RX) && defined(AM_BSP_GPIO_UART2_RTS)
    else if (g_sUartFifoCfg.uart_rx == 2)
    {
        am_hal_gpio_pinconfig( AM_BSP_GPIO_UART2_RX, g_AM_BSP_GPIO_UART2_RX);
        am_hal_gpio_pinconfig( AM_BSP_GPIO_UART2_RTS, g_AM_BSP_GPIO_UART2_RTS);
        am_util_stdio_printf("RX: %d\tRTS: %d\n", AM_BSP_GPIO_UART2_RX, AM_BSP_GPIO_UART2_RTS);
    }
#endif
#if defined(AM_BSP_GPIO_UART3_RX) && defined(AM_BSP_GPIO_UART3_RTS)
    else if (g_sUartFifoCfg.uart_rx == 3)
    {
        am_hal_gpio_pinconfig( AM_BSP_GPIO_UART3_RX, g_AM_BSP_GPIO_UART3_RX);
        am_hal_gpio_pinconfig( AM_BSP_GPIO_UART3_RTS, g_AM_BSP_GPIO_UART3_RTS);
        am_util_stdio_printf("RX: %d\tRTS: %d\n", AM_BSP_GPIO_UART3_RX, AM_BSP_GPIO_UART3_RTS);
    }
#endif

    am_hal_uart_initialize( g_sUartFifoCfg.uart_tx, &g_hUART_TX );
    am_hal_uart_power_control(g_hUART_TX, AM_HAL_SYSCTRL_WAKE, false);
    am_hal_uart_configure(g_hUART_TX, &sUartConfig);
    if ( g_sUartFifoCfg.uart_tx != g_sUartFifoCfg.uart_rx )
    {
        am_hal_uart_initialize( g_sUartFifoCfg.uart_rx, &g_hUART_RX );
        am_hal_uart_power_control(g_hUART_RX, AM_HAL_SYSCTRL_WAKE, false);
        am_hal_uart_configure(g_hUART_RX, &sUartConfig);
    }

#if UART_DMA_ENABLED
    if ( g_sUartFifoCfg.uart_tx == g_sUartFifoCfg.uart_rx )
    {
        am_util_stdio_printf("\n UART DMA is one-directional, have to use two instances for full duplex!!!\n");
        while(1);
    }
    else
    {
        //
        // Enable interrupts so we can receive messages from the boot host.
        //
        NVIC_EnableIRQ((IRQn_Type)(UART0_IRQn + g_sUartFifoCfg.uart_rx));
    }
    am_hal_uart_interrupt_enable(g_hUART_TX, (AM_HAL_UART_INT_OVER_RUN |
                                           AM_HAL_UART_INT_DMAERIS  |
                                           AM_HAL_UART_INT_DMACPRIS));
    am_hal_uart_interrupt_enable(g_hUART_RX, (AM_HAL_UART_INT_RX_TMOUT |
                                           AM_HAL_UART_INT_OVER_RUN |
                                           AM_HAL_UART_INT_DMAERIS  |
                                           AM_HAL_UART_INT_DMACPRIS));
#else
    if ( g_sUartFifoCfg.uart_tx != g_sUartFifoCfg.uart_rx )
    {
        am_hal_uart_interrupt_enable(g_hUART_TX, (AM_HAL_UART_INT_TX |
                                               AM_HAL_UART_INT_OVER_RUN |
                                               AM_HAL_UART_INT_TXCMP));
        am_hal_uart_interrupt_enable(g_hUART_RX, (AM_HAL_UART_INT_RX |
                                               AM_HAL_UART_INT_RX_TMOUT |
                                               AM_HAL_UART_INT_OVER_RUN));
    }
    else
    {
        am_hal_uart_interrupt_enable(g_hUART_TX, (AM_HAL_UART_INT_TX |
                                               AM_HAL_UART_INT_RX |
                                               AM_HAL_UART_INT_RX_TMOUT |
                                               AM_HAL_UART_INT_OVER_RUN |
                                               AM_HAL_UART_INT_TXCMP));
    }

    uint8_t* pTxQueue = NULL;
    uint8_t* pRxQueue = NULL;
    uint32_t TxQueueSize = 0;
    uint32_t RxQueueSize = 0;
    if ( g_sUartFifoCfg.txblocking == false )
    {
        pTxQueue = g_sUartFifoCfg.pui8QueueTx;
        TxQueueSize = sizeof(g_sUartFifoCfg.pui8QueueTx);
    }

    if ( g_sUartFifoCfg.rxblocking == false )
    {
        pRxQueue = g_sUartFifoCfg.pui8QueueRx;
        RxQueueSize = sizeof(g_sUartFifoCfg.pui8QueueRx);
    }
    if ( g_sUartFifoCfg.uart_tx != g_sUartFifoCfg.uart_rx )
    {
        am_hal_uart_buffer_configure(g_hUART_TX, pTxQueue, TxQueueSize, NULL, 0);
        am_hal_uart_buffer_configure(g_hUART_RX, NULL, 0, pRxQueue, RxQueueSize);
        //
        // Enable interrupts so we can receive messages from the boot host.
        //
        NVIC_EnableIRQ((IRQn_Type)(UART0_IRQn + g_sUartFifoCfg.uart_rx));
    }
    else
    {
        am_hal_uart_buffer_configure(g_hUART_TX, pTxQueue, TxQueueSize, pRxQueue, RxQueueSize);
    }
#endif
    //
    // Enable interrupts so we can receive messages from the boot host.
    //
    NVIC_EnableIRQ((IRQn_Type)(UART0_IRQn + g_sUartFifoCfg.uart_tx));
    am_hal_interrupt_master_enable();

    //
    // Loop forever.
    //

    uint32_t txTestLength = UART_TEST_PACKET_SIZE - 1;  // starting from 1 byte

    while(1)
    {
        g_bUartTxDone = false;
        g_bUartRxDone = false;

#if UART_FULLDUPLEX_HOST
        while(g_bSlaveReady == false);
        g_bSlaveReady = false;
#elif UART_FULLDUPLEX_SLAVE
        am_hal_gpio_output_set(UART_SLAVE_READY_OUT_PIN);
        am_hal_gpio_output_clear(UART_SLAVE_READY_OUT_PIN);
#endif
        // There maybe dummy byte after uart reset, flush the rx fifo before start transactions
        uint8_t pui8Data[4];
        uint32_t ui32BytesTransferred;
        am_hal_uart_fifo_read(g_hUART_RX, pui8Data, 4, &ui32BytesTransferred);

        // Generate tx data
        uint8_t checksum = 0;
        for ( uint32_t i = 0; i < txTestLength; i++ )
        {
            g_sUartFifoCfg.pui8SpaceTx[i] = i + 1;
            checksum += g_sUartFifoCfg.pui8SpaceTx[i];
        }
        // actual data length is txTestLength+1
        g_sUartFifoCfg.pui8SpaceTx[txTestLength] = checksum;

        // How many bytes should we send?
        sTransactionTx.ui32NumBytes = UART_TEST_PACKET_SIZE;
        // When the transaction is complete, this will be set to the number of
        // bytes we read.
        sTransactionTx.pui32BytesTransferred = &g_sUartFifoCfg.txBytesTransferred;
        sTransactionTx.ui32ErrorStatus = 0;

        // Data location to use for this transaction.
#if UART_DMA_ENABLED
        sRange.ui32StartAddr = (uint32_t)g_sUartFifoCfg.pui8SpaceTx;
        sRange.ui32Size = UART_TEST_PACKET_SIZE;
        am_hal_cachectrl_dcache_clean(&sRange);

        sTransactionTx.pui32TxBuffer = (uint32_t*)g_sUartFifoCfg.pui8SpaceTx;
        sTransactionTx.pui32RxBuffer = (uint32_t*)g_sUartFifoCfg.pui8SpaceRx;
        sTransactionTx.pfnCallback = tx_complete_callback;
        sTransactionTx.pfnFdRxCallback = rx_complete_callback;
        sTransactionTx.ui32FdRxNumBytes = UART_TEST_PACKET_SIZE;
        sTransactionTx.pui32FdRxBytesTransferred = &g_sUartFifoCfg.rxBytesTransferred;
        am_hal_uart_dma_fullduplex_transfer(g_hUART_TX, g_hUART_RX, &sTransactionTx);
#else
        sTransactionTx.pui8Data = g_sUartFifoCfg.pui8SpaceTx;
        // For blocking transactions, this determines how long the UART HAL should
        // wait before aborting the transaction.
        if (g_sUartFifoCfg.txblocking)
        {
            sTransactionTx.eType = AM_HAL_UART_BLOCKING_WRITE;
            sTransactionTx.ui32TimeoutMs = AM_HAL_UART_WAIT_FOREVER;
        }
        else
        {
            sTransactionTx.eType = AM_HAL_UART_NONBLOCKING_WRITE;
            sTransactionTx.ui32TimeoutMs = 0;
            // For non-blocking transfers, the UART HAL will call this callback
            // function as soon as the requested action is complete.
            sTransactionTx.pfnCallback = tx_complete_callback;
        }
        am_hal_uart_transfer(g_hUART_TX, &sTransactionTx);

        // How many bytes should we send?
        sTransactionRx.ui32NumBytes = UART_TEST_PACKET_SIZE;
        // When the transaction is complete, this will be set to the number of
        // bytes we read.
        sTransactionRx.pui32BytesTransferred = &g_sUartFifoCfg.rxBytesTransferred;
        sTransactionRx.ui32ErrorStatus = 0;
        // Data location to use for this transaction.
        sTransactionRx.pui8Data = g_sUartFifoCfg.pui8SpaceRx;
        // For blocking transactions, this determines how long the UART HAL should
        // wait before aborting the transaction.
        if (g_sUartFifoCfg.rxblocking)
        {
            // Is this a write or a read?
            sTransactionRx.eType = AM_HAL_UART_BLOCKING_READ;
            sTransactionRx.ui32TimeoutMs = AM_HAL_UART_WAIT_FOREVER;
        }
        else
        {
            // Is this a write or a read?
            sTransactionRx.eType = AM_HAL_UART_NONBLOCKING_READ;
            sTransactionRx.ui32TimeoutMs = 0;
            // For non-blocking transfers, the UART HAL will call this callback
            // function as soon as the requested action is complete.
            sTransactionRx.pfnCallback = rx_complete_callback;
        }

        if ( g_sUartFifoCfg.uart_tx != g_sUartFifoCfg.uart_rx )
        {
            am_hal_uart_transfer(g_hUART_RX, &sTransactionRx);
        }
        else
        {
            am_hal_uart_transfer(g_hUART_TX, &sTransactionRx);
        }
#endif
        am_hal_uart_tx_flush(g_hUART_TX);
        //while(g_bUartTxDone == false);
        while(g_bUartRxDone == false);

        if ( *sTransactionTx.pui32BytesTransferred != UART_TEST_PACKET_SIZE )
        {
            // We didn't send all of the data we were supposed to.
            am_util_stdio_printf("\nTX data length not matched!");
            break;
        }
#if UART_DMA_ENABLED
        if ( *sTransactionTx.pui32FdRxBytesTransferred != UART_TEST_PACKET_SIZE )
        {
            // We didn't receive all of the data we sent.
            am_util_stdio_printf("\nRX data length not matched!");
            break;
        }
#else
        if ( *sTransactionRx.pui32BytesTransferred != UART_TEST_PACKET_SIZE )
        {
            // We didn't receive all of the data we sent.
            am_util_stdio_printf("\nRX data length not matched!");
            break;
        }
#endif

        if ( data_handler(g_sUartFifoCfg.pui8SpaceRx, UART_TEST_PACKET_SIZE) )
        {
            break;
        }

        am_util_stdio_printf(".");

        g_ui16TestCount++;
        if ( g_ui16TestCount == UART_TEST_PACKET_CNT )
        {
            am_util_stdio_printf("\nTest %d transfer done\n", g_ui16TestCount);
            g_ui16TestCount = 0;
            break;
        }
#if UART_FULLDUPLEX_SLAVE
        am_util_delay_ms(1);
#endif
    }
    //
    // Shut everything back down.
    //
    am_hal_uart_power_control(g_hUART_TX, AM_HAL_SYSCTRL_DEEPSLEEP, false);
    am_hal_uart_deinitialize(g_hUART_TX);
    if ( g_sUartFifoCfg.uart_tx != g_sUartFifoCfg.uart_rx )
    {
        am_hal_uart_power_control(g_hUART_RX, AM_HAL_SYSCTRL_DEEPSLEEP, false);
        am_hal_uart_deinitialize(g_hUART_RX);
    }
}

