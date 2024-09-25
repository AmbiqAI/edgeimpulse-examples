//*****************************************************************************
//
//! @file ipc_stress_test_main.c
//!
//! @brief File to handle IPC stress test operation.
//!
//*****************************************************************************

//*****************************************************************************
//
// Copyright (c) 2024, Ambiq Micro, Inc.
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// 1. Redistributions of source code must retain the above copyright notice,
// this list of conditions and the following disclaimer.
//
// 2. Redistributions in binary form must reproduce the above copyright
// notice, this list of conditions and the following disclaimer in the
// documentation and/or other materials provided with the distribution.
//
// 3. Neither the name of the copyright holder nor the names of its
// contributors may be used to endorse or promote products derived from this
// software without specific prior written permission.
//
// Third party software included in this distribution is subject to the
// additional license terms as defined in the /docs/licenses directory.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
// ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
// LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
// CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
// SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
// INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
// CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
// ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
// POSSIBILITY OF SUCH DAMAGE.
//
// This is part of revision release_a5b_sdk2-748191cd0 of the AmbiqSuite Development Package.
//
//*****************************************************************************
#include "../../third_party/open-amp/libmetal/lib/errno.h"
#include "ipc_stress_test_main.h"


//*****************************************************************************
//
// Macro definitions.
//
//*****************************************************************************
#define     INVO0_SEND_EVENT       (0x00)
#define     INVO1_SEND_EVENT       (0x01)
#define     SYNC_EVENT_MASK        (0x02)


#define     RPMSG_QUEUE_HI         (8)
#define     RPMSG_QUEUE_LOW        (1)
#define     AM_TEST_REF_BUF_SIZE   (496) // 512-16
#define     XOR_BYTE               (0)

#define     USE_SYSIRQ             (0)

#define     random_num(max, min)   ((rand() % (max - min + 1)) + min)

// Counts of timer used in this example
#define CONFIG_TIMER_COUNT (2u)
#define TIMER_PER_HIGH               (50u)
#define TIMER_PER_LOW                (5u)


//*****************************************************************************
//
// Global variable.
//
//*****************************************************************************
typedef enum
{
    INVOCATION0_EP,
    INVOCATION1_EP,
    PINGPONG_EP,
    TEST_EP_NUM
} ipc_test_ep_e;

metal_thread_t ipc_send_task_handle;
metal_thread_t ipc_pingpong_task_handle;

EventGroupHandle_t send_event  = NULL;
EventGroupHandle_t sync_event  = NULL;

uint8_t g_pui8TestTXBuf[RPMSG_BUFFER_SIZE];
uint32_t g_sendIdx[3], g_recvIdx[2], g_sendNum[3] = {0};
float g_failNum, g_failRate = 0;

static struct ipc_ept ipc_invocation0_ep;
static struct ipc_ept ipc_invocation1_ep;
static struct ipc_ept ipc_pingpong_ep;
static struct ipc_ept* ipc_test_eps[TEST_EP_NUM] = {&ipc_invocation0_ep, &ipc_invocation1_ep, &ipc_pingpong_ep};

#define EP1_SEND_DATA_PERIOD (50u)
#define EP2_SEND_DATA_PERIOD (EP1_SEND_DATA_PERIOD * 3 / 2)
#define EP1_SW_TIMER_ID (0u)
#define EP2_SW_TIMER_ID (1u)

// An array to hold handles to the created timers.
TimerHandle_t ep_sd_tmr_handler[CONFIG_TIMER_COUNT];
uint32_t tmr_period[CONFIG_TIMER_COUNT] = {EP1_SEND_DATA_PERIOD, EP2_SEND_DATA_PERIOD};
uint32_t tmr_id[CONFIG_TIMER_COUNT] = {EP1_SW_TIMER_ID, EP2_SW_TIMER_ID};

bool g_wait_flag;
uint8_t g_return_value;

//*****************************************************************************
//
// Function prototypes
//
//*****************************************************************************
// delay millisecond while waiting for the response
#define IPC_REQUEST_RSP_WAIT_MS       (1)
// delay maximum counts while waiting for the response
#define IPC_REQUEST_RSP_WAIT_COUNT    (100)
#define IPC_ERR_TIMEOUT               (0xF0)
static void ipc_sync_rsp_wait(void)
{
    static uint16_t wait_cnt = 0;

    do
    {
        wait_cnt ++;

        vTaskDelay(pdMS_TO_TICKS(IPC_REQUEST_RSP_WAIT_MS));

        // wait at most 100ms for the response
        if(wait_cnt > IPC_REQUEST_RSP_WAIT_COUNT)
        {
            g_return_value = IPC_ERR_TIMEOUT;
            g_wait_flag = false;
            break;
        }
    }while (g_wait_flag);

    am_util_stdio_printf("S");

    wait_cnt = 0;
}

