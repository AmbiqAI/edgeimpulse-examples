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

#ifndef __AUST_REFRAME_ELEMENT_H__
#define __AUST_REFRAME_ELEMENT_H__

#include "aust_reframe_buffer.h"
#include "aust_empty_buffer.h"
#include "aust_msg_element.h"
#include "aust_bus.h"

/// @brief ReframeElement is used to output fixed size of data.
///
/// @attention This element should own the specific buffer of ReframeBuffer.
/// User should also configure properties "out-frame-size" and
/// "out-frame-number".
///
/// @ingroup PROPERTIES
/// @{
/// *****************************************************************
/// ## Properties
///
/// - ***Fixed frame size***
/// @code "out-frame-size" @endcode
/// This size is to fix the out frame size, user MUST configure this property.
/// Default value: 0
///
/// - ***Frame number in ring buffer***
/// @code "out-frame-number" @endcode
/// Number of frames in ring buffer, user MUST configure this property.
/// Default value: 0
///
/// *****************************************************************
/// @}
class ReframeElement : public MsgElement
{
  public:
    ReframeElement();
    ~ReframeElement();

  private:
    /// Frame number in ring buffer.
    uint16_t out_frame_number_ = 0;

    /// Fixed size of each frame in ring buffer.
    uint16_t out_frame_size_ = 0;

    /// Submit empty buffer or not. 0: Not submit, 1: submit.
    uint16_t submit_empty_ = 0;

    /// Out buffer in src pad.
    ReframeBuffer *reframe_buffer_;

    /// Empty buffer
    EmptyBuffer empty_buffer_;

    /// @brief Specific state change event handler.
    STATE_RETURN handleIdleToReady(void);
    STATE_RETURN handleReadyToIdle(void);
    STATE_RETURN handleReadyToPause(void);
    STATE_RETURN handlePauseToReady(void);

    /// @brief State change handler.
    /// @param to_state State transition event.
    /// @return Result of handling the state change event.
    STATE_RETURN stateChange(STATE_CHANGE to_state) override;

    /// @brief Process the data of in sink pad.
    /// @param pad Sink pad.
    /// @return AUST_SUCCESS if handled successfully, otherwise AUST_ERROR.
    AUST_ERR process(AustPad *pad) override;
};

#endif /* __AUST_REFRAME_ELEMENT_H__ */
