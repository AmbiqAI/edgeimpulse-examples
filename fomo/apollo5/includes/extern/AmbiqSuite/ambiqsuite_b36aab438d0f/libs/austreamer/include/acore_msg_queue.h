/**
 ******************************************************************************
 *
 * @file acore_msg_queue.h
 *
 * @brief Declaration Audio core message init and send APIs.
 *
 * Copyright (C) Ambiq Micro
 *
 *
 ******************************************************************************
 */

#ifndef _ACORE_MSG_QUEUE_H_
#define _ACORE_MSG_QUEUE_H_

#include "acore_msg_struct.h"
#include "acore_event.h"
#include "acore_queue.h"

/** Audio event handler of endpoint agent */
typedef void (*evt_hdl_t)(acore_msg_t *msg);

/**
 * @brief Endpoint agent.
 */
typedef struct ep_agent
{
    acore_queue_t        *msg_queue; /**< Message queue */
    enum AUDIO_EVENT_TYPE evt_id;    /**< Audio event type */
    evt_hdl_t             evt_hdl;   /**< Audio event handler */
} ep_agent_t;

/**
 * @brief Initialize message queue of endpoint agent.
 *
 * @param ep_agent Endpoint agent reference.
 *
 * @retval -EINVAL Event id is invalid.
 * @retval 0 successful.
 */
int acore_msg_queue_init(ep_agent_t *ep_agent);

/**
 * @brief Deinitialize message queue of endpoint agent.
 *
 * @param ep_agent Endpoint agent reference.
 */
void acore_msg_queue_deinit(ep_agent_t *ep_agent);

/**
 * @brief Send message to endpoint.
 *
 * @param ep_agent Endpoint agent reference.
 * @param msg Message reference.
 *
 * @retval -ENOSPC Message queue is full.
 * @retval -EINVAL @ref ep_agent or @ref msg is null.
 * @retval 0 successful.
 */
int acore_msg_queue_send(ep_agent_t *ep_agent, acore_msg_t *msg);

#endif /* _ACORE_MSG_QUEUE_H_ */
