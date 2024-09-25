/*
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdio.h>
#include <errno.h>
#include "metal-test.h"
#include <./mqueue.h>
#include <./thread.h>
#include <./log.h>
#include <./sys.h>

static const int test_count = 1000;

static const int queue_len = 32;
static const int item_size = sizeof(uint32_t);
static int error_count     = 0;
static const uint32_t stop_val = 0xDEADBEEF;
struct mqueue_test_data
{
    metal_mqueue_t queue[2];
};

metal_thread_t sender_thread;
metal_thread_t echo_thread;


static void queue_sender_thread(void *arg)
{
	struct mqueue_test_data *data = (struct mqueue_test_data *)arg;
	uint32_t i;
    uint32_t rval;
	for (i = 0; i < test_count; i++)
    {
        metal_mqueue_send( &data->queue[0], &i, METAL_MQUEUE_INF_TIMEOUT );
        rval = 0x5a5a5a5a;
        metal_mqueue_receive( &data->queue[1], &rval, METAL_MQUEUE_INF_TIMEOUT );
        if (rval != i)
            error_count++;
    }
    i = stop_val;
    metal_mqueue_send( &data->queue[0], &i, METAL_MQUEUE_INF_TIMEOUT );

    metal_thread_terminate(NULL);
}

static void queue_echo_thread(void *arg)
{
	struct mqueue_test_data *data = (struct mqueue_test_data *)arg;
    uint32_t rval = 0;
    while(1)
    {
        metal_mqueue_receive( &data->queue[0], &rval, METAL_MQUEUE_INF_TIMEOUT );
        if (rval != stop_val)
        {
            metal_mqueue_send( &data->queue[1], &rval, METAL_MQUEUE_INF_TIMEOUT );
        }
        else
        {
            break;
        }
    }

    metal_thread_terminate(NULL);
}

static int thread(void)
{
	int error = 0;
    int ret;
    struct mqueue_test_data data;
    const uint32_t STACK_SIZE = 16384;

    metal_mqueue_init( &data.queue[0], queue_len, item_size );
    metal_mqueue_init( &data.queue[1], queue_len, item_size );

    ret = metal_thread_init(&sender_thread,
                      queue_sender_thread,
                      "sender",
                      STACK_SIZE,
                      &data,
                      2);

    if (ret < 0) {
		metal_log(METAL_LOG_ERROR, "Failed to create sender thread: %d.\n", ret);
        error = ret;
        goto out;
	}

    ret = metal_thread_init(&echo_thread,
                      queue_echo_thread,
                      "echo",
                      STACK_SIZE,
                      &data,
                      2);

    if (ret < 0) {
		metal_log(METAL_LOG_ERROR, "Failed to create echo thread: %d.\n", ret);
        error = ret;
	}
    else
    {
        (void)metal_thread_join(&echo_thread);
    }

    (void)metal_thread_join(&sender_thread);

out:
   	if (!error) {
		if (error_count) {
			metal_log(METAL_LOG_DEBUG, "Number of errors = %d\n", error_count);
			error = -EINVAL;
		}
	}

    metal_mqueue_deinit( &data.queue[0] );
    metal_mqueue_deinit( &data.queue[1] );

	return error;
}

METAL_ADD_TEST(thread)
