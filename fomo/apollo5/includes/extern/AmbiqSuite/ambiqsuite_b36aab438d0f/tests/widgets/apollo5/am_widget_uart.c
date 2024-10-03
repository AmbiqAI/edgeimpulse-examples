//*****************************************************************************
//
//! @file am_widget_uart.c
//!
//! @brief This widget allows test cases to exercise the Apollo5 UART HAL
//! using TX and RX loopback on a UART module.
//!
//!
//! @verbatim
//!
//! The loopback pin configuration for Apollo5a_eb is:
//!     UART0
//!     -----
//!     GPIO[0]-TX
//!     GPIO[2]-RX
//!
//! The loopback pin configuration for other boards will default to using
//! the BSP UART_COM pins (board-specific)
//!
//! @endverbatim
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
#include "am_widget_uart.h"
#include "am_bsp.h"
#include "am_util.h"
#include <string.h>
#include "unity.h"

//*****************************************************************************
//
// Look for errors in the HAL.
//
//*****************************************************************************
#if 0

#define CHECK_STATUS(x, expected)                                             \
    {                                                                         \
        g_ui32ErrorStatus = (x);                                              \
        if (g_ui32ErrorStatus != (expected))                                  \
        {                                                                     \
            am_util_stdio_printf("ERROR: status 0x%02x in %s, L%d\n",         \
                                g_ui32ErrorStatus, __FILE__, __LINE__);       \
                                                                              \
            am_util_stdio_printf(" ...expected: 0x%02x\n", (expected));       \
                                                                              \
            return 1;                                                         \
        }                                                                     \
    }

#else

#define CHECK_STATUS(x, expected)                                             \
    {                                                                         \
        g_ui32ErrorStatus = (x);                                              \
        if (g_ui32ErrorStatus != (expected))                                  \
        {                                                                     \
            return 1;                                                         \
        }                                                                     \
    }

#endif

#define CHECK_PASS(x)   CHECK_STATUS(x, AM_HAL_STATUS_SUCCESS)

#define HW_FLOW_CTRL    0

//*****************************************************************************
//
// Tracking variable for error status.
//
//*****************************************************************************
uint32_t g_ui32ErrorStatus = 0;

//*****************************************************************************
//
// State variables for the UART.
//
//*****************************************************************************
void *g_hUART;
void *g_hUART1;

//uint8_t pui8TxBuffer[256];
//uint8_t pui8RxBuffer[256];

am_hal_uart_config_t sUartConfig =
{
#if defined APOLLO5_FPGA
    .ui32BaudRate = 2900000,
#elif defined(AM_PART_APOLLO5A)
    .ui32BaudRate = 2900000,
#else
    .ui32BaudRate = 3000000,
#endif
    .eDataBits = AM_HAL_UART_DATA_BITS_8,
    .eParity = AM_HAL_UART_PARITY_NONE,
    .eStopBits = AM_HAL_UART_TWO_STOP_BITS,
#if HW_FLOW_CTRL
    .eFlowControl = AM_HAL_UART_FLOW_CTRL_RTS_CTS,
#else
    .eFlowControl = AM_HAL_UART_FLOW_CTRL_NONE,
#endif
    .eTXFifoLevel = AM_HAL_UART_FIFO_LEVEL_28,
    .eRXFifoLevel = AM_HAL_UART_FIFO_LEVEL_28
};

am_hal_uart_transfer_t  sTransaction;

volatile uint32_t ui32TXDoneFlag = false;

volatile uint32_t ui32RXDoneFlag = false;

am_hal_uart_transfer_t sUartTXTrahsfer =
{
    .eDirection = AM_HAL_UART_TX,
    .ui8Priority = 0,
    .ui32NumBytes = DMA_BYTES,
    .pui32TxBuffer = NULL,
};

am_hal_uart_transfer_t sUartRXTrahsfer =
{
    .eDirection = AM_HAL_UART_RX,
    .ui8Priority = 0,
    .ui32NumBytes = DMA_BYTES,
    .pui32RxBuffer = NULL,
};

uint8_t g_TXModule = 0;
uint8_t g_RXModule = 0;

bool g_UseHFRC = false;

//*****************************************************************************
//
// Working space for test data.
//
//*****************************************************************************

