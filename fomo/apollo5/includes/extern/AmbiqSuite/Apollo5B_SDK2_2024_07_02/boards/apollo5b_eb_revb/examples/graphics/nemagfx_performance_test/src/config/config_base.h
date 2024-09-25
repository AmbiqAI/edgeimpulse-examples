//*****************************************************************************
//
//! @file condif_base.h
//!
//! @brief Baseline test config.
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
#ifndef CONFIG_BASE_H
#define CONFIG_BASE_H

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
    {TEST_FILL,         LT_NO_USED, LT_SSRAM, TEST_NA,     NEMA_RGB24,    NEMADC_RGB24,   ST_NO_USED, GPU_MSPI_BURST_SIZE_64, GPU_MSPI_BURST_SIZE_64, TEST_NA,           0,  0, 466, 466, 0,     false,  false},
    {TEST_COPY,         LT_SSRAM, LT_SSRAM, NEMA_RGB24,    NEMA_RGB24,    NEMADC_RGB24,   ST_TEXTURE, GPU_MSPI_BURST_SIZE_64, GPU_MSPI_BURST_SIZE_64, NEMA_FILTER_PS,  466, 466, 466, 466, 0,    false,  false},
    {TEST_COPY,         LT_SSRAM, LT_SSRAM, NEMA_RGB565,   NEMA_RGB24,    NEMADC_RGB24,   ST_TEXTURE, GPU_MSPI_BURST_SIZE_64, GPU_MSPI_BURST_SIZE_64, NEMA_FILTER_PS,  466, 466, 466, 466, 0,    false,  false},
    {TEST_BLEND,        LT_SSRAM, LT_SSRAM, NEMA_RGBA8888, NEMA_RGB24,    NEMADC_RGB24,   ST_TEXTURE, GPU_MSPI_BURST_SIZE_64, GPU_MSPI_BURST_SIZE_64, NEMA_FILTER_PS,  466, 466, 466, 466, 0,    false,  false},
    {TEST_BLEND_FONT,   LT_SSRAM, LT_SSRAM, NEMA_A4,       NEMA_RGB24,    NEMADC_RGB24,   ST_FONT,    GPU_MSPI_BURST_SIZE_64, GPU_MSPI_BURST_SIZE_64, TEST_NA,          40,  40,  40,  40, 0,    false,  false},
    {TEST_VG_FONT,      LT_SSRAM, LT_SSRAM, TEST_NA,       NEMA_RGB24,    NEMADC_RGB24,   ST_VG_FONT, GPU_MSPI_BURST_SIZE_64, GPU_MSPI_BURST_SIZE_64, NEMA_FILTER_PS,  0, 0, 40, 40, 0,          false,  false},
    {TEST_VG,           LT_SSRAM, LT_SSRAM, TEST_NA,       NEMA_RGB24,    NEMADC_RGB24,   ST_VG_SVG,  GPU_MSPI_BURST_SIZE_64, GPU_MSPI_BURST_SIZE_64, NEMA_FILTER_PS,  0, 0, 466, 466, 0,        false,  false},
    {TEST_COPY,         LT_PSRAM, LT_SSRAM, NEMA_RGB24,    NEMA_RGB24,    NEMADC_RGB24,   ST_TEXTURE, GPU_MSPI_BURST_SIZE_64, GPU_MSPI_BURST_SIZE_64, NEMA_FILTER_PS,  466, 466, 466, 466, 0,    false,  false},
    {TEST_COPY,         LT_PSRAM, LT_SSRAM, NEMA_RGB565,   NEMA_RGB24,    NEMADC_RGB24,   ST_TEXTURE, GPU_MSPI_BURST_SIZE_64, GPU_MSPI_BURST_SIZE_64, NEMA_FILTER_PS,  466, 466, 466, 466, 0,    false,  false},
    {TEST_BLEND,        LT_PSRAM, LT_SSRAM, NEMA_RGBA8888, NEMA_RGB24,    NEMADC_RGB24,   ST_TEXTURE, GPU_MSPI_BURST_SIZE_64, GPU_MSPI_BURST_SIZE_64, NEMA_FILTER_PS,  466, 466, 466, 466, 0,    false,  false},
    {TEST_BLEND_PART,   LT_PSRAM, LT_SSRAM, NEMA_RGBA8888, NEMA_RGB24,    NEMADC_RGB24,   ST_TEXTURE, GPU_MSPI_BURST_SIZE_64, GPU_MSPI_BURST_SIZE_64, NEMA_FILTER_PS,  466, 466, 466, 466, 0,    false,  false},
    {TEST_BLEND_FONT,   LT_PSRAM, LT_SSRAM, NEMA_A8,       NEMA_RGB24,    NEMADC_RGB24,   ST_FONT,    GPU_MSPI_BURST_SIZE_64, GPU_MSPI_BURST_SIZE_64, TEST_NA,          36,  36,  36,  36, 0,    false,  false},
    {TEST_BLEND_FONT,   LT_PSRAM, LT_SSRAM, NEMA_A4,       NEMA_RGB24,    NEMADC_RGB24,   ST_FONT,    GPU_MSPI_BURST_SIZE_64, GPU_MSPI_BURST_SIZE_64, TEST_NA,          36,  36,  36,  36, 0,    false,  false},
    {TEST_BLEND_GLB_A,  LT_PSRAM, LT_SSRAM, NEMA_RGB24,    NEMA_RGB24,    NEMADC_RGB24,   ST_TEXTURE, GPU_MSPI_BURST_SIZE_64, GPU_MSPI_BURST_SIZE_64, NEMA_FILTER_PS,  466, 466, 466, 466, 0,    false,  false},
    {TEST_SCALE,        LT_PSRAM, LT_SSRAM, NEMA_RGBA8888, NEMA_RGB24,    NEMADC_RGB24,   ST_TEXTURE, GPU_MSPI_BURST_SIZE_64, GPU_MSPI_BURST_SIZE_64, NEMA_FILTER_PS,  466, 466, 326, 326, 0,    false,  false},
    {TEST_SCALE,        LT_PSRAM, LT_SSRAM, NEMA_RGBA8888, NEMA_RGB24,    NEMADC_RGB24,   ST_TEXTURE, GPU_MSPI_BURST_SIZE_64, GPU_MSPI_BURST_SIZE_64, NEMA_FILTER_BL,  466, 466, 326, 326, 0,    false,  false},
    {TEST_SCALE,        LT_PSRAM, LT_SSRAM, NEMA_RGB24,    NEMA_RGB24,    NEMADC_RGB24,   ST_TEXTURE, GPU_MSPI_BURST_SIZE_64, GPU_MSPI_BURST_SIZE_64, NEMA_FILTER_BL,  466, 466, 326, 326, 0,    false,  false},
    {TEST_SCALE,        LT_PSRAM, LT_SSRAM, NEMA_RGBA8888, NEMA_RGB24,    NEMADC_RGB24,   ST_TEXTURE, GPU_MSPI_BURST_SIZE_64, GPU_MSPI_BURST_SIZE_64, NEMA_FILTER_PS,  466, 466, 792, 792, 0,    false,  false},
    {TEST_SCALE,        LT_PSRAM, LT_SSRAM, NEMA_RGBA8888, NEMA_RGB24,    NEMADC_RGB24,   ST_TEXTURE, GPU_MSPI_BURST_SIZE_64, GPU_MSPI_BURST_SIZE_64, NEMA_FILTER_BL,  466, 466, 792, 792, 0,    true,   false},
    {TEST_SCALE,        LT_PSRAM, LT_SSRAM, NEMA_RGB24,    NEMA_RGB24,    NEMADC_RGB24,   ST_TEXTURE, GPU_MSPI_BURST_SIZE_64, GPU_MSPI_BURST_SIZE_64, NEMA_FILTER_BL,  466, 466, 792, 792, 0,    true,   false},
    {TEST_ROTATE,       LT_PSRAM, LT_SSRAM, NEMA_RGBA8888, NEMA_RGB24,    NEMADC_RGB24,   ST_TEXTURE, GPU_MSPI_BURST_SIZE_64, GPU_MSPI_BURST_SIZE_64, NEMA_FILTER_PS,  466, 466, 466, 466, 89,   true,   true},
    {TEST_ROTATE,       LT_PSRAM, LT_SSRAM, NEMA_RGB24,    NEMA_RGB24,    NEMADC_RGB24,   ST_TEXTURE, GPU_MSPI_BURST_SIZE_64, GPU_MSPI_BURST_SIZE_64, NEMA_FILTER_PS,  466, 466, 466, 466, 89,   true,   true},
    {TEST_ROTATE,       LT_PSRAM, LT_SSRAM, NEMA_RGBA8888, NEMA_RGB24,    NEMADC_RGB24,   ST_TEXTURE, GPU_MSPI_BURST_SIZE_64, GPU_MSPI_BURST_SIZE_64, NEMA_FILTER_BL,  466, 466, 466, 466, 89,   true,   true},
    {TEST_ROTATE,       LT_PSRAM, LT_SSRAM, NEMA_RGBA8888, NEMA_RGB24,    NEMADC_RGB24,   ST_TEXTURE, GPU_MSPI_BURST_SIZE_64, GPU_MSPI_BURST_SIZE_64, NEMA_FILTER_PS,  466, 466, 466, 466, 90,   true,   true},
    {TEST_ROTATE,       LT_PSRAM, LT_SSRAM, NEMA_RGBA8888, NEMA_RGB24,    NEMADC_RGB24,   ST_TEXTURE, GPU_MSPI_BURST_SIZE_64, GPU_MSPI_BURST_SIZE_64, NEMA_FILTER_PS,  466, 466, 466, 466, 180,  false,  false},
    {TEST_ROTATE,       LT_PSRAM, LT_SSRAM, NEMA_RGBA8888, NEMA_RGB24,    NEMADC_RGB24,   ST_TEXTURE, GPU_MSPI_BURST_SIZE_64, GPU_MSPI_BURST_SIZE_64, NEMA_FILTER_BL,   20, 250, 466, 466, 89,   true,   true},
    {TEST_GAUSSIAN_BLUR, LT_PSRAM, LT_SSRAM, NEMA_RGB24,   NEMA_RGB24,    NEMADC_RGB24,   ST_TEXTURE, GPU_MSPI_BURST_SIZE_64, GPU_MSPI_BURST_SIZE_64, NEMA_FILTER_PS,  466, 466, 466, 466, 3,    false,  false},
    {TEST_GAUSSIAN_BLUR, LT_PSRAM, LT_SSRAM, NEMA_RGB24,   NEMA_RGB24,    NEMADC_RGB24,   ST_TEXTURE, GPU_MSPI_BURST_SIZE_64, GPU_MSPI_BURST_SIZE_64, NEMA_FILTER_PS,  466, 466, 466, 466, 10,   false,  false},
    {TEST_PERSPECTIVE,  LT_PSRAM, LT_SSRAM, NEMA_RGBA8888, NEMA_RGB24,    NEMADC_RGB24,   ST_TEXTURE, GPU_MSPI_BURST_SIZE_64, GPU_MSPI_BURST_SIZE_64, NEMA_FILTER_BL,  466, 466, 466, 466, 180,  true,   true},
    {TEST_VG_FONT,      LT_PSRAM, LT_SSRAM, TEST_NA,       NEMA_RGB24,    NEMADC_RGB24,   ST_VG_FONT, GPU_MSPI_BURST_SIZE_64, GPU_MSPI_BURST_SIZE_64, NEMA_FILTER_PS,  0, 0, 50, 50, 0,          false,  false},
    {TEST_VG,           LT_PSRAM, LT_SSRAM, TEST_NA,       NEMA_RGB24,    NEMADC_RGB24,   ST_VG_SVG,  GPU_MSPI_BURST_SIZE_64, GPU_MSPI_BURST_SIZE_64, NEMA_FILTER_PS,  0, 0, 400, 400, 0,        false,  false},
    {TEST_VG,           LT_PSRAM, LT_SSRAM, TEST_NA,       NEMA_RGB24,    NEMADC_RGB24,   ST_VG_SVG,  GPU_MSPI_BURST_SIZE_64, GPU_MSPI_BURST_SIZE_64, NEMA_FILTER_PS,  0, 0, 400, 400, 89,       false,  false},
    {TEST_COPY,         LT_SSRAM, LT_PSRAM, NEMA_RGB24,    NEMA_RGB24,    NEMADC_RGB24,   ST_TEXTURE, GPU_MSPI_BURST_SIZE_64, GPU_MSPI_BURST_SIZE_64, NEMA_FILTER_PS,  466, 466, 466, 466, 0,    false,  false},
    //Decode
    {TEST_COPY,         LT_SSRAM, LT_PSRAM, NEMA_TSC6,     NEMA_RGB24,    NEMADC_RGB24,   ST_TEXTURE, GPU_MSPI_BURST_SIZE_64, GPU_MSPI_BURST_SIZE_64, NEMA_FILTER_PS,  466, 466, 466, 466, 0,    false,  false},
    {TEST_COPY,         LT_PSRAM, LT_PSRAM, NEMA_TSC6,     NEMA_RGB24,    NEMADC_RGB24,   ST_TEXTURE, GPU_MSPI_BURST_SIZE_64, GPU_MSPI_BURST_SIZE_64, NEMA_FILTER_PS,  466, 466, 466, 466, 0,    false,  false},
};



#endif
