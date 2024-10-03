//*****************************************************************************
//
//! @file cordio_tag.c
//!
//! @brief Cordio Proximity Tag Example
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

#include "tag_api.h"
#include "app_ui.h"

#include "wsf_msg.h"

//*****************************************************************************
//
// Watch registers.
//
//*****************************************************************************
#define WATCH_REGn(module, instance, reg)                                     \
    volatile uint32_t *module##instance##_##reg = (uint32_t *) AM_REGADDRn(module, instance, reg)

//WATCH_REGn(BLEIF, 0, BSTATUS);  // BLEIF0_BSTATUS

//*****************************************************************************
//
// Forward declarations.
//
//*****************************************************************************
void exactle_stack_init(void);
void scheduler_timer_init(void);
void update_scheduler_timers(void);
void set_next_wakeup(void);
void button_handler(wsfEventMask_t event, wsfMsgHdr_t *pMsg);
extern void AppUiBtnTest(uint8_t btn);

//*****************************************************************************
//
// Timer for buttons.
//
//*****************************************************************************
wsfHandlerId_t ButtonHandlerId;
wsfTimer_t ButtonTimer;

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

//*****************************************************************************
//
// Tracking variable for the scheduler timer.
//
//*****************************************************************************
uint32_t g_ui32LastTime = 0;

//void ble_data_ready_handler(wsfEventMask_t event, wsfMsgHdr_t *pMsg)
//{
    // if (FALSE == HciDataReadyISR()) {
    //     // trigger event again to handle pending data from BLE controller
    //     WsfSetEvent(g_bleDataReadyHandlerId, BLE_DATA_READY_EVENT);
    // }
//}

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

    //g_bleDataReadyHandlerId = WsfOsSetNextHandler(ble_data_ready_handler);

    //ButtonHandlerId = WsfOsSetNextHandler(button_handler);
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
button_handler(wsfEventMask_t event, wsfMsgHdr_t *pMsg)
{
//    //
//    // Restart the button timer.
//    //
//    WsfTimerStartMs(&ButtonTimer, 10);
//
//    //
//    // Every time we get a button timer tick, check all of our buttons.
//    //
//    am_devices_button_array_tick(am_bsp_psButtons, AM_BSP_NUM_BUTTONS);
//
//    //
//    // If we got a a press, do something with it.
//    //
//    if(am_devices_button_released(am_bsp_psButtons[0]))
//    {
//      am_util_debug_printf("Got Button 0 Press\n");
//      AppUiBtnTest(APP_UI_BTN_1_SHORT);
//    }
//
//    if(am_devices_button_released(am_bsp_psButtons[1]))
//    {
//      am_util_debug_printf("Got Button 1 Press\n");
//      AppUiBtnTest(APP_UI_BTN_1_SHORT);
//    }
//
//    if(am_devices_button_released(am_bsp_psButtons[2]))
//    {
//      am_util_debug_printf("Got Button 2 Press\n");
//    }
}

//*****************************************************************************
//
// Sets up a button interface.
//
//*****************************************************************************
void
setup_buttons(void)
{
//    //
//    // Enable the buttons for user interaction.
//    //
//    am_devices_button_array_init(am_bsp_psButtons, AM_BSP_NUM_BUTTONS);
//
//    //
//    // Start a timer.
//    //
//    ButtonTimer.handlerId = ButtonHandlerId;
//    WsfTimerStartSec(&ButtonTimer, 2);
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
    am_hal_clkgen_sysclk_select(AM_HAL_CLKGEN_SYSCLK_MAX);

    //
    // Configure the MCU for low power operation, but leave the LFRC on.
    //

#ifdef AM_DEBUG_PRINTF
    am_util_stdio_printf_init((am_util_stdio_print_char_t) am_bsp_itm_string_print);
    am_bsp_pin_enable(ITM_SWO);
    am_hal_itm_enable();
    am_bsp_debug_printf_enable();
#endif

    am_util_debug_printf("Apollo3 Tag Example\n");

    //
    // Boot the radio.
    //
    HciDrvRadioBoot(0);

    //
    // Initialize the main ExactLE stack.
    //
    exactle_stack_init();

//    //
//    // Enable BLE data ready interrupt
//    //
//    am_hal_interrupt_enable(AM_HAL_INTERRUPT_GPIO);
//
//    //
//    // Prep the buttons for use
//    //
//    setup_buttons();

    //
    // Start the "Fit" profile.
    //
    TagStart();

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
            //am_hal_sysctrl_sleep(AM_HAL_SYSCTRL_SLEEP_DEEP);
        }
        am_hal_interrupt_master_enable();
    }
}
