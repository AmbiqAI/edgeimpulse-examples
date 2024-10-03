//*****************************************************************************
//
//! @file host_ipc_instance.c
//!
//! @brief Handle HOST ipc instances related data and functions.
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

#include "FreeRTOS.h"
#include "queue.h"
#include "event_groups.h"
#include "host_ipc_instance.h"
#include "am_logger.h"
#include "rpc_client_btdm.h"
#include "user_task.h"
#include "am_devices_ambt53_pwrdown.h"
// #### INTERNAL BEGIN ####
#ifdef IPC_STRESS_TEST_ENABLE
#include "ipc_stress_test_main.h"
#endif
// #### INTERNAL END ####
#include "rpc_client_nv_info.h"

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

/*
 * GLOBAL VARIABLE DEFINITIONS
 ****************************************************************************************
 */
ipc_ept_t* rpmsg_syscntl_ep;
ipc_ept_t* rpmsg_debug_ep;
ipc_ept_t* rpmsg_audio_ep;
struct ipc_device host_inst0;
struct ipc_device host_inst1;

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
        ((uint32_t)REMOTE_KICK_DATA_INST0 << 16),
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
        ((uint32_t)REMOTE_KICK_DATA_INST1 << 16),
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

    am_info_printf(true, "audio_ep_cb, len:%d \r\n", len);
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


    am_info_printf(true, "rpmsg_audio_ep_bound \r\n");

}

static void rpmsg_syscntl_ep_bound(void *priv)
{
    (void) priv;


    am_info_printf(true, "rpmsg_syscntl_ep_bound \r\n");

}

static void rpmsg_debug_ep_bound(void *priv)
{
    (void) priv;


    am_info_printf(true, "rpmsg_debug_ep_bound \r\n");

    UserQueueElem_t elem;

    elem.type = HANDLE_USER_START_EVENT;

    if ( !user_task_queue_send(&elem) )
    {
        am_error_printf(true, "send to user queue failed\r\n");
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

/*
 * EXPORTED FUNCTION DEFINITIONS
 ****************************************************************************************
 */

/**
 * host_ipc_instance_setup
 *
 * Create host instance and register endpoints.
 *
 * @param - None
 *
 * return - None
 */
void host_ipc_instance_setup(void)
{
    int status = 0;

    status = ipc_instance_create(&host_inst0);
    if ((status < 0) && (status != -EALREADY))
    {
        am_error_printf(true, "ipc_instance_create() failure\n");
        return;
    }

    rpmsg_audio_ep = (ipc_ept_t *)rpc_client_audio_ep_get();

    // Register host_inst0 endpoints
    status = ipc_service_register_endpoint(&host_inst0, rpmsg_audio_ep, &host_inst_ep_cfg[APP_RPMSG_AUDIO_STREAM_EP]);
    if (status < 0)
    {
        am_error_printf(true, "register audio ep failure\n");
        return;
    }

    status = ipc_instance_create(&host_inst1);
    if ((status < 0) && (status != -EALREADY))
    {
        am_error_printf(true, "crete host_inst1 failure\n");
        return;
    }

    // Register host_inst1 endpoints

    rpmsg_syscntl_ep = (ipc_ept_t *)rpc_client_syscntl_ep_get();
    status = ipc_service_register_endpoint(&host_inst1, rpmsg_syscntl_ep, &host_inst_ep_cfg[APP_RPMSG_SYS_CNTL_EP]);
    if (status < 0)
    {
        am_error_printf(true, "register sys cntl ep failure\n");
        return;
    }

    rpmsg_debug_ep = (ipc_ept_t *)rpc_client_debug_ep_get();
    status = ipc_service_register_endpoint(&host_inst1, rpmsg_debug_ep, &host_inst_ep_cfg[APP_RPMSG_DEBUG_EP]);
    if (status < 0)
    {
        am_error_printf(true, "register debug ep failure\n");
        return;
    }
}


/**
 * host_audio_task_dispatch
 *
 * Audio task event dispatched.  Designed to be called repeatedly from infinite loop.
 *
 * @param - NA
 *
 * return - NA
 */
void host_audio_task_dispatch(void)
{
    // TODO will handle audio data sending to ambt53 here

    vTaskDelay( pdMS_TO_TICKS(1) );
}

/**
 * mailbox_isr
 *
 * ambt53 Mailbox Interrupt handler.
 *
 * @param - NA
 *
 * return - NA
 */
void mailbox_isr(void)
{
    ipc_set_event(IPC_MBOX_RX_BIT);
}

/**
 * ipc_task_dispatch
 *
 * Receive task event dispatched.  Designed to be called repeatedly from infinite loop.
 *
 * @param - NA
 *
 * return - NA
 */
void ipc_task_dispatch(void)
{
    EventBits_t event_to_handle = 0;
    uint32_t buf_received = 0;
    bool ambt53_power_down_state;

    event_to_handle = ipc_wait_event();

    am_debug_printf(true, "ipc task event to handle: 0x%x\r\n", event_to_handle);

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
    }

    am_debug_printf(true, "buf_received=%d\r\n", buf_received);

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

    if (event_to_handle & IPC_BTDM_TX_BIT)
    {
        event_to_handle &= ~IPC_BTDM_TX_BIT;

        am_hal_interrupt_master_disable();
        ambt53_power_down_state = bIsPowerDown;
        am_hal_interrupt_master_enable();

        if ( ambt53_power_down_state == false )
        {
            ipc_data_t *p_buf_data = ipc_data_queue_pop(IPC_DATA_QUEUE_BTDM_REQ_TX);

            if (p_buf_data != NULL)
            {
                int send_status = rpc_client_btdm_send_packet(
                            p_buf_data->type, p_buf_data->data,
                            p_buf_data->len);

                am_debug_printf(true, "ipc task send packet,%d\r\n", send_status);

                metal_free_memory(p_buf_data);

                if ( send_status < 0 )
                {
                    am_error_printf(true, "send btdm pkt failed, status:%d\r\n",
                                    send_status);
                }
            }
            // inform the ambt53 can not enter power down until the tx finish
            am_device_ambt53_wait_mspi_idle(5);
            am_devices_force_ambt53_active_release(IPC_WRITE);
        }
    }
    else if (event_to_handle & IPC_NVM_TX_BIT)
    {
        event_to_handle &= ~IPC_NVM_TX_BIT;

        ipc_data_t *p_buf_data = ipc_data_queue_pop(IPC_DATA_QUEUE_NVM_REQ_TX);

        if (p_buf_data != NULL)
        {
            int send_status = rpc_client_nvm_send_packet(
                           p_buf_data->type, p_buf_data->data,
                           p_buf_data->len);

            am_debug_printf(true, "ipc task send nvm packet,%d\r\n", send_status);

            metal_free_memory(p_buf_data);

            if ( send_status < 0 )
            {
                am_error_printf(true, "send nvm pkt failed, status:%d\r\n",
                                send_status);
            }
        }
    }

}


// #### INTERNAL BEGIN ####
#ifdef IPC_STRESS_TEST_ENABLE
/**
 * host_ipc_stress_test_setup
 *
 * Set up IPC stress test.
 *
 * @param - NA
 *
 * return - NA
 */
void host_ipc_stress_test_setup(void)
{
    ipc_stress_test_setup(&host_inst0);
}
#endif
// #### INTERNAL END ####

