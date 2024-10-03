//*****************************************************************************
//
//! @file uart_task.c
//!
//! @brief Task to handle loopback of uart operations.
//!
//! Purpose: This example demonstrates how to read & write over UART. 
//!          The data is received by UART task from a message queue.
//!
//*****************************************************************************

//*****************************************************************************
//
// ${copyright}
//
// This is part of revision ${version} of the AmbiqSuite Development Package.
//
//*****************************************************************************

//*****************************************************************************
//
// Global includes for this project.
//
//*****************************************************************************
#include "audio_test_config.h"
#include "am_bsp_pins.h"

#define UART_TASK_ENABLE 1

#if  ( UART_TASK_ENABLE == 1 )
#define UART_MODULE_TOTAL                4   
uint32_t uart_test_instances[] =  {0};   //{0, 2, 3};
#define UART_TEST_INSTANCE_TOTAL         sizeof(uart_test_instances)/sizeof(uart_test_instances[0])   

#define UART_DATA_CHECK
//MSPI task loop delay
#define MAX_UART_TASK_DELAY    (100 * 2)

#define UART_BUFFER_SIZE        128
#define UART_TEST_PACKET_SIZE   128

static SemaphoreHandle_t g_semUARTSetupFinish = NULL;

// Queue parameters
#define PMODE_Tx 0644
#define QUEUE_NAME_PU  "/psram_uart_queue"
#define PATTERN_BUF_SIZE        128

volatile bool g_bUartTaskComplete = false;
extern volatile bool g_bEmmcTaskComplete;
extern volatile bool g_bMspiTaskComplete;
//*****************************************************************************
//
// UART Configuration
//
//*****************************************************************************
am_hal_uart_config_t sUartConfig =
{
    .ui32BaudRate = 500000,
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

}


//*****************************************************************************
//
// Data check.
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



//*****************************************************************************
//
// UART task handle.
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
    uint32_t instance = 0;
    uint32_t randomDelay;
    uint32_t ui32Count = 0;
    int TxQueueLength, RxQueueLength;
    
    mqd_t mqfd;
    uint8_t msg_buffer[PATTERN_BUF_SIZE];
    struct mq_attr attr;
    int open_flags = 0;
    size_t num_bytes_received = 0;
    uint8_t msg_buffer_Orig[PATTERN_BUF_SIZE];
    int status_Tx = 0;

    //Create semphone for sync
    g_semUARTSetupFinish = xSemaphoreCreateBinary();
    TEST_ASSERT_TRUE(g_semUARTSetupFinish != NULL );
    if ( g_semUARTSetupFinish == NULL )
    {
        am_util_stdio_printf("\n IOM%d: Create semphone failed!\n", i);
    }
    
    //
    // Clear the terminal and print the banner.
    //
    am_util_stdio_printf("\n UART task start\n");

    instance = uart_test_instances[0]; // Instance can be changed based on UART. Using UART0.

    // Wait for UART setup to complete
    xSemaphoreTake( g_semUARTSetupFinish, ( TickType_t ) 0);

    // Configure the UART pins.
	// Connect pins GP10 and GP9 on turbo board.
    am_util_stdio_printf("\n For UART0 testing, connect pin %d (TX) to pin %d (RX).\n",
    AM_BSP_GPIO_UART0_TX, AM_BSP_GPIO_UART0_RX);
    am_hal_gpio_pinconfig( AM_BSP_GPIO_UART0_TX, g_AM_BSP_GPIO_UART0_TX);
    am_hal_gpio_pinconfig( AM_BSP_GPIO_UART0_RX, g_AM_BSP_GPIO_UART0_RX);
  
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
	
	
    // Generate Original message sent by eMMc Task.
    for (int i = 0; i < PATTERN_BUF_SIZE; i++)
    {
        msg_buffer_Orig[i] = i & 0xFF;
    }

    /* Fill in attributes for message queue */
    // The maximum number of messages that can be stored on the queue. 
    // In the QNX implementation, an mq_maxmsg of 0 means that there is no maximum    
    attr.mq_maxmsg = 3;  
    // The maximum size of each message on the given message queue.
    attr.mq_msgsize = PATTERN_BUF_SIZE;
    // The options set for this queue. 0 implies blocking.
    attr.mq_flags   = 0;

    /* Set the flags for the open of the queue.
    * Make it a blocking open on the queue,
    * meaning it will block if this process tries to
    * send to the queue and the queue is full.
    * (Absence of O_NONBLOCK flag implies that
    * the open is blocking)
    *
    * Specify O_CREAT so that the file will get
    * created if it does not already exist.
    *
    * Specify O_RDONLY since we are only
    * planning to write to the queue,
    * although we could specify O_RDWR also.
    */
    open_flags = O_RDONLY|O_CREAT;

    /* Open the queue, and create it if the sending process hasn't
    * already created it.
    */
    mqfd = mq_open(QUEUE_NAME_PU,open_flags,PMODE_Tx,&attr);
    if (mqfd == ( mqd_t ) -1)
    {
        TEST_ASSERT_TRUE(false);
        am_util_stdio_printf("\n Failed to open UART Queue. \n");
    }

    // UART setup complete, end critical section.
    xSemaphoreGive(g_semUARTSetupFinish); 
    
