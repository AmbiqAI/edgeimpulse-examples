//*****************************************************************************
//
//! @file am_widget_uart.c
//!
//! @brief This widget allows test cases to exercise the Apollo2 UART HAL
//! using loopback between UART #0 and #1.
//!
//!
//! @verbatim
//! The Pin configuration is as follows:
//!     UART0
//!     -----
//!     GPIO[34]-TX
//!     GPIO[36]-RX 
//!
//!     UART0
//!     -----
//!     GPIO[12]-TX
//!     GPIO[14]-RX
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
void *UART;

//uint8_t pui8TxBuffer[256];
//uint8_t pui8RxBuffer[256];

am_hal_uart_config_t sUartConfig =
{
    .ui32BaudRate = 115200,
    .eDataBits = AM_HAL_UART_DATA_BITS_8,
    .eParity = AM_HAL_UART_PARITY_NONE,
    .eStopBits = AM_HAL_UART_ONE_STOP_BIT,
    .eFlowControl = AM_HAL_UART_FLOW_CTRL_NONE,
    .eTXFifoLevel = AM_HAL_UART_FIFO_LEVEL_28,
    .eRXFifoLevel = AM_HAL_UART_FIFO_LEVEL_28
};

am_hal_uart_transfer_t  sTransaction;

volatile uint32_t ui32TXDoneFlag = false;

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
void
am_uart_isr(void)
{
    //
    // Service the FIFOs as necessary, and clear the interrupts.
    //
    uint32_t ui32Status;

    am_hal_uart_interrupt_status_get(UART, &ui32Status, true);
    am_hal_uart_interrupt_clear(UART, ui32Status);
    am_hal_uart_interrupt_service(UART, ui32Status);

    ui32TXDoneFlag = true;
}

void
am_uart1_isr(void)
{
    //
    // Service the FIFOs as necessary, and clear the interrupts.
    //
    uint32_t ui32Status;

    am_hal_uart_interrupt_status_get(UART, &ui32Status, true);
    am_hal_uart_interrupt_clear(UART, ui32Status);
    am_hal_uart_interrupt_service(UART, ui32Status);

    ui32TXDoneFlag = true;
}


//*****************************************************************************
//
// Initialized UART loopback test.
//
//*****************************************************************************
void
am_widget_uart_loopback_nonblocking_init(am_widget_uart_loopback_config_t   *psConfigloopback,
                                         uint8_t enableQueue , uint32_t length)
{
    int   i;
    psConfigloopback->uart = 0;
    psConfigloopback->length = length;

    psConfigloopback->txblocking = 0;
    psConfigloopback->rxblocking = 0;

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
// Initialized UART loopback test.
//
//*****************************************************************************
void
am_widget_uart_loopback_blocking_init(am_widget_uart_loopback_config_t   *psConfigloopback,
                                      uint8_t enableQueue, uint32_t length)
{
    int   i;
    psConfigloopback->uart = 0;
    psConfigloopback->length = length;

    psConfigloopback->txblocking = 1;
    psConfigloopback->rxblocking = 1;

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
    //
    // Enable UART pins.
    //
    // AM_BSP_GPIO_UART_TX for UART1
    // AM_BSP_GPIO_COM_UART_TX  for UART0
    if ( psConfig->uart == 0 )
    {
        //FIXME: Eventually this should be a BSP call.
        am_hal_gpio_pinconfig( AM_BSP_GPIO_COM_UART_TX, g_AM_BSP_GPIO_COM_UART_TX);
        am_hal_gpio_pinconfig( AM_BSP_GPIO_COM_UART_RX, g_AM_BSP_GPIO_COM_UART_RX);
    }
    if ( psConfig->uart == 1 )
    {
        //FIXME: Eventually this should be a BSP call.
        am_hal_gpio_pinconfig( AM_BSP_GPIO_UART_TX, g_AM_BSP_GPIO_UART_TX);
        am_hal_gpio_pinconfig( AM_BSP_GPIO_UART_RX, g_AM_BSP_GPIO_UART_RX);
    }

    //
    // Enable the UART.
    //
    CHECK_PASS(am_hal_uart_initialize( psConfig->uart, &UART ) );
    CHECK_PASS(am_hal_uart_power_control(UART, AM_HAL_SYSCTRL_WAKE, false));

    CHECK_PASS(am_hal_uart_configure(UART, &sUartConfig));

    am_hal_uart_interrupt_enable(UART, UART0_IER_TXCMPMIM_Msk);

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
    am_hal_uart_buffer_configure(UART, psConfig->pui8QueueTx, TxQueueLength, psConfig->pui8QueueRx, RxQueueLength);

    NVIC_EnableIRQ(UART0_IRQn);
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
    CHECK_PASS(am_hal_uart_transfer(UART, &sTransaction));
    am_hal_uart_tx_flush(UART);

    if ( *sTransaction.pui32BytesTransferred != psConfig->length  )
    {
        // We didn't send all of the data we were supposed to.
        return 2;
    }
    while (ui32TXDoneFlag == false);// && AM_BFRn(UART, 0, FR, BUSY) == true);


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

    CHECK_PASS(am_hal_uart_transfer(UART, &sTransaction));

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
    CHECK_PASS(am_hal_uart_power_control(UART, AM_HAL_SYSCTRL_DEEPSLEEP, false));
    CHECK_PASS(am_hal_uart_deinitialize(UART));

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
