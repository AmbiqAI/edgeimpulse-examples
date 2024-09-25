//*****************************************************************************
//
//! @file nemagfx_blend.c
//!
//! @brief NemaGFX example.
//! A brief Example that demonstrates GPU blend feature.
//! Blending requires a series of calculations between the source (foreground)
//! and destination (background)color fragments for producing the final color,
//! which will be written in memory.This example use a constent table inside
//! most of the supported blending mode.demonstrates each more every 1 second.
//! the dst color is nema_rgba(0xff, 0, 0, 0x80), which is red color with 50%
//! alpha blending, the src color is nema_rgba(0, 0, 0xff, 0x80), which is blue
//! color with 50% alpha blending.
//!
//! Printing takes place over the ITM at 1M Baud.

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

#include "nemagfx_blend.h"
#include "nema_error.h"

#include "homer_rgba.h"
#include "greekisland_200x133_rgba.h"
#include "pic_48x48_rgba.h"
#include "pic_136x136_rgba8888.h"

//
// whether control display on/off when power on/off DC module
//
//#define DISPLAY_ON_OFF

//*****************************************************************************
//
// Variables Definition.
//
//*****************************************************************************
static img_obj_t            g_sFB;
static uint32_t next =      1;
static nemadc_layer_t       g_sDCLayer = {0};
//
// blend modes
//
const uint32_t g_ui32BlendMode[] =
{
    NEMA_BL_SIMPLE,
    NEMA_BL_CLEAR,
    NEMA_BL_SRC,
    NEMA_BL_SRC_OVER,
    NEMA_BL_DST_OVER,
    NEMA_BL_SRC_IN,
    NEMA_BL_DST_IN,
    NEMA_BL_SRC_OUT,
    NEMA_BL_DST_OUT,
    NEMA_BL_SRC_ATOP,
    NEMA_BL_DST_ATOP,
    NEMA_BL_ADD,
    NEMA_BL_XOR
};

//*****************************************************************************
//
//! @brief RTC interrupt function.
//!
//! This function could clear RTC alarm interrpt status register.
//!
//! @return None.
//
//*****************************************************************************
void
am_rtc_isr(void)
{
    static uint32_t i = 0;
    //
    // Clear the RTC alarm interrupt.
    //
    am_hal_rtc_interrupt_clear(AM_HAL_RTC_INT_ALM);

    am_util_stdio_printf("into interrupt %d\n", i++);
}

//*****************************************************************************
//
//! @brief RTC initialize function.
//!
//! This function configure RTC clock source,alarm interval,enable corresponding interrupt.
//!
//! @return None.
//
//*****************************************************************************
void
rtc_init(void)
{
    //
    // Enable the XT for the RTC.
    //
    am_hal_clkgen_control(AM_HAL_CLKGEN_CONTROL_RTC_SEL_XTAL, 0);

    //
    // Select XT for RTC clock source
    //
    am_hal_rtc_osc_select(AM_HAL_RTC_OSC_XT);

    am_hal_rtc_osc_enable();
        //
    // Set the alarm repeat interval to be every second.
    //
    am_hal_rtc_alarm_interval_set(AM_HAL_RTC_ALM_RPT_SEC);

    //
    // Clear the RTC alarm interrupt.
    //
    am_hal_rtc_interrupt_clear(AM_HAL_RTC_INT_ALM);

    //
    // Enable the RTC alarm interrupt.
    //
    am_hal_rtc_interrupt_enable(AM_HAL_RTC_INT_ALM);

    //
    // Enable GPIO interrupts to the NVIC.
    //
    NVIC_SetPriority(RTC_IRQn, AM_IRQ_PRIORITY_DEFAULT);
    NVIC_EnableIRQ(RTC_IRQn);

    //
    // Enable interrupts to the core.
    //
    am_hal_interrupt_master_enable();
    am_hal_rtc_interrupt_clear(AM_HAL_RTC_INT_ALM);
}

//*****************************************************************************
//
//! @brief initialize framebuffer, layer of DC if have
//!
//! @param  ui8Format           - display format,invalid for mspi drive display
//!
//! This function initialize FB size,format etc.it also initialize DC's layer if have
//!
//! @return None.
//
//*****************************************************************************
void
fb_reload_rgba(uint8_t ui8Format)
{
    g_sFB.w = RESX;
    g_sFB.h = RESY;
    g_sFB.color = 0;
    g_sFB.sampling_mode = 0;
    if ( ui8Format == NEMA_RGBA4444 )
    {
        g_sFB.format = NEMA_RGBA4444;
        g_sDCLayer.format = NEMADC_RGBA4444;
    }
    else if ( ui8Format == NEMA_RGBA5650 )
    {
        g_sFB.format = NEMA_RGBA5650;
        g_sDCLayer.format = NEMADC_RGB565;
    }
    else if ( ui8Format == NEMA_RGBA5551 )
    {
        g_sFB.format = NEMA_RGBA5551;
        g_sDCLayer.format = NEMADC_RGBA5551;
    }
    else if ( ui8Format == NEMA_RGB24 )
    {
        g_sFB.format = NEMA_RGB24;
        g_sDCLayer.format = NEMADC_RGB24;
    }
    else
    {
        g_sFB.format = NEMA_RGBA8888;
        g_sDCLayer.format = NEMADC_RGBA8888;
    }

    g_sFB.stride = nema_format_size(g_sFB.format) * g_sFB.w;
    g_sFB.bo = nema_buffer_create_pool(NEMA_MEM_POOL_FB, g_sFB.stride * g_sFB.h);
    memset((void*)(g_sFB.bo.base_phys), 0, g_sFB.stride * g_sFB.h);

    g_sDCLayer.stride = nemadc_stride_size(g_sDCLayer.format, RESX);
    g_sDCLayer.baseaddr_virt = (void*)g_sFB.bo.base_virt;
    g_sDCLayer.baseaddr_phys = (uintptr_t)g_sFB.bo.base_phys;
    g_sDCLayer.resx = RESX;
    g_sDCLayer.resy = RESY;
    g_sDCLayer.startx = 0;
    g_sDCLayer.starty = 0;
    g_sDCLayer.sizex = RESX;
    g_sDCLayer.sizey = RESY;
    g_sDCLayer.alpha = 0xFF;
    g_sDCLayer.blendmode = NEMADC_BL_SRC;
    g_sDCLayer.buscfg = 0;
    g_sDCLayer.mode = 0;
    g_sDCLayer.u_base = 0;
    g_sDCLayer.v_base = 0;
    g_sDCLayer.u_stride = 0;
    g_sDCLayer.v_stride = 0;
    g_sDCLayer.flipx_en = 0;
    g_sDCLayer.flipy_en = 0;
}

