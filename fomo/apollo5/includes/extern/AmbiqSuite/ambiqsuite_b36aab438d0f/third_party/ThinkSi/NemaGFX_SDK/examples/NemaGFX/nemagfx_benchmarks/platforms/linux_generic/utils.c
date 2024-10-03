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

#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
// #include <time.h>
#include <unistd.h>
#include <getopt.h>

#include "utils.h"
#include "nema_core.h"

#ifndef DONT_USE_NEMADC
#include "nema_dc.h"
#endif

float start_clk;
float start_wall;
float stop_clk;
float stop_wall;

#ifndef GPU_FREQ_MHZ
#define GPU_FREQ_MHZ 50
#endif

img_obj_t fb = {{0}, RESX, RESY, -1, 0, NEMA_RGBA8888, 0};
img_obj_t TSi_logo = {{0}, 200, 104, -1, 0, NEMA_RGBA8888, 0};


#ifndef DONT_USE_NEMADC
static nemadc_layer_t dc_layer = {(void *)0, 0, RESX, RESY, -1, 0, 0, RESX, RESY, 0xff, NEMADC_BL_SRC, 0, NEMADC_RGBA8888, 0, 0, 0, 0, 0};
#endif

static void print_test(int testno);
static void bench_report(float perf, const char *mes_unit, float utilization);

void print_test(int testno) {
    char *tests_str[37] = {
        "_",
        "Fill_Triangle",
        "Fill_Triangle_Blend",
        "Fill_Rectangle",
        "Fill_Rectangle_Blend",
        "Fill_Quad",
        "Fill_Quad_Blend",
        "Draw_String",
        "Draw_Line",
        "Draw_Line_Blend",
        "Draw_Rectangle",
        "Draw_Rectangle_Blend",
        "Blit",
        "Blit_Colorize",
        "Blit_Blend",
        "Blit_Blend_Colorize",
        "Blit_90",
        "Blit_180",
        "Blit_270",
        "Blit_Vertical_Flip",
        "Blit_Horizontal_Flip",
        "Blit_SRC_Colorkeyed",
        "Blit_DST_Colorkeyed",
        "Stretch_Blit_PS",
        "Stretch_Blit_Blend_PS",
        "Stretch_Blit_BL",
        "Stretch_Blit_Blend_BL",
        "Stretch_Blit_Rotate",
        "Stretch_Blit_Rotate_BL",
        "Textured_Triangle_PS",
        "Textured_Triangle_Blend_PS",
        "Textured_Triangle_BL",
        "Textured_Triangle_Blend_BL",
        "Textured_Quad_PS",
        "Textured_Quad_Blend_PS",
        "Textured_Quad_BL",
        "Textured_Quad_Blend_BL"
    };

    if (testno <= 0) {
        for (testno = 1; testno <= TEST_MAX; ++testno) {
            printf("%2d\t%-30s\n", testno, tests_str[testno]);
        }
    } else if (testno > TEST_MAX) {
        printf("%d is not a valid test\n", testno);
    } else {
        printf("%2d\t%-30s\t", testno, tests_str[testno]);
    }
}

int parse_params(int* testno, execution_mode* mode, int argc, char* argv[]) {

    int c;

    while ((c = getopt(argc, argv, "m:t:h")) != -1) {
        switch (c) {
        case 'm':
            if( strcmp(optarg, "cpu") != 0) {
                *mode = CPU_BOUND;
            }
            else if( strcmp(optarg, "gpu") != 0 ) {
                *mode = GPU_BOUND;
            }
            else {
                *mode = CPU_GPU;
            }

            break;
        case 't':
            *testno = atoi(optarg);
            break;
        case '?':
        case 'h':
            print_test(0);
            return 1;
            break;
        default:
            printf("Error parsing parameters\n");
            return -1;
        }
        opterr = 0;
    }

    return 0;
}


static void clear_background(uint32_t col)
{
    // Create CmdList
    nema_cmdlist_t cl = nema_cl_create();
    // Bind CmdList
    nema_cl_bind(&cl);
    // Bind Framebuffer
    nema_bind_dst_tex(fb.bo.base_phys, fb.w, fb.h, fb.format, fb.stride);
    nema_set_clip(0, 0, RESX, RESY);
    // Clear color
    nema_clear(col);

    nema_cl_unbind();
    nema_cl_submit(&cl);
    nema_cl_wait(&cl);

    nema_cl_destroy(&cl);
}

void suite_init()
{
    TSi_logo.bo = nema_load_file("./ThinkSiliconLogo200x104.rgba", TSi_logo.w*TSi_logo.h*4, 0);

    // Load framebuffer
    fb.bo = nema_buffer_create_pool(NEMA_MEM_POOL_FB, fb.w*fb.h*4);
    nema_buffer_map(&fb.bo);

    printf("FB: V: %p P: 0x%08x\n", fb.bo.base_virt, fb.bo.base_phys);

#ifndef DONT_USE_NEMADC
    dc_layer.baseaddr_phys = fb.bo.base_phys;
    dc_layer.baseaddr_virt = fb.bo.base_virt;

    // Format        | Pixclock | RESX | FP | SYNC | BP | RESY | FP | SYNC | BP
    // 800x600, 60Hz | 40.000   | 800  | 40 | 128  | 88 | 600  | 1  | 4    | 23
    nemadc_timing(800, 40, 128, 88, 600, 1, 4, 23);
    nemadc_set_layer(0, &dc_layer);
#endif
}

void suite_terminate() {
    nema_buffer_destroy(&fb.bo);
    nema_buffer_destroy(&TSi_logo.bo);
}


void bench_start(int testno)
{
    start_clk = nema_get_time();

    clear_background(0x0);

    print_test(testno);
}

void bench_report(float perf, const char *mes_unit, float utilization)
{
    printf("%10.3f %s\t", perf, mes_unit);
    //printf("%6.2f%%", utilization);
    /* printf("%2.2f %s \t", perf, mes_unit); */
    /* printf("Utilization: %2.2f %%\n",utilization); */
    printf("\n");
}

void bench_stop(int testno, int pix_count)
{
#ifndef DONT_USE_NEMADC
    nemadc_set_layer(0, &dc_layer);
#endif
    stop_clk = nema_get_time();

    float total_cpu_s  = stop_wall - start_wall;

    if (testno == 7) {
        //test 7 - draw_strings - is measured in KChars/sec
        bench_report((float)pix_count/ 1000.f / total_cpu_s, " KChars/sec", 0.f);
    } else {
        bench_report((float)pix_count/ 1000.f/ 1000.f / total_cpu_s, "MPixels/sec", 0.f);
    }
}
