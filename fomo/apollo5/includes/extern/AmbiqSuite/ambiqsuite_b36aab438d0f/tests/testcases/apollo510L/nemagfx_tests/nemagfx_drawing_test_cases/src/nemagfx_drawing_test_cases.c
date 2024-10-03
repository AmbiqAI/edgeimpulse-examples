//*****************************************************************************
//
//! @file nemagfx_drawing_test_cases.c
//!
//! @brief NemaGFX drawing test cases.
//! Need to connect RM67162 to DC SPI4 interface.
//! Need to run this case 30 times at least for 1 part, and hardware reset or
//! power cycle part before every run.
//
//*****************************************************************************

//*****************************************************************************
//
// ${copyright}
//
// This is part of revision ${version} of the AmbiqSuite Development Package.
//
//*****************************************************************************

//*****************************************************************************
//
// This application has a large number of common include files. For
// convenience, we'll collect them all together in a single header and include
// that everywhere.
//
//*****************************************************************************
#include "nemagfx_test_common.h"
#include "utils.h"

extern img_obj_t fb;
TLS_VAR nema_cmdlist_t *cl_cur, cl0, cl1, context_cl;
#define swap_cmd_lists() do {                   \
    cl_cur = (cl_cur == &cl0) ? &cl1 : &cl0;    \
} while(0)

void
setUp(void)
{
}

void
tearDown(void)
{
}

static int premultiply_color;

static int render_frame()
{
    int x0 = 0;
    int y2 = 60;
    int x1 = 0;
    uint32_t col = 0x000000FF;
    int y0 = 0;
    int x2 = 60;
    int y1 = 60;

    if (premultiply_color != 0)
    {
     //   col = nema_premultiply_rgba(col);
    }
    nema_fill_triangle(x0, y0, x1, y1, x2, y2, col);

    int area = (int)(0.5f * nema_abs(x0 * (y1 - y2) +
                               x1 * (y2 - y0) +
                               x2 * (y0 - y1)));

    return area;
}

void bench_fill_tri(int blend)
{
    context_cl = nema_cl_create();
    cl0 = nema_cl_create();
    cl1 = nema_cl_create();
    cl_cur = &cl0;

    nema_cl_bind(&context_cl);
    //Bind Framebuffer
    nema_bind_dst_tex(fb.bo.base_phys, fb.w, fb.h, (nema_tex_format_t)(fb.format), -1);
    //Set Clipping Rectangle
    nema_set_clip(0, 0, FB_RESX, FB_RESY);

    premultiply_color = blend;

    nema_cl_bind(cl_cur);
    nema_cl_rewind(cl_cur);
    nema_cl_branch(&context_cl);

    render_frame();

    nema_cl_return();
    nema_cl_submit(cl_cur);

    swap_cmd_lists();
    nema_cl_wait(cl_cur);

    nema_cl_bind(cl_cur);
    nema_cl_rewind(cl_cur);
    nema_cl_branch(&context_cl);

    nema_cl_wait(cl_cur);
    swap_cmd_lists();

    nema_cl_wait(cl_cur);

    nema_cl_destroy(&context_cl);
    nema_cl_destroy(&cl0);
    nema_cl_destroy(&cl1);
}
//*****************************************************************************
//
//! @brief Test NemaGFX drawing feature.
//!
//! @return bTestPass.
//
//*****************************************************************************
bool
nemagfx_drawing_test(void)
{
    bool bTestPass = true;

    int result = 0;
    suite_init();

    clear_background(0x0);
    bench_fill_tri(0);
    bTestPass = bench_stop(1, result);
    TEST_ASSERT_TRUE(bTestPass);

    return bTestPass;
}
