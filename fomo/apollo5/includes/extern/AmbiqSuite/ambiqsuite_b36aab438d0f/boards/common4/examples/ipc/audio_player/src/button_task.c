//*****************************************************************************
//
//! @file button_task.c
//!
//! @brief Task to handle button related operation.
//!
//*****************************************************************************

//*****************************************************************************
//
// ${copyright}
//
// This is part of revision ${version} of the AmbiqSuite Development Package.
//
//*****************************************************************************
#include <stdint.h>

//*****************************************************************************
//
// FreeRTOS include files.
//
//*****************************************************************************
#include "FreeRTOS.h"
#include "task.h"
#include "portmacro.h"
#include "portable.h"
#include "semphr.h"
#include "event_groups.h"

#include "third_party/open-amp/libmetal/lib/thread.h"

#include "am_bsp.h"
#include "am_util_debug.h"
#include "am_util_stdio.h"

#include "audio_task.h"
#include "button_task.h"

//*****************************************************************************
//
// Macro definitions.
//
//*****************************************************************************
#define TIMER_0             (0)
#define TIMER0_CMP_VALUE    (234)//(23437)

/*! \brief Button press enumeration */
enum
{
    APP_BUTTON_0,   /*!< \brief Button 0 */
    APP_BUTTON_1,   /*!< \brief Button 1 */
};

#define BUTTON_WAIT_EVENT_CHECK (0)

#define BUTTON_CHECK_BIT     (1 << BUTTON_WAIT_EVENT_CHECK)

#define BUTTON_WAITING_ALL_BITS (BUTTON_CHECK_BIT)

/*
 * LOCAL FUNCTION DECLARATIONS
 ****************************************************************************************
 */
static void button_task(void *pvParameters);

//*****************************************************************************
//
// Local variable.
//
//*****************************************************************************
static metal_thread_t     button_task_handle;
static EventGroupHandle_t button_event = NULL;

static am_hal_gpio_pincfg_t g_button_pincfg_array[AM_BSP_NUM_BUTTONS];


/**
 * button_create_event
 *
 * Function called to create button event
 *
 */
void button_create_event(void)
{
    // Create button event group
    if ((button_event = xEventGroupCreate()) == NULL)
    {
        am_util_debug_printf("Create button event group failed\r\n");
    }
}

/**
 * button_wait_event
 *
 * Function called to wait button event
 *
 * return  - the waited event bits
 */
EventBits_t button_wait_event(void)
{
    return xEventGroupWaitBits(button_event, BUTTON_WAITING_ALL_BITS, pdTRUE, pdFALSE, portMAX_DELAY);
}

/**
 * button_set_event
 *
 * Function called to set button event
 *
 * @param event_type the event type to set
 */
void button_set_event(uint8_t event_type)
{
    BaseType_t xResult;

    if (button_event != NULL)
    {
        if ( xPortIsInsideInterrupt() == pdTRUE )
        {
            BaseType_t xHigherPriorityTaskWoken = pdFALSE;

            xResult = xEventGroupSetBitsFromISR(button_event, event_type, &xHigherPriorityTaskWoken);
            if ( xResult == pdPASS )
            {
                portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
            }
        }
        else
        {
            xResult = xEventGroupSetBits(button_event, event_type);
        }
    }

}

static void button_detected(uint8_t btn, uint32_t type)
{
    switch (btn)
    {
        case APP_BUTTON_0:
        {
            if (type == B_LOW_TO_HIGH)
            {
                audio_record_start();
            }
            else if (type == B_HIGH_TO_LOW)
            {
                audio_record_stop();
            }
        }
        break;

        case APP_BUTTON_1:
        {
            if (type == B_LOW_TO_HIGH)
            {
                audio_playback_start();
            }
            else if (type == B_HIGH_TO_LOW)
            {
                audio_playback_stop();
            }
        }
        break;

        default:
        break;
    }
}

void button_detect(void)
{
    // activate, read and debounce each button, then make them inactive again
    am_devices_button_array_tick_pin_cfg(
                    am_bsp_psButtons,
                    g_button_pincfg_array,
                    AM_BSP_NUM_BUTTONS ) ;

    // If there has been a press and release, do something with it.
    for ( uint32_t button_num = 0; button_num < AM_BSP_NUM_BUTTONS; button_num++ )
    {
        if (am_devices_button_pressed(am_bsp_psButtons[button_num]))
        {
            am_util_debug_printf("btn: %d - pressed\n", am_bsp_psButtons[button_num]);
            button_detected(button_num, B_LOW_TO_HIGH);
        }
        else if (am_devices_button_released(am_bsp_psButtons[button_num]))
        {
            am_util_debug_printf("btn: %d - released\n", am_bsp_psButtons[button_num]);
            button_detected(button_num, B_HIGH_TO_LOW);
        }
    }
}

// Timer0 Interrupt Service Routine (ISR)
void am_timer00_isr(void)
{
    // Clear Timer0 Interrupt (write to clear).
    am_hal_timer_interrupt_clear(AM_HAL_TIMER_MASK(TIMER_0, AM_HAL_TIMER_COMPARE0));
    am_hal_timer_clear_stop(TIMER_0);

    am_hal_timer_start(TIMER_0);

    button_set_event(BUTTON_CHECK_BIT);
}

static void setup_timer0(uint32_t compare_val)
{
    am_hal_timer_config_t       Timer0Config;
    am_hal_timer_default_config_set(&Timer0Config);
    Timer0Config.eInputClock = AM_HAL_TIMER_CLOCK_HFRC_DIV4K;   // 96MHz/4K = 24KHz
    Timer0Config.eFunction = AM_HAL_TIMER_FN_UPCOUNT;
    Timer0Config.ui32Compare0 = compare_val ;

    am_hal_timer_config(TIMER_0, &Timer0Config);
    am_hal_timer_clear_stop(TIMER_0);

    am_hal_timer_interrupt_clear(AM_HAL_TIMER_MASK(TIMER_0, AM_HAL_TIMER_COMPARE0));
    am_hal_timer_interrupt_enable(AM_HAL_TIMER_MASK(TIMER_0, AM_HAL_TIMER_COMPARE0));

    NVIC_SetPriority(TIMER0_IRQn, AM_IRQ_PRIORITY_DEFAULT);

    NVIC_EnableIRQ(TIMER0_IRQn);
}

static void setup_buttons(void)
{
    // pull button pin cfg defs out of the bsp
    g_button_pincfg_array[0] = g_AM_BSP_GPIO_BUTTON0 ;
    g_button_pincfg_array[1] = g_AM_BSP_GPIO_BUTTON1 ;

    // Enable the buttons for user interaction.
    am_devices_button_array_init(am_bsp_psButtons, AM_BSP_NUM_BUTTONS);
}

void button_task_setup(void)
{
    setup_buttons();

    button_create_event();

    setup_timer0(TIMER0_CMP_VALUE);

    am_hal_timer_start(TIMER_0);

    int ret = metal_thread_init(&button_task_handle, button_task, "button task", 2 * 1024, 0, 4);
    if (ret)
    {
        am_util_debug_printf("metal_thread_init button task failure %d\n", ret);
        return;
    }
}

//*****************************************************************************
//
// Task to handle button messages.
//
//*****************************************************************************
static void button_task(void *pvParameters)
{
    (void) pvParameters;
    EventBits_t event = 0;

    am_util_debug_printf("button task start\r\n");

    while(1)
    {
        event = button_wait_event();
        if (event & BUTTON_CHECK_BIT)
        {
            button_detect();
        }
    }
}
