//*****************************************************************************
//
//! @file uart_fullduplex_common.c
//!
//! @brief This example demonstrates how apollo5 UART work in fullduplex mode.
//! To run this example, user needs two apollo5b_eb_revb boards and flash
//! uart_fullduplex_host and uart_fullduplex respectively for the host and
//! the slave, then power on the host first, then the slave.
//!
//! @verbatim
//! Pin connections for the UART Master board to the UART Slave board.
//!
//!     HOST (uart_fullduplex_host)             SLAVE (uart_fullduplex)
//!     --------------------                    ----------------
//!     GPIO[61]  UART3 TX                      GPIO[62]  UART0 RX
//!     GPIO[62]  UART0 RX                      GPIO[61]  UART3 TX
//!     GPIO[58]  UART0 RTS                     GPIO[60]  UART3 CTS
//!     GPIO[60]  UART3 CTS                     GPIO[58]  UART0 RTS
//!     GPIO[5]  Slave Ready INT (host)         GPIO[5]  Slave Ready INT (slave)
//!     GND                                     GND
//!
//! TX          Pin 61 : J57
//! RX          Pin 62 : J57
//! RTS         Pin 58 : J50
//! CTS         Pin 60 : J50
//! Handshake   Pin 5  : J48
//!
//*****************************************************************************

//*****************************************************************************
//
// ${copyright}
//
// This is part of revision ${version} of the AmbiqSuite Development Package.
//
//*****************************************************************************
#include "uart_fullduplex_common.h"

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
        am_util_stdio_printf("\nRX data not matched\n");
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
void common_setup(void)
{
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
    // Enable SYSPLL.
    //
    am_hal_clkmgr_clock_config(AM_HAL_CLKMGR_CLK_ID_SYSPLL, AM_HAL_UART_PLLCLK_FREQ, NULL);
}

void common_teardown(void)
{
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

void uart_flush_fifo(void)
{
    // There maybe dummy byte after uart reset, flush the rx fifo before start transactions
    uint8_t pui8Data[4];
    uint32_t ui32BytesTransferred;
    am_hal_uart_fifo_read(g_hUART_RX, pui8Data, 4, &ui32BytesTransferred);
}

int16_t uart_run_fullduplex_test(void)
{
    am_hal_uart_transfer_t  sTransactionTx;
#if (UART_DMA_ENABLED == 0)
    am_hal_uart_transfer_t  sTransactionRx;
#else
    am_hal_cachectrl_range_t sRange;
#endif

    g_bUartTxDone = false;
    g_bUartRxDone = false;

    // Generate tx data
    uint8_t checksum = 0;
    for ( uint32_t i = 0; i < UART_TEST_PACKET_SIZE - 1; i++ )
    {
        g_sUartFifoCfg.pui8SpaceTx[i] = i + 1;
        checksum += g_sUartFifoCfg.pui8SpaceTx[i];
    }
    g_sUartFifoCfg.pui8SpaceTx[UART_TEST_PACKET_SIZE - 1] = checksum;

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
        return -1;
    }
#if UART_DMA_ENABLED
    if ( *sTransactionTx.pui32FdRxBytesTransferred != UART_TEST_PACKET_SIZE )
    {
        // We didn't receive all of the data we sent.
        am_util_stdio_printf("\nRX data length not matched!");
        return -2;
    }
#else
    if ( *sTransactionRx.pui32BytesTransferred != UART_TEST_PACKET_SIZE )
    {
        // We didn't receive all of the data we sent.
        am_util_stdio_printf("\nRX data length not matched!");
        return -2;
    }
#endif

    if ( data_handler(g_sUartFifoCfg.pui8SpaceRx, UART_TEST_PACKET_SIZE) )
    {
        return -3;
    }

    am_util_stdio_printf(".");

    return 0;
}

