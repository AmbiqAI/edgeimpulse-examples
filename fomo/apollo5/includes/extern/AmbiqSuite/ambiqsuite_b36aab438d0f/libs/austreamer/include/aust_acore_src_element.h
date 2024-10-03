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

#ifndef __AUST_ACORE_SRC_ELEMENT_H__
#define __AUST_ACORE_SRC_ELEMENT_H__

#include "acore_msg_struct.h"
#include "aust_msg_element.h"

/// @brief AcoreSrcElement is used to configure audio hal and write the PCM data
/// to hal to play.
///
class AcoreSrcElement : public MsgElement
{
  private:
    /// Sample rate.
    uint16_t sample_rate_ = 16000;

    /// Number of channels.
    uint16_t channels_ = 2;

    /// Transaction tx id for HAL_Audio_Record_Data_Request message.
    uint8_t data_tx_id_ = 0;

    /// Transaction rx id for HAL_Audio_Record_Data_Response message.
    uint8_t data_rx_id_ = 0;

    /// Self object id.
    uint32_t obj_self_id_ = 0;

    /// @brief Handle need data message.
    /// @param msg Message.
    void handleNeedDataMsg(AustMessage *msg);

    /// @brief Send message to AcoreSink to get data.
    /// @param need_size Total size of need data in bytes.
    /// @param sample_rate Sample rate.
    /// @param channels Channels.
    void sendAgentMsgRecordDataReq(uint32_t need_size, uint16_t sample_rate,
                                   uint16_t channels);

    /// @brief Audio core message handler registered on message agent.
    /// @param msg Audio core message.
    /// @param priv User private data, this is always @a this pointer in C++.
    static void acoreMsgHandler(acore_msg_t *msg, void *priv);

    /// @brief Audio core message handler for individual instance.
    /// @param msg Audio core message.
    void acoreMsgHandlerInternal(acore_msg_t *msg);

    /// @brief Handle record data response message.
    /// @param msg Audio core message.
    void handleRecordDataResponse(acore_msg_t *msg);

    /// @brief Handle playback data request message.
    /// @param msg Audio core message.
    void handlePlaybackDataRequest(acore_msg_t *msg);

    /// @brief  Forward message.
    /// @param msg Audio core message.
    void handleBypassMessage(acore_msg_t *msg);

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

    /// @brief AustBus message handler.
    /// @param msg AustMessage reference.
    void msgHandler(AustMessage *msg) override;

  public:
    AcoreSrcElement();
    ~AcoreSrcElement();
};

#endif /* __AUST_ACORE_SRC_ELEMENT_H__ */
