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

#ifndef __AUST_NEED_DATA_MESSAGE_H__
#define __AUST_NEED_DATA_MESSAGE_H__

#include "aust_bus.h"
#include "aust_message.h"

/// @brief NeedDataMessage is used to create need data message exclusively.
///
/// This element is mainly for reducing CPU MCPS.
class NeedDataMessage : public AustMessage
{
  private:
    /// Sample rate.
    uint16_t sample_rate_ = 0;

    /// Number of channels.
    uint16_t channels_ = 0;

    /// Need size in bytes.
    uint32_t need_size_ = 0;

    /// Need time in millisecond.
    uint32_t need_time_ = 0;

  public:
    NeedDataMessage() : AustMessage(MSG_NEED_DATA){};
    ~NeedDataMessage(){};

    /// @brief Set sample rate.
    /// @param sample_rate Sample rate.
    void setSampleRate(uint16_t sample_rate);

    /// @brief Set channels.
    /// @param channels Number of channels.
    void setChannels(uint16_t channels);

    /// @brief Set need size in bytes.
    /// @param size Need size.
    void setNeedSize(uint32_t size);

    /// @brief Set need time.
    /// @param time_ms Need time in millisecond.
    void setNeedTime(uint32_t time_ms);

    /// @brief Get sample rate.
    /// @return Sample rate.
    uint16_t getSampleRate(void);

    /// @brief Get channels.
    /// @return Number of channels.
    uint16_t getChannels(void);

    /// @brief Get need size.
    /// @return Need size in bytes.
    uint32_t getNeedSize(void);

    /// @brief Get need time.
    /// @return Need time in millisecond.
    uint32_t getNeedTime(void);
};

#endif /* __AUST_NEED_DATA_MESSAGE_H__ */
