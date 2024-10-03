//*****************************************************************************
//
//! @file process_sensors_task.c
//!
//! @brief Task to handle processing of sensor hub data 
//!
//*****************************************************************************

//*****************************************************************************
//
// ${copyright}
//
// This is part of revision ${version} of the AmbiqSuite Development Package.
//
//*****************************************************************************
#include "sensor_hub_test_cases.h"
#include "process_sensors_task.h"

//*****************************************************************************
//
// Process Sensors task handle.
//
//*****************************************************************************
TaskHandle_t ProcessSensorsTaskHandle;

//*****************************************************************************
//
// Global variables and templates 
//
//*****************************************************************************
SemaphoreHandle_t g_xIosIntSemaphore = NULL;
volatile bool g_bProcessSensorsTaskComplete = false;
volatile bool g_bufferFull = false;
volatile bool g_ppBufIdx = 0; // ping pong buffer index

//*****************************************************************************
//
// Copied from ios_fifo_host.c
// I2C code disabled
//
//*****************************************************************************
#define     IOM_MODULE          1

#define     XOR_BYTE            0
#define     EMPTY_BYTE          0xEE

typedef enum
{
    AM_IOSTEST_CMD_START_DATA    = 0,
    AM_IOSTEST_CMD_STOP_DATA     = 1,
    AM_IOSTEST_CMD_ACK_DATA      = 2,
} AM_IOSTEST_CMD_E;

#define IOSOFFSET_WRITE_INTEN       0xF8
#define IOSOFFSET_WRITE_INTCLR      0xFA
#define IOSOFFSET_WRITE_CMD         0x80
#define IOSOFFSET_READ_INTSTAT      0x79
#define IOSOFFSET_READ_FIFO         0x7F
#define IOSOFFSET_READ_FIFOCTR      0x7C

#define AM_IOSTEST_IOSTOHOST_DATAAVAIL_INTMASK  1

#define HANDSHAKE_PIN            4
//*****************************************************************************
//
// Configure GPIOs for communicating with a SPI fram
// TODO: This should ideally come from BSP to keep the code portable
//
//*****************************************************************************

//*****************************************************************************
//
// Global message buffer for the IO master.
//
//*****************************************************************************
#define AM_TEST_RCV_BUF_SIZE 1024 // Max Size we can receive is 1023
uint8_t g_pui8RcvBuf[AM_TEST_RCV_BUF_SIZE];
uint8_t g_pui8SensorDataBuf[2][MSG_BUF_BYTES]; // ping pong buffer for data to send to eMMC task
volatile uint32_t g_startIdx = 0;
volatile uint32_t g_totalBytes = 0;
volatile bool bIosInt = false;

void *g_IOMHandle;

//*****************************************************************************
//
// Configuration structure for the IO Master.
//
//*****************************************************************************
static am_hal_iom_config_t g_sIOMSpiConfig =
{
    .eInterfaceMode = AM_HAL_IOM_SPI_MODE,
//    .ui32ClockFreq = AM_HAL_IOM_96MHZ,
//    .ui32ClockFreq = AM_HAL_IOM_48MHZ,
//    .ui32ClockFreq = AM_HAL_IOM_24MHZ,
//    .ui32ClockFreq = AM_HAL_IOM_16MHZ,
//    .ui32ClockFreq = AM_HAL_IOM_500KHZ,
    .ui32ClockFreq = AM_HAL_IOM_400KHZ,
//    .ui32ClockFreq = AM_HAL_IOM_375KHZ,
//    .ui32ClockFreq = AM_HAL_IOM_250KHZ,
//    .ui32ClockFreq = AM_HAL_IOM_125KHZ,
//    .ui32ClockFreq = AM_HAL_IOM_100KHZ,
//    .ui32ClockFreq = AM_HAL_IOM_50KHZ,
//    .ui32ClockFreq = AM_HAL_IOM_10KHZ,
    .eSpiMode = AM_HAL_IOM_SPI_MODE_0,
};

#define MAX_SPI_SIZE  1023 

const am_hal_gpio_pincfg_t g_AM_BSP_GPIO_HANDSHAKE =
{
    .GP.cfg_b.uFuncSel       = AM_HAL_PIN_4_GPIO,
    .GP.cfg_b.eDriveStrength = AM_HAL_GPIO_PIN_DRIVESTRENGTH_12MA,
    .GP.cfg_b.eIntDir        = AM_HAL_GPIO_PIN_INTDIR_LO2HI,
    .GP.cfg_b.eGPInput       = AM_HAL_GPIO_PIN_INPUT_ENABLE,
};

