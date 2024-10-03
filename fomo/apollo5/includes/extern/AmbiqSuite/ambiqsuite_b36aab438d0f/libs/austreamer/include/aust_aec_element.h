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

#ifndef __AUST_AEC_ELEMENT_H__
#define __AUST_AEC_ELEMENT_H__

#include "aust_ring_buffer.h"
#include "aust_algo_element.h"

/// Reference signal ring buffer size
#define REF_BUFFER_SIZE (1024 * 4)

#define FRAME_LEN_SHIFT (6)

/// MUST be 64.
#define AEC_FRAME_LEN (0x01 << FRAME_LEN_SHIFT)

/// @brief AecElement is used for Acoustic Echo Canceller process.
///
/// @note: AEC only supports 2 channels in and 1 channel out, the first element
/// that links to this element in pipeline will be treated as MIC signal
/// element, and the second will be as Reference signal element.
///
class AecElement : public AlgoElement
{
  private:
    /// Bitstream sample rate. Allowed sample rates are: 8000, 16000, 24000,
    /// 32000, 44100, 48000, 96000.
    uint32_t sample_rate_ = 0;

    /// Number of input channels MUST be two.
    const uint16_t channels_ = 2;

    /// Pointer to the persistent RAM data memory for AEC.
    void *fbaec_ = NULL;

    /// Frame length.
    uint16_t frame_len_ = AEC_FRAME_LEN;

    /// AEC enable.
    int16_t enable_ = 1;

    /// Filter length. MUST be multiple of frame_len_.
    uint16_t filter_len_ = AEC_FRAME_LEN * 1;

    /// Fixed delay in sample.
    uint16_t fixed_delay_ = 0;

    /// NLP enable.
    int16_t nlp_enable_ = 1;

    /// NLP level. Range from 0 to 2. 2 is the strongest.
    int16_t nlp_level_ = 1;

    /// Buffer for ref data to be processed by AEC.
    uint8_t *data_buf_ = NULL;

    /// Ring buffer to stage reference signal.
    ring_buf *sig_ref_ = NULL;

    /// Reference signal pad.
    AustPad *pad_ref_ = NULL;

    /// Microphone signal pad.
    AustPad *pad_mic_ = NULL;

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
    AecElement();
    ~AecElement();
};

#endif /* __AUST_AEC_ELEMENT_H__ */
