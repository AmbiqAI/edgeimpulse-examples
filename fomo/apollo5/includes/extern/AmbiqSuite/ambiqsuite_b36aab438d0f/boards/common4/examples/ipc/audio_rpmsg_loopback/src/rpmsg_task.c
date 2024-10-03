//*****************************************************************************
//
//! @file rpmsg_task.c
//!
//! @brief Task to handle rpmsg operation.
//!
//*****************************************************************************

//*****************************************************************************
//
// ${copyright}
//
// This is part of revision ${version} of the AmbiqSuite Development Package.
//
//*****************************************************************************
#include "ipc_service.h"
#include "../../../../../../third_party/open-amp/libmetal/lib/errno.h"
#include "audio_rpmsg_loopback.h"
#include "am_util_bootloader.h"

//*****************************************************************************
//
// Macro definitions.
//
//*****************************************************************************
#define     TIMER_0                (0)
#define     SEND_FREQ0             (120) // 20 ticks per time
#define     RECV_INTERVAL          (5)

#define     AUDIO_SEND_EVENT       (0x01)

#define     RPMSG_QUEUE_HI         (VRING_SIZE - 1)
#define     RPMSG_QUEUE_LOW        (1)
#define     AM_TEST_REF_BUF_SIZE   (496) // 512-16
#define     AUDIO_TEST_FRAME_BYTES (AM_TEST_REF_BUF_SIZE - 4) //2bytes index, 2bytes length
#define     XOR_BYTE               (0)

#define     USE_SYSIRQ             (0)

#define BYTES_TO_UINT16(n, p)     { n = ((uint16_t)(p)[0] + ((uint16_t)(p)[1] << 8)); }

#define AM_FLASH_PAGE_SIZE             (2*1024)
#define AM_RPMSG_READ_START_ADDRESS    (0x00100000)
#define AM_RPMSG_WRITE_START_ADDRESS   (0x00130000)
#define AM_RPMSG_TOTAL_TEST_BYTES      (0x10000)
#define WAIT_DATA_TIMEOUT_MS           (2000)
//*****************************************************************************
//
// Global variable.
//
//*****************************************************************************
metal_thread_t rpmsg_recv_task_handle;
metal_thread_t rpmsg_send_task_handle;

SemaphoreHandle_t g_audioRxSem = NULL;
EventGroupHandle_t g_sendEvent  = NULL;
TimerHandle_t g_waitDataTimer = NULL;
uint32_t wait_data_tmr_id;
bool is_waiting_data = false;

uint8_t g_pui8TestTXBuf[RPMSG_BUFFER_SIZE];
float g_failNum, g_failRate = 0;

struct ipc_ept ept;


typedef struct
{
    uint8_t     writeBuffer[AM_FLASH_PAGE_SIZE]   __attribute__((aligned(4)));   // needs to be 32-bit word aligned.
    uint16_t    bufferIndex;
} rpmsgFlashOp_t;

rpmsgFlashOp_t rpmsgFlash = {
    .bufferIndex = 0,
};

static uint16_t g_sendIndex = 0;
static uint16_t g_sendLength = 0;
static uint16_t g_recvIndex = 0;
static uint16_t g_recvLength = 0;
static uint32_t g_totalSend = 0;
static uint32_t g_readAddr = AM_RPMSG_READ_START_ADDRESS;
static uint32_t g_writeAddr = AM_RPMSG_WRITE_START_ADDRESS;

//*****************************************************************************
//
// Function prototypes
//
//*****************************************************************************

//Called from mailbox callback
static bool kick_cb(void* priv)
{
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    xSemaphoreGiveFromISR(g_audioRxSem, &xHigherPriorityTaskWoken);
    if (xHigherPriorityTaskWoken != pdFAIL)
    {
        portYIELD();
    }
    return false;
}

static void ept_bound(void *priv)
{
    (void)priv;
    am_util_stdio_printf("ept_bound\r\n");
}


