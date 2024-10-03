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
#ifndef __AUST_INTRLVR_H__
#define __AUST_INTRLVR_H__

#include "aust_type.h"

#ifdef __cplusplus
extern "C"
{
#endif

    /**
     * @defgroup AUST_INTERLEAVE Interleave And Deinterleave
     * @brief Interleave or deinterleave pcm data
     * @ingroup AUST_INTERLEAVE
     * @{
     */

    typedef enum
    {
        AUST_INTERLEAVE = 0,
        AUST_DEINTERLEAVE
    } aust_intrlvr_type_t;

    /**
     * @brief Processing data in original buffer for specify samples.
     * Data format must be 2 channels and 16-bit.
     *
     * For the better performance purpose, this function according to some
     * specify samples number. Need to add code to support a new sample number.
     *
     * @param type Select processing type, Interleave or deinterleave.
     * @param data Pointer to data buffer. This buffer data
     * will be replaced after processing. Buffer size must be 294
     * samples(588 Bytes).
     * @param samples Sample number of the data buffer. Sample size is 16 bits.
     *
     * @return 0 means success, 1 means this type and samples is unsupported
     *
     */
    uint32_t aust_intrlvr_1buff(aust_intrlvr_type_t type, int16_t *data,
                                uint32_t samples);

    /**
     * @brief Processing interleave/deinterleave data. Support 2
     * channels data only. Size of data_ch1_ch2 and data_interleave should be
     * the same. Data format must be 2 channels and 16-bit.
     *
     * @param type Select processing type, Interleave or deinterleave.
     * @param data_deinterleave Pointer to the buffer with deinterleaved data.
     * @param data_interleave Pointer to the buffer with interleaved data.
     * @param samples Sample number of the data buffer. Sample size is 16 bits.
     *
     */
    void aust_intrlvr_2buff(aust_intrlvr_type_t type,
                            int16_t            *data_deinterleave,
                            int16_t *data_interleave, uint32_t samples);

    /**
     * @brief Interleave or deinterleave processing for channel-1 and channel-2
     * buffer address are separated Data format must be 2 channels and 16-bit.
     *
     * @param data_ch1 Pointer to the buffer of channel-1 data.
     * @param data_ch2 Pointer to the buffer of channel-2 data.
     * @param data_interleave Pointer to the buffer with interleaved data.
     * @param samples Sample number of the interleaved data buffer. Sample size
     * is 16 bits.
     *
     */
    void aust_intrlvr_3buff(aust_intrlvr_type_t type, int16_t *data_ch1,
                            int16_t *data_ch2, int16_t *data_interleave,
                            uint32_t samples);

    /**
     * @brief Print all the start positions. This function is a test function to
     * support interleave swap algorithm for api aust_intrlvr_1buff.
     * This function must be called in metal thread context.
     *
     * @param type Interleave of Deinterleave
     * @param samples Sample number of the data buffer. Sample size is 16 bits.
     */
    void aust_intrlvr_print_start_pos(aust_intrlvr_type_t type,
                                      uint32_t            samples);

    /**
     * @}
     */

#ifdef __cplusplus
}
#endif

#endif /* __AUST_INTRLVR_H__ */
