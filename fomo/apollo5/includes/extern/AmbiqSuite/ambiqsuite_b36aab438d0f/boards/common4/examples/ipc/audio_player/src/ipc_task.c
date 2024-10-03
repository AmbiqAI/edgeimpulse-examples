//*****************************************************************************
//
//! @file ipc_task.c
//!
//! @brief Task to handle ipc related operation.
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

#include "errno.h"

#include "am_devices_ambt53.h"
#include "am_devices_ambt53_pwrdown.h"
#include "am_util_debug.h"
#include "am_util_stdio.h"

#include "ipc_service.h"
#include "rpc_common.h"
#include "rpc_client_audio.h"
#include "rpc_client_dbglog.h"
#include "rpc_client_sys_cntl.h"

#include "user_task.h"
#include "ipc_task.h"
#include "flash.h"

/*
 * LOCAL FUNCTION DECLARATIONS
 ****************************************************************************************
 */
static void rpmsg_audio_ep_cb(const void *data, size_t len, void *priv);
static void rpmsg_syscntl_ep_cb(const void *data, size_t len, void *priv);
static void rpmsg_debug_ep_cb(const void *data, size_t len, void *priv);
static void rpmsg_audio_ep_bound(void *priv);
static void rpmsg_syscntl_ep_bound(void *priv);
static void rpmsg_debug_ep_bound(void *priv);
static bool host_inst0_kick_cb(void* priv);
static bool host_inst1_kick_cb(void* priv);
static void host_ipc_instance_setup(void);
static void ipc_task(void *pvParameters);

/*
 * GLOBAL VARIABLE DEFINITIONS
 ****************************************************************************************
 */
ipc_ept_t* rpmsg_syscntl_ep;
ipc_ept_t* rpmsg_debug_ep;
ipc_ept_t* rpmsg_audio_ep;
struct ipc_device host_inst0;
struct ipc_device host_inst1;

//*****************************************************************************
//
// Local variable.
//
//*****************************************************************************
static metal_thread_t ipc_task_handle;

//Define host IPC instance0
ipc_dev_t host_inst0 =
{
    .name = "host_inst0",
    .config =
    {
        ROLE_HOST,
        CONFIG_IPC_SHM_BASE_ADDRESS_INST0,
        CONFIG_IPC_SHM_SIZE,
        ((uint32_t)HOST_KICK_DATA_INST0 << 16),
        ((uint32_t)HOST_KICK_DATA_INST0 << 16),
        &host_inst0_kick_cb,
    },
    .api = NULL,
    .data = NULL,
};

//Define host IPC instance1
ipc_dev_t host_inst1 =
{
    .name = "host_inst1",
    .config =
    {
        ROLE_HOST,
        CONFIG_IPC_SHM_BASE_ADDRESS_INST1,
        CONFIG_IPC_SHM_SIZE,
        ((uint32_t)HOST_KICK_DATA_INST1 << 16),
        ((uint32_t)HOST_KICK_DATA_INST1 << 16),
        &host_inst1_kick_cb,
    },
    .api = NULL,
    .data = NULL,
};

static ipc_ept_cfg_t host_inst_ep_cfg[] =
{
    [APP_RPMSG_AUDIO_STREAM_EP] =
    {
        .name = RPMSG_AUDIO_EP_NAME,
        .cb =
        {
            .bound    = rpmsg_audio_ep_bound,
            .received = rpmsg_audio_ep_cb,
        },
    },
    [APP_RPMSG_SYS_CNTL_EP] =
    {
        .name = RPMSG_SYS_CNTL_EP_NAME,
        .cb =
        {
            .bound    = rpmsg_syscntl_ep_bound,
            .received = rpmsg_syscntl_ep_cb,
        },
    },
    [APP_RPMSG_DEBUG_EP] =
    {
        .name = RPMSG_DEBUG_EP_NAME,
        .cb =
        {
            .bound    = rpmsg_debug_ep_bound,
            .received = rpmsg_debug_ep_cb,
        },
    }
};

/*
 * LOCAL FUNCTION DEFINITIONS
 ****************************************************************************************
 */

//*****************************************************************************
//
// Endpoint Callback.
//
//*****************************************************************************
static void rpmsg_audio_ep_cb(const void *data, size_t len, void *priv)
{
    (void) priv;

    if ((data == NULL) || (len == 0) || (len > RPMSG_BUFFER_SIZE))
    {
        metal_assert(false);
        return ;
    }

    rpc_client_audio_rx((uint8_t *)data);
}

static void rpmsg_syscntl_ep_cb(const void *data, size_t len, void *priv)
{
    (void) priv;

    if ((data == NULL) || (len == 0) || (len > RPMSG_BUFFER_SIZE))
    {
        metal_assert(false);
        return;
    }

    uint8_t *pdata = (uint8_t *)data;

    rpc_client_sys_cntl_rx(pdata);
}

