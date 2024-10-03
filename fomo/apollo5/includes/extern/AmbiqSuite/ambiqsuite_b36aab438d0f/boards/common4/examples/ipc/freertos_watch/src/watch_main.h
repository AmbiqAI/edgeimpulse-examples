//*****************************************************************************
//
//! @file watch_main.h
//!
//! @brief Functions and variables related to watch task.
//!
//*****************************************************************************

//*****************************************************************************
//
// ${copyright}
//
// This is part of revision ${version} of the AmbiqSuite Development Package.
//
//*****************************************************************************

#ifndef WATCH_MAIN_H
#define WATCH_MAIN_H

#include "ipc_service.h"
#include "rpmsg_virtio.h"
#include "am_devices_ambt53.h"
#include "app_btdm_main.h"
#include "throughput_api.h"

 // inquiry status
typedef enum
{
    APP_INQUIRY_STATUS_IDLE,
    APP_INQUIRY_STATUS_IN_PROGRES,
    APP_INQUIRY_STATUS_STOPPED
}eAPPInqStatus_t;

// HFP call status
typedef enum
{
    APP_HFP_CS_NONE,
    APP_HFP_CS_RCV_INCOMING_CALL,
    APP_HFP_CS_IN_PROGRESS,
    APP_HFP_CS_CALL_TERMINATED,
    APP_HFP_CS_AUDIO_TERMINATED,
}eHFPCallStatus_t;

// A2DP sink status
typedef enum
{
    APP_A2DP_SINK_CS_NONE,
    APP_A2DP_SINK_CS_STARTED,
    APP_A2DP_SINK_CS_SUSPEND,
    APP_A2DP_SINK_CS_CLOSED,
}eA2DPSinkStatus_t;

// A2DP sink status
typedef enum
{
    APP_A2DP_SRC_CS_NONE,
    APP_A2DP_SRC_CS_STARTED,
    APP_A2DP_SRC_CS_SUSPEND,
    APP_A2DP_SRC_CS_CLOSED,
}eA2DPSrcStatus_t;

// application environment structure
struct app_watch_env_t
{
    // BLE ADV index
    uint8_t         adv_actv_idx;
    // BLE Connection index
    uint8_t         ble_con_idx;
    // BLE bonded
    bool            ble_bonded;
    // BT Connection index
    uint8_t         bt_con_idx;

    // SBC Stream end-point identifier
    uint8_t         sbc_seid;
    // AAC Stream end-point identifier
    uint8_t         aac_seid;
    // ATRAC Stream end-point identifier
    uint8_t         atrac_seid;
    // Use to know if streaming started
    bool            is_streaming;
    // Use to know if button is pressed
    bool            button_pressed;

    uint8_t         peer_features[GAP_LE_FEATS_LEN];
    eAPPInqStatus_t inq_status;

    /// call status
    eHFPCallStatus_t call_status;

    uint8_t         hfp_hf_vol;
    bool            hfp_svc_lvl_con;
    // Battery level
    uint8_t         battery_lvl;
    char *          phone_num;
    uint8_t         avrcp_vol;
    eA2DPSinkStatus_t a2dp_sink_status;
    eA2DPSrcStatus_t  a2dp_src_status;
    uint16_t ota_pkt_offset;
    uint16_t ota_pkt_len;
    bool     last_pkt_flag;
    bool     ota_cmplt;
    uint8_t uplinkData[UPLINK_DATA_CNT];
} ;

/*
 * ENUMERATIONS
 ****************************************************************************************
 */

// application transitions
enum app_a2dp_sink_trans_event
{
    // --- Stream states

    // Stream Configured
    APP_STREAM_CONFIGURED = APP_FIRST_A2DP_SINK_TRANSITION,
    // Stream Started
    APP_A2DP_SINK_STREAM_STARTED,
    // Stream Suspended
    APP_A2DP_SINK_STREAM_SUSPENDED,
    // Stream closed
    APP_A2DP_SINK_STREAM_CLOSED,
};

enum app_a2dp_sourc_trans_event
{
    // --- Stream states

    // Stream Started
    APP_A2DP_SRC_STREAM_STARTED = APP_FIRST_A2DP_SOURCE_TRANSITION,
    // Stream Suspended
    APP_A2DP_SRC_STREAM_SUSPENDED,
    // Stream closed
    APP_A2DP_SRC_STREAM_CLOSED,
};

// application HFP HF transitions
enum app_hfp_hf_trans_event
{
    // --- HFP states
    APP_HFP_SERVICE_LEVEL_ESTABLISHED = APP_FIRST_HFP_HF_TRANSITION,
    APP_HFP_RECV_INCOMMING_CALL,
    APP_HFP_RING_ALERT,
    APP_HFP_CALL_STARTED,
    APP_HFP_CALL_TERMINATED,
    APP_HFP_AUDIO_TERMINATED,
};

enum app_avrcp_trans_event
{
    // --AVRCP target device received Button Event
    APP_AVRCP_TARGET_VOL_CHANGED = APP_FIRST_AVRCP_TRANSITION,
};
// application transitions
enum app_amota_trans_event
{
    APP_AMOTA_DATA_TRANSFER = APP_FIRST_AMOTA_TRANSITION,
    APP_AMOTA_HEADER_EVT,
    APP_AMOTA_UPDATE_FLAG,
    APP_AMOTA_CMPLT,
};

// application transitions
enum app_throughput_trans_event
{
    APP_THROUGHPUT_RCV_DATA = APP_FIRST_THROUGHPUT_TRANSITION,
    APP_THROUGHPUT_SEND_DATA,
};

// application transitions
enum app_ancs_trans_event
{
    // Received ANCS message
    APP_ANCS_RCV_MSG = APP_FIRST_ANCS_TRANSITION,
};

//*****************************************************************************
//
// watch task handle.
//
//*****************************************************************************
extern metal_thread_t audio_task_handle;
extern metal_thread_t ipc_task_handle;

//*****************************************************************************
//
// External function definitions.
//
//*****************************************************************************
extern void watch_task_setup(void);
extern void audio_task(void *pvParameters);
extern void ipc_task(void *pvParameters);
extern void user_task(void *pvParameters);

#endif // WATCH_MAIN_H
