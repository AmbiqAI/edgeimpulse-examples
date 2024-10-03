//*****************************************************************************
//
//! @file rpmsg_task.c
//!
//! @brief Task to handle rpmsg operation.
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
#include "am_devices_ambt53_pwrdown.h"
#include "rpc_client_btdm.h"
#include "uart_rpmsg_bridge.h"
#include "rpmsg_task.h"

/*
 * LOCAL FUNCTION DECLARATIONS
 ****************************************************************************************
 */
static void rpmsg_syscntl_ep_cb(const void *data, size_t len, void *priv);
static void rpmsg_syscntl_ep_bound(void *priv);
static bool host_inst0_kick_cb(void* priv);
void ipc_hci_msg_rx_handler(uint8_t *packet, uint32_t len);
/*
 * GLOBAL VARIABLE DEFINITIONS
 ****************************************************************************************
 */
metal_thread_t rpmsg_proc_task_handle;
ipc_ept_t* rpmsg_syscntl_ep;
struct ipc_device host_inst0;
uint32_t hci_tx_cnt = 0;
uint32_t hci_rx_cnt = 0;
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

static ipc_ept_cfg_t host_inst_ep_cfg[] =
{
    [APP_RPMSG_SYS_CNTL_EP] =
    {
        .name = RPMSG_SYS_CNTL_EP_NAME,
        .cb =
        {
            .bound    = rpmsg_syscntl_ep_bound,
            .received = rpmsg_syscntl_ep_cb,
        },
    },
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
    hci_rx_cnt++;
    am_util_stdio_printf("RPC HCI pkt rx, len=%d, hci_rx_cnt = %d\r\n", len, hci_rx_cnt);
}

static void rpmsg_syscntl_ep_bound(void *priv)
{
    (void) priv;

    am_util_stdio_printf("rpmsg_syscntl_ep_bound \r\n");
}

//Called from mailbox callback
static bool host_inst0_kick_cb(void* priv)
{
    *((uint32_t*) priv) |= IPC_INST0_KICK_BIT;

    return true;
}



//*****************************************************************************
// host_ipc_instance_setup
//
// Create host instance and register endpoints.
//
// @param - None
//
// return - None
//*****************************************************************************
void host_ipc_instance_setup(void)
{
    int status = 0;

    status = ipc_instance_create(&host_inst0);
    if ((status < 0) && (status != -EALREADY))
    {
        am_util_stdio_printf("ipc_instance_create() failure\n");
        return;
    }

    // Register host_inst1 endpoints
    rpmsg_syscntl_ep = (ipc_ept_t *)rpc_client_syscntl_ep_get();
    status = ipc_service_register_endpoint(&host_inst0, rpmsg_syscntl_ep, &host_inst_ep_cfg[APP_RPMSG_SYS_CNTL_EP]);
    if (status < 0)
    {
        am_util_stdio_printf("register sys cntl ep failure\n");
        return;
    }
}

//*****************************************************************************
// ipc_task_dispatch
//
// Receive task event dispatched.  Designed to be called repeatedly from infinite loop.
//
// @param - NA
//
// return - NA
//*****************************************************************************

