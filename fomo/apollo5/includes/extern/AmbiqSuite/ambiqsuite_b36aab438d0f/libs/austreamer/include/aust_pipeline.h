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

#ifndef __AUST_PIPELINE_H__
#define __AUST_PIPELINE_H__

#include <acore_mutex.h>

#include "aust_element.h"
#include "aust_bus.h"
#include "aust_sch.h"

/// @defgroup PROPERTIES Element Properties
/// @brief Elements set that has properties

/// @brief Message handler callback.
typedef void (*msg_handler)(uint32_t msg, void *args);

/// @brief AustPipeline is in charge of pipeline state machine. It has an
/// AustBus instance, and each AustElement added into this pipeline could
/// receive and send message.
///
/// @ingroup PROPERTIES
/// @{
/// *****************************************************************
/// ## Properties
///
/// - ***Sample rate***
/// @code "sample-rate" @endcode
/// Supported sample rate: 16000 or 48000
///
/// - ***Channels***
/// @code "channels" @endcode
/// Supported channels: 1 or 2
///
/// - ***pcm frame size***
/// @code "pcm-frame-size" @endcode
/// PCM data frame size. Unit is Byte.
///
/// *****************************************************************
/// @}
class AustPipeline : public AustElement, public AustListener
{
  private:
    /// AustBus instance for AustListener to send message.
    AustBus *bus_ = NULL;

    /// AustElement added into this pipeline.
    std::vector<AustElement *> elements_;

    /// The initial state when state starts to transfer.
    STATE initial_state_;

    /// The final state when state starts to transfer at the end.
    STATE final_state_;

    /// AustPipeline current state.
    STATE current_state_;

    /// AustPipeline next state that is being changing to.
    STATE next_state_;

    /// AustPipeline state change direction.
    DIRECTION direction_;

    /// Flag about setting state synchronously.
    bool sync_busy_;

    /// The result of the last state change.
    STATE_RETURN async_state_;

    /// Mutex for state change.
    metal_mutex_t sync_mutex_;

    /// AustBus message handler for user.
    msg_handler msg_handler_;

    /// AustBus message handler parameter for user.
    void *msg_args_;

    /// AustPcmSchElement instance.
    AustSch *sch_ = NULL;

    /// @brief Get the state transition event with current_state and direction.
    /// @param current_state Current state of pipeline.
    /// @param direction Direction of UP or DOWN.
    /// @return State transition event.
    STATE_CHANGE stateToTransfer(STATE current_state, DIRECTION direction);

    /// @brief Get the next state with current_state and direction.
    /// @param current_state Current state of pipeline.
    /// @param direction Direction of UP or DOWN.
    /// @return Next state of pipeline.
    STATE getNextState(STATE current_state, DIRECTION direction);

    /// @brief Handle the state machine internally.
    /// @param state The final state that pipeline will change to.
    /// @return CHANGE_SUCCESS if state change successfully, otherwise
    /// CHANGE_FAIL.
    STATE_RETURN setStateInternal(STATE state);

    /// @brief Set all elements state change to pending state.
    /// @param pending true if want to pending, otherwise false.
    void setAllStateChangePending(bool pending);

    /// @brief Set all elements from current state to next state with state
    /// change event.
    /// @param current_state Current state of pipeline.
    /// @param next_state Next state of pipeline.
    /// @param state_change State change event.
    /// @param abort Whether return once one element state change fail.
    /// - true means return immediately once one element state change fail.
    /// - false means won't return until all elements execute state change.
    /// @return CHANGE_SUCCESS if state change successfully, otherwise
    /// CHANGE_FAIL.
    STATE_RETURN setAllStateChange(STATE current_state, STATE next_state,
                                   STATE_CHANGE state_change, bool abort);

    /// @brief State transition event handler.
    /// @param to_state State transition event
    /// @return CHANGE_SUCCESS if state change successfully, otherwise
    /// CHANGE_FAIL.
    STATE_RETURN stateChange(STATE_CHANGE to_state) override;

    /// @brief AustBus message handler.
    /// @param msg AustMessage reference.
    void msgHandler(AustMessage *msg) override;

    /// @brief Pure virtual function, just override.
    /// @param pad NA.
    /// @return AUST_SUCCESS.
    AUST_ERR process(AustPad *pad) override;

  public:
    /// @brief Pipeline constructor.
    /// @param event_id Audio Event ID
    AustPipeline(AUDIO_EVENT_TYPE event_id);
    ~AustPipeline();

    /// @brief Set pipeline state synchronously and return the result
    /// immediately.
    /// @param state The state want to change to.
    /// @return The result of state change. CHANGE_SUCCESS or CHANGE_FAIL.
    STATE_RETURN setState(STATE state);

    /// @brief Get pipeline current state and return the last state change
    /// result.
    /// @param[out] state Current state will be set to this variable.
    /// @return The result of last state change.
    STATE_RETURN getState(STATE *state);

    /// @brief Add element into pipeline.
    /// @param element AustElement reference.
    void addElement(AustElement *element);

    /// @brief For user to register AustListener instance to receive bus
    /// message.
    /// @param listener User AustListener reference.
    /// @return true if registered successfully, otherwise false.
    bool registerListener(AustListener *listener);

    /// @brief For user to register message handler callback to receive bus
    /// message.
    /// @param handler Message handler callback.
    /// @param args User private data.
    /// @return true if registered successfully, otherwise false.
    bool registerMsgHandler(msg_handler handler, void *args);
};

#endif /* __AUST_PIPELINE_H__ */
