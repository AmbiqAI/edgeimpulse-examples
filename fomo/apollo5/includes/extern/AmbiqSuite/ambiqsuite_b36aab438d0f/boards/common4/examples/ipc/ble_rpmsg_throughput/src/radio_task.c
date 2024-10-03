//*****************************************************************************
//
//! @file radio_task.c
//!
//! @brief Task to handle radio operation.
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
#include <stdio.h>

//*****************************************************************************
//
// WSF standard includes.
//
//*****************************************************************************
#include "wsf_types.h"
#include "wsf_trace.h"
#include "wsf_buf.h"
#include "wsf_timer.h"

//*****************************************************************************
//
// Includes for operating the ExactLE stack.
//
//*****************************************************************************
#include "hci_handler.h"
#include "dm_handler.h"
#include "l2c_handler.h"
#include "att_handler.h"
#include "smp_handler.h"
#include "l2c_api.h"
#include "att_api.h"
#include "smp_api.h"
#include "app_api.h"
#include "hci_core.h"
#include "hci_drv.h"
#include "hci_drv_apollo.h"
#include "hci_drv_ambt53.h"

#include "am_mcu_apollo.h"
#include "am_util.h"
#include "am_bsp.h"

#include "wsf_msg.h"
#include "ipc_service.h"
#include "rpc_client_sys_cntl.h"
#include "rpc_client_dbglog.h"
#include "rpc_client_audio.h"
#include "rpc_client_hci.h"

#include "throughput_api.h"
#include "app_ui.h"
#include "ble_rpmsg_throughput.h"
#include "rpmsg_config.h"
#include "host_ipc_instance.h"

//*****************************************************************************
//
// Global variable.
//
//*****************************************************************************
#if (!USE_CORDIO_HOST)
#define SEND_FREQ0               120 // 20 ticks per time
#endif
#define SEND_FREQ1               80  // 30 ticks per time
#define AM_TEST_REF_BUF_SIZE     496 // 512-16
#define XOR_BYTE                 0

#define DYNAMIC_QUEUE            1

#define RPMSG_QUEUE_HI           (VRING_SIZE - 1)
#define RPMSG_QUEUE_LOW          1

uint8_t g_pui8TestTXBuf[RPMSG_BUFFER_SIZE];

metal_thread_t radio_task_handle;
metal_thread_t rpmsg_recv_task_handle;
metal_thread_t audio_task_handle;

SemaphoreHandle_t g_RadioTxSem = NULL;
SemaphoreHandle_t g_AudioTxSem = NULL;

SemaphoreHandle_t g_inst0_rx_sem = NULL;
SemaphoreHandle_t g_inst1_rx_sem = NULL;

extern struct ipc_device host_inst0;
extern struct ipc_device host_inst1;

#define RECV_INTERVAL            5

#if (!USE_CORDIO_HOST)
static uint32_t ui32RadioTxIdx = 0;
#endif
static uint32_t ui32AudioTxIdx = 0;
uint32_t ui32AudioRxIdx = 0;


//*****************************************************************************
//
// Function prototypes
//
//*****************************************************************************
#if (USE_CORDIO_HOST)
void exactle_stack_init(void);
void button_handler(wsfEventMask_t event, wsfMsgHdr_t *pMsg);
void setup_buttons(void);
//*****************************************************************************
//
// Timer for buttons.
//
//*****************************************************************************
wsfHandlerId_t ButtonHandlerId;
wsfTimer_t ButtonTimer;

//*****************************************************************************
//
// WSF buffer pools.
//
//*****************************************************************************
#define WSF_BUF_POOLS               4

// Important note: the size of g_pui32BufMem should includes both overhead of internal
// buffer management structure, wsfBufPool_t (up to 16 bytes for each pool), and pool
// description (e.g. g_psPoolDescriptors below).

// Memory for the buffer pool
// #### INTERNAL BEGIN ####
#if defined(USE_NONBLOCKING_HCI)
AM_SHARED_RW static uint32_t g_pui32BufMem[
        (WSF_BUF_POOLS*16
         + 16*8 + 32*4 + 64*6 + 280*14) / sizeof(uint32_t)];
