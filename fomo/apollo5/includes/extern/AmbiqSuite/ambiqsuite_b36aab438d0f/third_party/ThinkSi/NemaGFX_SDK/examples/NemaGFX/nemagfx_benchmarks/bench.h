/* TSI 2023.xmp */
/*******************************************************************************
 * Copyright (c) 2023 Think Silicon Single Member PC
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this file and/or associated documentation files to use, copy,
 * modify, merge, publish, distribute, sublicense, and/or sell copies of the
 * Materials, and to permit persons to whom the Materials are furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Materials.
 *
 * The software is provided 'as is', without warranty of any kind, express or
 * implied, including but not limited to the warranties of merchantability,
 * fitness for a particular purpose and noninfringement. In no event shall Think
 * Silicon Single Member PC be liable for any claim, damages or other liability,
 * whether in an action of contract, tort or otherwise, arising from, out of or
 * in connection with the software or the use or other dealings in the software.
 ******************************************************************************/

#ifndef __BENCH_H__
#define __BENCH_H__

#include "nema_hal.h"
#include "nema_cmdlist.h"
#include "nema_utils.h"

#include "utils.h"

#ifndef ITEMS_PER_CL
#define ITEMS_PER_CL 90
#endif

extern execution_mode exec_mode;
extern TLS_VAR img_obj_t TSi_logo;
static TLS_VAR nema_cmdlist_t *cl_cur, cl0, cl1, context_cl;

extern float start_wall, stop_wall;

static int render_frame();

// #define PRINTF printf
#define PRINTF(...)

void run_bench(int testno);

#define swap_cmd_lists() do {   \
    cl_cur = (cl_cur == &cl0) ? &cl1 : &cl0;    \
} while(0)

static inline float CL_CHECK_SUBMIT(int items_per_cl) {
    int _items_per_cl = items_per_cl;

    if (_items_per_cl == 0) {
        _items_per_cl = ITEMS_PER_CL;
    }

    int items = 0;
    int pix_count = 0;
    start_wall = nema_get_time();
    stop_wall = start_wall;

    nema_cl_bind(cl_cur);
    nema_cl_rewind(cl_cur);
    nema_cl_branch(&context_cl);

    if (exec_mode != GPU_BOUND) {
        do {
            pix_count += render_frame();

            ++items;
            if ( (items % _items_per_cl) == 0) {

                if(exec_mode != CPU_BOUND) {
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
                if ((stop_wall-start_wall) > TIMEOUT_S) {
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
    }else {
        int i;
        int pix_count_cl = 0;
        for (i=0; i < _items_per_cl; i++) {
            pix_count_cl += render_frame();
        }
        while (1) {
            nema_cl_return();
            nema_cl_submit(cl_cur);
            nema_cl_wait(cl_cur);
            items += _items_per_cl;
            pix_count += pix_count_cl;
            stop_wall = nema_get_time();
            if ((stop_wall-start_wall) > TIMEOUT_S) {
                break;
            }

        }
    }

    stop_wall = nema_get_time();

    return pix_count;
}

#endif
