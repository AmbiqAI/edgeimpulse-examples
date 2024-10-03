//*****************************************************************************
//
//! @file cordio_power_cycle.c
//!
//! @brief Cordio Power Cycle EM9304 Example
//!
//
//*****************************************************************************

//*****************************************************************************
//
// ${copyright}
//
// This is part of revision ${version} of the AmbiqSuite Development Package.
//
//*****************************************************************************

#include <stdint.h>
#include <stdbool.h>

#include "wsf_types.h"
#include "wsf_trace.h"
#include "wsf_buf.h"

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

#include "am_mcu_apollo.h"
#include "am_util.h"
#include "am_bsp.h"

#include "hci_apollo_config.h"

#include "tag_api.h"
#include "app_ui.h"

#include "wsf_msg.h"

//*****************************************************************************
//
// Forward declarations.
//
//*****************************************************************************
void exactle_stack_init(void);
void scheduler_timer_init(void);
void update_scheduler_timers(void);
void set_next_wakeup(void);
void generic_handler(wsfEventMask_t event, wsfMsgHdr_t *pMsg);
extern void AppUiBtnTest(uint8_t btn);

//*****************************************************************************
//
// UART configuration settings.
//
//*****************************************************************************
am_hal_uart_config_t g_sUartConfig =
{
    .ui32BaudRate = 115200,
    .ui32DataBits = AM_HAL_UART_DATA_BITS_8,
    .bTwoStopBits = false,
    .ui32Parity   = AM_HAL_UART_PARITY_NONE,
    .ui32FlowCtrl = AM_HAL_UART_FLOW_CTRL_NONE,
};

#define MAX_UART_PACKET_SIZE            2048
#define HCI_BRIDGE_UART                 0

//*****************************************************************************
//
// Global Variables
//
//*****************************************************************************
uint8_t g_pui8UARTTXBuffer[MAX_UART_PACKET_SIZE];
uint8_t g_pui8UARTRXBuffer[MAX_UART_PACKET_SIZE];
volatile bool g_bRxTimeoutFlag = false;

//*****************************************************************************
//
// Timer for buttons.
//
//*****************************************************************************
wsfHandlerId_t GenericHandlerId;
wsfTimer_t     ButtonTimer;

#define BUTTON_TIMER_EVENT 0xA0
//*****************************************************************************
//
// Timer for power cycle BLE.
//
//*****************************************************************************

wsfTimer_t PowerCycleTimer;
#define POWERCYCLE_TIMER_EVENT 0xA1

//*****************************************************************************
//
// Timer configuration macros.
//
//*****************************************************************************
#define MS_PER_TIMER_TICK           10  // Milliseconds per WSF tick
#define CLK_TICKS_PER_WSF_TICKS     5   // Number of CTIMER counts per WSF tick.

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
static uint32_t g_pui32BufMem[(1024 + 1024) / sizeof(uint32_t)];

// Default pool descriptor.
static wsfBufPoolDesc_t g_psPoolDescriptors[WSF_BUF_POOLS] =
{
    {  16,  8 },
    {  32,  4 },
    {  64,  6 },
    { 280,  4 }
};

wsfHandlerId_t g_bleDataReadyHandlerId;

bool ble_on = false;

//*****************************************************************************
//
// Tracking variable for the scheduler timer.
//
//*****************************************************************************
uint32_t g_ui32LastTime = 0;

//*****************************************************************************
//
// Initialization for the ExactLE stack.
//
//*****************************************************************************
void
exactle_stack_init(void)
{
    wsfHandlerId_t handlerId;

    //
    // Set up timers for the WSF scheduler.
    //
    scheduler_timer_init();
    WsfTimerInit();

    //
    // Initialize a buffer pool for WSF dynamic memory needs.
    //
    WsfBufInit(sizeof(g_pui32BufMem), (uint8_t*)g_pui32BufMem, WSF_BUF_POOLS, g_psPoolDescriptors);

    //
    // Initialize security.
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

    handlerId = WsfOsSetNextHandler(TagHandler);
    TagHandlerInit(handlerId);

    GenericHandlerId = WsfOsSetNextHandler(generic_handler);
}

