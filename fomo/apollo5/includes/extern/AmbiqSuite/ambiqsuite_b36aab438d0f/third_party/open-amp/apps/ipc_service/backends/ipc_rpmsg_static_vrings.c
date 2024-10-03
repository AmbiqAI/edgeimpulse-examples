/*
 * Copyright (c) 2021 Carlo Caione <ccaione@baylibre.com>
 * Copyright (c) 2022, Ambiq Micro, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "../../libmetal/lib/errno.h"
#include "../../libmetal/lib/atomic.h"
#include "am_devices_ambt53.h"

#include <ipc/ipc_service_backend.h>
#include <ipc/ipc_static_vrings.h>
#include <ipc/ipc_rpmsg.h>

#include "ipc_rpmsg_static_vrings.h"
#include "rpmsg_internal.h"

#define STATE_READY (0)
#define STATE_BUSY (1)
#define STATE_INITED (2)

/*
 * Each XIPMM reading will get back 16 bytes each packet. Setting 
 * RPMSG_RX_STORE_IN_LOCAL to 1 to read the data in share memory by 16 bytes
 * here and store to local buffer may reduce XIPMM traffic. Then the application
 * layer will get the data stored in the local buffer directly.
 */
#define RPMSG_RX_STORE_IN_LOCAL (0)

#if RPMSG_RX_STORE_IN_LOCAL
// Data buffer to store the current rpmsg received packet
static uint8_t g_rpmsg_rx_buffer[RPMSG_BUFFER_SIZE - sizeof(struct rpmsg_hdr)];
#endif

struct backend_data_t
{
    /* RPMsg */
    struct ipc_rpmsg_instance rpmsg_inst;

    /* Static VRINGs */
    struct ipc_static_vrings vr;

    unsigned int role;
    atomic_int state;
};

struct ipc_device *rpmsg_instance[NUM_INSTANCES] = {NULL};

static void rpmsg_service_unbind(struct rpmsg_endpoint *ep)
{
    rpmsg_destroy_ept(ep);
}

static struct ipc_rpmsg_ept *get_ept_slot_with_name(struct ipc_rpmsg_instance *rpmsg_inst,
                                                    const char *name)
{
    struct ipc_rpmsg_ept *rpmsg_ept;

    for (size_t i = 0; i < NUM_ENDPOINTS; i++)
    {
        rpmsg_ept = &rpmsg_inst->endpoint[i];

        if (strcmp(name, rpmsg_ept->name) == 0)
        {
            return &rpmsg_inst->endpoint[i];
        }
    }

    return NULL;
}

static struct ipc_rpmsg_ept *get_available_ept_slot(struct ipc_rpmsg_instance *rpmsg_inst)
{
    return get_ept_slot_with_name(rpmsg_inst, "");
}

/*
 * Returns:
 *  - true:  when the endpoint was already cached / registered
 *  - false: when the endpoint was never registered before
 *
 * Returns in **rpmsg_ept:
 *  - The endpoint with the name *name if it exists
 *  - The first endpoint slot available when the endpoint with name *name does
 *    not exist
 *  - NULL in case of error
 */
static bool get_ept(struct ipc_rpmsg_instance *rpmsg_inst,
                    struct ipc_rpmsg_ept **rpmsg_ept, const char *name)
{
    struct ipc_rpmsg_ept *ept;

    ept = get_ept_slot_with_name(rpmsg_inst, name);
    if (ept != NULL)
    {
        (*rpmsg_ept) = ept;
        return true;
    }

    ept = get_available_ept_slot(rpmsg_inst);
    if (ept != NULL)
    {
        (*rpmsg_ept) = ept;
        return false;
    }

    (*rpmsg_ept) = NULL;

    return false;
}

static void advertise_ept(struct ipc_rpmsg_instance *rpmsg_inst, struct ipc_rpmsg_ept *rpmsg_ept,
                          const char *name, uint32_t dest)
{
    struct rpmsg_device *rdev;
    int err;

    rdev = rpmsg_virtio_get_rpmsg_device(&rpmsg_inst->rvdev);

    err = rpmsg_create_ept(&rpmsg_ept->ep, rdev, name, RPMSG_ADDR_ANY,
                           dest, rpmsg_inst->cb, rpmsg_service_unbind);
    if (err != 0)
    {
        return;
    }

    rpmsg_ept->bound = true;
    if (rpmsg_inst->bound_cb)
    {
        rpmsg_inst->bound_cb(rpmsg_ept);
    }
}