static void ept_cb(const void *data, size_t len, void *priv)
{
    (void)priv;
    if ((data == NULL) || (len == 0))
    {
        return;
    }

    uint8_t *p_data = (uint8_t*)data;
    BYTES_TO_UINT16(g_recvIndex, p_data);
    BYTES_TO_UINT16(g_recvLength, (p_data + 2));
    p_data += 4;

    if (is_waiting_data)
    {
        is_waiting_data = false;
        xTimerStop(g_waitDataTimer, 0);
    }

    am_util_stdio_printf("idx %d, len %d\n", g_recvIndex, g_recvLength);
    if (rpmsgFlash.bufferIndex + g_recvLength < AM_FLASH_PAGE_SIZE)
    {
        memcpy(&rpmsgFlash.writeBuffer[rpmsgFlash.bufferIndex], p_data, g_recvLength);
        rpmsgFlash.bufferIndex += g_recvLength;
        is_waiting_data = true;
    }
    else
    {
        uint16_t remainingBytes = rpmsgFlash.bufferIndex + g_recvLength - AM_FLASH_PAGE_SIZE;
        memcpy(&rpmsgFlash.writeBuffer[rpmsgFlash.bufferIndex], p_data, (AM_FLASH_PAGE_SIZE - rpmsgFlash.bufferIndex));

        // Write the received data to MRAM by page
        am_util_bootloader_program_flash_page(g_writeAddr, (uint32_t*)&rpmsgFlash.writeBuffer[0], AM_FLASH_PAGE_SIZE);
        g_writeAddr += AM_FLASH_PAGE_SIZE;
        rpmsgFlash.bufferIndex = 0;
        memset(&rpmsgFlash.writeBuffer[0], 0, AM_FLASH_PAGE_SIZE);
        if (remainingBytes > 0)
        {
            memcpy(&rpmsgFlash.writeBuffer[0], (p_data + g_recvLength - remainingBytes), remainingBytes);
            rpmsgFlash.bufferIndex += remainingBytes;
            is_waiting_data = true;
        }
    }

    if (is_waiting_data)
    {
        xTimerStart(g_waitDataTimer, 0);
    }
}


//Define host IPC instance
struct ipc_device instance =
{
    .name = "AUD_RPMSG_INST",
    .config =
    {
        ROLE_HOST,
        CONFIG_IPC_SHM_BASE_ADDRESS_INST0,
        CONFIG_IPC_SHM_SIZE,
        (HOST_KICK_DATA_INST0 << 16),
        (HOST_KICK_DATA_INST0 << 16),
        &kick_cb,
    },
    .api = NULL,
    .data = NULL,
};

static struct ipc_ept_cfg ept_cfg =
{
    .name = "audio_rpmsg_ep",
    .cb = {
        .bound    = ept_bound,
        .received = ept_cb,
    }
};


// Timer0 Interrupt Service Routine (ISR)
void am_timer00_isr(void)
{
    BaseType_t xHigherPriorityTaskWoken;

    //
    // Clear Timer0 Interrupt (write to clear).
    //
    am_hal_timer_interrupt_clear(AM_HAL_TIMER_MASK(TIMER_0, AM_HAL_TIMER_COMPARE0));
    am_hal_timer_clear_stop(TIMER_0);

    xHigherPriorityTaskWoken = pdFALSE;

    xEventGroupSetBitsFromISR(g_sendEvent, AUDIO_SEND_EVENT,
                                        &xHigherPriorityTaskWoken);

    if (xHigherPriorityTaskWoken != pdFAIL)
    {
        portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
    }
}

//*****************************************************************************
//
// ambt53 MSPI Interrupt handler.
//
//*****************************************************************************
void
ambt53_mspi_isr(void)
{
    uint32_t ui32Status;

    am_hal_mspi_interrupt_status_get(pvRpmsgMspiHandle, &ui32Status, false);
    am_hal_mspi_interrupt_clear(pvRpmsgMspiHandle, ui32Status);
    am_hal_mspi_interrupt_service(pvRpmsgMspiHandle, ui32Status);
}


//*****************************************************************************
//
// Wait Data Timer Callback.
//
//*****************************************************************************
void vWaitDataTmrCallback(TimerHandle_t pxTimer)
{
    (void) pxTimer;

    // Write the received data to MRAM by page
    am_util_bootloader_program_flash_page(g_writeAddr, (uint32_t*)&rpmsgFlash.writeBuffer[0], AM_FLASH_PAGE_SIZE);
    g_writeAddr += AM_FLASH_PAGE_SIZE;
    rpmsgFlash.bufferIndex = 0;
    memset(&rpmsgFlash.writeBuffer[0], 0, AM_FLASH_PAGE_SIZE);
}


