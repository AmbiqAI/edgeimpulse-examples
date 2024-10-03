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

#ifndef __AUST_MBDRC_ELEMENT_H__
#define __AUST_MBDRC_ELEMENT_H__

#include "aust_algo_element.h"

/// @brief MbdrcElement is used for Multi-band DRC process.
class MbdrcElement : public AlgoElement
{
  private:
    /// Bitstream sample rate. Allowed sample rate: 16000.
    uint32_t sample_rate_ = 0;

    /// Number of channels.
    uint16_t channels_ = 0;

    /// Frame length.
    uint16_t frame_len_ = 0;

    /// Input data size.
    uint16_t in_size_ = 0;

    /// Output data size.
    uint16_t out_size_ = 0;

    /// Pointer to the persistent RAM data memory for MBDRC.
    void *mbdrc_ = NULL;

    /// MBDRC enable.
    int16_t enable_ = 1;

    /// Maximum band number of mbdrc algorithm.
    static const uint16_t max_band_num_ = 16;

    /// Default active band number.
    static const uint16_t default_band_num_ = 16;

    /// Subband number, range [1, max_band_num_].
    uint16_t band_num_ = default_band_num_;

    /// Subband frequency table.
    int bound_high_[max_band_num_] = {100,  200,  300,  500,  700,  1000,
                                      1300, 4000, 4500, 5000, 5500, 6000,
                                      6500, 7000, 7500, 8000};

    /// Subband compress threshold table.
    int16_t comp_thd_[max_band_num_] = {-20, -12, -20, -30, -30, -30, -30, -30,
                                        -30, -30, -30, -30, -30, -30, -30, -30};

    /// Subband compress slope table.
    float comp_slope_[max_band_num_] = {0.2, 0.3, 0.5, 0.7, 0.7, 0.7, 0.7, 0.7,
                                        0.7, 0.7, 0.7, 0.7, 0.7, 0.7, 0.7, 0.7};

    /// @brief Specific state change event handler.
    STATE_RETURN handleIdleToReady(void) override;
    STATE_RETURN handleReadyToPause(void) override;
    STATE_RETURN handlePauseToReady(void) override;

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
    MbdrcElement();
    ~MbdrcElement();
};

#endif /* __AUST_MBDRC_ELEMENT_H__ */