void uart_pin_config(void)
{
    uint8_t uart_tx_pin;
    uint8_t uart_rx_pin;
    uint8_t uart_cts_pin;
    uint8_t uart_rts_pin;

#if defined(AM_BSP_GPIO_UART0_TX) && defined(AM_BSP_GPIO_UART0_CTS)
    if (g_sUartFifoCfg.uart_tx == 0)
    {
        am_hal_gpio_pinconfig( AM_BSP_GPIO_UART0_TX, g_AM_BSP_GPIO_UART0_TX);
        am_hal_gpio_pinconfig( AM_BSP_GPIO_UART0_CTS, g_AM_BSP_GPIO_UART0_CTS);
        uart_tx_pin = AM_BSP_GPIO_UART0_TX;
        uart_cts_pin = AM_BSP_GPIO_UART0_CTS;
    }
#endif
#if defined(AM_BSP_GPIO_UART1_TX) && defined(AM_BSP_GPIO_UART1_CTS)
    else if (g_sUartFifoCfg.uart_tx == 1)
    {
        am_hal_gpio_pinconfig( AM_BSP_GPIO_UART1_TX, g_AM_BSP_GPIO_UART1_TX);
        am_hal_gpio_pinconfig( AM_BSP_GPIO_UART1_CTS, g_AM_BSP_GPIO_UART1_CTS);
        uart_tx_pin = AM_BSP_GPIO_UART1_TX;
        uart_cts_pin = AM_BSP_GPIO_UART1_CTS;
    }
#endif
#if defined(AM_BSP_GPIO_UART2_TX) && defined(AM_BSP_GPIO_UART2_CTS)
    else if (g_sUartFifoCfg.uart_tx == 2)
    {
        am_hal_gpio_pinconfig( AM_BSP_GPIO_UART2_TX, g_AM_BSP_GPIO_UART2_TX);
        am_hal_gpio_pinconfig( AM_BSP_GPIO_UART2_CTS, g_AM_BSP_GPIO_UART2_CTS);
        uart_tx_pin = AM_BSP_GPIO_UART2_TX;
        uart_cts_pin = AM_BSP_GPIO_UART2_CTS;
    }
#endif
#if defined(AM_BSP_GPIO_UART3_TX) && defined(AM_BSP_GPIO_UART3_CTS)
    else if (g_sUartFifoCfg.uart_tx == 3)
    {
        am_hal_gpio_pinconfig( AM_BSP_GPIO_UART3_TX, g_AM_BSP_GPIO_UART3_TX);
        am_hal_gpio_pinconfig( AM_BSP_GPIO_UART3_CTS, g_AM_BSP_GPIO_UART3_CTS);
        uart_tx_pin = AM_BSP_GPIO_UART3_TX;
        uart_cts_pin = AM_BSP_GPIO_UART3_CTS;
    }
#endif

#if defined(CUST_UART0RX_PIN) && defined(CUST_UART0RTS_PIN)
    if (g_sUartFifoCfg.uart_rx == 0)
    {
        am_hal_gpio_pinconfig( CUST_UART0RX_PIN, g_AM_BSP_GPIO_UART0_RX);
        am_hal_gpio_pinconfig( CUST_UART0RTS_PIN, g_AM_BSP_GPIO_UART0_RTS);
        uart_rx_pin = CUST_UART0RX_PIN;
        uart_rts_pin = CUST_UART0RTS_PIN;
    }
#elif defined(AM_BSP_GPIO_UART0_RX) && defined(AM_BSP_GPIO_UART0_RTS)
    if (g_sUartFifoCfg.uart_rx == 0)
    {
        am_hal_gpio_pinconfig( AM_BSP_GPIO_UART0_RX, g_AM_BSP_GPIO_UART0_RX);
        am_hal_gpio_pinconfig( AM_BSP_GPIO_UART0_RTS, g_AM_BSP_GPIO_UART0_RTS);
        uart_rx_pin = AM_BSP_GPIO_UART0_RX;
        uart_rts_pin = AM_BSP_GPIO_UART0_RTS;
    }
#endif
#if defined(AM_BSP_GPIO_UART1_RX) && defined(AM_BSP_GPIO_UART1_RTS)
    else if (g_sUartFifoCfg.uart_rx == 1)
    {
        am_hal_gpio_pinconfig( AM_BSP_GPIO_UART1_RX, g_AM_BSP_GPIO_UART1_RX);
        am_hal_gpio_pinconfig( AM_BSP_GPIO_UART1_RTS, g_AM_BSP_GPIO_UART1_RTS);
        uart_rx_pin = AM_BSP_GPIO_UART1_RX;
        uart_rts_pin = AM_BSP_GPIO_UART1_RTS;
    }
#endif
#if defined(AM_BSP_GPIO_UART2_RX) && defined(AM_BSP_GPIO_UART2_RTS)
    else if (g_sUartFifoCfg.uart_rx == 2)
    {
        am_hal_gpio_pinconfig( AM_BSP_GPIO_UART2_RX, g_AM_BSP_GPIO_UART2_RX);
        am_hal_gpio_pinconfig( AM_BSP_GPIO_UART2_RTS, g_AM_BSP_GPIO_UART2_RTS);
        uart_rx_pin = AM_BSP_GPIO_UART2_RX;
        uart_rts_pin = AM_BSP_GPIO_UART2_RTS;
    }
#endif
#if defined(AM_BSP_GPIO_UART3_RX) && defined(AM_BSP_GPIO_UART3_RTS)
    else if (g_sUartFifoCfg.uart_rx == 3)
    {
        am_hal_gpio_pinconfig( AM_BSP_GPIO_UART3_RX, g_AM_BSP_GPIO_UART3_RX);
        am_hal_gpio_pinconfig( AM_BSP_GPIO_UART3_RTS, g_AM_BSP_GPIO_UART3_RTS);
        uart_rx_pin = AM_BSP_GPIO_UART3_RX;
        uart_rts_pin = AM_BSP_GPIO_UART3_RTS;
    }
#endif
    am_util_stdio_printf("The pin jumpers are as follows:\n");
    am_util_stdio_printf("\n");
    am_util_stdio_printf("\t\t+--------------------+                    +--------------------+\n");
    am_util_stdio_printf("\t\t|   Board#1 (HOST)   |                    |   Board#2 (SLAVE)  |\n");
    am_util_stdio_printf("\t\t|       UART         |                    |       UART         |\n");
    am_util_stdio_printf("\t\t|--------------------|                    |--------------------|\n");
    am_util_stdio_printf("\t\t|   %3d (TX)         |--------TX----->----|   %3d (RX)         |\n", uart_tx_pin, uart_rx_pin);
    am_util_stdio_printf("\t\t|                    |                    |                    |\n");
    am_util_stdio_printf("\t\t|   %3d (RX)         |----<---RX----------|   %3d (TX)         |\n", uart_rx_pin, uart_tx_pin);
    am_util_stdio_printf("\t\t|                    |                    |                    |\n");
    am_util_stdio_printf("\t\t|   %3d (RTS)        |--------RTS---->----|   %3d (CTS)        |\n", uart_rts_pin, uart_cts_pin);
    am_util_stdio_printf("\t\t|                    |                    |                    |\n");
    am_util_stdio_printf("\t\t|   %3d (CTS)        |----<---CTS---------|   %3d (RTS)        |\n", uart_cts_pin, uart_rts_pin);
    am_util_stdio_printf("\t\t|                    |                    |                    |\n");
    am_util_stdio_printf("\t\t|   %3d (INT)        |----<---HANDSHAKE---|   %3d (HANDSHAKE)  |\n", UART_SLAVE_READY_OUT_PIN , UART_SLAVE_READY_OUT_PIN);
    am_util_stdio_printf("\t\t|                    |                    |                    |\n");
    am_util_stdio_printf("\t\t|       GND          |-------GND----->----|       GND          |\n");
    am_util_stdio_printf("\t\t+--------------------+                    +--------------------+\n");
    am_util_stdio_printf("\n");
    am_util_stdio_printf("Power Sequence for Board#1 (HOST) and Board#2 (SLAVE):\n");
    am_util_stdio_printf("\tStep 1: Reset Board#1 (HOST) \n");
    am_util_stdio_printf("\tStep 2: Reset Board#2 (SLAVE) \n");
    am_util_stdio_printf("\n");
    am_util_stdio_printf("Note 1: Host and slave need to use the same compiler %s\n", COMPILER_VERSION);
    am_util_stdio_printf("Note 2: Switch the IOX mode to 10 if use apollo5b_eb_revb \n");
    am_util_stdio_printf("\n");
}

void uart_init(void)
{
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
    am_util_stdio_printf("UART transmission (transmit/receive) in DMA mode \n");

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
}