static void ns_bind_cb(struct rpmsg_device *rdev, const char *name, uint32_t dest)
{
    struct ipc_rpmsg_instance *rpmsg_inst;
    struct rpmsg_virtio_device *p_rvdev;
    struct ipc_rpmsg_ept *rpmsg_ept;
    bool ept_cached;

    p_rvdev = CONTAINER_OF(rdev, struct rpmsg_virtio_device, rdev);
    rpmsg_inst = CONTAINER_OF(p_rvdev->shpool, struct ipc_rpmsg_instance, shm_pool);

    if (name == NULL || name[0] == '\0')
    {
        return;
    }

    __metal_mutex_acquire(&rpmsg_inst->mtx);
    ept_cached = get_ept(rpmsg_inst, &rpmsg_ept, name);

    if (rpmsg_ept == NULL)
    {
        metal_mutex_release(&rpmsg_inst->mtx);
        return;
    }

    if (ept_cached)
    {
        /*
         * The endpoint was already registered by the HOST core. The
         * endpoint can now be advertised to the REMOTE core.
         */
        metal_mutex_release(&rpmsg_inst->mtx);
        advertise_ept(rpmsg_inst, rpmsg_ept, name, dest);
    }
    else
    {
        /*
         * The endpoint is not registered yet, this happens when the
         * REMOTE core registers the endpoint before the HOST has
         * had the chance to register it. Cache it saving name and
         * destination address to be used by the next register_ept()
         * call by the HOST core.
         */
        strncpy(rpmsg_ept->name, name, sizeof(rpmsg_ept->name));
        rpmsg_ept->dest = dest;
        metal_mutex_release(&rpmsg_inst->mtx);
    }
}

static void bound_cb(struct ipc_rpmsg_ept *ept)
{
    rpmsg_send(&ept->ep, (uint8_t *)"", 0);

    if (ept->cb->bound)
    {
        ept->cb->bound(ept->priv);
    }
}

static int ept_cb(struct rpmsg_endpoint *ep, void *data, size_t len, uint32_t src, void *priv)
{
    struct ipc_rpmsg_ept *ept;

    ept = (struct ipc_rpmsg_ept *)priv;

    /*
     * the remote processor has send a ns announcement, we use an empty
     * message to advice the remote side that a local endpoint has been
     * created and that the processor is ready to communicate with this
     * endpoint
     *
     * ipc_rpmsg_register_ept
     *  rpmsg_send_ns_message --------------> ns_bind_cb
     *                                        bound_cb
     *                ept_cb <--------------- rpmsg_send [empty message]
     *              bound_cb
     */
    if (len == 0)
    {
        if (!ept->bound)
        {
            ept->bound = true;
            bound_cb(ept);
        }
        return RPMSG_SUCCESS;
    }

#if RPMSG_RX_STORE_IN_LOCAL
    uint16_t length = (uint16_t)len;
    uint8_t *pdata = (uint8_t*)data;
    uint8_t last_pkt = length % 16;
    for (uint16_t i = 0; i < (length / 16); i++)
    {
        memcpy(&g_rpmsg_rx_buffer[i * 16], &pdata[i * 16], 16);
    }
    if (last_pkt)
    {
        memcpy(&g_rpmsg_rx_buffer[length - last_pkt], &pdata[length - last_pkt], last_pkt);
    }

    if (ept->cb->received)
    {
        ept->cb->received(g_rpmsg_rx_buffer, len, ept->priv);
    }
#else
    if (ept->cb->received)
    {
        ept->cb->received(data, len, ept->priv);
    }
#endif

    return RPMSG_SUCCESS;
}

static int vr_shm_configure(struct ipc_static_vrings *vr, const struct backend_config *conf)
{
    unsigned int num_desc;

    num_desc = optimal_num_desc(conf->shm_size);
    if (num_desc == 0)
    {
        return -ENOMEM;
    }

    vr->shm_addr = ROUND_UP(conf->shm_addr + VDEV_STATUS_SIZE, MEM_ALIGNMENT);
    vr->shm_size = shm_size(num_desc);

    vr->rx_addr = vr->shm_addr + VRING_COUNT * vq_ring_size(num_desc);
    vr->tx_addr = ROUND_UP(vr->rx_addr + vring_size(num_desc, MEM_ALIGNMENT), MEM_ALIGNMENT);

    vr->status_reg_addr = conf->shm_addr;

    vr->vring_size = num_desc;

    return 0;
}

static void virtio_notify_cb(struct virtqueue *vq, void *priv)
{
    struct backend_config *conf = priv;
    mbox_send(conf->mbox_data_send);
}