static int ipc_send_sync_request(struct ipc_ept *ept, const void *data, size_t len)
{
    int status = RPMSG_SUCCESS;
    uint32_t tx_buf_len = 0;

    metal_assert(ept != NULL);
    metal_assert(data != NULL);
    metal_assert((len <= AM_TEST_REF_BUF_SIZE) && (len > 0));

    if((ept == NULL) || (data == NULL) || (len > AM_TEST_REF_BUF_SIZE)||(len == 0))
    {
        return RPMSG_ERR_PARAM;
    }

    // Set tx_buf_len to 0(unspecific) to get max TX buffer size
    tx_buf_len          = 0;
    uint8_t *tx_buf_ptr = NULL;
    // Get buffer from VRing
    status = ipc_service_get_tx_buffer(
		ept, (void **)&tx_buf_ptr, &tx_buf_len, false);
    if (0 != status)
    {
        am_util_stdio_printf("ipc send sync request, allocate buffer failed\r\n");
        return RPMSG_ERR_NO_BUFF;
    }

    g_wait_flag = true;

    memcpy(tx_buf_ptr, data, len);
    /*Send the buffer out through rpMsg*/
    status = ipc_service_send_nocopy(ept, tx_buf_ptr, len);
    if (status > 0)
    {
        ipc_sync_rsp_wait();
        status = (int)g_return_value;
    }
    else
    {
        am_util_debug_printf("send test sync request fail, status: %d\r\n", status);
        g_wait_flag = false;
    }

    return status;
}