//*****************************************************************************
//
// Clear Rx Buffer for comparison
//
//*****************************************************************************
void clear_rx_buf(void)
{
    uint32_t i;
    for ( i = 0; i < AM_TEST_RCV_BUF_SIZE; i++ )
    {
        g_pui8RcvBuf[i] = EMPTY_BYTE;
    }
}

//*****************************************************************************
//
// Validate Rx Buffer
// Returns true if no errors 
// Returns false and prints the first error if errors 
//
//*****************************************************************************
bool validate_rx_buf(uint32_t rxSize)
{
    uint32_t i;
    uint32_t bufBytesLeft = 0;
    uint32_t expectedData = 0;
    uint32_t sdBufIndex = 0;
    bool returnStatus = true;
    bool printError = true;
    
    // If rxSize is larger than the RcvBuf size, assert error and truncate data
    if (rxSize > AM_TEST_RCV_BUF_SIZE)
    {
        am_util_stdio_printf("\nvalidate_rx_buf: rxSize %d too large for Rx buf, limiting to RX Buf size %d\n", rxSize, AM_TEST_RCV_BUF_SIZE);
        TEST_ASSERT_TRUE(false);
        
        rxSize = AM_TEST_RCV_BUF_SIZE;
    }

    // Calculate # of bytes left in SensorDataBuf
    bufBytesLeft = MSG_BUF_BYTES - (g_startIdx - 1); // bytes left in SensorDataBuf
    expectedData = g_startIdx & 0xFF;

    for(i = 0; i < rxSize; i++)
    {
        g_pui8SensorDataBuf[g_ppBufIdx][g_startIdx + sdBufIndex] = g_pui8RcvBuf[i];
        if ( g_pui8RcvBuf[i] != expectedData)
        {
            if(printError)
            {
                am_util_stdio_printf("\nFailed to compare buffers at index %d, expected 0x%02X, got 0x%02X \n", sdBufIndex, expectedData, g_pui8RcvBuf[i]);
                printError = false; // only print the first error
            }
            am_hal_gpio_output_toggle(9);
            returnStatus = false;
        }
        sdBufIndex++;
        expectedData = (expectedData + 1) & 0xFF;
        if (i == bufBytesLeft)
        {
            g_startIdx = 0; 
            sdBufIndex = 0;
            g_bufferFull = true;
            g_ppBufIdx ^= 1;
        }            
    }

    // Set the reference for next chunk
    g_startIdx = g_startIdx + rxSize;

    return returnStatus;
}

// ISR callback for the host IOINT
void hostint_handler(void)
{
    bIosInt = true;
    if (g_xIosIntSemaphore != NULL)
    {   
        // Unblock the process sensors task by releasing the semaphore
        xSemaphoreGiveFromISR(g_xIosIntSemaphore, NULL); 
    }
}

//
// 2023/12/19
// Comment out for would cause multi define in am_devices_display_generic.c
//
////*****************************************************************************
////
//// Interrupt handler for the GPIO pins.
////
////*****************************************************************************
//void am_gpio0_001f_isr(void)
//{
//    //
//    // Read and clear the GPIO interrupt status.
//    //
//    uint32_t    ui32IntStatus;
//    AM_CRITICAL_BEGIN
//    am_hal_gpio_interrupt_irq_status_get(GPIO0_001F_IRQn, false, &ui32IntStatus);
//    am_hal_gpio_interrupt_irq_clear(GPIO0_001F_IRQn, ui32IntStatus);
//    AM_CRITICAL_END
//    am_hal_gpio_interrupt_service(GPIO0_001F_IRQn, ui32IntStatus);
//}

void iom_slave_read(uint32_t offset, uint32_t *pBuf, uint32_t size)
{
    am_hal_iom_transfer_t       Transaction;

    Transaction.ui32InstrLen    = 1;
    Transaction.ui64Instr = offset;
    Transaction.eDirection      = AM_HAL_IOM_RX;
    Transaction.ui32NumBytes    = size;
    Transaction.pui32RxBuffer   = pBuf;
    Transaction.bContinue       = false;
    Transaction.ui8RepeatCount  = 0;
    Transaction.ui32PauseCondition = 0;
    Transaction.ui32StatusSetClr = 0;

    Transaction.uPeerInfo.ui32SpiChipSelect = AM_BSP_IOM0_CS_CHNL;
    am_hal_iom_blocking_transfer(g_IOMHandle, &Transaction);
}

