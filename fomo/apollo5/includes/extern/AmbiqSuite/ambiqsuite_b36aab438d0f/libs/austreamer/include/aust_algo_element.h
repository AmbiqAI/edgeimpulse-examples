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

#ifndef __AUST_ALGO_ELEMENT_H__
#define __AUST_ALGO_ELEMENT_H__

#include "acore_msg_struct.h"

#include "aust_msg_element.h"
#include "aust_element.h"

/// @brief AlgoElement is a basic class to handle acore message for agc drc peq
/// etc.
///
/// @ingroup PROPERTIES
/// @{
/// *****************************************************************
/// ## Properties
///
/// - ***VAD enable***
/// @code "vad-enable" @endcode
/// Enable to handle VAD message to bypass data.
/// Default value: 0
///
/// - ***Frequency domain in***
/// @code "freq-in" @endcode
/// Whether the type of input data is frequency domain.
/// 0: time domain, 1: frequency domain.
/// Default value: 0
///
/// - ***Frequency domain out***
/// @code "freq-out" @endcode
/// Whether the type of output data is frequency domain.
/// 0: time domain, 1: frequency domain.
/// Default value: 0
///
/// *****************************************************************
/// @}
class AlgoElement : public MsgElement
{
  protected:
    /// Sniffer active flag, only both enable and active flags are set, sniffer
    /// data can be sent out.
    uint8_t sniffer_active_ = true;

    /// Algo bypass flag.
    uint8_t bypass_ = 0;

    /// VAD enable flag.
    uint8_t vad_enable_ = 0;

    /// VAD active flag.
    uint8_t vad_active_ = 0;

    /// Frequency data input flag.
    uint8_t freq_in_ = 0;

    /// Frequency data output flag.
    uint8_t freq_out_ = 0;

    /// Million Cycles Per Second.
    uint32_t mcps_ = 0;

    /// MCPS module ID.
    uint32_t mcps_module_ = 0;

    /// MCPS module name.
    const char *mcps_name_ = NULL;

  private:
    /// Object id.
    int obj_ = 0;

    /// Pipeline sample rate.
    uint32_t sample_rate_ = 0;

    /// Pipeline number of channels.
    uint16_t channels_ = 0;

    /// @brief Sniffer input data and send to PC.
    /// @param dir Input or output.
    /// @param index Pad node index.
    /// @param data Pointer to data.
    /// @param size Data size.
    void snifferData(uint32_t dir, uint8_t index, uint8_t *data, uint32_t size);

    /// @brief Get MCPS module by object ID.
    /// @param obj Object ID.
    /// @return MCPS module ID.
    uint32_t getMcpsModuleByObjId(uint32_t obj);

    /// @brief Sniffer process.
    /// @param pad Sniffer pad.
    void sniffProcess(AustPad *pad);

    /// @brief MCPS process.
    /// @param state START or END.
    void mcpsProcess(uint32_t state);

    /// @brief Bypass process.
    /// @param pad Input sink pad.
    void bypassProcess(AustPad *pad);

  public:
    /// @brief Set the object id which will be registered in message agent.
    /// @param obj Object id.
    void setObject(int obj);

    /// @brief Get configuration.
    /// @return true if get config successfully, otherwise false.
    virtual bool getConfig(void);

    /// @brief Initialize algorithm.
    /// @return true if init successfully, otherwise false.
    virtual bool initAlgo(void);

    /// @brief Deinitialize algorithm.
    virtual void deinitAlgo(void);

    /// @brief Specific state change event handler.
    virtual STATE_RETURN handleIdleToReady(void);
    virtual STATE_RETURN handleReadyToIdle(void);
    virtual STATE_RETURN handleReadyToPause(void);
    virtual STATE_RETURN handlePauseToReady(void);

    /// @brief State change handler.
    /// @param to_state State transition event.
    /// @return Result of handling the state change event.
    STATE_RETURN stateChange(STATE_CHANGE to_state) override;

    /// @brief AustBus message handler.
    /// @param msg AustMessage reference.
    void msgHandler(AustMessage *msg) override;

    /// @brief Process the data of in sink pad.
    /// @param pad Sink pad.
    /// @return AUST_SUCCESS if handled successfully, otherwise AUST_ERROR.
    AUST_ERR process(AustPad *pad) override;

    /// @brief Specific algorithm elements should invoke this API to process in
    /// pad data.
    /// @param pad In sink pad.
    /// @return AUST_SUCCESS if handled successfully, otherwise AUST_ERROR.
    virtual AUST_ERR algoProcess(AustPad *pad) = 0;

    /// @brief Specific algorithm elements should invoke this API to submit
    /// processed data.
    /// @param pad Out src pad.
    void algoSubmit(AustPad *pad);

  public:
    AlgoElement();
    ~AlgoElement();
};

#endif /* __AUST_ALGO_ELEMENT_H__ */
