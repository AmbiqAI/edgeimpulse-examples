//*****************************************************************************
//
// Copyright (c) 2023, Ambiq Micro, Inc.
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
//*****************************************************************************

#ifndef __AUST_QUEUE_H__
#define __AUST_QUEUE_H__

#include <stdint.h>
#include <errno.h>
#include <stdbool.h>

/**
 * @defgroup AUST_QUEUE Queue
 * @brief Queue with node
 * @ingroup AUST_QUEUE
 * @{
 */

typedef struct aust_queue aust_queue_t;

/**
 * @brief Create a aust_queue_t queue to store object reference, each node in
 * queue is NULL at the beginning.
 *
 * @param size Max size of the queue.
 *
 * @return aust_queue_t reference if creating successful, otherwise NULL.
 */
aust_queue_t *aust_queue_create(uint32_t size);

/**
 * @brief Destroy a aust_queue_t queue, it only release @ref aust_queue_t
 * structure, not responsible for the memory release of each node.
 *
 * @param queue Second-order pointer that point to a aust_queue_t reference.
 * After queue destroyed, aust_queue_t reference will be set to NULL inside.
 *
 */
void aust_queue_destroy(aust_queue_t **queue);

/**
 * @brief Get the size of queue.
 *
 * @param queue aust_queue_t reference.
 *
 * @return queue size.
 */
int aust_queue_get_size(aust_queue_t *queue);

/**
 * @brief Get the available size of queue.
 *
 * @param queue aust_queue_t reference.
 *
 * @return Available queue size.
 */
int aust_queue_get_avail(aust_queue_t *queue);

/**
 * @brief Get the used size of queue.
 *
 * @param queue aust_queue_t reference.
 *
 * @return Used queue size.
 */
int aust_queue_get_used(aust_queue_t *queue);

/**
 * @brief Claim an available node from the queue, if finished the operation
 * of the node, MUST invoke @ref aust_queue_enqueue_finish to confirm the
 * node is enqueued.
 *
 * @warning This API is non-thread-safe.
 *
 * @note Once one node is claimed out by @ref aust_queue_enqueue_claim,
 * then this node will be considered as pre-enqueued and being used.
 * Available queue size will decrease by 1, but used queue size won't increase
 * by 1 and still keep the same until @ref aust_queue_enqueue_finish invoked.
 *
 * @param[in] queue aust_queue_t reference.
 * @param[out] node Second-order pointer, the *node points to a node of the
 * queue.
 *
 * @return 0 at least one available node is in queue, otherwise -ENOBUFS,
 * queue full.
 */
int aust_queue_enqueue_claim(aust_queue_t *queue, void **node);

/**
 * @brief Determine whether a node can be enqueued, this MUST be invoked
 * after having claimed a node out by @ref aust_queue_enqueue_claim to
 * confirm the node is enqueued.
 *
 * @warning This API is non-thread-safe.
 *
 * @note This API confirms at most one node that to be enqueued, if multiply
 * invoking @ref aust_queue_enqueue_claim, only the first claimed node will
 * be enqueued, the others will turn back to available nodes into queue.
 *
 * @param queue aust_queue_t reference.
 * @param done true, the claimed node can be enqueued, otherwise won't be
 * enqueued and turns back to available node into queue.
 *
 * @return 0 a node is enqueued, otherwise -EINVAL, not enqueued.
 */
int aust_queue_enqueue_finish(aust_queue_t *queue, bool done);

/**
 * @brief Claim an used node from the queue, if finished the operation
 * of the node, MUST invoke @ref aust_queue_dequeue_finish to confirm the
 * node is dequeued.
 *
 * @warning This API is non-thread-safe.
 *
 * @note Once one node is claimed out by @ref aust_queue_dequeue_claim,
 * then this node will be considered as pre-dequeued and being used. Used
 * queue size will decrease by 1, but available size won't increase by 1 and
 * still keep the same until @ref aust_queue_dequeue_finish invoked.
 *
 * @param[in] queue aust_queue_t reference.
 * @param[out] node Second-order pointer, the *node points to a node of the
 * queue.
 *
 * @return 0 at least one used node is in queue, otherwise -ENOBUFS, queue
 * empty.
 */
int aust_queue_dequeue_claim(aust_queue_t *queue, void **node);

/**
 * @brief Determine whether a node can be dequeued, this MUST be invoked
 * after having claimed a node out by @ref aust_queue_dequeue_claim to
 * confirm the node is dequeued.
 *
 * @warning This API is non-thread-safe.
 *
 * @note This API confirms at most one node that to be dequeued, if multiply
 * invoking @ref aust_queue_dequeue_claim, only the first claimed node will
 * be dequeued, the others will turn back to used nodes into queue.
 *
 * @param queue aust_queue_t reference.
 * @param done true, the claimed node can be dequeued, otherwise won't be
 * dequeued and turns back to used node into queue.
 *
 * @return 0 an node is dequeued, otherwise -EINVAL, not dequeued.
 */
int aust_queue_dequeue_finish(aust_queue_t *queue, bool done);

/**
 * @brief Set the node into queue by index.
 *
 * @param queue aust_queue_t reference.
 * @param index Node index in the queue.
 * @param node Node pointer which points to the context to be enqueued.
 *
 * @return 0 the node is enqueued, otherwise -EINVAL, not enqueued.
 */
int aust_queue_set_node_by_index(aust_queue_t *queue, int index, void *node);

/**
 * @brief Get the node from queue by index.
 *
 * @param queue aust_queue_t reference.
 * @param index Node index in the queue.
 *
 * @return Node address.
 */
void *aust_queue_get_node_by_index(aust_queue_t *queue, int index);

/**
 * @}
 */

#endif /* __AUST_QUEUE_H__ */
