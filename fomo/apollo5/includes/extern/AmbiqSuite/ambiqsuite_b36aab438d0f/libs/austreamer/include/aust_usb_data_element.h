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

#ifndef __AUST_USB_DATA_ELEMENT_H__
#define __AUST_USB_DATA_ELEMENT_H__

#include "aust_msg_element.h"
#include "aust_ring_buffer.h"

/// @brief UsbDataElement is used to receive USB buffer data and enqueue data.
class UsbDataElement : public MsgElement
{
  public:
    UsbDataElement();
    ~UsbDataElement();

    /// @brief Handle USB packet. This is C API.
    /// @param buffer Data packet.
    /// @param length Data length.
    /// @param args Private args.
    static void handleUsbPacket(const uint8_t *buffer, uint32_t length,
                                void *args);

  private:
    struct ring_buf *rb_handle_    = NULL;
    uint32_t         rb_size_      = 10 * 1024;
    uint32_t         rb_threshold_ = 8 * 1024;
    int              rb_flag_      = 0;

    /// @brief Specific state change event handler.
    STATE_RETURN handleIdleToReady(void);
    STATE_RETURN handleReadyToIdle(void);
    STATE_RETURN handlePlayToPause(void);

    /// @brief State change handler.
    /// @param to_state State transition event.
    /// @return Result of handling the state change event.
    STATE_RETURN stateChange(STATE_CHANGE to_state) override;

    /// @brief AustBus message handler.
    /// @param msg AustMessage reference.
    void msgHandler(AustMessage *msg) override;

    /// @brief Handle need data message.
    /// @param msg Message.
    void handleNeedDataMsg(AustMessage *msg);
};

#endif /* __AUST_USB_DATA_ELEMENT_H__ */