// TODO: optimize receive handler
void ipc_receive(struct ipc_device *instance)
{
    struct backend_data_t *data;
    unsigned int vq_id;

    data = instance->data;
    vq_id = (data->role == ROLE_HOST) ? VIRTQUEUE_ID_HOST : VIRTQUEUE_ID_REMOTE;

    virtqueue_notification(data->vr.vq[vq_id]);
}

bool mbox_send(uint32_t data)
{
    uint32_t wr_len = 1;
    uint32_t data_buf = data;
    am_devices_ambt53_mailbox_write_data(&data_buf, &wr_len);
    return true;
}

//*****************************************************************************
//
// Read mailbox FIFO and clear interrupts/errors.
//
//*****************************************************************************
uint32_t mailbox_event_handle(uint32_t* buf_received_bitmap)
{
    uint32_t       d2m_error_val = 0;
    uint32_t       d2m_int_status = 0;
    uint32_t       d2m_data;
    uint32_t       rd_len = 1;
    // Clear the flag to avoid garbage data returned
    *buf_received_bitmap = 0;

    am_devices_ambt53_mailbox_get_d2m_int_status(&d2m_int_status);
    
    if (d2m_int_status & MAILBOX_INT_THRESHOLD_Msk)
    {
        /*Read all data stored in mailbox FIFO*/
        while (1)
        {
            d2m_data = 0;
            rd_len = 1;//Read FIFO one by one
            am_devices_ambt53_mailbox_read_data(&d2m_data, &rd_len);
            if(rd_len != 0)
            {
                for (uint32_t i = 0; i < NUM_INSTANCES; i++)
                {
                    if ((rpmsg_instance[i] != NULL) &&
                        (d2m_data == rpmsg_instance[i]->config.mbox_data_rcv))
                    {
                        rpmsg_instance[i]->config.user_kick_cbk(
                            buf_received_bitmap);
                    }
                }
            }
            else
            {
                break;
            }
        }
    }

    if (d2m_int_status & MAILBOX_INT_ERROR_Msk)
    {
        /*Read to clear the error status register*/
        am_devices_ambt53_mailbox_get_d2m_err(&d2m_error_val);
        if (d2m_error_val & MAILBOX_ERROR_FULL_Msk)
        {
            // FIFO has been cleared
        }
        if (d2m_error_val & MAILBOX_ERROR_EMPTY_Msk)
        {
            // Nothing to do
        }
        // Clear the error bit
        am_devices_ambt53_mailbox_clear_d2m_err(d2m_error_val);
    }
    // Clear iterrupts at end due to the interrupts are level sensitive, they
    // can be cleared only after interrupt source is deactive, new interrupt
    // coming in will not be impacted, the core will re-enter this ISR later
    // Clear both interrupts no matter what is read from IS register,
    am_devices_ambt53_mailbox_clear_d2m_int(MAILBOX_INT_THRESHOLD_Msk | MAILBOX_INT_ERROR_Msk);
    // Check IS again, to avoid XSPI error do not clear the interrupt successfully
    am_devices_ambt53_mailbox_get_d2m_int_status(&d2m_int_status);
    return d2m_int_status;
}

// Return 0 if success
static int mbox_init(void)
{
    // TODO
    // No mailbox initialization code is needed at this moment.
    // Might register mailbox callback here if available.
    return 0;
}

static struct ipc_rpmsg_ept *register_ept_on_host(struct ipc_rpmsg_instance *rpmsg_inst,
                                                  const struct ipc_ept_cfg *cfg)
{
    struct ipc_rpmsg_ept *rpmsg_ept;
    bool ept_cached;

    __metal_mutex_acquire(&rpmsg_inst->mtx);

    ept_cached = get_ept(rpmsg_inst, &rpmsg_ept, cfg->name);
    if (rpmsg_ept == NULL)
    {
        metal_mutex_release(&rpmsg_inst->mtx);
        return NULL;
    }

    rpmsg_ept->cb = &cfg->cb;
    rpmsg_ept->priv = cfg->priv;
    rpmsg_ept->bound = false;
    rpmsg_ept->ep.priv = rpmsg_ept;

    if (ept_cached)
    {
        /*
         * The endpoint was cached in the NS bind callback. We can finally
         * advertise it.
         */
        metal_mutex_release(&rpmsg_inst->mtx);
        advertise_ept(rpmsg_inst, rpmsg_ept, cfg->name, rpmsg_ept->dest);
    }
    else
    {
        /*
         * There is no endpoint in the cache because the REMOTE has
         * not registered the endpoint yet. Cache it.
         */
        strncpy(rpmsg_ept->name, cfg->name, sizeof(rpmsg_ept->name) - 1);
        metal_mutex_release(&rpmsg_inst->mtx);
    }

    return rpmsg_ept;
}

