//*****************************************************************************
//
//! @file ume_ugo5_ttf.c
//!
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

#ifndef UME_UGO5_TTF_C
#define UME_UGO5_TTF_C

#include "ume_ugo5_ttf.h"
#include "nema_vg_context.h"

static const nema_vg_float_t ume_ugo5_ttf_data[] =
{
  // 0x9f8d - 0x9f8d
  2576.00f, 3072.00f, 3840.00f, 3072.00f, 3840.00f, 2752.00f, 2576.00f, 2752.00f, 2576.00f, 2560.00f, 3712.00f, 2560.00f,
  3712.00f, 1696.00f, 2576.00f, 1696.00f, 2576.00f, 1536.00f, 3744.00f, 1536.00f, 3744.00f, 1216.00f, 2576.00f, 1216.00f,
  2576.00f, 1056.00f, 3744.00f, 1056.00f, 3744.00f, 736.00f, 2576.00f, 736.00f, 2576.00f, 576.00f, 3744.00f, 576.00f,
  3744.00f, 256.00f, 2576.00f, 256.00f, 2576.00f, 64.00f, 2576.00f, 0.00f, 2640.00f, 0.00f, 3440.00f, 0.00f,
  3552.00f, 0.00f, 3568.00f, 176.00f, 3936.00f, 48.00f, 3856.00f, -320.00f, 3488.00f, -320.00f, 2576.00f, -320.00f,
  2224.00f, -320.00f, 2224.00f, 64.00f, 2224.00f, 2016.00f, 3360.00f, 2016.00f, 3360.00f, 2240.00f, 2224.00f, 2240.00f,
  2224.00f, 3392.00f, 2576.00f, 3392.00f, 2576.00f, 3072.00f, 768.00f, 384.00f, 768.00f, -320.00f, 416.00f, -320.00f,
  416.00f, 1696.00f, 1904.00f, 1696.00f, 1904.00f, 64.00f, 1904.00f, -320.00f, 1568.00f, -320.00f, 1168.00f, -320.00f,
  1088.00f, 0.00f, 1472.00f, 0.00f, 1552.00f, 0.00f, 1552.00f, 64.00f, 1552.00f, 384.00f, 768.00f, 384.00f,
  1552.00f, 704.00f, 1552.00f, 880.00f, 768.00f, 880.00f, 768.00f, 704.00f, 1552.00f, 704.00f, 1552.00f, 1200.00f,
  1552.00f, 1376.00f, 768.00f, 1376.00f, 768.00f, 1200.00f, 1552.00f, 1200.00f, 992.00f, 2944.00f, 992.00f, 3392.00f,
  1344.00f, 3392.00f, 1344.00f, 2944.00f, 2080.00f, 2944.00f, 2080.00f, 2624.00f, 1792.00f, 2624.00f, 1744.00f, 2384.00f,
  1680.00f, 2176.00f, 2144.00f, 2176.00f, 2144.00f, 1856.00f, 208.00f, 1856.00f, 208.00f, 2176.00f, 672.00f, 2176.00f,
  672.00f, 2368.00f, 576.00f, 2624.00f, 288.00f, 2624.00f, 288.00f, 2944.00f, 992.00f, 2944.00f, 1024.00f, 2176.00f,
  1344.00f, 2176.00f, 1424.00f, 2400.00f, 1440.00f, 2624.00f, 912.00f, 2624.00f, 976.00f, 2384.00f, 1024.00f, 2176.00f
};

static const uint8_t ume_ugo5_ttf_segments[] =
{
  // 0x9f8d - 0x9f8d
  1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
  2, 2, 2, 2, 2, 2, 2, 2, 2, 5, 2, 5,
  2, 5, 2, 5, 2, 2, 2, 2, 2, 2, 2, 1,
  2, 2, 2, 2, 2, 5, 2, 2, 2, 5, 2, 2,
  1, 2, 2, 2, 2, 1, 2, 2, 2, 2, 1, 2,
  2, 2, 2, 2, 2, 5, 2, 2, 2, 2, 2, 5,
  2, 2, 2, 1, 2, 5, 2, 5
};

static const nema_vg_glyph_t ume_ugo5_ttfGlyphs0[] =
{
  {      0,    180,      0,     80,  4096.00f,    0,    0,    208,   -320,   3936,   3392},   // 0x00009F8D

};

static const nema_vg_font_range_t ume_ugo5_ttf_ranges[] =
{
  {0x00009f8d, 0x00009f8d, ume_ugo5_ttfGlyphs0},
  {0, 0, NULL}
};

nema_vg_font_t ume_ugo5_ttf =
{
    1,
    ume_ugo5_ttf_ranges,
    ume_ugo5_ttf_data,
    180,
    ume_ugo5_ttf_segments,
    80,
    4096.00f, //size
    4096.00f, //xAdvance
    3520.00f, //ascender
    -576.00f, //descender
    NULL,
    0x0
};

#endif //UME_UGO5_TTF_C
