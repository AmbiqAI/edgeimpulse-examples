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

#ifndef __AUST_SPLITTER_ELEMENT_H__
#define __AUST_SPLITTER_ELEMENT_H__

#include <acore_mutex.h>

#include "aust_sem.h"
#include "aust_buffer.h"
#include "aust_msg_element.h"

class SplitterElement;

/// @brief ShadowBuffer is an AustBuffer, only has one AustBuffer instance, but
/// can be used for several AustPad.
class ShadowBuffer : public AustBuffer
{
  private:
    /// Owner of this ShadowBuffer.
    SplitterElement *splitter_;

    /// @brief Require memory. As shadow buffer just reuse the upstream buffer
    /// and should be set by @ref setBuffer, so no need to require memory.
    /// @param size NA.
    /// @return Payload address.
    void *require(int size) override;

  public:
    ShadowBuffer(SplitterElement *splitter);
    ~ShadowBuffer();

    /// @brief Set the payload info of upstream buffer.
    /// @param payload Payload pointer.
    /// @param size Payload size.
    void setBuffer(void *payload, int size);

    /// @brief Release payload memory.
    void release(void) override;

    /// @brief Get the payload address.
    /// @return Payload address.
    void *payload(void) override;

    /// @brief Get the payload size.
    /// @return Payload size.
    int size(void) override;
};

/// @brief SplitterElement is used to handle input data with ShadowBuffer.
///
/// It won't copy the data to each source pad, just pass the input buffer to
/// next element, when all of the next elements process done, it will release
/// input buffer.
class SplitterElement : public MsgElement
{
  private:
    /// Map: <ShadowBuffer ref, Released flag>.
    std::map<ShadowBuffer *, bool> shadows_;

    /// Shadow buffer release semaphore if multi-thread is enabled.

  public:
    SplitterElement();
    ~SplitterElement();

    /// @brief Release buffer of the shadow buffer.
    /// @param shadow ShadowBuffer reference.
    void shadow_release(ShadowBuffer *shadow);

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

#endif /* __AUST_SPLITTER_ELEMENT_H__ */
