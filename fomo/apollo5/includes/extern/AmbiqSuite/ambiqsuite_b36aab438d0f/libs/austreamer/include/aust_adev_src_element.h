//*****************************************************************************
//
// Copyright (c) 2022, Ambiq Micro, Inc.
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

#ifndef __AUST_ADEV_SRC_ELEMENT_H__
#define __AUST_ADEV_SRC_ELEMENT_H__

#include "adev.h"

#include "aust_msg_element.h"

/// @brief AdevSrcElement is used to configure audio device and read data from
/// it.
///
/// User can configure the source as line-in or microphone.
///
/// @ingroup PROPERTIES
/// @{
/// *****************************************************************
/// ## Properties
///
/// - ***Sample rate***
/// @code "sample-rate" @endcode
/// Supported sample rate: 8000 16000 48000 or 96000
///
/// - ***Channels***
/// @code "channels" @endcode
/// Supported channels: 1 or 2
///
/// - ***Role***
/// @code "role" @endcode
/// Supported role: "master" or "slave".
/// Default value: "master"
///
/// - ***I2S format***
/// @code "i2s-format" @endcode
/// Supported I2S format: "standard" or "left_justified".
/// Default value: "standard"
///
/// - ***Endpoint***
/// @code "endpoint" @endcode
/// Supported endpoint: "i2s-0" "i2s-1" "pdm-0" "adc-0" "adc-1"
/// "adc-1". Default value: "pdm-0"
///
/// - ***Clock source***
/// @code "clksrc" @endcode
/// Supported clksrc: "xtal" "mclk".
/// Default value: "xtal"
///
/// *****************************************************************
/// @}
class AdevSrcElement : public MsgElement
{
  private:
    /// Sample rate.
    adev_sample_rate_t sample_rate_ = ADEV_SAMPLE_RATE_16000_HZ;

    /// Number of pipeline channels.
    uint16_t channels_ = 0;

    /// Data width.
    adev_sample_width_t sample_bits_ = ADEV_SAMPLE_WIDTH_16_BITS;

    /// Frame width.
    adev_i2s_frame_width_t frame_bits_ = ADEV_FRAME_WIDTH_16_BITS;

    /// I2S role.
    adev_i2s_role_t role_ = ADEV_I2S_MASTER;

    /// I2S format.
    adev_i2s_align_t i2s_format_ = ADEV_I2S_STANDARD;

    /// Endpoint.
    adev_ep_id_t ep_id_ = ADEV_EP_PDM_0;

    /// Clock source.
    adev_clksrc_t clksrc_ = ADEV_CLKSRC_XTAL;

    /// Frame size that each time pass to next element. Unit is Byte.
    uint32_t frame_size_ = 0;

    /// DMA interval in microsecond.
    uint32_t interval_us_ = 0;

    int32_t *buff_24bit_ = NULL;

    /// Pipeline sample rate.
    adev_sample_rate_t sample_rate_pipe_ = ADEV_SAMPLE_RATE_16000_HZ;

    /// Pipeline number of pipeline channels.
    uint16_t channels_pipe_ = 0;

    // Pipeline frame size that each time pass to next element. Unit is Byte.
    uint32_t frame_size_pipe_ = 0;

    /// Self data speed to pipeline data speed
    float ratio_ = 1.0;

    /// Flag of wait for data buffer ready to access
    bool wait_for_ready_event_ = false;

    /// @brief Specific state change event handler.
    STATE_RETURN handleIdleToReady(void);
    STATE_RETURN handleReadyToIdle(void);
    STATE_RETURN handleReadyToPause(void);
    STATE_RETURN handlePauseToReady(void);
    STATE_RETURN handlePauseToPlay(void);
    STATE_RETURN handlePlayToPause(void);

    /// @brief State change handler.
    /// @param to_state State transition event.
    /// @return Result of handling the state change event.
    STATE_RETURN stateChange(STATE_CHANGE to_state) override;

    /// @brief Fetch data and submit
    /// @return AUST_SUCCESS if handled successfully, otherwise AUST_ERROR.
    AUST_ERR fetch_data(void);

    /// @brief AustBus message handler.
    /// @param msg AustMessage reference.
    void msgHandler(AustMessage *msg) override;

  public:
    AdevSrcElement();
    ~AdevSrcElement();

    AUST_ERR dataAccessReady(void);
};

#endif /* __AUST_ADEV_SRC_ELEMENT_H__ */
