//*****************************************************************************
//
//! @file nemagfx_blend.c
//!
//! @brief NemaGFX Blend Example.
//!
//! @addtogroup graphics_examples Graphics Examples
//!
//! @defgroup nemagfx_blend NemaGFX Blend Example
//! @ingroup graphics_examples
//! @{
//!
//! Purpose: Example that demonstrates blend feature
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
// ${copyright}
//
// This is part of revision ${version} of the AmbiqSuite Development Package.
//
//*****************************************************************************

#include "nemagfx_blend.h"

#include "homer_rgba.h"
#include "greekisland_200x133_rgba.h"
#include "pic_48x48_rgba.h"
#include "pic_136x136_rgba8888.h"

//*****************************************************************************
//
// Variables Definition.
//
//*****************************************************************************
static img_obj_t            g_sFB;
static bool is_power_up =   false;
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
    }
    else if ( ui8Format == NEMA_RGBA5650 )
    {
        g_sFB.format = NEMA_RGBA5650;
    }
    else if ( ui8Format == NEMA_RGBA5551 )
    {
        g_sFB.format = NEMA_RGBA5551;
    }
    else if( ui8Format == NEMA_RGB24 )
    {
        g_sFB.format = NEMA_RGB24;
    }
    else
    {
        g_sFB.format = NEMA_RGBA8888;
    }

    g_sFB.stride = nema_format_size(g_sFB.format) * g_sFB.w;
    g_sFB.bo = nema_buffer_create(g_sFB.stride * g_sFB.h);
    memset((void*)(g_sFB.bo.base_phys), 0, g_sFB.stride * g_sFB.h);

    if ( ui8Format == NEMA_RGBA4444 )
    {
        g_sDCLayer.format = NEMADC_RGBA4444;
    }
    else if ( ui8Format == NEMA_RGBA5650 )
    {
        g_sDCLayer.format = NEMADC_RGB565;
    }
    else if ( ui8Format == NEMA_RGBA5551 )
    {
        g_sDCLayer.format = NEMADC_RGBA5551;
    }
    else if( ui8Format == NEMA_RGB24 )
    {
        g_sDCLayer.format = NEMADC_RGB24;
    }
    else
    {
        g_sDCLayer.format = NEMADC_RGBA8888;
    }

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
        nemadc_set_layer(0, &g_sDCLayer);

        nemadc_transfer_frame_prepare(false);
        //
        //It's necessary to launch frame manually when TE is disabled.
        //
        nemadc_transfer_frame_launch();
        nemadc_wait_vsync();

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
#ifdef LOAD_TO_SSRAM
    sObjHomerRGBA.bo = nema_buffer_create(i32HomerRGBALength);
    memcpy((void*)sObjHomerRGBA.bo.base_phys, ui8HomerRGBA, i32HomerRGBALength);
#else
    sObjHomerRGBA.bo.base_phys = (uintptr_t)ui8HomerRGBA;
    sObjHomerRGBA.bo.base_virt = (void*)sObjHomerRGBA.bo.base_phys;
#endif
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
    nemadc_set_layer(0, &g_sDCLayer);

    nemadc_transfer_frame_prepare(false);
    //
    //It's necessary to launch frame manually when TE is disabled.
    //
    nemadc_transfer_frame_launch();
    nemadc_wait_vsync();

#ifdef LOAD_TO_SSRAM
    nema_buffer_destroy(&sObjHomerRGBA.bo);
#endif
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
#ifdef LOAD_TO_SSRAM
    sObjHomerRGBA.bo = nema_buffer_create(i32HomerRGBALength);
    memcpy((void*)sObjHomerRGBA.bo.base_phys, ui8HomerRGBA, i32HomerRGBALength);
#else
    sObjHomerRGBA.bo.base_phys = (uintptr_t)ui8HomerRGBA;
    sObjHomerRGBA.bo.base_virt = (void*)sObjHomerRGBA.bo.base_phys;
#endif
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
#ifdef LOAD_TO_SSRAM
    sObjGreekIsland200x133RGBA.bo = nema_buffer_create(ui32GreekIsland200x133RGBALength);
    memcpy((void*)sObjGreekIsland200x133RGBA.bo.base_phys, ui8GreekIsland200x133RGBA, ui32GreekIsland200x133RGBALength);
