//*****************************************************************************
//
//! @file emmc_task.c
//!
//! @brief eMMC Task block read and write over SDIO and message tranfer to MSPI 
//!        Task over queue..
//!
//! Purpose: This example demonstrates how to read & write APIs with eMMC device. 
//!          The data is then transferred to MSPI task over a message queue.
//!
//! Additional Information:
//! To enable debug printing, add the following project-level macro definitions.
//!
//! AM_DEBUG_PRINTF
//!
//! When defined, debug messages will be sent over ITM/SWO at 1M Baud.
//!
//! Note that when these macros are defined, the device will never achieve deep
//! sleep, only normal sleep, due to the ITM (and thus the HFRC) being enabled.
//
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
// This application has a large number of common include files. For
// convenience, we'll collect them all together in a single header and include
// that everywhere.
//
//*****************************************************************************
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>

#include "audio_test_config.h"

#define ALIGN(x) __attribute__((aligned(1 << x)))

#define START_BLK 0
#define BLK_NUM 4
#define BUF_LEN 128 

// Queue Parameters.
#define QUEUE_NAME_EP  "/emmc_psram_queue"
#define PMODE 0666 

TaskHandle_t emmc_task_handle;
uint8_t emmcDataSent ;

static SemaphoreHandle_t g_semEMMCSetupFinish = NULL;

volatile uint8_t ui8RdBuf[BUF_LEN] ALIGN(12);
volatile uint8_t ui8WrBuf[BUF_LEN] ALIGN(12);

volatile bool g_bEmmcTaskComplete = false;
extern volatile bool g_bMspiTaskComplete;
extern volatile bool g_bUartTaskComplete;

am_hal_card_host_t *pSdhcCardHost = NULL;

//*****************************************************************************
//
// eMMC callback function.
//
//*****************************************************************************
void am_host_event_cb(am_hal_host_evt_t *pEvt)
{
    // Callback function.
}

//*****************************************************************************
//
// Data check.
//
//*****************************************************************************
void check_if_data_match(uint8_t *pui8RdBuf, uint8_t *pui8WrBuf, uint32_t ui32Len)
{
    uint32_t i;
    for (i = 0; i < ui32Len; i++)
    {
        if (pui8RdBuf[i] != pui8WrBuf[i])
        {
            am_util_stdio_printf("\n EMMC error: pui8RdBuf[%d] = %d and pui8WrBuf[%d] = %d\n", i, pui8RdBuf[i], i, pui8WrBuf[i]);
            break;
        }
    }

    if (i == ui32Len)
    {
			am_util_stdio_printf("\n Pass : EMMC SDIO TX and RX data matched. \n");
    }

}

//*****************************************************************************
//
// SDIO interrupt handler.
//
//*****************************************************************************
void am_sdio0_isr(void)
{
    uint32_t ui32IntStatus;

    am_hal_sdhc_intr_status_get(pSdhcCardHost->pHandle, &ui32IntStatus, true);
    am_hal_sdhc_intr_status_clear(pSdhcCardHost->pHandle, ui32IntStatus);
    am_hal_sdhc_interrupt_service(pSdhcCardHost->pHandle, ui32IntStatus);
}

//*****************************************************************************
//
// eMMC Task.
//
//*****************************************************************************

