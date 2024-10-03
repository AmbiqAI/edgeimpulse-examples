//*****************************************************************************
//
//! @file iom_psram.c
//!
//! @brief Example that demostrates IOM, connecting to a SPI PSRAM
//! PSRAM is initialized with a known pattern data using Blocking IOM Write.
//! This example starts a 1 second timer. At each 1 second period, it initiates
//! reading a fixed size block from the PSRAM device using Non-Blocking IOM
//! Read, and comparing againts the predefined pattern
//!
//! Define one of PSRAM_DEVICE_ macros to select the PSRAM device
//!
//! SWO is configured in 1M baud, 8-n-1 mode.
//
//*****************************************************************************

//*****************************************************************************
//
// ${copyright}
//
// This is part of revision ${version} of the AmbiqSuite Development Package.
//
//*****************************************************************************
#include "nemagfx_enhanced_stress_test.h"

#if  ( IOMPSRAM_TASK_ENABLE == 1 )
#include "am_devices_spipsram.h"

//*****************************************************************************
// Customize the following for the test
//*****************************************************************************
// Control the example execution
//#define PSRAM_IOM_MODULE        0
#define IOM_MODULE_TOTAL                8   
uint32_t iom_test_instances[] = IOM_INSTANCE_TO_PSRAM;
#define IOM_TEST_INSTANCE_TOTAL         sizeof(iom_test_instances)/sizeof(iom_test_instances[0])   
#define PATTERN_BUF_SIZE                128
#define BUFFER_SIZE                     (1024*5)

//#define TCM_BUFFER_DATA_PATTERN (0xFF)
//#define DEBUG_PIN_1             (64)
//#define DEBUG_PIN_2             (65)
//#define DEBUG_PIN_3             (66)

//Undefine this if you want do dummy read.
#define IOM_PSRAM_DATA_CHECK
#define MAX_IOM_PSRAM_TASK_DELAY    (100*2)

//*****************************************************************************
//*****************************************************************************

//#define PSRAM_DEVICE_ID          AM_DEVICES_SPIPSRAM_KGD_PASS
#define am_iom_test_devices_t           am_devices_spipsram_config_t

//#define IOM_INTERRUPT1(n)               AM_HAL_INTERRUPT_IOMASTER ## n
//#define IOM_INTERRUPT(n)                IOM_INTERRUPT1(n)
//#define PSRAM_IOM_IRQn                  ((IRQn_Type)(IOMSTR0_IRQn + PSRAM_IOM_MODULE))
#define IOM_TEST_FREQ                   AM_HAL_IOM_16MHZ
//
// Typedef - to encapsulate device driver functions
//
typedef struct
{
    uint8_t  devName[20];
    uint32_t (*psram_init)(uint32_t ui32Module, am_iom_test_devices_t *pDevConfig, void **ppHandle, void **ppIomHandle);
    uint32_t (*psram_term)(void *pHandle);

    uint32_t (*psram_read_id)(void *pHandle, uint32_t *pDeviceID);

    uint32_t (*psram_nonblocking_write)(void *pHandle, uint8_t *ui8TxBuffer,
                                uint32_t ui32WriteAddress,
                                uint32_t ui32NumBytes,
                                am_hal_iom_callback_t pfnCallback,
                                void *pCallbackCtxt);

    uint32_t (*psram_nonblocking_read)(void *pHandle, uint8_t *pui8RxBuffer,
                                                      uint32_t ui32ReadAddress,
                                                      uint32_t ui32NumBytes,
                                                      am_hal_iom_callback_t pfnCallback,
                                                      void *pCallbackCtxt);
} psram_device_func_t;

//*****************************************************************************
//
// Global Variables
//
//*****************************************************************************
void                                    *g_IomDevHdl[IOM_MODULE_TOTAL];
void                                    *g_pIOMHandle[IOM_MODULE_TOTAL];
am_hal_iom_buffer(PATTERN_BUF_SIZE)     gPatternBuf;
am_hal_iom_buffer(PATTERN_BUF_SIZE)     gRxBuf[IOM_MODULE_TOTAL];
//static uint8_t                          u8TCMBuffer[BUFFER_SIZE];
//AM_SHARED_RW static uint8_t             ui8SsramBuffer[BUFFER_SIZE];

// Buffer for non-blocking transactions
uint32_t                                DMATCBBuffer[IOM_MODULE_TOTAL][1024];


psram_device_func_t device_func =
{
    // Fireball installed SPI PSRAM device
    .devName = "SPI PSRAM APS6404L",
    .psram_init = am_devices_spipsram_init_no_check,
    .psram_term = am_devices_spipsram_term,
    .psram_read_id = am_devices_spipsram_read_id,
    .psram_nonblocking_write = am_devices_spipsram_nonblocking_write,
    .psram_nonblocking_read = am_devices_spipsram_nonblocking_read,
};

static SemaphoreHandle_t g_semDMAFinish[IOM_MODULE_TOTAL] = {NULL, };

//
// Take over the interrupt handler for whichever IOM we're using.
//
#define psram_iom_isr                                                          \
    am_iom_isr1(PSRAM_IOM_MODULE)
