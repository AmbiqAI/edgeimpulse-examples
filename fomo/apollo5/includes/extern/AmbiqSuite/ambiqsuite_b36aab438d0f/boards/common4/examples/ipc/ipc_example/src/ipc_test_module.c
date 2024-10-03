//*****************************************************************************
//
//! @file ipc_test_module.c
//!
//! @brief File to handle IPC test operation.
//!
//*****************************************************************************

//*****************************************************************************
//
// ${copyright}
//
// This is part of revision ${version} of the AmbiqSuite Development Package.
//
//*****************************************************************************
#include "../../third_party/open-amp/libmetal/lib/errno.h"
#include "ipc_test_module.h"

//*****************************************************************************
//
// Macro definitions.
//
//*****************************************************************************
#define     RPMSG_QUEUE_HI         (VRING_SIZE - 1)
#define     RPMSG_QUEUE_LOW        (1)
#define     AM_TEST_REF_BUF_SIZE   (496) // 512-16
#define     XOR_BYTE               (0)

// Counts of timer used in this example
#define CONFIG_TIMER_COUNT         (2u)
#define TIMER_PER_HIGH             (50u)
#define TIMER_PER_LOW              (5u)


//*****************************************************************************
//
// Global variable.
//
//*****************************************************************************

extern am_devices_mspi_ambt53_t gAMBT53[];
extern EventGroupHandle_t ipc_event;

static uint8_t g_tx_buf[RPMSG_BUFFER_SIZE];
static uint8_t g_rx_buf[RPMSG_BUFFER_SIZE];
static uint32_t g_send_idx[3], g_recv_idx[2], g_send_num[3] = {0};
static float g_fail_num, g_fail_rate = 0;

static struct ipc_ept ipc_inst0_ep0;
static struct ipc_ept ipc_inst0_ep1;
// static struct ipc_ept ipc_inst0_pingpong_ep;
static struct ipc_ept* ipc_test_eps[INST0_EP_NUM] = {&ipc_inst0_ep0, &ipc_inst0_ep1};
int32_t tmr_idx;

#define EP0_SEND_DATA_PERIOD (50u)
#define EP1_SEND_DATA_PERIOD (EP0_SEND_DATA_PERIOD * 3 / 2)
#define EP0_SW_TIMER_ID      (INST0_EP0)
#define EP1_SW_TIMER_ID      (INST0_EP1)

// An array to hold handles to the created timers.
static TimerHandle_t ep_sd_tmr_handler[CONFIG_TIMER_COUNT];
static uint32_t tmr_period[CONFIG_TIMER_COUNT] = {EP0_SEND_DATA_PERIOD, EP1_SEND_DATA_PERIOD};
static uint32_t tmr_id[CONFIG_TIMER_COUNT] = {EP0_SW_TIMER_ID, EP1_SW_TIMER_ID};

//*****************************************************************************
//
// Function prototypes
//
//*****************************************************************************
static void vTimerCallback(TimerHandle_t pxTimer);

static int parse_data(uint8_t* data, size_t len, uint32_t* idx)
{
    uint32_t i = 0;
    uint8_t ref = 0;

    if ( (!len) || (!data) )
    {
        return RPMSG_ERR_PARAM;
    }

    // Read back the data by random length to simulate aligned/unaligned
    // source address and different length XIPMM operation.
    uint16_t read_len = random_num(len, 1);
    uint16_t last_pkt = len % read_len;
    for (i = 0; i < (len / read_len); i++)
    {
        memcpy(&g_rx_buf[i * read_len], &data[i * read_len], read_len);
    }
    if (last_pkt)
    {
        memcpy(&g_rx_buf[len - last_pkt], &data[len - last_pkt], last_pkt);
    }

    for (i = 0; i < len; i++)
    {
        // Delay the processing to make the remote sender busy randomly
        am_util_delay_cycles(random_num(20, 0));
        ref = (((i & 0xFF) ^ XOR_BYTE) + *idx) % 0x100;

        if (g_rx_buf[i] != ref)
        {
            ambt53_status_reg_t stStatus = {0};
            am_devices_mspi_ambt53_status_get((void*)&gAMBT53[0], &stStatus);
            am_util_stdio_printf("XSPI Status: 0x%X\n", stStatus.STATUS);
            am_util_stdio_printf("Comparison failed at Index %2d with Received Value %2.2X while expected value was %2.2X\n", i, g_rx_buf[i], ref);
            am_util_stdio_printf("Failed data address: 0x%08X, XIPMM length: %d\r\n", &data[i], read_len);
            break;
        }
    }

    (*idx)++;

    if ( *idx == VRING_SIZE )
    {
        *idx = 0;
        am_util_stdio_printf("\n");
    }

    if ( i != len )
    {
        return RPMSG_ERR_DATA;
    }

    return RPMSG_SUCCESS;
}