#else
// #### INTERNAL END ####
static uint32_t g_pui32BufMem[
        (WSF_BUF_POOLS*16
         + 16*8 + 32*4 + 64*6 + 280*14) / sizeof(uint32_t)];
// #### INTERNAL BEGIN ####
#endif
// #### INTERNAL END ####
// Default pool descriptor.
static wsfBufPoolDesc_t g_psPoolDescriptors[WSF_BUF_POOLS] =
{
    {  16,  8 },
    {  32,  4 },
    {  64,  6 },
    { 280,  14 }
};

//*****************************************************************************
//
// Tracking variable for the scheduler timer.
//
//*****************************************************************************

void radio_timer_handler(void);
#endif

#if (!USE_CORDIO_HOST)
// Timer0 Interrupt Service Routine (ISR)
void am_timer00_isr(void)
{
    BaseType_t xHigherPriorityTaskWoken;

    //
    // Clear Timer0 Interrupt (write to clear).
    //
    am_hal_timer_interrupt_clear(AM_HAL_TIMER_MASK(0, AM_HAL_TIMER_COMPARE0));
    am_hal_timer_clear_stop(0);

    xHigherPriorityTaskWoken = pdFALSE;

    xSemaphoreGiveFromISR(g_RadioTxSem, &xHigherPriorityTaskWoken);

    if (xHigherPriorityTaskWoken != pdFAIL)
    {
        portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
    }
}
#endif

// Timer2 Interrupt Service Routine (ISR)
void am_timer02_isr(void)
{
    BaseType_t xHigherPriorityTaskWoken;

    //
    // Clear Timer0 Interrupt (write to clear).
    //
    am_hal_timer_interrupt_clear(AM_HAL_TIMER_MASK(2, AM_HAL_TIMER_COMPARE0));
    am_hal_timer_clear_stop(2);

    xHigherPriorityTaskWoken = pdFALSE;

    xSemaphoreGiveFromISR(g_AudioTxSem, &xHigherPriorityTaskWoken);

    if (xHigherPriorityTaskWoken != pdFAIL)
    {
        portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
    }
}

#if (USE_CORDIO_HOST)
//*****************************************************************************
//
// Poll the buttons.
//
//*****************************************************************************
void
button_handler(wsfEventMask_t event, wsfMsgHdr_t *pMsg)
{
    //
    // Restart the button timer.
    //
    WsfTimerStartMs(&ButtonTimer, 10);

#if AM_BSP_NUM_BUTTONS
    //
    // Every time we get a button timer tick, check all of our buttons.
    //
    am_devices_button_array_tick(am_bsp_psButtons, AM_BSP_NUM_BUTTONS);

    //
    // If we got a a press, do something with it.
    //
    if ( am_devices_button_released(am_bsp_psButtons[0]) )
    {
        //am_util_debug_printf("Got Button 0 Press\n");
        AppUiBtnTest(APP_UI_BTN_1_SHORT);
    }

    if ( am_devices_button_released(am_bsp_psButtons[1]) )
    {
        am_util_debug_printf("Got Button 1 Press\n");
        AppUiBtnTest(APP_UI_BTN_1_MED);
    }
#endif
}


//*****************************************************************************
//
// Sets up a button interface.
//
//*****************************************************************************
void
setup_buttons(void)
{
#if AM_BSP_NUM_BUTTONS
    //
    // Enable the buttons for user interaction.
    //
    am_devices_button_array_init(am_bsp_psButtons, AM_BSP_NUM_BUTTONS);
#endif
    //
    // Start a timer.
    //
    ButtonTimer.handlerId = ButtonHandlerId;
    WsfTimerStartSec(&ButtonTimer, 2);
}