void EmmcTask(void *pvParameters)
{
    uint32_t ui32Status,i,num_bytes_to_send,priority_of_msg ;
	uint32_t ui32Count = 0;
    mqd_t mqfd;
    uint8_t msg_buffer[BUF_LEN];
    struct mq_attr attr;
    uint32_t open_flags = 0;
    uint8_t sdioNum ;
    int iStatus;
    
    //Create semphone for sync
    g_semEMMCSetupFinish = xSemaphoreCreateBinary();
    TEST_ASSERT_TRUE(g_semEMMCSetupFinish != NULL );
    if ( g_semEMMCSetupFinish == NULL )
    {
        am_util_stdio_printf("\n IOM%d: Create semphone failed!\n", i);
    }
    
    //
    // Clear the terminal and print the banner.
    //
    am_util_stdio_printf("\n eMMC task start\n");
    
    //
    // initialize the test read and write buffers
    //
    for (i = 0; i < BUF_LEN; i++)
    {
        ui8WrBuf[i] = i % BUF_LEN;
        ui8RdBuf[i] = 0x0;
    }
    
    am_hal_card_t eMMCard;
    
    
    // Wait for EMMC setup to complete
    xSemaphoreTake( g_semEMMCSetupFinish, ( TickType_t ) 0);

    //
    // Get the uderlying SDHC card host instance
    //
    pSdhcCardHost = am_hal_get_card_host(AM_HAL_SDHC_CARD_HOST, true);

    if (pSdhcCardHost == NULL)
    {
        TEST_ASSERT_TRUE(false);
        am_util_stdio_printf("\n No such card host and stop\n");
        vTaskDelete(NULL);
    }

    am_util_stdio_printf("\n Card host is found. \n");

    //
    // Configure SDIO PINs.
    //
	  sdioNum = 0; //SDIO0
    am_bsp_sdio_pins_enable(sdioNum, AM_HAL_HOST_BUS_WIDTH_8);

    //
    // check if card is present
    //
    if (am_hal_card_host_find_card(pSdhcCardHost, &eMMCard) != AM_HAL_STATUS_SUCCESS)
    {
        TEST_ASSERT_TRUE(false);
        am_util_stdio_printf("\n No card is present now. \n");
        vTaskDelete(NULL);
    }

    if (am_hal_card_init(&eMMCard, AM_HAL_CARD_TYPE_EMMC, NULL, AM_HAL_CARD_PWR_CTRL_SDHC_OFF) != AM_HAL_STATUS_SUCCESS)
    {
        TEST_ASSERT_TRUE(false);
        am_util_stdio_printf("\n Card and host not ready.\n");
        vTaskDelete(NULL);
    }

    //
    // set the card type to eMMC by using 48MHz and 8-bit mode for read and write
    //
    am_hal_card_cfg_set(&eMMCard, AM_HAL_CARD_TYPE_EMMC,
        AM_HAL_HOST_BUS_WIDTH_8, 500000, AM_HAL_HOST_BUS_VOLTAGE_1_8,
        AM_HAL_HOST_UHS_SDR50);

    //
    // Register callback regardless
    //
    am_hal_card_register_evt_callback(&eMMCard, am_host_event_cb);

    emmcDataSent = 0;
   
    /* Fill in attributes for message queue */
    // The maximum number of messages that can be stored on the queue. 
    // In the QNX implementation, an mq_maxmsg of 0 means that there is no maximum    
    attr.mq_maxmsg = 10;  
    // The maximum size of each message on the given message queue.
    attr.mq_msgsize = BUF_LEN;
    // The options set for this queue. 0 implies blocking.
    attr.mq_flags   = 0;

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
    open_flags = O_WRONLY|O_CREAT;

    /* Open the queue, and create it if the receiving process hasn't
    * already created it.
    */
    mqfd = mq_open(QUEUE_NAME_EP,open_flags,PMODE,&attr);
    if (mqfd == ( mqd_t ) -1)
    {
        TEST_ASSERT_TRUE(false);
        am_util_stdio_printf("\n MSIP Queue open failure \n");
    };    

    // EMMC setup complete, end critical section.
    xSemaphoreGive(g_semEMMCSetupFinish); 
    
#ifdef TASK_LOOP_COUNT 
    while(ui32Count < TASK_LOOP_COUNT)
#else
    while(1)
#endif
    {   
        ui32Count ++;
        // Wait for data sent from EMMC task to MSPI task to be read.
        while(emmcDataSent == 1);

        //
        // power down SDIO peripheral
        //
        ui32Status = am_hal_card_pwrctrl_sleep(&eMMCard);
        if(ui32Status != AM_HAL_STATUS_SUCCESS)
        {
            am_util_stdio_printf("\n Failed to power down card. \n");
        }

        //
        // power up SDIO peripheral
        //
        ui32Status = am_hal_card_pwrctrl_wakeup(&eMMCard);
        if(ui32Status != AM_HAL_STATUS_SUCCESS)
        {
            am_util_stdio_printf("\n Failed to power up card.\n");
        }
		
        //
        // write 512 bytes to emmc flash
        //
        ui32Status = am_hal_card_block_write_sync(&eMMCard, START_BLK, 1, (uint8_t *)ui8WrBuf);
        if ((ui32Status & 0xFFFF) != AM_HAL_STATUS_SUCCESS)
        {
            TEST_ASSERT_TRUE(false);
            am_util_stdio_printf("\n eMMC Write failed.\n");
        }
		
		//
        // power down SDIO peripheral
        //
        ui32Status = am_hal_card_pwrctrl_sleep(&eMMCard);
        if(ui32Status != AM_HAL_STATUS_SUCCESS)
        {
            am_util_stdio_printf("\n Failed to power down card\n");
        }

        //
        // power up SDIO peripheral
        //
        ui32Status = am_hal_card_pwrctrl_wakeup(&eMMCard);
        if(ui32Status != AM_HAL_STATUS_SUCCESS)
        {
            am_util_stdio_printf("\n Failed to power up card.\n");
        }

        //
        // read back the first block of emmc flash
        //
        memset((void *)ui8RdBuf, 0x0, BUF_LEN);

        ui32Status = am_hal_card_block_read_sync(&eMMCard, START_BLK, 1, (uint8_t *)ui8RdBuf);
        if ((ui32Status & 0xFFFF) != AM_HAL_STATUS_SUCCESS)
        {
            TEST_ASSERT_TRUE(false);
            am_util_stdio_printf("\n eMMC Read failed.\n");
        }

        //
        // check if block data match or not
        //
        check_if_data_match((uint8_t *)ui8RdBuf, (uint8_t *)ui8WrBuf, 512);
        			
				// Check if Emmac to PSRAM queue is read.
        if (emmcDataSent == 0)
        {

            // Send the data read from eMMC over the queue.
            // Load the queue buffer
            memcpy(msg_buffer, (const void*)ui8RdBuf, BUF_LEN);
  
            num_bytes_to_send = BUF_LEN;
            priority_of_msg = 1;

            iStatus = mq_send(mqfd,(const char*)msg_buffer,num_bytes_to_send,priority_of_msg);
            if (iStatus == -1)
            {
                TEST_ASSERT_TRUE(false);
                am_util_stdio_printf("\n MSPI Queue send failure. \n");
            }
            else
            {
                am_util_stdio_printf("\n Successful sent data to MSPI Task. \n");
            } 
            vTaskDelay(1000); 
            
            // Set eMMC Data Sent Flag to 1
            emmcDataSent = 1;
        }
    }
		
    // Indicate the Task loops are complete
    g_bEmmcTaskComplete = true;
		
    //Wait for other tasks to complete
    vTaskDelay(1000);
    while (!g_bMspiTaskComplete && !g_bUartTaskComplete);
		
    // Done with queue, so close it 
    if (mq_close(mqfd) == -1)
    {
        TEST_ASSERT_TRUE(false);
        am_util_stdio_printf("\n MSPI Queue failure to Close.\n");
    }

    am_util_stdio_printf("\n About to exit the sending process after closing the queue \n");
    
    am_util_stdio_printf("\n eMMC task end!\n");
    
    //Deinit the emmc.
    
    ui32Status = am_hal_card_deinit(&eMMCard);
    TEST_ASSERT_TRUE(AM_HAL_STATUS_SUCCESS == ui32Status);
    if (AM_HAL_STATUS_SUCCESS != ui32Status)
    {
        am_util_stdio_printf("\n Failed to deinit eMMC correctly!\n");
    }

    // Suspend and delete the task		
    vTaskDelete(NULL);

}

