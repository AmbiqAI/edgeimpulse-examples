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
#include "ble_freertos_hid.h"

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
#include "hci_drv_cooper.h"

#include "wsf_msg.h"

//*****************************************************************************
//
// Includes for the ExactLE "fit" profile.
//
//*****************************************************************************
#include "hid_api.h"
#include "hidapp_api.h"

#include "app_ui.h"

//*****************************************************************************
//
// Radio task handle.
//
//*****************************************************************************
TaskHandle_t radio_task_handle;

//*****************************************************************************
//
// Function prototypes
//
//*****************************************************************************
void exactle_stack_init(void);

void button_handler(wsfEventMask_t event, wsfMsgHdr_t *pMsg);
void setup_buttons(void);


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
static uint32_t g_pui32BufMem[
        (WSF_BUF_POOLS*16
         + 16*8 + 32*4 + 64*6 + 380*14) / sizeof(uint32_t)];

// Default pool descriptor.
static wsfBufPoolDesc_t g_psPoolDescriptors[WSF_BUF_POOLS] =
{
    {  16,  8 },
    {  32,  4 },
    {  64,  6 },
    { 380,  14 }
};

//*****************************************************************************
//
// Timer for buttons.
//
//*****************************************************************************
wsfHandlerId_t g_ButtonHandlerId;
wsfTimer_t     g_ButtonTimer;

//*****************************************************************************
//
// Tracking variable for the scheduler timer.
//
//*****************************************************************************

void radio_timer_handler(void);

#if AM_BSP_NUM_BUTTONS
static am_hal_gpio_pincfg_t g_button_pincfg_array[AM_BSP_NUM_BUTTONS] ;
static uint32_t g_button_release_action[AM_BSP_NUM_BUTTONS] ;
static bool buttonInterfaceSet = false  ;
#endif

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
    WsfTimerStartMs(&g_ButtonTimer, 10);
#if AM_BSP_NUM_BUTTONS
    //
    // Every time we get a button timer tick, check all of our buttons.
    //
    if ( !buttonInterfaceSet )
    {
       am_util_debug_printf("button_handler: WARNING: setup_buttons() not previously called\n" ) ;
       setup_buttons() ;
    }

    //
    // activate, read and debounce each button, then make them inactive again
    //
    am_devices_button_array_tick_pin_cfg(
        am_bsp_psButtons,
        g_button_pincfg_array,
        AM_BSP_NUM_BUTTONS ) ;

    //
    // If there has been a press and release, do something with it.
    //
    for ( uint32_t button_num = 0; button_num < AM_BSP_NUM_BUTTONS; button_num++ )
    {
        if ( am_devices_button_released(am_bsp_psButtons[button_num]))
        {
            am_util_debug_printf("Got Button %ld Press\n", (button_num + 1) ) ;
            AppUiBtnTest(g_button_release_action[button_num]);
        }
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
   // pull button pin cfg defs out of the bsp
   //
   g_button_pincfg_array[0] = g_AM_BSP_GPIO_BUTTON0 ;
   g_button_release_action[0] = APP_UI_BTN_1_SHORT ;
   #if AM_BSP_NUM_BUTTONS > 1
   g_button_pincfg_array[1] = g_AM_BSP_GPIO_BUTTON1 ;
   g_button_release_action[1] = APP_UI_BTN_2_SHORT ;
   #endif
   #if AM_BSP_NUM_BUTTONS > 2
   for ( uint32_t i = 2; i < AM_BSP_NUM_BUTTONS; i++ )
   {
      button_pincfg_array[i] = g_AM_BSP_GPIO_BUTTON2 ;
      g_button_release_action[1] = APP_UI_BTN_2_SHORT ;
   }
   #endif
   buttonInterfaceSet = true ;

    //
    // Enable the buttons for user interaction.
    //
    am_devices_button_array_init(am_bsp_psButtons, AM_BSP_NUM_BUTTONS);
#endif
    //
    // Start a timer.
    //
    g_ButtonTimer.handlerId = g_ButtonHandlerId;
    WsfTimerStartSec(&g_ButtonTimer, 2);
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
    DmAdvInit();
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

    handlerId = WsfOsSetNextHandler(HidAppHandler);
    HidAppHandlerInit(handlerId);

    handlerId = WsfOsSetNextHandler(HciDrvHandler);
    HciDrvHandlerInit(handlerId);
    g_ButtonHandlerId = WsfOsSetNextHandler(button_handler);
}

//*****************************************************************************
//
// GPIO interrupt handler.
//
//*****************************************************************************
void
am_cooper_irq_isr(void)
{
    uint32_t    ui32IntStatus;

    am_hal_gpio_interrupt_irq_status_get(AM_COOPER_IRQn, false, &ui32IntStatus);
    am_hal_gpio_interrupt_irq_clear(AM_COOPER_IRQn, ui32IntStatus);
    am_hal_gpio_interrupt_service(AM_COOPER_IRQn, ui32IntStatus);
}

//*****************************************************************************
//
// UART interrupt handler.
//
//*****************************************************************************
#if AM_BSP_UART_PRINT_INST == 0
void am_uart_isr(void)
#elif AM_BSP_UART_PRINT_INST == 1
void am_uart1_isr(void)
#elif AM_BSP_UART_PRINT_INST == 2
void am_uart2_isr(void)
#elif AM_BSP_UART_PRINT_INST == 3
void am_uart3_isr(void)
#endif
{
    uint32_t ui32Status;

    //
    // Read and save the interrupt status, but clear out the status register.
    //
    ui32Status = UARTn(AM_BSP_UART_PRINT_INST)->MIS;
    UARTn(AM_BSP_UART_PRINT_INST)->IEC = ui32Status;
}

//*****************************************************************************
//
// Perform initial setup for the radio task.
//
//*****************************************************************************
void
RadioTaskSetup(void)
{
    am_util_debug_printf("RadioTask: setup\r\n");

    NVIC_SetPriority(COOPER_IOM_IRQn, NVIC_configMAX_SYSCALL_INTERRUPT_PRIORITY);
    NVIC_SetPriority(AM_COOPER_IRQn, NVIC_configMAX_SYSCALL_INTERRUPT_PRIORITY);
}

//*****************************************************************************
//
// Short Description.
//
//*****************************************************************************
void
RadioTask(void *pvParameters)
{
#if WSF_TRACE_ENABLED == TRUE
    //
    // Enable ITM
    //
    am_util_debug_printf("Starting hid demo:\n\n");
#endif
    //
    // Boot the radio.
    //
    HciDrvRadioBoot(1);


    //
    // Initialize the main ExactLE stack.
    //
    exactle_stack_init();


    // uncomment the following to set custom Bluetooth address here
    // {
    //     uint8_t bd_addr[6] = {0x11, 0x22, 0x33, 0x44, 0x55, 0x66};
    //     HciVscSetCustom_BDAddr(&bd_addr[0]);
    // }


    //
    // Prep the buttons for use
    //
    setup_buttons();

    //
    // Start hid profile.
    //
    HidAppStart();

    while (1)
    {
        //
        // Calculate the elapsed time from our free-running timer, and update
        // the software timers in the WSF scheduler.
        //
        wsfOsDispatcher();

    }
}
