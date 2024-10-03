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

#include "ipc_service.h"
#include "errno.h"
#include "rpc_client_sys_cntl.h"
#include "rpc_client_dbglog.h"
#include "rpc_client_audio.h"
#include "rpc_client_hci.h"

#include "ble_rpmsg_throughput.h"
#include "queue.h"
#include "host_ipc_instance.h"

#if (!USE_CORDIO_HOST)
static uint32_t ui32RadioRxIdx = 0;
#endif

extern uint32_t ui32AudioRxIdx;
extern SemaphoreHandle_t g_inst0_rx_sem;
extern SemaphoreHandle_t g_inst1_rx_sem;
extern int parse_data(uint8_t* data, size_t len, uint32_t* idx);
//*****************************************************************************
//
// Endpoint Callback.
//
//*****************************************************************************
static void rpmsg_audio_ep_cb(const void *data, size_t len, void *priv)
{
    if ((data == NULL) || (len == 0) || (len > RPMSG_BUFFER_SIZE))
    {
        metal_assert(false);
        return ;
    }

    (void) priv;

    int status = RPMSG_SUCCESS;

    status = parse_data((uint8_t *)data, len, &ui32AudioRxIdx);
    if (status != RPMSG_SUCCESS)
    {
        am_util_stdio_printf("audio data parsing %d error at index %d\n", status, ui32AudioRxIdx);
    }
    else
    {
        am_util_stdio_printf("*");
    }

    if ((status != RPMSG_SUCCESS) && ( ++ui32AudioRxIdx == VRING_SIZE))
    {
        ui32AudioRxIdx = 0;
    }
}

static void rpmsg_syscntl_ep_cb(const void *data, size_t len, void *priv)
{
    if ((data == NULL) || (len == 0) || (len > RPMSG_BUFFER_SIZE))
    {
        metal_assert(false);
        return;
    }

    (void) priv;

#if (USE_CORDIO_HOST)
    rpc_client_sys_cntl_rx((uint8_t *)data);
#else
    int status = RPMSG_SUCCESS;

    status = parse_data((uint8_t *)data, len, &ui32RadioRxIdx);

    if (status != RPMSG_SUCCESS)
    {
        am_util_stdio_printf("radio data parsing %d error at index %d\n", status, ui32RadioRxIdx);
    }
    else
    {
        am_util_stdio_printf(".");
    }

    if ((status != RPMSG_SUCCESS) && ( ++ui32RadioRxIdx == VRING_SIZE))
    {
        ui32RadioRxIdx = 0;
    }
#endif // #if (USE_CORDIO_HOST)
}

static void rpmsg_debug_ep_cb(const void *data, size_t len, void *priv)
{
    if ((data == NULL) || (len == 0) || (len > RPMSG_BUFFER_SIZE))
    {
        metal_assert(false);
        return;
    }

    (void) priv;

    rpc_client_debug_log_handler((uint8_t *)data, len);
}

static void rpmsg_audio_ep_bound(void *priv)
{
    am_util_stdio_printf("rpmsg_audio_ep_bound \r\n");
}

static void rpmsg_syscntl_ep_bound(void *priv)
{
    am_util_stdio_printf("rpmsg_syscntl_ep_bound \r\n");
}

static void rpmsg_debug_ep_bound(void *priv)
{
    am_util_stdio_printf("rpmsg_debug_ep_bound \r\n");
}

//Called from mailbox callback
static bool host_inst0_kick_cb(void* priv)
{
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    xSemaphoreGiveFromISR(g_inst0_rx_sem, &xHigherPriorityTaskWoken);
    if (xHigherPriorityTaskWoken != pdFAIL)
    {
        portYIELD();
    }
    return false;
}

static bool host_inst1_kick_cb(void* priv)
{
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    xSemaphoreGiveFromISR(g_inst1_rx_sem, &xHigherPriorityTaskWoken);
    if (xHigherPriorityTaskWoken != pdFAIL)
    {
        portYIELD();
    }
    return false;
}

//Define host IPC instance0
struct ipc_device host_inst0 =
{
    .name = "host_inst0",
    .config =
    {
        ROLE_HOST,
        CONFIG_IPC_SHM_BASE_ADDRESS_INST0,
        CONFIG_IPC_SHM_SIZE,
        (HOST_KICK_DATA_INST0 << 16),
        (HOST_KICK_DATA_INST0 << 16),
        &host_inst0_kick_cb,
    },
    .api = NULL,
    .data = NULL,
};

//Define host IPC instance1
struct ipc_device host_inst1 =
{
    .name = "host_inst1",
    .config =
    {
        ROLE_HOST,
        CONFIG_IPC_SHM_BASE_ADDRESS_INST1,
        CONFIG_IPC_SHM_SIZE,
        (HOST_KICK_DATA_INST1 << 16),
        (HOST_KICK_DATA_INST1 << 16),
        &host_inst1_kick_cb,
    },
    .api = NULL,
    .data = NULL,
};

static struct ipc_ept_cfg host_inst_ep_cfg[] =
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
        am_util_stdio_printf("ipc_instance_create() failure\n");
        return;
    }

    struct ipc_ept* rpmsg_audio_ep = (struct ipc_ept*)rpc_client_audio_ep_get();

    // Register host_inst0 endpoints
    status = ipc_service_register_endpoint(&host_inst0, rpmsg_audio_ep, &host_inst_ep_cfg[APP_RPMSG_AUDIO_STREAM_EP]);
    if (status < 0)
    {
        am_util_stdio_printf ("register audio ep failure\n");
        return;
    }

    status = ipc_instance_create(&host_inst1);
    if ((status < 0) && (status != -EALREADY))
    {
        am_util_stdio_printf("crete host_inst1 failure\n");
        return;
    }

    // Register host_inst1 endpoints

    struct ipc_ept* rpmsg_syscntl_ep = (struct ipc_ept*)rpc_client_syscntl_ep_get();
    struct ipc_ept* rpmsg_debug_ep   = (struct ipc_ept*)rpc_client_debug_ep_get();

    status = ipc_service_register_endpoint(&host_inst1, rpmsg_syscntl_ep, &host_inst_ep_cfg[APP_RPMSG_SYS_CNTL_EP]);
    if (status < 0)
    {
        am_util_stdio_printf ("register sys cntl ep failure\n");
        return;
    }

    status = ipc_service_register_endpoint(&host_inst1, rpmsg_debug_ep, &host_inst_ep_cfg[APP_RPMSG_DEBUG_EP]);
    if (status < 0)
    {
        am_util_stdio_printf ("register debug ep failure\n");
        return;
    }
}

bool is_audio_endpoint_bounded()
{
    struct ipc_ept* rpmsg_audio_ep = (struct ipc_ept*)rpc_client_audio_ep_get();

    return ipc_service_is_endpoint_bounded(rpmsg_audio_ep);
}

