//*****************************************************************************
//
//! @file utils.c
//!
//! @brief NemaGFX example.
//!
//! utils.c define and implement several functions that include initialize frame
//! buffer(abbreviation FB),printing step and performance information for users
//! to easily understand how to applicate GPU.
//!
//
//*****************************************************************************

//*****************************************************************************
//
// Copyright (c) 2024, Ambiq Micro, Inc.
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// 1. Redistributions of source code must retain the above copyright notice,
// this list of conditions and the following disclaimer.
//
// 2. Redistributions in binary form must reproduce the above copyright
// notice, this list of conditions and the following disclaimer in the
// documentation and/or other materials provided with the distribution.
//
// 3. Neither the name of the copyright holder nor the names of its
// contributors may be used to endorse or promote products derived from this
// software without specific prior written permission.
//
// Third party software included in this distribution is subject to the
// additional license terms as defined in the /docs/licenses directory.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
// ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
// LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
// CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
// SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
// INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
// CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
// ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
// POSSIBILITY OF SUCH DAMAGE.
//
// This is part of revision release_a5b_sdk2-748191cd0 of the AmbiqSuite Development Package.
//
//*****************************************************************************

#include "nemagfx_benchmarks.h"
#include "utils.h"
#include "Ambiq200x104.rgba.h"

//*****************************************************************************
//
// Variables definitions
//
//*****************************************************************************
float start_wall;
float stop_wall;

TLS_VAR img_obj_t g_sAmbiqLogo = {{0}, 200, 104, -1, 0, NEMA_RGBA8888, 0};

TLS_VAR img_obj_t g_sFB = {{0}, (FB_RESX / 4) * 4, (FB_RESY / 4) * 4, -1, 0, NEMA_RGB24, 0};

//*****************************************************************************
//
//! @brief print GPU's operation name
//!
//! @param i32TestNo - Selects operation types.
//!
//! @return None.
//
//*****************************************************************************
static void
print_test(int32_t i32TestNo)
{
    const char *pcTestsStr[TEST_MAX + 1] =
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

    if (i32TestNo <= 0)
    {
        for (i32TestNo = 1; i32TestNo <= TEST_MAX; ++i32TestNo)
        {
            am_util_stdio_printf("%d    %s\r\n", i32TestNo, pcTestsStr[i32TestNo]);
        }
    }
    else if (i32TestNo > TEST_MAX)
    {
        am_util_stdio_printf("%d is not a valid test\n", i32TestNo);
    }
    else
    {
        am_util_stdio_printf("%d: %s    ", i32TestNo, pcTestsStr[i32TestNo]);
    }
}

//*****************************************************************************
//
//! @brief setting background color with filling.
//!
//! @param col - target color with RGBA format.
//!
//! @return None.
//
//*****************************************************************************
static void
clear_background(uint32_t col)
{
    //
    // Create CmdList
    //
    static TLS_VAR nema_cmdlist_t cl;
    cl = nema_cl_create();
    //
    // Bind CmdList
    //
    nema_cl_bind(&cl);
    //
    // Bind Framebuffer
    //
    nema_bind_dst_tex(g_sFB.bo.base_phys, g_sFB.w, g_sFB.h, (nema_tex_format_t)(g_sFB.format), g_sFB.stride);
    nema_set_clip(0, 0, FB_RESX, FB_RESY);
    //
    // set color
    //
    nema_clear(col);

    nema_cl_unbind();
    nema_cl_submit(&cl);
    nema_cl_wait(&cl);

    nema_cl_destroy(&cl);
}

//*****************************************************************************
//
//! @brief initialize frame buffer
//!
//! this function will loop forever when create buffer failed.
//!
//! @return None.
//
//*****************************************************************************
void
suite_init()
{
    g_sAmbiqLogo.bo = nema_buffer_create_pool(NEMA_MEM_POOL_ASSETS, g_sAmbiqLogo.w * g_sAmbiqLogo.h * 4);
    if ( g_sAmbiqLogo.bo.base_virt == NULL )
    {
        am_util_stdio_printf("Failed to create FB!\n");
        while(1);
    }
    nema_memcpy(g_sAmbiqLogo.bo.base_virt, ui8Ambiq200x104, g_sAmbiqLogo.w * g_sAmbiqLogo.h * 4);
    nema_buffer_flush(&g_sAmbiqLogo.bo);
    //
    // Load framebuffer
    //
    g_sFB.bo = nema_buffer_create_pool(NEMA_MEM_POOL_FB, g_sFB.w * g_sFB.h * 4);
    if ( g_sFB.bo.base_virt == NULL )
    {
        am_util_stdio_printf("Failed to create FB!\n");
        while(1);
    }
    nema_buffer_map(&g_sFB.bo);
}

//*****************************************************************************
//
//! @brief destroy frame buffer memories.
//!
//! @return None.
//
//*****************************************************************************
void
suite_terminate()
{
    nema_buffer_destroy(&g_sFB.bo);
    nema_buffer_destroy(&g_sAmbiqLogo.bo);
}

//*****************************************************************************
//
//! @brief clean framebuffer memory,printing GPU's future step operation.
//!
//! @param i32TestNo - Selects target GPU operation.
//!
//! This function called before GPU start operation to pre-print information
//!
//! @return None.
//
//*****************************************************************************
void
bench_start(int32_t i32TestNo)
{
    clear_background(0x0);
    print_test(i32TestNo);
}

//*****************************************************************************
//
//! @brief convert float to String and print
//!
//! @param perf      - target float value.
//! @param pcMesUnit - measurement unit pointer.
//!
//! This function could convert float value to string,also could append measurement
//! unit.
//!
//! @return None.
//
//*****************************************************************************
void
bench_report(float perf, char *pcMesUnit)
{
    am_util_stdio_printf("%.2f %s\n", perf, pcMesUnit);
}

//*****************************************************************************
//
//! @brief convert float to String and print
//!
//! @param i32TestNo      - GPU's performance of index.
//! @param i32PixCount    - The total number of pixels rendered by the current
//!                         operation of GPU
//!
//! this function could transfer frame to display panel and printing GPU performance
//!
//! @return None.
//
//*****************************************************************************
void
bench_stop(int i32TestNo, int i32PixCount)
{
    nema_buffer_flush(&g_sFB.bo);
    //
    // transfer frame to the display
    //
    am_devices_display_transfer_frame(g_sFB.w,
                                      g_sFB.h,
                                      g_sFB.bo.base_phys,
                                      NULL, NULL);
    //
    // wait transfer done
    //
    am_devices_display_wait_transfer_done();

    float total_cpu_s  = stop_wall - start_wall;

    if (i32TestNo == 7)
    {
        //
        // test 7 - draw_strings - is measured in KChars/sec
        //
        bench_report(i32PixCount / 1000.f / total_cpu_s, " KChars/sec");
    }
    else if (i32TestNo == 37 || i32TestNo == 38 )
    {
        bench_report(i32PixCount / 1000.f / total_cpu_s, " KCLs/sec");
    }
    else
    {
        bench_report(i32PixCount / 1000.f / 1000.f / total_cpu_s, "MPixels/sec");
    }
}

