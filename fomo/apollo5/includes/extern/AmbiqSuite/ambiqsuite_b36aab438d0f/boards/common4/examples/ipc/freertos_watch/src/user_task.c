//*****************************************************************************
//
//! @file user_task.c
//!
//! @brief Task to handle user related operation.
//!
//*****************************************************************************

//*****************************************************************************
//
// ${copyright}
//
// This is part of revision ${version} of the AmbiqSuite Development Package.
//
//*****************************************************************************
#include "FreeRTOS.h"
#include "queue.h"
#include "am_logger.h"
#include <string.h>
#include "user_task.h"
#include "app_btdm_main.h"
#include "app_bt_gap_api.h"
#include "rpc_common.h"
#include "am_bsp.h"
#include "semphr.h"
#include "watch_main.h"
#include "app_bt_hfp_api.h"
#include "app_bt_avrcp_api.h"
#include "app_ble_gatt_api.h"
#include "app_ble_gap_api.h"
#include "app_bt_a2dp_api.h"
#include "app_nvm.h"

//*****************************************************************************
//
// Macro definitions.
//
//*****************************************************************************
#define TIMER_0             (0)

/*! \brief Button press enumeration */
enum
{
    APP_BTN_NONE,     /*!< \brief No button press */
    APP_BTN_1_DOWN,   /*!< \brief Button 1 pressed */
    APP_BTN_2_DOWN,   /*!< \brief Button 2 pressed */
};


//*****************************************************************************
//
// Global variable.
//
//*****************************************************************************
metal_thread_t           user_task_handle;

QueueHandle_t g_UserQueue;
static am_hal_gpio_pincfg_t g_button_pincfg_array[AM_BSP_NUM_BUTTONS] ;
extern struct app_watch_env_t app_env;
// #### INTERNAL BEGIN ####
#if (STRESS_TEST_ENABLE)
TimerHandle_t userTimer;
TimerHandle_t battery_levle_timer; // used to update battery level periodically
#endif
// #### INTERNAL END ####


