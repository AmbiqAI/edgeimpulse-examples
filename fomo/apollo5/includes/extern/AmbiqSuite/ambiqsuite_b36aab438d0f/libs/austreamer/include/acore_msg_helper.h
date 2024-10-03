/**
 ******************************************************************************
 *
 * @file acore_msg_helper.h
 *
 * @brief Helper for resolving audio core message.
 *
 * Copyright (C) Ambiq Micro
 *
 *
 ******************************************************************************
 */

#ifndef _ACORE_MSG_HELPER_H_
#define _ACORE_MSG_HELPER_H_

#include <stdbool.h>

#include "acore_msg_struct.h"

#if defined __cplusplus
extern "C"
{
#endif

    /**
     * @brief Validate audio core message which has sub blocks.
     *
     * @param msg Audio core message reference.
     * @return true If message is valid, otherwise false.
     */
    bool acore_msg_validate_sb(acore_msg_t *msg);

    /**
     * @brief Get the next sub block.
     *
     * @param block Current sub block reference.
     * @return Next sub block reference.
     */
    uint8_t *acore_msg_get_next_sb(uint8_t *block);

    /**
     * @brief Fill audio core message header for the req message.
     *
     * @param req Req message reference.
     * @param rcv_dev Receiver device id.
     * @param send_dev Sender device id.
     * @param rcv_obj Receiver object id.
     * @param send_obj Sender object id.
     * @param trans_id Transaction id.
     * @param msg_id Message id.
     * @param msg_len Message length in bytes.
     */
    void acore_msg_fill_req_header(acore_msg_t *req, uint8_t rcv_dev,
                                   uint8_t send_dev, uint8_t rcv_obj,
                                   uint8_t send_obj, uint8_t trans_id,
                                   uint8_t msg_id, uint16_t msg_len);

    /**
     * @brief Fill audio core message header for the resp message.
     *
     * @param resp Resp message reference.
     * @param msg_len Message length in bytes.
     * @param msg_id Message id.
     * @param req Req message reference which is of sender.
     */
    void acore_msg_fill_resp_header(acore_msg_t *resp, uint16_t msg_len,
                                    uint8_t msg_id, acore_msg_t *req);

#if defined __cplusplus
}
#endif

#endif /* _ACORE_MSG_HELPER_H_ */