//*****************************************************************************
//
// Helper functions.
//
//*****************************************************************************
static uint32_t buffer_check(void *pv1, void *pv2, uint32_t size);

//*****************************************************************************
//
// UART0 interrupt handler.
//
//*****************************************************************************
void uart_isr_service(uint8_t module)
{
    //
    // Service the FIFOs as necessary, and clear the interrupts.
    //
    uint32_t ui32Status;

    if((g_TXModule != g_RXModule) && (g_RXModule == module))
    {
        am_hal_uart_interrupt_status_get(g_hUART1, &ui32Status, true);
        am_hal_uart_interrupt_clear(g_hUART1, ui32Status);
        am_hal_uart_interrupt_service(g_hUART1, ui32Status);
        if(ui32Status & AM_HAL_UART_INT_DMACPRIS)
        {
            ui32RXDoneFlag = true;
        }
    }
    else
    {
        am_hal_uart_interrupt_status_get(g_hUART, &ui32Status, true);
        am_hal_uart_interrupt_clear(g_hUART, ui32Status);
        am_hal_uart_interrupt_service(g_hUART, ui32Status);
#if defined(TEST_UART_DMA)
        if(ui32Status & AM_HAL_UART_INT_DMACPRIS)
        {
            if(!ui32TXDoneFlag)
            {
                ui32TXDoneFlag = true;
            }
            else
            {
                ui32RXDoneFlag = true;
            }
        }
#else
        ui32TXDoneFlag = true;
#endif
    }
}

void
am_uart_isr(void)
{
    uart_isr_service(0);
}

void
am_uart1_isr(void)
{
    uart_isr_service(1);
}

void
am_uart2_isr(void)
{
    uart_isr_service(2);
}

void
am_uart3_isr(void)
{
    uart_isr_service(3);
}

void
reconfig_baudrate(uint32_t ui32Module)
{
    uint64_t ui64FractionDivisorLong;
    uint64_t ui64IntermediateLong;
    uint32_t ui32IntegerDivisor;
    uint32_t ui32FractionDivisor;
    uint32_t ui32BaudClk;
    uint32_t ui32UartClkFreq = 48000000;

    //
    // Calculate register values.
    //
    ui32BaudClk = 16 * 3000000;
    ui32IntegerDivisor = (uint32_t)(ui32UartClkFreq / ui32BaudClk);
    ui64IntermediateLong = (ui32UartClkFreq * 64) / ui32BaudClk; // Q58.6
    ui64FractionDivisorLong = ui64IntermediateLong - (ui32IntegerDivisor * 64); // Q58.6
    ui32FractionDivisor = (uint32_t)ui64FractionDivisorLong;

    //
    // Write the UART regs.
    //
    UARTn(ui32Module)->IBRD = ui32IntegerDivisor;
    UARTn(ui32Module)->FBRD = ui32FractionDivisor;
}

//*****************************************************************************
//
// Initialized UART loopback test.
//
//*****************************************************************************
void
am_widget_uart_loopback_init(am_widget_uart_loopback_config_t *psConfigloopback, uint8_t blocking,
                                         uint8_t enableQueue, uint32_t length)
{
    int   i;
    g_TXModule = psConfigloopback->uart_tx;
    g_RXModule = psConfigloopback->uart_rx;
    psConfigloopback->length = length;

    psConfigloopback->txblocking = blocking;
    psConfigloopback->rxblocking = blocking;

    psConfigloopback->txbufferSize = psConfigloopback->length;
    psConfigloopback->rxbufferSize = psConfigloopback->length;

    psConfigloopback->txQueueEnable = enableQueue;
    psConfigloopback->rxQueueEnable = enableQueue;

    psConfigloopback->txBytesTransferred = 0;
    psConfigloopback->rxBytesTransferred = 0;

    for ( i = 0; i < psConfigloopback->txbufferSize; i++ )
    {
        psConfigloopback->pui8SpaceTx[i] = i+1;
        psConfigloopback->pui8SpaceRx[i] = 0;
    }

    psConfigloopback->result = 0;
}

