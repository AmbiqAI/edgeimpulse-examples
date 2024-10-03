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

#ifndef __AUST_AUDIO_COMMON_H__
#define __AUST_AUDIO_COMMON_H__

/// @brief Audio codec type.
enum AudioCodec
{
    AUDIO_CODEC_SBC,  ///< SBC
    AUDIO_CODEC_MSBC, ///< mSBC
    AUDIO_CODEC_CVSD, ///< CVSD
    AUDIO_CODEC_LC3,  ///< Lc3
    AUDIO_CODEC_MP3,  ///< MP3
    AUDIO_CODEC_AAC,  ///< AAC
    AUDIO_CODEC_FLAC, ///< Flac
    AUDIO_CODEC_ALAC, ///< Alac
    AUDIO_CODEC_OGG,  ///< Ogg
    AUDIO_CODEC_WAV,  ///< Wav
    AUDIO_CODEC_AIFF, ///< Aiff
};

/// @brief Audio channel mode.
enum ChannelMode
{
    MONO         = 0, ///< Mono
    DUAL_CHANNEL = 1, ///< Dual channel
    STEREO       = 2, ///< Stereo
    JOINT_STEREO = 3, ///< Joint stereo
};

/// @brief Audio sample rate.
enum SampleRate
{
    RATE_8000    = 8000,       ///< 8KHz
    RATE_16000   = 16000,      ///< 16KHz
    RATE_24000   = 24000,      ///< 24KHz
    RATE_32000   = 32000,      ///< 32KHz
    RATE_44100   = 44100,      ///< 44.1KHz
    RATE_48000   = 48000,      ///< 48KHz
    DEFAULT_RATE = RATE_44100, ///< Default sample rate set as 44.1KHz
};

/// Bluetooth SBC packet header length.
#define SBC_PACKET_HEADER_BYTE 1

#endif /* __AUST_AUDIO_COMMON_H__ */
