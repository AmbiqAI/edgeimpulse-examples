//*****************************************************************************
//
//! @file uart_task.c
//!
//! @brief Task to handle loopback of uart operations.
//!
//!
//*****************************************************************************

//*****************************************************************************
//
// ${copyright}
//
// This is part of revision ${version} of the AmbiqSuite Development Package.
//
//*****************************************************************************
#include "mspi_dma_xip_parallel_stress_test_cases.h"

#if  ( UART_TASK_ENABLE == 1 )
#define UART_MODULE_TOTAL                4   
uint32_t uart_test_instances[] =  {0, 2, 3};
#define UART_TEST_INSTANCE_TOTAL         sizeof(uart_test_instances)/sizeof(uart_test_instances[0])   

#define UART_DATA_CHECK
//MSPI task loop delay
#define MAX_UART_TASK_DELAY    (100 * 2)

#define UART_BUFFER_SIZE        256
#define UART_TEST_PACKET_SIZE   128
//*****************************************************************************
//
// UART Configuration
//
//*****************************************************************************
am_hal_uart_config_t sUartConfig =
{
    .ui32BaudRate = 115200,
    .eDataBits = AM_HAL_UART_DATA_BITS_8,
    .eParity = AM_HAL_UART_PARITY_NONE,
    .eStopBits = AM_HAL_UART_ONE_STOP_BIT,
#if UART_RTS_CTS_ENABLED
    .eFlowControl = AM_HAL_UART_FLOW_CTRL_RTS_CTS,
#else
    .eFlowControl = AM_HAL_UART_FLOW_CTRL_NONE,
#endif
    .eTXFifoLevel = AM_HAL_UART_FIFO_LEVEL_16,
    .eRXFifoLevel = AM_HAL_UART_FIFO_LEVEL_16,
};

void *g_hUART[UART_MODULE_TOTAL];

typedef struct
{
    uint8_t  uart;
    uint32_t length;

    bool txblocking;
    bool rxblocking;

    uint32_t txbufferSize;
    uint32_t rxbufferSize;

    uint32_t txBytesTransferred;
    uint32_t rxBytesTransferred;

    bool txQueueEnable;
    bool rxQueueEnable;

    uint8_t pui8QueueTx[UART_BUFFER_SIZE];
    uint8_t pui8QueueRx[UART_BUFFER_SIZE];

    uint8_t pui8SpaceTx[UART_BUFFER_SIZE];
    uint8_t pui8SpaceRx[UART_BUFFER_SIZE];

    uint32_t result;
}
am_uart_fifo_config_t;
am_uart_fifo_config_t   g_sUartFifoCfg[UART_MODULE_TOTAL];
am_hal_uart_transfer_t  sTransaction;
static SemaphoreHandle_t g_semTXDone[UART_MODULE_TOTAL] = {NULL, };
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

    am_hal_uart_interrupt_status_get(g_hUART[0], &ui32Status, true);
    am_hal_uart_interrupt_clear(g_hUART[0], ui32Status);
    am_hal_uart_interrupt_service(g_hUART[0], ui32Status);

    xSemaphoreGiveFromISR(g_semTXDone[0], NULL);
}

void
am_uart1_isr(void)
{
    //
    // Service the FIFOs as necessary, and clear the interrupts.
    //
    uint32_t ui32Status;

    am_hal_uart_interrupt_status_get(g_hUART[1], &ui32Status, true);
    am_hal_uart_interrupt_clear(g_hUART[1], ui32Status);
    am_hal_uart_interrupt_service(g_hUART[1], ui32Status);

    xSemaphoreGiveFromISR(g_semTXDone[1], NULL);
}

void
am_uart2_isr(void)
{
    //
    // Service the FIFOs as necessary, and clear the interrupts.
    //
    uint32_t ui32Status;

    am_hal_uart_interrupt_status_get(g_hUART[2], &ui32Status, true);
    am_hal_uart_interrupt_clear(g_hUART[2], ui32Status);
    am_hal_uart_interrupt_service(g_hUART[2], ui32Status);

    xSemaphoreGiveFromISR(g_semTXDone[2], NULL);
}