static struct ipc_rpmsg_ept *register_ept_on_remote(struct ipc_rpmsg_instance *rpmsg_inst,
                                                    const struct ipc_ept_cfg *cfg)
{
    struct ipc_rpmsg_ept *rpmsg_ept;
    int err;

    rpmsg_ept = get_available_ept_slot(rpmsg_inst);
    if (rpmsg_ept == NULL)
    {
        return NULL;
    }

    rpmsg_ept->cb = &cfg->cb;
    rpmsg_ept->priv = cfg->priv;
    rpmsg_ept->bound = false;
    rpmsg_ept->ep.priv = rpmsg_ept;

    strncpy(rpmsg_ept->name, cfg->name, sizeof(rpmsg_ept->name) - 1);

    err = ipc_rpmsg_register_ept(rpmsg_inst, RPMSG_REMOTE, rpmsg_ept);
    if (err != 0)
    {
        return NULL;
    }

    return rpmsg_ept;
}

static int register_ept(const struct ipc_device *instance, void **token,
                        const struct ipc_ept_cfg *cfg)
{
    struct backend_data_t *data = instance->data;
    struct ipc_rpmsg_instance *rpmsg_inst;
    struct ipc_rpmsg_ept *rpmsg_ept;

    /* Instance is not ready */
    if (__atomic_load_n((int *)&(data->state), __ATOMIC_SEQ_CST) !=
        STATE_INITED)
    {
        return -EBUSY;
    }

    /* Empty name is not valid */
    if (cfg->name == NULL || cfg->name[0] == '\0')
    {
        return -EINVAL;
    }

    rpmsg_inst = &data->rpmsg_inst;

    rpmsg_ept = (data->role == ROLE_HOST) ? register_ept_on_host(rpmsg_inst, cfg) : register_ept_on_remote(rpmsg_inst, cfg);
    if (rpmsg_ept == NULL)
    {
        return -EINVAL;
    }

    (*token) = rpmsg_ept;

    return 0;
}

static int send(const struct ipc_device *instance, void *token,
                const void *msg, size_t len)
{
    struct backend_data_t *data = instance->data;
    struct ipc_rpmsg_ept *rpmsg_ept;
    int                    ret;

    /* Instance is not ready */
    if (__atomic_load_n((int *)&(data->state), __ATOMIC_SEQ_CST) !=
        STATE_INITED)
    {
        return -EBUSY;
    }

    /* Empty message is not allowed */
    if (len == 0)
    {
        return -EBADMSG;
    }

    rpmsg_ept = (struct ipc_rpmsg_ept *)token;

    ret = rpmsg_send(&rpmsg_ept->ep, msg, len);

    /* No buffers available */
    if (ret == RPMSG_ERR_NO_BUFF)
    {
        return -ENOMEM;
    }

    return ret;
}

static int send_nocopy(const struct ipc_device *instance, void *token,
                const void *msg, size_t len)
{
    struct backend_data_t *data = instance->data;
    struct ipc_rpmsg_ept *rpmsg_ept;

    /* Instance is not ready */
    if (__atomic_load_n((int *)&(data->state), __ATOMIC_SEQ_CST) !=
        STATE_INITED)
    {
        return -EBUSY;
    }

    /* Empty message is not allowed */
    if (len == 0)
    {
        return -EBADMSG;
    }

    rpmsg_ept = (struct ipc_rpmsg_ept *)token;

    return rpmsg_send_nocopy(&rpmsg_ept->ep, msg, len);
}

static int open(const struct ipc_device *instance)
{
    const struct backend_config *conf = &(instance->config);
    struct backend_data_t *data = instance->data;
    struct ipc_rpmsg_instance *rpmsg_inst;
    uint32_t vring_size_in_shm = 0;
    int err;
    int c = STATE_READY;

    if (!atomic_compare_exchange_strong(&data->state, &c, STATE_BUSY))
    {
        return -EALREADY;
    }

    err = vr_shm_configure(&data->vr, conf);
    if (err != 0)
    {
        goto error;
    }

    data->vr.notify_cb = virtio_notify_cb;
    data->vr.priv = (void *)conf;

    err = ipc_static_vrings_init(&data->vr, conf->role);
    if (err != 0)
    {
        goto error;
    }

    err = mbox_init();
    if (err != 0)
    {
        goto error;
    }

    rpmsg_inst = &data->rpmsg_inst;

    rpmsg_inst->bound_cb = bound_cb;
    rpmsg_inst->cb = ept_cb;
    //Calculate the vring's size in share memory, shm pool should be shm_size - the vring area
    vring_size_in_shm = (VRING_COUNT * vring_size(data->vr.vring_size, MEM_ALIGNMENT));

    err = ipc_rpmsg_init(rpmsg_inst, data->role, data->vr.shm_io, &data->vr.vdev,
                         (void *)data->vr.shm_device.regions->virt,
                         data->vr.shm_device.regions->size - vring_size_in_shm, ns_bind_cb);
    if (err != 0)
    {
        goto error;
    }

    atomic_store(&data->state, STATE_INITED);
    return 0;

error:
    /* Back to the ready state */
    atomic_store(&data->state, STATE_READY);
    return err;
}