#ifdef TASK_LOOP_COUNT 
    while(ui32Count < TASK_LOOP_COUNT)
#else
    while(1)
#endif
    {
        ui32Count++;
			
        // Wait till UART Queue receives data from MSPI Task.
        while (psramDataSent == 0);
			
        // Check id there is anything to be read in the queue.
        if(psramDataSent ==1)
        {
					  // Clear the message buffer before read,
		        memset((void *)msg_buffer, 0x0, PATTERN_BUF_SIZE);
					
            // Get data from the queue.
            status_Tx = mq_receive(mqfd,(char*)msg_buffer,PATTERN_BUF_SIZE, 0);
            if (status_Tx == -1)
            {
                TEST_ASSERT_TRUE(false);
                am_util_stdio_printf("\n Failure to read data from UART Queue.");
            }
            else
            {
                am_util_stdio_printf("\n Data successfully read from UART Queue. \n");
            }
      
        }
   														
        // Send data received over Queue to UART Tx buffer.
        memcpy(g_sUartFifoCfg[instance].pui8SpaceTx, msg_buffer, PATTERN_BUF_SIZE);
						
        // Clear UART RX Buffer.
        memset((void *)g_sUartFifoCfg[instance].pui8SpaceRx, 0x0, PATTERN_BUF_SIZE);

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

        am_hal_uart_tx_flush(g_hUART[instance]);             

        if ( *sTransaction.pui32BytesTransferred != sTransaction.ui32NumBytes )
        {
            TEST_ASSERT_TRUE(false);
            am_util_stdio_printf("\n TX data length not matched!");
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
            TEST_ASSERT_TRUE(false);
            //
            // We didn't receive all of the data we sent.
            //
            am_util_stdio_printf("\n UART#%d RX data length not matched!\n", instance);
        }
										
            
#ifdef UART_DATA_CHECK            
        //
        // Check for errors.
        //
        if (buffer_check(g_sUartFifoCfg[instance].pui8SpaceTx, g_sUartFifoCfg[instance].pui8SpaceRx, sTransaction.ui32NumBytes))
        {
            am_util_stdio_printf("\n UART RX and TX data not matched. \n");
            am_util_stdio_printf("\n TX:\n");
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
        else
        {
            am_util_stdio_printf("\n Pass: UART RX and TX data matched. \n");	
        }							
						
        // Check if Original Data Sent from eMMC Task matches the Data Received over UART.
        if (buffer_check(msg_buffer_Orig, g_sUartFifoCfg[instance].pui8SpaceRx, sTransaction.ui32NumBytes))
        {
            am_util_stdio_printf("\n UART RX and Original data not matched!\n");
            am_util_stdio_printf("\n Original:\n");
            for(uint32_t i = 0; i < sTransaction.ui32NumBytes; i++)
            {
                am_util_stdio_printf("%x\t", msg_buffer_Orig[i]);
            }
            am_util_stdio_printf("\n RX:\n");
            for(uint32_t i = 0; i < sTransaction.ui32NumBytes; i++)
            {
                am_util_stdio_printf("%x\t", g_sUartFifoCfg[instance].pui8SpaceRx[i]);
            }
        }
        else
        {
            am_util_stdio_printf("\n PASS : UART RX and Original data matched.\n");	
        }						
						
#endif  
				
        // Clear the UART queue sent flag.
        psramDataSent = 0;

    }
    
    // Indicate the Task loops are complete
    g_bUartTaskComplete = true;    
		 
    // Task delay for task synchronization and to let ther tasks complete.
    vTaskDelay(1000);

    // Cleanup

    am_util_stdio_printf("\n UART task end!\n");

    for ( i = 0; i < UART_TEST_INSTANCE_TOTAL; i++ )
    {
        am_hal_uart_power_control(g_hUART[uart_test_instances[0]], AM_HAL_SYSCTRL_DEEPSLEEP, false);
        am_hal_uart_deinitialize(g_hUART[uart_test_instances[0]]);
        NVIC_DisableIRQ((IRQn_Type)(UART0_IRQn + uart_test_instances[0]));
        am_hal_uart_interrupt_disable(g_hUART[uart_test_instances[0]], 0xFFFF);

    }
    // Suspend and delete the task
    vTaskDelete(NULL);
}

#endif  //( UART_TASK_ENABLE == 1 )