//*****************************************************************************
//
// Set up a pair of timers to handle the WSF scheduler.
//
//*****************************************************************************
void
scheduler_timer_init(void)
{
    //
    // One of the timers will run in one-shot mode and provide interrupts for
    // scheduled events.
    //
    am_hal_ctimer_clear(0, AM_HAL_CTIMER_TIMERA);
    am_hal_ctimer_config_single(0, AM_HAL_CTIMER_TIMERA,
                                (AM_HAL_CTIMER_INT_ENABLE |
                                 AM_HAL_CTIMER_LFRC_512HZ |
                                 AM_HAL_CTIMER_FN_ONCE));

    //
    // The other timer will run continuously and provide a constant time-base.
    //
    am_hal_ctimer_clear(0, AM_HAL_CTIMER_TIMERB);
    am_hal_ctimer_config_single(0, AM_HAL_CTIMER_TIMERB,
                                 (AM_HAL_CTIMER_LFRC_512HZ |
                                 AM_HAL_CTIMER_FN_CONTINUOUS));

    //
    // Start the continuous timer.
    //
    am_hal_ctimer_start(0, AM_HAL_CTIMER_TIMERB);

    //
    // Enable the timer interrupt.
    //
    am_hal_ctimer_int_enable(AM_HAL_CTIMER_INT_TIMERA0);
    am_hal_interrupt_enable(AM_HAL_INTERRUPT_CTIMER);
}

//*****************************************************************************
//
// Calculate the elapsed time, and update the WSF software timers.
//
//*****************************************************************************
void
update_scheduler_timers(void)
{
    uint32_t ui32CurrentTime, ui32ElapsedTime;

    //
    // Read the continuous timer.
    //
    ui32CurrentTime = am_hal_ctimer_read(0, AM_HAL_CTIMER_TIMERB);

    //
    // Figure out how long it has been since the last time we've read the
    // continuous timer. We should be reading often enough that we'll never
    // have more than one overflow.
    //
    ui32ElapsedTime = (ui32CurrentTime >= g_ui32LastTime ?
                       (ui32CurrentTime - g_ui32LastTime) :
                       (0x10000 + ui32CurrentTime - g_ui32LastTime));

    //
    // Check to see if any WSF ticks need to happen.
    //
    if ( (ui32ElapsedTime / CLK_TICKS_PER_WSF_TICKS) > 0 )
    {
        //
        // Update the WSF timers and save the current time as our "last
        // update".
        //
        WsfTimerUpdate(ui32ElapsedTime / CLK_TICKS_PER_WSF_TICKS);

        g_ui32LastTime = ui32CurrentTime;
    }
}

//*****************************************************************************
//
// Set a timer interrupt for the next upcoming scheduler event.
//
//*****************************************************************************
void
set_next_wakeup(void)
{
    bool_t bTimerRunning;
    wsfTimerTicks_t xNextExpiration;

    //
    // Stop and clear the scheduling timer.
    //
    am_hal_ctimer_stop(0, AM_HAL_CTIMER_TIMERA);
    am_hal_ctimer_clear(0, AM_HAL_CTIMER_TIMERA);

    //
    // Check to see when the next timer expiration should happen.
    //
    xNextExpiration = WsfTimerNextExpiration(&bTimerRunning);

    //
    // If there's a pending WSF timer event, set an interrupt to wake us up in
    // time to service it. Otherwise, set an interrupt to wake us up in time to
    // prevent a double-overflow of our continuous timer.
    //
    if ( xNextExpiration )
    {
        am_hal_ctimer_period_set(0, AM_HAL_CTIMER_TIMERA,
                                 xNextExpiration * CLK_TICKS_PER_WSF_TICKS, 0);
    }
    else
    {
        am_hal_ctimer_period_set(0, AM_HAL_CTIMER_TIMERA, 0x8000, 0);
    }

    //
    // Start the scheduling timer.
    //
    am_hal_ctimer_start(0, AM_HAL_CTIMER_TIMERA);
}

