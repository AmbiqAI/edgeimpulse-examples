//*****************************************************************************
//
//! @file utils.h
//!
//! @brief NemaGFX example.
//!
//
//*****************************************************************************

//*****************************************************************************
//
// ${copyright}
//
// This is part of revision ${version} of the AmbiqSuite Development Package.
//
//*****************************************************************************

// -----------------------------------------------------------------------------
// Copyright (c) 2019 Think Silicon S.A.
// Think Silicon S.A. Confidential Proprietary
// -----------------------------------------------------------------------------
//     All Rights reserved - Unpublished -rights reserved under
//         the Copyright laws of the European Union
//
//  This file includes the Confidential information of Think Silicon S.A.
//  The receiver of this Confidential Information shall not disclose
//  it to any third party and shall protect its confidentiality by
//  using the same degree of care, but not less than a reasonable
//  degree of care, as the receiver uses to protect receiver's own
//  Confidential Information. The entire notice must be reproduced on all
//  authorised copies and copies may only be made to the extent permitted
//  by a licensing agreement from Think Silicon S.A..
//
//  The software is provided 'as is', without warranty of any kind, express or
//  implied, including but not limited to the warranties of merchantability,
//  fitness for a particular purpose and noninfringement. In no event shall
//  Think Silicon S.A. be liable for any claim, damages or other liability, whether
//  in an action of contract, tort or otherwise, arising from, out of or in
//  connection with the software or the use or other dealings in the software.
//
//
//                    Think Silicon S.A.
//                    http://www.think-silicon.com
//                    Patras Science Park
//                    Rion Achaias 26504
//                    Greece
// -----------------------------------------------------------------------------

#ifndef __UTILS_H_
#define __UTILS_H_

#include "nema_hal.h"
#include "nema_utils.h"

#ifndef FB_RESX
#define FB_RESX 390
#endif

#ifndef FB_RESY
#define FB_RESY 390
#endif

#define TEST_MAX    36

#ifndef TIMEOUT_S
#define TIMEOUT_S 1.f
#endif

extern TLS_VAR img_obj_t fb;

typedef enum
{
    CPU_GPU   = 0,
    CPU_BOUND = 1,
    GPU_BOUND = 2
} execution_mode;

int parse_params(int* testno, execution_mode* mode, int argc, char* argv[]);

void suite_init();
void suite_terminate();

void bench_start(int testno);
void bench_stop(int testno, int pix_count);

#endif