void
am_uart3_isr(void)
{
    //
    // Service the FIFOs as necessary, and clear the interrupts.
    //
    uint32_t ui32Status;

    am_hal_uart_interrupt_status_get(g_hUART[3], &ui32Status, true);
    am_hal_uart_interrupt_clear(g_hUART[3], ui32Status);
    am_hal_uart_interrupt_service(g_hUART[3], ui32Status);

    xSemaphoreGiveFromISR(g_semTXDone[3], NULL);
}

//
// Compare two buffers.
//

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

//*****************************************************************************
//
// adc task handle.
//
//*****************************************************************************
TaskHandle_t UARTTaskHandle;

//*****************************************************************************
//
// Main function.
//
//*****************************************************************************
void
UARTTask(void *pvParameters)    
{
    uint32_t i,j;
    uint32_t instance;
    uint32_t      randomDelay;
    
    //
    // Clear the terminal and print the banner.
    //
    am_util_stdio_printf("UART Loopback task start\n");
    
    for ( i = 0; i < UART_TEST_INSTANCE_TOTAL; i++ )
    {    
        instance = uart_test_instances[i];
        //Create semphone for sync
        g_semTXDone[instance] = xSemaphoreCreateBinary();
        if ( g_semTXDone[instance] == NULL )
        {
            am_util_stdio_printf("UART%d: Create semphone failed!\n", instance);
        }
    }
    
    for ( i = 0; i < UART_TEST_INSTANCE_TOTAL; i++ ) 
    {
        instance = uart_test_instances[i];
        switch(instance)
        {    
#if defined(AM_BSP_GPIO_UART0_TX) && defined(AM_BSP_GPIO_UART0_RX)
        case 0:
            am_util_stdio_printf("  For UART0 testing, connect pin %d (TX) to pin %d (RX).\n",
                         AM_BSP_GPIO_UART0_TX, AM_BSP_GPIO_UART0_RX);
            am_hal_gpio_pinconfig( AM_BSP_GPIO_UART0_TX, g_AM_BSP_GPIO_UART0_TX);
            am_hal_gpio_pinconfig( AM_BSP_GPIO_UART0_RX, g_AM_BSP_GPIO_UART0_RX);
            break;
#endif
#if defined(AM_BSP_GPIO_UART1_TX) && defined(AM_BSP_GPIO_UART1_RX)
        case 1:
            am_util_stdio_printf("  For UART1 testing, connect pin %d (TX) to pin %d (RX).\n",
                         AM_BSP_GPIO_UART1_TX, AM_BSP_GPIO_UART1_RX);
            am_hal_gpio_pinconfig( AM_BSP_GPIO_UART1_TX, g_AM_BSP_GPIO_UART1_TX);
            am_hal_gpio_pinconfig( AM_BSP_GPIO_UART1_RX, g_AM_BSP_GPIO_UART1_RX);
            break;
#endif
#if defined(AM_BSP_GPIO_UART2_TX) && defined(AM_BSP_GPIO_UART2_RX)
        case 2:
            am_util_stdio_printf("  For UART2 testing, connect pin %d (TX) to pin %d (RX).\n",
                         AM_BSP_GPIO_UART2_TX, AM_BSP_GPIO_UART2_RX);
            am_hal_gpio_pinconfig( AM_BSP_GPIO_UART2_TX, g_AM_BSP_GPIO_UART2_TX);
            am_hal_gpio_pinconfig( AM_BSP_GPIO_UART2_RX, g_AM_BSP_GPIO_UART2_RX);
            break;
#endif
#if defined(AM_BSP_GPIO_UART3_TX) && defined(AM_BSP_GPIO_UART3_RX)
        case 3:
            am_util_stdio_printf("  For UART3 testing, connect pin %d (TX) to pin %d (RX).\n",
                         AM_BSP_GPIO_UART3_TX, AM_BSP_GPIO_UART3_RX);
            am_hal_gpio_pinconfig( AM_BSP_GPIO_UART3_TX, g_AM_BSP_GPIO_UART3_TX);
            am_hal_gpio_pinconfig( AM_BSP_GPIO_UART3_RX, g_AM_BSP_GPIO_UART3_RX);
            break;
#endif
        default:
            am_util_stdio_printf("\nUART%d not defined in BSP", instance);
            break;
        }    
  
        //
        // Enable the UART.
        //
        am_hal_uart_initialize( instance, &g_hUART[instance] ) ;
        am_hal_uart_power_control(g_hUART[instance], AM_HAL_SYSCTRL_WAKE, false);
        am_hal_uart_interrupt_enable(g_hUART[instance], (AM_HAL_UART_INT_TX |
                                               AM_HAL_UART_INT_RX |
                                               AM_HAL_UART_INT_RX_TMOUT |
                                               AM_HAL_UART_INT_OVER_RUN |
                                               AM_HAL_UART_INT_TXCMP));
        am_hal_uart_configure(g_hUART[instance], &sUartConfig);
        
        //
        // Enable the interrupt in the NVIC.
        //
        IRQn_Type irq_no = ((IRQn_Type)(UART0_IRQn + instance));

        NVIC_SetPriority(irq_no, UART_ISR_PRIORITY);
        NVIC_ClearPendingIRQ(irq_no);
        NVIC_EnableIRQ(irq_no); 
    }   
    int TxQueueLength, RxQueueLength;
    for ( i = 0; i < UART_TEST_INSTANCE_TOTAL; i++ ) 
    {
        instance = uart_test_instances[i];
        
        g_sUartFifoCfg[instance].uart = instance;
        g_sUartFifoCfg[instance].length = UART_TEST_PACKET_SIZE;

        g_sUartFifoCfg[instance].txblocking = 0;
        g_sUartFifoCfg[instance].rxblocking = 0;

        g_sUartFifoCfg[instance].txbufferSize = UART_BUFFER_SIZE;
        g_sUartFifoCfg[instance].rxbufferSize = UART_BUFFER_SIZE;

        g_sUartFifoCfg[instance].txQueueEnable = 1;
        g_sUartFifoCfg[instance].rxQueueEnable = 1;

        g_sUartFifoCfg[instance].txBytesTransferred = 0;
        g_sUartFifoCfg[instance].rxBytesTransferred = 0;
        g_sUartFifoCfg[instance].result = 0;
        if (g_sUartFifoCfg[instance].txQueueEnable)
        {
            TxQueueLength = sizeof(g_sUartFifoCfg[instance].pui8QueueTx);
        }
        else
        {
            TxQueueLength = 0;
        }
        
        if (g_sUartFifoCfg[instance].rxQueueEnable)
        {
            RxQueueLength = sizeof(g_sUartFifoCfg[instance].pui8QueueRx);
        }
        else
        {
            RxQueueLength = 0;
        } 
        
        am_hal_uart_buffer_configure(g_hUART[instance], g_sUartFifoCfg[instance].pui8QueueTx, TxQueueLength, g_sUartFifoCfg[instance].pui8QueueRx, RxQueueLength);
        
        for ( j = 0; j < g_sUartFifoCfg[instance].txbufferSize; j++ )
        { 
            g_sUartFifoCfg[instance].pui8SpaceTx[j] = j+1;
            g_sUartFifoCfg[instance].pui8SpaceRx[j] = 0;
        }
    }

    while(1)
    {
        for (i = 0; i < UART_TEST_INSTANCE_TOTAL; i++)
        {
            instance = uart_test_instances[i];
            sTransaction.pui8Data = g_sUartFifoCfg[instance].pui8SpaceTx;
            sTransaction.ui32NumBytes = g_sUartFifoCfg[instance].txbufferSize;
            sTransaction.pui32BytesTransferred = &g_sUartFifoCfg[instance].txBytesTransferred;
            if (g_sUartFifoCfg[instance].txblocking)
            {
                sTransaction.eType = AM_HAL_UART_BLOCKING_WRITE;
                sTransaction.ui32TimeoutMs = AM_HAL_UART_WAIT_FOREVER;
            }
            else
            {
                sTransaction.eType = AM_HAL_UART_NONBLOCKING_WRITE;
                sTransaction.ui32TimeoutMs = 0;
            }
            sTransaction.pfnCallback = NULL;
            sTransaction.ui32ErrorStatus=0;

            am_hal_uart_transfer(g_hUART[instance], &sTransaction);
            xSemaphoreTake( g_semTXDone[instance], portMAX_DELAY);
            am_hal_uart_tx_flush(g_hUART[instance]);             

            if ( *sTransaction.pui32BytesTransferred != sTransaction.ui32NumBytes )
            {
                am_util_stdio_printf("\nTX data length not matched!");
            }

            sTransaction.pui8Data = g_sUartFifoCfg[instance].pui8SpaceRx;
            sTransaction.ui32NumBytes = g_sUartFifoCfg[instance].rxbufferSize;   
            sTransaction.pui32BytesTransferred = &g_sUartFifoCfg[instance].rxBytesTransferred;
            if (g_sUartFifoCfg[instance].rxblocking)
            {
                sTransaction.eType = AM_HAL_UART_BLOCKING_READ;
                sTransaction.ui32TimeoutMs = AM_HAL_UART_WAIT_FOREVER;
            }
            else
            {
                sTransaction.eType = AM_HAL_UART_NONBLOCKING_READ;
                sTransaction.ui32TimeoutMs = 0;
            }

            am_hal_uart_transfer(g_hUART[instance], &sTransaction);

            if (*sTransaction.pui32BytesTransferred != sTransaction.ui32NumBytes)
            {
                //
                // We didn't receive all of the data we sent.
                //
                am_util_stdio_printf("\nUART#%d RX data length not matched!\n", instance);
            }
            
            am_util_stdio_printf("[TASK] : UART#%d\n", instance);
            
#ifdef UART_DATA_CHECK            
            //
            // Check for errors.
            //
            if (buffer_check(g_sUartFifoCfg[instance].pui8SpaceTx, g_sUartFifoCfg[instance].pui8SpaceRx, sTransaction.ui32NumBytes))
            {
                am_util_stdio_printf("\nRX data not matched!\n");
                am_util_stdio_printf("TX:\n");
                for(uint32_t i = 0; i < sTransaction.ui32NumBytes; i++)
                {
                    am_util_stdio_printf("%x\t", g_sUartFifoCfg[instance].pui8SpaceTx[i]);
                }
                am_util_stdio_printf("\nRX:\n");
                for(uint32_t i = 0; i < sTransaction.ui32NumBytes; i++)
                {
                    am_util_stdio_printf("%x\t", g_sUartFifoCfg[instance].pui8SpaceRx[i]);
                }
            }  
#endif        
            srand(xTaskGetTickCount());
            randomDelay = rand() % MAX_UART_TASK_DELAY;
            vTaskDelay(randomDelay);
        }
    }
    
    // Cleanup
    am_util_stdio_printf("\nUART task end!\n");

    for ( i = 0; i < UART_TEST_INSTANCE_TOTAL; i++ )
    {
        am_hal_uart_power_control(g_hUART[uart_test_instances[i]], AM_HAL_SYSCTRL_DEEPSLEEP, false);
        am_hal_uart_deinitialize(g_hUART[uart_test_instances[i]]);
        NVIC_DisableIRQ((IRQn_Type)(UART0_IRQn + uart_test_instances[i]));
        am_hal_uart_interrupt_disable(g_hUART[uart_test_instances[i]], 0xFFFF);

    }

    vTaskSuspend(NULL);
}

#endif  //( UART_TASK_ENABLE == 1 )
