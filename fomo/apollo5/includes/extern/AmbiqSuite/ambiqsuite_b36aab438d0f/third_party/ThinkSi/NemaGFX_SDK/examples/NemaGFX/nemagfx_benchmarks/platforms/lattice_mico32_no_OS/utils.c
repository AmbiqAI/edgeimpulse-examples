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
#include <unistd.h>

#include "utils.h"
#include "assist.h"
#include "nema_ocd.h"
#include "nema_core.h"

float start_clk;
float start_wall;
float stop_clk;
float stop_wall;

#ifndef GPU_FREQ_MHZ
#define GPU_FREQ_MHZ 50
#endif

uint8_t has_OCD = 0;
uint8_t ncores = 0;

img_obj_t fb = {{0}, RESX, RESY, -1, 0, NEMA_RGBA8888, 0};
img_obj_t TSi_logo = {{0}, 200, 104, -1, 0, NEMA_RGBA8888, 0};

static void print_test(int testno);
static void bench_report(float perf, char *mes_unit, float utilization);


#ifndef LOAD_TEXTURE_FROM_FS
#include "ThinkSiliconLogo200x104.rgba.h"
#endif

void print_test(int testno) {

    if (testno > TEST_MAX) {
        nprintd(testno);
        aprint("is not a valid test\n");
        return;
    }

    char *tests_str[37] = {
        "                              ",
        "Fill_Triangle                 ",
        "Fill_Triangle_Blend           ",
        "Fill_Rectangle                ",
        "Fill_Rectangle_Blend          ",
        "Fill_Quad                     ",
        "Fill_Quad_Blend               ",
        "Draw_String                   ",
        "Draw_Line                     ",
        "Draw_Line_Blend               ",
        "Draw_Rectangle                ",
        "Draw_Rectangle_Blend          ",
        "Blit                          ",
        "Blit_Colorize                 ",
        "Blit_Blend                    ",
        "Blit_Blend_Colorize           ",
        "Blit_90                       ",
        "Blit_180                      ",
        "Blit_270                      ",
        "Blit_Vertical_Flip            ",
        "Blit_Horizontal_Flip          ",
        "Blit_SRC_Colorkeyed           ",
        "Blit_DST_Colorkeyed           ",
        "Stretch_Blit_PS               ",
        "Stretch_Blit_Blend_PS         ",
        "Stretch_Blit_BL               ",
        "Stretch_Blit_Blend_BL         ",
        "Stretch_Blit_Rotate           ",
        "Stretch_Blit_Rotate_BL        ",
        "Textured_Triangle_PS          ",
        "Textured_Triangle_Blend_PS    ",
        "Textured_Triangle_BL          ",
        "Textured_Triangle_Blend_BL    ",
        "Textured_Quad_PS              ",
        "Textured_Quad_Blend_PS        ",
        "Textured_Quad_BL              ",
        "Textured_Quad_Blend_BL        "
    };



    if (testno <= 0) {
        for (testno = 1; testno <= TEST_MAX; ++testno) {
            aprint("Testno: ");
            nprintd(testno);
            aprint(tests_str[testno]);
            aprint("\n");
        }
    } else {
        char cur_test_str[100] = "      ";

        if (testno >= 10) {
            int testx0 = testno/10;
            cur_test_str[0] = testx0 + '0';
        }

        int test0x = testno%10;
        cur_test_str[1] = test0x + '0';

        ts_strcat(cur_test_str, tests_str[testno]);

        aprint(cur_test_str);
    }
}

int parse_params(int* testno, execution_mode* exec_mode, int argc, char* argv[])
{
    *testno = tb_testno();
    *exec_mode = tb_testmode();
    return 0;
}