//*****************************************************************************
//
//! @brief release framebuffer
//!
//! @return None.
//
//*****************************************************************************
void
fb_release(void)
{
    nema_buffer_destroy(&g_sFB.bo);
}

//*****************************************************************************
//
//! @brief transfer frame data to panel
//!
//! This function block processor until the action is completed.
//!
//! @return None.
//
//*****************************************************************************
void
transfer_frame(void)
{
    nemadc_set_layer(0, &g_sDCLayer);
    nemadc_transfer_frame_prepare(g_sDispCfg.eTEType != DISP_TE_DISABLE);
    if ( g_sDispCfg.eTEType == DISP_TE_DISABLE )
    {
        //
        //It's necessary to launch frame manually when TE is disabled.
        //
        nemadc_transfer_frame_launch();
    }
    nemadc_wait_vsync();
#ifdef DISPLAY_ON_OFF
    nemadc_mipi_cmd_write(MIPI_set_display_on, NULL, 0, true, false);
    am_util_delay_ms(200);
#endif
}

//*****************************************************************************
//
//! @brief demonstrate blend modes
//!
//! This function demonstrate various of blend modes
//!
//! @return None.
//
//*****************************************************************************
void
test_blend_mode(void)
{
    nema_cmdlist_t sCL;
    //
    // Create Command Lists
    //
    sCL = nema_cl_create();
    //
    // Bind Command List
    //
    nema_cl_bind(&sCL);
    //
    // Bind Framebuffer
    //
    nema_bind_dst_tex(g_sFB.bo.base_phys, g_sFB.w, g_sFB.h, g_sFB.format, g_sFB.stride);
    //
    // Set Clipping Rectangle
    //
    nema_set_clip(0, 0, RESX, RESY);
    for (uint32_t ui32I = 0; ui32I < 13; ui32I++)
    {
        nema_set_blend_fill(NEMA_BL_SIMPLE);
        nema_fill_rect(0, 0, RESX / 2, RESY / 2, nema_rgba(0xff, 0, 0, 0x80));
        nema_set_blend_fill(g_ui32BlendMode[ui32I]);
        nema_fill_rect(RESX / 4, RESY / 4, RESX / 2 , RESY / 2, nema_rgba(0, 0, 0xff, 0x80));

        nema_cl_submit(&sCL);
        nema_cl_wait(&sCL);
        nema_cl_rewind(&sCL);

        transfer_frame();

        nema_clear(0x00000000);
        am_util_delay_ms(200);
    }

    nema_cl_destroy(&sCL);
}

//*****************************************************************************
//
//! @brief demonstrate Source/Destination Color Key
//!
//! This function demonstrate Source/Destination Color Key.
//! for Source Color Key,it will draw only when src color doesn't match colorkey
//! for Destination Color Key,it will draw only when dst color matches colorkey
//!
//! @return None.
//
//*****************************************************************************
void
tsuite2d_srcdstkey(void)
{
    nema_cmdlist_t sCL = nema_cl_create();
    nema_cl_bind(&sCL);

    nema_bind_dst_tex(g_sFB.bo.base_phys, g_sFB.w, g_sFB.h, g_sFB.format, g_sFB.stride);
    nema_clear(0x00000000);

    img_obj_t sObjHomerRGBA = {{0}, 32, 72, 32 * 4, 0, NEMA_RGBA8888, 0};

    sObjHomerRGBA.bo = nema_buffer_create_pool(NEMA_MEM_POOL_ASSETS, i32HomerRGBALength);
    nema_memcpy((void*)sObjHomerRGBA.bo.base_phys, ui8HomerRGBA, i32HomerRGBALength);

    nema_bind_src_tex(sObjHomerRGBA.bo.base_phys,
                      sObjHomerRGBA.w,
                      sObjHomerRGBA.h,
                      sObjHomerRGBA.format,
                      sObjHomerRGBA.stride,
                      NEMA_FILTER_PS);
    nema_set_clip(0, 0, 360, 360);
    nema_set_blend_blit(NEMA_BL_SRC);
    nema_blit(50, 50);

    nema_set_blend_fill(NEMA_BL_SRC);
    nema_fill_rect(200, 200, 32, 72, OPAQUE(RED));
    nema_set_src_color_key(0xff00e100);
    nema_set_dst_color_key(RED);
    nema_set_blend_blit(NEMA_BL_SRC | NEMA_BLOP_DST_CKEY | NEMA_BLOP_SRC_CKEY);
    nema_blit(200, 200);

    nema_cl_unbind();
    nema_cl_submit(&sCL);
    nema_cl_wait(&sCL);

    transfer_frame();

    nema_buffer_destroy(&sObjHomerRGBA.bo);
    nema_cl_destroy(&sCL);
}

