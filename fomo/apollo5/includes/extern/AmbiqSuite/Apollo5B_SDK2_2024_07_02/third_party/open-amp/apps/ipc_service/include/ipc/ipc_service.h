/*
 * Copyright (c) 2021 Nordic Semiconductor ASA
 * Copyright (c) 2022, Ambiq Micro, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef IPC_SERVICE_H_
#define IPC_SERVICE_H_

#include <stdio.h>
#include "rpmsg_config.h"

#ifdef __cplusplus
extern "C"
{
#endif

    /* Return value with one bit set. */
    #define BIT_GET(pos) (1UL<<(pos))

    /* Macro to get a structure from one of its structure field. */
    #define CONTAINER_OF(ptr, type, member)    ((type *)( (char *)ptr - offsetof(type,member) ))

    /**
     * @brief Configuration to driver instance
     */
    struct backend_config
    {
        // Role for this instance
        unsigned int role;
        // Share memory start address for this instance
        uintptr_t shm_addr;
        // Share memory size for this instance
        size_t shm_size;
        // 32bit mailbox data for this instance to send buffer(generating kick interrupt to peer device)
        uint32_t mbox_data_send;
        // 32bit mailbox data for this instance to receive buffer(generating kick interrupt from peer device)
        uint32_t mbox_data_rcv;
        /**
         * @brief Called in mailbox interrupt each time a mailbox interrupt generated,
         * used to notify app there's data received, should not block in this cbk.
         *
         * @param priv Private user data(will fill corresponding instance's struct ipc_device).
         *
         * @retval true  user acknowledged the kick, otherwise false
         */
        bool (*user_kick_cbk)(void *priv);
    };
    /**
     * @brief Runtime device structure (in ROM) per driver instance
     */
    struct ipc_device
    {
        /** Name of the device instance */
        const char *name;
        /** Device instance config information */
        struct backend_config config;
        /** Address of the API structure exposed by the device instance */
        const void *api;
        /** Address of the common device state */
        // struct device_state * const state;
        /** Address of the device instance private data */
        void *data;
    };
    /**
     * @brief IPC Service API
     * @defgroup ipc_service_api IPC service APIs
     * @{
     *
     * Some terminology:
     *
     * - INSTANCE: an instance is the external representation of a physical
     *             communication channel between two domains / CPUs.
     *
     *             The actual implementation and internal representation of the
     *             instance is peculiar to each backend. For example for
     *             OpenAMP-based backends, an instance is usually represented by a
     *             shared memory region and a couple of IPM devices for RX/TX
     *             signalling.
     *
     *             It's important to note that an instance per se is not used to
     *             send data between domains / CPUs. To send and receive data the
     *             user have to create (register) an endpoint in the instance
     *             connecting the two domains of interest.
     *
     *             It's possible to have zero or multiple endpoints in one single
     *             instance, each one used to exchange data, possibly with different
     *             priorities.
     *
     *             The creation of the instances is left to the backend (usually at
     *             init time), while the registration of the endpoints is left to
     *             the user (usually at run time).
     *
     * - ENDPOINT: an endpoint is the entity the user must use to send / receive
     *             data between two domains (connected by the instance). An
     *             endpoint is always associated to an instance.
     *
     * - BACKEND: the backend must take care of at least two different things:
     *
     *            1) creating the instances at init time
     *            2) creating / registering the endpoints onto an instance at run
     *               time when requested by the user
     *
     *            The API doesn't mandate a way for the backend to create the
     *            instances but itis strongly recommended to use the DT to retrieve
     *            the configuration parameters for the instance.
     * Common API usage from the application prospective:
     *
     *   HOST                                         REMOTE
     *   -----------------------------------------------------------------------------
     *   # Open the (same) instance on host and remote
     *   ipc_service_open()                           ipc_service_open()
     *
     *   # Register the endpoints
     *   ipc_service_register_endpoint() ipc_service_register_endpoint()
     *   .bound()                                     .bound()
     *
     *   # After the .bound() callbacks are received the communication channel
     *   # is ready to be used
     *
     *   # Start sending and receiving data
     *   ipc_service_send()
     *                                                .receive()
     *                                                ipc_service_send()
     *   .receive()
     *
     *
     * Common API usage from the application prospective when using NOCOPY
     * feature:
     *
     *   HOST                                         REMOTE
     *   -----------------------------------------------------------------------------
     *   ipc_service_open()                           ipc_service_open()
     *
     *   ipc_service_register_endpoint() ipc_service_register_endpoint()
     *   .bound()                                     .bound()
     *
     *   # Get a pointer to an available TX buffer
     *   ipc_service_get_tx_buffer()
     *
     *   # Fill the buffer with data
     *
     *   # Send out the buffer
     *   ipc_service_send_nocopy()
     *                                                .receive()
     *
     *                                                # Get hold of the received
     *                                                # RX buffer
     *                                                # in the .receive callback
     *                                                #
     * ipc_service_hold_rx_buffer()
     *
     *                                                # Copy the data out of the
     *                                                # buffer at # user
     * convenience
     *
     *                                                # Release the buffer when
     *                                                # done
     * ipc_service_release_rx_buffer()
     *
     *    # Get another TX buffer
     *    ipc_service_get_tx_buffer()
     *
     *    # We can also drop it if needed
     *    ipc_service_drop_tx_buffer()
     *
     */

    /** @brief Event callback structure.
     *
     *  It is registered during endpoint registration.
     *  This structure is part of the endpoint configuration.
     */
    struct ipc_service_cb
    {
        /** @brief Bind was successful.
         *
         *  This callback is called when the endpoint binding is successful.
         *
         *  @param[in] priv Private user data.
         */
        void (*bound)(void *priv);

        /** @brief New packet arrived.
         *
         *  This callback is called when new data is received.
         *
         *  @note When @ref ipc_service_hold_rx_buffer is not used, the data
         *	  buffer is to be considered released and available again only
         *	  when this callback returns.
         *
         *  @param[in] data Pointer to data buffer.
         *  @param[in] len Length of @a data.
         *  @param[in] priv Private user data.
         */
        void (*received)(const void *data, size_t len, void *priv);

        /** @brief An error occurred.
         *
         *  @param[in] message Error message.
         *  @param[in] priv Private user data.
         */
        void (*error)(const char *message, void *priv);
    };

    /** @brief Endpoint instance.
     *
     *  Token is not important for user of the API. It is implemented in a
     *  specific backend.
     */
    struct ipc_ept
    {

        /** Instance this endpoint belongs to. */
        const struct ipc_device *instance;

        /** Backend-specific token used to identify an endpoint in an instance. */
        void *token;
    };

    /** @brief Endpoint configuration structure. */
    struct ipc_ept_cfg
    {

        /** Name of the endpoint. */
        const char *name;

        /** Endpoint priority. If the backend supports priorities. */
        int prio;

        /** Event callback structure. */
        struct ipc_service_cb cb;

        /** Private user data. */
        void *priv;
    };

    /** @brief Initiate an instance
     *
     *  Function to be used to initiate an instance and then open it.
     *
     *  @param instance Instance pointer to register, max number of instances can register is defined in NUM_INSTANCES.
     *
     *  @retval -EINVAL when instance configuration is invalid(duplicate name or invalid instance pointer).
     *  @retval -ENOSPC when no space avaliable for registration or failed to allocate memory for instance's data field.
     *  @retval -EALREADY when the instance is already opened(or created).
     *
     *  @retval 0 on success.
     */
    int ipc_instance_create(struct ipc_device *instance);

    /** @brief Register IPC endpoint onto an instance.
     *
     *  Registers IPC endpoint onto an instance to enable communication with a
     *  remote device.
     *
     *  The same function registers endpoints for both host and remote devices.
     *
     *  @param[in] instance Instance to register the endpoint onto.
     *  @param[in] ept Endpoint object.
     *  @param[in] cfg Endpoint configuration.
     *
     *  @note Keep the variable pointed by @p cfg alive when endpoint is in use.
     *
     *  @retval -EIO when no backend is registered.
     *  @retval -EINVAL when instance, endpoint or configuration is invalid.
     *  @retval -EBUSY when the instance is busy.
     *
     *  @retval 0 on success.
     *  @retval other errno codes depending on the implementation of the backend.
     */
    int ipc_service_register_endpoint(const struct ipc_device *instance,
                                      struct ipc_ept *ept,
                                      const struct ipc_ept_cfg *cfg);

    /** @brief Send data using given IPC endpoint.
     *
     *  @param[in] ept Registered endpoint by @ref ipc_service_register_endpoint.
     *  @param[in] data Pointer to the buffer to send.
     *  @param[in] len Number of bytes to send.
     *
     *  @retval -EIO when no backend is registered or send hook is missing from
     *	    backend.
     *  @retval -EINVAL when instance or endpoint is invalid, or the data buffer is allocated
     *          by ipc_service_get_tx_buffer(), if so user should use ipc_service_send_nocopy() API;
     *  @retval -EBADMSG when the message is invalid.
     *  @retval -EBUSY when the instance is busy.
     *
     *  @retval 0 on success.
     *  @retval other errno codes depending on the implementation of the backend.
     */
    int ipc_service_send(struct ipc_ept *ept, const void *data, size_t len);

    /** @brief Get the TX buffer size
     *
     *  Get the maximal size of a buffer which can be obtained by @ref
     *  ipc_service_get_tx_buffer
     *
     *  @param[in] ept Registered endpoint by @ref
     *  ipc_service_register_endpoint.
     *
     *  @retval -EIO when no backend is registered or send hook is missing from
     *		 backend.
     *  @retval -EINVAL when instance or endpoint is invalid.
     *  @retval -ENOTSUP when the operation is not supported by backend.
     *
     *  @retval size TX buffer size on success.
     *  @retval other errno codes depending on the implementation of the
     *  backend.
     */
    int ipc_service_get_tx_buffer_size(struct ipc_ept *ept);

    /** @brief Get an empty TX buffer to be sent using @ref
     *  ipc_service_send_nocopy
     *
     *  This function can be called to get an empty TX buffer so that the
     *  application can directly put its data into the sending buffer without
     *  copy from an application buffer.
     *
     *  It is the application responsibility to correctly fill the allocated TX
     *  buffer with data and passing correct parameters to @ref
     *  ipc_service_send_nocopy function to perform data no-copy-send mechanism.
     *
     *  The size parameter can be used to request a buffer with a certain size:
     *  - if the size can be accommodated the function returns no errors and the
     *    buffer is allocated
     *  - if the requested size is too big, the function returns -ENOMEM and the
     *    the buffer is not allocated.
     *  - if the requested size is '0' the buffer is allocated with the maximum
     *    allowed size.
     *
     *  In all the cases on return the size parameter contains the maximum size
     *  for the returned buffer.
     *
     *  When the function returns no errors, the buffer is intended as allocated
     *  and it is released under two conditions: (1) when sending the buffer
     *  using
     *  @ref ipc_service_send_nocopy (and in this case the buffer is
     *  automatically released by the backend), (2) when using @ref
     *  ipc_service_drop_tx_buffer on a buffer not sent.
     *
     *  @param[in] ept Registered endpoint by @ref
     *  ipc_service_register_endpoint.
     *  @param[out] data Pointer to the empty TX buffer.
     *  @param[in,out] size Pointer to store the requested TX buffer size. If
     *  the function returns -ENOMEM, this parameter returns the maximum allowed
     *  size.
     *  @param[in] wait Timeout waiting for an available TX buffer.
     *
     *  @retval -EIO when no backend is registered or send hook is missing from
     *		 backend.
     *  @retval -EINVAL when instance or endpoint is invalid.
     *  @retval -ENOTSUP when the operation or the timeout is not supported by
     *  backend.
     *  @retval -ENOBUFS when there are no TX buffers available.
     *  @retval -EALREADY when a buffer was already claimed and not yet
     *  released.
     *  @retval -ENOMEM when the requested size is too big (and the size
     *  parameter contains the maximum allowed size).
     *
     *  @retval 0 on success.
     *  @retval other errno codes depending on the implementation of the
     *  backend.
     */
    int ipc_service_get_tx_buffer(struct ipc_ept *ept, void **data,
                                  uint32_t *size, int wait);

    /** @brief Drop and release a TX buffer
     *
     *  Drop and release a TX buffer. It is possible to drop only TX buffers
     *  obtained by using @ref ipc_service_get_tx_buffer.
     *
     *  @param[in] ept Registered endpoint by @ref
     *  ipc_service_register_endpoint.
     *  @param[in] data Pointer to the TX buffer.
     *
     *  @retval -EIO when no backend is registered or send hook is missing from
     *		 backend.
     *  @retval -EINVAL when instance or endpoint is invalid.
     *  @retval -ENOTSUP when this is not supported by backend.
     *  @retval -EALREADY when the buffer was already dropped.
     *  @retval -ENXIO when the buffer was not obtained using @ref
     *		   ipc_service_get_tx_buffer
     *
     *  @retval 0 on success.
     *  @retval other errno codes depending on the implementation of the
     *  backend.
     */
    int ipc_service_drop_tx_buffer(struct ipc_ept *ept, const void *data);

    /** @brief Send data in a TX buffer reserved by @ref
     *  ipc_service_get_tx_buffer using the given IPC endpoint.
     *
     *  This is equivalent to @ref ipc_service_send but in this case the TX
     *  buffer has been obtained by using @ref ipc_service_get_tx_buffer.
     *
     *  The application has to take the responsibility for getting the TX buffer
     *  using @ref ipc_service_get_tx_buffer and filling the TX buffer with the
     *  data.
     *
     *  After the @ref ipc_service_send_nocopy function is issued the TX buffer
     *  is no more owned by the sending task and must not be touched anymore
     *  unless the function fails and returns an error.
     *
     *  If this function returns an error, @ref ipc_service_drop_tx_buffer can
     *  be used to drop the TX buffer.
     *
     *  @param[in] ept Registered endpoint by @ref
     *  ipc_service_register_endpoint.
     *  @param[in] data Pointer to the buffer to send obtained by @ref
     *		    ipc_service_get_tx_buffer.
     *  @param[in] len Number of bytes to send.
     *
     *  @retval -EIO when no backend is registered or send hook is missing from
     *		 backend.
     *  @retval -EINVAL when instance or endpoint is invalid, , or the data
     *  buffer is not allocated by ipc_service_get_tx_buffer(), if so user
     *  should use ipc_service_send() API;
     *  @retval -EBADMSG when the data is invalid (i.e. invalid data format,
     *		     invalid length, ...)
     *  @retval -EBUSY when the instance is busy.
     *
     *  @retval bytes number of bytes sent.
     *  @retval other errno codes depending on the implementation of the
     *  backend.
     */
    int ipc_service_send_nocopy(struct ipc_ept *ept, const void *data,
                                size_t len);

    /** @brief Holds the RX buffer for usage outside the receive callback.
     *
     *  Calling this function prevents the receive buffer from being released
     *  back to the pool of shmem buffers. This function can be called in the
     *  receive callback when the user does not want to copy the message out in
     *  the callback itself.
     *
     *  After the message is processed, the application must release the buffer
     *  using the @ref ipc_service_release_rx_buffer function.
     *
     *  @param[in] ept Registered endpoint by @ref
     *  ipc_service_register_endpoint.
     *  @param[in] data Pointer to the RX buffer to release.
     *
     *  @retval -EIO when no backend is registered or release hook is missing
     *  from backend.
     *  @retval -EINVAL when instance or endpoint is invalid.
     *  @retval -EALREADY when the buffer data has been hold already.
     *  @retval -ENOTSUP when this is not supported by backend.
     *
     *  @retval 0 on success.
     *  @retval other errno codes depending on the implementation of the
     *  backend.
     */
    int ipc_service_hold_rx_buffer(struct ipc_ept *ept, void *data);

    /** @brief Release the RX buffer for future reuse.
     *
     *  When supported by the backend, this function can be called after the
     *  received message has been processed and the buffer can be marked as
     *  reusable again.
     *
     *  It is possible to release only RX buffers on which @ref
     *  ipc_service_hold_rx_buffer was previously used.
     *
     *  @param[in] ept Registered endpoint by @ref
     *  ipc_service_register_endpoint.
     *  @param[in] data Pointer to the RX buffer to release.
     *
     *  @retval -EIO when no backend is registered or release hook is missing
     *  from backend.
     *  @retval -EINVAL when instance or endpoint is invalid.
     *  @retval -EALREADY when the buffer data has been already released.
     *  @retval -ENOTSUP when this is not supported by backend.
     *  @retval -ENXIO when the buffer was not hold before using @ref
     *		   ipc_service_hold_rx_buffer
     *
     *  @retval 0 on success.
     *  @retval other errno codes depending on the implementation of the
     *  backend.
     */
    int ipc_service_release_rx_buffer(struct ipc_ept *ept, void *data);
    /** @brief Check the given IPC endpoint is bounded or not.
     *
     *  @param ept Registered endpoint by @ref ipc_service_register_endpoint.
     *
     *  @retval -true if bounded.
     */
    bool ipc_service_is_endpoint_bounded(struct ipc_ept *ept);

    /**
     * @brief send data through mailbox to notify the peer device to receive data. This function
     *        is defined as WEAK so the application can do the specific implemtation. Or the
     *        application will use the default function implemented in ipc_rpmsg_static_vrings.c.
     *
     * @param data   32bit data to send through mailbox
     *
     * @retval true  send successfully
     * @retval false send failed
     */
    bool mbox_send(uint32_t data) __attribute((weak));

    /** @brief Try to receive data on specific instance, if new data is avaliable, will call the corresponding
     *         endpoint's received cbk, refer to ipc_service_cb, this function should be called in application
     *         after getting notice by user_kick_cbk().
     *
     *  @param instance Pointer to the instance to receive data
     */
    void ipc_receive(struct ipc_device *instance);

    /** @brief Called in task after mailbox interrupt triggered, used to read mailbox FIFO and trigger
     *  corresponding IPC instance to receive IPC buffers, and clear the mailbox interrupt source.
     *
     *  @param buf_received_bitmap bit map to set in mailbox_event_handle to indicate which instance
     *  should receive buffer
     * 
     *  @retval 0 successfully, otherwise failed to handle mailbox event
     */
    uint32_t mailbox_event_handle(uint32_t* buf_received_bitmap);
    /**
     * @}
     */

#ifdef __cplusplus
}
#endif

#endif /* IPC_SERVICE_H_ */