static unsigned long s[] = {123456789, 362436069};
unsigned int get_random_value()
{
    uint64_t x = s[0];
    uint64_t const y = s[1];
    s[0] = y;
    x ^= x << 23; // a
    s[1] = x ^ y ^ (x >> 17) ^ (y >> 26); // b, c

    return s[1] + y;

//     return ts_rand();
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

uint32_t nema_readHwConfig(void);

void suite_init()
{
    nema_reg_write(0x94,(1U<<31));

#ifdef LOAD_TEXTURE_FROM_FS
    TSi_logo.bo = nema_load_file("./Sw/BareMetal/NemaGFX_SDK/examples/nemagfx_benchmarks/ThinkSiliconLogo200x104.rgba", TSi_logo.w*TSi_logo.h*4, 0);
#else
    TSi_logo.bo.base_virt = ThinkSiliconLogo200x104;
    TSi_logo.bo.base_phys = fb.bo.base_virt;
#endif

    // Load framebuffer
    fb.bo = nema_buffer_create_pool(NEMA_MEM_POOL_FB, fb.w*fb.h*4);
    nema_buffer_map(&fb.bo);

    /* char imp_path[300]; */
    /* char ref_path[300]; */

    /* ts_strcpy(imp_path,"Images/uNema/RGBX8888"); */
    /* ts_strcat(imp_path,"/imp"); */

    /* ts_strcpy(ref_path,"Images/uNema/RGBX8888"); */
    /* ts_strcat(ref_path,"/ref"); */

    uint32_t config = nema_readHwConfig();
    has_OCD = (config & 0xf000U) == 0U ? 0 : 1;
    ncores= (config & 0xf00U) >> 8U;

    // aprint("NCores: ");
    // nprintd(ncores);
    // aprint("\n");
}

void suite_terminate() {
    nema_buffer_destroy(&fb.bo);

#ifdef LOAD_TEXTURE_FROM_FS
    nema_buffer_destroy(&TSi_logo.bo);
#endif

    // theend();
}


void bench_start(int testno)
{
    start_clk = nema_get_time();

    //clear_background(0x0);
//

    if (has_OCD != 0U) {
        nema_ocd_write(NEMA_OCD_START|NEMA_OCD_CLEAR, NEMA_OCD_C_TOTAL);
        nema_ocd_write(NEMA_OCD_START|NEMA_OCD_CLEAR, NEMA_OCD_C_BUSY);
        nema_ocd_write(NEMA_OCD_START|NEMA_OCD_CLEAR, NEMA_OCD_C0_E_PIXELS);
        if (ncores > 1) {
            nema_ocd_write(NEMA_OCD_START|NEMA_OCD_CLEAR, NEMA_OCD_C1_E_PIXELS);
        }
        if (ncores > 2) {
            nema_ocd_write(NEMA_OCD_START|NEMA_OCD_CLEAR, NEMA_OCD_C2_E_PIXELS);
        }
        if (ncores > 2) {
            nema_ocd_write(NEMA_OCD_START|NEMA_OCD_CLEAR, NEMA_OCD_C3_E_PIXELS);
        }
    } else {
        aprint("Nema OCD is not active, cannot retrieve data regarding performance\n");
    }
}


void bench_stop(int testno, int pix_count)
{
    stop_clk = nema_get_time();

//    float total_cpu_us = stop_wall - start_wall;
//    float active_cpu_us = stop_clk - start_clk;

//    snapshot_name("benshim", testno);

    if (has_OCD != 0U) {
        print_test(testno);

        nema_ocd_stop();

        float active_gpu_us = nema_ocd_read_counter(NEMA_OCD_C_BUSY) / GPU_FREQ_MHZ;

        nema_ocd_write(NEMA_OCD_MAP|NEMA_OCD_STOP, NEMA_OCD_C0_E_PIXELS);
        pix_count = nema_ocd_read();

        if (ncores > 1) {
        nema_ocd_write(NEMA_OCD_MAP|NEMA_OCD_STOP, NEMA_OCD_C1_E_PIXELS);
        pix_count += nema_ocd_read();
        }

        if (ncores > 2) {
        nema_ocd_write(NEMA_OCD_MAP|NEMA_OCD_STOP, NEMA_OCD_C2_E_PIXELS);
        pix_count += nema_ocd_read();
        }

        if (ncores > 3) {
        nema_ocd_write(NEMA_OCD_MAP|NEMA_OCD_STOP, NEMA_OCD_C3_E_PIXELS);
        pix_count += nema_ocd_read();
        }

        char mpix_sec_str[] = "   .    Mpixels/sec  ";
        float mpix_sec = (float)pix_count / active_gpu_us;
        int mpix_sec_i = ((int)mpix_sec)%1000;
        int mpix_sec_f = ((int)(1000.f*mpix_sec))%1000;
        if (mpix_sec > 100.f) {
            int mpix_X00_000 = mpix_sec_i/100;
            mpix_sec_str[0] = mpix_X00_000 + '0';
            mpix_sec_i %= 100;
        }
        if (mpix_sec > 10.f) {
            int mpix_0X0_000 = mpix_sec_i/10;
            mpix_sec_str[1] = mpix_0X0_000 + '0';
            mpix_sec_i %= 10;
        }
        {
            int mpix_00X_000 = mpix_sec_i;
            mpix_sec_str[2] = mpix_00X_000 + '0';
        }

        {
            int mpix_000_X00 = mpix_sec_f/100;
            mpix_sec_str[4] = mpix_000_X00 + '0';
            mpix_sec_f %= 100;
        }
        {
            int mpix_000_0X0 = mpix_sec_f/10;
            mpix_sec_str[5] = mpix_000_0X0 + '0';
            mpix_sec_f %= 10;
        }
        {
            int mpix_000_00X = mpix_sec_f;
            mpix_sec_str[6] = mpix_000_00X + '0';
        }

        aprint(mpix_sec_str);
        aprint("\n");

//        nprintd();
//        aprint("GPU Utilization: ");
//        nprintd(active_gpu_us *100.0 / total_gpu_us);
//        aprint("\n");
    }
}


void bench_report(float perf, char *mes_unit, float utilization)
{
    /* printf("%2.2f %s \t", perf, mes_unit); */
    /* printf("Utilization: %2.2f %%\n",utilization); */
}
