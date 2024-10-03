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

#ifndef __AUST_ELEMENT_H__
#define __AUST_ELEMENT_H__

#include <vector>

#include "aust_bus.h"
#include "aust_pad.h"
#include "aust_type.h"

/// @brief States of the AustElement.
typedef enum
{
    STATE_IDLE  = 0, ///< Pipeline is in idle.
    STATE_READY = 1, ///< Pipeline is in ready.
    STATE_PAUSE = 2, ///< Pipeline is in pause.
    STATE_PLAY  = 3, ///< Pipeline is in playing.
    STATE_MAX
} STATE;

/// @brief Results of the last state change.
typedef enum
{
    CHANGE_SUCCESS, ///< Pipeline state change success.
    CHANGE_ASYNC,   ///< Pipeline is being in state changing.
    CHANGE_FAIL,    ///< Pipeline state change fail.
} STATE_RETURN;

/// @brief State transition events, from first state to second state.
typedef enum
{
    IDLE_TO_READY  = 0, ///< Event of from idle state to ready state.
    READY_TO_PAUSE = 1, ///< Event of from ready state to pause state.
    PAUSE_TO_PLAY  = 2, ///< Event of from pause state to play state.
    PLAY_TO_PAUSE  = 3, ///< Event of from play state to pause state.
    PAUSE_TO_READY = 4, ///< Event of from pause state to ready state.
    READY_TO_IDLE  = 5, ///< Event of from ready state to idle state.
    STATE_CHANGE_ERROR
} STATE_CHANGE;

/// @brief State change direction, UP means IDLE->PLAY, DOWN means PLAY->IDLE.
typedef enum
{
    UP   = 0, ///< Any state changes in IDLE->READY->PAUSE->PLAY means UP.
    DOWN = 1, ///< Any state changes in PLAY->PAUSE->READY->IDLE means DOWN.
} DIRECTION;

/// @brief State machine.
class StateMachine
{
  private:
    /// Current state.
    STATE state_;

    /// State change pending state.
    bool state_change_pending_;

  protected:
    StateMachine() : state_(STATE_IDLE), state_change_pending_(false){};
    ~StateMachine(){};

  public:
    /// @brief Set current state.
    /// @param state Machine state.
    void setState(STATE state)
    {
        state_ = state;
    }

    /// @brief Get current state.
    /// @return Machine state.
    STATE getState(void)
    {
        return state_;
    }

    /// @brief Set state change pending state.
    /// @param pending True or false.
    void setStateChangePending(bool pending)
    {
        state_change_pending_ = pending;
    }

    /// @brief Get state change pending state.
    /// @return True: state change is pending, otherwise, false.
    bool getStateChangePending(void)
    {
        return state_change_pending_;
    }

    /// @brief State transition event handler.
    /// @param to_state State change event.
    /// @return Result of handling the state change event.
    virtual STATE_RETURN stateChange(STATE_CHANGE to_state)
    {
        return CHANGE_SUCCESS;
    }
};

/// @brief AustElement is the basic building block for a media pipeline.
///
/// @details It can add sink pads to receive data or source pads to send data,
/// and can be linked together by its pad from source pad to sink pad. Each
/// element has an AustBuffer instance which will be filled with payload and
/// pass to next element by its sink pad.
class AustElement : public AustObject, public StateMachine
{
  private:
    /// AustElement alias.
    std::string name_;

  protected:
    /// Reference to AustPipeline AustBus to receive or send message.
    AustBus *bus_;

    /// AustBuffer instance used to load with payload and pass to next element
    /// by pad.
    AustBuffer *buffer_;

    /// Source pad is for sending buffer to next element sink pad.
    std::vector<AustPad *> srcPads_;

    /// Sink pad is for receiving buffer from previous element source pad.
    std::vector<AustPad *> sinkPads_;

    /// Out pad 0 of this element, aka src pad 0.
    AustPad *out_pad_0_ = NULL;

    /// Out pad 1 of this element, aka src pad 1.
    AustPad *out_pad_1_ = NULL;

    /// Pipeline.addElement() will add itself to element.
    AustElement *pipeline_;

    /// @brief Add pad to element.
    /// @param pad Src pad.
    void addSrcPad(AustPad *pad);

    /// @brief Add pad to element.
    /// @param pad Sink pad.
    void addSinkPad(AustPad *pad);

  public:
    AustElement();
    virtual ~AustElement();

    /// @brief Set the name for element.
    /// @param name Name of string.
    void setName(std::string name);

    /// @brief Get the name of element.
    /// @return Name of the element.
    std::string getName(void);

    /// @brief Set bus reference to element to receive or send message.
    /// @param bus AustBus reference.
    virtual void setBus(AustBus *bus);

    /// @brief Set buffer instance, element can use it to load with payload.
    /// @param buffer AustBuffer reference.
    void setBuffer(AustBuffer *buffer);

    /// @brief Set pipeline instance, element can use it to get pipeline
    /// property
    /// @param pipeline AustElement reference.
    void setPipeline(AustElement *pipeline);

    /// @brief Link this element to next element, aka link source pad to sink
    /// pad.
    /// @param next Next AustElement to be linked.
    /// @return AUST_SUCCESS if link successfully, otherwise AUST_ERROR.
    AUST_ERR linkElement(AustElement *next);

    /// @brief Process the data passed by in pad which is one of the sink pad.
    /// @param pad One of the sink pad of this element. It depends on which
    /// linked peer source pad of the previous element is processing.
    /// @return AUST_SUCCESS if handled successfully, otherwise AUST_ERROR.
    virtual AUST_ERR process(AustPad *pad) = 0;
};

#endif /* __AUST_ELEMENT_H__ */
