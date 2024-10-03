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

#include "nema_core.h"
#include "nema_graphics.h"

#ifndef DONT_USE_NEMADC
#include "nema_dc.h"
#endif

#include "utils.h"
#include "bench.h"


float bench_fill_quad(int blend);
float bench_fill_rect(int blend);
float bench_fill_tri(int blend);

float bench_draw_circle(int blendmode);
float bench_draw_line(int blendmode);
float bench_draw_rect(int blendmode);
float bench_draw_string(int blendmode);

float bench_blit(int blendmode, int rotation);
float bench_stretch_blit(int blendmode, float scale, int nema_tex_mode);
float bench_stretch_blit_rotate(int blendmode, float scale, int nema_tex_mode);

float bench_textured_tri(int blendmode, int nema_tex_mode);
float bench_textured_quad(int blendmode, int nema_tex_mode);

#ifndef DEFAULT_EXEC_MODE
#define DEFAULT_EXEC_MODE CPU_GPU
#endif

execution_mode exec_mode = DEFAULT_EXEC_MODE;

extern float start_wall, stop_wall;

static int test_irq(int mode) {

    cl0 = nema_cl_create();
    nema_cl_bind(&cl0);

    //just some dummy instructions
    nema_set_blend_fill(NEMA_BL_SRC);


    start_wall = nema_get_time();
    stop_wall  = start_wall;

    int cl_count = 0;

    do {
        do {
            nema_cl_submit(&cl0);
            if (mode == 0) {
                nema_cl_wait(&cl0);
            }
            ++cl_count;
        } while(cl_count%1000 != 0);

        stop_wall = nema_get_time();
    } while( (stop_wall - start_wall) < TIMEOUT_S );

    nema_cl_destroy(&cl0);

    return cl_count;
}

