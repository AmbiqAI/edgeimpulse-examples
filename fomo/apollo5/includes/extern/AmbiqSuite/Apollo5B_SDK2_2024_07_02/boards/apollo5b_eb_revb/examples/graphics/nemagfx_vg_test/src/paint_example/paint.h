//*****************************************************************************
//
//! @file paint.h
//!
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

#ifndef __PAINT_H__
#define __PAINT_H__

#include "nema_vg.h"

#ifdef __cplusplus
extern "C" {
#endif

#define COLOR_STOPS       5
#define STARS_IN_ONE_LINE 8
#define STAR_DIST_X       100
#define STAR_DIST_Y       120
#define SEG_SIZE_STAR     6
#define DATA_SIZE_STAR    12
#define REF_IMG_W         80
#define REF_IMG_H         100

float grad[STARS_IN_ONE_LINE][4] =
{
    {10.0f , 60.0f,  75.0f , 110.0f}, // right down
    {75.0f , 110.0f, 10.0f , 60.0f}, // right up
    {90.0f , 60.0f,  25.0f , 110.0f}, // left down
    {25.0f , 110.0f, 90.0f , 60.0f}, // left up
    {10.0f , 60.0f,  90.0f , 60.0f}, // right
    {90.0f , 60.0f,  10.0f , 60.0f}, // left
    {50.0f , 10.0f , 50.0f , 110.0f}, // down
    {50.0f , 110.0f, 50.0f , 10.0f}  // up
};

float data_star_small[]   = {10.0f, 60.0f,
                             90.0f, 60.0f,
                             25.0f, 110.0f,
                             50.0f, 10.0f,
                             75.0f, 110.0f,
                             10.0f, 60.0f};

uint8_t cmds_star[] = {NEMA_VG_PRIM_MOVE,
                       NEMA_VG_PRIM_LINE,
                       NEMA_VG_PRIM_LINE,
                       NEMA_VG_PRIM_LINE,
                       NEMA_VG_PRIM_LINE,
                       NEMA_VG_PRIM_LINE};

float stops[COLOR_STOPS] = {0.0f, 0.25f, 0.50f, 0.75f, 1.0f};
color_var_t colors[COLOR_STOPS] = {{0  , 0  , 0  , 255}, //black
                                   {255, 0  , 0  , 255}, //red
                                   {0  , 255, 0  , 255}, //green
                                   {0  , 0  , 255, 255}, //blue
                                   {255, 255, 255, 255}}; //white

#ifdef __cplusplus
}
#endif

#endif //__PAINT_H__
