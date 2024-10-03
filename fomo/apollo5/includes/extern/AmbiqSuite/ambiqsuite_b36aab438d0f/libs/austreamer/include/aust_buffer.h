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

#ifndef __AUST_BUFFER_H__
#define __AUST_BUFFER_H__

#include <stdint.h>
#include <stddef.h>

/// @brief AustBuffer abstract class, implementer can require/release memory.
class AustBuffer
{
  protected:
    /// Buffer type.
    int type_ = 0;

    /// Buffer payload address.
    void *payload_ = NULL;

    /// Buffer payload size.
    int size_ = 0;

  public:
    AustBuffer(){};
    virtual ~AustBuffer(){};

    /// @brief Get the buffer type.
    /// @return buffer type.
    int getType(void)
    {
        return type_;
    }

    /// @brief Allocate memory.
    /// @param size Size of bytes to be allocated.
    /// @return Allocated memory address.
    virtual void *require(int size) = 0;

    /// @brief Release memory that have been required.
    virtual void release(void) = 0;

    /// @brief Get payload address.
    /// @return Payload address.
    virtual void *payload(void) = 0;

    /// @brief Get payload size.
    /// @return Payload size in bytes.
    virtual int size(void) = 0;
};

#endif /* __AUST_BUFFER_H__ */