#else
    sObjGreekIsland200x133RGBA.bo.base_phys = (uintptr_t)ui8GreekIsland200x133RGBA;
    sObjGreekIsland200x133RGBA.bo.base_virt = (void*)sObjGreekIsland200x133RGBA.bo.base_phys;
#endif
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

    nemadc_set_layer(0, &g_sDCLayer);

    nemadc_transfer_frame_prepare(false);
    //
    //It's necessary to launch frame manually when TE is disabled.
    //
    nemadc_transfer_frame_launch();
    nemadc_wait_vsync();

#ifdef LOAD_TO_SSRAM
    nema_buffer_destroy(&sObjHomerRGBA.bo);
    nema_buffer_destroy(&sObjGreekIsland200x133RGBA.bo);
#endif
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

#ifdef LOAD_TO_SSRAM
    sObjPic48x48RGBA.bo = nema_buffer_create(i32Pic48x48RGBALength);
    memcpy((void*)sObjPic48x48RGBA.bo.base_phys, ui8Pic48x48RGBA, i32Pic48x48RGBALength);
#else
    sObjPic48x48RGBA.bo.base_phys = (uintptr_t)ui8Pic48x48RGBA;
    sObjPic48x48RGBA.bo.base_virt = (void*)sObjPic48x48RGBA.bo.base_phys;
#endif
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
    nemadc_set_layer(0, &g_sDCLayer);

    nemadc_transfer_frame_prepare(false);
    //
    //It's necessary to launch frame manually when TE is disabled.
    //
    nemadc_transfer_frame_launch();
    nemadc_wait_vsync();
#ifdef LOAD_TO_SSRAM
    nema_buffer_destroy(&sObjPic48x48RGBA.bo);
#endif
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

    sObj_pic_rgba.bo = nema_buffer_create(sizeof(g_ui8Pic_136x136_rgba8888));
    memcpy((void*)sObj_pic_rgba.bo.base_phys, g_ui8Pic_136x136_rgba8888, sizeof(g_ui8Pic_136x136_rgba8888));

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

    nemadc_set_layer(0, &g_sDCLayer);
    nemadc_transfer_frame_prepare(false);
    //
    //It's necessary to launch frame manually when TE is disabled.
    //
    nemadc_transfer_frame_launch();
    nemadc_wait_vsync();

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

    nemadc_set_layer(0, &g_sDCLayer);

    nemadc_transfer_frame_prepare(false);
    //
    //It's necessary to launch frame manually when TE is disabled.
    //
    nemadc_transfer_frame_launch();
    nemadc_wait_vsync();
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
    nema_cl_unbind();
    nema_cl_submit(&cl);
    nema_cl_wait(&cl);
    nema_cl_destroy(&cl);

    nemadc_set_layer(0, &g_sDCLayer);
    nemadc_transfer_frame_prepare(false);
    //
    //It's necessary to launch frame manually when TE is disabled.
    //
    nemadc_transfer_frame_launch();
    nemadc_wait_vsync();
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
    nemadc_set_layer(0, &g_sDCLayer);
    nemadc_transfer_frame_prepare(false);
    //
    //It's necessary to launch frame manually when TE is disabled.
    //
    nemadc_transfer_frame_launch();
    nemadc_wait_vsync();
}

