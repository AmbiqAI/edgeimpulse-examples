/**
 ******************************************************************************
 *
 * @file acore_dsp_ep_agent.h
 *
 * @brief Declaration Audio core dsp endpoint message agent.
 *
 * Copyright (C) Ambiq Micro
 *
 *
 ******************************************************************************
 */

#ifndef _ACORE_DSP_EP_AGENT_H_
#define _ACORE_DSP_EP_AGENT_H_

#include "acore_msg_struct.h"
#include "acore_common.h"

#if defined __cplusplus
extern "C"
{
#endif

    /**
     * @brief Audio core message handler.
     *
     * @param msg Message handler.
     * @param priv Private data of the object.
     */
    typedef void (*msg_hdl_t)(acore_msg_t *msg, void *priv);

    /**
     * @brief DSP endpoint agent init.
     *
     * @return 0 Successful, otherwise fail.
     */
    int acore_dsp_ep_agent_init(void);

    /**
     * @brief DSP endpoint agent deinit.
     *
     * @return 0 Successful, otherwise fail.
     */
    int acore_dsp_ep_agent_deinit(void);

    /**
     * @brief Register object into DSP endpoint agent.
     *
     * @param obj Object id.
     * @param handler Message handler.
     * @param priv User private data.
     * @return 0 Successful, otherwise fail.
     */
    int acore_dsp_ep_agent_register_obj(int obj, msg_hdl_t handler, void *priv);

    /**
     * @brief Unregister object from DSP endpoint agent.
     *
     * @param obj Object id.
     */
    void acore_dsp_ep_agent_unregister_obj(int obj);

    /**
     * @brief Send message into DSP endpoint agent.
     *
     * @param msg Message reference.
     * @return 0 Successful, otherwise fail.
     */
    int acore_dsp_ep_agent_send(acore_msg_t *msg);

#if defined __cplusplus
}
#endif

#endif /* _ACORE_DSP_EP_AGENT_H_ */
