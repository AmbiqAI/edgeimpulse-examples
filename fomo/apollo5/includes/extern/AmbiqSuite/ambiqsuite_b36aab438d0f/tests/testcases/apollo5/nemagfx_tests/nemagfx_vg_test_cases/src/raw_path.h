//*****************************************************************************
//
//! @file paint.h
//!
//*****************************************************************************

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
// This is part of revision stable-35bd4d2434 of the AmbiqSuite Development Package.
//
//*****************************************************************************

#ifndef __RAW_PATH_H__
#define __RAW_PATH_H__

#include "nema_vg.h"

#define NUM_SEGS_H 14
#define NUM_COORDS_H 26

const unsigned char segs_H[NUM_SEGS_H] = {NEMA_VG_PRIM_MOVE,
                      NEMA_VG_PRIM_LINE,
                      NEMA_VG_PRIM_LINE,
                      NEMA_VG_PRIM_LINE,
                      NEMA_VG_PRIM_LINE,
                      NEMA_VG_PRIM_LINE,
                      NEMA_VG_PRIM_LINE,
                      NEMA_VG_PRIM_LINE,
                      NEMA_VG_PRIM_LINE,
                      NEMA_VG_PRIM_LINE,
                      NEMA_VG_PRIM_LINE,
                      NEMA_VG_PRIM_LINE,
                      NEMA_VG_PRIM_LINE,
                      NEMA_VG_PRIM_CLOSE};
const float points_H[NUM_COORDS_H] = {8.67f,  -0.00f, 7.23f,  -0.00f,  7.23f,   -5.27f, 2.66f,   -5.27f, 2.66f,
                      -0.00f, 1.23f,  -0.00f, 1.23f,   -11.38f, 2.66f,  -11.38f, 2.66f,  -6.48f,
                      7.23f,  -6.48f, 7.23f,  -11.38f, 8.67f,   -11.38, 8.67f,   -0.00f};


#define NUM_SEGS_e 40
#define NUM_COORDS_e 138
const unsigned char segs_e[NUM_SEGS_e] = {NEMA_VG_PRIM_MOVE,        NEMA_VG_PRIM_BEZIER_QUAD, NEMA_VG_PRIM_BEZIER_QUAD,
                       NEMA_VG_PRIM_BEZIER_QUAD, NEMA_VG_PRIM_BEZIER_QUAD, NEMA_VG_PRIM_LINE,
                       NEMA_VG_PRIM_BEZIER_QUAD, NEMA_VG_PRIM_BEZIER_QUAD, NEMA_VG_PRIM_BEZIER_QUAD,
                       NEMA_VG_PRIM_BEZIER_QUAD, NEMA_VG_PRIM_BEZIER_QUAD, NEMA_VG_PRIM_BEZIER_QUAD,
                       NEMA_VG_PRIM_BEZIER_QUAD, NEMA_VG_PRIM_BEZIER_QUAD, NEMA_VG_PRIM_LINE,
                       NEMA_VG_PRIM_LINE,        NEMA_VG_PRIM_LINE,        NEMA_VG_PRIM_BEZIER_QUAD,
                       NEMA_VG_PRIM_BEZIER_QUAD, NEMA_VG_PRIM_BEZIER_QUAD, NEMA_VG_PRIM_BEZIER_QUAD,
                       NEMA_VG_PRIM_BEZIER_QUAD, NEMA_VG_PRIM_BEZIER_QUAD, NEMA_VG_PRIM_LINE,
                       NEMA_VG_PRIM_BEZIER_QUAD, NEMA_VG_PRIM_BEZIER_QUAD, NEMA_VG_PRIM_BEZIER_QUAD,
                       NEMA_VG_PRIM_BEZIER_QUAD, NEMA_VG_PRIM_MOVE,        NEMA_VG_PRIM_BEZIER_QUAD,
                       NEMA_VG_PRIM_BEZIER_QUAD, NEMA_VG_PRIM_BEZIER_QUAD, NEMA_VG_PRIM_BEZIER_QUAD,
                       NEMA_VG_PRIM_LINE,        NEMA_VG_PRIM_LINE,        NEMA_VG_PRIM_BEZIER_QUAD,
                       NEMA_VG_PRIM_BEZIER_QUAD, NEMA_VG_PRIM_BEZIER_QUAD, NEMA_VG_PRIM_BEZIER_QUAD,
                       NEMA_VG_PRIM_CLOSE};
const float points_e[NUM_COORDS_e] = {
        4.06f, 0.16f,  3.30f, 0.16f,  2.69f, -0.06f, 2.08f, -0.30f, 1.66f, -0.77f, 1.23f, -1.23f,
        1.00f, -1.94f, 0.78f, -2.66f, 0.78f, -3.62f, 0.78f, -4.58f, 0.78f, -5.69f, 1.05f, -6.45f,
        1.31f, -7.23f, 1.73f, -7.70f, 2.17f, -8.19f, 2.73f, -8.39f, 3.30f, -8.61f, 3.91f, -8.61f,
        4.70f, -8.61f, 5.27f, -8.34f, 5.83f, -8.09f, 6.19f, -7.58f, 6.55f, -7.08f, 6.70f, -6.33f,
        6.88f, -5.59f, 6.88f, -4.62f, 6.88f, -3.81f, 2.16f, -3.81f, 2.16f, -3.62f, 2.16f, -2.27f,
        2.66f, -1.64f, 3.17f, -1.03,  4.14f, -1.03f, 4.48f, -1.03f, 4.77f, -1.09f, 5.05f, -1.17f,
        5.28f, -1.30f, 5.52f, -1.42f, 5.70f, -1.59f, 5.91f, -1.78f, 6.08f, -1.98f, 6.80f, -1.11f,
        6.62f, -0.88f, 6.38f, -0.64f, 6.14f, -0.42f, 5.80f, -0.23f, 5.47f, -0.06f, 5.03f, 0.05f,
        4.61f, 0.16f,  4.06f, 0.16f,  3.91f, -7.42f, 3.55f, -7.42f, 3.23f, -7.31f, 2.94f, -7.20f,
        2.70f, -6.92f, 2.47f, -6.66f, 2.33f, -6.19f, 2.19f, -5.72f, 2.16f, -5.00f, 5.48f, -5.00f,
        5.48f, -5.19f, 5.47f, -5.69f, 5.38f, -6.09f, 5.30f, -6.50f, 5.12f, -6.80f, 4.95f, -7.09f,
        4.66f, -7.25f, 4.36f, -7.42f, 3.91f, -7.42f};





#endif //__RAW_PATH_H__