static int get_tx_buffer_size(const struct ipc_device *instance, void *token)
{
    struct backend_data_t     *data = instance->data;
    struct ipc_rpmsg_instance *rpmsg_inst;
    struct rpmsg_device       *rdev;
    int                        size;

    rpmsg_inst = &data->rpmsg_inst;
    rdev       = rpmsg_virtio_get_rpmsg_device(&rpmsg_inst->rvdev);

    size = rpmsg_virtio_get_buffer_size(rdev);
    if (size < 0)
    {
        return -EIO;
    }

    return size;
}

static int get_tx_buffer(const struct ipc_device *instance, void *token,
                         void **r_data, uint32_t *size, int wait)
{
    struct ipc_rpmsg_ept *rpmsg_ept;
    void                 *payload;
    int                   buf_size;

    rpmsg_ept = (struct ipc_rpmsg_ept *)token;

    if (!r_data || !size)
    {
        return -EINVAL;
    }

    /* OpenAMP only supports a binary wait / no-wait */

    /* The user requested a specific size */
    if (*size)
    {
        buf_size = get_tx_buffer_size(instance, token);
        if (buf_size < 0)
        {
            return -EIO;
        }

        /* Too big to fit */
        if (*size > buf_size)
        {
            *size = buf_size;
            return -ENOMEM;
        }
    }

    payload = rpmsg_get_tx_payload_buffer(&rpmsg_ept->ep, size, wait);
    if (!payload)
    {
        return -EIO;
    }

    (*r_data) = payload;

    return 0;
}

static int hold_rx_buffer(const struct ipc_device *instance, void *token,
                          void *data)
{
    struct ipc_rpmsg_ept *rpmsg_ept;

    rpmsg_ept = (struct ipc_rpmsg_ept *)token;

    rpmsg_hold_rx_buffer(&rpmsg_ept->ep, data);

    return 0;
}

static int release_rx_buffer(const struct ipc_device *instance, void *token,
                             void *data)
{
    struct ipc_rpmsg_ept *rpmsg_ept;

    rpmsg_ept = (struct ipc_rpmsg_ept *)token;

    rpmsg_release_rx_buffer(&rpmsg_ept->ep, data);

    return 0;
}

static int drop_tx_buffer(const struct ipc_device *instance, void *token,
                          const void *data)
{
    /* Not yet supported by OpenAMP */
    return -ENOTSUP;
}

const static struct ipc_service_backend backend_ops = {
    .register_endpoint  = register_ept,
    .send               = send,
    .send_nocopy        = send_nocopy,
    .drop_tx_buffer     = drop_tx_buffer,
    .get_tx_buffer      = get_tx_buffer,
    .get_tx_buffer_size = get_tx_buffer_size,
    .hold_rx_buffer     = hold_rx_buffer,
    .release_rx_buffer  = release_rx_buffer,
};

int ipc_instance_create(struct ipc_device *instance)
{
    struct backend_data_t *data;
    if ((instance == NULL) || (instance->name == NULL))
    {
        return -EINVAL;
    }

    for (uint32_t i = 0; i < NUM_INSTANCES; i++)
    {
        if (rpmsg_instance[i] != NULL)
        {
            // Different instance should use different name
            if ((rpmsg_instance[i]->name != NULL)
                && (strcmp(instance->name, rpmsg_instance[i]->name) == 0))
            {
                // The instance is already registered, return false
                return -EINVAL;
            }
        }
        else
        {
            data = metal_allocate_memory(sizeof(struct backend_data_t));
            if (data == NULL)
            {
                return -ENOSPC;
            }
            // Clear the allocated memory
            memset(data, 0, sizeof(struct backend_data_t));
            // instance->config = config;
            instance->data = data;
            instance->api = &backend_ops;

            rpmsg_instance[i] = instance;
            data->role = instance->config.role;

            metal_mutex_init(&data->rpmsg_inst.mtx);
            atomic_store(&data->state, STATE_READY);
            // Registered the new instance, return 0
            return open(rpmsg_instance[i]);
        }
    }
    // No space for new instance return -ENOMEM
    return -ENOSPC;
}
