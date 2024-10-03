/*
 * Copyright (c) 2021 Carlo Caione <ccaione@baylibre.com>
 * Copyright (c) 2022, Ambiq Micro, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#ifndef IPC_SERVICE_IPC_RPMSG_STATIC_VRINGS_H_
#define IPC_SERVICE_IPC_RPMSG_STATIC_VRINGS_H_

#include "dsp/none.h"
#include "dsp/utils.h"
#include "virtio_ring.h"
#include "rpmsg_virtio.h"

#include <ipc/ipc_static_vrings.h>

/*
 * Endpoint registration flow:
 *
 *	>>> Case #1: Endpoint registered on HOST first <<<
 *
 *  [B] backend
 *  [O] OpenAMP
 *
 * REMOTE				HOST
 * -----------------------------------------------------------------
 *					[B] register_ept **
 * [B] register_ept **
 * [B] ipc_rpmsg_register_ept
 * [B] rpmsg_create_ept
 * [O] rpmsg_send_ns_message
 * [O] virtqueue_kick
 * [O] virtio_notify_cb
 * [B] mbox_send
 *					[B] mbox_callback
 *					[B] mbox_callback_process
 *					[B] virtqueue_notification
 *					[O] rpmsg_virtio_rx_callback
 *					[B] ns_bind_cb
 *					[B] rpmsg_create_ept
 *					[B] bound_cb
 *					[B] rpmsg_send
 *					[B] virtio_notify_cb
 *					[B] mbox_send
 * [B] mbox_callback
 * [B] mbox_callback_process
 * [B] virtqueue_notification
 * [O] rpmsg_virtio_rx_callback
 * [O] ept_cb
 * [B] bound_cb
 *
 *	>>> Case #2: Endpoint registered on REMOTE first <<<
 *
 *  [B] backend
 *  [O] OpenAMP
 *
 * REMOTE				HOST
 * -----------------------------------------------------------------
 * [B] register_ept **
 * [B] ipc_rpmsg_register_ept
 * [B] rpmsg_create_ept
 * [O] rpmsg_send_ns_message
 * [O] virtqueue_kick
 * [O] virtio_notify_cb
 * [O] mbox_send
 *					[B] mbox_callback
 *					[B] mbox_callback_process
 *					[B] virtqueue_notification
 *					[O] rpmsg_virtio_rx_callback
 *					[B] ns_bind_cb
 *
 *					[B] register_ept **
 *					[B] rpmsg_create_ept
 *					[B] bound_cb
 *					[B] rpmsg_send
 *					[B] virtio_notify_cb
 *					[B] mbox_send
 * [B] mbox_callback
 * [B] mbox_callback_process
 * [B] virtqueue_notification
 * [O] rpmsg_virtio_rx_callback
 * [O] ept_cb
 * [B] bound_cb
 *
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 *
 * Endpoint registration flow (with focus on backend):
 *
 *	>>> Case #1: Endpoint registered on HOST first <<<
 *
 * REMOTE				HOST
 * -----------------------------------------------------------------
 *					register_ept()
 *					register_ept_on_host()
 *					get_ept() returns NULL
 *					name is cached in rpmsg_ept->name
 * register_ept()
 * register_ept_on_remote()
 * ipc_rpmsg_register_ept()
 *					ns_bind_cb()
 *					get_ept() returns endpoint with cached name
 *					advertise_ept()
 *					rpmsg_create_ept()
 *					bound_cb()
 *					rpmsg_send()
 * mbox_callback()
 * mbox_callback_process()
 * virtqueue_notification()
 * ept_cb()
 * bound_cb()
 *
 *	>>> Case #2: Endpoint registered on REMOTE first <<<
 *
 * REMOTE				HOST
 * -----------------------------------------------------------------
 * register_ept()
 * register_ept_on_remote()
 * ipc_rpmsg_register_ept()
 *					ns_bind_cb()
 *					get_ept() return NULL
 *					name is cached in rpmsg_ept->name
 *					...
 *					register_ept()
 *					register_ept_on_host()
 *					get_ept() returns endpoint with cached name
 *					advertise_ept()
 *					rpmsg_create_ept()
 *					bound_cb()
 *					rpmsg-send()
 * mbox_callback()
 * mbox_callback_process()
 * virtqueue_notification()
 * ept_cb()
 * bound_cb()
 *
 */


/**
 *
 * @brief Count leading zeros.
 *
 * @param[in] val Value to count the number of leading zeros on.
 *
 * @return Number of leading zeros when value is written as 32 bits.
 */
static inline uint32_t clz(uint32_t val)
{
    #if defined(__arm__)
    return __builtin_clz(val);
    #elif defined(__GNUC__)
    if (val == 0)
    {
        return 32;
    }
    return __builtin_clz(val);
    #else
    uint32_t i;
    for (i = 0; i < 32; i++)
    {
        if (val & BIT_GET(31 - i))
            break;
    }
    return i;
    #endif // defined(__arm__)
}

/**
 *
 * @brief find most significant bit set in a 32-bit word
 *
 * This routine finds the first bit set starting from the most significant bit
 * in the argument passed in and returns the index of that bit.  Bits are
 * numbered starting at 1 from the least significant bit.  A return value of
 * zero indicates that the value passed is zero.
 *
 * @return most significant bit set, 0 if @a op is 0
 */

static inline unsigned int find_msb_set(uint32_t op)
{
    if (op == 0)
    {
        return 0;
    }

    return 32 - clz(op);
}

static inline size_t vq_ring_size(unsigned int num)
{
    return ROUND_UP((RPMSG_BUFFER_SIZE * num), MEM_ALIGNMENT);
}

static inline size_t shm_size(unsigned int num)
{
    return (VRING_COUNT * (vq_ring_size(num) +
            ROUND_UP(vring_size(num, MEM_ALIGNMENT), MEM_ALIGNMENT)));
}

static inline unsigned int optimal_num_desc(size_t shm_size)
{
    size_t available, single_alloc;
    unsigned int num_desc;

    available = shm_size - VDEV_STATUS_SIZE;
    single_alloc = VRING_COUNT * (vq_ring_size(1) + vring_size(1, MEM_ALIGNMENT));

    num_desc = (unsigned int)(available / single_alloc);

    return (1 << (find_msb_set(num_desc) - 1));
}

#endif /* IPC_SERVICE_IPC_RPMSG_STATIC_VRINGS_H_ */
