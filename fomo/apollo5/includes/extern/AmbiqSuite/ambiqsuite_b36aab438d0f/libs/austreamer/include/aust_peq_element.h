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

#ifndef __AUST_PEQ_ELEMENT_H__
#define __AUST_PEQ_ELEMENT_H__

#include "aust_algo_element.h"

/// @brief PeqElement is used for Parameters of EQ process.
///
/// @ingroup PROPERTIES
/// @{
/// *****************************************************************
/// ## Properties
///
/// - ***Link direction***
/// @code "link" @endcode
/// Supported link: uplink or downlink
///
/// *****************************************************************
/// @}

/// The direction of peq.
enum
{
    PEQ_UPLINK = 0,
    PEQ_DOWNLINK
};

class PeqElement : public AlgoElement
{
  private:
    /// Bitstream sample rate. Allowed sample rates are: 8000, 16000, 24000,
    /// 32000, 44100, 48000, 96000.
    uint32_t sample_rate_ = 0;

    /// Bitstream number of channels.
    uint16_t channels_ = 0;

    /// The direction of peq.
    uint16_t direction_ = PEQ_UPLINK;

    /// EQ instance.
    void *peq_ = NULL;

    /// PEQ enable.
    int16_t enable_ = 1;

    /// Maximum band number of peq algorithm.
    static const uint16_t max_band_num_ = 16;

    /// Default active band number.
    static const uint16_t default_band_num_ = 16;

    /// EQ band number.
    int band_num_ = default_band_num_;

    /// EQ center frequency list of every band. Unit is Hz.
    int band_center_freq_[max_band_num_] = {60,   100,  200,  400,  800,  1400,
                                            2000, 2500, 5500, 5700, 6000, 6200,
                                            6500, 7000, 7400, 7800};

    /// EQ Q factor list of every band.
    float band_qfactor_[max_band_num_] = {0.7, 2.5, 2.5, 2.5, 2.5, 2.5,
                                          2.5, 2.5, 2.5, 2.5, 2.5, 2.5,
                                          2.5, 2.5, 2.5, 2.5};

    /// EQ Gain list of every band. Unit is dB.
    int band_gain_[max_band_num_] = {0, 0, 0, 0, 0, 0, 0, 0,
                                     0, 0, 0, 0, 0, 0, 0, 0};

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
    PeqElement();
    ~PeqElement();
};

#endif /* __AUST_PEQ_ELEMENT_H__ */