//*****************************************************************************
//
// Initialization for the ExactLE stack.
//
//*****************************************************************************
void
exactle_stack_init(void)
{
    wsfHandlerId_t handlerId;
    uint16_t       wsfBufMemLen;
    //
    // Set up timers for the WSF scheduler.
    //
    WsfOsInit();
    WsfTimerInit();

    //
    // Initialize a buffer pool for WSF dynamic memory needs.
    //
    wsfBufMemLen = WsfBufInit(sizeof(g_pui32BufMem), (uint8_t *)g_pui32BufMem, WSF_BUF_POOLS,
               g_psPoolDescriptors);

    if (wsfBufMemLen > sizeof(g_pui32BufMem))
    {
        am_util_debug_printf("Memory pool is too small by %d\r\n",
                             wsfBufMemLen - sizeof(g_pui32BufMem));
    }

    //
    // Initialize the WSF security service.
    //
    SecInit();
    SecAesInit();
    SecCmacInit();
    SecEccInit();

    //
    // Set up callback functions for the various layers of the ExactLE stack.
    //
    handlerId = WsfOsSetNextHandler(HciHandler);
    HciHandlerInit(handlerId);

    handlerId = WsfOsSetNextHandler(DmHandler);
    DmDevVsInit(0);
#if defined(PHY_TEST) && (PHY_TEST == 1)
    DmExtAdvInit();
#else
    DmAdvInit();
#endif
    DmPhyInit();
    DmConnInit();
    DmConnSlaveInit();
    DmSecInit();
    DmSecLescInit();
    DmPrivInit();
    DmHandlerInit(handlerId);

    handlerId = WsfOsSetNextHandler(L2cSlaveHandler);
    L2cSlaveHandlerInit(handlerId);
    L2cInit();
    L2cSlaveInit();

    handlerId = WsfOsSetNextHandler(AttHandler);
    AttHandlerInit(handlerId);
    AttsInit();
    AttsIndInit();
    AttcInit();

    handlerId = WsfOsSetNextHandler(SmpHandler);
    SmpHandlerInit(handlerId);
    SmprInit();
    SmprScInit();
    HciSetMaxRxAclLen(251);

    handlerId = WsfOsSetNextHandler(AppHandler);
    AppHandlerInit(handlerId);

    handlerId = WsfOsSetNextHandler(ThroughputHandler);
    ThroughputHandlerInit(handlerId);

    handlerId = WsfOsSetNextHandler(HciDrvHandler);
    HciDrvHandlerInit(handlerId);

    ButtonHandlerId = WsfOsSetNextHandler(button_handler);

}
#endif

//*****************************************************************************
//
// Function prototypes
//
//*****************************************************************************
int parse_data(uint8_t* data, size_t len, uint32_t* idx)
{
    uint32_t i = 0;
    uint8_t ref = 0;

    if ( (!len) || (!data) )
    {
        return RPMSG_ERR_PARAM;
    }

    for (i = 0; i < len; i++)
    {
        ref = (((i & 0xFF) ^ XOR_BYTE) + *idx) % 0x100;
        if (data[i] != ref)
        {
            am_util_stdio_printf("Comparison failed at Index %2d with Received Value %2.2X while expected value was %2.2X\n", i, data[i], ref);
            break;
        }
    }
    (*idx)++;
    if ( *idx == VRING_SIZE )
    {
        *idx = 0;
        am_util_stdio_printf("\n");
    }

    if ( i != len )
    {
        return RPMSG_ERR_DATA;
    }

    return RPMSG_SUCCESS;
}


