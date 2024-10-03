//*****************************************************************************
//
//! @file bench.h
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

#ifndef __BENCH_H__
#define __BENCH_H__

#include "am_hal_global.h"

#include "nema_hal.h"
#include "nema_cmdlist.h"
#include "nema_utils.h"
#include "nema_programHW.h"
#include "am_util_stdio.h"
#include "string.h"
#include "utils.h"
#include "am_bsp.h"
#include "am_hal_stimer.h"
#ifndef ITEMS_PER_CL
#define ITEMS_PER_CL 90
#endif

extern execution_mode exec_mode;
extern TLS_VAR img_obj_t Ambiq_logo;
extern TLS_VAR nema_cmdlist_t *cl_cur, cl0, cl1, context_cl;

extern float start_wall, stop_wall;
static int render_frame();

#ifdef DEBUG_PRINTF
#define PRINTF am_util_debug_printf
#else
#define PRINTF(...)
#endif

void run_bench(int testno);

#define swap_cmd_lists() do {                   \
    cl_cur = (cl_cur == &cl0) ? &cl1 : &cl0;    \
} while(0)

static inline int
CL_CHECK_SUBMIT(int items_per_cl)
{
    int _items_per_cl = items_per_cl;
    if (_items_per_cl == 0)
    {
        _items_per_cl = ITEMS_PER_CL;
    }
    int items = 0;
    int pix_count = 0;
    start_wall = nema_get_time();
    stop_wall = start_wall;
    nema_cl_bind(cl_cur);
    nema_cl_rewind(cl_cur);
    nema_cl_branch(&context_cl);
    if (exec_mode != GPU_BOUND)
    {
        do
        {
            pix_count += render_frame();
            ++items;
            if ( (items % _items_per_cl) == 0)
            {
                if(exec_mode != CPU_BOUND)
                {
                    nema_cl_return();
                    nema_cl_submit(cl_cur);
                    PRINTF("Submitted CL %d\r\n", cl_cur->submission_id);
                }

                swap_cmd_lists();
                PRINTF("Waiting for CL %d\r\n", cl_cur->submission_id);
                nema_cl_wait(cl_cur);

                nema_cl_bind(cl_cur);
                nema_cl_rewind(cl_cur);
                nema_cl_branch(&context_cl);

                stop_wall = nema_get_time();
                if ((stop_wall-start_wall) > TIMEOUT_S)
                {
                    break;
                }
            }
        } while (1);

        PRINTF("Finishing Test\r\n");
        PRINTF("Waiting for CL %d\r\n", cl_cur->submission_id);
        nema_cl_wait(cl_cur);
        swap_cmd_lists();

        PRINTF("Waiting for CL %d\r\n", cl_cur->submission_id);
        nema_cl_wait(cl_cur);
    }
    else
    {
        int i;
        int pix_count_cl = 0;
        for ( i = 0; i < _items_per_cl; i++ )
        {
            pix_count_cl += render_frame();
        }
        while (1)
        {
            nema_cl_return();
            nema_cl_submit(cl_cur);
            nema_cl_wait(cl_cur);
            items += _items_per_cl;
            pix_count += pix_count_cl;
            stop_wall = nema_get_time();
            if ((stop_wall-start_wall) > TIMEOUT_S)
            {
                break;
            }
        }
    }
    stop_wall = nema_get_time();
    return pix_count;
}
#endif
