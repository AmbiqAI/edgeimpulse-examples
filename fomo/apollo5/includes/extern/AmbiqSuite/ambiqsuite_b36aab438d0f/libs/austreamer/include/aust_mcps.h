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
#ifndef __AUST_MCPS_H__
#define __AUST_MCPS_H__

#include <stdint.h>

#ifdef __cplusplus
extern "C"
{
#endif

    /**
     * @brief MCPS enable or disable.
     */
    enum
    {
        MCPS_OFF,
        MCPS_ON,
    };

    /**
     * @brief MCPS module.
     */
    enum
    {
        MCPS_SYS,
        MCPS_ASP,
        MCPS_AEC,
        MCPS_AGC,
        MCPS_NS,
        MCPS_PEQ_UL,
        MCPS_PEQ_DL,
        MCPS_DRC_UL,
        MCPS_DRC_DL,
        MCPS_MBDRC,
        MCPS_GAIN,

        MCPS_MAX
    };

    /**
     * @brief Initialise MCPS and enable or disable it.
     *
     * @param mcu_freq CPU frequency.
     */
    void aust_mcps_init(uint32_t mcu_freq);

    /**
     * @brief Enable or disable MCPS module, each module is independent.
     *
     * @param module MCPS module.
     * @param enable false disable, otherwise enable.
     * @param name MCPS module name alias.
     */
    void aust_mcps_module_enable(uint32_t module, bool enable,
                                 const char *name);

    /**
     * @brief Start count for specific module.
     *
     * @param module MCPS module.
     */
    void aust_mcps_count_begin(uint32_t module);

    /**
     * @brief Stop count for specific module.
     *
     * The count will be added up, MCPS will be calculated after interval time.
     *
     * @param module MCPS module.
     */
    void aust_mcps_count_end(uint32_t module);

    /**
     * @brief Get MCPS value about the specific module.
     *
     * @param module MCPS module.
     * @return MCPS, unit M.
     */
    uint32_t aust_mcps_get_mcps(uint32_t module);

    /**
     * @brief Get the module name.
     *
     * @param module MCPS module.
     * @return Pointer to module name.
     */
    const char *aust_mcps_get_module_name(uint32_t module);

    /**
     * @brief System MCPS update interval.
     *
     * @param seconds Seconds to update the MCPS value.
     */
    void aust_mcps_set_sys_update_interval(uint32_t seconds);

    /**
     * @brief Algorithms MCPS update interval.
     *
     * @param seconds Seconds to update the MCPS value.
     */
    void aust_mcps_set_algo_update_interval(uint32_t seconds);

#ifdef __cplusplus
}
#endif

#endif /* __AUST_MCPS_H__ */
