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

#ifndef __AUST_GENERIC_ELEMENT_H__
#define __AUST_GENERIC_ELEMENT_H__

#include "aust_msg_element.h"

/// @brief Generic element processing callback. Input and output with the same
/// sample number. Property "type" has option "process", "src" and "sink".
/// @param in Data input buffer.
/// @param out Data output buffer.
/// @param size Number of sample. One Sample is 16bit, 2 bytes.
typedef void (*GENERIC_PROCESS_CALLBALCK)(int16_t *in, int16_t *out, int size);

class GenericElement : public MsgElement
{

    enum
    {
        GENERIC_TYPE_PROCESS = 0,
        GENERIC_TYPE_SRC     = 1,
        GENERIC_TYPE_SINK    = 2,
        GENERIC_TYPE_BYPASS  = 3
    };

  private:
    int type_ = GENERIC_TYPE_PROCESS;

    /// Size of needed data from message for source type element.
    uint32_t need_data_size_ = 0;

    /// Callback handler.
    GENERIC_PROCESS_CALLBALCK generic_process_cb = NULL;

    STATE_RETURN handleIdleToReady(void);
    STATE_RETURN handleReadyToIdle(void);

    STATE_RETURN stateChange(STATE_CHANGE to_state) override;

    AUST_ERR process(AustPad *pad) override;

    /// @brief AustBus message handler.
    /// @param msg AustMessage reference.
    void msgHandler(AustMessage *msg) override;

  public:
    GenericElement();
    ~GenericElement();

    /// @brief Register a processing callback function.
    /// @param cb Callback function.
    void register_process_cb(GENERIC_PROCESS_CALLBALCK cb);
};

#endif /* __AUST_GENERIC_ELEMENT_H__ */
