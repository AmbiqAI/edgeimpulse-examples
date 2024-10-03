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

#ifndef __AUST_PROMPT_SRC_ELEMENT_H__
#define __AUST_PROMPT_SRC_ELEMENT_H__

#include "adev.h"
#include "aust_wave_src_element.h"

/// Prompt type.
enum
{
    PROMPT_SINE_WAVE, /**< Auto generated sine wave */
    PROMPT_WAV,       /**< Waveform audio file */
    PROMPT_PCM,       /**< Audio raw data file */
    PROMPT_SBC,       /**< SBC audio file */
    PROMPT_MP3,       /**< MPEG-1 Audio Layer III file */
};

/// @brief PromptSrcElement is used for prompt control.
class PromptSrcElement : public WaveSrcElement
{
  private:
    /// Pipeline sample rate.
    adev_sample_rate_t sample_rate_pipe_ = ADEV_SAMPLE_RATE_16000_HZ;

    /// Pipeline number of pipeline channels.
    uint16_t channels_pipe_ = 0;

    // Pipeline frame size that each time pass to next element. Unit is Byte.
    uint32_t frame_size_pipe_ = 0;

    /// Self data speed to pipeline data speed
    float ratio_ = 1.0;

    /// Prompt start flag.
    bool playing_ = false;

    /// Prompt play duration.
    uint32_t duration_ms_ = 0;

    /// Data size of duration.
    uint32_t duration_size_ = 0;

    /// Silence size after the played duration end.
    uint32_t silence_size_ = 0;

    /// File address.
    uint32_t address_ = 0;

    /// File length.
    uint32_t length_ = 0;

    /// Prompt type.
    uint32_t type_ = 0;

    /// File pointer.
    uint32_t fp_ = 0;

    /// Audio file read position.
    uint32_t offset_ = 0;

  private:
    /// @brief Check if prompt is started.
    /// @return true if started, otherwise false.
    bool isPromptStarted(void);

    /// @brief Prepare prompt playing.
    /// @return AUST_SUCCESS if handled successfully, otherwise AUST_ERROR.
    AUST_ERR promptStart(void);

    /// @brief Stop prompt playing.
    void promptStop(void);

    /// @brief Prompt data processing.
    /// @return AUST_SUCCESS if handled successfully, otherwise AUST_ERROR.
    AUST_ERR promptProcess(void);

    /// @brief Sine wave audio prepare.
    /// @return AUST_SUCCESS if handled successfully, otherwise AUST_ERROR.
    AUST_ERR wavePrepare(void);

    /// @brief Local waveform audio prepare.
    /// @return AUST_SUCCESS if handled successfully, otherwise AUST_ERROR.
    AUST_ERR wavPrepare(void);

    /// @brief Local PCM audio prepare.
    /// @return AUST_SUCCESS if handled successfully, otherwise AUST_ERROR.
    AUST_ERR pcmPrepare(void);

    /// @brief Local MP3 audio prepare.
    /// @return AUST_SUCCESS if handled successfully, otherwise AUST_ERROR.
    AUST_ERR mp3Prepare(void);

    /// @brief Sine wave audio stop.
    void waveStop(void);

    /// @brief Waveform audio stop.
    void wavStop(void);

    /// @brief PCM audio stop.
    void pcmStop(void);

    /// @brief MP3 audio stop.
    void mp3Stop(void);

    /// @brief Sine wave audio data processing.
    /// @return AUST_SUCCESS if handled successfully, otherwise AUST_ERROR.
    AUST_ERR waveProcess(void);

    /// @brief Waveform audio data processing.
    /// @return AUST_SUCCESS if handled successfully, otherwise AUST_ERROR.
    AUST_ERR wavProcess(void);

    /// @brief PCM audio data processing.
    /// @return AUST_SUCCESS if handled successfully, otherwise AUST_ERROR.
    AUST_ERR pcmProcess(void);

    /// @brief MP3 audio data processing.
    /// @return AUST_SUCCESS if handled successfully, otherwise AUST_ERROR.
    AUST_ERR mp3Process(void);

    /// @brief Submit silence data processing.
    /// @return AUST_SUCCESS if handled successfully, otherwise AUST_EOF.
    AUST_ERR silenceProcess(void);

    /// @brief Read data from flash to out buffer.
    /// @param out Pointer of out buffer.
    /// @param size Size of data to read in bytes.
    /// @return Actually read size.
    uint32_t fileRead(uint8_t *out, uint32_t size);

    /// @brief Parse waveform audio header.
    /// @param audio_header Header pointer.
    /// @return true if parsing successful, otherwise false.
    bool wavParse(void *audio_header);

  public:
    PromptSrcElement();
    ~PromptSrcElement();

    /// @brief Specific state change event handler.
    STATE_RETURN handleIdleToReady(void);
    STATE_RETURN handleReadyToIdle(void);
    STATE_RETURN handleReadyToPause(void);
    STATE_RETURN handlePauseToReady(void);

    /// @brief State change handler.
    /// @param to_state State transition event.
    /// @return Result of handling the state change event.
    STATE_RETURN stateChange(STATE_CHANGE to_state) override;

    /// @brief AustBus message handler.
    /// @param msg AustMessage reference.
    void msgHandler(AustMessage *msg) override;

    /// @brief Start sine wave audio prompt playing.
    /// @param frequency Sine wave frequency.
    /// @param duration Play time in milliseconds.
    void prompt_play_sine_wave(uint32_t frequency, uint32_t duration_ms);

    /// @brief Start Waveform audio prompt playing.
    /// @param address Waveform file address in flash.
    /// @param length Waveform file size in bytes.
    void prompt_play_wav(uint32_t address, uint32_t length);

    /// @brief Start PCM audio prompt playing.
    /// @param address PCM file address in flash.
    /// @param length PCM file size in bytes.
    void prompt_play_pcm(uint32_t address, uint32_t length);

    /// @brief Start MP3 audio prompt playing.
    /// @param address MP3 file address in flash.
    /// @param length MP3 file size in bytes.
    void prompt_play_mp3(uint32_t address, uint32_t length);

    /// @brief Stop prompt playing.
    void prompt_stop(void);
};

#endif /* __AUST_PROMPT_SRC_ELEMENT_H__ */