//*****************************************************************************
//
// Poll the buttons.
//
//*****************************************************************************
void
generic_handler(wsfEventMask_t event, wsfMsgHdr_t *pMsg)
{
    if ( pMsg->event == BUTTON_TIMER_EVENT)
    {
        //
        // Restart the button timer.
        //
        WsfTimerStartMs(&ButtonTimer, 10);

        //
        // Every time we get a button timer tick, check all of our buttons.
        //
        am_devices_button_array_tick(am_bsp_psButtons, AM_BSP_NUM_BUTTONS);

        //
        // If we got a a press, do something with it.
        //
        if ( am_devices_button_released(am_bsp_psButtons[0]) )
        {
          am_util_debug_printf("Got Button 0 Press\n");
          AppUiBtnTest(APP_UI_BTN_1_SHORT);
        }

        if ( am_devices_button_released(am_bsp_psButtons[1]) )
        {
          am_util_debug_printf("Got Button 1 Press\n");
          AppUiBtnTest(APP_UI_BTN_1_SHORT);
        }

        if ( am_devices_button_released(am_bsp_psButtons[2]) )
        {
          am_util_debug_printf("Got Button 2 Press\n");
        }
    }

    if (pMsg->event == POWERCYCLE_TIMER_EVENT)
    {
        // restart timer
        if ( ble_on == true )
        {
            dmConnId_t  connId;

            WsfTimerStartSec(&PowerCycleTimer, 1);

            if ((connId = AppConnIsOpen()) != DM_CONN_ID_NONE)
            {
                AppConnClose(connId);
                return;
            }

            if ( AppSlaveIsAdvertising() == true )
            {
                AppAdvStop();
                return;
            }
            am_util_debug_printf("Power off em9304\n");
            HciDrvRadioShutdown();
            ble_on = false;
        }
        else
        {
            am_util_debug_printf("Power on em9304\n");
            HciDrvRadioBoot(0);
            DmDevReset();
            ble_on = true;
            WsfTimerStartSec(&PowerCycleTimer, 10);
        }
    }
}

//*****************************************************************************
//
// Sets up a button interface.
//
//*****************************************************************************
void
setup_buttons(void)
{
    //
    // Enable the buttons for user interaction.
    //
    am_devices_button_array_init(am_bsp_psButtons, AM_BSP_NUM_BUTTONS);

    //
    // Start a timer.
    //
    ButtonTimer.handlerId = GenericHandlerId;
    ButtonTimer.msg.event = BUTTON_TIMER_EVENT;

    WsfTimerStartSec(&ButtonTimer, 2);


    //
    // Start a timer.
    //
    PowerCycleTimer.handlerId = GenericHandlerId;
    PowerCycleTimer.msg.event = POWERCYCLE_TIMER_EVENT;

    WsfTimerStartSec(&PowerCycleTimer, 4);

}

//*****************************************************************************
//
// Interrupt handler for the CTIMERs
//
//*****************************************************************************
void
am_ctimer_isr(void)
{
    uint32_t ui32Status;

    //
    // Check and clear any active CTIMER interrupts.
    //
    ui32Status = am_hal_ctimer_int_status_get(true);
    am_hal_ctimer_int_clear(ui32Status);
}

//*****************************************************************************
//
// UART initialization.
//
//*****************************************************************************
void
uart_init(uint32_t ui32Module)
{
    //
    // Make sure the UART RX and TX pins are enabled.
    //
    am_bsp_pin_enable(COM_UART_TX);
    am_bsp_pin_enable(COM_UART_RX);

    //
    // Power on the selected UART
    //
    am_hal_uart_pwrctrl_enable(ui32Module);

    //
    // Start the UART interface, apply the desired configuration settings, and
    // enable the FIFOs.
    //
    am_hal_uart_clock_enable(ui32Module);

    //
    // Disable the UART before configuring it.
    //
    am_hal_uart_disable(ui32Module);

    //
    // Configure the UART.
    //
    am_hal_uart_config(ui32Module, &g_sUartConfig);

    //
    // Enable the UART FIFO.
    //
    am_hal_uart_fifo_config(ui32Module, AM_HAL_UART_TX_FIFO_1_2 |
                                        AM_HAL_UART_RX_FIFO_1_2);

    //
    // Enable the UART.
    //
    am_hal_uart_enable(ui32Module);
    am_hal_uart_int_enable(ui32Module, AM_HAL_UART_INT_RX_TMOUT |
                                       AM_HAL_UART_INT_RX       |
                                       AM_HAL_UART_INT_TXCMP);

    //
    // Initialize the buffered UART.
    //
    am_hal_uart_init_buffered(ui32Module, g_pui8UARTRXBuffer,
                              MAX_UART_PACKET_SIZE,
                              g_pui8UARTTXBuffer, MAX_UART_PACKET_SIZE);

    //
    // Enable the UART interrupt handler.
    //
    am_hal_interrupt_enable(AM_HAL_INTERRUPT_UART + ui32Module);
}

