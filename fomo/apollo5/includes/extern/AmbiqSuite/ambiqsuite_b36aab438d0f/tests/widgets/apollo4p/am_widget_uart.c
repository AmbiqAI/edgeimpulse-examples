//*****************************************************************************
//
//! @file am_widget_uart.c
//!
//! @brief This widget allows test cases to exercise the Apollo4 UART HAL
//! using TX and RX loopback on a UART module.
//!
//!
//! @verbatim
//!
//! The loopback pin configuration for STMBv2 is:
//!     UART0
//!     -----
//!     GPIO[53]-TX
//!     GPIO[55]-RX
//!     
//!     Switch configuration to loopback TX/RX:
//!     SW13: PDM_ETM_MUX_SEL: PDM (3.3V) 
//!     SW28: PDM_LOOPBACK_SEL: 0-3 1-2 (3.3V) 
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

#if defined(apollo4p_bga_sys_test_rev2)
#define UART_LB_MODULE 0 // STMBv2 has muxes to loop back UART0TX/RX
#else
#define UART_LB_MODULE AM_BSP_UART_PRINT_INST // original test uses UART_COM
#endif

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

    am_hal_uart_interrupt_status_get(g_hUART, &ui32Status, true);
    am_hal_uart_interrupt_clear(g_hUART, ui32Status);
    am_hal_uart_interrupt_service(g_hUART, ui32Status);

    ui32TXDoneFlag = true;
}

void
am_uart1_isr(void)
{
    //
    // Service the FIFOs as necessary, and clear the interrupts.
    //
    uint32_t ui32Status;

    am_hal_uart_interrupt_status_get(g_hUART, &ui32Status, true);
    am_hal_uart_interrupt_clear(g_hUART, ui32Status);
    am_hal_uart_interrupt_service(g_hUART, ui32Status);

    ui32TXDoneFlag = true;
}

void
am_uart2_isr(void)
{
    //
    // Service the FIFOs as necessary, and clear the interrupts.
    //
    uint32_t ui32Status;

    am_hal_uart_interrupt_status_get(g_hUART, &ui32Status, true);
    am_hal_uart_interrupt_clear(g_hUART, ui32Status);
    am_hal_uart_interrupt_service(g_hUART, ui32Status);

    ui32TXDoneFlag = true;
}

void
am_uart3_isr(void)
{
    //
    // Service the FIFOs as necessary, and clear the interrupts.
    //
    uint32_t ui32Status;

    am_hal_uart_interrupt_status_get(g_hUART, &ui32Status, true);
    am_hal_uart_interrupt_clear(g_hUART, ui32Status);
    am_hal_uart_interrupt_service(g_hUART, ui32Status);

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
    psConfigloopback->uart = UART_LB_MODULE;    // UART COM is not always module 0
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
    psConfigloopback->uart = UART_LB_MODULE;    // UART COM is not always module 0
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
    // AM_BSP_GPIO_UART1_TX for UART1
    // AM_BSP_GPIO_COM_UART_TX  for UART0 (AM_BSP_UART_PRINT_INST)
    if ( psConfig->uart == AM_BSP_UART_PRINT_INST ) // UART COM
    {
        //FIXME: Eventually this should be a BSP call.
        am_hal_gpio_pinconfig( AM_BSP_GPIO_COM_UART_TX, g_AM_BSP_GPIO_COM_UART_TX);
        am_hal_gpio_pinconfig( AM_BSP_GPIO_COM_UART_RX, g_AM_BSP_GPIO_COM_UART_RX);
    }
    else
    {
        switch ( psConfig->uart )
        {
#if defined(AM_BSP_GPIO_UART0_TX) && defined(AM_BSP_GPIO_UART0_RX)
            case 0:
                am_hal_gpio_pinconfig( AM_BSP_GPIO_UART0_TX, g_AM_BSP_GPIO_UART0_TX);
                am_hal_gpio_pinconfig( AM_BSP_GPIO_UART0_RX, g_AM_BSP_GPIO_UART0_RX);
                break;
#endif
#if defined(AM_BSP_GPIO_UART1_TX) && defined(AM_BSP_GPIO_UART1_RX)
            case 1:
                am_hal_gpio_pinconfig( AM_BSP_GPIO_UART1_TX, g_AM_BSP_GPIO_UART1_TX);
                am_hal_gpio_pinconfig( AM_BSP_GPIO_UART1_RX, g_AM_BSP_GPIO_UART1_RX);
                break;
#endif
#if defined(AM_BSP_GPIO_UART2_TX) && defined(AM_BSP_GPIO_UART2_RX)
            case 2:
                am_hal_gpio_pinconfig( AM_BSP_GPIO_UART2_TX, g_AM_BSP_GPIO_UART2_TX);
                am_hal_gpio_pinconfig( AM_BSP_GPIO_UART2_RX, g_AM_BSP_GPIO_UART2_RX);
                break;
#endif
#if defined(AM_BSP_GPIO_UART3_TX) && defined(AM_BSP_GPIO_UART3_RX)
            case 3:
                am_hal_gpio_pinconfig( AM_BSP_GPIO_UART3_TX, g_AM_BSP_GPIO_UART3_TX);
                am_hal_gpio_pinconfig( AM_BSP_GPIO_UART3_RX, g_AM_BSP_GPIO_UART3_RX);
                break;
#endif
            default:
                break;
        }
    }


    //
    // Enable the UART.
    //
    CHECK_PASS(am_hal_uart_initialize( psConfig->uart, &g_hUART ) );
    CHECK_PASS(am_hal_uart_power_control(g_hUART, AM_HAL_SYSCTRL_WAKE, false));

    CHECK_PASS(am_hal_uart_configure(g_hUART, &sUartConfig));

    am_hal_uart_interrupt_enable(g_hUART, (AM_HAL_UART_INT_TX |
                                           AM_HAL_UART_INT_RX |
                                           AM_HAL_UART_INT_RX_TMOUT |
                                           AM_HAL_UART_INT_OVER_RUN |
                                           AM_HAL_UART_INT_TXCMP));


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
    am_hal_uart_buffer_configure(g_hUART, psConfig->pui8QueueTx, TxQueueLength, psConfig->pui8QueueRx, RxQueueLength);

    NVIC_EnableIRQ((IRQn_Type)(UART0_IRQn + psConfig->uart));
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

    CHECK_PASS(am_hal_uart_transfer(g_hUART, &sTransaction));

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
