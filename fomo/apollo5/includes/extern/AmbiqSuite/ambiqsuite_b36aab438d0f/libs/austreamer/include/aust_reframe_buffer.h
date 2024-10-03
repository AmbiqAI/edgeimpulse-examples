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

#ifndef __AUST_REFRAME_BUFFER_H__
#define __AUST_REFRAME_BUFFER_H__

#include "aust_buffer.h"
#include "aust_type.h"

/// @brief Reframe buffer internal structure.
typedef struct
{
    char    *buf;       ///< buffer address
    uint32_t len;       ///< the whole buffer length
    uint32_t out_size;  ///< output the length of the buffer
    uint32_t wr;        ///< write index
    uint32_t rd;        ///< read index
    uint32_t used_size; ///< the size used in buffer
} reframe_rb_t;

/// @brief ReframeBuffer is like a ring buffer, its payload size is fixed.
///
/// This fixed size is determined by its initial function @ref create, the first
/// parameter out_size will be the size, and buffer total size is
/// out_size*number, payload is not need to @ref require any more, just need to
/// @ref append instead.
class ReframeBuffer : public AustBuffer
{
  private:
    /// Reframe buffer.
    reframe_rb_t *rb_ = NULL;

    /// @brief Require memory. As memory require is replaced by @ref create, so
    /// no need to require again, and make it as private method.
    /// @param size NA.
    /// @return NA.
    void *require(int size) override;

  public:
    ReframeBuffer();
    ~ReframeBuffer();

    /// @brief Create a reframe buffer. Ring buffer size = out_size * number.
    /// @param out_size Reframe size.
    /// @param number Number of reframe size of the ring buffer.
    /// @return AUST_SUCCESS if create successfully, otherwise AUST_ERROR.
    AUST_ERR create(uint16_t out_size, uint16_t number);

    /// @brief Destroy reframe buffer.
    void destroy();

    /// @brief Used size in ring buffer.
    /// @return Used size.
    int getUsedSize();

    /// @brief Free size in ring buffer.
    /// @return Free size.
    int getFreeSize();

    /// @brief Append buffer data into ring buffer.
    /// @param buf Data pointer.
    /// @param in_size Data size.
    /// @return AUST_SUCCESS if appended into ring buffer, otherwise AUST_ERROR.
    AUST_ERR append(const char *buf, uint32_t in_size);

    /// @brief Release payload memory.
    void release(void) override;

    /// @brief Get the payload address.
    /// @return Payload address.
    void *payload(void) override;

    /// @brief Get the payload size.
    /// @return Payload size.
    int size(void) override;
};

#endif /* __AUST_REFRAME_BUFFER_H__ */
