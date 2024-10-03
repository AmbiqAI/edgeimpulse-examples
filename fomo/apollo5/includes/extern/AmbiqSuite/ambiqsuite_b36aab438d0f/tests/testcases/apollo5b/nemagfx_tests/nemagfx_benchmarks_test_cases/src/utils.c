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

#include "utils.h"
#include "nemagfx_test_common.h"
#include "Ambiq200x104.rgba.h"

extern nema_font_t DejaVuSerif12pt8b;
float start_clk;
float start_wall;
float stop_clk;
float stop_wall;
#ifndef GPU_FREQ_MHZ
#define GPU_FREQ_MHZ 50
#endif
gpu_test_cfg_t sCurrentCfg;
gpu_test_cfg_t sTestCfg[TEST_NUM] =
{
#ifdef USE_PSRAM
    {TEX_PSRAM, FB_SSRAM, WCACHE_OFF, TILING_OFF/*, MSPI_FULL_SPEED*/},
    {TEX_PSRAM, FB_SSRAM, WCACHE_OFF, TILING_ON/*, MSPI_FULL_SPEED*/},
    {TEX_PSRAM, FB_SSRAM, WCACHE_ON, TILING_OFF/*, MSPI_FULL_SPEED*/},
    {TEX_PSRAM, FB_SSRAM, WCACHE_ON, TILING_ON/*, MSPI_FULL_SPEED*/},

    {TEX_PSRAM, FB_PSRAM, WCACHE_OFF, TILING_OFF/*, MSPI_FULL_SPEED*/},
    {TEX_PSRAM, FB_PSRAM, WCACHE_OFF, TILING_ON/*, MSPI_FULL_SPEED*/},
    {TEX_PSRAM, FB_PSRAM, WCACHE_ON, TILING_OFF/*, MSPI_FULL_SPEED*/},
    {TEX_PSRAM, FB_PSRAM, WCACHE_ON, TILING_ON/*, MSPI_FULL_SPEED*/},
#endif // USE_PSRAM
    {TEX_SSRAM, FB_SSRAM, WCACHE_OFF, TILING_OFF/*, MSPI_FULL_SPEED*/},
    {TEX_SSRAM, FB_SSRAM, WCACHE_OFF, TILING_ON/*, MSPI_FULL_SPEED*/},
    {TEX_SSRAM, FB_SSRAM, WCACHE_ON, TILING_OFF/*, MSPI_FULL_SPEED*/},
    {TEX_SSRAM, FB_SSRAM, WCACHE_ON, TILING_ON/*, MSPI_FULL_SPEED*/}
};

TLS_VAR img_obj_t fb = {{0}, (FB_RESX / 4) * 4, (FB_RESY / 4) * 4, -1, 0, NEMA_RGB24, 0};
TLS_VAR img_obj_t Ambiq_logo = {{0}, 200, 104, -1, 0, NEMA_RGBA8888, 0};

static void print_test(int testno);
static void bench_report(float perf, char *mes_unit, float utilization);

const uint32_t expected_crc[TEST_MAX + 1] = 
{
    0xffffffff,//"_",
    0xF918ECD9,//"Fill_Triangle              ",
    0xE4AB6455,//"Fill_Triangle_Blend        ",
    0x62AA9B74,//"Fill_Rectangle             ",
    0xBD4C819A,//"Fill_Rectangle_Blend       ",
    0x090AA891,//"Fill_Quad                  ",
    0x5E01F3D4,//"Fill_Quad_Blend            ",
    0x1E6FCD74,//"Draw_String                ",
    0xAB33863C,//"Draw_Line                  ",
    0xCB5A1E05,//"Draw_Line_Blend            ",
    0xB9AEA67B,//"Draw_Rectangle            ",
    0xA62BE68F,//"Draw_Rectangle_Blend      ",
    0xB1451343,//"Blit                      ",
    0xDE512632,//"Blit_Colorize             ",
    0xBD708A3C,//"Blit_Blend                ",
    0x8EC3B072,//"Blit_Blend_Colorize       ",
    0x432AB723,//"Blit_90                   ",
    0xADF74F91,//"Blit_180                  ",
    0x5AE4BD62,//"Blit_270                  ",
    0x8901A909,//"Blit_Vertical_Flip        ",
    0x6121BE7D,//"Blit_Horizontal_Flip      ",
    0x4216B437,//"Blit_SRC_Colorkeyed       ",
    0xC7183CA5,//"Blit_DST_Colorkeyed       ",
    0xC6E44FD9,//"Stretch_Blit_PS           ",
    0x62CABF03,//"Stretch_Blit_Blend_PS     ",
    0x5584033E,//"Stretch_Blit_BL           ",
    0x87816938,//"Stretch_Blit_Blend_BL     ",
    0x5153821A,//"Stretch_Blit_Rotate       ",
    0xA4264B57,//"Stretch_Blit_Rotate_BL    ",
    0x3D6335CC,//"Textured_Triangle_PS      ",
    0x050C5B73,//"Textured_Triangle_Blend_PS",
    0x46DA2A07,//"Textured_Triangle_BL      ",
    0xC5013701,//"Textured_Triangle_Blend_BL",
    0xBC5D8AE9,//"Textured_Quad_PS          ",
    0xE3701E96,//"Textured_Quad_Blend_PS    ",
    0x6702460F,//"Textured_Quad_BL          ",
    0xD585BD8E,//"Textured_Quad_Blend_BL    "
};