//*****************************************************************************
//
// Send data with the UART, and retreive it with the same UART.
//
//*****************************************************************************
uint32_t
am_widget_uart_loopback(am_widget_uart_loopback_config_t *psConfig)
{
    int TxQueueLength, RxQueueLength;
    am_util_stdio_printf("\nTX: UART%d, RX: UART%d\n", psConfig->uart_tx, psConfig->uart_rx);
    //
    // Enable UART pins.
    //
    switch ( psConfig->uart_tx )
    {
#if defined(AM_BSP_GPIO_UART0_TX)
        case 0:
            am_hal_gpio_pinconfig( AM_BSP_GPIO_UART0_TX, g_AM_BSP_GPIO_UART0_TX);
            am_hal_gpio_pinconfig( AM_BSP_GPIO_UART0_CTS, g_AM_BSP_GPIO_UART0_CTS);
            break;
#endif
#if defined(AM_BSP_GPIO_UART1_TX)
        case 1:
            am_hal_gpio_pinconfig( AM_BSP_GPIO_UART1_TX, g_AM_BSP_GPIO_UART1_TX);
            am_hal_gpio_pinconfig( AM_BSP_GPIO_UART1_CTS, g_AM_BSP_GPIO_UART1_CTS);
            break;
#endif
#if defined(AM_BSP_GPIO_UART2_TX)
        case 2:
            am_hal_gpio_pinconfig( AM_BSP_GPIO_UART2_TX, g_AM_BSP_GPIO_UART2_TX);
            break;
#endif
#if defined(AM_BSP_GPIO_UART3_TX)
        case 3:
            am_hal_gpio_pinconfig( AM_BSP_GPIO_UART3_TX, g_AM_BSP_GPIO_UART3_TX);
            break;
#endif
        default:
            TEST_FAIL_MESSAGE("Please check the UART pin of the BSP.");
            break;
    }
    switch ( psConfig->uart_rx )
    {
#if defined(AM_BSP_GPIO_UART0_RX)
        case 0:
            am_hal_gpio_pinconfig( AM_BSP_GPIO_UART0_RX, g_AM_BSP_GPIO_UART0_RX);
            am_hal_gpio_pinconfig( AM_BSP_GPIO_UART0_RTS, g_AM_BSP_GPIO_UART0_RTS);
            break;
#endif
#if defined(AM_BSP_GPIO_UART1_RX)
        case 1:
            am_hal_gpio_pinconfig( AM_BSP_GPIO_UART1_RX, g_AM_BSP_GPIO_UART1_RX);
            am_hal_gpio_pinconfig( AM_BSP_GPIO_UART1_RTS, g_AM_BSP_GPIO_UART1_RTS);
            break;
#endif
#if defined(AM_BSP_GPIO_UART2_RX)
        case 2:
            am_hal_gpio_pinconfig( AM_BSP_GPIO_UART2_RX, g_AM_BSP_GPIO_UART2_RX);
            break;
#endif
#if defined(AM_BSP_GPIO_UART3_RX)
        case 3:
            am_hal_gpio_pinconfig( AM_BSP_GPIO_UART3_RX, g_AM_BSP_GPIO_UART3_RX);
            break;
#endif
        default:
            TEST_FAIL_MESSAGE("Please check the UART pin of the BSP.");
            break;
    }

    //
    // Enable the UART.
    //
    CHECK_PASS(am_hal_uart_initialize( psConfig->uart_tx, &g_hUART ) );
    CHECK_PASS(am_hal_uart_power_control(g_hUART, AM_HAL_SYSCTRL_WAKE, false));
    sUartConfig.eStopBits = AM_HAL_UART_TWO_STOP_BITS;
    CHECK_PASS(am_hal_uart_configure(g_hUART, &sUartConfig));
#if !defined(AM_PART_APOLLO5A)
#if defined(AM_PART_APOLLO5B)
    if((APOLLO5_GE_B1) && (g_UseHFRC))
#elif defined(AM_PART_APOLLO510L)
    if(g_UseHFRC)
#endif
    {
        am_util_stdio_printf("\nUse HFRC as clock source\n");
        UARTn(psConfig->uart_tx)->CR_b.CLKSEL = UART0_CR_CLKSEL_HFRC_48MHZ;
        reconfig_baudrate(psConfig->uart_tx);
        am_hal_clkmgr_clock_release(AM_HAL_CLKMGR_CLK_ID_SYSPLL, (AM_HAL_CLKMGR_USER_ID_UART0 + psConfig->uart_tx));
        am_hal_clkmgr_clock_request(AM_HAL_CLKMGR_CLK_ID_HFRC, (AM_HAL_CLKMGR_USER_ID_UART0 + psConfig->uart_tx));
    }
    else
    {
        am_util_stdio_printf("\nUse SYSPLL as clock source\n");
    }
#endif
    if(psConfig->uart_tx == psConfig->uart_rx)
    {
        am_hal_uart_interrupt_enable(g_hUART, (AM_HAL_UART_INT_TX |
                                               AM_HAL_UART_INT_RX |
                                               AM_HAL_UART_INT_RX_TMOUT |
                                               AM_HAL_UART_INT_OVER_RUN |
                                               AM_HAL_UART_INT_TXCMP));
    }
    else
    {
        CHECK_PASS(am_hal_uart_initialize( psConfig->uart_rx, &g_hUART1 ) );
        CHECK_PASS(am_hal_uart_power_control(g_hUART1, AM_HAL_SYSCTRL_WAKE, false));
        sUartConfig.eStopBits = AM_HAL_UART_ONE_STOP_BIT;
        CHECK_PASS(am_hal_uart_configure(g_hUART1, &sUartConfig));
#if !defined(AM_PART_APOLLO5A)
#if defined(AM_PART_APOLLO5B)
        if((APOLLO5_GE_B1) && (g_UseHFRC))
#elif defined(AM_PART_APOLLO510L)
        if(g_UseHFRC)
#endif
        {
            UARTn(psConfig->uart_rx)->CR_b.CLKSEL = UART0_CR_CLKSEL_HFRC_48MHZ;
            reconfig_baudrate(psConfig->uart_rx);
            am_hal_clkmgr_clock_release(AM_HAL_CLKMGR_CLK_ID_SYSPLL, (AM_HAL_CLKMGR_USER_ID_UART0 + psConfig->uart_rx));
            am_hal_clkmgr_clock_request(AM_HAL_CLKMGR_CLK_ID_HFRC, (AM_HAL_CLKMGR_USER_ID_UART0 + psConfig->uart_rx));
        }
#endif
        am_hal_uart_interrupt_enable(g_hUART, (AM_HAL_UART_INT_TX |
                                               AM_HAL_UART_INT_OVER_RUN |
                                               AM_HAL_UART_INT_TXCMP));
        am_hal_uart_interrupt_enable(g_hUART1, (AM_HAL_UART_INT_RX |
                                               AM_HAL_UART_INT_RX_TMOUT |
                                               AM_HAL_UART_INT_OVER_RUN));
    }

    if (psConfig->txQueueEnable)
    {
        TxQueueLength = sizeof(psConfig->pui8QueueTx);
    }
    else
    {
        TxQueueLength = 0;
    }

    if (psConfig->rxQueueEnable)
    {
        RxQueueLength = sizeof(psConfig->pui8QueueRx);
    }
    else
    {
        RxQueueLength = 0;
    }
    if(psConfig->uart_tx == psConfig->uart_rx)
    {
        am_hal_uart_buffer_configure(g_hUART, psConfig->pui8QueueTx, TxQueueLength, psConfig->pui8QueueRx, RxQueueLength);
    }
    else
    {
        am_hal_uart_buffer_configure(g_hUART, psConfig->pui8QueueTx, TxQueueLength, NULL, 0);
        am_hal_uart_buffer_configure(g_hUART1, NULL, 0, psConfig->pui8QueueRx, RxQueueLength);
        NVIC_EnableIRQ((IRQn_Type)(UART0_IRQn + psConfig->uart_rx));
    }
    NVIC_EnableIRQ((IRQn_Type)(UART0_IRQn + psConfig->uart_tx));
    am_hal_interrupt_master_enable();

    //
    // Generate some test data.
    //
    ui32TXDoneFlag = false;

    // Data location to use for this transaction.
    sTransaction.pui8Data = psConfig->pui8SpaceTx;
    // How many bytes should we send?
    sTransaction.ui32NumBytes = psConfig->txbufferSize;
    // When the transaction is complete, this will be set to the number of
    // bytes we read.
    sTransaction.pui32BytesTransferred = &psConfig->txBytesTransferred;
    // For blocking transactions, this determines how long the UART HAL should
    // wait before aborting the transaction.
    if (psConfig->txblocking)
    {
        // Is this a write or a read?
        sTransaction.eType = AM_HAL_UART_BLOCKING_WRITE;
        sTransaction.ui32TimeoutMs = AM_HAL_UART_WAIT_FOREVER;
    }
    else
    {
        // Is this a write or a read?
        sTransaction.eType = AM_HAL_UART_NONBLOCKING_WRITE;
        sTransaction.ui32TimeoutMs = 0;
    }
    // For non-blocking transfers, the UART HAL will call this callback
    // function as soon as the requested action is complete.
    //void (*pfnCallback)(uint32_t ui32ErrorStatus, void *pvContext);
    sTransaction.pfnCallback = NULL;
    // This context variable will be saved and provided to the callback
    // function when it is called.
    //sTransaction.pvContext;
    // This context variable will be saved and provided to the callback
    // function when it is called.
    sTransaction.ui32ErrorStatus=0;
    CHECK_PASS(am_hal_uart_transfer(g_hUART, &sTransaction));
    am_hal_uart_tx_flush(g_hUART);

    if ( *sTransaction.pui32BytesTransferred != psConfig->length  )
    {
        // We didn't send all of the data we were supposed to.
        return 2;
    }
    while (ui32TXDoneFlag == false);// && AM_BFRn(g_hUART, 0, FR, BUSY) == true);

    sTransaction.pui8Data = psConfig->pui8SpaceRx;
    // How many bytes should we send?
    sTransaction.ui32NumBytes = psConfig->rxbufferSize;
    // When the transaction is complete, this will be set to the number of
    // bytes we read.
    sTransaction.pui32BytesTransferred = &psConfig->rxBytesTransferred;
    // For blocking transactions, this determines how long the UART HAL should
    // wait before aborting the transaction.
    if (psConfig->rxblocking)
    {
        // Is this a write or a read?
        sTransaction.eType = AM_HAL_UART_BLOCKING_READ;
        sTransaction.ui32TimeoutMs = AM_HAL_UART_WAIT_FOREVER;
    }
    else
    {
        // Is this a write or a read?
        sTransaction.eType = AM_HAL_UART_NONBLOCKING_READ;
        sTransaction.ui32TimeoutMs = 0;
    }

    if(psConfig->uart_tx == psConfig->uart_rx)
    {
        CHECK_PASS(am_hal_uart_transfer(g_hUART, &sTransaction));
    }
    else
    {
        CHECK_PASS(am_hal_uart_transfer(g_hUART1, &sTransaction));
    }

    if (*sTransaction.pui32BytesTransferred != psConfig->length)
    {
        //
        // We didn't receive all of the data we sent.
        //
        //am_util_stdio_printf("ERROR: sent %d, received %d\n", ui32DataSent, ui32DataReceived);
        return 3;
    }

    //
    // Check for errors.
    //
    if (buffer_check(psConfig->pui8SpaceTx, psConfig->pui8SpaceRx, psConfig->length))
    {
        return 4;
    }
    //
    // Shut everything back down.
    //
    CHECK_PASS(am_hal_uart_power_control(g_hUART, AM_HAL_SYSCTRL_DEEPSLEEP, false));
    CHECK_PASS(am_hal_uart_deinitialize(g_hUART));
    g_hUART = NULL;
    if(psConfig->uart_tx != psConfig->uart_rx)
    {
        CHECK_PASS(am_hal_uart_power_control(g_hUART1, AM_HAL_SYSCTRL_DEEPSLEEP, false));
        CHECK_PASS(am_hal_uart_deinitialize(g_hUART1));
        g_hUART1 = NULL;
    }

    am_util_stdio_printf("\nPASS\n");
    return 0;
}