//*****************************************************************************
//
//! @brief demonstrate Destination Color Key
//!
//! This function demonstrate Destination Color Key.
//! for Destination Color Key,it will draw only when dst color matches colorkey
//!
//! @return None.
//
//*****************************************************************************
void
tsuite2d_dst_ckey(void)
{
    nema_cmdlist_t sCL = nema_cl_create();
    nema_cl_bind(&sCL);
    nema_bind_dst_tex(g_sFB.bo.base_phys, g_sFB.w, g_sFB.h, g_sFB.format, g_sFB.stride);
    nema_clear(0x00000000);
    img_obj_t sObjHomerRGBA = {{0}, 32, 72, 32 * 4, 0, NEMA_RGBA8888, 0};

    sObjHomerRGBA.bo = nema_buffer_create_pool(NEMA_MEM_POOL_ASSETS, i32HomerRGBALength);
    nema_memcpy((void*)sObjHomerRGBA.bo.base_phys, ui8HomerRGBA, i32HomerRGBALength);
    nema_set_clip(0, 0, RESX, RESY);
    nema_bind_src_tex(sObjHomerRGBA.bo.base_phys,
                  sObjHomerRGBA.w,
                  sObjHomerRGBA.h,
                  sObjHomerRGBA.format,
                  sObjHomerRGBA.stride,
                  NEMA_FILTER_PS);

    nema_set_blend_blit(NEMA_BL_SRC);
    nema_blit(50 - 40, 50 + 100);
    nema_blit(100 - 40, 50 + 100);

    nema_set_dst_color_key(0xff00e100);
    nema_set_blend_fill(NEMA_BL_SRC | NEMA_BLOP_DST_CKEY);
    nema_fill_rect(40 - 40, 40 + 100, 100, 100, OPAQUE(RED));

    nema_set_blend_fill (NEMA_BL_SRC);
    nema_fill_rect(200 - 80, 40 + 100, 202, 135, 0xff00e100);
    nema_set_blend_blit(NEMA_BL_SRC);
    nema_blit(250 - 80, 50 + 100);
    nema_blit(300 - 80, 50 + 100);

    img_obj_t sObjGreekIsland200x133RGBA = {{0}, 200, 133, 200 * 4, 0, NEMA_RGBA8888, 0};

    sObjGreekIsland200x133RGBA.bo = nema_buffer_create_pool(NEMA_MEM_POOL_ASSETS, ui32GreekIsland200x133RGBALength);
    nema_memcpy((void*)sObjGreekIsland200x133RGBA.bo.base_phys, ui8GreekIsland200x133RGBA, ui32GreekIsland200x133RGBALength);

    nema_bind_src_tex(sObjGreekIsland200x133RGBA.bo.base_phys,
                      sObjGreekIsland200x133RGBA.w,
                      sObjGreekIsland200x133RGBA.h,
                      sObjGreekIsland200x133RGBA.format,
                      sObjGreekIsland200x133RGBA.stride,
                      NEMA_FILTER_PS);

    nema_set_dst_color_key(0xff00e100);
    nema_set_blend_blit(NEMA_BL_SRC | NEMA_BLOP_DST_CKEY);
    nema_blit(201 - 80, 41 + 100);

    nema_cl_unbind();
    nema_cl_submit(&sCL);
    nema_cl_wait(&sCL);

    transfer_frame();

    nema_buffer_destroy(&sObjHomerRGBA.bo);
    nema_buffer_destroy(&sObjGreekIsland200x133RGBA.bo);

    nema_cl_destroy(&sCL);
}

//*****************************************************************************
//
//! @brief demonstrate stretch texture
//!
//! This function demonstrate how to stetch textures.
//!
//! @return None.
//
//*****************************************************************************
void
blit_texture_scale(void)
{
    nema_cmdlist_t sCL;
    img_obj_t sObjPic48x48RGBA = {{0}, 48, 48, -1, 0, NEMA_RGBA8888, 0};
    sCL = nema_cl_create();
    nema_cl_bind(&sCL);
    nema_bind_dst_tex(g_sFB.bo.base_phys, g_sFB.w, g_sFB.h, g_sFB.format, g_sFB.stride);

    sObjPic48x48RGBA.bo = nema_buffer_create_pool(NEMA_MEM_POOL_ASSETS, i32Pic48x48RGBALength);
    nema_memcpy((void*)sObjPic48x48RGBA.bo.base_phys, ui8Pic48x48RGBA, i32Pic48x48RGBALength);

    nema_bind_src_tex(sObjPic48x48RGBA.bo.base_phys,
                      sObjPic48x48RGBA.w,
                      sObjPic48x48RGBA.h,
                      sObjPic48x48RGBA.format,
                      sObjPic48x48RGBA.stride,
                      NEMA_FILTER_PS);
    nema_set_clip(0, 0, RESX, RESY);
    nema_set_blend_blit(NEMA_BL_SRC);
    nema_blit((RESX - 48) / 2, 20);

    nema_bind_src_tex(sObjPic48x48RGBA.bo.base_phys,
                      sObjPic48x48RGBA.w,
                      sObjPic48x48RGBA.h,
                      sObjPic48x48RGBA.format,
                      sObjPic48x48RGBA.stride,
                      NEMA_FILTER_PS);
    nema_blit_rect_fit(0, 100, 48 * 4, 48 * 4);

    nema_bind_src_tex(sObjPic48x48RGBA.bo.base_phys,
                      sObjPic48x48RGBA.w,
                      sObjPic48x48RGBA.h,
                      sObjPic48x48RGBA.format,
                      sObjPic48x48RGBA.stride,
                      NEMA_FILTER_BL);
    nema_blit_rect_fit(200, 100, 48 * 4, 48 * 4);

    nema_cl_submit(&sCL);
    nema_cl_wait(&sCL);

    transfer_frame();

    nema_buffer_destroy(&sObjPic48x48RGBA.bo);
    nema_cl_destroy(&sCL);
}

