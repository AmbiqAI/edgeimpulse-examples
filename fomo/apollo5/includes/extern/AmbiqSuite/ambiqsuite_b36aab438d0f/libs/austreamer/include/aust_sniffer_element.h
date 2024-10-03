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

#ifndef __AUST_SNIFFER_ELEMENT_H__
#define __AUST_SNIFFER_ELEMENT_H__

#include "aust_algo_element.h"

/// @brief SnifferElement is used for activate or deactivate data sniffer.
///
class SnifferElement : public AlgoElement
{
  private:
    /// @brief Handle messages from message manager.
    /// @param type Message type.
    /// @param blocks Message data.
    /// @param len Message length.
    /// @param priv Private pointer.
    /// @return 0 if handled successfully, otherwise error code.
    static uint16_t handleMessage(uint8_t type, uint8_t *blocks, uint32_t len,
                                  void *priv);
    uint16_t handleMessageInteranl(uint8_t type, uint8_t *blocks, uint32_t len);

    /// @brief Check algorithm parameters callback.
    /// @param id Sub-block ID described in sub_block_id_t.
    /// @param data Pointer to algorithm parameters.
    /// @param size Size of algorithm parameters.
    /// @param priv Private pointer of the checking module.
    /// @return 0 if check successfully, otherwise error code.
    static int32_t checkAlgoParamCb(uint32_t id, const uint8_t *data,
                                    uint32_t size, void *priv);
    int32_t        checkAlgoParamCbInteranl(uint32_t id, const uint8_t *data,
                                            uint32_t size);

    /// @brief Notify that the parameter of Sub-block ID is changed.
    /// @param id Sub-block ID described in sub_block_id_t.
    /// @param priv Private pointer of the receiving notify module.
    static void notifyCb(uint32_t id, void *priv);
    void        notifyCbInternal(uint32_t id);

    /// @brief Get the object ID by given Sub-block ID.
    /// @param id Sub-block ID described in sub_block_id_t.
    /// @return Object ID.
    uint32_t getObjIdBySbId(uint32_t id);

    /// @brief Specific state change event handler.
    STATE_RETURN handleIdleToReady(void) override;

    /// @brief Process the data of in sink pad.
    /// @param pad Sink pad.
    /// @return AUST_SUCCESS if handled successfully, otherwise AUST_ERROR.
    AUST_ERR algoProcess(AustPad *pad) override;

  public:
    SnifferElement();
    ~SnifferElement();
};

#endif /* __AUST_SNIFFER_ELEMENT_H__ */
