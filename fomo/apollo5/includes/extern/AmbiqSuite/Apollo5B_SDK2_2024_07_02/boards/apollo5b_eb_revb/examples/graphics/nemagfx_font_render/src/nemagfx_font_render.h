//*****************************************************************************
//
//! @file nemagfx_font_render.h
//!
//! @brief Global includes for the app.
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

#ifndef NEMAGFX_FONT_RENDER_H
#define NEMAGFX_FONT_RENDER_H

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
#include "am_hal_global.h"
#include "am_util_delay.h"

//*****************************************************************************
//
// third-party/devices include files.
//
//*****************************************************************************
#include "nema_hal.h"
#include "nema_core.h"
#include "nema_utils.h"
#include "nema_font.h"
#include "nema_programHW.h"

//*****************************************************************************
//
// Miscellaneous.
//
//*****************************************************************************
#ifndef USE_1BIT_FONT
#define USE_1BIT_FONT 1
#endif
#ifndef USE_2BIT_FONT
#define USE_2BIT_FONT 1
#endif
#ifndef USE_4BIT_FONT
#define USE_4BIT_FONT 1
#endif
#ifndef USE_8BIT_FONT
#define USE_8BIT_FONT 1
#endif

#define NEMA_FONT_IMPLEMENTATION
#if USE_1BIT_FONT != 0
    #include "DejaVuSerif8pt1b.h"
#endif
#if USE_2BIT_FONT != 0
    #include "DejaVuSerif8pt2b.h"
#endif
#if USE_4BIT_FONT != 0
    #include "DejaVuSerif8pt4b.h"
#endif
#if USE_8BIT_FONT != 0
    #include "DejaVuSerif8pt8b.h"
#endif

#undef NEMA_FONT_IMPLEMENTATION

#define RESX 390
#define RESY 390

#endif // NEMAGFX_FONT_RENDER_H