//*****************************************************************************
//
//! @brief demonstrate dithering effects.
//!
//! @param  dithering           - true means enable,false means disable dithering effects.
//!
//! This function demonstrate GPU's dethering feature.Applicable output formats
//! are RGBA5650,RGBA5551 or RGBA4444. Others have no effects.
//!
//! @return None.
//
//*****************************************************************************
void
test_gpu_dithering(bool dithering)
{
    img_obj_t sObj_pic_rgba = {{0}, 136, 136, -1, 0, NEMA_RGBA8888, 0};

    sObj_pic_rgba.bo = nema_buffer_create_pool(NEMA_MEM_POOL_ASSETS, sizeof(g_ui8Pic_136x136_rgba8888));
    nema_memcpy((void*)sObj_pic_rgba.bo.base_phys, g_ui8Pic_136x136_rgba8888, sizeof(g_ui8Pic_136x136_rgba8888));

    nema_cmdlist_t sCL;

    sCL = nema_cl_create();
    nema_cl_bind(&sCL);
    if ( dithering == true )
    {
        nema_bind_dst_tex(g_sFB.bo.base_phys, g_sFB.w, g_sFB.h, g_sFB.format | NEMA_DITHER, -1);
    }
    else
    {
        nema_bind_dst_tex(g_sFB.bo.base_phys, g_sFB.w, g_sFB.h, g_sFB.format, -1);
    }
    nema_clear(0x00000000);
    nema_bind_src_tex(sObj_pic_rgba.bo.base_phys,
                      sObj_pic_rgba.w,
                      sObj_pic_rgba.h,
                      sObj_pic_rgba.format,
                      sObj_pic_rgba.stride,
                      NEMA_FILTER_PS);
    nema_set_clip(0, 0, RESX, RESY);
    nema_set_blend_blit(NEMA_BL_SRC);
    nema_blit((RESX - sObj_pic_rgba.w) / 2, (RESY - sObj_pic_rgba.h) / 2);

    nema_cl_submit(&sCL);
    nema_cl_wait(&sCL);

    transfer_frame();

    nema_cl_unbind();
    nema_cl_destroy(&sCL);

    nema_buffer_destroy(&sObj_pic_rgba.bo);
}

void
ts_srand(uint32_t seed)
{
    next = seed;
}

uint32_t
ts_rand(void)
{
    next = next * 1103515245 + 12345;
    return (uint32_t)(next & 0x7fffffff);
}

//*****************************************************************************
//
//! @brief demonstrate triangle blend & AA effects.
//!
//! @param  iterations      - random number of triangles.
//! @param  blend           - blend modes
//! @param  aa              - true means enable antialias,false is disable.
//!
//! This function demonstrate GPU's AA & blend feature.
//!
//! @return None.
//
//*****************************************************************************
void
tsuite2d_triangle_blend(int32_t iterations, int32_t blend, int32_t aa)
{
    int32_t x1, x2, x3, y1, y2, y3;
    int32_t color = 0;
    int32_t alpha = 0;
    int32_t i = 0;
    int32_t rect_w = 64;
    int32_t rect_h = 64;

    nema_cmdlist_t cl = nema_cl_create();
    nema_cl_bind(&cl);

    ts_srand(1712);
    nema_set_blend_fill ( blend );
    nema_enable_aa(aa, aa, aa, aa);

    while ( i < iterations )
    {
        i = i + 1;
        x1 = ts_rand() % (RESX - rect_w);
        y1 = ts_rand() % (RESY - rect_h);


        x2 = x1 + rect_w - 1;
        y2 = y1 + rect_h - 1;

        x3 = x1;
        y3 = y1 + rect_h - 1;

        color = ts_rand() + ts_rand();
        color = ((color >> 24) & 0x000000ff) |
                ((color >>  8) & 0x0000ff00) |
                ((color <<  8) & 0x00ff0000) ;
        alpha = ts_rand() & 0xff;

        nema_fill_triangle(x1, y1, x2, y2, x3, y3, TRANSP(alpha, color));
    }

    nema_fill_triangle(  0,   0,   0, 100, 100,   0, TRANSP(alpha, color));
    nema_fill_triangle(640,   0, 540,   0, 640, 100, TRANSP(alpha, color));
    nema_fill_triangle(  0, 480, 100, 480,   0, 380, TRANSP(alpha, color));
    nema_fill_triangle(640, 480, 640, 380, 540, 480, TRANSP(alpha, color));
    //---------------------------------------------------------------------
    nema_enable_aa(0, 0, 0, 0);
    nema_cl_unbind();
    nema_cl_submit(&cl);
    nema_cl_wait(&cl);
    nema_cl_destroy(&cl);

    transfer_frame();
}

