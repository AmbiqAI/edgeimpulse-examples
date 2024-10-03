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

#ifndef __AUST_RESAMPLE_ELEMENT_H__
#define __AUST_RESAMPLE_ELEMENT_H__

#include "aust_algo_element.h"

/// @brief ResampleElement is used for Fast Resample process.
///
/// @ingroup PROPERTIES
/// @{
/// *****************************************************************
/// ## Properties
///
/// - ***Sample rate in***
/// @code "sr-in" @endcode
/// Supported in sample rate: 48000 and 44100
///
/// - ***Sample rate out***
/// @code "sr-out" @endcode
/// Supported out sample rate: 16000 and 8000
///
/// *****************************************************************
/// @}

/// A rule map for resample to convert from sr_in to sr_out, and input sample
/// limitation.
typedef struct resample_map
{
    /** Input sample rate */
    uint16_t sr_in;

    /** Output sample rate */
    uint16_t sr_out;

    /** Input samples factor, number of input samples must be multiple of it */
    uint16_t sample_base;
} resample_map;

class ResampleElement : public AlgoElement
{
  private:
    /// Bitstream sample rate.
    /// Allowed sample rates are: 8000, 16000, 44100, 48000.
    uint32_t sample_rate_ = 0;

    /// Number of channels.
    uint16_t channels_ = 0;

    /// Pointer to the persistent RAM data memory for resample.
    void *ptr_resample_ = NULL;

    /// Input sample rate.
    uint32_t sr_in_ = 0;

    /// Output sample rate.
    uint32_t sr_out_ = 0;

    /// Input samples factor, number of input samples must be multiple of it.
    uint16_t sample_base_ = 0;

    /// Only 6 kinds of resample rule are supported.
    const resample_map resample_tbl[6] = {
        {48000, 16000, 3},  {48000, 8000, 6},  {44100, 16000, 441},
        {44100, 8000, 441}, {16000, 48000, 1}, {8000, 16000, 1},
    };

    /// @brief Specific state change event handler.
    STATE_RETURN handleIdleToReady(void) override;

    /// @brief Process the data of in sink pad.
    /// @param pad Sink pad.
    /// @return AUST_SUCCESS if handled successfully, otherwise AUST_ERROR.
    AUST_ERR algoProcess(AustPad *pad) override;

    /// @brief Initialize algorithm.
    /// @return true if init successfully, otherwise false.
    bool initAlgo(void) override;

    /// @brief Deinitialize algorithm.
    void deinitAlgo(void) override;

  public:
    ResampleElement();
    ~ResampleElement();
};

#endif /* __AUST_RESAMPLE_ELEMENT_H__ */