void iom_slave_write(uint32_t offset, uint32_t *pBuf, uint32_t size)
{
    am_hal_iom_transfer_t       Transaction;

    Transaction.ui32InstrLen    = 1;
    Transaction.ui64Instr = offset;
    Transaction.eDirection      = AM_HAL_IOM_TX;
    Transaction.ui32NumBytes    = size;
    Transaction.pui32TxBuffer   = pBuf;
    Transaction.bContinue       = false;
    Transaction.ui8RepeatCount  = 0;
    Transaction.ui32PauseCondition = 0;
    Transaction.ui32StatusSetClr = 0;

    Transaction.uPeerInfo.ui32SpiChipSelect = AM_BSP_IOM0_CS_CHNL;
    am_hal_iom_blocking_transfer(g_IOMHandle, &Transaction);
}

static void iom_set_up(uint32_t iomModule)
{
    uint32_t ioIntEnable = AM_IOSTEST_IOSTOHOST_DATAAVAIL_INTMASK;

    //
    // Initialize the IOM.
    //
    am_hal_iom_initialize(iomModule, &g_IOMHandle);

    am_hal_iom_power_ctrl(g_IOMHandle, AM_HAL_SYSCTRL_WAKE, false);

    //
    // Set the required configuration settings for the IOM.
    //
    am_hal_iom_configure(g_IOMHandle, &g_sIOMSpiConfig);

    //
    // Configure the IOM pins.
    //
    am_bsp_iom_pins_enable(iomModule, AM_HAL_IOM_SPI_MODE);

    //
    // Enable all the interrupts for the IOM module.
    //
    //am_hal_iom_InterruptEnable(g_IOMHandle, 0xFF);
    //am_hal_interrupt_enable(AM_HAL_INTERRUPT_IOMASTER0);

    //
    // Enable the IOM.
    //
    am_hal_iom_enable(g_IOMHandle);
    am_hal_gpio_pinconfig(HANDSHAKE_PIN, g_AM_BSP_GPIO_HANDSHAKE);

    uint32_t IntNum = HANDSHAKE_PIN;
    am_hal_gpio_state_write(HANDSHAKE_PIN, AM_HAL_GPIO_OUTPUT_CLEAR);
    // Set up the host IO interrupt
    am_hal_gpio_interrupt_clear(AM_HAL_GPIO_INT_CHANNEL_0, (am_hal_gpio_mask_t*)&IntNum);
    // Register handler for IOS => IOM interrupt
    am_hal_gpio_interrupt_register(AM_HAL_GPIO_INT_CHANNEL_0, HANDSHAKE_PIN,
                                    (am_hal_gpio_handler_t)hostint_handler, NULL);
    am_hal_gpio_interrupt_control(AM_HAL_GPIO_INT_CHANNEL_0,
                                  AM_HAL_GPIO_INT_CTRL_INDV_ENABLE,
                                  (void *)&IntNum);
    NVIC_SetPriority(GPIO0_001F_IRQn, NVIC_configMAX_SYSCALL_INTERRUPT_PRIORITY);
    NVIC_EnableIRQ(GPIO0_001F_IRQn);

    // Set up IOCTL interrupts
    // IOS ==> IOM
    iom_slave_write(IOSOFFSET_WRITE_INTEN, &ioIntEnable, 1);
}

uint32_t g_ui32LastUpdate = 0;

//*****************************************************************************
//
// Print a progress message.
//
//*****************************************************************************
void update_progress(uint32_t ui32NumPackets)
{
    //
    // Print a dot every 10000 packets.
    //
    if ( (ui32NumPackets - g_ui32LastUpdate) > 1000 )
    {
        am_util_stdio_printf("*");
        g_ui32LastUpdate = ui32NumPackets;
    }
}

//*****************************************************************************
//
// Perform initial setup for the Process Sensors task.
//
//*****************************************************************************
void
ProcessSensorsTaskSetup(void)
{
    am_util_stdio_printf("\nProcess Sensors task: setup\r\n");
}