//*****************************************************************************
//
//! @brief demonstrates drawing lines with AA effects.
//!
//! This function demonstrates drawing lines with AA effects.
//!
//! @return None.
//
//*****************************************************************************
void
tsuite2d_lines_aa(void)
{
    nema_cmdlist_t cl = nema_cl_create();
    nema_cl_bind(&cl);
    nema_set_clip(0, 0, RESX, RESY);
    nema_bind_dst_tex(g_sFB.bo.base_phys, g_sFB.w, g_sFB.h, g_sFB.format, -1);
    nema_clear(0x00000000);
    //---------------------------------------------------------------------
    nema_draw_line_aa( 50.0f, 60.0f, 100.0f, 120.0f, 1.0f, 0xff0000ff);
    nema_draw_line_aa( 70.0f, 60.0f, 120.0f, 120.0f, 3.0f, 0xff0000ff);
    nema_draw_line_aa( 90.0f, 60.0f, 140.0f, 120.0f, 5.0f, 0xff0000ff);
    //---------------------------------------------------------------------
    nema_draw_line_aa( 150.0f, 60.0f, 200.0f, 100.0f,  7.0f, 0xff0000ff);
    nema_draw_line_aa( 190.0f, 60.0f, 250.0f, 130.0f,  9.0f, 0xff00ff00);
    nema_draw_line_aa( 220.0f, 60.0f, 280.0f, 160.0f, 10.0f, 0xffff0000);
    //---------------------------------------------------------------------

    nema_draw_line(50, 160, 100, 300, 0xff0000ff);
    nema_draw_line_aa(70.0f, 160.f, 120.f, 300.f, 1.f, 0xff0000ff);

    nema_fill_circle(150, 230, 50, 0xff0000ff);
    nema_fill_circle_aa(270.0f, 230.f, 50.f, 0xff0000ff);

    nema_cl_unbind();
    nema_cl_submit(&cl);
    nema_cl_wait(&cl);
    nema_cl_destroy(&cl);

    transfer_frame();
}

//*****************************************************************************
//
//! @brief demonstrates drawing circles with AA effects.
//!
//! This function demonstrates drawing circles with AA effects.
//!
//! @return None.
//
//*****************************************************************************
void
tsuite2d_circle_aa(void)
{
    uint16_t offset = 10;
    nema_cmdlist_t cl = nema_cl_create();
    nema_cl_bind(&cl);
    //---------------------------------------------------------------------
    nema_draw_circle_aa( offset + 50.0f,  150 + 60.0f,  0.0f, 20.0f,  0xff0000ff);
    nema_draw_circle_aa( offset + 150.0f, 150 + 60.0f, 20.0f,  0.0f,  0xff00ff00);
    nema_draw_circle_aa( offset + 250.0f, 150 + 60.0f, 20.0f, 20.0f,  0xffff0000);
    //---------------------------------------------------------------------
    nema_cl_unbind();
    nema_cl_submit(&cl);
    nema_cl_wait(&cl);
    nema_cl_destroy(&cl);
    transfer_frame();
}

//*****************************************************************************
//
//! @brief Enable display
//!
//! @note the panel exit sleep mode and display image.
//
//*****************************************************************************
int32_t
display_panel_enable(void)
{
#ifdef DISPLAY_ON_OFF
    nemadc_mipi_cmd_write(MIPI_exit_sleep_mode, NULL, 0, true, false);
    am_util_delay_ms(130);

    // nemadc_mipi_cmd_write(MIPI_set_display_on, NULL, 0, true, false);
    // am_util_delay_ms(200);
#endif
    return 0;
}

//*****************************************************************************
//
//! @brief Disable display
//!
//! @note the panel disbled and enter sleep mode.
//
//*****************************************************************************
int32_t
display_panel_disable(void)
{
#ifdef DISPLAY_ON_OFF
    nemadc_mipi_cmd_write(MIPI_set_display_off, NULL, 0, true, false);
    am_util_delay_ms(20);

    nemadc_mipi_cmd_write(MIPI_enter_sleep_mode, NULL, 0, true, false);
    am_util_delay_ms(120);
#endif
    return 0;
}

static uint32_t nema_dc_reg_config = 0;
static uint32_t nema_dc_reg_resxy  = 0;

static uint32_t dsi_reg_function   = 0;
static uint32_t dsi_reg_afetrim1   = 0;

//*****************************************************************************
//
//! @brief Disable DSI to reduce power consumption.
//!
//! Retained two important present registers value for DSI power on.
//!
//! @return true
//
//*****************************************************************************
static bool
dphy_power_down(void)
{
    bool status;
    am_hal_pwrctrl_periph_enabled(AM_HAL_PWRCTRL_PERIPH_DISPPHY, &status);
    if ( status )
    {
        //
        // Get DSI functional programming register
        //
        dsi_reg_function = DSI->DSIFUNCPRG;

        //
        // Get DSI trim register that included DSI frequency
        //
        dsi_reg_afetrim1 = DSI->AFETRIM1 & 0x0000007F;

        am_hal_dsi_napping(ULPS_PATTERN_ON);
    }
    return true;
}

//*****************************************************************************
//
//! @brief Enable DSI
//!
//! Resume the previous DSI configuration.
//!
//! @return bool.
//
//*****************************************************************************
static bool
dphy_power_up(void)
{
    if (dsi_reg_afetrim1 == 0 && dsi_reg_function == 0)
    {
        //
        // DSI configuration parameters are invalid.
        //
        return false;
    }

    uint32_t ui32FreqTrim = _FLD2VAL(DSI_AFETRIM1_AFETRIM1, dsi_reg_afetrim1);
    uint8_t ui8LanesNum = (uint8_t)_FLD2VAL(DSI_DSIFUNCPRG_DATALANES, dsi_reg_function);
    uint8_t ui8DBIBusWidth = (uint8_t)_FLD2VAL(DSI_DSIFUNCPRG_REGNAME, dsi_reg_function);
    if (4 == ui8DBIBusWidth)
    {
        ui8DBIBusWidth = 16;
    }
    else if (3 == ui8DBIBusWidth)
    {
        ui8DBIBusWidth = 8;
    }
    else if (2 == ui8DBIBusWidth)
    {
        ui8DBIBusWidth = 9;
    }
    else
    {
        return false;
    }

    am_hal_dsi_wakeup(ui8LanesNum, ui8DBIBusWidth, ui32FreqTrim, ULPS_PATTERN_ON);
    return true;
}

