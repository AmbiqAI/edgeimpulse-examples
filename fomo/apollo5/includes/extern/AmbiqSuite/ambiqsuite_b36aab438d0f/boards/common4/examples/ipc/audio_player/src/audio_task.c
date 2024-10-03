//*****************************************************************************
//
//! @file audio_task.c
//!
//! @brief Task to handle audio related operation.
//!
//*****************************************************************************

//*****************************************************************************
//
// ${copyright}
//
// This is part of revision ${version} of the AmbiqSuite Development Package.
//
//*****************************************************************************

#include <stdbool.h>

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

#include "rpmsg.h"
#include "rpc_client_audio.h"

#include "am_util_debug.h"
#include "am_util_stdio.h"

#include "flash.h"

/*
 * LOCAL FUNCTION DECLARATIONS
 ****************************************************************************************
 */
static void audio_task(void *pvParameters);

//*****************************************************************************
//
// Local variable.
//
//*****************************************************************************
static metal_thread_t audio_task_handle;
static int            record_stop = 0;

// static uint32_t total_sent = 0;
// static uint32_t total_recv = 0;

void audio_record_start(void)
{
    // total_recv = 0;
    record_stop = 0;
    flash_write_init();
    ipc_audio_req_send(RPMSG_AUDIO_RECORD_START, NULL, 0);
}

void audio_record_stop(void)
{
    record_stop = 1;
    flash_write_flush();
    ipc_audio_req_send(RPMSG_AUDIO_RECORD_STOP, NULL, 0);
}

void audio_playback_start(void)
{
    // total_sent = 0;
    flash_read_init();
    ipc_audio_req_send(RPMSG_AUDIO_PLAYBACK_START, NULL, 0);
}

void audio_playback_stop(void)
{
    ipc_audio_req_send(RPMSG_AUDIO_PLAYBACK_STOP, NULL, 0);
}

static void audio_req_ack_handler(uint8_t *evt_pkt, uint16_t len)
{
    uint8_t  type;
    uint16_t ack_status = 0;

    BSTREAM_TO_UINT8(type, evt_pkt);
    BSTREAM_TO_UINT16(ack_status, evt_pkt);

    am_util_debug_printf("audio_req_ack_handler type %d, ack %d\n", type, ack_status);

    switch ( type )
    {
        case RPMSG_AUDIO_RECORD_START:
        case RPMSG_AUDIO_RECORD_STOP:
        case RPMSG_AUDIO_PLAYBACK_START:
        case RPMSG_AUDIO_PLAYBACK_STOP:
        {
            if (rpc_request_is_busy())
            {
                m_return_value = ack_status;
                rpc_update_rsp_wait_flag(false);
            }
        }
        break;

        default:
            break;
    }
}

static void audio_dsp_data_transfer_handler(uint8_t *evt_pkt, uint16_t len)
{
    flash_write(evt_pkt, len);
    // When record stop command was sent to DSP, DSP may still transfer one
    // or more packets, so add flush function to save it immediately.
    if (record_stop)
    {
        flash_write_flush();
    }
    // total_recv += len;
    // am_util_debug_printf("audio received size %d\n", total_recv);
}

static void audio_dsp_need_data_handler(uint8_t *evt_pkt, uint16_t len)
{
    uint32_t need_size = 0;
    uint32_t get_size  = 0;
    uint32_t out_size  = 0;
    uint8_t *data;

    metal_assert(len == sizeof(uint32_t));

    BSTREAM_TO_UINT32(need_size, evt_pkt);
    // am_util_debug_printf("audio_dsp_need_data_handler need size %d\n", need_size);

    while (need_size)
    {
        get_size = (need_size > RPMSG_AUDIO_BUF_SIZE) ? RPMSG_AUDIO_BUF_SIZE : need_size;
        data = flash_get(get_size, &out_size);
        if (!data || !out_size || (out_size > get_size))
        {
            // metal_assert(false);
            break;
        }
        // total_sent += out_size;
        // am_util_debug_printf("audio need %d get %d out %d total %d\n",
        //                         need_size, get_size, out_size, total_sent);
        rpc_client_audio_send_packet(RPMSG_AUDIO_DATA_TRANS, data, out_size);
        need_size -= out_size;
    }
}

void audio_msg_handler(uint8_t type, uint8_t *evt_pkt, uint16_t len)
{
    // am_util_debug_printf("audio_msg_handler type:%d len:%d\n", type, len);

    switch (type)
    {
        case RPMSG_REQ_ACK:
        {
            audio_req_ack_handler(evt_pkt, len);
            break;
        }

        case RPMSG_AUDIO_DATA_TRANS:
        {
            audio_dsp_data_transfer_handler(evt_pkt, len);
            break;
        }

        case RPMSG_AUDIO_DSP_NEED_DATA:
        {
            audio_dsp_need_data_handler(evt_pkt, len);
            break;
        }

        default:
            break;
    }
}

void audio_task_setup(void)
{
    rpc_client_audio_cb_set(audio_msg_handler);

    ipc_data_queue_init(IPC_DATA_QUEUE_AUDIO_REQ_TX);

    int ret = metal_thread_init(&audio_task_handle, audio_task, "audio task", 2 * 1024, 0, 4);
    if (ret)
    {
        am_util_debug_printf("metal_thread_init audio task failure %d\n", ret);
        return;
    }
}

//*****************************************************************************
//
// Task to handle audio messages.
//
//*****************************************************************************
static void audio_task(void *pvParameters)
{
    (void) pvParameters;

    am_util_debug_printf("audio task start\r\n");

    while(1)
    {
        // TODO: Handle audio task.
        vTaskDelay( pdMS_TO_TICKS(10000) );
    }
}