static void ipc_inst0_ep0_bound(void *priv)
{
    am_util_stdio_printf("ipc_inst0_ep0_bound\r\n");
}

static void ipc_inst0_ep1_bound(void *priv)
{
    am_util_stdio_printf("ipc_inst0_ep1_bound\r\n");
}

static void ipc_inst0_ep0_cb(const void *data, size_t len, void *priv)
{
    int status = RPMSG_SUCCESS;

    status = parse_data((uint8_t*)data, len, &g_recv_idx[INST0_EP0]);
    if (status != RPMSG_SUCCESS)
    {
        am_util_stdio_printf("invo0 data parsing %d error at index %d\n", status, g_recv_idx[INST0_EP0]);
        while (1);
    }
    else
    {
        am_util_stdio_printf(".");
    }

    if ((status != RPMSG_SUCCESS) && ( ++g_recv_idx[INST0_EP0] == VRING_SIZE))
    {
        g_recv_idx[INST0_EP0] = 0;
    }
}

static void ipc_inst0_ep1_cb(const void *data, size_t len, void *priv)
{
    int status = RPMSG_SUCCESS;

    status = parse_data((uint8_t*)data, len, &g_recv_idx[INST0_EP1]);
    if (status != RPMSG_SUCCESS)
    {
        am_util_stdio_printf("invo1 data parsing %d error at index %d\n", status, g_recv_idx[INST0_EP1]);
        while (1);
    }
    else
    {
        am_util_stdio_printf("*");
    }

    if ((status != RPMSG_SUCCESS) && ( ++g_recv_idx[INST0_EP1] == VRING_SIZE))
    {
        g_recv_idx[INST0_EP1] = 0;
    }
}

static struct ipc_ept_cfg host_test_inst0_ep_cfg[] =
{
    [INST0_EP0] =
    {
        .name = "am_rpmsg_demo",
        .cb =
        {
            .bound    = ipc_inst0_ep0_bound,
            .received = ipc_inst0_ep0_cb,
        },
    },
    [INST0_EP1] =
    {
        .name = "am_rpmsg_demo_ep2",
        .cb =
        {
            .bound    = ipc_inst0_ep1_bound,
            .received = ipc_inst0_ep1_cb,
        },
    },
};

/* Make sure we register endpoint before RPMsg Service is initialized. */
static int ipc_test_endpoint_register(struct ipc_device *instance)
{
    int status = 0;

    if ( instance == NULL )
    {
        return -EINVAL;
    }

    am_util_debug_printf("ipc_test_endpoint_register\r\n");

    for (uint8_t i = 0; i < INST0_EP_NUM; i++)
    {
        status = ipc_service_register_endpoint(instance, ipc_test_eps[i], &host_test_inst0_ep_cfg[i]);
        if (status < 0)
        {
            am_util_stdio_printf ("ipc_service_register_endpoint failure %d, status:%d\n", i, status);
            return status;
        }
    }

    return status;
}


int ipc_test_setup(struct ipc_device *instance)
{
    int status = 0;

    status = ipc_test_endpoint_register(instance);
    if ( status < 0 )
    {
        am_util_stdio_printf("stress test task register ep fail, status:%d\r\n", status);

        return status;
    }

    // Initialize Test Data
    for (int j = 0; j < RPMSG_BUFFER_SIZE; j++)
    {
        g_tx_buf[j] = ((j & 0xFF) ^ XOR_BYTE) % 0x100;
    }

    // initiate timer for triggering sending data

    for (uint32_t i = 0; i < CONFIG_TIMER_COUNT; i++)
    {
        //Start a timer with a certain period, Timer ISR gives a semaphare to wake this task to send a ordered buffer data
        ep_sd_tmr_handler[i] = xTimerCreate("Timer",
                                            pdMS_TO_TICKS(tmr_period[i]),
                                            pdTRUE,
                                            (void *)tmr_id[i],
                                            vTimerCallback);
        if (ep_sd_tmr_handler[i] == NULL)
        {
            // The timer was not created.
            am_util_stdio_printf("ep_sd_tmr_handler%d create fail\r\n", i);
        }
        else
        {
            // Start the timer.  No block time is specified, and even if one was
            // it would be ignored because the scheduler has not yet been
            // started.
            if (xTimerStart(ep_sd_tmr_handler[i], 0) != pdPASS)
            {
                // The timer could not be set into the Active state.
                am_util_stdio_printf("ep_sd_tmr_handler%d start fail\r\n", i);
            }
        }
    }

    return status;
}

