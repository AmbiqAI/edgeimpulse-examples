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

#ifndef __AUST_WAVE_SRC_ELEMENT_H__
#define __AUST_WAVE_SRC_ELEMENT_H__

#include "wavegen.h"

#include "aust_msg_element.h"
#include "aust_message.h"

/// @brief WaveSrcElement is used to generate sine wave data.
///
/// @ingroup PROPERTIES
/// @{
/// *****************************************************************
/// ## Properties
///
/// - ***Sine wave frequency***
/// @code "sine-wave-frequency" @endcode
/// Sine wave frequency should divide into sample rate.
/// Default value: 400
///
/// - ***Sine wave gain***
/// @code "gain" @endcode
/// Sine wave gain coefficient.
/// Default value: 0.5
///
/// *****************************************************************
/// @}
class WaveSrcElement : public MsgElement
{
  protected:
    /// Sample rate.
    unsigned int sample_rate_ = 0;

    /// Number of channels.
    uint16_t channels_ = 1;

    /// Sine wave frequency.
    unsigned int sine_wave_frequency_ = 400;

    /// Sine wave gain, range from 0.0 to 1.0.
    float gain_ = 0.5;

    /// Frame size that each time pass to next element.
    unsigned int frame_size_ = 256;

    /// Need data size.
    volatile uint32_t need_data_size = 0;

    /// Wave generator instance.
    wavegen_inst wavegen_inst_ = NULL;

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

    /// @brief AustBus message handler.
    /// @param msg AustMessage reference.
    void msgHandler(AustMessage *msg) override;

  public:
    WaveSrcElement();
    ~WaveSrcElement();
};

#endif /* __AUST_WAVE_SRC_ELEMENT_H__ */
