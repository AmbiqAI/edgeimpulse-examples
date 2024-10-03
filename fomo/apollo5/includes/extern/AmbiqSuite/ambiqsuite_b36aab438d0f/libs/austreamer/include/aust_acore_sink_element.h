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

#ifndef __AUST_ACORE_SINK_ELEMENT_H__
#define __AUST_ACORE_SINK_ELEMENT_H__

#include "acore_msg_struct.h"
#include "aust_msg_element.h"

/// @brief AcoreSinkElement is used to configure audio hal and write the PCM
/// data to hal to play.
///
class AcoreSinkElement : public MsgElement
{
  private:
    /// Sample rate.
    uint16_t sample_rate_ = 16000;

    /// Number of channels.
    uint16_t channels_ = 2;

    /// Transaction tx id for HAL_Audio_Playback_Data_Request message.
    uint8_t data_tx_id_ = 0;

    /// Transaction rx id for HAL_Audio_Playback_Data_Response message.
    uint8_t data_rx_id_ = 0;

    /// In pad buffer reference, just for coder playback request.
    AustBuffer *in_buffer_ = NULL;

    /// Self object id.
    uint32_t obj_self_id_ = 0;

    /// Record message pointer to save record data request message.
    void *record_msg_ = NULL;

    /// Flag of receiving record data request message.
    bool record_msg_flag_ = false;

    /**
     * @brief AcoreSink in coder pipeline processes the data of in sink pad.
     * @param buffer Buffer instance of the in sink pad.
     */
    void coderProcess(AustBuffer *buffer);

    /**
     * @brief AcoreSink in dsp pipeline processes the data of in sink pad.
     * @param buffer Buffer instance of the in sink pad.
     */
    void dspProcess(AustBuffer *buffer);

    /**
     * @brief AcoreSink in tool pipeline processes the data of in sink pad.
     * @param buffer Buffer instance of the in sink pad.
     */
    void toolProcess(AustBuffer *buffer);

    /// @brief Audio core message handler registered on message agent.
    /// @param msg Audio core message.
    /// @param priv User private data, this is always @a this pointer in C++.
    static void acoreMsgHandler(acore_msg_t *msg, void *priv);

    /// @brief Audio core message handler for individual instance.
    /// @param msg Audio core message.
    void acoreMsgHandlerInternal(acore_msg_t *msg);

    /// @brief Handle record data request message.
    /// @param msg Audio core message.
    void handleRecordDataRequest(acore_msg_t *msg);

    /// @brief Handle playback data response message.
    /// @param msg Audio core message.
    void handlePlaybackDataResponse(acore_msg_t *msg);

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

    /// @brief Process the data of in sink pad.
    /// @param pad Sink pad.
    /// @return AUST_SUCCESS if handled successfully, otherwise AUST_ERROR.
    AUST_ERR process(AustPad *pad) override;

  public:
    AcoreSinkElement();
    ~AcoreSinkElement();
};

#endif /* __AUST_ACORE_SINK_ELEMENT_H__ */
