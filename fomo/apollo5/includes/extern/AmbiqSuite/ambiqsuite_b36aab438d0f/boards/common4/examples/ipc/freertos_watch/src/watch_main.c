//*****************************************************************************
//
//! @file watch_main.c
//!
//! @brief Task to handle watch example operation.
//!
//*****************************************************************************

//*****************************************************************************
//
// ${copyright}
//
// This is part of revision ${version} of the AmbiqSuite Development Package.
//
//*****************************************************************************
#include "freertos_watch.h"
#include "host_ipc_instance.h"
#include "app_ble_gap_api.h"
#include "app_bt_gap_api.h"
#include "app_btdm_main.h"
#include "app_bt_device_api.h"
#include "am_assert.h"
#include "am_logger.h"
#include "am_alloc.h"
#include <string.h>
#include "user_task.h"
#include "app_bt_hfp_api.h"
#include "app_bt_avrcp_api.h"
#include "amotas_api.h"
#include "ancsc_api.h"
#include "app_nvm.h"

//*****************************************************************************
//
// Macro definitions.
//
//*****************************************************************************
#define TIMER0_CMP_VALUE    (200)


//*****************************************************************************
//
// Global variable.
//
//*****************************************************************************
metal_thread_t           audio_task_handle;
metal_thread_t           ipc_task_handle;

struct app_watch_env_t app_env;
static logger_context_t g_log;

// Targeted device address
uint8_t target_device_addr[GAP_BD_ADDR_LEN] =
    { 0x90, 0x13, 0x0C, 0xE0, 0x52, 0x28};
    //{ 0xbf, 0xf5, 0x41, 0x58, 0x7a, 0x90};//WI_C200
//*****************************************************************************
//
// Function prototypes
//
//*****************************************************************************
static void init_system();


/*
 ****************************************************************************************
 * @brief Function to handle the received inquiry report.
 *
 * @param[in] msg      pointer to the received inquiry report
 ****************************************************************************************
 */
void app_inquiry_report_received(uint8_t *msg)
{
    uint8_t eir_len    = 0;
    gapm_inquiry_report_t inq_rep;

    uint8_t *p_msg = msg;

    memset(&inq_rep, 0x0, sizeof(gapm_inquiry_report_t));

    BSTREAM_TO_UINT32(inq_rep.class_of_device, p_msg);
    memcpy(inq_rep.addr.addr, p_msg, GAP_BD_ADDR_LEN);
    p_msg += GAP_BD_ADDR_LEN;
    BSTREAM_TO_INT8(inq_rep.rssi, p_msg);
    BSTREAM_TO_UINT8(inq_rep.page_scan_rep_mode, p_msg);
    BSTREAM_TO_UINT16(inq_rep.clk_off, p_msg);

    BSTREAM_TO_UINT8(eir_len, p_msg);

    if ( eir_len > 0 )
    {
        uint8_t dev_name_len = 0;
        const uint8_t *p_dev_name = NULL;
        device_name_t dev_name = {0};

        p_dev_name = app_bt_get_ltv_value(GAP_AD_TYPE_COMPLETE_NAME, eir_len, p_msg, &dev_name_len);

        if ( p_dev_name != NULL )
        {
            memcpy(dev_name, p_dev_name, dev_name_len);
        }

        am_info_printf(true, "INQUIRY REPORT RECEIVED: %02X:%02X:%02X:%02X:%02X:%02X, cod:0x%06X, rssi:%d, name:%s\n\r",
                 inq_rep.addr.addr[0], inq_rep.addr.addr[1], inq_rep.addr.addr[2], inq_rep.addr.addr[3], inq_rep.addr.addr[4], inq_rep.addr.addr[5], inq_rep.class_of_device,
                 inq_rep.rssi, dev_name);

        if ( memcmp(inq_rep.addr.addr, target_device_addr, GAP_BD_ADDR_LEN) == 0 )
        {
            UserQueueElem_t elem;

            am_info_printf(true, "device %s found \r\n", dev_name);
            elem.type = HANDLE_USER_DEV_CONNECT;
            elem.data = target_device_addr;
            elem.len = GAP_BD_ADDR_LEN;
            user_task_queue_send(&elem);
        }
    }
    else
    {
        am_info_printf(true, "INQUIRY REPORT RECEIVED: %02X:%02X:%02X:%02X:%02X:%02X, cod:0x%06X, rssi:%d\n\r",
                 inq_rep.addr.addr[0], inq_rep.addr.addr[1], inq_rep.addr.addr[2], inq_rep.addr.addr[3], inq_rep.addr.addr[4], inq_rep.addr.addr[5], inq_rep.class_of_device,
                 inq_rep.rssi);
    }
}


