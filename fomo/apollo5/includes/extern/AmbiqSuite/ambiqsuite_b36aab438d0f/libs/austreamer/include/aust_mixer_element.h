//*****************************************************************************
//
// Copyright (c) 2024, Ambiq Micro, Inc.
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

#ifndef __AUST_MIXER_ELEMENT_H__
#define __AUST_MIXER_ELEMENT_H__

#include "aust_ring_buffer.h"
#include "aust_algo_element.h"

typedef struct pad_buf *pad_buf_t;

#define DEFAULT_FADE_IN_FRAMES  (50) // Default number of frames for fade in.
#define DEFAULT_FADE_OUT_FRAMES (5)  // Default number of frames for fade out.

#define DEFAULT_MAIN_TARGET_GAIN   (0.3)
#define DEFAULT_BRANCH_TARGET_GAIN (0.9)

/// @brief MixerElement is used to mix two mono data of two individual sink pad
/// into stereo data of one single src pad.
///
/// @ingroup PROPERTIES
/// @{
/// *****************************************************************
/// ## Properties
///
/// - ***Fade in frames***
/// @code "fade-in-frames" @endcode
/// Fade in number of frames.
/// Default value: 50
///
/// - ***Fade out frames***
/// @code "fade-out-frames" @endcode
/// Fade out number of frames.
/// Default value: 5
///
/// - ***Pad 0 target gain***
/// @code "pad-0-target-gain" @endcode
/// Pad 0 target gain while fading out.
/// Default value: "0.3"
///
/// - ***Pad 1 target gain***
/// @code "pad-1-target-gain" @endcode
/// Pad 1 target gain while fading in.
/// Default value: "0.9"
///
/// *****************************************************************
/// @}
class MixerElement : public AlgoElement
{
  private:
    /// Sample rate.
    uint32_t sample_rate_ = 0;

    /// Number of channels.
    uint16_t channels_ = 0;

    /// Number of input sink pad.
    uint16_t src_num_ = 0;

    /// Number of frame size in ring buffer.
    uint16_t ring_buffer_frame_num_ = 0;

    /// Number of frames for fade in.
    uint32_t fade_in_frames_ = DEFAULT_FADE_IN_FRAMES;

    /// Number of frames for fade out.
    uint32_t fade_out_frames_ = DEFAULT_FADE_OUT_FRAMES;

    /// Frame size.
    uint32_t frame_size_ = 0;

    /// Main line target gain while fade out to mixing.
    float main_target_gain_ = DEFAULT_MAIN_TARGET_GAIN;

    /// Branch line target gain while fade in to mixing.
    float branch_target_gain_ = DEFAULT_BRANCH_TARGET_GAIN;

    /// Pointer to all pad and buffer map information.
    pad_buf_t pad_buf_ = NULL;

    /// @brief Mix message handler.
    void mixMsgHandler(void);

    /// @brief Check if fade condition is reached.
    /// @return true if changed, otherwise false.
    bool fadeConditionCheck(void);

    /// @brief Recalculate weighted and reconfigure fade module.
    void reCalcWtdAndReConfFade(void);

    /// @brief Fade process and mix process.
    void fadeAndMix(void);

    /// @brief Initialize ring buffer.
    /// @param buf Ring buffer pointer.
    /// @param frame_num Frame number, buffer size = frame size * frame num.
    /// @return true if initializing successfully, otherwise false.
    bool initBuffer(ring_buf **buf, uint32_t frame_num);

    /// @brief Deinitialize ring buffer.
    /// @param buf Ring buffer pointer.
    void deinitBuffer(ring_buf *buf);

    /// @brief Specific state change event handler.
    STATE_RETURN handleIdleToReady(void) override;
    STATE_RETURN handleReadyToPause(void) override;
    STATE_RETURN handlePauseToReady(void) override;

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
    MixerElement();
    ~MixerElement();
};

#endif /* __AUST_MIXER_ELEMENT_H__ */