static void rpmsg_debug_ep_cb(const void *data, size_t len, void *priv)
{
    (void) priv;

    if ((data == NULL) || (len == 0) || (len > RPMSG_BUFFER_SIZE))
    {
        metal_assert(false);
        return;
    }

    uint8_t *pdata = (uint8_t *)data;

    rpc_client_debug_log_handler(pdata, len);
}

static void rpmsg_audio_ep_bound(void *priv)
{
    (void) priv;

    int      status = RPMSG_SUCCESS;
    uint8_t  ui8EpBindFlag    = 0xFF;

    am_util_debug_printf("rpmsg_audio_ep_bound \r\n");

    status = host_ipc_ep_send_sync_packet(APP_RPMSG_AUDIO_STREAM_EP, &ui8EpBindFlag, 1);
    if (status < RPMSG_SUCCESS)
    {
        am_util_debug_printf("audio ep write bind flag fail\r\n");
    }
}

static void rpmsg_syscntl_ep_bound(void *priv)
{
    (void) priv;

    int      status = RPMSG_SUCCESS;
    uint8_t  ui8EpBindFlag    = 0xFF;

    am_util_debug_printf("rpmsg_syscntl_ep_bound \r\n");

    status = host_ipc_ep_send_sync_packet(APP_RPMSG_SYS_CNTL_EP, &ui8EpBindFlag, 1);
    if (status < RPMSG_SUCCESS)
    {
        am_util_debug_printf("sys cntl ep write bind flag fail\r\n");
    }
}

static void rpmsg_debug_ep_bound(void *priv)
{
    (void) priv;

    int      status = RPMSG_SUCCESS;
    uint8_t  ui8EpBindFlag    = 0xFF;

    am_util_debug_printf("rpmsg_debug_ep_bound \r\n");

    status = host_ipc_ep_send_sync_packet(APP_RPMSG_DEBUG_EP, &ui8EpBindFlag, 1);
    if (status < RPMSG_SUCCESS)
    {
        am_util_debug_printf("debug ep write bind flag fail\r\n");
    }
    else
    {
        UserQueueElem_t elem;

        elem.type = HANDLE_USER_START_EVENT;

        if ( !user_task_queue_send(&elem) )
        {
            am_util_debug_printf("send to user queue failed\r\n");
        }
    }
}

//Called from mailbox callback
static bool host_inst0_kick_cb(void* priv)
{
    *((uint32_t*) priv) |= IPC_INST0_KICK_BIT;

    return true;
}

static bool host_inst1_kick_cb(void* priv)
{
    *((uint32_t*) priv) |= IPC_INST1_KICK_BIT;

    return true;
}

/**
 * host_ipc_ep_send_sync_packet
 *
 * Send 1 byte synchronization pakcet to remote
 *
 * @param - ep_idx, endpoint identifier
 * @param - data, pointer to data to send
 * @param - len, length of data to send
 *
 * return - status to indicate if sending fail or the sent out data length
 */
int host_ipc_ep_send_sync_packet(eAppRPMsgEpIndex ep_idx, const void *data, size_t len)
{
    switch ( ep_idx )
    {
        case APP_RPMSG_AUDIO_STREAM_EP:
        {
            return ipc_service_send(rpmsg_audio_ep, data, len);
        }
        break;

        case APP_RPMSG_SYS_CNTL_EP:
        {
            return ipc_service_send(rpmsg_syscntl_ep, data, len);
        }
        break;

        case APP_RPMSG_DEBUG_EP:
        {
            return ipc_service_send(rpmsg_debug_ep, data, len);
        }
        break;

        default:
        break;
    }

    return -EINVAL;
}

/**
 * mailbox_isr
 *
 * ambt53 Mailbox Interrupt handler.
 *
 * @param - NA
 *
 * @return - NA
 */
void mailbox_isr(void)
{
    ipc_set_event(IPC_MBOX_RX_BIT);
}

/**
 * host_ipc_instance_setup
 *
 * Create host instance and register endpoints.
 *
 * @param - None
 *
 * @return - None
 */