static void vTimerCallback(TimerHandle_t pxTimer)
{
    int32_t tmr_idx;

    // Optionally do something if the pxTimer parameter is NULL.
    configASSERT(pxTimer);

    // Which timer expired?
    tmr_idx = (int32_t)pvTimerGetTimerID(pxTimer);

    if (tmr_idx <= (int32_t)CONFIG_TIMER_COUNT)
    {
        if ((ipc_service_is_endpoint_bounded(&ipc_inst0_ep0))
        && (ipc_service_is_endpoint_bounded(&ipc_inst0_ep1)))
        {
            //Notify the ipc task to send some buffers
            xEventGroupSetBits(ipc_event, (1 << tmr_idx));
        }
    }
}

//*****************************************************************************
//
// Short Description.
//
//*****************************************************************************
uint32_t ipc_send_data_event_handler(uint32_t ept_to_send)
{
    int status = RPMSG_SUCCESS;
    uint8_t txFrameNum = 0;
    uint32_t total_sent_num = 0;

    /* Since we are using name service, we need to wait for a response
    * from NS setup and than we need to process it
    */
    if ((!ipc_service_is_endpoint_bounded(&ipc_inst0_ep0))
        || (!ipc_service_is_endpoint_bounded(&ipc_inst0_ep1)))
    {
        return AM_DEVICES_AMBT53_STATUS_FAIL;
    }

    for ( uint32_t j = INST0_EP0; j <= INST0_EP1; j++ )
    {
        txFrameNum = random_num(RPMSG_QUEUE_HI, RPMSG_QUEUE_LOW);
        if ( (ept_to_send & (1 << j)) == 0 )
        {
            continue;
        }
        for (int i = 0; i < txFrameNum; i++)
        {
            uint32_t tx_buf_len = 0;
            uint16_t tx_random_len = 0;
            // Get buffer from VRing
            uint8_t *tx_buf_ptr = NULL;
            status = ipc_service_get_tx_buffer(ipc_test_eps[j], (void **)&tx_buf_ptr, &tx_buf_len, false);
            if (0 != status)
            {
                am_util_stdio_printf("EP%d Get buffer failed\r\n", j);
                return AM_DEVICES_AMBT53_STATUS_FAIL;
            }

            tx_random_len = random_num(AM_TEST_REF_BUF_SIZE, 2);

            memcpy(tx_buf_ptr, &g_tx_buf[g_send_idx[j]], tx_random_len);
            /*Send the buffer out through rpMsg*/
            status = ipc_service_send_nocopy(ipc_test_eps[j], tx_buf_ptr, tx_random_len);

            g_send_num[j]++;
            if (status < 0)
            {
                g_fail_num++;
                for (uint32_t i = 0; i < INST0_EP_NUM; i++)
                {
                    total_sent_num += g_send_num[i];
                }

                g_fail_rate = g_fail_num / (total_sent_num) * 100.00;
                am_util_stdio_printf("\ninvo%d send_message(%d) failed with status %d, current fail rate is %6.2f%%\n", j, g_send_idx[j], status, g_fail_rate);
                if (status == RPMSG_ERR_NO_BUFF)
                {
                    // Do not expect to get here
                    am_util_stdio_printf("\nBuffer full, will send again.\n");
                }
            }
            else
            {
                g_send_idx[j]++;
            }
            if (g_send_idx[j] == VRING_SIZE)
            {
                g_send_idx[j] = 0;
            }
        }

        // Change timer period to send the packets periodically
        TickType_t timer_period = random_num(TIMER_PER_HIGH, TIMER_PER_LOW);
        xTimerChangePeriod(ep_sd_tmr_handler[j], timer_period, 0);
    }
    return AM_DEVICES_AMBT53_STATUS_SUCCESS;
}

void ipc_set_send_data_event(void)
{
     BaseType_t xHigherPriorityTaskWoken = pdFALSE;
     BaseType_t result                   = pdFALSE;

    if (pdTRUE == xPortIsInsideInterrupt())
    {
        result = xEventGroupSetBitsFromISR(
            ipc_event, (1 << tmr_idx), &xHigherPriorityTaskWoken);
        if (result == pdPASS)
        {
            portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
        }
    }
    else
    {
        xEventGroupSetBits(ipc_event, (1 << tmr_idx));
    }
}