static int parse_data(uint8_t* data, size_t len, uint32_t* idx)
{
    uint32_t i = 0;
    uint8_t ref = 0;

    if ( (!len) || (!data) )
    {
        return RPMSG_ERR_PARAM;
    }

    for (i = 0; i < len; i++)
    {
        ref = (((i & 0xFF) ^ XOR_BYTE) + *idx) % 0x100;

        if (data[i] != ref)
        {
            am_util_stdio_printf("Comparison failed at Index %2d with Received Value %2.2X while expected value was %2.2X\n", i, data[i], ref);
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

static void invocation0_bound(void *priv)
{
    am_util_stdio_printf("invocation0_bound\r\n");
}

static void invocation1_bound(void *priv)
{
    am_util_stdio_printf("invocation1_bound\r\n");
}

static void pingpong_bound(void *priv)
{
    am_util_stdio_printf("pingpong_bound\r\n");
}

static void invocation0_cb(const void *data, size_t len, void *priv)
{
    int status = RPMSG_SUCCESS;

    status = parse_data((uint8_t*)data, len, &g_recvIdx[INVOCATION0_EP]);
    if (status != RPMSG_SUCCESS)
    {
        am_util_stdio_printf("invo0 data parsing %d error at index %d\n", status, g_recvIdx[INVOCATION0_EP]);
    }
    else
    {
        am_util_stdio_printf(".");
    }

    if ((status != RPMSG_SUCCESS) && ( ++g_recvIdx[INVOCATION0_EP] == VRING_SIZE))
    {
        g_recvIdx[INVOCATION0_EP] = 0;
    }
}

static void invocation1_cb(const void *data, size_t len, void *priv)
{
    int status = RPMSG_SUCCESS;

    status = parse_data((uint8_t*)data, len, &g_recvIdx[INVOCATION1_EP]);
    if (status != RPMSG_SUCCESS)
    {
        am_util_stdio_printf("invo1 data parsing %d error at index %d\n", status, g_recvIdx[INVOCATION1_EP]);
    }
    else
    {
        am_util_stdio_printf("*");
    }

    if ((status != RPMSG_SUCCESS) && ( ++g_recvIdx[INVOCATION1_EP] == VRING_SIZE))
    {
        g_recvIdx[INVOCATION1_EP] = 0;
    }
}

static void pingpong_cb(const void *data, size_t len, void *priv)
{
    int status = RPMSG_SUCCESS;

    status = parse_data((uint8_t*)data, len, &g_sendIdx[PINGPONG_EP]);
    am_util_stdio_printf("#");

    // the first byte indicates the result status
    g_return_value = *((uint8_t *)data);
    g_wait_flag = false;

    xEventGroupSetBits(sync_event, SYNC_EVENT_MASK);
    
    if (status != RPMSG_SUCCESS)
    {
        am_util_stdio_printf("pingpong_cb, parse data fail:%d\r\n", status);
    }
}

static struct ipc_ept_cfg host_test_inst0_ep_cfg[] =
{
    [0] =
    {
        .name = "am_rpmsg_demo",
        .cb =
        {
            .bound    = invocation0_bound,
            .received = invocation0_cb,
        },
    },
    [1] =
    {
        .name = "am_rpmsg_demo_ep2",
        .cb =
        {
            .bound    = invocation1_bound,
            .received = invocation1_cb,
        },
    },
    [2] =
    {
        .name = "am_rpmsg_pingpong",
        .cb =
        {
            .bound    = pingpong_bound,
            .received = pingpong_cb,
        },
    }
};

static int ipc_stress_test_task_setup(void)
{
    BaseType_t stat = 0;

    if ((send_event = xEventGroupCreate()) == NULL)
    {
        am_util_stdio_printf("event group failed to alloc");
        return -1;
    }

    if ((sync_event = xEventGroupCreate()) == NULL)
    {
        am_util_stdio_printf("sync event group failed to alloc");
        return -1;
    }

#if (ENABLE_GRAPHIC)
    GraphicTaskSetup();
#endif

    stat = metal_thread_init(&ipc_send_task_handle, IPCSendTask, "IPCSendTask", 1024, 0, 3);
    if (stat != 0)
    {
        am_util_stdio_printf("Cannot create ipc_send_task_handle task \r\n");
        return stat;
    }

    stat = metal_thread_init(&ipc_pingpong_task_handle, IPCPingPongTask, "IPCPingPongTask", 1024, 0, 3);
    if (stat != 0)
    {
        am_util_stdio_printf("Cannot create ipc_pingpong_task_handle task \r\n");
        return stat;
    }
#if (ENABLE_GRAPHIC)
    stat = metal_thread_init(&graphic_store_task_handle, GraiphicStoreTask, "GraphicStoreTask", 1024, 0, 3);
    if (stat != 0)
    {
        am_util_stdio_printf("Cannot create graphic_store_task_handle task \r\n");
        return stat;
    }

    stat = metal_thread_init(&graphic_compose_task_handle, GraiphicComposeTask, "GraphicComposeTask", 1024, 0, 3);
    if (stat != 0)
    {
        am_util_stdio_printf("Cannot create graphic_compose_task_handle task \r\n");
        return stat;
    }
#endif

    // Initialize Test Data
    for (int j = 0; j < RPMSG_BUFFER_SIZE; j++)
    {
        g_pui8TestTXBuf[j] = ((j & 0xFF) ^ XOR_BYTE) % 0x100;
    }

    return stat;
}


/* Make sure we register endpoint before RPMsg Service is initialized. */
static int ipc_test_endpoint_register(struct ipc_device *instance)
{
    int status = 0;

    if(instance == NULL)
    {
        return -EINVAL;
    }

    am_util_debug_printf("ipc_test_endpoint_register\r\n");

    for (uint8_t i = 0; i < TEST_EP_NUM; i++)
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


int ipc_stress_test_setup(struct ipc_device *instance)
{
    int status = 0;

    status = ipc_test_endpoint_register(instance);
    if(status < 0)
    {
        am_util_stdio_printf("stress test task register ep fail, status:%d\r\n", status);

        return status;
    }

    status = ipc_stress_test_task_setup();
    if(status != 0)
    {
        am_util_stdio_printf("stress test task setup fail, status:%d\r\n", status);
    }

    return status;
}

void vTimerCallback(TimerHandle_t pxTimer)
{
    int32_t tmr_idx;

    // Optionally do something if the pxTimer parameter is NULL.
    configASSERT(pxTimer);

    // Which timer expired?
    tmr_idx = (int32_t)pvTimerGetTimerID(pxTimer);

    if (tmr_idx <= (int32_t)INVO1_SEND_EVENT)
    {
        //Notify the send task to send some buffers
        xEventGroupSetBits(send_event, (1 << tmr_idx));
    }
}

//*****************************************************************************
//
// Short Description.
//
//*****************************************************************************
void
IPCSendTask(void *pvParameters)
{
    int status = RPMSG_SUCCESS;
    EventBits_t event = 0;
    uint8_t txFrameNum = 0;

    am_util_stdio_printf("IPCSendTask\r\n");

    /* Since we are using name service, we need to wait for a response
    * from NS setup and than we need to process it
    */
    while ((!ipc_service_is_endpoint_bounded(&ipc_invocation0_ep))
        || (!ipc_service_is_endpoint_bounded(&ipc_invocation1_ep)))
    {
        metal_thread_yield();
    }

    // Start to send

    for (uint32_t i = 0; i < CONFIG_TIMER_COUNT; i++)
    {
        //Start a timer with a certain period, Timer ISR gives a semaphare to wake this task to send a ordered buffer data
        ep_sd_tmr_handler[i] = xTimerCreate("Timer",                      // Just a text name, not used by the kernel.
                                            pdMS_TO_TICKS(tmr_period[i]), // The timer period in ticks.
                                            pdTRUE,                       // The timers will auto-reload themselves when they expire.
                                            (void *)tmr_id[i],            // Assign each timer a unique id equal to its array index.
                                            vTimerCallback                // Each timer calls the same callback when it expires.
        );
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


    while(1)
    {
        event = xEventGroupWaitBits(send_event, INVO0_SEND_EVENT | INVO1_SEND_EVENT, pdTRUE, pdFALSE, portMAX_DELAY);
        for ( uint32_t j = INVOCATION0_EP; j <= INVOCATION1_EP; j++ )
        {
            txFrameNum = random_num(RPMSG_QUEUE_HI, RPMSG_QUEUE_LOW);
            if ( (event & (1 << j)) == 0 )
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
                    break;
                }

                tx_random_len = random_num(AM_TEST_REF_BUF_SIZE, 2);

                memcpy(tx_buf_ptr, &g_pui8TestTXBuf[g_sendIdx[j]], tx_random_len);
                /*Send the buffer out through rpMsg*/
                status = ipc_service_send_nocopy(ipc_test_eps[j], tx_buf_ptr, tx_random_len);

                g_sendNum[j]++;
                if (status < 0)
                {
                    g_failNum++;
                    g_failRate = g_failNum / (g_sendNum[INVOCATION0_EP] + g_sendNum[INVOCATION1_EP] + g_sendNum[PINGPONG_EP]) * 100.00;
                    am_util_stdio_printf("\ninvo%d send_message(%d) failed with status %d, current fail rate is %6.2f%%\n", j, g_sendIdx[j], status, g_failRate);
                    if (status == RPMSG_ERR_NO_BUFF)
                    {
                        am_util_stdio_printf("\nBuffer full, will send again.\n");
                    }
                }
                else
                {
                    g_sendIdx[j]++;
                }
                if (g_sendIdx[j] == VRING_SIZE)
                {
                    g_sendIdx[j] = 0;
                }
            }

            // Change timer period to send the packets periodically
            TickType_t timer_period = random_num(TIMER_PER_HIGH, TIMER_PER_LOW);
            xTimerChangePeriod(ep_sd_tmr_handler[j], timer_period, 0);
        }
    }

}

void
IPCPingPongTask(void *pvParameters)
{
    int status = RPMSG_SUCCESS;

    /* Since we are using name service, we need to wait for a response
    * from NS setup and than we need to process it
    */
    am_util_stdio_printf("IPCPingPongTask\r\n");
    while (!ipc_service_is_endpoint_bounded(&ipc_pingpong_ep))
    {
        metal_thread_yield();
    }

    while(1)
    {
        status = ipc_send_sync_request(&ipc_pingpong_ep, &g_pui8TestTXBuf[g_sendIdx[PINGPONG_EP]], AM_TEST_REF_BUF_SIZE);

        g_sendNum[PINGPONG_EP]++;
        if (status < RPMSG_SUCCESS)
        {
            g_failNum++;
            g_failRate = g_failNum / (g_sendNum[INVOCATION0_EP] + g_sendNum[INVOCATION1_EP] + g_sendNum[PINGPONG_EP]) * 100.00;
            am_util_stdio_printf("Pingpong send_message(%d) failed with status %d, current fail rate is %6.2f%%\n", g_sendIdx[PINGPONG_EP], status, g_failRate);
        }

        xEventGroupWaitBits(sync_event, SYNC_EVENT_MASK , pdTRUE, pdFALSE, portMAX_DELAY);
    }
}