//*****************************************************************************
//
//! @brief Disable Display Controller
//!
//! @note Retained two important present registers value for DC power on.
//!
//! @return true.
//
//*****************************************************************************
static bool
dc_power_down(void)
{
    bool status;
    am_hal_pwrctrl_periph_enabled(AM_HAL_PWRCTRL_PERIPH_DISP, &status);
    if ( status )
    {
        //
        // Get DC present color format
        //
        nema_dc_reg_config = nemadc_reg_read(NEMADC_REG_INTERFACE_CFG);
        //
        // Get DC present resolution
        //
        nema_dc_reg_resxy = nemadc_reg_read(NEMADC_REG_RESXY);
        nemadc_reg_write(NEMADC_REG_CLKCTRL_CG, 0);
        nemadc_MIPI_CFG_out(0);
        nemadc_set_mode(0);
        am_hal_pwrctrl_periph_disable(AM_HAL_PWRCTRL_PERIPH_DISP);
    }
    return true;
}

//*****************************************************************************
//
//! @brief Enable & Initilize Display controller
//!
//! Resume the previous DC configuration
//!
//! @return bool
//
//*****************************************************************************
static bool
dc_power_up(void)
{
    bool status;
    am_hal_pwrctrl_periph_enabled(AM_HAL_PWRCTRL_PERIPH_DISP, &status);
    if ( !status )
    {
        if (nema_dc_reg_config == 0 && nema_dc_reg_resxy == 0)
        {
            //
            // DC configuration parameters are invalid.
            //
            return false;
        }

        am_hal_pwrctrl_periph_enable(AM_HAL_PWRCTRL_PERIPH_DISP);
        if (0 != nemadc_init())
        {
            //
            // Initialize DC failed.
            //
            return false;
        }

        nemadc_initial_config_t sDCConfig = {0};
        
        sDCConfig.ui32PixelFormat = nema_dc_reg_config & 0x1FF;
        uint32_t interface = sDCConfig.ui32PixelFormat >> 6 << 6;
        if ((interface == MIPICFG_PF_DBI8) || 
            (interface == MIPICFG_PF_DBI9) || 
            (interface == MIPICFG_PF_DBI16))
        {
           sDCConfig.eInterface = DISP_INTERFACE_DBIDSI;
        }
        else if(interface == MIPICFG_PF_QSPI)
        {
            sDCConfig.eInterface = DISP_INTERFACE_QSPI;
        }
        else if(interface == MIPICFG_PF_DSPI)
        {
            sDCConfig.eInterface = DISP_INTERFACE_DSPI;
        }
        else if(interface == MIPICFG_PF_SPI)
        {
            sDCConfig.eInterface = DISP_INTERFACE_SPI4;
        }
        else
        {
            //
            // Invalid interface.
            //
            return false;
        }

        sDCConfig.bTEEnable = (nema_dc_reg_config & MIPICFG_DIS_TE) != 0;

        sDCConfig.ui16ResX = (uint16_t)(nema_dc_reg_resxy >> 16);
        sDCConfig.ui16ResY = (uint16_t)(nema_dc_reg_resxy & 0xFFFF);

        nemadc_configure(&sDCConfig);
    }
    return true;
}

//*****************************************************************************
//
//! @brief Power off GPU
//!
//! @note after called this function,please don't operation any GPU registers
//! until enable and initial the GPU.
//!
//! @return 0: success. 
//!         -1: GPU is busy, try it later.
//
//*****************************************************************************
int32_t
gpu_power_down()
{
    bool status;
    am_hal_pwrctrl_periph_enabled(AM_HAL_PWRCTRL_PERIPH_GFX, &status);
    if ( status )
    {
        if(nema_reg_read(NEMA_STATUS) == 0)
        {
            // If GPU is not busy, we can power off the GPU safely.
            am_hal_pwrctrl_periph_disable(AM_HAL_PWRCTRL_PERIPH_GFX);
        }
        else
        {
            return -1;
        }
    }

    return 0;
}
//*****************************************************************************
//
//! @brief Power on GPU power and restore nemaGFX context.
//!
//! @return 0: GPU power have initialize completely.
//!         -1: GPU power initialize error.
//
//*****************************************************************************
int32_t
gpu_power_up()
{
    int32_t i32Ret = 0;
    bool enabled;
    am_hal_pwrctrl_periph_enabled(AM_HAL_PWRCTRL_PERIPH_GFX, &enabled);
    if ( !enabled )
    {
        am_hal_pwrctrl_periph_enable(AM_HAL_PWRCTRL_PERIPH_GFX);
        //
        // Initialize NemaGFX
        //
        nema_reinit();
        if(nema_get_error() != NEMA_ERR_NO_ERROR)
        {
            am_util_debug_printf("Nemagfx reinit error!\n");
            return -1;
        }

        nema_reset_last_cl_id();
    }
    return i32Ret;
}

int32_t
gfx_power_down(void)
{
    display_panel_disable();
    if (g_sDispCfg.eInterface == DISP_IF_DSI)
    {
        dphy_power_down();
    }
    else
    {
        am_hal_clkgen_control(AM_HAL_CLKGEN_CONTROL_DISPCLKSEL_OFF, NULL);
        am_hal_clkgen_control(AM_HAL_CLKGEN_CONTROL_DCCLK_DISABLE, NULL);
    }
    dc_power_down();
    gpu_power_down();
    return 0;
}

