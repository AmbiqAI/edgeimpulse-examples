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

#ifndef __AUST_BUS_H__
#define __AUST_BUS_H__

#include <vector>

#include "aust_object.h"
#include "aust_queue.h"
#include "acore_event.h"

/// @brief AustMessage derived from AustObject, it's a message object.
class AustMessage : public AustObject
{
  private:
    /// Unique message ID, used for identifying message type.
    /// AustMessage detail information stored in properties.
    int msg_id_;

  public:
    AustMessage(int msg_id) : msg_id_(msg_id){};
    ~AustMessage(){};

    /// @brief Get the message ID.
    /// @return Message ID.
    int getMsgID(void)
    {
        return msg_id_;
    }
};

/// @brief AustListener abstract class, implementer can be registered on AustBus
/// to receive message.
class AustListener
{
  public:
    AustListener(){};
    ~AustListener(){};

    /// @brief Message handler for listener on AustBus to receive message.
    virtual void msgHandler(AustMessage *msg) = 0;
};

/// @brief AustBus is for AustListener to register and publish message.
class AustBus
{
    /// Message queue maximum size.
#define MSG_QUEUE_SIZE (10)

  private:
    AUDIO_EVENT_TYPE event_id_;

    /// Store all registered listeners.
    std::vector<AustListener *> listeners_;

    /// AustMessage queue.
    aust_queue_t *msg_queue_;

    /// @brief Message handle worker. Usually it's invoked by user C API.
    /// @param arg Private context of this instance.
    static void msgHandleThread(void *arg);

    /// @brief Message handle worker implementation for @ref msgHandleThread.
    void msgHandleThreadImpl(void);

  public:
    AustBus(AUDIO_EVENT_TYPE event_id);
    ~AustBus();

    /// @brief Subscribe listener into bus.
    /// @param listener AustListener reference.
    void subscribe(AustListener *listener);

    /// @brief Publish message to each listener which registered into bus.
    /// @param msg AustMessage reference.
    void publish(AustMessage *msg);
};

#endif /* __AUST_BUS_H__ */