//*****************************************************************************
//
// Initialized UART loopback test.
//
//*****************************************************************************
void
am_widget_uart_dma_init(am_widget_uart_dma_config_t *psConfigdma)
{
    g_TXModule = psConfigdma->uart_tx;
    g_RXModule = psConfigdma->uart_rx;

    memset(psConfigdma->pui8DMABuffer, 0, DMA_BYTES * 2);

    for ( int i = 0; i < DMA_BYTES; i++ )
    {
        psConfigdma->pui8DMABuffer[i] = i;
    }
}

uint32_t
am_widget_uart_dma_transfer(am_widget_uart_dma_config_t *psConfig)
{
    am_util_stdio_printf("\nTX: UART%d, RX: UART%d\n", psConfig->uart_tx, psConfig->uart_rx);

    //
    // Enable UART pins.
    //
    switch ( psConfig->uart_tx )
    {
#if defined(AM_BSP_GPIO_UART0_TX)
        case 0:
            am_hal_gpio_pinconfig( AM_BSP_GPIO_UART0_TX, g_AM_BSP_GPIO_UART0_TX);
            am_hal_gpio_pinconfig( AM_BSP_GPIO_UART0_CTS, g_AM_BSP_GPIO_UART0_CTS);
            break;
#endif
#if defined(AM_BSP_GPIO_UART1_TX)
        case 1:
            am_hal_gpio_pinconfig( AM_BSP_GPIO_UART1_TX, g_AM_BSP_GPIO_UART1_TX);
            am_hal_gpio_pinconfig( AM_BSP_GPIO_UART1_CTS, g_AM_BSP_GPIO_UART1_CTS);
            break;
#endif
#if defined(AM_BSP_GPIO_UART2_TX)
        case 2:
            am_hal_gpio_pinconfig( AM_BSP_GPIO_UART2_TX, g_AM_BSP_GPIO_UART2_TX);
            break;
#endif
#if defined(AM_BSP_GPIO_UART3_TX)
        case 3:
            am_hal_gpio_pinconfig( AM_BSP_GPIO_UART3_TX, g_AM_BSP_GPIO_UART3_TX);
            break;
#endif
        default:
            TEST_FAIL_MESSAGE("Please check the UART pin of the BSP.");

            break;
    }
    switch ( psConfig->uart_rx )
    {
#if defined(AM_BSP_GPIO_UART0_RX)
        case 0:
            am_hal_gpio_pinconfig( AM_BSP_GPIO_UART0_RX, g_AM_BSP_GPIO_UART0_RX);
            am_hal_gpio_pinconfig( AM_BSP_GPIO_UART0_RTS, g_AM_BSP_GPIO_UART0_RTS);
            break;
#endif
#if defined(AM_BSP_GPIO_UART1_RX)
        case 1:
            am_hal_gpio_pinconfig( AM_BSP_GPIO_UART1_RX, g_AM_BSP_GPIO_UART1_RX);
            am_hal_gpio_pinconfig( AM_BSP_GPIO_UART1_RTS, g_AM_BSP_GPIO_UART1_RTS);
            break;
#endif
#if defined(AM_BSP_GPIO_UART2_RX)
        case 2:
            am_hal_gpio_pinconfig( AM_BSP_GPIO_UART2_RX, g_AM_BSP_GPIO_UART2_RX);
            break;
#endif
#if defined(AM_BSP_GPIO_UART3_RX)
        case 3:
            am_hal_gpio_pinconfig( AM_BSP_GPIO_UART3_RX, g_AM_BSP_GPIO_UART3_RX);
            break;
#endif
        default:
            TEST_FAIL_MESSAGE("Please check the UART pin of the BSP.");

            break;
    }

    //
    // Enable the UART.
    //
    CHECK_PASS(am_hal_uart_initialize( psConfig->uart_tx, &g_hUART ) );
    CHECK_PASS(am_hal_uart_power_control(g_hUART, AM_HAL_SYSCTRL_WAKE, false));
    sUartConfig.eStopBits = AM_HAL_UART_TWO_STOP_BITS;
    CHECK_PASS(am_hal_uart_configure(g_hUART, &sUartConfig));

    if(psConfig->uart_tx != psConfig->uart_rx)
    {
        CHECK_PASS(am_hal_uart_initialize( psConfig->uart_rx, &g_hUART1 ) );
        CHECK_PASS(am_hal_uart_power_control(g_hUART1, AM_HAL_SYSCTRL_WAKE, false));
        sUartConfig.eStopBits = AM_HAL_UART_ONE_STOP_BIT;
        CHECK_PASS(am_hal_uart_configure(g_hUART1, &sUartConfig));
        sUartTXTrahsfer.ui32NumBytes = DMA_BYTES;
        sUartRXTrahsfer.ui32NumBytes = DMA_BYTES;
    }
    else
    {
        sUartTXTrahsfer.ui32NumBytes = 32;
        sUartRXTrahsfer.ui32NumBytes = 32;
    }
    uint32_t ui32UARTDataPtr = (uint32_t)((uint32_t*)&psConfig->pui8DMABuffer[0]);
    sUartTXTrahsfer.pui32TxBuffer = (uint32_t*)ui32UARTDataPtr;
    sUartRXTrahsfer.pui32RxBuffer = (uint32_t*)(ui32UARTDataPtr + sUartTXTrahsfer.ui32NumBytes);

    ui32TXDoneFlag = false;
    ui32RXDoneFlag = false;

    if(psConfig->bDMA_enable)
    {
        if(psConfig->uart_tx == psConfig->uart_rx)
        {
            am_hal_uart_interrupt_enable(g_hUART, (AM_HAL_UART_INT_OVER_RUN |
                                                   AM_HAL_UART_INT_RX_TMOUT |
                                                   AM_HAL_UART_INT_DMAERIS  |
                                                   AM_HAL_UART_INT_DMACPRIS));
        }
        else
        {
            am_hal_uart_interrupt_enable(g_hUART, (AM_HAL_UART_INT_OVER_RUN |
                                                   AM_HAL_UART_INT_DMAERIS  |
                                                   AM_HAL_UART_INT_DMACPRIS));


            am_hal_uart_interrupt_enable(g_hUART1, (AM_HAL_UART_INT_RX_TMOUT |
                                                   AM_HAL_UART_INT_OVER_RUN |
                                                   AM_HAL_UART_INT_DMAERIS  |
                                                   AM_HAL_UART_INT_DMACPRIS));

            NVIC_EnableIRQ((IRQn_Type)(UART0_IRQn + psConfig->uart_rx));
        }
        NVIC_EnableIRQ((IRQn_Type)(UART0_IRQn + psConfig->uart_tx));
        am_hal_interrupt_master_enable();

        CHECK_PASS(am_hal_uart_dma_transfer(g_hUART, &sUartTXTrahsfer));
        if(psConfig->uart_tx == psConfig->uart_rx)
        {
            while (ui32TXDoneFlag == false) {};
            CHECK_PASS(am_hal_uart_dma_transfer(g_hUART, &sUartRXTrahsfer));
        }
        else
        {
            CHECK_PASS(am_hal_uart_dma_transfer(g_hUART1, &sUartRXTrahsfer));
        }
        while (ui32TXDoneFlag == false || ui32RXDoneFlag == false) {};
    }
    else
    {
        //TODO
    }

    //
    // Shut everything back down.
    //
    CHECK_PASS(am_hal_uart_power_control(g_hUART, AM_HAL_SYSCTRL_DEEPSLEEP, false));
    CHECK_PASS(am_hal_uart_deinitialize(g_hUART));
    g_hUART = NULL;
    if(psConfig->uart_tx != psConfig->uart_rx)
    {
        CHECK_PASS(am_hal_uart_power_control(g_hUART1, AM_HAL_SYSCTRL_DEEPSLEEP, false));
        CHECK_PASS(am_hal_uart_deinitialize(g_hUART1));
        g_hUART1 = NULL;
    }
    if (buffer_check(psConfig->pui8DMABuffer, psConfig->pui8DMABuffer + sUartTXTrahsfer.ui32NumBytes, sUartTXTrahsfer.ui32NumBytes))
    {
        return 1;
    }
    am_util_stdio_printf("\nPASS\n");
    return 0;
}

//*****************************************************************************
//
// Compare two buffers.
//
//*****************************************************************************
uint32_t
buffer_check(void *pv1, void *pv2, uint32_t size)
{
    uint8_t *p1 = pv1;
    uint8_t *p2 = pv2;

    for (uint32_t i = 0; i < size; i++)
    {
        if (p1[i] != p2[i])
        {
            return 1;
        }
    }

    return 0;
}

