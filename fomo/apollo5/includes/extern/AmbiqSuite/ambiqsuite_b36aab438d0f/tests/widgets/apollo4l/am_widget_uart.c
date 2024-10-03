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
//!     GPIO[53]-TX
//!     GPIO[55]-RX
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
            while(1);                                                         \
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

uint32_t g_TestBaudRate[] =
{
    115200, 460800, 921600, 1500000, 2000000, 3000000
};
#define TEST_RATE_START 0
#if defined(apollo4l_blue_eb_slt)
#define TEST_RATE_END   0
#else
#define TEST_RATE_END   5
#endif

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

#if defined(FPGA_HOST)
extern volatile bool g_bFPGAReady;
#define UART_FPGA_READY_IN_PIN 4

//*****************************************************************************
//
// Interrupt handler for the GPIO pins.
//
//*****************************************************************************
// ISR callback for the FPGA ready int
static void fpga_rdy_handler(void *pArg)
{
    g_bFPGAReady = true;
}
#endif

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
am_widget_uart_loopback_init(am_widget_uart_loopback_config_t *psConfigloopback, uint8_t blocking,
                                         uint8_t enableQueue, uint32_t length, uint8_t uart)
{
    int   i;
    psConfigloopback->uart = uart;
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
    //
    // Enable UART pins.
    //
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
#if defined(AM_BSP_GPIO_COM_UART_TX) && defined(AM_BSP_GPIO_COM_UART_RX)
            am_hal_gpio_pinconfig( AM_BSP_GPIO_COM_UART_TX, g_AM_BSP_GPIO_COM_UART_TX);
            am_hal_gpio_pinconfig( AM_BSP_GPIO_COM_UART_RX, g_AM_BSP_GPIO_COM_UART_RX);
            psConfig->uart = AM_BSP_UART_PRINT_INST;
            break;
#else
            am_util_stdio_printf("\nUART%d not defined in BSP\n", psConfig->uart);
            return 0;
#endif
    }

    //
    // Enable the UART.
    //
    CHECK_PASS(am_hal_uart_initialize( psConfig->uart, &g_hUART ) );
    CHECK_PASS(am_hal_uart_power_control(g_hUART, AM_HAL_SYSCTRL_WAKE, false));

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

#if defined(FPGA_HOST)
    uint32_t IntNum = UART_FPGA_READY_IN_PIN;

    am_hal_gpio_pinconfig(IntNum, am_hal_gpio_pincfg_input);

    uint32_t ui32IntStatus = (1 << IntNum);
    am_hal_gpio_interrupt_irq_clear(GPIO0_001F_IRQn, ui32IntStatus);

    am_hal_gpio_interrupt_register(AM_HAL_GPIO_INT_CHANNEL_0, IntNum, fpga_rdy_handler, NULL);
    am_hal_gpio_interrupt_control(AM_HAL_GPIO_INT_CHANNEL_0,
                                  AM_HAL_GPIO_INT_CTRL_INDV_ENABLE,
                                  (void *)&IntNum);
    NVIC_EnableIRQ(GPIO0_001F_IRQn);
#endif

    am_hal_interrupt_master_enable();

    for(uint8_t brIndex = TEST_RATE_START; brIndex <= TEST_RATE_END; brIndex++)
    {
#if defined(FPGA_HOST)
        while(g_bFPGAReady == false);
        g_bFPGAReady = false;
#endif

        sUartConfig.ui32BaudRate = g_TestBaudRate[brIndex];
        am_util_stdio_printf("\nBaud rate: %d\n", sUartConfig.ui32BaudRate);
        uint32_t ui32Status =  am_hal_uart_configure(g_hUART, &sUartConfig);
        if(ui32Status == AM_HAL_STATUS_OUT_OF_RANGE)
        {
            am_util_stdio_printf("Skip this baud rate since 4lite limitation\n");
            continue;
        }

        //
        // Generate some test data.
        //
        ui32TXDoneFlag = false;

        // Data location to use for this transaction.
        sTransaction.pui8Data = psConfig->pui8SpaceTx;
        // How many bytes should we send?
        // As we use the same uart instance for loopback test, there will be data loss with high baudrate and large packet
        if(sUartConfig.ui32BaudRate >= 1500000)
        {
            sTransaction.ui32NumBytes = 32;
        }
        else
        {
            sTransaction.ui32NumBytes = psConfig->txbufferSize;
        }
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

        if ( *sTransaction.pui32BytesTransferred != sTransaction.ui32NumBytes )
        {
            // We didn't send all of the data we were supposed to.
            am_util_stdio_printf("\nTX data length not matched!\n");
            continue;
        }
        while (ui32TXDoneFlag == false);// && AM_BFRn(g_hUART, 0, FR, BUSY) == true);

        sTransaction.pui8Data = psConfig->pui8SpaceRx;
        // How many bytes should we receive?
        // As we use the same uart instance for loopback test, there will be data loss with high baudrate and large packet
        if(sUartConfig.ui32BaudRate >= 1500000)
        {
            sTransaction.ui32NumBytes = 32;
        }
        else
        {
            sTransaction.ui32NumBytes = psConfig->rxbufferSize;
        }
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

        if (*sTransaction.pui32BytesTransferred != sTransaction.ui32NumBytes)
        {
            //
            // We didn't receive all of the data we sent.
            //
            am_util_stdio_printf("\nRX data length not matched!\n");
            am_util_stdio_printf("TX:\n");
            for(uint32_t i = 0; i < sTransaction.ui32NumBytes; i++)
            {
                am_util_stdio_printf("%x\t", psConfig->pui8SpaceTx[i]);
            }
            am_util_stdio_printf("\nRX:\n");
            for(uint32_t i = 0; i < sTransaction.ui32NumBytes; i++)
            {
                am_util_stdio_printf("%x\t", psConfig->pui8SpaceRx[i]);
            }
            am_util_stdio_printf("\n");
            continue;
        }

        //
        // Check for errors.
        //
        if (buffer_check(psConfig->pui8SpaceTx, psConfig->pui8SpaceRx, sTransaction.ui32NumBytes))
        {
            am_util_stdio_printf("\nRX data not matched!\n");
            am_util_stdio_printf("TX:\n");
            for(uint32_t i = 0; i < sTransaction.ui32NumBytes; i++)
            {
                am_util_stdio_printf("%x\t", psConfig->pui8SpaceTx[i]);
            }
            am_util_stdio_printf("\nRX:\n");
            for(uint32_t i = 0; i < sTransaction.ui32NumBytes; i++)
            {
                am_util_stdio_printf("%x\t", psConfig->pui8SpaceRx[i]);
            }
            am_util_stdio_printf("\n");
        }
    }

    //
    // Shut everything back down.
    //
    CHECK_PASS(am_hal_uart_power_control(g_hUART, AM_HAL_SYSCTRL_DEEPSLEEP, false));
    CHECK_PASS(am_hal_uart_deinitialize(g_hUART));
    NVIC_DisableIRQ((IRQn_Type)(UART0_IRQn + psConfig->uart));
    am_hal_uart_interrupt_disable(g_hUART, 0xFFFF);
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
