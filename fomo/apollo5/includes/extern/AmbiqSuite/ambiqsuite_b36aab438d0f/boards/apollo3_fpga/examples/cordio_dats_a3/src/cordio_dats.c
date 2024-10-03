//*****************************************************************************
//
//! @file cordio_dats.c
//!
//! @brief Cordio Data Server example.
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

#include "dats_api.h"
#include "app_ui.h"

#include "wsf_msg.h"

typedef struct
{
    uint32_t *FIFO;
    uint32_t *FIFOPTR;
    uint32_t *FIFOTHR;
    uint32_t *FIFOPOP;
    uint32_t *FIFOPUSH;
    uint32_t *FIFOCTRL;
    uint32_t *FIFOLOC;
    uint32_t *CLKCFG;
    uint32_t *CMD;
    uint32_t *CMDRPT;
    uint32_t *OFFSETHI;
    uint32_t *CMDSTAT;
    uint32_t *INTEN;
    uint32_t *INTSTAT;
    uint32_t *INTCLR;
    uint32_t *INTSET;
    uint32_t *DMATRIGEN;
    uint32_t *DMATRIGSTAT;
    uint32_t *DMACFG;
    uint32_t *DMATOTCOUNT;
    uint32_t *DMATARGADDR;
    uint32_t *DMASTAT;
    uint32_t *CQCFG;
    uint32_t *CQADDR;
    uint32_t *CQSTAT;
    uint32_t *CQFLAGS;
    uint32_t *CQSETCLEAR;
    uint32_t *CQPAUSEEN;
    uint32_t *CQCURIDX;
    uint32_t *CQENDIDX;
    uint32_t *STATUS;
    uint32_t *MSPICFG;
    uint32_t *BLECFG;
    uint32_t *PWRCMD;
    uint32_t *BSTATUS;
    uint32_t *DBG0;
    uint32_t *DBG1;
    uint32_t *BLEDBG;
}
BLEIF_t;

BLEIF_t BLEIF0 = {
    (uint32_t *) 0x5000C000, // FIFO
    (uint32_t *) 0x5000C100, // FIFOPTR
    (uint32_t *) 0x5000C104, // FIFOTHR
    (uint32_t *) 0x5000C108, // FIFOPOP
    (uint32_t *) 0x5000C10C, // FIFOPUSH
    (uint32_t *) 0x5000C110, // FIFOCTRL
    (uint32_t *) 0x5000C114, // FIFOLOC
    (uint32_t *) 0x5000C200, // CLKCFG
    (uint32_t *) 0x5000C20C, // CMD
    (uint32_t *) 0x5000C210, // CMDRPT
    (uint32_t *) 0x5000C214, // OFFSETHI
    (uint32_t *) 0x5000C218, // CMDSTAT
    (uint32_t *) 0x5000C220, // INTEN
    (uint32_t *) 0x5000C224, // INTSTAT
    (uint32_t *) 0x5000C228, // INTCLR
    (uint32_t *) 0x5000C22C, // INTSET
    (uint32_t *) 0x5000C230, // DMATRIGEN
    (uint32_t *) 0x5000C234, // DMATRIGSTAT
    (uint32_t *) 0x5000C238, // DMACFG
    (uint32_t *) 0x5000C23C, // DMATOTCOUNT
    (uint32_t *) 0x5000C240, // DMATARGADDR
    (uint32_t *) 0x5000C244, // DMASTAT
    (uint32_t *) 0x5000C248, // CQCFG
    (uint32_t *) 0x5000C24C, // CQADDR
    (uint32_t *) 0x5000C250, // CQSTAT
    (uint32_t *) 0x5000C254, // CQFLAGS
    (uint32_t *) 0x5000C258, // CQSETCLEAR
    (uint32_t *) 0x5000C25C, // CQPAUSEEN
    (uint32_t *) 0x5000C260, // CQCURIDX
    (uint32_t *) 0x5000C264, // CQENDIDX
    (uint32_t *) 0x5000C268, // STATUS
    (uint32_t *) 0x5000C300, // MSPICFG
    (uint32_t *) 0x5000C304, // BLECFG
    (uint32_t *) 0x5000C308, // PWRCMD
    (uint32_t *) 0x5000C30C, // BSTATUS
    (uint32_t *) 0x5000C408, // DBG0
    (uint32_t *) 0x5000C40C, // DBG1
    (uint32_t *) 0x5000C410, // BLEDBG
};

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
// Forward declarations.
//
//*****************************************************************************
void exactle_stack_init(void);
void scheduler_timer_init(void);
void update_scheduler_timers(void);
void set_next_wakeup(void);

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
static uint32_t g_pui32BufMem[(2048 * 8) / sizeof(uint32_t)];

// Default pool descriptor.
static wsfBufPoolDesc_t g_psPoolDescriptors[WSF_BUF_POOLS] =
{
    {  16,  8 },
    {  32,  4 },
    {  64,  6 },
    { 280,  4 }
};

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

    handlerId = WsfOsSetNextHandler(DatsHandler);
    DatsHandlerInit(handlerId);
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
    //am_hal_pwrctrl_bucks_enable();

#ifdef AM_DEBUG_PRINTF
//    am_util_stdio_printf_init((am_util_stdio_print_char_t) am_bsp_itm_string_print);
//    am_bsp_pin_enable(ITM_SWO);
//    am_hal_itm_enable();
//    am_bsp_debug_printf_enable();
    am_util_stdio_printf_init((am_util_stdio_print_char_t)am_bsp_uart_string_print);
    am_bsp_uart_printf_init(AM_BSP_UART_PRINT_INST, (am_hal_uart_config_t *)0);
    uart_init(0);
#endif

    am_util_debug_printf("Apollo2 Cordio Dats Example\n");

    //
    // Boot the radio.
    //
    HciDrvRadioBoot(0);

    //
    // Initialize the main ExactLE stack.
    //
    exactle_stack_init();

    //
    // Start the "Fit" profile.
    //
    DatsStart();

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