//*****************************************************************************
//
//! @brief this function used to disable D-PHY power and clocks then reduce
//! power consumption.
//!
//! @return 1- D-PHY power had disabled before,will not output entering
//!            ULPS sequence.
//!         0- D-PHY have output entering ULPS sequence(escape mode entry + LPDT command).
//
//*****************************************************************************
int32_t
dphy_power_down()
{
    bool status;
    am_hal_pwrctrl_periph_enabled(AM_HAL_PWRCTRL_PERIPH_DISPPHY, &status);
    if ( status )
    {
       am_hal_dsi_napping(false);
    }
    else
    {
        return 1;
    }
    return 0;
}
//*****************************************************************************
//
//! @brief this function used to enable D-PHY power and clocks and generate
//! exit escape sequence.
//!
//! @param ui8LanesNum      - D-PHY data lanes numbers.
//! @param ui8DBIBusWidth   - display bus interface width.
//! @param ui32FreqTrim     - frequency trim.
//!
//! @return 0.
//
//*****************************************************************************
int32_t
dphy_power_up(uint8_t ui8LanesNum, uint8_t ui8DBIBusWidth, uint32_t ui32FreqTrim)
{
    am_hal_dsi_wakeup(ui8LanesNum, ui8DBIBusWidth, ui32FreqTrim, false);
    return 0;
}
//*****************************************************************************
//
//! @brief this function used to disable DC power
//!
//! @note after called this function,please don't operation any DC registers
//! until enable and initial the DC.
//!
//! @return 1- DC power had disabled before
//!         0- DC power have disabled now.
//
//*****************************************************************************
int32_t
dc_power_down(void)
{
    bool status;
    am_hal_pwrctrl_periph_enabled(AM_HAL_PWRCTRL_PERIPH_DISP, &status);
    if ( status )
    {
        nemadc_MIPI_CFG_out(0);
        nemadc_set_mode(0);
        am_hal_pwrctrl_periph_disable(AM_HAL_PWRCTRL_PERIPH_DISP);
    }
    else
    {
        return 1;
    }
    return 0;
}
//*****************************************************************************
//
//! @brief this function used to enable DC power and initilize DC
//!
//! @param ui32PixelFormat   graphics pixel format.
//!
//! @return 0- DC power have initialize completely.
//!         other - DC initallize error.
//
//*****************************************************************************
int32_t
dc_power_up(uint32_t ui32PixelFormat)
{
    int32_t i32Ret = 1;
    bool status;
    am_hal_pwrctrl_periph_enabled(AM_HAL_PWRCTRL_PERIPH_DISP, &status);
    if ( !status )
    {
        am_hal_pwrctrl_periph_enable(AM_HAL_PWRCTRL_PERIPH_DISP);
        i32Ret = nemadc_init();
        if (i32Ret != 0)
        {
            return i32Ret;
        }

        switch (g_sDispCfg.eInterface)
        {
            case DISP_IF_SPI4:

                nemadc_MIPI_CFG_out(MIPICFG_SPI4 | MIPICFG_SPI_CSX_V | MIPICFG_DBI_EN |
                                    MIPICFG_RESX | MIPICFG_1RGB888_OPT0 | MIPICFG_DIS_TE);
                nemadc_timing(RESX, 4, 10, 10,
                              RESY, 10, 50, 10);
                break;

            case DISP_IF_DSPI:
                nemadc_MIPI_CFG_out(MIPICFG_DSPI | MIPICFG_SPI4 | MIPICFG_SPI_CSX_V |
                                    MIPICFG_DBI_EN | MIPICFG_RESX |
                                    MIPICFG_2RGB888_OPT0 | MIPICFG_DIS_TE);
                nemadc_timing(RESX, 4, 10, 10,
                              RESY, 10, 50, 10);
                break;

            case DISP_IF_QSPI:
                nemadc_MIPI_CFG_out(MIPICFG_QSPI | MIPICFG_SPI4 | MIPICFG_SPI_CSX_V |
                                    MIPICFG_DBI_EN | MIPICFG_RESX |
                                    MIPICFG_4RGB888_OPT0 | MIPICFG_DIS_TE);
                nemadc_timing(RESX, 4, 10, 10,
                              RESY, 10, 50, 10);
                break;

            case DISP_IF_DSI:
            default:
                //
                // Enable dc clock
                //
                nemadc_reg_write(NEMADC_REG_CLKCTRL_CG, NemaDC_clkctrl_cg_clk_en);
                //
                // Set clock divider. B2 and later versions of Apollo4 support setting DC primary clock divide ratio to 1.
                //
                if (APOLLO4_GE_B2)
                {
                    nemadc_clkdiv(1, 1, 4, 0);
                }
                else
                {
                    nemadc_clkdiv(2, 1, 4, 0);
                }
                //
                // Enable fast pixel generation slow transfer
                //
                if (APOLLO4_GE_B2)
                {
                    nemadc_reg_write(NEMADC_REG_CLKCTRL_CG,
                                    (NemaDC_clkctrl_cg_clk_swap |
                                     NemaDC_clkctrl_cg_l0_bus_clk |
                                     NemaDC_clkctrl_cg_clk_en));
                }
                else
                {
                    nemadc_reg_write(NEMADC_REG_CLKCTRL_CG,
                                    (NemaDC_clkctrl_cg_clk_swap |
                                     NemaDC_clkctrl_cg_clk_en));
                }
                nemadc_clkctrl((uint32_t)TB_LCDPANEL_MIPI_DBIB );
                //
                // Program NemaDC MIPI interface
                //
                if (APOLLO4_GE_B2)
                {
                    nemadc_MIPI_CFG_out(MIPICFG_DBI_EN          |
                                        MIPICFG_RESX            |
                                        MIPICFG_EXT_CTRL        |
                                        MIPICFG_EN_STALL        |
#if (!defined(ENABLE_DC_TE) || defined(ENABLE_GPIO_TE))
                                        MIPICFG_DIS_TE          | // comment out this line when TE is enabled
#endif
                                        MIPICFG_PIXCLK_OUT_EN   |
                                        ui32PixelFormat
                    );
                }
                else
                {
                    nemadc_MIPI_CFG_out(MIPICFG_DBI_EN          |
                                        MIPICFG_RESX            |
                                        MIPICFG_EXT_CTRL        |
#if (!defined(ENABLE_DC_TE) || defined(ENABLE_GPIO_TE))
                                        MIPICFG_DIS_TE          | // comment out this line when TE is enabled
#endif
                                        MIPICFG_PIXCLK_OUT_EN   |
                                        ui32PixelFormat
                    );
                }
                //
                // Program NemaDC to transfer a resx*resy region
                //
                nemadc_timing(RESX, 4, 10, 1,
                              RESY, 1, 1, 1);
                break;
        }
    }

    return i32Ret;
}