#define am_iom_isr1(n)                                                        \
    am_iom_isr(n)
#define am_iom_isr(n)                                                         \
    am_iomaster ## n ## _isr

//*****************************************************************************
//
// GUI task handle.
//
//*****************************************************************************
TaskHandle_t IomPsramTaskHandle;

//*****************************************************************************
//
// IOM ISRs.
//
//*****************************************************************************
//
//! Take over default ISR. (Queue mode service)
//
//void psram_iom_isr(void)
//{
//    uint32_t ui32Status;
//
//    if (!am_hal_iom_interrupt_status_get(g_pIOMHandle, true, &ui32Status))
//    {
//        if ( ui32Status )
//        {
//            am_hal_iom_interrupt_clear(g_pIOMHandle, ui32Status);
//            am_hal_iom_interrupt_service(g_pIOMHandle, ui32Status);
//        }
//    }
//}

void am_iomaster0_isr(void)
{
    uint32_t ui32Status;

    if (!am_hal_iom_interrupt_status_get(g_pIOMHandle[0], true, &ui32Status))
    {
        if ( ui32Status )
        {
            am_hal_iom_interrupt_clear(g_pIOMHandle[0], ui32Status);
            am_hal_iom_interrupt_service(g_pIOMHandle[0], ui32Status);
        }
    }
}

void am_iomaster1_isr(void)
{
    uint32_t ui32Status;

    if (!am_hal_iom_interrupt_status_get(g_pIOMHandle[1], true, &ui32Status))
    {
        if ( ui32Status )
        {
            am_hal_iom_interrupt_clear(g_pIOMHandle[1], ui32Status);
            am_hal_iom_interrupt_service(g_pIOMHandle[1], ui32Status);
        }
    }
}
void am_iomaster2_isr(void)
{
    uint32_t ui32Status;

    if (!am_hal_iom_interrupt_status_get(g_pIOMHandle[2], true, &ui32Status))
    {
        if ( ui32Status )
        {
            am_hal_iom_interrupt_clear(g_pIOMHandle[2], ui32Status);
            am_hal_iom_interrupt_service(g_pIOMHandle[2], ui32Status);
        }
    }
}
void am_iomaster3_isr(void)
{
    uint32_t ui32Status;

    if (!am_hal_iom_interrupt_status_get(g_pIOMHandle[3], true, &ui32Status))
    {
        if ( ui32Status )
        {
            am_hal_iom_interrupt_clear(g_pIOMHandle[3], ui32Status);
            am_hal_iom_interrupt_service(g_pIOMHandle[3], ui32Status);
        }
    }
}
void am_iomaster4_isr(void)
{
    uint32_t ui32Status;

    if (!am_hal_iom_interrupt_status_get(g_pIOMHandle[4], true, &ui32Status))
    {
        if ( ui32Status )
        {
            am_hal_iom_interrupt_clear(g_pIOMHandle[4], ui32Status);
            am_hal_iom_interrupt_service(g_pIOMHandle[4], ui32Status);
        }
    }
}
void am_iomaster5_isr(void)
{
    uint32_t ui32Status;

    if (!am_hal_iom_interrupt_status_get(g_pIOMHandle[5], true, &ui32Status))
    {
        if ( ui32Status )
        {
            am_hal_iom_interrupt_clear(g_pIOMHandle[5], ui32Status);
            am_hal_iom_interrupt_service(g_pIOMHandle[5], ui32Status);
        }
    }
}
void am_iomaster6_isr(void)
{
    uint32_t ui32Status;

    if (!am_hal_iom_interrupt_status_get(g_pIOMHandle[6], true, &ui32Status))
    {
        if ( ui32Status )
        {
            am_hal_iom_interrupt_clear(g_pIOMHandle[6], ui32Status);
            am_hal_iom_interrupt_service(g_pIOMHandle[6], ui32Status);
        }
    }
}
void am_iomaster7_isr(void)
{
    uint32_t ui32Status;

    if (!am_hal_iom_interrupt_status_get(g_pIOMHandle[7], true, &ui32Status))
    {
        if ( ui32Status )
        {
            am_hal_iom_interrupt_clear(g_pIOMHandle[7], ui32Status);
            am_hal_iom_interrupt_service(g_pIOMHandle[7], ui32Status);
        }
    }
}



void
read_complete(void *pCallbackCtxt, uint32_t transactionStatus)
{
    uint32_t index = (uint32_t)pCallbackCtxt;

    if ( index >= IOM_MODULE_TOTAL )
    {
        am_util_stdio_printf("\nIOM ERROR index:%d!\n", index);
    }

    if (transactionStatus != AM_HAL_STATUS_SUCCESS)
    {
        am_util_stdio_printf("\nIOM%d: PSRAM Read Failed 0x%x\n", index, transactionStatus);
    }
    else
    {
        xSemaphoreGiveFromISR(g_semDMAFinish[index], NULL);
    }

}


