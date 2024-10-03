//*****************************************************************************
//
//! @file mspi_task.c
//!
//! @brief Task to handle PSRAM MSPI operations.
//!
//! Purpose: This example demonstrates how to read & write using APIs with
//!          PSRAM device over HEX MSPI.
//!          The data is then transferred to UART task over a message queue
//!
//! AM_DEBUG_PRINTF
//! If enabled, debug messages will be sent over ITM.
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
#include "am_devices_mspi_psram_aps25616n.h"
#include "am_devices_spipsram.h"
#include "am_mcu_apollo.h"
#include "am_util.h"

// Queue Parameters.
#define PMODE 0666
#define PMODE_Tx 0644
#define QUEUE_NAME_EP  "/emmc_psram_queue"
#define QUEUE_NAME_PU  "/psram_uart_queue"

#define START_BLK 3000
#define BLK_NUM 4
#define BUF_LEN 128*BLK_NUM

extern void            *g_pPsramHandle;

volatile bool g_bMspiTaskComplete = false;
extern volatile bool g_bEmmcTaskComplete;
extern volatile bool g_bUartTaskComplete;

//*****************************************************************************
//
// Macro definitions
//
//*****************************************************************************
#define FLASH_TARGET_ADDRESS     0
#define MSPI_BUFFER_SIZE        (128)  // 512 example buffer size.


//Undefine this if you want do dummy read.
//#define MSPI_DATA_CHECK
//#define MSPI_TASK_DEBUG_LOG

//MSPI task loop delay
#define MAX_MSPI_TASK_DELAY    (100 * 2)

//Delay to wait read operation complete.
#define MSPI_READ_DELAY        (10)

//*****************************************************************************
//
// MSPI task handle.
//
//*****************************************************************************
TaskHandle_t MspiTaskHandle;

//*****************************************************************************
//
// Global Variables
//
//*****************************************************************************

// Flag to synchronize data transfer in queue.
uint8_t psramDataSent = 0;
extern uint8_t emmcDataSent;

//*****************************************************************************
//
// MSPI Buffer.
//
//*****************************************************************************

//static uint8_t  gFlashRXBufferTCM[MSPI_BUFFER_SIZE];

//*****************************************************************************
//
// Data check function.
//
//*****************************************************************************

void check_if_data_match_mspi(uint8_t *pui8RdBuf, uint8_t *pui8WrBuf, uint32_t ui32Len)
{
    uint32_t i;
    for (i = 0; i < ui32Len; i++)
    {
        if (pui8RdBuf[i] != pui8WrBuf[i])
        {
            TEST_ASSERT_TRUE(false);
            am_util_stdio_printf("\n MSPI error: pui8RdBuf[%d] = %d and pui8WrBuf[%d] = %d\n", i, pui8RdBuf[i], i, pui8WrBuf[i]);
            break;
        }
    }

    if (i == ui32Len)
    {
			am_util_stdio_printf("\n Pass: PSRAM MSPI TX and RX data matched. \n");
    }

}

