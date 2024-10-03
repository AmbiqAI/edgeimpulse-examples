//*****************************************************************************
//
//! @file main.c
//!
//! @brief NemaGFX example.
//! this example demonstrate the Nema GPU and CPU performance use Nema GPU's
//! basic characteristics, we should care about the FPS after each individual
//! test.
//! need a timer to get the accurate time past.
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
#include "nemagfx_test_common.h"
#include "am_bsp.h"
#include "nema_core.h"
#include "nema_utils.h"

#ifndef DONT_USE_NEMADC
#include "nema_dc.h"
#include "nema_dc_mipi.h"
#endif

#include "utils.h"
#include "bench.h"
#include "am_devices_dsi_rm67162.h"
#include "am_devices_nemadc_rm67162.h"

int bench_fill_quad(int blend);
int bench_fill_rect(int blend);
int bench_fill_tri(int blend);

int bench_draw_circle(int blendmode);
int bench_draw_line(int blendmode);
int bench_draw_rect(int blendmode);
int bench_draw_string(int blendmode);

int bench_blit(int blendmode, int rotation);
int bench_stretch_blit(int blendmode, float scale, int nema_tex_mode);
int bench_stretch_blit_rotate(int blendmode, float scale, int nema_tex_mode);

int bench_textured_tri(int blendmode, int nema_tex_mode);
int bench_textured_quad(int blendmode, int nema_tex_mode);

#ifndef DEFAULT_EXEC_MODE
#define DEFAULT_EXEC_MODE CPU_GPU
#endif

TLS_VAR AM_SHARED_RW nema_cmdlist_t *cl_cur, cl0, cl1, context_cl;
execution_mode exec_mode = DEFAULT_EXEC_MODE;

extern float start_wall, stop_wall;

static int
render_frame()
{
  return 0;
}

static int
test_irq(int mode)
{

    cl0 = nema_cl_create();
    nema_cl_bind(&cl0);

    //just some dummy instructions
    nema_set_blend_fill(NEMA_BL_SRC);


    start_wall = nema_get_time();
    stop_wall  = start_wall;

    int cl_count = 0;

    do
    {
        do
        {
            nema_cl_submit(&cl0);
            if (mode == 0)
            {
                nema_cl_wait(&cl0);
            }
            ++cl_count;
        } while ( cl_count % 1000 != 0 );

        stop_wall = nema_get_time();
    } while ( (stop_wall - start_wall) < TIMEOUT_S );

    nema_cl_destroy(&cl0);

    return cl_count;
}

void run_bench(int testno)
{
    suite_init();

    int result = 0;
    
    AM_REGVAL(0x40090090) = 0x00000002;

    switch (testno)
    {
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
            result = bench_blit(NEMA_BL_SRC | NEMA_BLOP_MODULATE_RGB, NEMA_ROT_000_CCW);
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

static int init(void)
{
    am_hal_timer_config_t       TimerConfig;
    am_hal_timer_default_config_set(&TimerConfig);
    TimerConfig.eInputClock = AM_HAL_TIMER_CLOCK_HFRC_DIV16;
    TimerConfig.eFunction = AM_HAL_TIMER_FN_EDGE;

    am_hal_timer_config(0, &TimerConfig);
    am_hal_timer_start(0);

    g_eDispType = RM67162_SPI4;

    am_devices_nemadc_rm67162_init(MIPICFG_SPI4, MIPICFG_1RGB565_OPT0, FB_RESX, FB_RESY, 0, 0);

    return 0;
}

bool
nemagfx_benchmarks_test()
{
    int testno = 0;
    bool bTestPass = true; 

    init();

    if ( testno != 0 )
    {
        run_bench(testno);
    }
    else
    {
        for (int test = 1; test <= TEST_MAX; ++test )
        {
            run_bench(test);
        }
    }


    TEST_ASSERT_TRUE(bTestPass);

    return bTestPass;
}
