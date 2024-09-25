//*****************************************************************************
//
//! @file nemagfx_blend.h
//!
//! @brief Global includes for the nemagfx rotating crate example.
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

#ifndef NEMAGFX_BLEND_H
#define NEMAGFX_BLEND_H

//*****************************************************************************
//
// Required built-ins.
//
//*****************************************************************************
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>

//*****************************************************************************
//
// Standard AmbiqSuite includes.
//
//*****************************************************************************
#include "am_mcu_apollo.h"
#include "am_bsp.h"
#include "am_util.h"
#include "am_util_delay.h"
#include "am_util_stdio.h"

//*****************************************************************************
//
// third-party/devices include files.
//
//*****************************************************************************
#include "nema_regs.h"
#include "nema_core.h"
#include "nema_utils.h"

#include "nema_dc_mipi.h"
#include "nema_dc.h"
#include "nema_dc_regs.h"
//*****************************************************************************
//
// Miscellaneous.
//
//*****************************************************************************
#define RESX            400
#define RESY            400

#define RED                     (0x000000FFU)
#define OPAQUE(color)           ((color) | 0xFF000000)

#define TRANSP(alpha, color)    ((color & 0x00FFFFFF) | ((alpha & 0xff) << 24))

#ifndef APOLLO5_FPGA
    #define CPU_DEEP_SLEEP
#endif

//
// DSI sends ULPS pattern on or off when DSI enters/exits ULPS mode
//
#define ULPS_PATTERN_ON     true

#endif // NEMAGFX_BLEND_H