//*****************************************************************************
//
// Main function.
//
//*****************************************************************************
void
IomPsramTask(void *pvParameters)
{
    uint32_t ui32Status;
    uint32_t i;
    uint32_t randomDelay;
    uint32_t instance;
    volatile uint32_t iom_test_instance_failure[sizeof(iom_test_instances) / sizeof(uint32_t)] = {0};

    //
    // Clear the terminal and print the banner.
    //
    am_util_stdio_printf("IOM PSRAM task start\n");

    for ( i = 0; i < IOM_TEST_INSTANCE_TOTAL; i++ )
    {    
        instance = iom_test_instances[i];
        //Create semphone for sync
        g_semDMAFinish[instance] = xSemaphoreCreateBinary();
        if ( g_semDMAFinish[instance] == NULL )
        {
            am_util_stdio_printf("IOM%d: Create semphone failed!\n", instance);
        }


    }
    for ( i = 0; i < IOM_TEST_INSTANCE_TOTAL; i++ )
    {
        instance = iom_test_instances[i];
        //
        // Enable the interrupt in the NVIC.
        //
        IRQn_Type irq_no = ((IRQn_Type)(IOMSTR0_IRQn + instance));

        NVIC_SetPriority(irq_no, IOM_ISR_PRIORITY);
        NVIC_ClearPendingIRQ(irq_no);
        NVIC_EnableIRQ(irq_no);      
    }

    //
    // fill data to buffer
    //
    for (i = 0; i < PATTERN_BUF_SIZE; i++)
    {
        gPatternBuf.bytes[i] = i & 0xFF;
    }

    //
    // Initialize the PSRAM Device
    //
    am_iom_test_devices_t stPsramConfig[IOM_MODULE_TOTAL];

    for ( i = 0; i < IOM_TEST_INSTANCE_TOTAL; i++ )
    {
        instance = iom_test_instances[i];
        
        stPsramConfig[instance].ui32ClockFreq = IOM_TEST_FREQ;
        stPsramConfig[instance].pNBTxnBuf = &DMATCBBuffer[instance][0];
        stPsramConfig[instance].ui32NBTxnBufLength = sizeof(DMATCBBuffer) / IOM_MODULE_TOTAL / 4;
        
        ui32Status = device_func.psram_init(instance, &stPsramConfig[instance], &g_IomDevHdl[instance], &g_pIOMHandle[instance]);
        if ( ui32Status != 0 )
        {
            am_util_stdio_printf("IOM%d: psram init Failed!\n", instance);
            iom_test_instance_failure[i] = 1;
            continue;
        }

        iom_test_instance_failure[i] = 0;

        ui32Status = device_func.psram_nonblocking_write(g_IomDevHdl[instance], &gPatternBuf.bytes[0], 0, PATTERN_BUF_SIZE, NULL, NULL);
        if ( ui32Status != 0 )
        {
            am_util_stdio_printf("IOM%d: psram write Failed!\n", instance);
        }

        vTaskDelay(10);
    }

    //
    // Loop forever.
    //
    while(1)
    {
        for (i = 0; i < IOM_TEST_INSTANCE_TOTAL; i++)
        {
            if(iom_test_instance_failure[i] == 1)
                continue;
            
            instance = iom_test_instances[i];
            memset(gRxBuf[instance].bytes, 0, PATTERN_BUF_SIZE);

            // Initiate read of a block of data from PSRAM
            ui32Status = device_func.psram_nonblocking_read(g_IomDevHdl[instance], &gRxBuf[instance].bytes[0], 0, PATTERN_BUF_SIZE, read_complete, (void *)instance);
            if ( ui32Status != 0 )
            {
                am_util_stdio_printf("[TASK] : IOM#%d: Start read failed!\n", instance);
            }
            else
            {
                am_util_stdio_printf("[TASK] : IOM#%d\n", instance);
            }

            //wait psram complete
            xSemaphoreTake( g_semDMAFinish[instance], portMAX_DELAY);

#ifdef IOM_PSRAM_DATA_CHECK
            // Verify Read PSRAM data  
            for (uint32_t j = 0; j < PATTERN_BUF_SIZE; j++)
            {
                if (gPatternBuf.bytes[j] != gRxBuf[instance].bytes[j])
                {
                    am_util_stdio_printf("IOM%d: Receive Data Compare failed at offset %d - Expected = 0x%x, Received = 0x%x\n",
                        instance, j,  gPatternBuf.bytes[j],  gRxBuf[instance].bytes[j]);
                }
            }
#endif
        }

        srand(xTaskGetTickCount());
        randomDelay = rand() % MAX_IOM_PSRAM_TASK_DELAY;
        vTaskDelay(randomDelay);
    }

    // Cleanup
    am_util_stdio_printf("\nIOM PSRAM task end!\n");

    // disable the interrupts in the NVIC.
    for ( i = 0; i < IOM_TEST_INSTANCE_TOTAL; i++ )
    {
        device_func.psram_term(g_IomDevHdl[iom_test_instances[i]]);
    }

    vTaskSuspend(NULL);
}

#endif  //( IOMPSRAM_TASK_ENABLE == 1 )