//*****************************************************************************
//
//! @brief this function used to disable GPU power
//!
//! @note after called this function,please don't operation any GPU registers
//! until enable and initial the GPU.
//!
//! @return 1- GPU power had disabled before
//!         0- GPU power have disabled now.
//
//*****************************************************************************
int32_t
gpu_power_down()
{
    bool status;
    am_hal_pwrctrl_periph_enabled(AM_HAL_PWRCTRL_PERIPH_GFX, &status);
    if ( status )
    {
        //
        // Disable clock
        //
        nema_reg_write(NEMA_CGCTRL, 0);
        //
        // Reset GPU status
        //
        nema_reg_write(NEMA_STATUS, 0xFFFFFFFF);
        am_hal_pwrctrl_periph_disable(AM_HAL_PWRCTRL_PERIPH_GFX);
    }
    else
    {
        return 1;
    }
    return 0;
}
//*****************************************************************************
//
//! @brief this function used to enable GPU power and initialize nemaGFX
//!
//! @return 0- GPU power have initialize completely.
//!         other- GPU power initialize error.
//
//*****************************************************************************
int32_t
gpu_power_up()
{
    int32_t i32Ret = 1;
    bool status;
    am_hal_pwrctrl_periph_enabled(AM_HAL_PWRCTRL_PERIPH_GFX, &status);
    if ( !status )
    {
        am_hal_pwrctrl_periph_enable(AM_HAL_PWRCTRL_PERIPH_GFX);
        //
        // Initialize NemaGFX
        //
        i32Ret = nema_init();
    }
    return i32Ret;
}