void ipc_task_dispatch(void)
{
    EventBits_t event_to_handle = 0;
    uint32_t buf_received = 0;


    event_to_handle = ipc_wait_event();

    while (event_to_handle & IPC_MBOX_RX_BIT)
    {

        uint32_t mailbox_status = mailbox_event_handle(&buf_received);
        if ( mailbox_status == 0 )
        {
            // Mailbox successfully handled, clear mbox event bit
            event_to_handle &= ~IPC_MBOX_RX_BIT;
        }
    }

    if (buf_received & IPC_INST0_KICK_BIT)
    {
        buf_received &= ~IPC_INST0_KICK_BIT;
        ipc_receive(&host_inst0);
    }


    if (event_to_handle & IPC_HCI_TX_BIT)
    {
        event_to_handle &= ~IPC_HCI_TX_BIT;
            int send_status = rpc_client_hci_send_packet(
                g_pui8UARTRXBuffer, g_ui32UARTRxIndex);
            hci_tx_cnt++;
            if ( send_status < 0 )
            {
                am_util_stdio_printf("send btdm pkt failed, status:%d\r\n",
                                send_status);
            }
            else
            {
                am_util_stdio_printf("RPC HCI pkt tx, len=%d,hci_tx_cnt =%d\r\n", (g_ui32UARTRxIndex + RPMSG_HCI_PDU_HEADER_LEN), hci_tx_cnt);
            }

            g_ui32UARTRxIndex = 0;
            g_bRxTimeoutFlag = false;
            serial_irq_enable();
    }

    if (event_to_handle & IPC_HCI_RX_BIT)
    {
        event_to_handle &= ~IPC_HCI_RX_BIT;

        while(1)
        {
            ipc_data_t *p_buf_data = ipc_data_queue_pop(IPC_DATA_QUEUE_HCI_MSG_RX);

            if ((p_buf_data != NULL))
            {
                serial_data_write(p_buf_data->data, p_buf_data->len);
                am_util_delay_ms(2);
                metal_free_memory(p_buf_data);
            }
            else
            {
                break;
            }
        }
    }
}

//*****************************************************************************
//
// ambt53 Mailbox Interrupt handler.
//
//*****************************************************************************
void mailbox_isr(void)
{
    ipc_set_event(IPC_MBOX_RX_BIT);
}

//*****************************************************************************
//
// ambt53 MSPI Interrupt handler.
//
//*****************************************************************************
void ambt53_mspi_isr(void)
{
    uint32_t ui32Status;

    am_hal_mspi_interrupt_status_get(pvRpmsgMspiHandle, &ui32Status, false);
    am_hal_mspi_interrupt_clear(pvRpmsgMspiHandle, ui32Status);
    am_hal_mspi_interrupt_service(pvRpmsgMspiHandle, ui32Status);
}

//*****************************************************************************
//
// Perform initial setup for the rpmsg task.
//
//*****************************************************************************
void RPMsgTaskSetup(void)
{
    //
    // Boot the RPMsg.
    //
    int status = am_devices_ambt53_boot();
    am_util_debug_printf("boot ambt53, status:%d\r\n", status);
    if (status != 0)
    {
        am_util_stdio_printf("am_devices_ambt53_boot failure\n");
        return;
    }

    // Register the mailbox callback into SCPM interrupt handlers.
    am_devices_ambt53_interrupt_register(SCPM_IRQ_MAILBOX_THRESHOLD, SCPM_IRQ_POS_EDGE, mailbox_isr);

    host_ipc_instance_setup();

    ipc_create_event();

    ipc_data_queue_init(IPC_DATA_QUEUE_HCI_MSG_RX);
    // Register the serial_data_write to output the HCI packet from the controller
    rpc_client_hci_msg_handler_cb_register(ipc_hci_msg_rx_handler);
}

//*****************************************************************************
//
// Short Description.
//
//*****************************************************************************

void RPMsgProcessTask(void *pvParameters)
{
    (void) pvParameters;

    while(1)
    {
        ipc_task_dispatch();
    }
}

void ipc_hci_msg_rx_handler(uint8_t *packet, uint32_t len)
{
    ipc_data_t *p_buf;

    p_buf = (ipc_data_t *)metal_allocate_memory(sizeof(ipc_data_t) + len);

    p_buf->data = (uint8_t *)p_buf + sizeof(ipc_data_t);

    if (p_buf->data != NULL)
    {
        p_buf->len = len;
        memcpy(p_buf->data, packet, len);

        ipc_data_queue_push(IPC_DATA_QUEUE_HCI_MSG_RX, p_buf);

        ipc_set_event(IPC_HCI_RX_BIT);

    }
    else
    {
        am_util_stdio_printf("btdm pkt send alloc fail,len:%d\r\n", (sizeof(ipc_data_t) + len));
    }
}