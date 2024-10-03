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

#ifndef __AUST_DRC_ELEMENT_H__
#define __AUST_DRC_ELEMENT_H__

#include "aust_algo_element.h"

/// @brief DrcElement is used for Dynamic Range Control process.
///
/// @ingroup PROPERTIES
/// @{
/// *****************************************************************
/// ## Properties
///
/// - ***Link direction***
/// @code "link" @endcode
/// Supported link: "uplink" or "downlink"
///
/// *****************************************************************
/// @}

/// The direction of drc.
enum
{
    DRC_UPLINK = 0,
    DRC_DOWNLINK
};

class DrcElement : public AlgoElement
{
  private:
    /// Bitstream sample rate. Allowed sample rates are: 8000, 16000, 24000,
    /// 32000, 44100, 48000, 96000.
    uint32_t sample_rate_ = 0;

    /// Bitstream number of channels.
    uint16_t channels_ = 0;

    /// The direction of drc
    uint16_t direction_ = DRC_UPLINK;

    /// DRC instance.
    void *drc_ = NULL;

    /// DRC frame length. Unit is Word(int16_t)
    int frm_len_ = 0;

    /// DRC enable.
    int16_t enable_ = 1;

    /// DRC Attack Time. Unit is ms.
    int attack_time_ = 10;

    /// DRC Decay Time. Unit is ms.
    int decay_time_ = 110;

    /// DRC Knee Threshold.
    int knee_thrd_ = -10;

    /// DRC Noise Gate.
    int noise_gate_ = -30;

    /// DRC Slope. Q10.
    int slope_ = 0.1 * (1 << 10);

    /// @brief Specific state change event handler.
    STATE_RETURN handleIdleToReady(void) override;

    /// @brief Process the data of in sink pad.
    /// @param pad Sink pad.
    /// @return AUST_SUCCESS if handled successfully, otherwise AUST_ERROR.
    AUST_ERR algoProcess(AustPad *pad) override;

    /// @brief Get configuration.
    /// @return true if get config successfully, otherwise false.
    bool getConfig(void) override;

    /// @brief Initialize algorithm.
    /// @return true if init successfully, otherwise false.
    bool initAlgo(void) override;

    /// @brief Deinitialize algorithm.
    void deinitAlgo(void) override;

  public:
    DrcElement();
    ~DrcElement();
};

#endif /* __AUST_DRC_ELEMENT_H__ */
