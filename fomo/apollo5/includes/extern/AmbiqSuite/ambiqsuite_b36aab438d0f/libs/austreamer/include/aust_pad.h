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

#ifndef __AUST_PAD_H__
#define __AUST_PAD_H__

#include "aust_buffer.h"

class AustElement;

/// @brief Pad type.
enum
{
    SINK_PAD = 0,
    SRC_PAD  = 1,
};

/// @brief AustPad is subject to AustElement, which is used to link element and
/// streaming data from source pad of previous element to sink pad of this
/// Element.
///
/// It may have different output capabilities, so before streaming, capability
/// negotiation will be needed to make sure sink pad can handle source pad's
/// data.
class AustPad : public AustObject
{
  private:
    /// Owner of this pad.
    AustElement *owner_;

    /// Peer pad that connected to this pad.
    AustPad *peerPad_;

    /// AustBuffer instance stores AustElement buffer reference. Source pad sets
    /// AustElement buffer_ into it, then pass to peer sink pad, peer pad will
    /// set it to its own buffer_ for peer AustElement to use.
    AustBuffer *buffer_;

    /// @brief Sink pad call this to handle the buffer set by peer source pad.
    void receive(void);

  public:
    /// Pad type.
    uint8_t type = 0;

    /// Pad index.
    uint8_t index = 0;

    /// Sniffer enable flag.
    uint8_t sniffer = 0;

  public:
    /// @brief Constructor.
    /// @param owner Owner of this pad.
    AustPad(AustElement *owner);
    ~AustPad();

    /// @brief Source pad call this to submits its buffer to peer pad, in this
    /// function the peer pad will call its own receive to handle the buffer.
    void submit(void);

    /// @brief Set peer pad which is linked.
    /// @param pad Peer pad.
    void setPeerPad(AustPad *pad);

    /// @brief Get peer pad which is linked.
    /// @return Peer pad reference.
    AustPad *getPeerPad(void);

    /// @brief Set buffer reference into this pad.
    /// @param buffer AustBuffer reference.
    void setBuffer(AustBuffer *buffer);

    /// @brief Get buffer reference of this pad.
    /// @return AustBuffer reference.
    AustBuffer *getBuffer(void);
};

#endif /* __AUST_PAD_H__ */