//*****************************************************************************
//
// Perform initial setup for the radio task.
//
//*****************************************************************************
void
RadioTaskSetup(void)
{
    int status = 0;

    am_util_debug_printf("RadioTask: setup\r\n");
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

    //
    // Boot the RPMsg.
    //
    status = am_devices_ambt53_boot();
    if (status != 0)
    {
        am_util_stdio_printf("am_devices_ambt53_boot failure\n");
        return;
    }

    host_ipc_instance_setup();

    if ((g_inst0_rx_sem = xSemaphoreCreateBinary()) == NULL)
    {
        am_util_stdio_printf("g_inst0_rx_sem create failure\n");
        return;
    }

    if ((g_inst1_rx_sem = xSemaphoreCreateBinary()) == NULL)
    {
        am_util_stdio_printf("g_inst0_rx_sem create failure\n");
        return;
    }

    //
    // Create Semaphore
    //
    if ((g_RadioTxSem = xSemaphoreCreateBinary()) == NULL)
    {
        am_util_stdio_printf("Semaphore failed to alloc\n");
        return;
    }
    if ((g_AudioTxSem = xSemaphoreCreateBinary()) == NULL)
    {
        am_util_stdio_printf("Semaphore failed to alloc\n");
        return;
    }

    //
    // Initialize Test Data
    //
    for (int j = 0; j < RPMSG_BUFFER_SIZE; j++)
    {
        g_pui8TestTXBuf[j] = ((j & 0xFF) ^ XOR_BYTE) % 0x100;
    }

#if (!USE_CORDIO_HOST)
    //
    // Set up timer 0.
    //
    am_hal_timer_config_t       Timer0Config;
    am_hal_timer_default_config_set(&Timer0Config);
    Timer0Config.eInputClock = AM_HAL_TIMER_CLOCK_HFRC_DIV4K;   // 96MHz/4K = 24KHz
    Timer0Config.eFunction = AM_HAL_TIMER_FN_UPCOUNT;
    Timer0Config.ui32Compare0 = 24000 / SEND_FREQ0 ;

    am_hal_timer_config(0, &Timer0Config);
    am_hal_timer_clear_stop(0);

    am_hal_timer_interrupt_clear(AM_HAL_TIMER_MASK(0, AM_HAL_TIMER_COMPARE0));
    am_hal_timer_interrupt_enable(AM_HAL_TIMER_MASK(0, AM_HAL_TIMER_COMPARE0));

    NVIC_SetPriority(TIMER0_IRQn, AM_IRQ_PRIORITY_DEFAULT);

    NVIC_EnableIRQ(TIMER0_IRQn);
#endif

    //
    // Set up timer 2.
    //
    am_hal_timer_config_t       Time2Config;
    am_hal_timer_default_config_set(&Time2Config);
    Time2Config.eInputClock = AM_HAL_TIMER_CLOCK_HFRC_DIV4K;   // 96MHz/4K = 24KHz
    Time2Config.eFunction = AM_HAL_TIMER_FN_UPCOUNT;
    Time2Config.ui32Compare0 = 24000 / SEND_FREQ1;

    am_hal_timer_config(2, &Time2Config);
    am_hal_timer_clear_stop(2);

    am_hal_timer_interrupt_clear(AM_HAL_TIMER_MASK(2, AM_HAL_TIMER_COMPARE0));
    am_hal_timer_interrupt_enable(AM_HAL_TIMER_MASK(2, AM_HAL_TIMER_COMPARE0));

    NVIC_SetPriority(TIMER2_IRQn, AM_IRQ_PRIORITY_DEFAULT);

    NVIC_EnableIRQ(TIMER2_IRQn);
}

#if (USE_SYSIRQ == 0)
//*****************************************************************************
//
// Simulate the ipm interrupt
//
//*****************************************************************************
bool
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

uint32_t
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
        if (ui32Val == host_inst0.config.mbox_data_rcv)
        {
            ipc_receive(&host_inst0);
        }
        else if ( ui32Val == host_inst1.config.mbox_data_rcv )
        {
            ipc_receive(&host_inst1);
        }
    } while (1);
    return ui32Val;
}
#endif