//*****************************************************************************
//
// Function prototypes
//
//*****************************************************************************
static void watchBtnCback(uint8_t btn)
{
    UserQueueElem_t elem;

    elem.type = HANDLE_USER_EVENT_INVALID;

    am_info_printf(true, "btn: %d - pressed, call status:%d, a2dp_sink_status:%d \r\n", btn, app_env.call_status, app_env.a2dp_sink_status);

    switch (btn)
    {
        case APP_BTN_1_DOWN:
        {
            if ( app_env.a2dp_sink_status == APP_A2DP_SINK_CS_STARTED )
            {
                elem.type = HANDLE_USER_AVRCP_STOP;
            }
            else if ( (app_env.a2dp_sink_status == APP_A2DP_SINK_CS_SUSPEND )
                 || (app_env.a2dp_sink_status == APP_A2DP_SINK_CS_CLOSED))
            {
                elem.type = HANDLE_USER_AVRCP_PLAY;
            }
            else if ( (app_env.a2dp_sink_status == APP_A2DP_SINK_CS_SUSPEND) || (app_env.a2dp_sink_status == APP_A2DP_SINK_CS_CLOSED) )
            {
                elem.type = HANDLE_USER_AVRCP_PLAY;
            }
            // Use button1 to answer incoming call
            else if ( app_env.call_status == APP_HFP_CS_RCV_INCOMING_CALL )
            {
                elem.type = HANDLE_USER_ANSWER_INCOMING_CALL;
            }
            else if ( app_env.call_status == APP_HFP_CS_IN_PROGRESS )
            {
                // volume up
                elem.type = HANDLE_USER_HF_SET_SPEAKER_VOL;

                if ( app_env.hfp_hf_vol < HFP_VOL_GAIN_MAX )
                {
                    app_env.hfp_hf_vol++;
                }
            }
            else if ( app_env.inq_status == APP_INQUIRY_STATUS_IN_PROGRES )
            {
                elem.type = HANDLE_USER_STOP_INQUIRY;
            }
            else if ( (app_env.inq_status == APP_INQUIRY_STATUS_IDLE) || (app_env.inq_status == APP_INQUIRY_STATUS_STOPPED) )
            {
                app_env.inq_status = APP_INQUIRY_STATUS_IN_PROGRES;

                elem.type = HANDLE_USER_START_INQUIRY;
            }
        }
        break;

        case APP_BTN_2_DOWN:
        {
            am_info_printf(true, "bt_con_idx:%d, call status:%d\r\n", app_env.bt_con_idx, app_env.call_status);
            if ( (app_env.bt_con_idx != GAP_INVALID_ACTV_IDX )
                && (app_env.call_status == APP_HFP_CS_NONE))
            {
                elem.type = HANDLE_USER_HFP_INIT_OUTGOING_CALL;
            }
            // use button2 to reject incoming call
            else if ( app_env.call_status == APP_HFP_CS_RCV_INCOMING_CALL )
            {
                elem.type = HANDLE_USER_REJECT_INCOMING_CALL;
            }
            else if ( app_env.call_status == APP_HFP_CS_IN_PROGRESS )
            {
                // volume down
                elem.type = HANDLE_USER_HF_SET_SPEAKER_VOL;

                if ( app_env.hfp_hf_vol > HFP_VOL_GAIN_MIN )
                {
                    app_env.hfp_hf_vol--;
                }
            }
        }
        break;

        default:
        break;
    }

    if ( elem.type != HANDLE_USER_EVENT_INVALID )
    {
        if ( !user_task_queue_send(&elem) )
        {
            am_error_printf(true, "send to user queue failed\r\n");
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

    // activate, read and debounce each button, then make them inactive again
    am_devices_button_array_tick_pin_cfg(
                    am_bsp_psButtons,
                    g_button_pincfg_array,
                    AM_BSP_NUM_BUTTONS ) ;

    // If there has been a press and release, do something with it.
    for ( uint32_t button_num = 0; button_num < AM_BSP_NUM_BUTTONS; button_num++ )
    {
        if ( am_devices_button_released(am_bsp_psButtons[button_num]))
        {
            watchBtnCback(button_num + 1);
        }
    }
}


/**
 * user_task_dispatch
 *
 * User task event dispatched.  Designed to be called repeatedly from infinite loop.
 *
 * @param - NA
 *
 * return - NA
 */
static void user_task_dispatch(void)
{
    UserQueueElem_t element;

    element.type = 0xFFFFFFFF;

    if (user_task_queue_read(&element) == pdPASS)
    {
        int send_status = RPMSG_SUCCESS;

        switch ( element.type )
        {
            case HANDLE_USER_START_EVENT:
            {
                // load nvm parameters before stat message.
                send_status = app_nvm_load(NVM_ITEM_DEV_BOND_INFO);
                if ( send_status < 0 )
                {
                    am_error_printf(true, "load NVM fail, status:%d\r\n", send_status);
                }

                send_status = app_nvm_load(NVM_ITEM_HFP_VOL);
                if ( send_status < 0 )
                {
                    am_error_printf(true, "load NVM fail, status:%d\r\n", send_status);
                }
                send_status = app_btdm_start();
                if ( send_status < 0 )
                {
                    am_error_printf(true, "set dev config fail, status:%d\r\n", send_status);
                }
            }
            break;

            case HANDLE_USER_START_INQUIRY:
            {
                send_status = app_inquiry_start(app_inquiry_report_received);
                if ( send_status < 0 )
                {
                    am_error_printf(true, "start inquiry fail, status:%d\r\n", send_status);
                }
            }
            break;

            case HANDLE_USER_STOP_INQUIRY:
            {
                send_status = app_inquiry_stop();
                if ( send_status < 0 )
                {
                    am_error_printf(true, "stop inquiry fail, status:%d\r\n", send_status);
                }
            }
            break;

            case HANDLE_USER_START_INQ_SCAN:
            {
                send_status = app_inquiry_scan_start();
                if ( send_status < 0 )
                {
                    am_error_printf(true, "start inquiry scan fail, status:%d\r\n", send_status);
                }

                send_status = app_page_scan_start();
                if ( send_status < 0 )
                {
                    am_error_printf(true, "start page scan fail, status:%d\r\n", send_status);
                }

                send_status = app_adv_start();
                if ( send_status < 0 )
                {
                    am_error_printf(true, "start BLE adv fail, status:%d\r\n", send_status);
                }
            }
            break;

            case HANDLE_USER_DEV_CONNECT:
            {
                gap_addr_t bd_addr;

                send_status = app_inquiry_stop();

                if ( send_status < 0 )
                {
                    am_error_printf(true, "send stop inquiry req fail, status:%d\r\n", send_status);
                }

                memcpy(bd_addr.addr, element.data, GAP_BD_ADDR_LEN);

                send_status = app_bt_con_estab(&bd_addr);
                if ( send_status < 0 )
                {
                    am_error_printf(true, "send create connection req fail, status:%d\r\n", send_status);
                }
            }
            break;

            case HANDLE_USER_ANSWER_INCOMING_CALL:
            {
                send_status = hfp_hf_answer_call();
                if ( send_status < 0 )
                {
                    am_error_printf(true, "hfp answer incoming call fail, status:%d\r\n", send_status);
                }
            }
            break;

            case HANDLE_USER_REJECT_INCOMING_CALL:
            {
                send_status = hfp_hf_reject_call();
                if ( send_status < 0 )
                {
                    am_error_printf(true, "hfp reject incoming call fail, status:%d\r\n", send_status);
                }
            }
            break;

            case HANDLE_USER_HFP_INIT_OUTGOING_CALL:
            {
                send_status = hfp_hf_initiate_outgoing_call((uint8_t *)app_env.phone_num, strlen(app_env.phone_num));
                if ( send_status < 0 )
                {
                    am_error_printf(true, "hfp init outgoing call fail, status:%d\r\n", send_status);
                }
            }
            break;

            case HANDLE_USER_HF_SET_SPEAKER_VOL:
            {
                am_info_printf(true, "user set speaker vol:%d\r\n", app_env.hfp_hf_vol);

                app_nvm_update_hfp_vol(app_env.hfp_hf_vol);

                send_status = hfp_hf_set_speaker_volume(app_env.hfp_hf_vol);
                if ( send_status < 0 )
                {
                    am_error_printf(true, "hfp hf set speaker volume, status:%d\r\n", send_status);
                }

                if ( app_env.hfp_hf_vol == 0 )
                {
                    send_status = hfp_hf_hang_up_call();
                    if ( send_status < 0 )
                    {
                        am_error_printf(true, "hfp hang up call, status:%d\r\n", send_status);
                    }
                }
            }
            break;

            case HANDLE_USER_AVRCP_SET_VOL:
            {
                am_info_printf(true, "set avrcp vol:%d\r\n", app_env.avrcp_vol);

                send_status = app_avrcp_set_volume(app_env.avrcp_vol);
                if ( send_status < 0 )
                {
                    am_error_printf(true, "avrcp set volume fail, status:%d\r\n", send_status);
                }
            }
            break;

            case HANDLE_USER_AVRCP_PLAY:
            {
                am_info_printf(true, "avrcp play\r\n");

                send_status = app_avrcp_play();
                if ( send_status < 0 )
                {
                    am_error_printf(true, "avrcp play fail, status:%d\r\n", send_status);
                }
            }
            break;

            case HANDLE_USER_AVRCP_STOP:
            {
                am_info_printf(true, "avrcp stop\r\n");

                send_status = app_avrcp_stop();
                if ( send_status < 0 )
                {
                    am_error_printf(true, "avrcp stop fail, status:%d\r\n", send_status);
                }
            }
            break;

            case HANDLE_USER_AVRCP_FORWARD:
            {
                am_info_printf(true, "avrcp forward\r\n");

                send_status = app_avrcp_play_forward();
                if ( send_status < 0 )
                {
                    am_error_printf(true, "avrcp play forward, status:%d\r\n", send_status);
                }
            }
            break;

            case HANDLE_USER_AVRCP_BACKWARD:
            {
                am_info_printf(true, "avrcp backward\r\n");

                send_status = app_avrcp_play_backward();
                if ( send_status < 0 )
                {
                    am_error_printf(true, "avrcp play backward, status:%d\r\n", send_status);
                }
            }
            break;

            case HANDLE_USER_UPDATE_BATTERY_LVL:
            {
                // #### INTERNAL BEGIN ####
                #if !(STRESS_TEST_ENABLE)
                app_env.battery_lvl = HFP_HF_MAX_BATTERY_VALUE;
                #endif
                // #### INTERNAL END ####

                if ( app_env.hfp_svc_lvl_con )
                {
                    send_status = app_hfp_hf_chg_bat_lvl(app_env.battery_lvl);
                    if ( send_status < 0 )
                    {
                        am_error_printf(true, "hfp battery level send fail, status:%d\r\n", send_status);
                    }
                }

                if ( app_env.ble_bonded )
                {
                    send_status = app_bass_chg_bat_lvl(app_env.battery_lvl);
                    if ( send_status < 0 )
                    {
                        am_error_printf(true, "BLE BASS battery level send fail, status:%d\r\n", send_status);
                    }
                }
            }
            break;

            case HANDLE_USER_A2DP_SRC_PLAY:
            {
                send_status = app_a2dp_src_stream_start();
                if ( send_status < 0 )
                {
                    am_error_printf(true, "A, status:%d\r\n", send_status);
                }
            }
            break;
            case HANDLE_USER_A2DP_SRC_SUSPEND:
            {
                send_status = app_a2dp_src_stream_suspend();
                if ( send_status < 0 )
                {
                    am_error_printf(true, "A, status:%d\r\n", send_status);
                }
            }
            break;
            default:
            break;
        }
    }
    else
    {
        am_error_printf(true, "read usr queue failed\r\n");
    }
}

/*
 * EXPORTED FUNCTION DEFINITIONS
 ****************************************************************************************
 */
 //*****************************************************************************
//
// Task to handle UI related behavior.
//
//*****************************************************************************
void setup_buttons(void)
{
    // pull button pin cfg defs out of the bsp
    g_button_pincfg_array[0] = g_AM_BSP_GPIO_BUTTON0 ;
    g_button_pincfg_array[1] = g_AM_BSP_GPIO_BUTTON1 ;

    // Enable the buttons for user interaction.
    am_devices_button_array_init(am_bsp_psButtons, AM_BSP_NUM_BUTTONS);

    am_hal_timer_start(TIMER_0);
}

void app_setup_timer0(uint32_t compare_val)
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

// #### INTERNAL BEGIN ####
#if (STRESS_TEST_ENABLE)
void app_start_timer(void)
{
    xTimerStart(userTimer, 0);
}

static void userTimer_handler(TimerHandle_t xTimer)
{
    UserQueueElem_t elem;

    if ( app_env.a2dp_src_status != APP_A2DP_SRC_CS_NONE )
    {
        if ( app_env.a2dp_src_status == APP_A2DP_SRC_CS_STARTED )
        {
            elem.type = HANDLE_USER_A2DP_SRC_SUSPEND;
        }
        else
        {
            elem.type = HANDLE_USER_A2DP_SRC_PLAY;
        }

        if ( !user_task_queue_send(&elem) )
        {
            am_error_printf(true, "a2dp src start/suspend control send to user queue failed\r\n");
        }
    }
    else
    {
        watchBtnCback(APP_BTN_1_DOWN);
    }
}

void app_start_battery_level_timer(void)
{
    if ( battery_levle_timer != NULL )
    {
        xTimerStart(battery_levle_timer, 0);
    }
}

void app_stop_battery_level_timer(void)
{
    if ( xTimerIsTimerActive(battery_levle_timer) )
    {
        xTimerStop(battery_levle_timer, 0);
    }
}

static void battery_level_timer_handler(TimerHandle_t xTimer)
{
    UserQueueElem_t elem;

    app_env.battery_lvl++;

    if ( app_env.battery_lvl > HFP_HF_MAX_BATTERY_VALUE )
    {
        app_env.battery_lvl = 0;
    }

    elem.type = HANDLE_USER_UPDATE_BATTERY_LVL;

    if ( !user_task_queue_send(&elem) )
    {
        am_error_printf(true, "send to user queue failed\r\n");
    }
}
#endif
// #### INTERNAL END ####

void user_task(void *pvParameters)
{
    (void) pvParameters;

    am_info_printf(true, "user task\r\n");

// #### INTERNAL BEGIN ####
#if (STRESS_TEST_ENABLE)
    if ( userTimer == NULL )
    {
      userTimer = xTimerCreate("user Timer", pdMS_TO_TICKS(random_num(USER_TIMER_MS_MAX, USER_TIMER_MS_MIN)),
                               pdTRUE, NULL, userTimer_handler);
      configASSERT(userTimer);
    }

    if ( battery_levle_timer == NULL )
    {
        battery_levle_timer = xTimerCreate("battery Timer", pdMS_TO_TICKS(random_num(BATTERY_LVL_TIMER_MS_MAX, BATTERY_LVL_TIMER_MS_MIN)),
              pdTRUE, NULL, battery_level_timer_handler);
        configASSERT(battery_levle_timer);
    }
#endif
// #### INTERNAL END ####

    while(1)
    {
        user_task_dispatch();
    }
}

/**
*
* Initialize the user task queue
* Returns true for success.
*/
bool user_task_queue_create()
{
    // Create the user task Queue
    g_UserQueue = xQueueCreate(USER_TASK_QUEUE_NUM, sizeof(UserQueueElem_t) );
    return (g_UserQueue != NULL);
}

/**
* Send a message to the SER Queue.
*
* This may block and if it does it blocks forever.
*
* Returns true if message successfully sent.
*/
bool user_task_queue_send(UserQueueElem_t *elem)
{
    if ( xPortIsInsideInterrupt() == pdTRUE )
    {
        BaseType_t xHigherPriorityTaskWoken, xResult;
        xHigherPriorityTaskWoken = 0;

        xResult = xQueueSendFromISR(g_UserQueue, elem, &xHigherPriorityTaskWoken);

        if (xResult != pdFAIL)
        {
            portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
        }

        return xResult != pdFAIL;
    }
    else
    {
        return xQueueSendToBack(g_UserQueue, elem, 0 );
    }
}

/**
* Read a message from the user task Queue.
*
* This may block and if it does it blocks forever.
*
* Returns true if message successfully read.
*/
bool user_task_queue_read(UserQueueElem_t *elem)
{
     return xQueueReceive(g_UserQueue, elem, portMAX_DELAY );
}