int32_t
gfx_power_down(void)
{
    if ( is_power_up == false )
    {
        return 1;
    }
    is_power_up = false;

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
gfx_power_up(uint32_t ui32PixelFormat)
{
    if ( is_power_up == true )
    {
        return 1;
    }
    is_power_up = true;
    gpu_power_up();

    dc_power_up(ui32PixelFormat);
    if (g_sDispCfg.eInterface == DISP_IF_DSI)
    {
        dphy_power_up(g_sDispCfg.ui8NumLanes, g_sDispCfg.eDbiWidth, g_sDispCfg.eDsiFreq);
    }
    else
    {
        am_hal_clkgen_control(AM_HAL_CLKGEN_CONTROL_DISPCLKSEL_HFRC96, NULL);
        am_hal_clkgen_control(AM_HAL_CLKGEN_CONTROL_DCCLK_ENABLE, NULL);
    }
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
#ifndef AM_PART_APOLLO4L
    am_hal_pwrctrl_dsp_memory_config_t sExtSRAMMemCfg =
    {
        .bEnableICache      = false,
        .bRetainCache       = false,
        .bEnableRAM         = true,
        .bActiveRAM         = false,
        .bRetainRAM         = true
    };
    am_hal_pwrctrl_dsp_memory_config(AM_HAL_DSP0, &sExtSRAMMemCfg);
    am_hal_pwrctrl_dsp_memory_config(AM_HAL_DSP1, &sExtSRAMMemCfg);
#endif
    am_hal_cachectrl_config(&am_hal_cachectrl_defaults);
    am_hal_cachectrl_enable();
    am_bsp_low_power_init();
    //
    // Disable crypto
    //
    am_hal_pwrctrl_periph_disable(AM_HAL_PWRCTRL_PERIPH_CRYPTO);
// #### INTERNAL BEGIN ####
    //
    // Reset BLE
    //
//    am_hal_gpio_pinconfig(42, am_hal_gpio_pincfg_output);
//    am_hal_gpio_state_write(42, AM_HAL_GPIO_OUTPUT_SET);
//    am_hal_gpio_state_write(42, AM_HAL_GPIO_OUTPUT_CLEAR);
// #### INTERNAL END ####

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
            sDisplayPanelConfig.ui32PixelFormat = ui32MipiCfg;
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

    is_power_up = true;

    gfx_power_down();
#ifdef CPU_DEEP_SLEEP
    rtc_init();
    am_hal_sysctrl_sleep(AM_HAL_SYSCTRL_SLEEP_DEEP);
#endif
    gfx_power_up(ui32MipiCfg);
    // Anti-Aliasing test
    fb_reload_rgba(NEMA_RGBA8888);
    //tsuite2d_triangle_blend(10, NEMA_BL_SRC, 1);
    tsuite2d_lines_aa();
    //tsuite2d_circle_aa();
    fb_release();
    gfx_power_down();
#ifdef CPU_DEEP_SLEEP
    am_hal_sysctrl_sleep(AM_HAL_SYSCTRL_SLEEP_DEEP);
#endif
    gfx_power_up(ui32MipiCfg);
    // dithering test
    fb_reload_rgba(NEMA_RGBA4444);
    test_gpu_dithering(false);
    fb_release();
    gfx_power_down();

#ifdef CPU_DEEP_SLEEP
    am_hal_sysctrl_sleep(AM_HAL_SYSCTRL_SLEEP_DEEP);
#endif
    gfx_power_up(ui32MipiCfg);
    // dithering test
    fb_reload_rgba(NEMA_RGBA4444);
    test_gpu_dithering(true);
    fb_release();
    gfx_power_down();

#ifdef CPU_DEEP_SLEEP
    am_hal_sysctrl_sleep(AM_HAL_SYSCTRL_SLEEP_DEEP);
#endif
    gfx_power_up(ui32MipiCfg);
    fb_reload_rgba(NEMA_RGBA8888);
    test_blend_mode();
    fb_release();
    gfx_power_down();

#ifdef CPU_DEEP_SLEEP
    am_hal_sysctrl_sleep(AM_HAL_SYSCTRL_SLEEP_DEEP);
#endif
    gfx_power_up(ui32MipiCfg);

    fb_reload_rgba(NEMA_RGBA8888);
    tsuite2d_srcdstkey();
    fb_release();

    gfx_power_down();

#ifdef CPU_DEEP_SLEEP
    am_hal_sysctrl_sleep(AM_HAL_SYSCTRL_SLEEP_DEEP);
#endif
    gfx_power_up(ui32MipiCfg);

    fb_reload_rgba(NEMA_RGBA8888);
    tsuite2d_dst_ckey();
    fb_release();

    gfx_power_down();

#ifdef CPU_DEEP_SLEEP
    am_hal_sysctrl_sleep(AM_HAL_SYSCTRL_SLEEP_DEEP);
#endif
    gfx_power_up(ui32MipiCfg);
    fb_reload_rgba(NEMA_RGBA8888);
    blit_texture_scale();
    fb_release();

    while (1)
    {
    }
}

//*****************************************************************************
//
// End Doxygen group.
//! @}
//
//*****************************************************************************

