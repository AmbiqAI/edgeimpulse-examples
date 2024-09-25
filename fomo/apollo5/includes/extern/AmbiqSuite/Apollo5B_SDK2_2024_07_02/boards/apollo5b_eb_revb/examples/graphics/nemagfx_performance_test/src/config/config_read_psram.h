//*****************************************************************************
//
//! @file config_write_psram.h
//!
//! @brief read psram performance .
//
//*****************************************************************************
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
// This is part of revision release_a5b_sdk2-748191cd0 of the AmbiqSuite Development Package.
//
//*****************************************************************************
#ifndef CONFIG_READ_PSRAM_H
#define CONFIG_READ_PSRAM_H

#include "nemagfx_performance_test.h"


//*****************************************************************************
//
// Test item list
//
//*****************************************************************************
const TestItem_t g_sTestItemList[] =
{
    //!<TestType,         TEX,      FB,      TexFormat,     FBFormat,     DCFormat,     texture/font, Src Burst size,    Des Burst size,    SamplingMode, SrcWidth,SrcHight,DesWidth,DesHight,32Angle,tiling,morton
    {TEST_UNKNOWN,      LT_PSRAM, LT_SSRAM, NEMA_RGBA8888, NEMA_RGB24,    NEMADC_RGB24,   ST_NO_USED, GPU_MSPI_BURST_SIZE_64, GPU_MSPI_BURST_SIZE_64, TEST_NA,        466, 466, 466, 466, 0,     false,  false},

    {TEST_COPY,         LT_PSRAM, LT_SSRAM, NEMA_RGBA8888, NEMA_RGB24,    NEMADC_RGB24,   ST_TEXTURE, GPU_MSPI_BURST_SIZE_64, GPU_MSPI_BURST_SIZE_64, NEMA_FILTER_PS,  466, 466, 466, 466, 0,    false,  false},
    {TEST_COPY,         LT_PSRAM, LT_SSRAM, NEMA_RGB24,    NEMA_RGB24,    NEMADC_RGB24,   ST_TEXTURE, GPU_MSPI_BURST_SIZE_64, GPU_MSPI_BURST_SIZE_64, NEMA_FILTER_PS,  466, 466, 466, 466, 0,    false,  false},
    {TEST_COPY,         LT_PSRAM, LT_SSRAM, NEMA_RGB565,   NEMA_RGB24,    NEMADC_RGB24,   ST_TEXTURE, GPU_MSPI_BURST_SIZE_64, GPU_MSPI_BURST_SIZE_64, NEMA_FILTER_PS,  466, 466, 466, 466, 0,    false,  false},

};



#endif
