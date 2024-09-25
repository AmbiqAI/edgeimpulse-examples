//*****************************************************************************
//
//! @file config_morton.h
//!
//! @brief explore the side effect of morton and tiling to normal blit
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
#ifndef CONFIG_MORTON_H
#define CONFIG_MORTON_H

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
    //!< The first test was used to warm up the GPU, its result will not be print, leave it alone.
    {TEST_COPY,         LT_PSRAM, LT_SSRAM, NEMA_RGBA8888, NEMA_RGB24,    NEMADC_RGB24,   ST_TEXTURE, GPU_MSPI_BURST_SIZE_64, GPU_MSPI_BURST_SIZE_64, NEMA_FILTER_PS,  400, 400, 400, 400, 0,    false,  false},
    {TEST_COPY,         LT_PSRAM, LT_SSRAM, NEMA_RGBA8888, NEMA_RGB24,    NEMADC_RGB24,   ST_TEXTURE, GPU_MSPI_BURST_SIZE_64, GPU_MSPI_BURST_SIZE_64, NEMA_FILTER_PS,  400, 400, 400, 400, 0,    false,  true},
    {TEST_COPY,         LT_PSRAM, LT_SSRAM, NEMA_RGBA8888, NEMA_RGB24,    NEMADC_RGB24,   ST_TEXTURE, GPU_MSPI_BURST_SIZE_64, GPU_MSPI_BURST_SIZE_64, NEMA_FILTER_PS,  400, 400, 400, 400, 0,    true,  false},
    {TEST_COPY,         LT_PSRAM, LT_SSRAM, NEMA_RGBA8888, NEMA_RGB24,    NEMADC_RGB24,   ST_TEXTURE, GPU_MSPI_BURST_SIZE_64, GPU_MSPI_BURST_SIZE_64, NEMA_FILTER_PS,  400, 400, 400, 400, 0,    true,  true},

    {TEST_ROTATE,       LT_PSRAM, LT_SSRAM, NEMA_RGBA8888, NEMA_RGB24,    NEMADC_RGB24,   ST_TEXTURE, GPU_MSPI_BURST_SIZE_64, GPU_MSPI_BURST_SIZE_64, NEMA_FILTER_BL,  400, 400, 400, 400, 5,    false,   false},
    {TEST_ROTATE,       LT_PSRAM, LT_SSRAM, NEMA_RGBA8888, NEMA_RGB24,    NEMADC_RGB24,   ST_TEXTURE, GPU_MSPI_BURST_SIZE_64, GPU_MSPI_BURST_SIZE_64, NEMA_FILTER_BL,  400, 400, 400, 400, 15,   false,   false},
    {TEST_ROTATE,       LT_PSRAM, LT_SSRAM, NEMA_RGBA8888, NEMA_RGB24,    NEMADC_RGB24,   ST_TEXTURE, GPU_MSPI_BURST_SIZE_64, GPU_MSPI_BURST_SIZE_64, NEMA_FILTER_BL,  400, 400, 400, 400, 25,   false,   false},
    {TEST_ROTATE,       LT_PSRAM, LT_SSRAM, NEMA_RGBA8888, NEMA_RGB24,    NEMADC_RGB24,   ST_TEXTURE, GPU_MSPI_BURST_SIZE_64, GPU_MSPI_BURST_SIZE_64, NEMA_FILTER_BL,  400, 400, 400, 400, 35,   false,   false},
    {TEST_ROTATE,       LT_PSRAM, LT_SSRAM, NEMA_RGBA8888, NEMA_RGB24,    NEMADC_RGB24,   ST_TEXTURE, GPU_MSPI_BURST_SIZE_64, GPU_MSPI_BURST_SIZE_64, NEMA_FILTER_BL,  400, 400, 400, 400, 45,   false,   false},
    {TEST_ROTATE,       LT_PSRAM, LT_SSRAM, NEMA_RGBA8888, NEMA_RGB24,    NEMADC_RGB24,   ST_TEXTURE, GPU_MSPI_BURST_SIZE_64, GPU_MSPI_BURST_SIZE_64, NEMA_FILTER_BL,  400, 400, 400, 400, 55,   false,   false},
    {TEST_ROTATE,       LT_PSRAM, LT_SSRAM, NEMA_RGBA8888, NEMA_RGB24,    NEMADC_RGB24,   ST_TEXTURE, GPU_MSPI_BURST_SIZE_64, GPU_MSPI_BURST_SIZE_64, NEMA_FILTER_BL,  400, 400, 400, 400, 65,   false,   false},
    {TEST_ROTATE,       LT_PSRAM, LT_SSRAM, NEMA_RGBA8888, NEMA_RGB24,    NEMADC_RGB24,   ST_TEXTURE, GPU_MSPI_BURST_SIZE_64, GPU_MSPI_BURST_SIZE_64, NEMA_FILTER_BL,  400, 400, 400, 400, 75,   false,   false},
    {TEST_ROTATE,       LT_PSRAM, LT_SSRAM, NEMA_RGBA8888, NEMA_RGB24,    NEMADC_RGB24,   ST_TEXTURE, GPU_MSPI_BURST_SIZE_64, GPU_MSPI_BURST_SIZE_64, NEMA_FILTER_BL,  400, 400, 400, 400, 85,   false,   false},

    {TEST_ROTATE,       LT_PSRAM, LT_SSRAM, NEMA_RGBA8888, NEMA_RGB24,    NEMADC_RGB24,   ST_TEXTURE, GPU_MSPI_BURST_SIZE_64, GPU_MSPI_BURST_SIZE_64, NEMA_FILTER_BL,  400, 400, 400, 400, 5,    false,   true},
    {TEST_ROTATE,       LT_PSRAM, LT_SSRAM, NEMA_RGBA8888, NEMA_RGB24,    NEMADC_RGB24,   ST_TEXTURE, GPU_MSPI_BURST_SIZE_64, GPU_MSPI_BURST_SIZE_64, NEMA_FILTER_BL,  400, 400, 400, 400, 15,   false,   true},
    {TEST_ROTATE,       LT_PSRAM, LT_SSRAM, NEMA_RGBA8888, NEMA_RGB24,    NEMADC_RGB24,   ST_TEXTURE, GPU_MSPI_BURST_SIZE_64, GPU_MSPI_BURST_SIZE_64, NEMA_FILTER_BL,  400, 400, 400, 400, 25,   false,   true},
    {TEST_ROTATE,       LT_PSRAM, LT_SSRAM, NEMA_RGBA8888, NEMA_RGB24,    NEMADC_RGB24,   ST_TEXTURE, GPU_MSPI_BURST_SIZE_64, GPU_MSPI_BURST_SIZE_64, NEMA_FILTER_BL,  400, 400, 400, 400, 35,   false,   true},
    {TEST_ROTATE,       LT_PSRAM, LT_SSRAM, NEMA_RGBA8888, NEMA_RGB24,    NEMADC_RGB24,   ST_TEXTURE, GPU_MSPI_BURST_SIZE_64, GPU_MSPI_BURST_SIZE_64, NEMA_FILTER_BL,  400, 400, 400, 400, 45,   false,   true},
    {TEST_ROTATE,       LT_PSRAM, LT_SSRAM, NEMA_RGBA8888, NEMA_RGB24,    NEMADC_RGB24,   ST_TEXTURE, GPU_MSPI_BURST_SIZE_64, GPU_MSPI_BURST_SIZE_64, NEMA_FILTER_BL,  400, 400, 400, 400, 55,   false,   true},
    {TEST_ROTATE,       LT_PSRAM, LT_SSRAM, NEMA_RGBA8888, NEMA_RGB24,    NEMADC_RGB24,   ST_TEXTURE, GPU_MSPI_BURST_SIZE_64, GPU_MSPI_BURST_SIZE_64, NEMA_FILTER_BL,  400, 400, 400, 400, 65,   false,   true},
    {TEST_ROTATE,       LT_PSRAM, LT_SSRAM, NEMA_RGBA8888, NEMA_RGB24,    NEMADC_RGB24,   ST_TEXTURE, GPU_MSPI_BURST_SIZE_64, GPU_MSPI_BURST_SIZE_64, NEMA_FILTER_BL,  400, 400, 400, 400, 75,   false,   true},
    {TEST_ROTATE,       LT_PSRAM, LT_SSRAM, NEMA_RGBA8888, NEMA_RGB24,    NEMADC_RGB24,   ST_TEXTURE, GPU_MSPI_BURST_SIZE_64, GPU_MSPI_BURST_SIZE_64, NEMA_FILTER_BL,  400, 400, 400, 400, 85,   false,   true},

    {TEST_ROTATE,       LT_PSRAM, LT_SSRAM, NEMA_RGBA8888, NEMA_RGB24,    NEMADC_RGB24,   ST_TEXTURE, GPU_MSPI_BURST_SIZE_64, GPU_MSPI_BURST_SIZE_64, NEMA_FILTER_BL,  400, 400, 400, 400, 5,    true,   false},
    {TEST_ROTATE,       LT_PSRAM, LT_SSRAM, NEMA_RGBA8888, NEMA_RGB24,    NEMADC_RGB24,   ST_TEXTURE, GPU_MSPI_BURST_SIZE_64, GPU_MSPI_BURST_SIZE_64, NEMA_FILTER_BL,  400, 400, 400, 400, 15,   true,   false},
    {TEST_ROTATE,       LT_PSRAM, LT_SSRAM, NEMA_RGBA8888, NEMA_RGB24,    NEMADC_RGB24,   ST_TEXTURE, GPU_MSPI_BURST_SIZE_64, GPU_MSPI_BURST_SIZE_64, NEMA_FILTER_BL,  400, 400, 400, 400, 25,   true,   false},
    {TEST_ROTATE,       LT_PSRAM, LT_SSRAM, NEMA_RGBA8888, NEMA_RGB24,    NEMADC_RGB24,   ST_TEXTURE, GPU_MSPI_BURST_SIZE_64, GPU_MSPI_BURST_SIZE_64, NEMA_FILTER_BL,  400, 400, 400, 400, 35,   true,   false},
    {TEST_ROTATE,       LT_PSRAM, LT_SSRAM, NEMA_RGBA8888, NEMA_RGB24,    NEMADC_RGB24,   ST_TEXTURE, GPU_MSPI_BURST_SIZE_64, GPU_MSPI_BURST_SIZE_64, NEMA_FILTER_BL,  400, 400, 400, 400, 45,   true,   false},
    {TEST_ROTATE,       LT_PSRAM, LT_SSRAM, NEMA_RGBA8888, NEMA_RGB24,    NEMADC_RGB24,   ST_TEXTURE, GPU_MSPI_BURST_SIZE_64, GPU_MSPI_BURST_SIZE_64, NEMA_FILTER_BL,  400, 400, 400, 400, 55,   true,   false},
    {TEST_ROTATE,       LT_PSRAM, LT_SSRAM, NEMA_RGBA8888, NEMA_RGB24,    NEMADC_RGB24,   ST_TEXTURE, GPU_MSPI_BURST_SIZE_64, GPU_MSPI_BURST_SIZE_64, NEMA_FILTER_BL,  400, 400, 400, 400, 65,   true,   false},
    {TEST_ROTATE,       LT_PSRAM, LT_SSRAM, NEMA_RGBA8888, NEMA_RGB24,    NEMADC_RGB24,   ST_TEXTURE, GPU_MSPI_BURST_SIZE_64, GPU_MSPI_BURST_SIZE_64, NEMA_FILTER_BL,  400, 400, 400, 400, 75,   true,   false},
    {TEST_ROTATE,       LT_PSRAM, LT_SSRAM, NEMA_RGBA8888, NEMA_RGB24,    NEMADC_RGB24,   ST_TEXTURE, GPU_MSPI_BURST_SIZE_64, GPU_MSPI_BURST_SIZE_64, NEMA_FILTER_BL,  400, 400, 400, 400, 85,   true,   false},

    {TEST_ROTATE,       LT_PSRAM, LT_SSRAM, NEMA_RGBA8888, NEMA_RGB24,    NEMADC_RGB24,   ST_TEXTURE, GPU_MSPI_BURST_SIZE_64, GPU_MSPI_BURST_SIZE_64, NEMA_FILTER_BL,  400, 400, 400, 400, 5,    true,   true},
    {TEST_ROTATE,       LT_PSRAM, LT_SSRAM, NEMA_RGBA8888, NEMA_RGB24,    NEMADC_RGB24,   ST_TEXTURE, GPU_MSPI_BURST_SIZE_64, GPU_MSPI_BURST_SIZE_64, NEMA_FILTER_BL,  400, 400, 400, 400, 15,   true,   true},
    {TEST_ROTATE,       LT_PSRAM, LT_SSRAM, NEMA_RGBA8888, NEMA_RGB24,    NEMADC_RGB24,   ST_TEXTURE, GPU_MSPI_BURST_SIZE_64, GPU_MSPI_BURST_SIZE_64, NEMA_FILTER_BL,  400, 400, 400, 400, 25,   true,   true},
    {TEST_ROTATE,       LT_PSRAM, LT_SSRAM, NEMA_RGBA8888, NEMA_RGB24,    NEMADC_RGB24,   ST_TEXTURE, GPU_MSPI_BURST_SIZE_64, GPU_MSPI_BURST_SIZE_64, NEMA_FILTER_BL,  400, 400, 400, 400, 35,   true,   true},
    {TEST_ROTATE,       LT_PSRAM, LT_SSRAM, NEMA_RGBA8888, NEMA_RGB24,    NEMADC_RGB24,   ST_TEXTURE, GPU_MSPI_BURST_SIZE_64, GPU_MSPI_BURST_SIZE_64, NEMA_FILTER_BL,  400, 400, 400, 400, 45,   true,   true},
    {TEST_ROTATE,       LT_PSRAM, LT_SSRAM, NEMA_RGBA8888, NEMA_RGB24,    NEMADC_RGB24,   ST_TEXTURE, GPU_MSPI_BURST_SIZE_64, GPU_MSPI_BURST_SIZE_64, NEMA_FILTER_BL,  400, 400, 400, 400, 55,   true,   true},
    {TEST_ROTATE,       LT_PSRAM, LT_SSRAM, NEMA_RGBA8888, NEMA_RGB24,    NEMADC_RGB24,   ST_TEXTURE, GPU_MSPI_BURST_SIZE_64, GPU_MSPI_BURST_SIZE_64, NEMA_FILTER_BL,  400, 400, 400, 400, 65,   true,   true},
    {TEST_ROTATE,       LT_PSRAM, LT_SSRAM, NEMA_RGBA8888, NEMA_RGB24,    NEMADC_RGB24,   ST_TEXTURE, GPU_MSPI_BURST_SIZE_64, GPU_MSPI_BURST_SIZE_64, NEMA_FILTER_BL,  400, 400, 400, 400, 75,   true,   true},
    {TEST_ROTATE,       LT_PSRAM, LT_SSRAM, NEMA_RGBA8888, NEMA_RGB24,    NEMADC_RGB24,   ST_TEXTURE, GPU_MSPI_BURST_SIZE_64, GPU_MSPI_BURST_SIZE_64, NEMA_FILTER_BL,  400, 400, 400, 400, 85,   true,   true},
};



#endif
