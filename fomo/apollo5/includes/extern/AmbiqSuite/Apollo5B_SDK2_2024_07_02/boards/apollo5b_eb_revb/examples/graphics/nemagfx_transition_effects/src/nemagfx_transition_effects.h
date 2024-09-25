//*****************************************************************************
//
//! @file nemagfx_transition_effects.h
//!
//! @brief Global includes for the freertos_fit app.
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

#ifndef NEMAGFX_TRANSITION_H
#define NEMAGFX_TRANSITION_H

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

//*****************************************************************************
//
// Third-party/Devices include files.
//
//*****************************************************************************

#include "nema_core.h"
#include "nema_utils.h"
#include "nema_transitions.h"


#ifndef BAREMETAL
//*****************************************************************************
//
// FreeRTOS include files.
//
//*****************************************************************************
#include "FreeRTOS.h"
#include "task.h"
#include "portmacro.h"
#include "portable.h"
#include "semphr.h"
#include "event_groups.h"
#include "rtos.h"

//*****************************************************************************
//
// Task include files.
//
//*****************************************************************************
#include "gui_task.h"
#endif // BAREMETAL

//*****************************************************************************
//
// Miscellaneous.
//
//*****************************************************************************

#define PANEL_RESX          (g_sDispCfg.ui16ResX/4*4)
#define PANEL_RESY          (g_sDispCfg.ui16ResY/4*4)

#define RESX 256
#define RESY 256

#define PANEL_OFFSET        ((PANEL_RESX-RESX)/2)

//
// use small frame buffer if defined macro SMALLFB
//
//#define SMALLFB

#if defined(SMALLFB)
#define SMALLFB_STRIPES     4
#define FRAME_BUFFERS       2
#else
#define SMALLFB_STRIPES     1
#define FRAME_BUFFERS       1
#endif

#define ANIMATION_STEP_0_1  0.02f
#define MIN_STEP            0.0f
#define MAX_STEP            1.0f


#endif