//*****************************************************************************
//
// ProcessSensorsTask
//
//*****************************************************************************
extern bool g_bTestEnd;

void
ProcessSensorsTask(void *pvParameters)
{
    uint32_t iom = IOM_MODULE;
    bool bReadIosData = false;
    bool bDone = false;
    uint32_t data;
    uint32_t maxSize = MAX_SPI_SIZE;
    
    // Create semaphore for IOS Interrupt handler
    g_xIosIntSemaphore = xSemaphoreCreateBinary();
    TEST_ASSERT_TRUE(g_xIosIntSemaphore != NULL);
    if ( g_xIosIntSemaphore == NULL )
    {
        am_util_stdio_printf("\nCreate semphore failed!\n");
        vTaskDelete(NULL);
    }

    //
    // Enable Interrupts.
    //
    am_hal_interrupt_master_enable();

    //
    // Set up the IOM
    //
    iom_set_up(iom);

    // Send the START
    data = AM_IOSTEST_CMD_START_DATA;
    iom_slave_write(IOSOFFSET_WRITE_CMD, &data, 1);

    am_util_stdio_printf("\nRunning ProcessSensorsTask\r\n");

    uint32_t counter = 0;

    //while (!bDone)
    while(1)
    {
        // Wait for Semaphore from ISR
        xSemaphoreTake(g_xIosIntSemaphore, portMAX_DELAY);

        bIosInt = false;
        // Read & Clear the IOINT status
        iom_slave_read(IOSOFFSET_READ_INTSTAT, &data, 1);
        //am_util_stdio_printf("cmd rcvd=%d\n", data);

        // We need to clear the bit by writing to IOS
        if ( data & AM_IOSTEST_IOSTOHOST_DATAAVAIL_INTMASK )
        {
            data = AM_IOSTEST_IOSTOHOST_DATAAVAIL_INTMASK;
            iom_slave_write(IOSOFFSET_WRITE_INTCLR, &data, 1);
            // Set bReadIosData
            bReadIosData = true;
        }
        if ( bReadIosData )
        {
            uint32_t iosSize = 0;

            bReadIosData = false;

            // Read the Data Size
            iom_slave_read(IOSOFFSET_READ_FIFOCTR, &iosSize, 2);
            iosSize = (iosSize > maxSize)? maxSize: iosSize;

            // Initialize Rx Buffer for later comparison
            clear_rx_buf();

            // Read the data
            iom_slave_read(IOSOFFSET_READ_FIFO, (uint32_t *)g_pui8RcvBuf, iosSize);

            // Send the ACK/STOP
            data = AM_IOSTEST_CMD_ACK_DATA;
            iom_slave_write(IOSOFFSET_WRITE_CMD, &data, 1);

            update_progress(g_startIdx);

            // Validate Content
            if ( !validate_rx_buf(iosSize) )
            {
                TEST_ASSERT_TRUE(false);
                am_util_stdio_printf("\nData Verification failed Accum:%lu rx=%d, counter=%d, g_totalBytes=%d\n",
                    g_startIdx, iosSize, counter, g_totalBytes);
            }

            counter++;

            if(g_bufferFull)
            {
                g_totalBytes += g_startIdx;
                g_startIdx = 0;
                //bDone = true;
                data = AM_IOSTEST_CMD_STOP_DATA;
                iom_slave_write(IOSOFFSET_WRITE_CMD, &data, 1);

                am_util_stdio_printf("\nPS: %d bytes received from sensor hub, send to eMMC...\n", MSG_BUF_BYTES);
                xMessageBufferSend(g_xSensorDataMessageBuffer, (void *)&g_pui8SensorDataBuf[g_ppBufIdx^1], MSG_BUF_BYTES, portMAX_DELAY); 
                g_bufferFull = false;
                vTaskDelay(1000 / portTICK_PERIOD_MS); 

                // Send the START
                data = AM_IOSTEST_CMD_START_DATA;
                iom_slave_write(IOSOFFSET_WRITE_CMD, &data, 1);

            }

        }
        if(g_bTestEnd)
        {
            break;
        }

    }
    // Indicate the ProcessSensorsTask loops are complete
    g_bProcessSensorsTaskComplete = true;
    // Indicate task is ending
    am_util_stdio_printf("\nProcessSensorsTask end!\n");
    // Suspend and delete the task
    vTaskDelete(NULL);
}
