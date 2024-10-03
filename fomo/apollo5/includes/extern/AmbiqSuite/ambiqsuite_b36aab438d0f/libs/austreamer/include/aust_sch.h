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

#ifndef __AUST_SCH_H__
#define __AUST_SCH_H__

#include "aust_msg_element.h"

/// @brief AustSch is the pipeline scheduler.
///

class AustSch : public MsgElement
{
  private:
    /// Ignore some frame number at the beginning for system becoming normal
    /// state Too many SWO print at the beginning will delay the scheduling.
    const static uint32_t FRAME_TO_STABLE = 1;

    /// Sample rate.
    uint32_t sample_rate_ = 0;

    /// Number of channels.
    uint16_t channels_ = 0;

    /// Scheduler interval in microsecond.
    uint32_t interval_us_ = 0;

    /// Frame size.
    uint32_t frame_size_ = 0;

    /// Ignore some frame number at the beginning for system becoming normal
    /// state Too many SWO print at the beginning will delay the scheduling.
    uint32_t frame_to_stable_ = FRAME_TO_STABLE;

    /// @brief Convert interval time to frame size.
    /// @param time_us Interval time in microsecond.
    /// @return Frame size.
    uint32_t usToSize(uint32_t time_us);

    /// @brief Convert frame size to interval time.
    /// @param frame_size Frame size.
    /// @return Interval time in microsecond.
    uint32_t sizeToUs(uint32_t frame_size);

    /// @brief Specific state change event handler.
    STATE_RETURN handleIdleToReady(void);
    STATE_RETURN handleReadyToIdle(void);
    STATE_RETURN handleReadyToPause(void);
    STATE_RETURN handlePauseToReady(void);

    /// @brief State change handler.
    /// @param to_state State transition event.
    /// @return Result of handling the state change event.
    STATE_RETURN stateChange(STATE_CHANGE to_state) override;

  public:
    AustSch();
    ~AustSch();

    /// @brief Need data event handler
    void needDataEventHandler(void);
};

#endif /* __AUST_SCH_H__ */