void run_bench(int testno) {
    suite_init();

    float result = 0;

    switch (testno) {
    case 37:
        bench_start(testno);
        result = test_irq(0);
        bench_stop(testno, result);
        break;
    case 38:
        bench_start(testno);
        result = test_irq(1);
        bench_stop(testno, result);
        break;

    case 1:
        bench_start(testno);
        result = bench_fill_tri(0);
        bench_stop(testno, result);
        break;

    case 2:
        bench_start(testno);
        result = bench_fill_tri(1);
        bench_stop(testno, result);
        break;

    case 3:
        bench_start(testno);
        result = bench_fill_rect(0);
        bench_stop(testno, result);
        break;

    case 4:
        bench_start(testno);
        result = bench_fill_rect(1);
        bench_stop(testno, result);
        break;

    case 5:
        bench_start(testno);
        result = bench_fill_quad(0);
        bench_stop(testno, result);
        break;

    case 6:
        bench_start(testno);
        result = bench_fill_quad(1);
        bench_stop(testno, result);
        break;

    case 7:
        bench_start(testno);
        result = bench_draw_string(NEMA_BL_SRC);
        bench_stop(testno, result);
        break;

    case 8:
        bench_start(testno);
        result = bench_draw_line(0);
        bench_stop(testno, result);
        break;

    case 9:
        bench_start(testno);
        result = bench_draw_line(1);
        bench_stop(testno, result);
        break;

    case 10:
        bench_start(testno);
        result = bench_draw_rect(0);
        bench_stop(testno, result);
        break;

    case 11:
        bench_start(testno);
        result = bench_draw_rect(1);
        bench_stop(testno, result);
        break;

    case 12:
        bench_start(testno);
        result = bench_blit(NEMA_BL_SRC, NEMA_ROT_000_CCW);
        bench_stop(testno, result);
        break;

    case 13:
        bench_start(testno);
        result = bench_blit(NEMA_BL_SRC|NEMA_BLOP_MODULATE_RGB, NEMA_ROT_000_CCW);
        bench_stop(testno, result);
        break;

    case 14:
        bench_start(testno);
        result = bench_blit(NEMA_BL_SIMPLE, NEMA_ROT_000_CCW);
        bench_stop(testno, result);
        break;

    case 15:
        bench_start(testno);
        result = bench_blit(NEMA_BL_SIMPLE | NEMA_BLOP_MODULATE_RGB, NEMA_ROT_000_CCW);
        bench_stop(testno, result);
        break;

    case 16:
        bench_start(testno);
        result = bench_blit(NEMA_BL_SRC, NEMA_ROT_090_CW);
        bench_stop(testno, result);
        break;

    case 17:
        bench_start(testno);
        result = bench_blit(NEMA_BL_SRC, NEMA_ROT_180_CW);
        bench_stop(testno, result);
        break;

    case 18:
        bench_start(testno);
        result = bench_blit(NEMA_BL_SRC, NEMA_ROT_270_CW);
        bench_stop(testno, result);
        break;

    case 19:
        bench_start(testno);
        result = bench_blit(NEMA_BL_SRC, NEMA_MIR_VERT);
        bench_stop(testno, result);
        break;

    case 20:
        bench_start(testno);
        result = bench_blit(NEMA_BL_SRC, NEMA_MIR_HOR);
        bench_stop(testno, result);
        break;

    case 21:
        bench_start(testno);
        result = bench_blit(NEMA_BL_SRC | NEMA_BLOP_SRC_CKEY, NEMA_ROT_000_CCW);
        bench_stop(testno, result);
        break;

    case 22:
        bench_start(testno);
        result = bench_blit(NEMA_BL_SRC | NEMA_BLOP_DST_CKEY, NEMA_ROT_000_CCW);
        bench_stop(testno, result);
        break;

    case 23:
        bench_start(testno);
        result = bench_stretch_blit(NEMA_BL_SRC, 1.5, NEMA_FILTER_PS);
        bench_stop(testno, result);
        break;

    case 24:
        bench_start(testno);
        result = bench_stretch_blit(NEMA_BL_SIMPLE, 1.5, NEMA_FILTER_PS);
        bench_stop(testno, result);
        break;

    case 25:
        bench_start(testno);
        result = bench_stretch_blit(NEMA_BL_SRC, 1.5, NEMA_FILTER_BL);
        bench_stop(testno, result);
        break;

    case 26:
        bench_start(testno);
        result = bench_stretch_blit(NEMA_BL_SIMPLE, 1.5, NEMA_FILTER_BL);
        bench_stop(testno, result);
        break;

    case 27:
        bench_start(testno);
        result = bench_stretch_blit_rotate(NEMA_BL_SRC, 0.75, NEMA_FILTER_PS);
        bench_stop(testno, result);
        break;

    case 28:
        bench_start(testno);
        result = bench_stretch_blit_rotate(NEMA_BL_SRC, 0.75, NEMA_FILTER_BL);
        bench_stop(testno, result);
        break;

    case 29:
        bench_start(testno);
        result = bench_textured_tri(NEMA_BL_SRC, NEMA_FILTER_PS);
        bench_stop(testno, result);
        break;

    case 30:
        bench_start(testno);
        result = bench_textured_tri(NEMA_BL_SIMPLE, NEMA_FILTER_PS);
        bench_stop(testno, result);
        break;

    case 31:
        bench_start(testno);
        result = bench_textured_tri(NEMA_BL_SRC, NEMA_FILTER_BL);
        bench_stop(testno, result);
        break;

    case 32:
        bench_start(testno);
        result = bench_textured_tri(NEMA_BL_SIMPLE, NEMA_FILTER_BL);
        bench_stop(testno, result);
        break;

    case 33:
        bench_start(testno);
        result = bench_textured_quad(NEMA_BL_SRC, NEMA_FILTER_PS);
        bench_stop(testno, result);
        break;

    case 34:
        bench_start(testno);
        result = bench_textured_quad(NEMA_BL_SIMPLE, NEMA_FILTER_PS);
        bench_stop(testno, result);
        break;

    case 35:
        bench_start(testno);
        result = bench_textured_quad(NEMA_BL_SRC, NEMA_FILTER_BL);
        bench_stop(testno, result);
        break;

    case 36:
        bench_start(testno);
        result = bench_textured_quad(NEMA_BL_SIMPLE, NEMA_FILTER_BL);
        bench_stop(testno, result);
        break;
    default:
        return;
    }

    suite_terminate();
}

#ifdef STANDALONE
int main(int argc, char* argv[])
{
    int testno = 0;
    if ( parse_params(&testno, &exec_mode, argc, argv) != 0) {
        return 1;
    }

    int ret = nema_init();
    if (ret != 0) {
        return 1;
    }

#ifndef DONT_USE_NEMADC
    ret = nemadc_init();
    if (ret != 0) {
        return 1;
    }
#endif

    if (testno != 0) {
        run_bench(testno);
    } else {
        for (int test = 1; test <= TEST_MAX; ++test ) {
            run_bench(test);
        }
    }


    return 0;
}
#endif