//*****************************************************************************
//
// Perform initial setup for the rpmsg task.
//
//*****************************************************************************
void
RPMsgTaskSetup(void)
{
    int status = 0;
    am_util_debug_printf("RPMsgTask: setup\r\n");
    // #### INTERNAL BEGIN ####
#ifdef AM_DEBUG_PRINTF
    //
    // Print some device information.
    //
    am_util_id_t sIdDevice;
    uint32_t ui32StrBuf;
    am_util_id_device(&sIdDevice);
    am_util_stdio_printf("Vendor Name: %s\n", sIdDevice.pui8VendorName);
    am_util_stdio_printf("Device type: %s\n",
         sIdDevice.pui8DeviceName);

    am_util_stdio_printf("Device Info:\n"
                         "\tPart number: 0x%08X\n"
                         "\tChip ID0:    0x%08X\n"
                         "\tChip ID1:    0x%08X\n"
                         "\tRevision:    0x%08X (Rev%c%c)\n",
                         sIdDevice.sMcuCtrlDevice.ui32ChipPN,
                         sIdDevice.sMcuCtrlDevice.ui32ChipID0,
                         sIdDevice.sMcuCtrlDevice.ui32ChipID1,
                         sIdDevice.sMcuCtrlDevice.ui32ChipRev,
                         sIdDevice.ui8ChipRevMaj, sIdDevice.ui8ChipRevMin );

    //
    // If not a multiple of 1024 bytes, append a plus sign to the KB.
    //
    ui32StrBuf = ( sIdDevice.sMcuCtrlDevice.ui32MRAMSize % 1024 ) ? '+' : 0;
    am_util_stdio_printf("\tMRAM size:   %7d (%d KB%s)\n",
                         sIdDevice.sMcuCtrlDevice.ui32MRAMSize,
                         sIdDevice.sMcuCtrlDevice.ui32MRAMSize / 1024,
                         &ui32StrBuf);

    ui32StrBuf = ( sIdDevice.sMcuCtrlDevice.ui32DTCMSize % 1024 ) ? '+' : 0;
    am_util_stdio_printf("\tDTCM size:   %7d (%d KB%s)\n",
                         sIdDevice.sMcuCtrlDevice.ui32DTCMSize,
                         sIdDevice.sMcuCtrlDevice.ui32DTCMSize / 1024,
                         &ui32StrBuf);

    ui32StrBuf = ( sIdDevice.sMcuCtrlDevice.ui32SSRAMSize % 1024 ) ? '+' : 0;
    am_util_stdio_printf("\tSSRAM size:  %7d (%d KB%s)\n\n",
                         sIdDevice.sMcuCtrlDevice.ui32SSRAMSize,
                         sIdDevice.sMcuCtrlDevice.ui32SSRAMSize / 1024,
                         &ui32StrBuf);
#endif // AM_DEBUG_PRINTF
    // #### INTERNAL END ####

    // Boot the rpmsg.
    status = am_devices_ambt53_boot();
    if (status != 0)
    {
        am_util_stdio_printf("am_devices_ambt53_boot failure\n");
        return;
    }

    if ((g_audioRxSem = xSemaphoreCreateBinary()) == NULL)
    {
        am_util_stdio_printf("g_audioRxSem create failure\n");
        return;
    }

    status = ipc_instance_create(&instance);
    if ((status < 0) && (status != -EALREADY))
    {
        am_util_stdio_printf("ipc_instance_create() failure\n");
        return;
    }

    status = ipc_service_register_endpoint(&instance, &ept, &ept_cfg);
    if (status < 0)
    {
        am_util_stdio_printf ("ipc_service_register_endpoint failure\n");
        return;
    }

    if ((g_sendEvent = xEventGroupCreate()) == NULL)
    {
        am_util_stdio_printf("event group failed to alloc");
        return;
    }

    // Create the timer to wait for the remaining data of each page. It is always used
    // for the scenario of the last frame of received data but the last page is not
    // full. Need to fill with the remaining fields to 0x00 and write them to MRAM by page.
    if ((g_waitDataTimer = xTimerCreate("WaitDataTimer",
                                        pdMS_TO_TICKS(WAIT_DATA_TIMEOUT_MS),
                                        pdFALSE,
                                        (void *)wait_data_tmr_id,
                                        vWaitDataTmrCallback)) == NULL)
    {
        am_util_stdio_printf("wait data timer failed to create");
        return;
    }

    // Initialize Test Data
    for (uint16_t j = 0; j < RPMSG_BUFFER_SIZE; j++)
    {
        g_pui8TestTXBuf[j] = 0;
    }

    //
    // Set up timer 0.
    //
    am_hal_timer_config_t       Timer0Config;
    am_hal_timer_default_config_set(&Timer0Config);
    Timer0Config.eInputClock = AM_HAL_TIMER_CLOCK_HFRC_DIV4K;   // 96MHz/4K = 24KHz
    Timer0Config.eFunction = AM_HAL_TIMER_FN_UPCOUNT;
    Timer0Config.ui32Compare0 = 24000 / SEND_FREQ0 ;

    am_hal_timer_config(TIMER_0, &Timer0Config);
    am_hal_timer_clear_stop(TIMER_0);

    am_hal_timer_interrupt_clear(AM_HAL_TIMER_MASK(TIMER_0, AM_HAL_TIMER_COMPARE0));
    am_hal_timer_interrupt_enable(AM_HAL_TIMER_MASK(TIMER_0, AM_HAL_TIMER_COMPARE0));

    NVIC_SetPriority(TIMER0_IRQn, AM_IRQ_PRIORITY_DEFAULT);

    NVIC_EnableIRQ(TIMER0_IRQn);
}