int32_t
gfx_power_up()
{
    gpu_power_up();
    dc_power_up();
    if (g_sDispCfg.eInterface == DISP_IF_DSI)
    {
        dphy_power_up();
    }
    else
    {
        am_hal_clkgen_control(AM_HAL_CLKGEN_CONTROL_DISPCLKSEL_HFRC96, NULL);
        am_hal_clkgen_control(AM_HAL_CLKGEN_CONTROL_DCCLK_ENABLE, NULL);
    }
    display_panel_enable();
    return 0;
}

//*****************************************************************************
//
// Main Function
//
//*****************************************************************************
int
main(void)
{
    uint32_t ui32MipiCfg = MIPICFG_16RGB888_OPT0;    //!< default config
    uint16_t ui16MinX, ui16MinY;
    nemadc_initial_config_t sDCConfig;
    am_devices_dc_xspi_raydium_config_t sDisplayPanelConfig;

    //
    // External power on
    //
    am_bsp_external_pwr_on();
    am_util_delay_ms(100);

    //
    // Configure the board for low power operation.
    //
    am_bsp_low_power_init();

    //
    //  Enable the I-Cache and D-Cache.
    //
    am_hal_cachectrl_icache_enable();
    am_hal_cachectrl_dcache_enable(true);

    //
    // Initialize the printf interface for ITM/SWO output.
    //
    am_bsp_itm_printf_enable();

    //
    // Clear the terminal and print the banner.
    //
    am_util_stdio_terminal_clear();
    //
    // Disable crypto
    //
    am_hal_pwrctrl_periph_disable(AM_HAL_PWRCTRL_PERIPH_CRYPTO);

    //
    // Set the display region to center
    //
    if (RESX < g_sDispCfg.ui16ResX)
    {
        sDisplayPanelConfig.ui16ResX = RESX;
    }
    else
    {
        sDisplayPanelConfig.ui16ResX = g_sDispCfg.ui16ResX;
    }
    ui16MinX = (g_sDispCfg.ui16ResX - sDisplayPanelConfig.ui16ResX) >> 1;
    ui16MinX = (ui16MinX >> 1) << 1;

    if (RESY < g_sDispCfg.ui16ResY)
    {
        sDisplayPanelConfig.ui16ResY = RESY;
    }
    else
    {
        sDisplayPanelConfig.ui16ResY = g_sDispCfg.ui16ResY;
    }
    ui16MinY = (g_sDispCfg.ui16ResY - sDisplayPanelConfig.ui16ResY) >> 1;
    ui16MinY = (ui16MinY >> 1) << 1;

    g_sDispCfg.eTEType = DISP_TE_DISABLE;

    sDCConfig.ui16ResX = sDisplayPanelConfig.ui16ResX;
    sDCConfig.ui16ResY = sDisplayPanelConfig.ui16ResY;
    sDCConfig.bTEEnable = (g_sDispCfg.eTEType == DISP_TE_DC);
    sDisplayPanelConfig.ui16MinX = ui16MinX + g_sDispCfg.ui16Offset;
    sDisplayPanelConfig.ui16MinY = ui16MinY;
    sDisplayPanelConfig.bTEEnable = (g_sDispCfg.eTEType != DISP_TE_DISABLE);
    sDisplayPanelConfig.bFlip = g_sDispCfg.bFlip;

    //
    // If you want to change display configurations in example, please change them here as below.
    //
    // g_sDispCfg.ui8NumLanes = 1;
    // g_sDispCfg.eDbiWidth = AM_HAL_DSI_DBI_WIDTH_8;
    // g_sDispCfg.eDsiFreq = AM_HAL_DSI_FREQ_TRIM_X12;

    am_bsp_disp_pins_enable();
    if (g_sDispCfg.eInterface == DISP_IF_DSI)
    {
        //
        // VDD18 control callback function
        //
        am_hal_dsi_register_external_vdd18_callback(am_bsp_external_vdd18_switch);
        //
        // Enable DSI power and configure DSI clock.
        //
        am_hal_dsi_init();
    }
    else
    {
        am_hal_clkgen_control(AM_HAL_CLKGEN_CONTROL_DISPCLKSEL_HFRC96, NULL);
        am_hal_clkgen_control(AM_HAL_CLKGEN_CONTROL_DCCLK_ENABLE, NULL);
    }
    am_hal_pwrctrl_periph_enable(AM_HAL_PWRCTRL_PERIPH_GFX);
    am_hal_pwrctrl_periph_enable(AM_HAL_PWRCTRL_PERIPH_DISP);

    nema_sys_init();
    //
    //Initialize NemaDC
    //
    if (nemadc_init() != 0)
    {
        return -2;
    }
    if (g_sDispCfg.eInterface == DISP_IF_DSI)
    {
        uint8_t ui8LanesNum = g_sDispCfg.ui8NumLanes;
        uint8_t ui8DbiWidth = g_sDispCfg.eDbiWidth;
        uint32_t ui32FreqTrim = g_sDispCfg.eDsiFreq;
        pixel_format_t eFormat = FMT_RGB888;
        if (am_hal_dsi_para_config(ui8LanesNum, ui8DbiWidth, ui32FreqTrim, false) != 0)
        {
            return -3;
        }
        switch (eFormat)
        {
            case FMT_RGB888:
                if (ui8DbiWidth == 16)
                {
                    ui32MipiCfg = MIPICFG_16RGB888_OPT0;
                }
                if (ui8DbiWidth == 8)
                {
                    ui32MipiCfg = MIPICFG_8RGB888_OPT0;
                }
                break;

            case FMT_RGB565:
                if (ui8DbiWidth == 16)
                {
                    ui32MipiCfg = MIPICFG_16RGB565_OPT0;
                }
                if (ui8DbiWidth == 8)
                {
                    ui32MipiCfg = MIPICFG_8RGB565_OPT0;
                }
                break;

            default:
                //
                // invalid color component index
                //
                return -3;
        }
    }

    //
    // Initialize the display
    //
    switch (g_sDispCfg.eInterface)
    {
        case DISP_IF_SPI4:
            am_devices_dc_xspi_raydium_hardware_reset();
            sDCConfig.eInterface = DISP_INTERFACE_SPI4;
            sDCConfig.ui32PixelFormat = MIPICFG_1RGB888_OPT0;
            sDisplayPanelConfig.ui32PixelFormat = sDCConfig.ui32PixelFormat;
            nemadc_configure(&sDCConfig);
            am_devices_dc_xspi_raydium_init(&sDisplayPanelConfig);
            break;
        case DISP_IF_DSPI:
            am_devices_dc_xspi_raydium_hardware_reset();
            sDCConfig.eInterface = DISP_INTERFACE_DSPI;
            sDCConfig.ui32PixelFormat = MIPICFG_2RGB888_OPT0;
            sDisplayPanelConfig.ui32PixelFormat = sDCConfig.ui32PixelFormat;
            nemadc_configure(&sDCConfig);
            am_devices_dc_xspi_raydium_init(&sDisplayPanelConfig);

            break;
        case DISP_IF_QSPI:
            am_devices_dc_xspi_raydium_hardware_reset();
            sDCConfig.eInterface = DISP_INTERFACE_QSPI;
            sDCConfig.ui32PixelFormat = MIPICFG_4RGB888_OPT0;
            sDisplayPanelConfig.ui32PixelFormat = sDCConfig.ui32PixelFormat;
            nemadc_configure(&sDCConfig);
            am_devices_dc_xspi_raydium_init(&sDisplayPanelConfig);
            break;
        case DISP_IF_DSI:
            am_devices_dc_dsi_raydium_hardware_reset();
            sDCConfig.eInterface = DISP_INTERFACE_DBIDSI;
            sDCConfig.ui32PixelFormat = ui32MipiCfg;
            sDisplayPanelConfig.ui32PixelFormat = sDCConfig.ui32PixelFormat;
            nemadc_configure(&sDCConfig);
            am_devices_dc_dsi_raydium_init(((am_devices_dc_dsi_raydium_config_t *) &sDisplayPanelConfig));
            break;
        default:
            ; //NOP
    }
    //
    // Global interrupt enable
    //
    am_hal_interrupt_master_enable();

    //
    // Initialize NemaGFX
    //
    nema_init();

    //
    // Power down and sleep
    //
    gfx_power_down();
#ifdef CPU_DEEP_SLEEP
    rtc_init();
    am_hal_sysctrl_sleep(AM_HAL_SYSCTRL_SLEEP_DEEP);
#endif

    //
    // Run the anti-aliasing test
    //
    gfx_power_up();
    // Anti-Aliasing test
    fb_reload_rgba(NEMA_RGBA8888);
    //tsuite2d_triangle_blend(10, NEMA_BL_SRC, 1);
    tsuite2d_lines_aa();
    //tsuite2d_circle_aa();
    fb_release();

    //
    // Power down and sleep
    //
    gfx_power_down();
#ifdef CPU_DEEP_SLEEP
    am_hal_sysctrl_sleep(AM_HAL_SYSCTRL_SLEEP_DEEP);
#endif

    //
    // Run the dithering test, dithering off
    //
    gfx_power_up();
    // dithering test
    fb_reload_rgba(NEMA_RGBA4444);
    test_gpu_dithering(false);
    fb_release();

    //
    // Power down and sleep
    //
    gfx_power_down();
#ifdef CPU_DEEP_SLEEP
    am_hal_sysctrl_sleep(AM_HAL_SYSCTRL_SLEEP_DEEP);
#endif

    //
    // Run the dithering test, dithering on
    //
    gfx_power_up();
    fb_reload_rgba(NEMA_RGBA4444);
    test_gpu_dithering(true);
    fb_release();

    //
    // Power down and sleep
    //
    gfx_power_down();
#ifdef CPU_DEEP_SLEEP
    am_hal_sysctrl_sleep(AM_HAL_SYSCTRL_SLEEP_DEEP);
#endif

    //
    // Run blend mode test
    //
    gfx_power_up();
    fb_reload_rgba(NEMA_RGBA8888);
    test_blend_mode();
    fb_release();

    //
    // Power down and sleep
    //
    gfx_power_down();
#ifdef CPU_DEEP_SLEEP
    am_hal_sysctrl_sleep(AM_HAL_SYSCTRL_SLEEP_DEEP);
#endif

    //
    // Run the source color key test
    //
    gfx_power_up();
    fb_reload_rgba(NEMA_RGBA8888);
    tsuite2d_srcdstkey();
    fb_release();

    //
    // Power down and sleep
    //
    gfx_power_down();
#ifdef CPU_DEEP_SLEEP
    am_hal_sysctrl_sleep(AM_HAL_SYSCTRL_SLEEP_DEEP);
#endif

    //
    // Run the destination color key test
    //
    gfx_power_up();
    fb_reload_rgba(NEMA_RGBA8888);
    tsuite2d_dst_ckey();
    fb_release();

    //
    // Power down and sleep
    //
    gfx_power_down();
#ifdef CPU_DEEP_SLEEP
    am_hal_sysctrl_sleep(AM_HAL_SYSCTRL_SLEEP_DEEP);
#endif

    //
    // Run the texture strench test
    //
    gfx_power_up();
    fb_reload_rgba(NEMA_RGBA8888);
    blit_texture_scale();
    fb_release();

    while (1)
    {
    }
}
