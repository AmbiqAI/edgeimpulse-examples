/**
 ******************************************************************************
 *
 * @file message_manager.h
 *
 * @brief Audio message manager APIs.
 *
 * Copyright (C) Ambiq Micro
 *
 *
 ******************************************************************************
 */

#ifndef _MESSAGE_MANAGER_H_
#define _MESSAGE_MANAGER_H_

#include <stdint.h>
#include <stdbool.h>

#if defined __cplusplus
extern "C"
{
#endif

    typedef struct config_ep
    {
        /**
         * @brief Read sub-block from config manager.
         *
         * @param id sub-block id to be read.
         * @param data Data pointer to sub-block.
         * @param size Sub-block size.
         *
         * @return 0 if reading successfully, otherwise error code.
         */
        int32_t (*read_sb)(uint32_t id, const uint8_t **data, uint32_t *size);

        /**
         * @brief Update sub-block to config manager.
         *
         * @param id sub-block id to be updated.
         * @param data Data pointer to sub-block.
         * @param size Data size.
         *
         * @return 0 if updating successfully, otherwise error code.
         */
        int32_t (*update_sb)(uint32_t id, const uint8_t *data, uint32_t size);

        /**
         * @brief Save all configuration.
         *
         * @return 0 if saved successfully, otherwise error code.
         */
        int32_t (*save_config)(void);

        void (*reset_config)(void);
    } config_ep_t;

    /**
     * @brief Message type for message callback message_cb_t.
     */
    enum
    {
        MSG_SNIFFER,
        MSG_PROMPT,
        MSG_BYPASS,
    };

    /**
     * @brief Acore message resolved to upper layer callback.
     *
     * @param type Message type.
     * @param msg Message pointer.
     * @param len Message length.
     * @param priv Private pointer.
     *
     * @return 0 if handled successfully, otherwise error code.
     */
    typedef uint16_t (*message_cb_t)(uint8_t type, uint8_t *msg, uint32_t len,
                                     void *priv);

    /**
     * @brief Initialize audio message.
     *
     * @return true if initialized successfully, otherwise false.
     */
    bool audio_message_init(void);

    /**
     * @brief Register config endpoint.
     *
     * @param ep Config endpoint.
     *
     * @return true if registered successfully, otherwise false.
     */
    bool audio_message_register_config_ep(config_ep_t *ep);

    /**
     * @brief Register upper layer message callback for resolved acore message.
     *
     * @param cb Message callback.
     * @param priv Private pointer.
     *
     * @return true if registered successfully, otherwise false.
     */
    bool audio_message_register_message_handler(message_cb_t cb, void *priv);

#if defined __cplusplus
}
#endif

#endif /* _MESSAGE_MANAGER_H_ */
