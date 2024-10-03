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
#include <errno.h>

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
#include "audio_task.h"

extern int load_ambt53_fw(void);
extern int load_ambt53_section(const char *);

//*****************************************************************************
//
// Global variable.
//
//*****************************************************************************
SemaphoreHandle_t xSemaphore = NULL;

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

static char *codec_list[LOADER_TYPE_NUMBER][LOADER_NUMBER] = {
    {"loader_none",                "loader_none"               },
    {"overlay_1.sbc.decoder.code", "overlay_1.sbc.decoder.data"},
    {"overlay_1.sbc.encoder.code", "overlay_1.sbc.encoder.data"},
    {"overlay_1.msbc.code",        "overlay_1.msbc.data"       },
};

static void
audio_msg_handler(uint8_t type, uint8_t *evt_pkt, uint16_t len)
{
    // am_util_debug_printf("audio_msg_handler type:%d len:%d\n", type, len);

    uint8_t status      = LOADER_FAIL;
    uint8_t section       = LOADER_NONE;
    uint8_t ack_pkt[2]  = {0};
    uint8_t *p_ack_pkt = ack_pkt;
    // convert recived payload to data
    BSTREAM_TO_UINT8(section, evt_pkt);

    switch (type)
    {
        case RPMSG_AUDIO_LOADER_REQ:
        {
            if ((load_ambt53_section(codec_list[section][LOADER_CODE]) < 0) ||
                (load_ambt53_section(codec_list[section][LOADER_DATA]) < 0))
            {
                am_util_debug_printf("Load section: %d failed\r\n", section);
                goto fail;
            }
            else
            {
                am_util_debug_printf("Load section: %d successfully\r\n", section);
                goto success;
            }

            break;
        }

        default:
            break;
    }

fail:
    // convert data to payload
    UINT8_TO_BSTREAM(p_ack_pkt, status);
    UINT8_TO_BSTREAM(p_ack_pkt, section);
    ipc_audio_data_send(RPMSG_AUDIO_LOADER_RSP, ack_pkt, sizeof(ack_pkt));
    return;

success:
    status = LOADER_SUC;
    // convert data to payload
    UINT8_TO_BSTREAM(p_ack_pkt, status);
    UINT8_TO_BSTREAM(p_ack_pkt, section);
    ipc_audio_data_send(RPMSG_AUDIO_LOADER_RSP, ack_pkt, sizeof(ack_pkt));
}

void audio_task_setup(void)
{
    xSemaphore = xSemaphoreCreateMutex();

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