//*****************************************************************************
//
// Interrupt handler for the UART.
//
//*****************************************************************************
void
am_uart_isr(void)
{
    uint32_t ui32Status;

    //
    // Read the masked interrupt status from the UART.
    //
    ui32Status = am_hal_uart_int_status_get(HCI_BRIDGE_UART, true);

    //
    // Clear the UART interrupts.
    //
    am_hal_uart_int_clear(HCI_BRIDGE_UART, ui32Status);

    //
    // If there are TMOUT, RX or TX interrupts then service them.
    //
    if (ui32Status & (AM_HAL_UART_INT_RX_TMOUT | AM_HAL_UART_INT_TX | AM_HAL_UART_INT_RX))
    {
        am_hal_uart_service_buffered_timeout_save(HCI_BRIDGE_UART, ui32Status);
    }

    //
    // If there is a TMOUT interrupt, then indicate that to the main routine.
    //
    if (ui32Status & (AM_HAL_UART_INT_RX_TMOUT))
    {
        g_bRxTimeoutFlag = true;
    }
}


//*****************************************************************************
//
// Interrupt handler for BLE
//
//*****************************************************************************
void
am_ble_isr(void)
{
    HciDrvIntService();
}

//*****************************************************************************
//
// Main function.
//
//*****************************************************************************
int
main(void)
{
    //
    // Configure the system clock to run at 24 MHz
    //
    am_hal_clkgen_control(AM_HAL_CLKGEN_CONTROL_SYSCLK_MAX, 0);

    //
    // Configure the MCU for low power operation, but leave the LFRC on.
    //
    am_hal_pwrctrl_bucks_enable();

#ifdef AM_DEBUG_PRINTF
//    am_util_stdio_printf_init((am_util_stdio_print_char_t) am_bsp_itm_string_print);
//    am_bsp_pin_enable(ITM_SWO);
//    am_hal_itm_enable();
//    am_bsp_debug_printf_enable();
    am_util_stdio_printf_init((am_util_stdio_print_char_t)am_bsp_uart_string_print);
    am_bsp_uart_printf_init(AM_BSP_UART_PRINT_INST, (am_hal_uart_config_t *)0);
    uart_init(0);
#endif

    am_util_debug_printf("Apollo2 Cordio Power Cycle Example\n");

    //
    // Boot the radio.
    //
    HciDrvRadioBoot(0);

    //
    // Initialize the main ExactLE stack.
    //
    exactle_stack_init();

    //
    // Enable BLE data ready interrupt
    //
    am_hal_interrupt_enable(AM_HAL_INTERRUPT_GPIO);

    //
    // Prep the buttons for use
    //
    setup_buttons();

    //
    // Start the "Tag" profile.
    //
    TagStart();

    ble_on = true;
    while (TRUE)
    {
        //
        // Calculate the elapsed time from our free-running timer, and update
        // the software timers in the WSF scheduler.
        //
        update_scheduler_timers();
        wsfOsDispatcher();

        //
        // Enable an interrupt to wake us up next time we have a scheduled event.
        //
        set_next_wakeup();

        am_hal_interrupt_master_disable();

        //
        // Check to see if the WSF routines are ready to go to sleep.
        //
        if ( wsfOsReadyToSleep() )
        {
            am_hal_sysctrl_sleep(AM_HAL_SYSCTRL_SLEEP_DEEP);
        }
        am_hal_interrupt_master_enable();
    }
}
