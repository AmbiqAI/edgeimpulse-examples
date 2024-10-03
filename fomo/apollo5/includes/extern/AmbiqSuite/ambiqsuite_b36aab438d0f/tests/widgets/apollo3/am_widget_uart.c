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
//!     UART0           UART1
//!     -----           -----
//!     GPIO[1]-TX      GPIO[9]-RX
//!     GPIO[2]-RX      GPIO[8]-TX
//!     GPIO[3]-RTS     GPIO[11]-CTS
//!     GPIO[4]-CTS     GPIO[10]-RTS
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

uint8_t pui8TxBuffer[256];
uint8_t pui8RxBuffer[256];

am_hal_uart_config_t sUartConfig =
{
    .ui32BaudRate = 115200,
    .ui32DataBits = AM_HAL_UART_DATA_BITS_8,
    .ui32Parity = AM_HAL_UART_PARITY_NONE,
    .ui32StopBits = AM_HAL_UART_ONE_STOP_BIT,
    .ui32FlowControl = AM_HAL_UART_FLOW_CTRL_NONE,

    .ui32FifoLevels = (AM_HAL_UART_TX_FIFO_1_2 |
                       AM_HAL_UART_RX_FIFO_1_2),
};

volatile uint32_t ui32TXDoneFlag = false;

//*****************************************************************************
//
// Working space for test data.
//
//*****************************************************************************
uint8_t pui8Space0[1024];
uint8_t pui8Space1[1024];

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
    uint32_t ui32UartIdle;

    am_hal_uart_interrupt_status_get(UART, &ui32Status, true);
    am_hal_uart_interrupt_clear(UART, ui32Status);
    am_hal_uart_interrupt_service(UART, ui32Status, &ui32UartIdle);

    ui32TXDoneFlag = ui32UartIdle;
}

//*****************************************************************************
//
// Send data with the UART, and retreive it with the same UART.
//
//*****************************************************************************
uint32_t
am_widget_uart_loopback_clkSpd(am_widget_uart_loopback_config_t *psConfig,
                        am_hal_uart_clock_speed_e eClockSpeed )
{
    uint32_t ui32DataSent     = 0;
    uint32_t ui32DataReceived = 0;

    //
    // Enable UART pins.
    //
    am_hal_gpio_pinconfig(39, g_AM_BSP_GPIO_COM_UART_TX);
    am_hal_gpio_pinconfig(40, g_AM_BSP_GPIO_COM_UART_RX);

    //
    // Enable the UART.
    //
    CHECK_PASS(am_hal_uart_initialize(0, &UART));
    CHECK_PASS(am_hal_uart_power_control(UART, AM_HAL_SYSCTRL_WAKE, false));


    sUartConfig.pui8TxBuffer     = pui8TxBuffer;
    sUartConfig.ui32TxBufferSize = psConfig->txbuffer;
    sUartConfig.pui8RxBuffer     = pui8RxBuffer;
    sUartConfig.ui32RxBufferSize = psConfig->rxbuffer;

    //
    // set the uart clock speed
    //
    CHECK_PASS(am_hal_uart_control(UART, AM_HAL_UART_CONTROL_CLKSEL, &eClockSpeed )) ;
    CHECK_PASS(am_hal_uart_configure(UART, &sUartConfig));


    am_hal_uart_interrupt_enable(UART, UART0_IER_TXCMPMIM_Msk);

    NVIC_EnableIRQ(UART0_IRQn);
    am_hal_interrupt_master_enable();

    //
    // Generate some test data.
    //
    for (uint32_t i = 0; i < psConfig->length; i++)
    {
        pui8Space0[i] = 'a' + (i % 26);
    }

    ui32TXDoneFlag = false;


    //
    // Send and receive the data.
    //
    am_hal_uart_transfer_t sTransaction;

    sTransaction.ui32Direction         = AM_HAL_UART_WRITE;
    sTransaction.pui8Data              = pui8Space0;
    sTransaction.ui32NumBytes          = psConfig->length;
    sTransaction.pui32BytesTransferred = &ui32DataSent;

    if (psConfig->txblocking)
    {
        sTransaction.ui32TimeoutMs = AM_HAL_UART_WAIT_FOREVER;
    }
    else
    {
        sTransaction.ui32TimeoutMs = 0;
    }

    CHECK_PASS(am_hal_uart_transfer(UART, &sTransaction));


    if (ui32DataSent != psConfig->length)
    {
        //
        // We didn't send all of the data we were supposed to.
        //
        return 2;
    }

    am_hal_uart_tx_flush(UART);
    //while (ui32TXDoneFlag == false && AM_BFRn(UART, 0, FR, BUSY) == true);

    sTransaction.ui32Direction         = AM_HAL_UART_READ;
    sTransaction.pui8Data              = pui8Space1;
    sTransaction.ui32NumBytes          = psConfig->length;
    sTransaction.pui32BytesTransferred = &ui32DataReceived;

    if (psConfig->rxblocking)
    {
        sTransaction.ui32TimeoutMs = AM_HAL_UART_WAIT_FOREVER;
    }
    else
    {
        sTransaction.ui32TimeoutMs = 0;
    }

    CHECK_PASS(am_hal_uart_transfer(UART, &sTransaction));

    if (ui32DataSent != ui32DataReceived)
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
    if (buffer_check(pui8Space0, pui8Space1, ui32DataSent))
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

uint32_t
am_widget_uart_loopback(am_widget_uart_loopback_config_t *psConfig)
{

    return am_widget_uart_loopback_clkSpd( psConfig, eUART_CLK_SPEED_DEFAULT ) ;
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
