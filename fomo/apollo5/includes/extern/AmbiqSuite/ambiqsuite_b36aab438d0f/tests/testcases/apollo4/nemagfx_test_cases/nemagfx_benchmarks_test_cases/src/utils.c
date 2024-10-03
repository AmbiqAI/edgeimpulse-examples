//*****************************************************************************
//
//! @file utils.c
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

#include "nema_core.h"
#include "nema_regs.h"
#include "nema_dc.h"
#include "nema_dc_mipi.h"
#include "utils.h"

#include "am_util_stdio.h"
#include "Ambiq200x104.rgba.h"
#include "am_devices_dsi_rm67162.h"
#include "am_devices_nemadc_rm67162.h"
#include "am_bsp.h"

float start_clk;
float start_wall;
float stop_clk;
float stop_wall;

#ifndef GPU_FREQ_MHZ
#define GPU_FREQ_MHZ 50
#endif

TLS_VAR img_obj_t fb = {{0}, (FB_RESX / 4) * 4, (FB_RESY / 4) * 4, -1, 0, NEMA_RGB565, 0};
TLS_VAR img_obj_t Ambiq_logo = {{0}, 200, 104, -1, 0, NEMA_RGBA8888, 0};
static TLS_VAR nemadc_layer_t dc_layer = {(void *)0, 0, (FB_RESX / 4) * 4, (FB_RESY / 4) * 4, -1, 0, 0, (FB_RESX / 4) * 4, (FB_RESY / 4) * 4, 0xff, NEMADC_BL_SRC, 0, NEMADC_RGB565, 0, 0, 0, 0, 0,0,0};

static void print_test(int testno);
static void bench_report(float perf, char *mes_unit, float utilization);

void print_test(int testno)
{
    const char *tests_str[TEST_MAX + 1] =
    {
        "_",
        "Fill_Triangle              ",
        "Fill_Triangle_Blend        ",
        "Fill_Rectangle             ",
        "Fill_Rectangle_Blend       ",
        "Fill_Quad                  ",
        "Fill_Quad_Blend            ",
        "Draw_String                ",
        "Draw_Line                  ",
        "Draw_Line_Blend            ",
        "Draw_Rectangle            ",
        "Draw_Rectangle_Blend      ",
        "Blit                      ",
        "Blit_Colorize             ",
        "Blit_Blend                ",
        "Blit_Blend_Colorize       ",
        "Blit_90                   ",
        "Blit_180                  ",
        "Blit_270                  ",
        "Blit_Vertical_Flip        ",
        "Blit_Horizontal_Flip      ",
        "Blit_SRC_Colorkeyed       ",
        "Blit_DST_Colorkeyed       ",
        "Stretch_Blit_PS           ",
        "Stretch_Blit_Blend_PS     ",
        "Stretch_Blit_BL           ",
        "Stretch_Blit_Blend_BL     ",
        "Stretch_Blit_Rotate       ",
        "Stretch_Blit_Rotate_BL    ",
        "Textured_Triangle_PS      ",
        "Textured_Triangle_Blend_PS",
        "Textured_Triangle_BL      ",
        "Textured_Triangle_Blend_BL",
        "Textured_Quad_PS          ",
        "Textured_Quad_Blend_PS    ",
        "Textured_Quad_BL          ",
        "Textured_Quad_Blend_BL    "
    };

    if (testno <= 0)
    {
        for (testno = 1; testno <= TEST_MAX; ++testno)
        {
            am_util_stdio_printf("%d    %s\r\n", testno, tests_str[testno]);
        }
    }
    else if (testno > TEST_MAX)
    {
        am_util_stdio_printf("%d is not a valid test\n", testno);
    }
    else
    {
        am_util_stdio_printf("%d: %s    ", testno, tests_str[testno]);
    }
}

int parse_params(int* testno, execution_mode* mode, int argc, char* argv[])
{
    return 0;
}

static int _memcpy(uint8_t* dst, const uint8_t* src, uint32_t nbytes)
{
    uint32_t i;
    int count = 0;
    for (i = 0; i < nbytes; i++)
    {
        dst[i] = src[i];
        count++;
    }

    return count;
}

static void clear_background(uint32_t col)
{
    // Create CmdList
    static TLS_VAR nema_cmdlist_t cl;
    cl = nema_cl_create();
    // Bind CmdList
    nema_cl_bind(&cl);
    // Bind Framebuffer
    nema_bind_dst_tex(fb.bo.base_phys, fb.w, fb.h, (nema_tex_format_t)(fb.format), fb.stride);
    nema_set_clip(0, 0, FB_RESX, FB_RESY);
    // Clear color
    nema_clear(col);

    nema_cl_unbind();
    nema_cl_submit(&cl);
    nema_cl_wait(&cl);

    nema_cl_destroy(&cl);
}

#define ALLOC_TSILOGO_MEM

void suite_init()
{
#ifdef ALLOC_TSILOGO_MEM
    Ambiq_logo.bo = nema_buffer_create_pool(NEMA_MEM_POOL_ASSETS, Ambiq_logo.w*Ambiq_logo.h*4);

    _memcpy(Ambiq_logo.bo.base_virt, Ambiq200x104, Ambiq_logo.w*Ambiq_logo.h*4);
    nema_buffer_flush(&Ambiq_logo.bo);
#else
    Ambiq_logo.bo.base_virt = (void *)Ambiq200x104;
    Ambiq_logo.bo.base_phys = (uintptr_t)Ambiq200x104;
    Ambiq_logo.bo.size      = Ambiq_logo.w*Ambiq_logo.h*4;
#endif

    // Load framebuffer
    fb.bo = nema_buffer_create_pool(NEMA_MEM_POOL_FB, fb.w*fb.h*4);
    nema_buffer_map(&fb.bo);

    dc_layer.baseaddr_phys = fb.bo.base_phys;
    dc_layer.baseaddr_virt = fb.bo.base_virt;
}

void suite_terminate()
{
    nema_buffer_destroy(&fb.bo);

#ifdef ALLOC_TSILOGO_MEM
    nema_buffer_destroy(&Ambiq_logo.bo);
#endif
}


void bench_start(int testno)
{
    clear_background(0x0);

    print_test(testno);

    start_clk = nema_get_time();
}

float perfs[64];
void bench_report(float perf, char *mes_unit, float utilization)
{
    int perf_i = (int)perf;
    perf = perf-perf_i;
    int perf_f = (int)(perf*100.f);
    am_util_stdio_printf("%d.%d %s\r\n", perf_i, perf_f, mes_unit);
}

void bench_stop(int testno, int pix_count)
{
    stop_clk = nema_get_time();
    nema_buffer_flush(&fb.bo);
    nemadc_set_layer(0, &dc_layer);
    nemadc_send_frame_single();

    float total_cpu_s  = stop_wall - start_wall;

    if (testno == 7)
    {
        //test 7 - draw_strings - is measured in KChars/sec
        bench_report(pix_count / 1000.f / total_cpu_s, " KChars/sec", 0.f);
    }
    else if (testno == 37 || testno == 38 )
    {
        bench_report(pix_count / 1000.f / total_cpu_s, " KCLs/sec", 0.f);
    }
    else
    {
        bench_report(pix_count / 1000.f / 1000.f / total_cpu_s, "MPixels/sec", 0.f);
    }

    // sleep(1);
}

