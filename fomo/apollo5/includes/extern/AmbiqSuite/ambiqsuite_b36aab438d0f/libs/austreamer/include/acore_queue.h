/**
 ******************************************************************************
 *
 * @file acore_queue.h
 *
 * @brief Audio core basic queue.
 *
 * Copyright (C) Ambiq Micro
 *
 *
 ******************************************************************************
 */

#ifndef __ACORE_QUEUE_H__
#define __ACORE_QUEUE_H__
#if defined __cplusplus
extern "C"
{
#endif

#include <stdint.h>
#include <stdbool.h>
#include <errno.h>

    /**
     * @defgroup ACORE_QUEUE Queue
     * @brief Queue with node
     * @ingroup ACORE_QUEUE
     * @{
     */

    typedef struct acore_queue acore_queue_t;

    /**
     * @brief Create a acore_queue_t queue to store object reference, each node
     * in queue is NULL at the beginning.
     *
     * @param queue_len Length of the queue.
     * @param item_size Size of each item.
     *
     * @return acore_queue_t reference if creating successful, otherwise NULL.
     */
    acore_queue_t *acore_queue_create(uint32_t queue_len, uint32_t item_size);

    /**
     * @brief Destroy a acore_queue_t queue, it only release @ref acore_queue_t
     * structure, not responsible for the memory release of each node.
     *
     * @param queue Second-order pointer that point to a acore_queue_t
     * reference. After queue destroyed, acore_queue_t reference will be set to
     * NULL inside.
     *
     */
    void acore_queue_destroy(acore_queue_t **queue);

    /**
     * @brief Get the size of queue.
     *
     * @param queue acore_queue_t reference.
     *
     * @return queue size.
     */
    int acore_queue_get_size(acore_queue_t *queue);

    /**
     * @brief Get the available size of queue.
     *
     * @param queue acore_queue_t reference.
     *
     * @return Available queue size.
     */
    int acore_queue_get_avail(acore_queue_t *queue);

    /**
     * @brief Get the used size of queue.
     *
     * @param queue acore_queue_t reference.
     *
     * @return Used queue size.
     */
    int acore_queue_get_used(acore_queue_t *queue);

    /**
     * @brief Claim an available node from the queue, if finished the operation
     * of the node, MUST invoke @ref acore_queue_enqueue_finish to confirm the
     * node is enqueued.
     *
     * @warning This API is non-thread-safe.
     *
     * @note Once one node is claimed out by @ref acore_queue_enqueue_claim,
     * then this node will be considered as pre-enqueued and being used.
     * Available queue size will decrease by 1, but used queue size won't
     * increase by 1 and still keep the same until @ref
     * acore_queue_enqueue_finish invoked.
     *
     * @param[in] queue acore_queue_t reference.
     * @param[out] node Second-order pointer, the *node points to a node of the
     * queue.
     *
     * @return 0 at least one available node is in queue, otherwise -ENOBUFS,
     * queue full.
     */
    int acore_queue_enqueue_claim(acore_queue_t *queue, void **node);

    /**
     * @brief Determine whether a node can be enqueued, this MUST be invoked
     * after having claimed a node out by @ref acore_queue_enqueue_claim to
     * confirm the node is enqueued.
     *
     * @warning This API is non-thread-safe.
     *
     * @note This API confirms at most one node that to be enqueued, if multiply
     * invoking @ref acore_queue_enqueue_claim, only the first claimed node will
     * be enqueued, the others will turn back to available nodes into queue.
     *
     * @param queue acore_queue_t reference.
     * @param done true, the claimed node can be enqueued, otherwise won't be
     * enqueued and turns back to available node into queue.
     *
     * @return 0 a node is enqueued, otherwise -EINVAL, not enqueued.
     */
    int acore_queue_enqueue_finish(acore_queue_t *queue, bool done);

    /**
     * @brief Claim an used node from the queue, if finished the operation
     * of the node, MUST invoke @ref acore_queue_dequeue_finish to confirm the
     * node is dequeued.
     *
     * @warning This API is non-thread-safe.
     *
     * @note Once one node is claimed out by @ref acore_queue_dequeue_claim,
     * then this node will be considered as pre-dequeued and being used. Used
     * queue size will decrease by 1, but available size won't increase by 1 and
     * still keep the same until @ref acore_queue_dequeue_finish invoked.
     *
     * @param[in] queue acore_queue_t reference.
     * @param[out] node Second-order pointer, the *node points to a node of the
     * queue.
     *
     * @return 0 at least one used node is in queue, otherwise -ENOBUFS, queue
     * empty.
     */
    int acore_queue_dequeue_claim(acore_queue_t *queue, void **node);

    /**
     * @brief Determine whether a node can be dequeued, this MUST be invoked
     * after having claimed a node out by @ref acore_queue_dequeue_claim to
     * confirm the node is dequeued.
     *
     * @warning This API is non-thread-safe.
     *
     * @note This API confirms at most one node that to be dequeued, if multiply
     * invoking @ref acore_queue_dequeue_claim, only the first claimed node will
     * be dequeued, the others will turn back to used nodes into queue.
     *
     * @param queue acore_queue_t reference.
     * @param done true, the claimed node can be dequeued, otherwise won't be
     * dequeued and turns back to used node into queue.
     *
     * @return 0 an node is dequeued, otherwise -EINVAL, not dequeued.
     */
    int acore_queue_dequeue_finish(acore_queue_t *queue, bool done);

    /**
     * @brief Set the node into queue by index.
     *
     * @param queue acore_queue_t reference.
     * @param index Node index in the queue.
     * @param node Node pointer which points to the context to be enqueued.
     *
     * @return 0 the node is enqueued, otherwise -EINVAL, not enqueued.
     */
    int acore_queue_set_node_by_index(acore_queue_t *queue, int index,
                                      void *node);

    /**
     * @brief Get the node from queue by index.
     *
     * @param queue acore_queue_t reference.
     * @param index Node index in the queue.
     *
     * @return Node address.
     */
    void *acore_queue_get_node_by_index(acore_queue_t *queue, int index);

/**
 * @}
 */
#if defined __cplusplus
}
#endif
#endif /* __ACORE_QUEUE_H__ */