bool crc_result = true;

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

// static int _memcpy(uint8_t* dst, const uint8_t* src, uint32_t nbytes)
// {
//     uint32_t i;
//     int count = 0;
//     for (i = 0; i < nbytes; i++)
//     {
//         dst[i] = src[i];
//         count++;
//     }

//     return count;
// }

static void clear_background(uint32_t col)
{
    // Create CmdList
    static TLS_VAR nema_cmdlist_t cl;
    cl = nema_cl_create();
    // Bind CmdList
    nema_cl_bind(&cl);
    // Bind Framebuffer
    nema_bind_dst_tex(fb.bo.base_phys, fb.w, fb.h, (nema_tex_format_t)(fb.format), fb.stride);
    nema_set_clip(0, 0, fb.w, fb.h);
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

    if (sCurrentCfg.eTexLocation == TEX_PSRAM)
    {
        Ambiq_logo.bo.base_phys = (uintptr_t)MSPI_XIP_BASE_ADDRESS;
        Ambiq_logo.bo.base_virt = (void*)Ambiq_logo.bo.base_phys;
        nema_memcpy(Ambiq_logo.bo.base_virt, Ambiq200x104, Ambiq_logo.w*Ambiq_logo.h*4);
    }
    else if (sCurrentCfg.eTexLocation == TEX_SSRAM)
    {
        Ambiq_logo.bo = nema_buffer_create_pool(NEMA_MEM_POOL_ASSETS, Ambiq_logo.w*Ambiq_logo.h*4);

        nema_memcpy(Ambiq_logo.bo.base_virt, Ambiq200x104, Ambiq_logo.w*Ambiq_logo.h*4);
        nema_buffer_flush(&Ambiq_logo.bo);
    }
    else
    {
        am_util_stdio_printf("Invalid texture location setting!\n");
    }

    if (sCurrentCfg.eFbLocation == FB_PSRAM)
    {
        // Create frame buffer and adjust its alignment.
        // Note, frame buffer should alige to 32bytes to avoid potential display issue.
        fb.bo.base_phys = ((uintptr_t)(MSPI_XIP_BASE_ADDRESS + Ambiq_logo.w*Ambiq_logo.h*4 + DejaVuSerif12pt8b.bitmap_size + 31) >> 5) << 5;
        fb.bo.base_virt = (void*)fb.bo.base_phys;
        fb.bo.size = fb.w*fb.h*4;
    }
    else if (sCurrentCfg.eFbLocation == FB_SSRAM)
    {
        // Load framebuffer
        fb.bo = nema_buffer_create_pool(NEMA_MEM_POOL_FB, fb.w*fb.h*4);
        nema_buffer_map(&fb.bo);
    }
    else
    {
        am_util_stdio_printf("Invalid FB location setting!\n");
    }


    // Default value when NemaP Boots
    // bit [31]: WCACHE ENABLE
    // bits [30:16]: TIMEOUT
    // TIMEOUT: big value, lose performance in lines/strings
    // TIMEOUT: small value, regular flushes
    if (sCurrentCfg.eWcache == WCACHE_ON)
    {
        nema_reg_write(0xe4, 0x80640000);
    }
    else if (sCurrentCfg.eWcache == WCACHE_OFF)
    {
        nema_reg_write(0xe4, 0x00640000);
    }
    else
    {
        am_util_stdio_printf("Invalid texture location setting!\n");
    }

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

#ifdef USE_DISPLAY
    am_devices_display_transfer_frame(fb.w, fb.h, fb.bo.base_phys, NULL, NULL);
    //
    // wait transfer done
    //
    am_devices_display_wait_transfer_done();
#endif

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

#ifdef USE_SOFT_CRC32
    //
    // We should invalidate the cache before checking data.
    //
    am_hal_cachectrl_range_t Range;
    Range.ui32Size = fb.h * nema_stride_size(fb.format, NEMA_TEX_CLAMP, fb.w);
    Range.ui32StartAddr = fb.bo.base_phys;
    am_hal_cachectrl_dcache_invalidate(&Range, false);
    //Run the CRC Check
    uint32_t crc32_value = crc32(fb.bo.base_virt, fb.h * nema_stride_size(fb.format, NEMA_TEX_CLAMP, fb.w));
    if (crc32_value != expected_crc[testno])
    {
        crc_result = false;
        am_util_stdio_printf("Framebuffer content check failed! Expected:%08X, Got:%08X\n", expected_crc[testno], crc32_value);
    }
#endif

    // sleep(1);
}