#if (USE_SYSIRQ == 0)
// Simulate the ipm interrupt
static bool
mailbox_check(void)
{
    uint32_t ui32Val = 0;
    am_devices_ambt53_mailbox_get_status(&ui32Val);
    // Check mailbox interrupt
    if ((ui32Val & MAILBOX_STATUS_D2M_EMPTY_Msk) == 0)
    {
        return true;
    }
    return false;
}

static void
mailbox_flush(void)
{
    uint32_t ui32Val, rd_len, ui32Status = 0;
    do
    {
        rd_len = 1;     //Read FIFO one by one
        am_devices_ambt53_mailbox_read_data(&ui32Val, &rd_len);
        // No more data in FIFO
        if ( rd_len == 0 )
        {
            break;
        }
        if (ui32Val == instance.config.mbox_data_rcv)
        {
            xSemaphoreGive(g_audioRxSem);
        }
    } while (1);
}
#endif

//*****************************************************************************
//
// Short Description.
//
//*****************************************************************************
void
RPMsgReceiveTask(void *pvParameters)
{
    while(1)
    {
#if (USE_SYSIRQ)
        if (xSemaphoreTake(g_audioRxSem, 0) == pdTRUE)
        {
            ipc_receive(&instance);
        }
#else
        if (mailbox_check())
        {
            mailbox_flush();
        }
        if (xSemaphoreTake(g_audioRxSem, 0) == pdTRUE)
        {
            ipc_receive(&instance);
        }
#endif
        metal_thread_delay(&rpmsg_recv_task_handle, RECV_INTERVAL);
    }
}

void
RPMsgSendTask(void *pvParameters)
{
    int status = RPMSG_SUCCESS;
    EventBits_t event = 0;

    /* Since we are using name service, we need to wait for a response
    * from NS setup and than we need to process it
    */
    while (!ipc_service_is_endpoint_bounded(&ept))
    {
        metal_thread_yield();
    }


    // Start to send
    am_hal_timer_start(TIMER_0);

    while(1)
    {
        event = xEventGroupWaitBits(g_sendEvent, AUDIO_SEND_EVENT, pdTRUE, pdFALSE, portMAX_DELAY);

        if (event == 0)
        {
            continue;
        }

        if (g_totalSend < AM_RPMSG_TOTAL_TEST_BYTES)
        {
            if (g_totalSend + AUDIO_TEST_FRAME_BYTES < AM_RPMSG_TOTAL_TEST_BYTES)
            {
                g_sendLength = AUDIO_TEST_FRAME_BYTES;
            }
            else
            {
                g_sendLength = AM_RPMSG_TOTAL_TEST_BYTES - g_totalSend;
            }

            g_pui8TestTXBuf[0] = (uint8_t)g_sendIndex;
            g_pui8TestTXBuf[1] = (uint8_t)(g_sendIndex >> 8);
            g_pui8TestTXBuf[2] = (uint8_t)g_sendLength;
            g_pui8TestTXBuf[3] = (uint8_t)(g_sendLength >> 8);

            memcpy(&g_pui8TestTXBuf[4], (uint8_t*)g_readAddr, g_sendLength);
            status = ipc_service_send(&ept, &g_pui8TestTXBuf[0], (g_sendLength + 4));
            if (status < RPMSG_SUCCESS)
            {
                am_util_stdio_printf("send fail\n");
            }
            else
            {
                am_util_stdio_printf("*");
                g_sendIndex ++;
                g_readAddr += g_sendLength;
                g_totalSend += g_sendLength;
            }
        }

        am_hal_timer_start(TIMER_0);
    }
}