//*****************************************************************************
//
// Short Description.
//
//*****************************************************************************
void
RadioTask(void *pvParameters)
{
    uint8_t ui8EpBindFlag = 0xFF;
    struct ipc_ept* rpmsg_syscntl_ep = (struct ipc_ept*)rpc_client_syscntl_ep_get();
    struct ipc_ept* rpmsg_debug_ep = (struct ipc_ept*)rpc_client_debug_ep_get();

    int status = RPMSG_SUCCESS;

#if WSF_TRACE_ENABLED == TRUE
    //
    // Enable ITM
    //
    am_util_debug_printf("Starting wicentric trace:\n\n");
#endif

#if (USE_CORDIO_HOST)
    //
    // Boot the radio.
    //
    HciDrvRadioBoot(1);

    //
    // Initialize the main ExactLE stack.
    //
    exactle_stack_init();

    //
    // Prep the buttons for use
    //
    setup_buttons();
#endif

    //
    // Wait for the response from NS setup and then send one packet to
    // sync with remote HCI RPMsg endpoint(s)
    //
    while (!ipc_service_is_endpoint_bounded(rpmsg_syscntl_ep))
    {
        metal_thread_yield();
    }
    status = ipc_service_send(rpmsg_syscntl_ep, &ui8EpBindFlag, 1);
    if (status < RPMSG_SUCCESS)
    {
        am_util_stdio_printf("sys cntl ep write bind flag fail\r\n");
    }

    while (!ipc_service_is_endpoint_bounded(rpmsg_debug_ep))
    {
        metal_thread_yield();
    }

    status = ipc_service_send(rpmsg_debug_ep, &ui8EpBindFlag, 1);
    if (status < RPMSG_SUCCESS)
    {
        am_util_stdio_printf("debug ep write bind flag fail\r\n");
    }

#if (USE_CORDIO_HOST)
    ThroughputStart();
#else
    am_hal_timer_start(0);
#endif

    am_util_debug_printf("Radio task starts..\n");

    while (1)
    {
#if (USE_CORDIO_HOST)
        //
        // Calculate the elapsed time from our free-running timer, and update
        // the software timers in the WSF scheduler.
        //
        wsfOsDispatcher();
#else // #if (USE_CORDIO_HOST)
        if (xSemaphoreTake(g_RadioTxSem, 0) == pdTRUE)
        {
            uint8_t txFrameNum = random_num(RPMSG_QUEUE_HI, RPMSG_QUEUE_LOW);
            //am_util_stdio_printf("%d packets sent\n", txFrameNum);
            for (int i = 0; i < txFrameNum; i++)
            {
                status = ipc_service_send(rpmsg_syscntl_ep, &g_pui8TestTXBuf[ui32RadioTxIdx], AM_TEST_REF_BUF_SIZE);
                if (status < RPMSG_SUCCESS)
                {
                    am_util_stdio_printf("radio send_message(%d) failed with status %d\n", ++ui32RadioTxIdx, status);
                }
                else
                {
                    ui32RadioTxIdx ++;
                }
                if (ui32RadioTxIdx == VRING_SIZE)
                {
                    ui32RadioTxIdx = 0;
                }
            }
            am_hal_timer_start(0);
        }
        metal_thread_yield();
#endif // #if (USE_CORDIO_HOST)
    }
}

#define RCV_TASK_DELAY_MS   5
void
RPMsgRecvTask(void *pvParameters)
{
    am_util_debug_printf("RPMsgRecvTask starts..\n");

    while(1)
    {
#if (USE_SYSIRQ)
        if (xSemaphoreTake(g_inst0_rx_sem, 0) == pdTRUE)
        {
            ipc_receive(&host_inst0);
        }
        else if (xSemaphoreTake(g_inst1_rx_sem, 0) == pdTRUE)
        {
            ipc_receive(&host_inst1);
        }
#else
        mailbox_flush();
#endif
        metal_thread_delay(&rpmsg_recv_task_handle, RECV_INTERVAL);
    }
}

void
AudioTask(void *pvParameters)
{
    uint8_t ui8EpBindFlag = 0xFF;

    struct ipc_ept* rpmsg_audio_ep = (struct ipc_ept*)rpc_client_audio_ep_get();

    /* Since we are using name service, we need to wait for a response
     * from NS setup and than we need to process it
     */
    while (!ipc_service_is_endpoint_bounded(rpmsg_audio_ep))
    {
        metal_thread_yield();
    }
    ipc_service_send(rpmsg_audio_ep, &ui8EpBindFlag, 1);

    // Start to send
    // am_hal_timer_start(2);

    am_util_debug_printf("Audio Task starts..\n");

    while(1)
    {
        if (xSemaphoreTake(g_AudioTxSem, 0) == pdTRUE)
        {
            int status = RPMSG_SUCCESS;
            uint8_t txFrameNum = random_num(RPMSG_QUEUE_HI, RPMSG_QUEUE_LOW);
            for (int i = 0; i < txFrameNum; i++)
            {
                status = ipc_service_send(rpmsg_audio_ep, &g_pui8TestTXBuf[ui32AudioTxIdx], AM_TEST_REF_BUF_SIZE);
                if (status < RPMSG_SUCCESS)
                {
                    am_util_stdio_printf("audio send_message(%d) failed with status %d\n", ++ui32AudioTxIdx, status);
                }
                else
                {
                    //am_util_stdio_printf("One packet send.\n");
                    ui32AudioTxIdx ++;
                }
                if (ui32AudioTxIdx == VRING_SIZE)
                {
                    ui32AudioTxIdx = 0;
                }
            }
            am_hal_timer_start(2);
        }
        metal_thread_yield();
    }
}

