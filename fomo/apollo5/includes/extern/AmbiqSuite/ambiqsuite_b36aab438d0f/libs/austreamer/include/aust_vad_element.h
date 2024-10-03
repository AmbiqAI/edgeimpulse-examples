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

#ifndef __AUST_VAD_ELEMENT_H__
#define __AUST_VAD_ELEMENT_H__

#include "aust_algo_element.h"

/// Default VAD active debounce threshold, unit in times.
#define DEFAULT_ACTIVE_DEBOUNCE_THD (2)

/// Default VAD inactive debounce threshold, unit in times.
#define DEFAULT_INACTIVE_DEBOUNCE_THD (50)

typedef enum VAD_STATUS
{
    VAD_INACTIVE = 0,
    VAD_ACTIVE   = 1,
    VAD_ERROR    = 2,
} VAD_STATUS;

typedef struct
{
    /// @brief Voice active state change.
    /// @param status VAD status.
    void (*state_change)(VAD_STATUS status);
} vad_ops_t;

/// @brief VadElement is used for Voice Activity Detection.
///
/// @ingroup PROPERTIES
/// @{
/// *****************************************************************
/// ## Properties
///
/// - ***Active frames threshold***
/// @code "active-frames" @endcode
/// Active frames threshold.
/// Default value: 2
///
/// - ***Inactive frames threshold***
/// @code "inactive-frames" @endcode
/// Inactive frames threshold.
/// Default value: 50
///
/// *****************************************************************
/// @}
class VadElement : public AlgoElement
{
  private:
    /// Sample rate. Allowed sample rates are: 8000, 16000, 32000, 48000.
    uint32_t sample_rate_ = 0;

    /// VAD instance.
    void *vad_ = NULL;

    /// VAD enable.
    int16_t enable_ = 1;

    /// VAD status.
    VAD_STATUS status_ = VAD_INACTIVE;

    /// Active debounce counter.
    uint16_t active_debounce_cnt_ = 0;

    /// Active debounce counter threshold.
    uint16_t active_thd_ = DEFAULT_ACTIVE_DEBOUNCE_THD;

    /// Non-active debounce counter.
    uint16_t inactive_debounce_cnt_ = 0;

    /// Non-active debounce counter threshold.
    uint16_t inactive_thd_ = DEFAULT_INACTIVE_DEBOUNCE_THD;

    /// Callback handler.
    vad_ops_t *ops_ = NULL;

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
    VadElement();
    ~VadElement();

    /// @brief Set callback operations.
    /// @param ops Callback operations.
    void setOps(vad_ops_t *ops);
};

#endif /* __AUST_VAD_ELEMENT_H__ */