static void host_ipc_instance_setup(void)
{
    int status = 0;

    status = ipc_instance_create(&host_inst0);
    if ((status < 0) && (status != -EALREADY))
    {
        am_util_debug_printf("ipc create host instance0 failure\n");
        return;
    }

    // Register host_inst0 endpoints
    rpmsg_audio_ep = (ipc_ept_t *)rpc_client_audio_ep_get();
    status = ipc_service_register_endpoint(&host_inst0, rpmsg_audio_ep, &host_inst_ep_cfg[APP_RPMSG_AUDIO_STREAM_EP]);
    if (status < 0)
    {
        am_util_debug_printf("register audio ep failure\n");
        return;
    }

    status = ipc_instance_create(&host_inst1);
    if ((status < 0) && (status != -EALREADY))
    {
        am_util_debug_printf("ipc create host instance1 failure\n");
        return;
    }

    // Register host_inst1 endpoints
    rpmsg_syscntl_ep = (ipc_ept_t *)rpc_client_syscntl_ep_get();
    status = ipc_service_register_endpoint(&host_inst1, rpmsg_syscntl_ep, &host_inst_ep_cfg[APP_RPMSG_SYS_CNTL_EP]);
    if (status < 0)
    {
        am_util_debug_printf("register sys cntl ep failure\n");
        return;
    }

    rpmsg_debug_ep = (ipc_ept_t *)rpc_client_debug_ep_get();
    status = ipc_service_register_endpoint(&host_inst1, rpmsg_debug_ep, &host_inst_ep_cfg[APP_RPMSG_DEBUG_EP]);
    if (status < 0)
    {
        am_util_debug_printf("register debug ep failure\n");
        return;
    }
}

void ipc_task_setup(void)
{
    //
    // Boot the RPMsg.
    //
    int status = am_devices_ambt53_boot();
    am_util_debug_printf("boot ambt53, status:%d\r\n", status);
    if (status != 0)
    {
        am_util_debug_printf("am_devices_ambt53_boot failure\n");
        return;
    }

    // Register the mailbox callback into SCPM interrupt handlers.
    am_devices_ambt53_interrupt_register(SCPM_IRQ_MAILBOX_THRESHOLD, SCPM_IRQ_POS_EDGE, mailbox_isr);

    host_ipc_instance_setup();

    ipc_create_event();

    int ret = metal_thread_init(&ipc_task_handle, ipc_task, "ipc task", 2 * 1024, 0, 3);
    if (ret)
    {
        am_util_debug_printf("metal_thread_init ipc task failure %d\n", ret);
        return;
    }
}

static void ipc_task(void *pvParameters)
{
    (void) pvParameters;

    am_util_debug_printf("ipc task start\r\n");

    while(1)
    {
        EventBits_t event_to_handle = 0;
        uint32_t buf_received = 0;
        bool ambt53_power_down_state;

        event_to_handle = ipc_wait_event();

        // am_util_debug_printf("ipc task event to handle: 0x%x\r\n", event_to_handle);

        while (event_to_handle & IPC_MBOX_RX_BIT)
        {
            // inform the ambt53 don't enter power down
            am_devices_force_ambt53_active_req(IPC_READ);
            // wake up the ambt53
            am_devices_ambt53_wakeup_from_pwrdown();
            while(1)
            {
                if ( am_devices_check_ambt53_active_state() )
                {
                    break;
                }
            }

            uint32_t mailbox_status = mailbox_event_handle(&buf_received);
            if ( mailbox_status == 0 )
            {
                // Mailbox successfully handled, clear mbox event bit
                event_to_handle &= ~IPC_MBOX_RX_BIT;
                if ( buf_received == 0 )
                {
                    am_devices_force_ambt53_active_release(IPC_READ);
                }
            }
            else
            {
                am_util_debug_printf("mail box status error:0x%x\r\n", mailbox_status);
            }
        }

        // am_util_debug_printf("buf_received=%d\r\n", buf_received);

        if (buf_received & IPC_INST0_KICK_BIT)
        {
            buf_received &= ~IPC_INST0_KICK_BIT;
            ipc_receive(&host_inst0);
            am_devices_force_ambt53_active_release(IPC_READ);
        }

        if (buf_received & IPC_INST1_KICK_BIT)
        {
            buf_received &= ~IPC_INST1_KICK_BIT;
            ipc_receive(&host_inst1);
            am_devices_force_ambt53_active_release(IPC_READ);
        }

        if (event_to_handle & IPC_AUDIO_TX_BIT)
        {
            event_to_handle &= ~IPC_AUDIO_TX_BIT;

            am_hal_interrupt_master_disable();
            ambt53_power_down_state = bIsPowerDown;
            am_hal_interrupt_master_enable();

            if ( ambt53_power_down_state == false )
            {
                // Loopback all queue message.
                ipc_data_t *p_buf_data;
                do
                {
                    p_buf_data = ipc_data_queue_pop(IPC_DATA_QUEUE_AUDIO_REQ_TX);

                    if (p_buf_data != NULL)
                    {
                        int send_status = rpc_client_audio_send_packet(
                                    p_buf_data->type, p_buf_data->data, p_buf_data->len);

                        // am_util_debug_printf("ipc task send audio packet, size %d\r\n", send_status);

                        metal_free_memory(p_buf_data);

                        if ( send_status < 0 )
                        {
                            am_util_debug_printf("send audio pkt failed, status:%d\r\n", send_status);
                        }
                    }
                    // inform the ambt53 can not enter power down until the tx finish
                    am_device_ambt53_wait_mspi_idle(5);
                    am_devices_force_ambt53_active_release(IPC_WRITE);
                } while (p_buf_data);
            }
        }
    }
}
