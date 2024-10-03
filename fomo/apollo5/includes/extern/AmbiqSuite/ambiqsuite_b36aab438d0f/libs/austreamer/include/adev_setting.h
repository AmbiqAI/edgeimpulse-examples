/**
 ****************************************************************************************
 *
 * @file adev_setting.h
 *
 * @brief Declaration of the Audio Device setting API.
 *
 * Copyright (C) Ambiq Micro
 *
 *
 ****************************************************************************************
 */

#ifndef _ADEV_SETTING_H_
#define _ADEV_SETTING_H_

#include <stdint.h>

#include "adev_comm.h"
#include "adev_drv_i2s.h"

#if defined __cplusplus
extern "C"
{
#endif

    /**
     ****************************************************************************************
     * @brief Set ADEV clock source. All the endpoints use the same clock source
     * @param clksrc Clock source
     * @return ADEV error code
     ****************************************************************************************
     */
    uint32_t adev_set_clksrc(adev_clksrc_t clksrc);

    /**
     ****************************************************************************************
     * @brief Set Endpoint channel number
     * @param epid Endpoint ID
     * @param ch_num Channel number
     * @return  ADEV error code
     ****************************************************************************************
     */
    uint32_t adev_set_channels(adev_ep_id_t epid, adev_channels_t channels);

    /**
     ****************************************************************************************
     * @brief Set Endpoint duration
     * @param epid Endpoint ID
     * @param duration_us Duration in microseconds.
     * Duration * SampleRate * ChannelNumber * SampleWidth = PingPong half sizes
     * @return  ADEV error code
     ****************************************************************************************
     */
    uint32_t adev_set_duration(adev_ep_id_t epid, uint32_t duration_us);

    /**
    ****************************************************************************************
    * @brief Set endpoint sample rate
    * @param epid Endpoint ID
    * @param sample_rate Audio sample rate
    * @return  ADEV error code
    ****************************************************************************************
    */
    uint32_t adev_set_sample_rate(adev_ep_id_t       epid,
                                  adev_sample_rate_t sample_rate);

    /**
     ****************************************************************************************
     * @brief Set I2S endpoint frame width. Frame width >= sample width
     * @param epid Endpoint ID
     * @param frame_width I2S frame width
     * @return  ADEV error code
     ****************************************************************************************
     */
    uint32_t adev_set_i2s_frame_width(adev_ep_id_t           epid,
                                      adev_i2s_frame_width_t frame_width);

    /**
     ****************************************************************************************
     * @brief ADEV set I2S endpoint sample width. Sample width <= frame width
     * @param epid Endpoint ID
     * @param sample_width I2S sample width
     * @return  ADEV error code
     ****************************************************************************************
     */
    uint32_t adev_set_i2s_sample_width(adev_ep_id_t        epid,
                                       adev_sample_width_t sample_width);

    /**
     ****************************************************************************************
     * @brief Set I2S endpoint role
     * @param epid Endpoint ID
     * @param role I2S role. Master or slave.
     * @return  ADEV error code
     ****************************************************************************************
     */
    uint32_t adev_set_i2s_role(adev_ep_id_t epid, adev_i2s_role_t role);

    /**
     ****************************************************************************************
     * @brief Set I2S endpoint alignment
     * @param epid Endpoint ID
     * @param align I2S align mode
     * @return  ADEV error code
     ****************************************************************************************
     */
    uint32_t adev_set_i2s_align(adev_ep_id_t epid, adev_i2s_align_t align);

    /**
     ****************************************************************************************
     * @brief Set I2S endpoint data direction
     * @param epid Endpoint ID
     * @param dir Data direction. In, out or both
     * @return  ADEV error code
     ****************************************************************************************
     */
    uint32_t adev_set_i2s_dir(adev_ep_id_t epid, adev_data_dir_t dir);

#if defined __cplusplus
}
#endif
#endif /* _ADEV_SETTING_H_ */