//*****************************************************************************
//
// Perform initial setup for watch task.
//
//*****************************************************************************
void watch_task_setup(void)
{
    init_system();

    //
    // Boot the RPMsg.
    //
    int status = am_devices_ambt53_boot();
    am_util_debug_printf("boot ambt53, status:%d\r\n", status);
    if (status != 0)
    {
        am_critical_printf(true, "am_devices_ambt53_boot failure\n");
        return;
    }

    // Register the mailbox callback into SCPM interrupt handlers.
    am_devices_ambt53_interrupt_register(SCPM_IRQ_MAILBOX_THRESHOLD, SCPM_IRQ_POS_EDGE, mailbox_isr);

    host_ipc_instance_setup();

    ipc_create_event();

    if ( !user_task_queue_create() )
    {
        am_error_printf(true, "user task queue crate failed\r\n");
    }

    app_btdm_init();
    app_nvm_init();

    //
    // Set up timer 0.
    //
    app_setup_timer0(TIMER0_CMP_VALUE);

    setup_buttons();
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
// Short Description.
//
//*****************************************************************************

void ipc_task(void *pvParameters)
{
    (void) pvParameters;

    am_info_printf(true, "ipc task\r\n");

    while(1)
    {
        ipc_task_dispatch();
    }
}

//*****************************************************************************
//
// Task to handle audio messages.
//
//*****************************************************************************
void audio_task(void *pvParameters)
{
    (void) pvParameters;

    am_info_printf(true, "audio task\r\n");

    while(1)
    {
        host_audio_task_dispatch();
    }
}

//*****************************************************************************
//
// Application state machine.
//
//*****************************************************************************

static void app_watch_state_transition(uint8_t event, uint16_t status, const void *p_param)
{
    switch ( event )
    {
        case APP_STARTED:
        {
            am_info_printf(true, "app started\r\n");

            UserQueueElem_t elem;

            elem.type = HANDLE_USER_START_INQ_SCAN;
            if ( !user_task_queue_send(&elem) )
            {
                am_error_printf(true, "send to user queue failed\r\n");
            }

        }break;

        case APP_INQUIRY_STOPPED:
        {
            app_env.inq_status = APP_INQUIRY_STATUS_STOPPED;
            am_info_printf(true, "inquiry stopped \r\n");
        }break;

        case APP_DISCONNECTED:
        {
            am_info_printf(true, "disconnected, con_idx:%d\r\n", *((uint8_t *)p_param));

            if ( app_env.bt_con_idx == *((uint8_t *)p_param) )
            {
                am_info_printf(true, "bt disconnect\r\n");
                app_env.bt_con_idx   = GAP_INVALID_ACTV_IDX;
                app_env.call_status  = APP_HFP_CS_NONE;
            }
            else if ( app_env.ble_con_idx == *((uint8_t *)p_param) )
            {
                am_info_printf(true, "ble disconnect\r\n");
                app_env.ble_con_idx  = GAP_INVALID_ACTV_IDX;
                app_env.ble_bonded   = false;

                // when OTA data transfer completes, it will disconnect the BLE connection with phone,
                // will execute reboot after received disconnect complete event.
                if ( app_env.ota_cmplt )
                {
                    amota_reset_start();
                }
            }

            // #### INTERNAL BEGIN ####
            #if (STRESS_TEST_ENABLE)
            if ( (app_env.bt_con_idx == GAP_INVALID_ACTV_IDX )
                 && (app_env.ble_con_idx == GAP_INVALID_ACTV_IDX))
            {
                app_stop_battery_level_timer();
            }
            #endif
            // #### INTERNAL END ####
        } break;

        case APP_LE_CONNECTION_ESTABLISHED:
        {
            am_info_printf(true, "ble connection established, con idx:%d\r\n", *((uint8_t *)p_param));
            app_env.ble_con_idx    = *((uint8_t *)p_param);
            app_env.last_pkt_flag  = false;
            app_env.ota_pkt_offset = 0;
            app_env.ota_cmplt      = false;
        }break;

        case APP_BT_CONNECTION_ESTABLISHED:
        {
            am_info_printf(true, "BT connection established, idx:%d, status:%d\r\n", *((uint8_t *)p_param), status);
            app_env.bt_con_idx = *((uint8_t *)p_param);
            app_env.avrcp_vol = AVRCP_VOL_DEF;
            app_env.a2dp_sink_status = APP_A2DP_SINK_CS_NONE;
            app_env.hfp_svc_lvl_con = false;
            app_env.battery_lvl = 0;
        } break;

        case APP_PAIRING_SUCCEED:
        {
            if ( app_env.ble_con_idx == *((uint8_t *)p_param) )
            {
                am_info_printf(true, "ble bonded\r\n");
                app_env.ble_bonded = true;

// #### INTERNAL BEGIN ####
            #if (STRESS_TEST_ENABLE)
                app_start_battery_level_timer();
            #endif
// #### INTERNAL END ####
            }
            else if ( app_env.bt_con_idx == *((uint8_t *)p_param) )
            {
                am_info_printf(true, "bt bonded\r\n");
            }
        }break;

        case APP_A2DP_SINK_STREAM_STARTED:
        {
            app_env.is_streaming = true;
            app_env.a2dp_sink_status = APP_A2DP_SINK_CS_STARTED;
            am_info_printf(true, "audio stream started \r\n");
// #### INTERNAL BEGIN ####
#if (STRESS_TEST_ENABLE)
            app_start_timer();
#endif
// #### INTERNAL END ####
        } break;

        case APP_A2DP_SINK_STREAM_SUSPENDED:
        {
            app_env.is_streaming = false;
            app_env.a2dp_sink_status = APP_A2DP_SINK_CS_SUSPEND;
            am_info_printf(true, "audio stream suspended \r\n");
        } break;

        case APP_A2DP_SINK_STREAM_CLOSED:
        {
            app_env.is_streaming = false;
            app_env.a2dp_sink_status = APP_A2DP_SINK_CS_CLOSED;
            am_info_printf(true, "audio stream closed \r\n");
        } break;

        case APP_A2DP_SRC_STREAM_STARTED:
        {
            app_env.is_streaming = true;
            app_env.a2dp_src_status = APP_A2DP_SRC_CS_STARTED;
            am_info_printf(true, "a2dp_src audio stream started \r\n");
// #### INTERNAL BEGIN ####
#if (STRESS_TEST_ENABLE)
            app_start_timer();
#endif
// #### INTERNAL END ####
        } break;
        case APP_A2DP_SRC_STREAM_SUSPENDED:
        {
            app_env.is_streaming = false;
            app_env.a2dp_src_status = APP_A2DP_SRC_CS_SUSPEND;
            am_info_printf(true, "a2dp_src audio stream suspended \r\n");
        } break;

        case APP_A2DP_SRC_STREAM_CLOSED:
        {
            app_env.is_streaming = false;
            app_env.a2dp_src_status = APP_A2DP_SRC_CS_CLOSED;
            am_info_printf(true, "a2dp_src audio stream closed \r\n");
        } break;

        case APP_AVRCP_TARGET_VOL_CHANGED:
        {
            app_env.avrcp_vol = *((uint8_t *)p_param);

            am_info_printf(true, "AVRCP target volume changed %d\r\n", app_env.avrcp_vol);
        }break;

        case APP_HFP_SERVICE_LEVEL_ESTABLISHED:
        {
            am_info_printf(true, "HFP service level established\r\n");

            app_env.hfp_svc_lvl_con = true;
// #### INTERNAL BEGIN ####
        #if (STRESS_TEST_ENABLE)
            app_start_battery_level_timer();
        #endif
// #### INTERNAL END ####

        }break;

        case APP_HFP_RECV_INCOMMING_CALL:
        {
            app_env.call_status = APP_HFP_CS_RCV_INCOMING_CALL;
        }break;

        case APP_HFP_CALL_STARTED:
        {
            am_info_printf(true, "hfp call started\r\n");
            app_env.call_status = APP_HFP_CS_IN_PROGRESS;
            app_env.hfp_hf_vol   = HFP_VOL_GAIN_DEF;
        }break;

        case APP_HFP_CALL_TERMINATED:
        {
            am_info_printf(true, "hfp call terminated\r\n");

            if ( app_env.call_status == APP_HFP_CS_IN_PROGRESS )
            {
                app_env.call_status = APP_HFP_CS_CALL_TERMINATED;
            }
        }break;

        case APP_HFP_AUDIO_TERMINATED:
        {
            am_info_printf(true, "hfp audio terminated\r\n");

            if ( app_env.call_status == APP_HFP_CS_IN_PROGRESS )
            {
                app_env.call_status = APP_HFP_CS_AUDIO_TERMINATED;
            }
        }break;

        case APP_AMOTA_DATA_TRANSFER:
        {
            uint16_t data_len    = 0; // data lenth in APP_AMOTA_DATA_TRANSFER
            uint16_t write_len   = 0;
            uint8_t *p_buf       = (uint8_t *)p_param;

            BSTREAM_TO_UINT16(data_len, p_buf);

            // new packet
            if (app_env.ota_pkt_offset == 0)
            {
                BSTREAM_TO_UINT16(app_env.ota_pkt_len, p_buf);
                BSTREAM_TO_UINT8(app_env.last_pkt_flag, p_buf);
                write_len = data_len - AMOTA_FIRST_PKT_HDR_LEN;
                app_env.ota_pkt_len -= AMOTA_FIRST_PKT_HDR_LEN;
            }
            else
            {
                write_len = data_len;
            }

            app_env.ota_pkt_offset += write_len;

            if ( app_env.ota_pkt_offset == app_env.ota_pkt_len )
            {
                // start next new packet
                app_env.ota_pkt_offset = 0;
            }

           amota_write2flash(write_len, p_buf, app_env.last_pkt_flag);
        }
        break;

        case APP_AMOTA_HEADER_EVT:
        {
            amota_packet_header_handle((uint8_t *)p_param);
        }break;

        case APP_AMOTA_UPDATE_FLAG:
        {
            am_info_printf(true, "Update OTA flag\r\n");
            amota_update_flag();
        }break;

        case APP_AMOTA_CMPLT:
        {
            am_info_printf(true, "OTA data transfer completed\r\n");
            app_env.ota_cmplt = true;
        }break;

        case APP_THROUGHPUT_RCV_DATA:
        {
            static uint16_t old_index  = 1;
            uint8_t *p_data            = (uint8_t *)p_param;
            uint16_t new_index         = 0;

            p_data += 1;
            BSTREAM_TO_UINT16(new_index, p_data);

            if ((((new_index & 0x3ff) == 0) && (new_index >> 10 != 0)) || (app_env.ble_con_idx == GAP_INVALID_ACTV_IDX))
            {
                am_info_printf(true, "Device rx %d packages\r\n", new_index);
            }

            if ( new_index - old_index > 1 )
            {
                am_error_printf(true, "lost packet, old index:0x%x, new index:0x%x\r\n", old_index, new_index);
                while(1);
            }

            old_index = new_index;
        }
        break;

        case APP_THROUGHPUT_SEND_DATA:
        {
            uint8_t *p_send_data    = (uint8_t *)p_param;
            uint8_t  data_len  = 0;
            uint16_t index     = 0;

            BSTREAM_TO_UINT8(data_len, p_send_data);
            BSTREAM_TO_UINT16(index, p_send_data);

            memset(app_env.uplinkData, UPLINK_DATA, UPLINK_DATA_CNT);

            app_env.uplinkData[0] = index & 0xff;
            app_env.uplinkData[1] = (index >> 8) & 0xff;

            if ((((index & 0x3ff) == 0) && (index >> 10 != 0)) || (app_env.ble_con_idx != GAP_INVALID_ACTV_IDX))
            {
                am_info_printf(true, "Device tx %d packages\r\n", index);
            }

            if ( app_env.ble_con_idx != GAP_INVALID_ACTV_IDX )
            {
                app_throughput_send_data(app_env.uplinkData, data_len);
            }
        }
        break;

        case APP_ANCS_RCV_MSG:
        {
            // process received ANCS message
            app_ancs_data_process((uint8_t *)p_param) ;
        }
        break;

        default: { /* Ignore */ } break;
    }
}

static void __stack_overflow_handler(const char *thread_name)
{
    am_critical_printf(true, "__stack_overflow_handler %s\n\r", thread_name);
}

static void init_system()
{
    logger_init_context(&g_log, "swo", LOG_LEVEL_INFO, swo_log_handler_get());
    logger_set_default_context(&g_log);

    metal_set_stack_overflow_handler(__stack_overflow_handler);
}

/*
 * EXPORTED FUNCTION DEFINITIONS
 ****************************************************************************************
 */
/**
 ****************************************************************************************
 * @brief Function used to get application state transition callback.
 *
 ****************************************************************************************
 */
app_state_transition_cb bt_app_get_transition_cb(void)
{
    // Initialize memory
    app_env.adv_actv_idx = GAP_INVALID_ACTV_IDX;
    app_env.ble_con_idx  = GAP_INVALID_ACTV_IDX;
    app_env.bt_con_idx   = GAP_INVALID_ACTV_IDX;
    app_env.inq_status   = APP_INQUIRY_STATUS_IDLE;
    app_env.call_status  = APP_HFP_CS_NONE;
    app_env.phone_num = "10086";

    ipc_data_queue_init(IPC_DATA_QUEUE_BTDM_REQ_TX);

    amota_init();

    return app_watch_state_transition;
}