//*****************************************************************************
//
// MSPI task.
//
//*****************************************************************************
void
MspiTask(void *pvParameters)
{
    uint32_t      ui32Status,randomDelay;
    uint32_t      ui32Count = 0;
    uint8_t       ui8PSRAMRXBuffer[MSPI_BUFFER_SIZE], ui8PSRAMTXBuffer[MSPI_BUFFER_SIZE],ui8OriginalBuf[MSPI_BUFFER_SIZE],msg_buffer[MSPI_BUFFER_SIZE];

    mqd_t mqfd,mqfd_Tx;
    struct mq_attr attr, attr_Tx;
    uint8_t open_flags = 0;
    ssize_t num_bytes_received = 0;

    int status_Tx = 0;
    uint8_t open_flags_Tx = 0;
    uint8_t num_bytes_to_send_Tx,priority_of_msg_Tx;


    //
    // Write the TX buffer into the target sector.
    //
    uint32_t ui32size = MSPI_BUFFER_SIZE;
    uint32_t NumBytes = ui32size;
    uint32_t ByteOffset = 0;
    uint32_t SectorAddress = 0x00000000;
    uint32_t SectorOffset = 0;
    uint32_t UnscrambledSector = 0;

    //
    // Clear the terminal and print the banner.
    //
    am_util_stdio_printf("\n MSPI PSRAM task start. \n");

    // Initialize Rx Queue.

    /* Fill in attributes for message queue */
    // The maximum number of messages that can be stored on the queue.
    // In the QNX implementation, an mq_maxmsg of 0 means that there is no maximum
    attr.mq_maxmsg = 3;
    // The maximum size of each message on the given message queue.
    attr.mq_msgsize = MSPI_BUFFER_SIZE;
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

    /* Open the queue for RX, and create it if the sending process hasn't
    * already created it.
    */
    mqfd = mq_open(QUEUE_NAME_EP,open_flags,PMODE,&attr);
    if (mqfd == ( mqd_t ) -1)
    {
        TEST_ASSERT_TRUE(false);
        am_util_stdio_printf("\n Message Queue open failure from main. \n");
    }

    // Initialize Tx queue.

    /* Fill in attributes for message queue */
    // The maximum number of messages that can be stored on the queue.
    // In the QNX implementation, an mq_maxmsg of 0 means that there is no maximum
    attr_Tx.mq_maxmsg = 3;
    // The maximum size of each message on the given message queue.
    attr_Tx.mq_msgsize = MSPI_BUFFER_SIZE;
    // The options set for this queue. 0 implies blocking.
    attr_Tx.mq_flags   = 0;

    /* Set the flags for the open of the queue.
    * Make it a blocking open on the queue, meaning it will block if
    * this process tries to send to the queue and the queue is full.
    * (Absence of O_NONBLOCK flag implies that the open is blocking)
    *
    * Specify O_CREAT so that the file will get created if it does not
    * already exist.
    *
    * Specify O_WRONLY since we are only planning to write to the queue,
    * although we could specify O_RDWR also.
    */
    open_flags_Tx = O_WRONLY|O_CREAT;

    /* Open the queue for Tx, and create it if the receiving process hasn't
    * already created it.
    */
    mqfd_Tx = mq_open(QUEUE_NAME_PU,open_flags_Tx,PMODE_Tx,&attr_Tx);
    if (mqfd_Tx == ( mqd_t ) -1)
    {
        TEST_ASSERT_TRUE(false);
        am_util_stdio_printf("\n Message Queue open failure. \n");
    }

    // Initialize the Tx Queue sent Flag to 0 initially.
    psramDataSent = 0;

#ifdef TASK_LOOP_COUNT
    while(ui32Count < TASK_LOOP_COUNT)
#else
    while(1)
#endif
    {
        ui32Count ++;
        // Wait for data to be sent from EMMC Task
        while (emmcDataSent == 0);

        // Check if data is present in the queue from emmc task.
        if (emmcDataSent ==1)
        {
            // Read the queue.
            num_bytes_received = mq_receive(mqfd,ui8PSRAMTXBuffer,MSPI_BUFFER_SIZE,0);
            if (num_bytes_received == -1)
            {
                TEST_ASSERT_TRUE(false);
                am_util_stdio_printf("\n Failure to receive data from MSPI Queue.");
            }
            else
            {
                am_util_stdio_printf("\n Data read successfully from MSPI Queue. \n");
            }

		        am_util_stdio_printf("\n Writing %d Bytes to Sector %d\n", NumBytes, (SectorAddress + SectorOffset));

            ui32Status = am_devices_mspi_psram_aps25616n_ddr_write(g_pPsramHandle, ui8PSRAMTXBuffer, (SectorAddress + SectorOffset), NumBytes, true);

		        TEST_ASSERT_TRUE(AM_DEVICES_MSPI_PSRAM_STATUS_SUCCESS == ui32Status);
            if (AM_DEVICES_MSPI_PSRAM_STATUS_SUCCESS != ui32Status)
            {
                am_util_stdio_printf("Failed to write buffer to Flash Device!\n");
            }

            //
            //Clear RX buffer
            //
		        memset((void *)ui8PSRAMRXBuffer, 0x0, NumBytes);

            //
            // Read the data back into the RX buffer.
            //
            ui32Status = am_devices_mspi_psram_aps25616n_ddr_read(g_pPsramHandle, ui8PSRAMRXBuffer, (SectorAddress + SectorOffset), NumBytes, true);

            TEST_ASSERT_TRUE(AM_DEVICES_MSPI_PSRAM_STATUS_SUCCESS == ui32Status);
            if ( AM_HAL_MSPI_FIFO_FULL_CONDITION == ui32Status )
            {
                am_util_stdio_printf("FIFO full condition is detected!\n");
            }
            else if (AM_DEVICES_MSPI_PSRAM_STATUS_SUCCESS != ui32Status)
            {
                am_util_stdio_printf("Failed to read buffer from Flash Device!\n");
            }

            //
            // Check if data matched.
            //
            check_if_data_match_mspi((uint8_t *)ui8PSRAMTXBuffer, (uint8_t *)ui8PSRAMRXBuffer, MSPI_BUFFER_SIZE);

						// Wait for data to be sent from MSPI Task to be read by UART Task.
            while (psramDataSent == 1);

						// Set eMMC Data Sent Flag to 0 to receive next set of data
            emmcDataSent = 0;

						// Check if Emmac to PSRAM queue is read.
            if (psramDataSent == 0)
            {

                // Send the data read from eMMC over the queue.
                // Load the queue buffer
					      memcpy(msg_buffer, ui8PSRAMRXBuffer, MSPI_BUFFER_SIZE);

                num_bytes_to_send_Tx = MSPI_BUFFER_SIZE;
                priority_of_msg_Tx = 2;

                status_Tx = mq_send(mqfd_Tx,msg_buffer,num_bytes_to_send_Tx,priority_of_msg_Tx);
                if (status_Tx == -1)
                {
                    TEST_ASSERT_TRUE(false);
                    am_util_stdio_printf("\n Failure to send data to UART Queue \n");
                }
                else
                {
                    am_util_stdio_printf("\n Data successfully sent to UART Queue. \n");
                }
                am_util_delay_ms(100);

                // Set PSRAM Data Sent Flag to 1 to allow Data from Queue to be read.
                psramDataSent = 1;
            }
        }
	}

    // Indicate the Task loops are complete
    g_bMspiTaskComplete = true;

    //Wait for other tasks to complete
    vTaskDelay(1000);
    while (!g_bUartTaskComplete);

    am_util_stdio_printf("\n MSPI task end! \n");


    // Done with TX queue, so close it
    if (mq_close(mqfd_Tx) == -1)
    {
        TEST_ASSERT_TRUE(false);
        am_util_stdio_printf("\n mq_close failure on mqfd_Tx");
    }

    //
    // Need to make sure all pending XIPMM transactions are flushed
    //
    am_hal_cachectrl_dcache_invalidate(NULL, true);

    //
    // Deinit the PSRAM device.
    //
    ui32Status = am_devices_mspi_psram_aps25616n_ddr_deinit(g_pPsramHandle);
    TEST_ASSERT_TRUE(AM_DEVICES_MSPI_PSRAM_STATUS_SUCCESS == ui32Status);
    if (AM_DEVICES_MSPI_PSRAM_STATUS_SUCCESS != ui32Status)
    {
        am_util_stdio_printf("\n Failed to deinit the MSPI and Flash Device correctly!\n");
    }

    // Suspend and delete the task
    vTaskDelete(NULL);

}
