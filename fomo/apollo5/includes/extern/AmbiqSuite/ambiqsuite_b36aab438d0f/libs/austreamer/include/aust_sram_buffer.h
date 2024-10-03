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

#ifndef __AUST_SRAM_BUFFER_H__
#define __AUST_SRAM_BUFFER_H__

#include <acore_atomic.h>

#include "aust_buffer.h"

#define AUST_SRAM_MAX_SIZE 1024 * 40

/// @brief SramBuffer is used to require buffer in SRAM.
class SramBuffer : public AustBuffer
{
  private:
    /// Sum of allocated memory of all SramBuffer instances.
    static volatile atomic_int sum_;

    /// @brief Truly free the allocated memory.
    void try_free_payload(void);

  public:
    SramBuffer();
    ~SramBuffer();

    /// @brief Require memory from SRAM.
    /// @param size Size of bytes.
    /// @return Payload address if successful, otherwise NULL.
    void *require(int size) override;

    /// @brief Release payload memory.
    void release(void) override;

    /// @brief Get the payload address.
    /// @return Payload address.
    void *payload(void) override;

    /// @brief Get the payload size.
    /// @return Payload size.
    int size(void) override;
};

#endif /* __AUST_SRAM_BUFFER_H__ */
