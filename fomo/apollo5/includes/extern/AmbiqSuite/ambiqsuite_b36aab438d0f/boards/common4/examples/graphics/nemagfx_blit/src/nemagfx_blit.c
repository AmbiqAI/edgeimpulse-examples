//*****************************************************************************
//
//! @file nemagfx_blit.c
//!
//! @brief NemaGFX Blit Example.
//!
//! @addtogroup graphics_examples Graphics Examples
//!
//! @defgroup nemagfx_blit NemaGFX Blit Example
//! @ingroup graphics_examples
//! @{
//!
//! Purpose: This example uses two frame buffer demonstrate a digital Quartz clock, with
//! Nema GPU support, the shader effect continue shows while timer passing, the
//! function needs a timer to get the accurate time past.
//! AM_DEBUG_PRINTF
//! If enabled, debug messages will be sent over ITM.
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
#include "nemagfx_blit.h"
#include "math.h"
#include "am_devices_mspi_psram_aps12808l.h"

#include "simhei36pt1b.h"
#include "simhei36pt4b.h"
#include "simhei36pt8b.h"
#include "arial48pt1b.h"

#include "psram_malloc.h"

#include "greek_island_100x100_alpha_rgba.h"
#include "greek_island_100x100_rgba565.h"
#include "greek_island_100x100_tsc6.h"
#include "greek_island_100x100_tsc4.h"
#include "greek_island_100x100_tsc6a.h"
#include "greek_island_200x200_rgba.h"
#include "greek_island_32x32_rgba.h"

#include "hour_20x250_rgba.h"
#include "hour_20x250_tsc6.h"
#include "hour_20x250_tsc6a.h"
#include "fz250_rgb24_100x100.h"
#include "fz250_99x99_rgba.h"
#include "monkey_trans_48x48_tsc6a.h"
#include "girl_tsc6.h"
#include "watchface_star_360x360_tsc6a.h"
#include "sport_150x168_rgba565.h"
#include "oli_360x360_alpha_rgba5551.h"

#define AM_DEBUG_PRINTF

#define BLACK     0xFF000000
#define GREEN     0xFF00FF00
#define RED       0xFF0000FF
#define YELLOW    0xFF00FFFF
#define WHITE     0xFFFFFFFF

static img_obj_t g_sFB = {{0}, RESX, RESY, -1, 0, NEMA_RGBA8888, 0};
img_obj_t g_sGreekIslandRGBA8888 = {{0}, 100, 100, -1, 0, NEMA_RGBA8888, 0};
img_obj_t g_sGreekIslandRGBA5650 = {{0}, 100, 100, -1, 0, NEMA_RGBA5650, 0};
img_obj_t g_sGreekIslandTSC6 = {{0}, 100, 100, -1, 0, NEMA_TSC6, 0};
img_obj_t g_sGreekIslandTSC4 = {{0}, 100, 100, -1, 0, NEMA_TSC4, 0};
img_obj_t g_sGreekIslandTSC6A = {{0}, 100, 100, -1, 0, NEMA_TSC6A, 0};

nemadc_layer_t g_sDCLayer = {(void *)0, 0, RESX, RESY, -1, 0, 0, RESX, RESY, 0xff, NEMADC_BL_SRC, 0, NEMADC_RGBA8888, 0, 0, 0, 0, 0, 0, 0, 0};

float start_time;
float end_time;

AM_SHARED_RW uint32_t        g_ui32DMATCBBuffer[2560];
void            *g_pPSRAMHandle;
void            *g_pMSPIHandle;

am_devices_mspi_psram_config_t MSPI_PSRAM_OctalCE1MSPIConfig =
{
    .eDeviceConfig            = AM_BSP_MSPI_PSRAM_MODULE_OCTAL_DDR_CE,
    .eClockFreq               = AM_HAL_MSPI_CLK_96MHZ,
    .ui32NBTxnBufLength       = sizeof(g_ui32DMATCBBuffer) / sizeof(uint32_t),
    .pNBTxnBuf                = g_ui32DMATCBBuffer,
    .ui32ScramblingStartAddr  = 0,
    .ui32ScramblingEndAddr    = 0,
};
//! MSPI interrupts.
static const IRQn_Type mspi_interrupts[] =
{
    MSPI0_IRQn,
    MSPI1_IRQn,
    MSPI2_IRQn,
};

//
// Take over the interrupt handler for whichever MSPI we're using.
//
#define psram_mspi_isr                                                          \
    am_mspi_isr1(MSPI_PSRAM_MODULE)
#define am_mspi_isr1(n)                                                        \
    am_mspi_isr(n)
#define am_mspi_isr(n)                                                         \
    am_mspi ## n ## _isr

//*****************************************************************************
//
// MSPI ISRs.
//
//*****************************************************************************
void psram_mspi_isr(void)
{
    uint32_t      ui32Status;

    am_hal_mspi_interrupt_status_get(g_pMSPIHandle, &ui32Status, false);

    am_hal_mspi_interrupt_clear(g_pMSPIHandle, ui32Status);

    am_hal_mspi_interrupt_service(g_pMSPIHandle, ui32Status);
}

void
test_nema_malloc(void)
{
    uint32_t i;

    for (i = 0; i < 100; i++)
    {
        g_sFB.bo = nema_buffer_create(360*360*3);
        nema_buffer_destroy(&g_sFB.bo);
    }
}

int32_t
align16(int32_t size)
{
/*
    if (size % 16 == 0){
        return size;
    } else {
        return size/16*16+32;
    }
*/

    return size;
}

void
fb_release(void)
{
    nema_buffer_destroy(&g_sFB.bo);
}

static void
frame_transfer(void)
{
    nemadc_transfer_frame_prepare(false);
    //
    //It's necessary to launch frame manually when TE is disabled.
    //
    nemadc_transfer_frame_launch();
    nemadc_wait_vsync();
}

void
load_objects(void)
{
    g_sFB.format = NEMA_RGBA8888;
    g_sFB.bo = nema_buffer_create(align16(g_sFB.w*g_sFB.h*4));

    g_sDCLayer.format = NEMADC_RGBA8888;
    g_sDCLayer.baseaddr_phys = g_sFB.bo.base_phys;
    g_sDCLayer.baseaddr_virt = g_sFB.bo.base_virt;

//    greek_islend.bo = nema_buffer_create(greek_islend_100x100_alpha_rgba_length);
//    nema_memcpy(greek_islend.bo.base_virt, g_ui8GreekIsland100x100RGBA, greek_islend_100x100_alpha_rgba_length);

    g_sGreekIslandRGBA8888.bo.base_phys = (uintptr_t)g_ui8GreekIsland100x100RGBA;
    g_sGreekIslandRGBA8888.bo.base_virt = (void*)g_sGreekIslandRGBA8888.bo.base_phys;

    g_sGreekIslandRGBA5650.bo.base_phys = (uintptr_t)g_ui8GreekIsland100x100RGBA565;
    g_sGreekIslandRGBA5650.bo.base_virt = (void*)g_sGreekIslandRGBA5650.bo.base_phys;

    g_sGreekIslandTSC6.bo.base_phys = (uintptr_t)g_ui8GreekIsland100x100TSC6;
    g_sGreekIslandTSC6.bo.base_virt = (void*)g_sGreekIslandTSC6.bo.base_phys;

    g_sGreekIslandTSC4.bo.base_phys = (uintptr_t)g_ui8GreekIsland100x100TSC4;
    g_sGreekIslandTSC4.bo.base_virt = (void*)g_sGreekIslandTSC4.bo.base_phys;

    am_util_stdio_printf("g_sFB use rgba8888\n");
}

void
load_objects_with_free(void)
{
    g_sFB.format = NEMA_RGBA8888;
    g_sFB.bo = nema_buffer_create(align16(g_sFB.w * g_sFB.h * 4));

    g_sDCLayer.format = NEMADC_RGBA8888;
    g_sDCLayer.baseaddr_phys = g_sFB.bo.base_phys;
    g_sDCLayer.baseaddr_virt = g_sFB.bo.base_virt;

//    greek_islend.bo = nema_buffer_create(greek_islend_100x100_alpha_rgba_length);
//    nema_memcpy(greek_islend.bo.base_virt, g_ui8GreekIsland100x100RGBA, greek_islend_100x100_alpha_rgba_length);

    g_sGreekIslandRGBA8888.bo.base_phys = (uintptr_t)g_ui8GreekIsland100x100RGBA;
    g_sGreekIslandRGBA8888.bo.base_virt = (void*)g_sGreekIslandRGBA8888.bo.base_phys;

    g_sGreekIslandRGBA5650.bo.base_phys = (uintptr_t)g_ui8GreekIsland100x100RGBA565;
    g_sGreekIslandRGBA5650.bo.base_virt = (void*)g_sGreekIslandRGBA5650.bo.base_phys;

    g_sGreekIslandTSC6.bo.base_phys = (uintptr_t)g_ui8GreekIsland100x100TSC6;
    g_sGreekIslandTSC6.bo.base_virt = (void*)g_sGreekIslandTSC6.bo.base_phys;

    g_sGreekIslandTSC4.bo.base_phys = (uintptr_t)g_ui8GreekIsland100x100TSC4;
    g_sGreekIslandTSC4.bo.base_virt = (void*)g_sGreekIslandTSC4.bo.base_phys;

    am_util_stdio_printf("g_sFB use rgba8888\n");
}

void
fill_round_rect_test(void)
{
    nema_cmdlist_t sCL;

    sCL = nema_cl_create();
    nema_cl_bind(&sCL);
    nema_bind_dst_tex(g_sFB.bo.base_phys, g_sFB.w, g_sFB.h, g_sFB.format, g_sFB.stride);

    nema_set_clip(0, 0, RESX, RESY);
    nema_set_blend_fill(NEMA_BL_SRC);

    nema_fill_rect(100, 100, 100, 100, GREEN);

    nema_cl_submit(&sCL);
    nema_cl_wait(&sCL);
    nemadc_set_layer(0, &g_sDCLayer);

    frame_transfer();

    nema_cl_destroy(&sCL);
}

void
tsuite2d_tileblit(void)
{

    img_obj_t obj_greek_island_32x32_rgba = {{0}, 32, 32, -1, 0, NEMA_RGBA8888, 0};

    int32_t tilex = RESX;
    int32_t tiley = RESY;
    nema_cmdlist_t sCL = nema_cl_create();
    nema_cl_bind(&sCL);
    nema_bind_dst_tex(g_sFB.bo.base_phys, g_sFB.w, g_sFB.h, g_sFB.format, g_sFB.stride);

    nema_set_clip(0, 0, RESX, RESY);

    obj_greek_island_32x32_rgba.bo = nema_buffer_create(sizeof(g_ui8GreekIsland32x32RGBA));
    if ( NULL == obj_greek_island_32x32_rgba.bo.base_phys )
    {
        while(1);           //error
    }
    memcpy((void*)obj_greek_island_32x32_rgba.bo.base_phys, g_ui8GreekIsland32x32RGBA, sizeof(g_ui8GreekIsland32x32RGBA));
    nema_bind_src_tex(obj_greek_island_32x32_rgba.bo.base_phys,
                      obj_greek_island_32x32_rgba.w,
                      obj_greek_island_32x32_rgba.h,
                      obj_greek_island_32x32_rgba.format,
                      obj_greek_island_32x32_rgba.stride,
                      NEMA_FILTER_PS | NEMA_TEX_REPEAT);

//    nema_set_blend_fill ( NEMA_BL_SRC );
//    nema_fill_rect(0, 0, tilex + 10, tiley + 10, 0xff0000ff);

    nema_set_blend_blit(NEMA_BL_SRC);
    nema_blit_rect(0, 0, tilex, tiley);
    nema_cl_unbind();
    start_time = nema_get_time();
    nema_cl_submit(&sCL);
    nema_cl_wait(&sCL);

    end_time = nema_get_time();
    am_util_stdio_printf("blit_texture_468x468 rgba8888 used %f s\n", end_time - start_time);

    nemadc_set_layer(0, &g_sDCLayer);
    frame_transfer();

    nema_cl_destroy(&sCL);

    nema_buffer_destroy(&obj_greek_island_32x32_rgba.bo);
}

void
blit_texture_100x100_tile(void)
{
    int32_t tilex = RESX;
    int32_t tiley = RESY;

    //---------------------------------------------------------------------
    nema_cmdlist_t sCL = nema_cl_create();
    nema_cl_bind(&sCL);
    nema_bind_dst_tex(g_sFB.bo.base_phys, g_sFB.w, g_sFB.h, g_sFB.format, g_sFB.stride);

    nema_set_clip(0, 0, RESX, RESY);

    g_sGreekIslandRGBA8888.bo = nema_buffer_create(sizeof(g_ui8GreekIsland100x100RGBA));
    if ( NULL == g_sGreekIslandRGBA8888.bo.base_phys )
    {
        while(1);           //error
    }
    memcpy((void*)g_sGreekIslandRGBA8888.bo.base_phys, g_ui8GreekIsland100x100RGBA, sizeof(g_ui8GreekIsland100x100RGBA));
    nema_bind_src_tex(g_sGreekIslandRGBA8888.bo.base_phys,
                      g_sGreekIslandRGBA8888.w,
                      g_sGreekIslandRGBA8888.h,
                      g_sGreekIslandRGBA8888.format,
                      g_sGreekIslandRGBA8888.stride,
                      NEMA_FILTER_PS | NEMA_TEX_REPEAT);

    nema_set_blend_blit(NEMA_BL_SRC);
    nema_blit_rect(0, 0, tilex, tiley);
    nema_cl_unbind();
    nema_cl_submit(&sCL);
    nema_cl_wait(&sCL);

    nemadc_set_layer(0, &g_sDCLayer);
    frame_transfer();

    nema_cl_destroy(&sCL);

    nema_buffer_destroy(&g_sGreekIslandRGBA8888.bo);
}

void
blit_texture_100x100(void)
{
    nema_cmdlist_t sCL;

    sCL = nema_cl_create();
    nema_cl_bind(&sCL);
    nema_bind_dst_tex(g_sFB.bo.base_phys, g_sFB.w, g_sFB.h, g_sFB.format, g_sFB.stride);

    g_sGreekIslandRGBA8888.bo = nema_buffer_create(sizeof(g_ui8GreekIsland100x100RGBA));
    if ( NULL == g_sGreekIslandRGBA8888.bo.base_phys )
    {
        while(1);           //error
    }
    memcpy((void*)g_sGreekIslandRGBA8888.bo.base_phys, g_ui8GreekIsland100x100RGBA, sizeof(g_ui8GreekIsland100x100RGBA));
    nema_bind_src_tex(g_sGreekIslandRGBA8888.bo.base_phys,
                      g_sGreekIslandRGBA8888.w,
                      g_sGreekIslandRGBA8888.h,
                      g_sGreekIslandRGBA8888.format,
                      g_sGreekIslandRGBA8888.stride,
                      NEMA_FILTER_PS);
    nema_set_clip(0, 0, RESX, RESY);
    nema_set_blend_blit(NEMA_BL_SRC);
    start_time = nema_get_time();
    // nema_blit_rotate(0, 0, NEMA_MIR_HOR);
    // nema_blit_rect_fit(0, 0, RESX, RESY);
    nema_blit(0, 0);

    nema_cl_submit(&sCL);
    nema_cl_wait(&sCL);

    end_time = nema_get_time();
    am_util_stdio_printf("blit_texture_100x100 rgba8888 used %f s\n", end_time - start_time);

    nemadc_set_layer(0, &g_sDCLayer);
    frame_transfer();

    nema_cl_unbind();
    nema_cl_destroy(&sCL);

    nema_buffer_destroy(&g_sGreekIslandRGBA8888.bo);
}

void blit_texture_200x200(void)
{
    img_obj_t obj_greek_islend_200x200_rgba = {{0}, 200, 200, -1, 0, NEMA_RGBA8888, 0};

    nema_cmdlist_t sCL;

    sCL = nema_cl_create();
    nema_cl_bind(&sCL);
    nema_bind_dst_tex(g_sFB.bo.base_phys, g_sFB.w, g_sFB.h, g_sFB.format, g_sFB.stride);

    obj_greek_islend_200x200_rgba.bo = nema_buffer_create(sizeof(g_ui8GreekIsland200x200RGBA));
    if ( NULL == obj_greek_islend_200x200_rgba.bo.base_phys )
    {
        while(1);           //error
    }
    memcpy((void*)obj_greek_islend_200x200_rgba.bo.base_phys, g_ui8GreekIsland200x200RGBA, sizeof(g_ui8GreekIsland200x200RGBA));
    nema_bind_src_tex(obj_greek_islend_200x200_rgba.bo.base_phys,
                      obj_greek_islend_200x200_rgba.w,
                      obj_greek_islend_200x200_rgba.h,
                      obj_greek_islend_200x200_rgba.format,
                      obj_greek_islend_200x200_rgba.stride,
                      NEMA_FILTER_PS);
    nema_set_clip(0, 0, RESX, RESY);
    nema_set_blend_blit(NEMA_BL_SRC);
    start_time = nema_get_time();
    // nema_blit_rotate(0, 0, NEMA_MIR_HOR);
    nema_blit(0, 0);

    nema_cl_submit(&sCL);
    nema_cl_wait(&sCL);

    end_time = nema_get_time();
    am_util_stdio_printf("blit_texture_200x200 rgba8888 used %f s\n", end_time - start_time);

    nemadc_set_layer(0, &g_sDCLayer);
    frame_transfer();

    nema_buffer_destroy(&obj_greek_islend_200x200_rgba.bo);
    nema_cl_unbind();
    nema_cl_destroy(&sCL);
}

void
blit_texture_360x360(void)
{
    img_obj_t obj_oli_360x360_alpha_rgba5551 = {{0}, 360, 360, -1, 0, NEMA_RGBA5551, 0};

    nema_cmdlist_t sCL;

    sCL = nema_cl_create();
    nema_cl_bind(&sCL);
    nema_bind_dst_tex(g_sFB.bo.base_phys, g_sFB.w, g_sFB.h, g_sFB.format, g_sFB.stride);

    obj_oli_360x360_alpha_rgba5551.bo = nema_buffer_create(sizeof(oli_360x360_alpha_rgba5551));
    if ( NULL == obj_oli_360x360_alpha_rgba5551.bo.base_phys )
    {
        while(1);           //error
    }
    memcpy((void*)obj_oli_360x360_alpha_rgba5551.bo.base_phys, oli_360x360_alpha_rgba5551, sizeof(oli_360x360_alpha_rgba5551));
    nema_bind_src_tex(obj_oli_360x360_alpha_rgba5551.bo.base_phys,
                      obj_oli_360x360_alpha_rgba5551.w,
                      obj_oli_360x360_alpha_rgba5551.h,
                      obj_oli_360x360_alpha_rgba5551.format,
                      obj_oli_360x360_alpha_rgba5551.stride,
                      NEMA_FILTER_PS);
    nema_set_clip(0, 0, RESX, RESY);
    nema_set_blend_blit(NEMA_BL_SRC | NEMA_BLOP_MODULATE_RGB);
	
    start_time = nema_get_time();
    // nema_blit_rotate(0, 0, NEMA_MIR_HOR);
    nema_blit(0, 0);

    nema_cl_submit(&sCL);
    nema_cl_wait(&sCL);

    end_time = nema_get_time();
    am_util_stdio_printf("blit_texture_360x360 rgba5551 used %f s\n", end_time - start_time);

    nemadc_set_layer(0, &g_sDCLayer);
    frame_transfer();

    nema_cl_unbind();
    nema_cl_destroy(&sCL);

    nema_buffer_destroy(&obj_oli_360x360_alpha_rgba5551.bo);
}

static void
load_font_objects(void)
{
    g_sSimhei36pt1b.bo = nema_buffer_create( align16(g_sSimhei36pt1b.bitmap_size) );
    (void)nema_buffer_map(&g_sSimhei36pt1b.bo);
    (void)nema_memcpy(g_sSimhei36pt1b.bo.base_virt, g_sSimhei36pt1b.bitmap, (size_t)g_sSimhei36pt1b.bitmap_size);

    g_sSimhei36pt4b.bo = nema_buffer_create( align16(g_sSimhei36pt4b.bitmap_size) );
    (void)nema_buffer_map(&g_sSimhei36pt4b.bo);
    (void)nema_memcpy(g_sSimhei36pt4b.bo.base_virt, g_sSimhei36pt4b.bitmap, (size_t)g_sSimhei36pt4b.bitmap_size);

    g_sSimhei36pt8b.bo = nema_buffer_create( align16(g_sSimhei36pt8b.bitmap_size) );
    (void)nema_buffer_map(&g_sSimhei36pt8b.bo);
    (void)nema_memcpy(g_sSimhei36pt8b.bo.base_virt, g_sSimhei36pt8b.bitmap, (size_t)g_sSimhei36pt8b.bitmap_size);
}

static void
release_font_objects(void)
{
    nema_buffer_destroy( &g_sSimhei36pt1b.bo );
    nema_buffer_destroy( &g_sSimhei36pt4b.bo );
    nema_buffer_destroy( &g_sSimhei36pt8b.bo );
}

void
blit_font(void)
{
    load_font_objects();

    nema_cmdlist_t sCL;
    //
    // Create Command Lists
    //
    sCL  = nema_cl_create();
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
    nema_set_blend_fill(NEMA_BL_SRC);
    //
    // Fill Rectangle with Color
    //
    nema_fill_rect(0, 0, RESX, RESY, 0x10101010);

    //char ui8Str[] = "鱀鱁\
    //              鱀鱁\n";

    char ui8Str[] = "鱀\n";
    int32_t w, h;

    nema_bind_font(&g_sSimhei36pt1b);
    nema_bind_font(&g_sSimhei36pt4b);
    nema_bind_font(&g_sSimhei36pt8b);

    const int32_t i32Spacing = 5;

    (void)nema_string_get_bbox(ui8Str, &w, &h, RESX / 2 - i32Spacing * 3, 1);
    h += i32Spacing;    w += i32Spacing;

    int32_t x, y;

    const int32_t xs[4] = {i32Spacing, w + 2 * i32Spacing, i32Spacing, w + 2 * i32Spacing};
    const int32_t ys[4] = {i32Spacing, i32Spacing, h + 2 * i32Spacing, h + 2 * i32Spacing};
    int32_t idx = 0;

#if USE_1BIT_FONT != 0
    x = xs[idx];
    y = ys[idx];
    ++idx;

    nema_set_blend_fill(NEMA_BL_SRC);
    nema_fill_rect(x, y, w, h, 0x70707070U);
    nema_bind_font(&g_sSimhei36pt1b);
    nema_print(ui8Str, x, y, w, h, 0xff00ff80U, NEMA_ALIGNX_LEFT | NEMA_TEXT_WRAP | NEMA_ALIGNY_TOP);

    nema_cl_submit(&sCL);
    (void)nema_cl_wait(&sCL);
    nemadc_set_layer(0, &g_sDCLayer);
    frame_transfer();
    nema_cl_rewind(&sCL);
#endif

#if USE_4BIT_FONT != 0
    x = xs[idx];
    y = ys[idx];
    ++idx;

    nema_set_blend_fill(NEMA_BL_SRC);
    nema_fill_rect(x, y, w, h, 0x70707070U);
    nema_bind_font(&g_sSimhei36pt4b);
    nema_print(ui8Str, x, y, w, h, 0xff00ff80U, NEMA_ALIGNX_LEFT   | NEMA_TEXT_WRAP | NEMA_ALIGNY_TOP);
    start_time = nema_get_time();
    nema_cl_submit(&sCL);
    (void)nema_cl_wait(&sCL);
    end_time = nema_get_time();
    am_util_stdio_printf("USE_4BIT_FONT used %f s\n", end_time - start_time);
    nemadc_set_layer(0, &g_sDCLayer);
    frame_transfer();
    nema_cl_rewind(&sCL);
#endif

#if USE_8BIT_FONT != 0

    x = xs[idx];
    y = ys[idx];
    ++idx;

    nema_set_blend_fill(NEMA_BL_SRC);
    nema_fill_rect(x, y, w, h, 0x70707070U);
    nema_bind_font(&g_sSimhei36pt8b);
    nema_print(ui8Str, x, y, w, h, 0xff00ff80U, NEMA_ALIGNX_LEFT   | NEMA_TEXT_WRAP | NEMA_ALIGNY_TOP);

    nema_cl_submit(&sCL);
    (void)nema_cl_wait(&sCL);
    nemadc_set_layer(0, &g_sDCLayer);
    frame_transfer();
#endif

    nema_cl_destroy(&sCL);
    release_font_objects();
}

void
blit_font_psram(void)
{
    load_font_objects();

    nema_cmdlist_t sCL;

    //Create Command Lists
    sCL  = nema_cl_create();

    //Bind Command List
    nema_cl_bind(&sCL);

    //Bind Framebuffer
    nema_bind_dst_tex(g_sFB.bo.base_phys, g_sFB.w, g_sFB.h, g_sFB.format, g_sFB.stride);
    //Set Clipping Rectangle
    nema_set_clip(0, 0, RESX, RESY);
    nema_set_blend_fill(NEMA_BL_SRC);
    //Fill Rectangle with Color
    nema_fill_rect(0, 0, RESX, RESY, 0x10101010);

    char ui8Str[] = "鱀鱁\
                  鱀鱁\n";
    int32_t w, h;

    const int32_t i32Spacing = 5;

    (void)nema_string_get_bbox(ui8Str, &w, &h, RESX / 2 - i32Spacing * 3, 1);
    h += i32Spacing;    w += i32Spacing;

    int32_t x, y;

    const int32_t xs[4] = {i32Spacing, w + 2 * i32Spacing, i32Spacing, w + 2 * i32Spacing};
    const int32_t ys[4] = {i32Spacing, i32Spacing, h + 2 * i32Spacing, h + 2 * i32Spacing};
    int32_t idx = 0;

#if USE_1BIT_FONT != 0
    x = xs[idx];
    y = ys[idx];
    ++idx;

    nema_set_blend_fill(NEMA_BL_SRC);
    nema_fill_rect(x, y, w, h, 0x70707070U);

    nema_bind_font(&g_sSimhei36pt1b);
    char *psram_simhei36pt1bBitmaps = psram_malloc(g_sSimhei36pt1b.bitmap_size);
    memcpy(psram_simhei36pt1bBitmaps, g_sSimhei36pt1b.bitmap, g_sSimhei36pt1b.bitmap_size);
    g_sSimhei36pt1b.bo.base_phys = (uintptr_t)psram_simhei36pt1bBitmaps;
    g_sSimhei36pt1b.bo.base_virt = (void*)g_sSimhei36pt1b.bo.base_phys;

    nema_print(ui8Str, x, y, w, h, 0xff00ff80U, NEMA_ALIGNX_LEFT   | NEMA_TEXT_WRAP | NEMA_ALIGNY_TOP);

    nema_cl_submit(&sCL);
    (void)nema_cl_wait(&sCL);
    nemadc_set_layer(0, &g_sDCLayer);
    frame_transfer();
#endif

#if USE_4BIT_FONT != 0
    x = xs[idx];
    y = ys[idx];
    ++idx;

    nema_set_blend_fill(NEMA_BL_SRC);
    nema_fill_rect(x, y, w, h, 0x70707070U);

    nema_bind_font(&g_sSimhei36pt4b);
    char *psram_simhei36pt4bBitmaps = psram_malloc(g_sSimhei36pt4b.bitmap_size);
    memcpy(psram_simhei36pt4bBitmaps, g_sSimhei36pt4b.bitmap, g_sSimhei36pt4b.bitmap_size);
    g_sSimhei36pt4b.bo.base_phys = (uintptr_t)psram_simhei36pt4bBitmaps;
    g_sSimhei36pt4b.bo.base_virt = (void*)g_sSimhei36pt4b.bo.base_phys;

    nema_print(ui8Str, x, y, w, h, 0xff00ff80U, NEMA_ALIGNX_LEFT   | NEMA_TEXT_WRAP | NEMA_ALIGNY_TOP);
    nema_cl_submit(&sCL);
    (void)nema_cl_wait(&sCL);
    nemadc_set_layer(0, &g_sDCLayer);
    frame_transfer();
#endif

#if USE_8BIT_FONT != 0

    x = xs[idx];
    y = ys[idx];
    ++idx;

    nema_set_blend_fill(NEMA_BL_SRC);
    nema_fill_rect(x, y, w, h, 0x70707070U);

    nema_bind_font(&g_sSimhei36pt8b);
    char *psram_simhei36pt8bBitmaps = psram_malloc(g_sSimhei36pt8b.bitmap_size);
    memcpy(psram_simhei36pt8bBitmaps, g_sSimhei36pt8b.bitmap, g_sSimhei36pt8b.bitmap_size);
    g_sSimhei36pt8b.bo.base_phys = (uintptr_t)psram_simhei36pt8bBitmaps;
    g_sSimhei36pt8b.bo.base_virt = (void*)g_sSimhei36pt8b.bo.base_phys;

    nema_print(ui8Str, x, y, w, h, 0xff00ff80U, NEMA_ALIGNX_LEFT   | NEMA_TEXT_WRAP | NEMA_ALIGNY_TOP);

    nema_cl_submit(&sCL);
    (void)nema_cl_wait(&sCL);
    nemadc_set_layer(0, &g_sDCLayer);
    frame_transfer();
#endif

    nema_cl_destroy(&sCL);
    psram_free(psram_simhei36pt1bBitmaps);
    psram_free(psram_simhei36pt4bBitmaps);
    psram_free(psram_simhei36pt8bBitmaps);
    release_font_objects();
}

void
fb_reload_tsc4(void)
{
    g_sFB.bo = nema_buffer_create(RESX * RESY / 2);
    g_sFB.w = RESX;
    g_sFB.h = RESY;
    g_sFB.stride = RESX * 2;
    g_sFB.color = 0;
    g_sFB.format = NEMA_TSC4;
    g_sFB.sampling_mode = 0;

    g_sDCLayer.format = NEMADC_TSC4;
    g_sDCLayer.baseaddr_virt = (void*)g_sFB.bo.base_virt;
    g_sDCLayer.baseaddr_phys = (uintptr_t)g_sFB.bo.base_phys;
    g_sDCLayer.resx = RESX;
    g_sDCLayer.resy = RESY;
    g_sDCLayer.stride = RESX * 2;
    g_sDCLayer.startx = 0;
    g_sDCLayer.starty = 0;
    g_sDCLayer.sizex = RESX;
    g_sDCLayer.sizey = RESY;
    g_sDCLayer.alpha = 0xFF;
    g_sDCLayer.blendmode = NEMADC_BL_SRC;
    g_sDCLayer.buscfg = 0;
    g_sDCLayer.format = NEMADC_TSC4;
    g_sDCLayer.mode = 0;
    g_sDCLayer.u_base = 0;
    g_sDCLayer.v_base = 0;
    g_sDCLayer.u_stride = 0;
    g_sDCLayer.v_stride = 0;
    am_util_stdio_printf("g_sFB use tsc4\n");

}

void
fb_reload_tsc6(void)
{
    g_sFB.bo = nema_buffer_create(RESX * RESY * 4 / 3);
    g_sFB.w = RESX;
    g_sFB.h = RESY;
    g_sFB.stride = RESX * 3;
    g_sFB.color = 0;
    g_sFB.format = NEMA_TSC6;
    g_sFB.sampling_mode = 0;

    g_sDCLayer.format = NEMADC_TSC6;
    g_sDCLayer.baseaddr_virt = (void*)g_sFB.bo.base_virt;
    g_sDCLayer.baseaddr_phys = (uintptr_t)g_sFB.bo.base_phys;
    g_sDCLayer.resx = RESX;
    g_sDCLayer.resy = RESY;
    g_sDCLayer.stride = RESX * 3;
    g_sDCLayer.startx = 0;
    g_sDCLayer.starty = 0;
    g_sDCLayer.sizex = RESX;
    g_sDCLayer.sizey = RESY;
    g_sDCLayer.alpha = 0xFF;
    g_sDCLayer.blendmode = NEMADC_BL_SRC;
    g_sDCLayer.buscfg = 0;
    g_sDCLayer.format = NEMADC_TSC6;
    g_sDCLayer.mode = 0;
    g_sDCLayer.u_base = 0;
    g_sDCLayer.v_base = 0;
    g_sDCLayer.u_stride = 0;
    g_sDCLayer.v_stride = 0;
    am_util_stdio_printf("g_sFB use tsc6\n");

}

void
fb_reload_tsc6_new_size(uint16_t w, uint16_t h)
{
    nema_buffer_destroy(&g_sFB.bo);
    g_sFB.format = NEMA_TSC6;
    g_sFB.w = w;
    g_sFB.h = h;

    int32_t width;
    int32_t height;

    width = g_sFB.w;
    height = g_sFB.h;

    width  = ((width + 3) / 4) * 4;
    height = ((height + 3) / 4) * 4;
    g_sFB.bo.size = ((width*height) / 4) * 3;
    g_sFB.bo = nema_buffer_create(align16(g_sFB.bo.size));

    g_sDCLayer.format = NEMADC_TSC6;
    g_sDCLayer.baseaddr_phys = g_sFB.bo.base_phys;
    g_sDCLayer.baseaddr_virt = g_sFB.bo.base_virt;

    am_util_stdio_printf("g_sFB use tsc6 new size\n");
}

void
fb_reload_rgb24(void)
{
    g_sFB.bo = nema_buffer_create(RESX*RESY*3);
    g_sFB.w = RESX;
    g_sFB.h = RESY;
    g_sFB.stride = RESX * 3;
    g_sFB.color = 0;
    g_sFB.format = NEMA_RGB24;
    g_sFB.sampling_mode = 0;
    printf("FB: V:%p P:0x%08x\n", (void *)g_sFB.bo.base_virt, g_sFB.bo.base_phys);

    g_sDCLayer.format = NEMADC_RGB24;
    g_sDCLayer.baseaddr_virt = (void*)g_sFB.bo.base_virt;
    g_sDCLayer.baseaddr_phys = (uintptr_t)g_sFB.bo.base_phys;
    g_sDCLayer.resx = RESX;
    g_sDCLayer.resy = RESY;
    g_sDCLayer.stride = RESX * 3;
    g_sDCLayer.startx = 0;
    g_sDCLayer.starty = 0;
    g_sDCLayer.sizex = RESX;
    g_sDCLayer.sizey = RESY;
    g_sDCLayer.alpha = 0xFF;
    g_sDCLayer.blendmode = NEMADC_BL_SRC;
    g_sDCLayer.buscfg = 0;
    g_sDCLayer.format = NEMADC_RGB24;
    g_sDCLayer.mode = 0;
    g_sDCLayer.u_base = 0;
    g_sDCLayer.v_base = 0;
    g_sDCLayer.u_stride = 0;
    g_sDCLayer.v_stride = 0;

    am_util_stdio_printf("g_sFB use rgb24\n");
}

void
fb_reload_psram_rgba8888(void)
{
    nema_buffer_destroy(&g_sFB.bo);
    g_sFB.format = NEMA_RGBA8888;
    g_sFB.bo = nema_psram_buffer_create(g_sFB.w * g_sFB.h * 4);

    g_sDCLayer.format = NEMADC_RGBA8888;
    g_sDCLayer.stride = g_sDCLayer.resx * 4;
    g_sDCLayer.baseaddr_phys = g_sFB.bo.base_phys;
    g_sDCLayer.baseaddr_virt = g_sFB.bo.base_virt;
    printf("FB: V:%p P:0x%08x\n", (void *)g_sFB.bo.base_virt, g_sFB.bo.base_phys);
}

void
fb_reload_psram_tsc6(void)
{
    nema_buffer_destroy(&g_sFB.bo);
    g_sFB.format = NEMA_RGBA8888;

    int32_t width;
    int32_t height;

    width = g_sFB.w;
    height = g_sFB.h;

    width  = ((width  + 3) / 4) * 4;
    height = ((height + 3) / 4) * 4;
    g_sFB.bo.size = width * height * 4;
    g_sFB.bo = nema_psram_buffer_create(align16(g_sFB.bo.size));

    g_sDCLayer.format = NEMADC_RGBA8888;
    g_sDCLayer.baseaddr_phys = g_sFB.bo.base_phys;
    g_sDCLayer.baseaddr_virt = g_sFB.bo.base_virt;

    am_util_stdio_printf("g_sFB use tsc6\n");
}

void fb_reload_rgba565(void)
{
    g_sFB.bo = nema_buffer_create(RESX * RESY * 2);
    if ( g_sFB.bo.base_phys == 0 )
    {
        while (1);
    }
    g_sFB.w = RESX;
    g_sFB.h = RESY;
    g_sFB.stride = RESX * 2;
    g_sFB.color = 0;
    g_sFB.format = NEMA_RGB565;
    g_sFB.sampling_mode = 0;
    printf("FB: V:%p P:0x%08x\n", (void *)g_sFB.bo.base_virt, g_sFB.bo.base_phys);

    g_sDCLayer.format = NEMADC_RGB565;
    g_sDCLayer.baseaddr_virt = (void*)g_sFB.bo.base_virt;
    g_sDCLayer.baseaddr_phys = (uintptr_t)g_sFB.bo.base_phys;
    g_sDCLayer.resx = RESX;
    g_sDCLayer.resy = RESY;
    g_sDCLayer.stride = RESX * 2;
    g_sDCLayer.startx = 0;
    g_sDCLayer.starty = 0;
    g_sDCLayer.sizex = RESX;
    g_sDCLayer.sizey = RESY;
    g_sDCLayer.alpha = 0xFF;
    g_sDCLayer.blendmode = NEMADC_BL_SRC;
    g_sDCLayer.buscfg = 0;
    g_sDCLayer.format = NEMADC_RGB565;
    g_sDCLayer.mode = 0;
    g_sDCLayer.u_base = 0;
    g_sDCLayer.v_base = 0;
    g_sDCLayer.u_stride = 0;
    g_sDCLayer.v_stride = 0;
    am_util_stdio_printf("g_sFB use rgb565\n");
}

void
fb_reload_rgba8888(void)
{
    g_sFB.bo = nema_buffer_create(RESX * RESY * 4);
    if ( g_sFB.bo.base_phys == 0 )
    {
        while(1);
    }
    g_sFB.w = RESX;
    g_sFB.h = RESY;
    g_sFB.stride = RESX * 4;
    g_sFB.color = 0;
    g_sFB.format = NEMA_RGBA8888;
    g_sFB.sampling_mode = 0;

    g_sDCLayer.format = NEMADC_RGBA8888;
    g_sDCLayer.baseaddr_virt = (void*)g_sFB.bo.base_virt;
    g_sDCLayer.baseaddr_phys = (uintptr_t)g_sFB.bo.base_phys;
    g_sDCLayer.resx = RESX;
    g_sDCLayer.resy = RESY;
    g_sDCLayer.stride = RESX * 4;
    g_sDCLayer.startx = 0;
    g_sDCLayer.starty = 0;
    g_sDCLayer.sizex = RESX;
    g_sDCLayer.sizey = RESY;
    g_sDCLayer.alpha = 0xFF;
    g_sDCLayer.blendmode = NEMADC_BL_SIMPLE;
    g_sDCLayer.buscfg = 0;
    g_sDCLayer.format = NEMADC_RGBA8888;
    g_sDCLayer.mode = 0;
    g_sDCLayer.u_base = 0;
    g_sDCLayer.v_base = 0;
    g_sDCLayer.u_stride = 0;
    g_sDCLayer.v_stride = 0;

    am_util_stdio_printf("g_sFB use rgba8888\n");
}

void
blit_texture_psram(void)
{
    nema_cmdlist_t sCL;

    sCL = nema_cl_create();
    nema_cl_bind(&sCL);
    nema_bind_dst_tex(g_sFB.bo.base_phys, g_sFB.w, g_sFB.h, g_sFB.format, g_sFB.stride);

    char *psram_greek_islend_100x100_alpha_rgba = psram_malloc(sizeof(g_ui8GreekIsland100x100RGBA));
    memcpy(psram_greek_islend_100x100_alpha_rgba, g_ui8GreekIsland100x100RGBA, sizeof(g_ui8GreekIsland100x100RGBA));
    g_sGreekIslandRGBA8888.bo.base_phys = (uintptr_t)psram_greek_islend_100x100_alpha_rgba;
    g_sGreekIslandRGBA8888.bo.base_virt = (void*)g_sGreekIslandRGBA8888.bo.base_phys;
    nema_bind_src_tex(g_sGreekIslandRGBA8888.bo.base_phys,
                      g_sGreekIslandRGBA8888.w,
                      g_sGreekIslandRGBA8888.h,
                      g_sGreekIslandRGBA8888.format,
                      g_sGreekIslandRGBA8888.stride,
                      NEMA_FILTER_PS);
    nema_set_clip(0, 0, RESX, RESY);
//!< nema_set_const_color(YELLOW);
    nema_set_blend_blit(NEMA_BL_SRC);
    nema_blit(0, 0);

    char *psram_greek_islend_100x100_rgba565 = psram_malloc(sizeof(g_ui8GreekIsland100x100RGBA565));
    memcpy(psram_greek_islend_100x100_rgba565, g_ui8GreekIsland100x100RGBA565, sizeof(g_ui8GreekIsland100x100RGBA565));
    g_sGreekIslandRGBA5650.bo.base_phys = (uintptr_t)psram_greek_islend_100x100_rgba565;
    g_sGreekIslandRGBA5650.bo.base_virt = (void*)g_sGreekIslandRGBA5650.bo.base_phys;
    nema_bind_src_tex(g_sGreekIslandRGBA5650.bo.base_phys,
                      g_sGreekIslandRGBA5650.w,
                      g_sGreekIslandRGBA5650.h,
                      g_sGreekIslandRGBA5650.format,
                      g_sGreekIslandRGBA5650.stride,
                      NEMA_FILTER_PS);
    nema_blit(100, 0);

    char *psram_greek_islend_100x100_tsc6 = psram_malloc(sizeof(g_ui8GreekIsland100x100TSC6));
    memcpy(psram_greek_islend_100x100_tsc6, g_ui8GreekIsland100x100TSC6, sizeof(g_ui8GreekIsland100x100TSC6));
    g_sGreekIslandTSC6.bo.base_phys = (uintptr_t)psram_greek_islend_100x100_tsc6;
    g_sGreekIslandTSC6.bo.base_virt = (void*)g_sGreekIslandTSC6.bo.base_phys;
    nema_bind_src_tex(g_sGreekIslandTSC6.bo.base_phys,
                      g_sGreekIslandTSC6.w,
                      g_sGreekIslandTSC6.h,
                      g_sGreekIslandTSC6.format,
                      g_sGreekIslandTSC6.stride,
                      NEMA_FILTER_PS);
    nema_blit(200, 0);

    char *psram_greek_islend_100x100_tsc4 = psram_malloc(sizeof(g_ui8GreekIsland100x100TSC4));
    memcpy(psram_greek_islend_100x100_tsc4, g_ui8GreekIsland100x100TSC4, sizeof(g_ui8GreekIsland100x100TSC4));
    g_sGreekIslandTSC4.bo.base_phys = (uintptr_t)psram_greek_islend_100x100_tsc4;
    g_sGreekIslandTSC4.bo.base_virt = (void*)g_sGreekIslandTSC4.bo.base_phys;
    nema_bind_src_tex(g_sGreekIslandTSC4.bo.base_phys,
                      g_sGreekIslandTSC4.w,
                      g_sGreekIslandTSC4.h,
                      g_sGreekIslandTSC4.format,
                      g_sGreekIslandTSC4.stride,
                      NEMA_FILTER_PS);
    nema_blit(0, 100);

    char *psram_greek_islend_100x100_tsc6a = psram_malloc(sizeof(g_ui8GreekIsland100x100TSC6A));
    memcpy(psram_greek_islend_100x100_tsc6a, g_ui8GreekIsland100x100TSC6A, sizeof(g_ui8GreekIsland100x100TSC6A));
    g_sGreekIslandTSC6A.bo.base_phys = (uintptr_t)psram_greek_islend_100x100_tsc6a;
    g_sGreekIslandTSC6A.bo.base_virt = (void*)g_sGreekIslandTSC6A.bo.base_phys;
    nema_bind_src_tex(g_sGreekIslandTSC6A.bo.base_phys,
                      g_sGreekIslandTSC6A.w,
                      g_sGreekIslandTSC6A.h,
                      g_sGreekIslandTSC6A.format,
                      g_sGreekIslandTSC6A.stride,
                      NEMA_FILTER_PS);
    nema_blit(100, 100);

    img_obj_t sFZ25099x99RGBA8888 = {{0}, 99, 99, -1, 0, NEMA_RGBA8888, 0};
    char *psram_fz250_99x99_rgba8888 = psram_malloc(sizeof(g_ui8FZ25099x99RGBA));
    memcpy(psram_fz250_99x99_rgba8888, g_ui8FZ25099x99RGBA, sizeof(g_ui8FZ25099x99RGBA));
    sFZ25099x99RGBA8888.bo.base_phys = (uintptr_t)psram_fz250_99x99_rgba8888;
    sFZ25099x99RGBA8888.bo.base_virt = (void*)sFZ25099x99RGBA8888.bo.base_phys;
    nema_bind_src_tex(sFZ25099x99RGBA8888.bo.base_phys,
                      sFZ25099x99RGBA8888.w,
                      sFZ25099x99RGBA8888.h,
                      sFZ25099x99RGBA8888.format,
                      sFZ25099x99RGBA8888.stride,
                      NEMA_FILTER_PS);
    nema_blit(200, 100);

    nema_cl_submit(&sCL);
    nema_cl_wait(&sCL);
    nemadc_set_layer(0, &g_sDCLayer);
    frame_transfer();

    nema_cl_destroy(&sCL);

    psram_free((void*)g_sGreekIslandRGBA8888.bo.base_phys);
    psram_free((void*)g_sGreekIslandRGBA5650.bo.base_phys);
    psram_free((void*)g_sGreekIslandTSC6.bo.base_phys);
    psram_free((void*)g_sGreekIslandTSC4.bo.base_phys);
    psram_free((void*)g_sGreekIslandTSC6A.bo.base_phys);
    psram_free((void*)sFZ25099x99RGBA8888.bo.base_phys);
}

void
blit_texture_psram_100x100(void)
{
    nema_cmdlist_t sCL;

    sCL = nema_cl_create();
    nema_cl_bind(&sCL);
    nema_bind_dst_tex(g_sFB.bo.base_phys, g_sFB.w, g_sFB.h, g_sFB.format, g_sFB.stride);

    char *psram_greek_islend_100x100_alpha_rgba = psram_malloc(sizeof(g_ui8GreekIsland100x100RGBA));
    memcpy(psram_greek_islend_100x100_alpha_rgba, g_ui8GreekIsland100x100RGBA, sizeof(g_ui8GreekIsland100x100RGBA));
    g_sGreekIslandRGBA8888.bo.base_phys = (uintptr_t)psram_greek_islend_100x100_alpha_rgba;
    g_sGreekIslandRGBA8888.bo.base_virt = (void*)g_sGreekIslandRGBA8888.bo.base_phys;
    nema_bind_src_tex(g_sGreekIslandRGBA8888.bo.base_phys,
                      g_sGreekIslandRGBA8888.w,
                      g_sGreekIslandRGBA8888.h,
                      g_sGreekIslandRGBA8888.format,
                      g_sGreekIslandRGBA8888.stride,
                      NEMA_FILTER_PS);
    nema_set_clip(0, 0, RESX, RESY);
//!< nema_set_const_color(YELLOW);
    start_time = nema_get_time();
    nema_set_blend_blit(NEMA_BL_SRC);
    nema_blit(0, 0);

    nema_cl_submit(&sCL);
    nema_cl_wait(&sCL);

    end_time = nema_get_time();
    am_util_stdio_printf("blit_texture_psram_100x100 rgba8888 used %f s\n", end_time - start_time);

    nemadc_set_layer(0, &g_sDCLayer);
    frame_transfer();

    nema_cl_unbind();
    nema_cl_destroy(&sCL);

    psram_free((void*)g_sGreekIslandRGBA8888.bo.base_phys);
}

void
blit_texture_psram_360x360(void)
{
    img_obj_t obj_oli_360x360_alpha_rgba5551 = {{0}, 360, 360, -1, 0, NEMA_RGBA5551, 0};
    nema_cmdlist_t sCL;

    sCL = nema_cl_create();
    nema_cl_bind(&sCL);
    nema_bind_dst_tex(g_sFB.bo.base_phys, g_sFB.w, g_sFB.h, g_sFB.format, g_sFB.stride);

    char *psram_oli_360x360_alpha_rgba5551 = psram_malloc(sizeof(oli_360x360_alpha_rgba5551));
    memcpy(psram_oli_360x360_alpha_rgba5551, oli_360x360_alpha_rgba5551, sizeof(oli_360x360_alpha_rgba5551));
    obj_oli_360x360_alpha_rgba5551.bo.base_phys = (uintptr_t)psram_oli_360x360_alpha_rgba5551;
    obj_oli_360x360_alpha_rgba5551.bo.base_virt = (void*)obj_oli_360x360_alpha_rgba5551.bo.base_phys;
    nema_bind_src_tex(obj_oli_360x360_alpha_rgba5551.bo.base_phys,
                      obj_oli_360x360_alpha_rgba5551.w,
                      obj_oli_360x360_alpha_rgba5551.h,
                      obj_oli_360x360_alpha_rgba5551.format,
                      obj_oli_360x360_alpha_rgba5551.stride,
                      NEMA_FILTER_PS);
    nema_set_clip(0, 0, RESX, RESY);
//!< nema_set_const_color(YELLOW);
    start_time = nema_get_time();
    nema_set_blend_blit(NEMA_BL_SRC);
    nema_blit(0, 0);

    nema_cl_submit(&sCL);
    nema_cl_wait(&sCL);

    end_time = nema_get_time();
    am_util_stdio_printf("blit_texture_psram_360x360 rgba565 used %f s\n", end_time - start_time);

    nemadc_set_layer(0, &g_sDCLayer);
    frame_transfer();

    nema_cl_unbind();
    nema_cl_destroy(&sCL);

    psram_free((void*)obj_oli_360x360_alpha_rgba5551.bo.base_phys);
}

void
blit_texture_psram_blend(void)
{
    nema_cmdlist_t sCL;

    sCL = nema_cl_create();
    nema_cl_bind(&sCL);
    nema_bind_dst_tex(g_sFB.bo.base_phys, g_sFB.w, g_sFB.h, g_sFB.format, g_sFB.stride);

    char *psram_greek_islend_100x100_alpha_rgba = psram_malloc(sizeof(g_ui8GreekIsland100x100RGBA));
    memcpy(psram_greek_islend_100x100_alpha_rgba, g_ui8GreekIsland100x100RGBA, sizeof(g_ui8GreekIsland100x100RGBA));
    g_sGreekIslandRGBA8888.bo.base_phys = (uintptr_t)psram_greek_islend_100x100_alpha_rgba;
    g_sGreekIslandRGBA8888.bo.base_virt = (void*)g_sGreekIslandRGBA8888.bo.base_phys;
    nema_bind_src_tex(g_sGreekIslandRGBA8888.bo.base_phys,
                      g_sGreekIslandRGBA8888.w,
                      g_sGreekIslandRGBA8888.h,
                      g_sGreekIslandRGBA8888.format,
                      g_sGreekIslandRGBA8888.stride,
                      NEMA_FILTER_PS);
    nema_set_clip(0, 0, RESX, RESY);
    nema_set_const_color(YELLOW);
    nema_set_blend_blit(NEMA_BL_SRC | NEMA_BLOP_MODULATE_RGB);
    nema_blit(0, 0);

    char *psram_greek_islend_100x100_rgba565 = psram_malloc(sizeof(g_ui8GreekIsland100x100RGBA565));
    memcpy(psram_greek_islend_100x100_rgba565, g_ui8GreekIsland100x100RGBA565, sizeof(g_ui8GreekIsland100x100RGBA565));
    g_sGreekIslandRGBA5650.bo.base_phys = (uintptr_t)psram_greek_islend_100x100_rgba565;
    g_sGreekIslandRGBA5650.bo.base_virt = (void*)g_sGreekIslandRGBA5650.bo.base_phys;
    nema_bind_src_tex(g_sGreekIslandRGBA5650.bo.base_phys,
                      g_sGreekIslandRGBA5650.w,
                      g_sGreekIslandRGBA5650.h,
                      g_sGreekIslandRGBA5650.format,
                      g_sGreekIslandRGBA5650.stride,
                      NEMA_FILTER_PS);
    nema_blit(100, 0);

    char *psram_greek_islend_100x100_tsc6 = psram_malloc(sizeof(g_ui8GreekIsland100x100TSC6));
    memcpy(psram_greek_islend_100x100_tsc6, g_ui8GreekIsland100x100TSC6, sizeof(g_ui8GreekIsland100x100TSC6));
    g_sGreekIslandTSC6.bo.base_phys = (uintptr_t)psram_greek_islend_100x100_tsc6;
    g_sGreekIslandTSC6.bo.base_virt = (void*)g_sGreekIslandTSC6.bo.base_phys;
    nema_bind_src_tex(g_sGreekIslandTSC6.bo.base_phys,
                      g_sGreekIslandTSC6.w,
                      g_sGreekIslandTSC6.h,
                      g_sGreekIslandTSC6.format,
                      g_sGreekIslandTSC6.stride,
                      NEMA_FILTER_PS);
    nema_blit(200, 0);

    char *psram_greek_islend_100x100_tsc4 = psram_malloc(sizeof(g_ui8GreekIsland100x100TSC4));
    memcpy(psram_greek_islend_100x100_tsc4, g_ui8GreekIsland100x100TSC4, sizeof(g_ui8GreekIsland100x100TSC4));
    g_sGreekIslandTSC4.bo.base_phys = (uintptr_t)psram_greek_islend_100x100_tsc4;
    g_sGreekIslandTSC4.bo.base_virt = (void*)g_sGreekIslandTSC4.bo.base_phys;
    nema_bind_src_tex(g_sGreekIslandTSC4.bo.base_phys,
                      g_sGreekIslandTSC4.w,
                      g_sGreekIslandTSC4.h,
                      g_sGreekIslandTSC4.format,
                      g_sGreekIslandTSC4.stride,
                      NEMA_FILTER_PS);
    nema_blit(0, 100);

    char *psram_greek_islend_100x100_tsc6a = psram_malloc(sizeof(g_ui8GreekIsland100x100TSC6A));
    memcpy(psram_greek_islend_100x100_tsc6a, g_ui8GreekIsland100x100TSC6A, sizeof(g_ui8GreekIsland100x100TSC6A));
    g_sGreekIslandTSC6A.bo.base_phys = (uintptr_t)psram_greek_islend_100x100_tsc6a;
    g_sGreekIslandTSC6A.bo.base_virt = (void*)g_sGreekIslandTSC6A.bo.base_phys;
    nema_bind_src_tex(g_sGreekIslandTSC6A.bo.base_phys,
                      g_sGreekIslandTSC6A.w,
                      g_sGreekIslandTSC6A.h,
                      g_sGreekIslandTSC6A.format,
                      g_sGreekIslandTSC6A.stride,
                      NEMA_FILTER_PS);
    nema_blit(100, 100);

    nema_cl_submit(&sCL);
    nema_cl_wait(&sCL);
    nemadc_set_layer(0, &g_sDCLayer);
    frame_transfer();

    nema_cl_destroy(&sCL);

    psram_free((void*)g_sGreekIslandRGBA8888.bo.base_phys);
    psram_free((void*)g_sGreekIslandRGBA5650.bo.base_phys);
    psram_free((void*)g_sGreekIslandTSC6.bo.base_phys);
    psram_free((void*)g_sGreekIslandTSC4.bo.base_phys);
    psram_free((void*)g_sGreekIslandTSC6A.bo.base_phys);
}

void
blit_texture_psram_scale(void)
{
    nema_cmdlist_t sCL;

    sCL = nema_cl_create();
    nema_cl_bind(&sCL);
    nema_bind_dst_tex(g_sFB.bo.base_phys, g_sFB.w, g_sFB.h, g_sFB.format, g_sFB.stride);

    char *psram_greek_islend_100x100_alpha_rgba = psram_malloc(sizeof(g_ui8GreekIsland100x100RGBA));
    memcpy(psram_greek_islend_100x100_alpha_rgba, g_ui8GreekIsland100x100RGBA, sizeof(g_ui8GreekIsland100x100RGBA));
    g_sGreekIslandRGBA8888.bo.base_phys = (uintptr_t)psram_greek_islend_100x100_alpha_rgba;
    g_sGreekIslandRGBA8888.bo.base_virt = (void*)g_sGreekIslandRGBA8888.bo.base_phys;
    nema_bind_src_tex(g_sGreekIslandRGBA8888.bo.base_phys,
                      g_sGreekIslandRGBA8888.w,
                      g_sGreekIslandRGBA8888.h,
                      g_sGreekIslandRGBA8888.format,
                      g_sGreekIslandRGBA8888.stride,
                      NEMA_FILTER_PS);
    nema_set_clip(0, 0, RESX, RESY);
    nema_set_blend_blit(NEMA_BL_SRC);
    nema_blit_rect_fit(20, 20, 115, 115);

//    char *psram_greek_islend_100x100_rgba565 = psram_malloc(sizeof(g_ui8GreekIsland100x100RGBA565));
//    memcpy(psram_greek_islend_100x100_rgba565, g_ui8GreekIsland100x100RGBA565, sizeof(g_ui8GreekIsland100x100RGBA565));
//    g_sGreekIslandRGBA5650.bo.base_phys = (uintptr_t)psram_greek_islend_100x100_rgba565;
//    g_sGreekIslandRGBA5650.bo.base_virt = (void*)g_sGreekIslandRGBA5650.bo.base_phys;
//    nema_bind_src_tex(g_sGreekIslandRGBA5650.bo.base_phys,
//                      g_sGreekIslandRGBA5650.w,
//                      g_sGreekIslandRGBA5650.h,
//                      g_sGreekIslandRGBA5650.format,
//                      g_sGreekIslandRGBA5650.stride,
//                      NEMA_FILTER_PS);
//    nema_blit(100, 0);

    char *psram_greek_islend_100x100_tsc6 = psram_malloc(sizeof(g_ui8GreekIsland100x100TSC6));
    memcpy(psram_greek_islend_100x100_tsc6, g_ui8GreekIsland100x100TSC6, sizeof(g_ui8GreekIsland100x100TSC6));
    g_sGreekIslandTSC6.bo.base_phys = (uintptr_t)psram_greek_islend_100x100_tsc6;
    g_sGreekIslandTSC6.bo.base_virt = (void*)g_sGreekIslandTSC6.bo.base_phys;
    nema_bind_src_tex(g_sGreekIslandTSC6.bo.base_phys,
                      g_sGreekIslandTSC6.w,
                      g_sGreekIslandTSC6.h,
                      g_sGreekIslandTSC6.format,
                      g_sGreekIslandTSC6.stride,
                      NEMA_FILTER_PS);
    nema_blit_rect_fit(20 + 115 + 10, 20, 115, 115);

//    char *psram_greek_islend_100x100_tsc4 = psram_malloc(sizeof(g_ui8GreekIsland100x100TSC4));
//    memcpy(psram_greek_islend_100x100_tsc4, g_ui8GreekIsland100x100TSC4, sizeof(g_ui8GreekIsland100x100TSC4));
//    g_sGreekIslandTSC4.bo.base_phys = (uintptr_t)psram_greek_islend_100x100_tsc4;
//    g_sGreekIslandTSC4.bo.base_virt = (void*)g_sGreekIslandTSC4.bo.base_phys;
//    nema_bind_src_tex(g_sGreekIslandTSC4.bo.base_phys,
//                      g_sGreekIslandTSC4.w,
//                      g_sGreekIslandTSC4.h,
//                      g_sGreekIslandTSC4.format,
//                      g_sGreekIslandTSC4.stride,
//                      NEMA_FILTER_PS);
//    nema_blit(0, 100);

    char *psram_greek_islend_100x100_tsc6a = psram_malloc(sizeof(g_ui8GreekIsland100x100TSC6A));
    memcpy(psram_greek_islend_100x100_tsc6a, g_ui8GreekIsland100x100TSC6A, sizeof(g_ui8GreekIsland100x100TSC6A));
    g_sGreekIslandTSC6A.bo.base_phys = (uintptr_t)psram_greek_islend_100x100_tsc6a;
    g_sGreekIslandTSC6A.bo.base_virt = (void*)g_sGreekIslandTSC6A.bo.base_phys;
    nema_bind_src_tex(g_sGreekIslandTSC6A.bo.base_phys,
                      g_sGreekIslandTSC6A.w,
                      g_sGreekIslandTSC6A.h,
                      g_sGreekIslandTSC6A.format,
                      g_sGreekIslandTSC6A.stride,
                      NEMA_FILTER_PS);
    nema_blit_rect_fit(20, 20 + 115 + 10, 115, 115);

    nema_cl_submit(&sCL);
    nema_cl_wait(&sCL);
    nemadc_set_layer(0, &g_sDCLayer);
    frame_transfer();

    nema_cl_destroy(&sCL);

    psram_free((void*)g_sGreekIslandRGBA8888.bo.base_phys);
//!< psram_free((void*)g_sGreekIslandRGBA5650.bo.base_phys);
    psram_free((void*)g_sGreekIslandTSC6.bo.base_phys);
//!< psram_free((void*)g_sGreekIslandTSC4.bo.base_phys);
    psram_free((void*)g_sGreekIslandTSC6A.bo.base_phys);
}

void
blit_texture_psram_rotate_rgba8888(void)
{
    img_obj_t sHourRGBA8888 = {{0}, 20, 250, -1, 0, NEMA_RGBA8888, 0};

//    img_obj_t hour_tsc6 = {{0}, 20, 250, -1, 0, NEMA_TSC6, 0};
//    img_obj_t hour_tsc6a = {{0}, 20, 250, -1, 0, NEMA_TSC6A, 0};

    float x0, x1, x2, x3, y0, y1, y2, y3;
    nema_matrix3x3_t m;

    nema_cmdlist_t sCL;

    sCL = nema_cl_create();
    nema_cl_bind(&sCL);
    nema_bind_dst_tex(g_sFB.bo.base_phys, g_sFB.w, g_sFB.h, g_sFB.format, g_sFB.stride);

    char *psram_hour_rgba8888 = psram_malloc(sizeof(g_ui8Hour20x250RGBA));
    memcpy(psram_hour_rgba8888, g_ui8Hour20x250RGBA, sizeof(g_ui8Hour20x250RGBA));
    sHourRGBA8888.bo.base_phys = (uintptr_t)psram_hour_rgba8888;
    sHourRGBA8888.bo.base_virt = (void*)sHourRGBA8888.bo.base_phys;
    nema_bind_src_tex(sHourRGBA8888.bo.base_phys,
                      sHourRGBA8888.w,
                      sHourRGBA8888.h,
                      sHourRGBA8888.format,
                      sHourRGBA8888.stride,
                      NEMA_FILTER_PS);
    nema_set_clip(0, 0, RESX, RESY);
    nema_set_blend_blit(NEMA_BL_SIMPLE);

    start_time = nema_get_time();

    int32_t w = sHourRGBA8888.w;
    int32_t h = sHourRGBA8888.h;
    x0 = 0; y0 = 0; x1 = w; y1 = 0; x2 = w; y2 = h; x3 = 0; y3 = h;
    nema_mat3x3_load_identity(m);
    nema_mat3x3_rotate(m, -39);
    nema_mat3x3_translate(m, 40, 40);
    nema_mat3x3_mul_vec(m, &x0, &y0);
    nema_mat3x3_mul_vec(m, &x1, &y1);
    nema_mat3x3_mul_vec(m, &x2, &y2);
    nema_mat3x3_mul_vec(m, &x3, &y3);
    nema_blit_quad_fit(x0, y0, x1, y1, x2, y2, x3, y3);

    nema_cl_submit(&sCL);
    nema_cl_wait(&sCL);

    end_time = nema_get_time();
    am_util_stdio_printf("used %f s\n", end_time - start_time);

    nemadc_set_layer(0, &g_sDCLayer);
    frame_transfer();

    nema_cl_destroy(&sCL);

    psram_free((void*)g_sGreekIslandRGBA8888.bo.base_phys);
}

void
blit_texture_psram_rotate_tsc6(void)
{
//!< img_obj_t sHourRGBA8888 = {{0}, 20, 250, -1, 0, NEMA_RGBA8888, 0};
    img_obj_t hour_tsc6 = {{0}, 20, 250, -1, 0, NEMA_TSC6, 0};
//!< img_obj_t hour_tsc6a = {{0}, 20, 250, -1, 0, NEMA_TSC6A, 0};

    float x0, x1, x2, x3, y0, y1, y2, y3;
    nema_matrix3x3_t m;

    nema_cmdlist_t sCL;

    sCL = nema_cl_create();
    nema_cl_bind(&sCL);
    nema_bind_dst_tex(g_sFB.bo.base_phys, g_sFB.w, g_sFB.h, g_sFB.format, g_sFB.stride);

    char *psram_hour_tsc6 = psram_malloc(sizeof(g_ui8Hour20x250TSC6));
    memcpy(psram_hour_tsc6, g_ui8Hour20x250TSC6, sizeof(g_ui8Hour20x250TSC6));
    hour_tsc6.bo.base_phys = (uintptr_t)psram_hour_tsc6;
    hour_tsc6.bo.base_virt = (void*)hour_tsc6.bo.base_phys;
    nema_bind_src_tex(hour_tsc6.bo.base_phys,
                      hour_tsc6.w,
                      hour_tsc6.h,
                      hour_tsc6.format,
                      hour_tsc6.stride,
                      NEMA_FILTER_PS);
    nema_set_clip(0, 0, RESX, RESY);
    nema_set_blend_blit(NEMA_BL_SIMPLE);
    int32_t w = hour_tsc6.w;
    int32_t h = hour_tsc6.h;
    x0 = 0; y0 = 0; x1 = w; y1 = 0; x2 = w; y2 = h; x3 = 0; y3 = h;
    nema_mat3x3_load_identity(m);
    nema_mat3x3_rotate(m, -39);
    nema_mat3x3_translate(m, 40, 40);
    nema_mat3x3_mul_vec(m, &x0, &y0);
    nema_mat3x3_mul_vec(m, &x1, &y1);
    nema_mat3x3_mul_vec(m, &x2, &y2);
    nema_mat3x3_mul_vec(m, &x3, &y3);
    nema_blit_quad_fit(x0, y0, x1, y1, x2, y2, x3, y3);

    nema_cl_submit(&sCL);
    nema_cl_wait(&sCL);
    nemadc_set_layer(0, &g_sDCLayer);
    frame_transfer();

    nema_cl_destroy(&sCL);

    psram_free((void*)hour_tsc6.bo.base_phys);
}

void
blit_texture_psram_rotate_tsc6a(void)
{
//!< img_obj_t sHourRGBA8888 = {{0}, 20, 250, -1, 0, NEMA_RGBA8888, 0};
    img_obj_t hour_tsc6a = {{0}, 20, 250, -1, 0, NEMA_TSC6A, 0};
//!< img_obj_t hour_tsc6a = {{0}, 20, 250, -1, 0, NEMA_TSC6A, 0};

    float x0, x1, x2, x3, y0, y1, y2, y3;
    nema_matrix3x3_t m;

    nema_cmdlist_t sCL;

    sCL = nema_cl_create();
    nema_cl_bind(&sCL);
    nema_bind_dst_tex(g_sFB.bo.base_phys, g_sFB.w, g_sFB.h, g_sFB.format, g_sFB.stride);

    char *psram_hour_tsc6a = psram_malloc(sizeof(g_ui8Hour20x250TSC6A));
    memcpy(psram_hour_tsc6a, g_ui8Hour20x250TSC6A, sizeof(g_ui8Hour20x250TSC6A));
    hour_tsc6a.bo.base_phys = (uintptr_t)psram_hour_tsc6a;
    hour_tsc6a.bo.base_virt = (void*)hour_tsc6a.bo.base_phys;
    nema_bind_src_tex(hour_tsc6a.bo.base_phys,
                      hour_tsc6a.w,
                      hour_tsc6a.h,
                      hour_tsc6a.format,
                      hour_tsc6a.stride,
                      NEMA_FILTER_PS);
    nema_set_clip(0, 0, RESX, RESY);
    nema_set_blend_blit(NEMA_BL_SIMPLE);
    int32_t w = hour_tsc6a.w;
    int32_t h = hour_tsc6a.h;
    x0 = 0; y0 = 0; x1 = w; y1 = 0; x2 = w; y2 = h; x3 = 0; y3 = h;
    nema_mat3x3_load_identity(m);
    nema_mat3x3_rotate(m, -39);
    nema_mat3x3_translate(m, 40, 40);
    nema_mat3x3_mul_vec(m, &x0, &y0);
    nema_mat3x3_mul_vec(m, &x1, &y1);
    nema_mat3x3_mul_vec(m, &x2, &y2);
    nema_mat3x3_mul_vec(m, &x3, &y3);
    nema_blit_quad_fit(x0, y0, x1, y1, x2, y2, x3, y3);

    nema_cl_submit(&sCL);
    nema_cl_wait(&sCL);
    nemadc_set_layer(0, &g_sDCLayer);
    frame_transfer();

    nema_cl_destroy(&sCL);

    psram_free((void*)hour_tsc6a.bo.base_phys);
}

void
fill_const(void)
{
    nema_cmdlist_t sCL;

    sCL = nema_cl_create();
    nema_cl_bind(&sCL);

//    nema_bind_src_tex(g_sFB.bo.base_phys,
//                          g_sFB.w,
//                          g_sFB.h,
//                          g_sFB.format,
//                          g_sFB.stride,
//                          NEMA_FILTER_PS);

    nema_bind_dst_tex(g_sFB.bo.base_phys, g_sFB.w, g_sFB.h, g_sFB.format, g_sFB.stride);

    nema_set_clip(0, 0, RESX, RESY);
    nema_set_blend_fill(NEMA_BL_SIMPLE);
    nema_fill_rect(20, 20, 100, 100, nema_rgba(0xff, 0, 0, 0x80));

    nema_cl_submit(&sCL);
    nema_cl_wait(&sCL);
    nemadc_set_layer(0, &g_sDCLayer);
    frame_transfer();

    //nema_cl_unbind();
    nema_cl_destroy(&sCL);
}

void
blit_clear_rgba8888(void)
{
    fb_reload_rgba8888();

    nema_cmdlist_t sCL;

    sCL = nema_cl_create();
    nema_cl_bind(&sCL);
    nema_bind_dst_tex(g_sFB.bo.base_phys, g_sFB.w, g_sFB.h, g_sFB.format, g_sFB.stride);
    nema_clear(BLACK);
    nema_cl_submit(&sCL);
    nema_cl_wait(&sCL);
    nemadc_set_layer(0, &g_sDCLayer);
    frame_transfer();
    nema_cl_unbind();
    nema_cl_destroy(&sCL);
    fb_release();
}

void
blit_clear_tsc6(void)
{
    fb_reload_tsc6();

    nema_cmdlist_t sCL;

    sCL = nema_cl_create();
    nema_cl_bind(&sCL);
    nema_bind_dst_tex(g_sFB.bo.base_phys, g_sFB.w, g_sFB.h, g_sFB.format, g_sFB.stride);
    nema_clear(BLACK);
    nema_cl_submit(&sCL);
    nema_cl_wait(&sCL);
    nemadc_set_layer(0, &g_sDCLayer);
    frame_transfer();

    nema_cl_unbind();
    nema_cl_destroy(&sCL);
    fb_release();
}

void
blit_clear(void)
{
    fb_reload_tsc6();

    nema_cmdlist_t sCL;

    sCL = nema_cl_create();
    nema_cl_bind(&sCL);
    nema_bind_dst_tex(g_sFB.bo.base_phys, g_sFB.w, g_sFB.h, g_sFB.format, g_sFB.stride);
    nema_clear(BLACK);
    nema_cl_submit(&sCL);
    nema_cl_wait(&sCL);
    nemadc_set_layer(0, &g_sDCLayer);
    frame_transfer();

    nema_cl_destroy(&sCL);
//!< nema_buffer_destroy(&g_sFB.bo);
    fb_release();
}

void
blit_psram_clear(void)
{
    nema_cmdlist_t sCL;

    sCL = nema_cl_create();
    nema_cl_bind(&sCL);
    nema_bind_dst_tex(g_sFB.bo.base_phys, g_sFB.w, g_sFB.h, g_sFB.format, g_sFB.stride);
    nema_clear(BLACK);
    nema_cl_submit(&sCL);
    nema_cl_wait(&sCL);
    nemadc_set_layer(0, &g_sDCLayer);
    frame_transfer();

    nema_cl_destroy(&sCL);
}

//    am_util_delay_ms(2000);
//    extern bool bDoScrambling;
//    extern bool run_mspi_xipmm(uint32_t block, bool bUseWordAccesses);
//    run_mspi_xipmm(0, bDoScrambling);
//    memset((uint32_t *)MSPI_XIPMM_BASE_ADDRESS, 0, g_sFB.w*g_sFB.h*4);
//
//    nema_clear(0x00000000);
//    fb_reload_psram_rgba8888();
//    blit_font();

void
blit_rgb24_texture(void)
{
    img_obj_t gImage_fz250 = {{0}, 100, 100, -1, 0, NEMA_RGB24, 0};

    nema_cmdlist_t sCL;

    sCL = nema_cl_create();
    nema_cl_bind(&sCL);
    nema_bind_dst_tex(g_sFB.bo.base_phys, g_sFB.w, g_sFB.h, g_sFB.format, g_sFB.stride);

    gImage_fz250.bo.base_phys = (uintptr_t)(&g_ui8ImageFZ250RGB24100x100);
    nema_bind_src_tex(gImage_fz250.bo.base_phys,
                      gImage_fz250.w,
                      gImage_fz250.h,
                      gImage_fz250.format,
                      gImage_fz250.stride,
                      NEMA_FILTER_PS);
    nema_set_clip(0, 0, RESX, RESY);
    nema_set_blend_blit(NEMA_BL_SRC);
    nema_blit(100, 100);

    nema_cl_submit(&sCL);
    nema_cl_wait(&sCL);
    nemadc_set_layer(0, &g_sDCLayer);
    frame_transfer();

    nema_cl_destroy(&sCL);
}

void
fb_psram_test(void)
{
    am_util_delay_ms(SHOW_INTERVAL);
    blit_clear();
    fb_reload_psram_rgba8888();

/*     blit_rgb24_texture(); */
}

int32_t
test_config(void)
{

    uint32_t config = nema_readHwConfig();

    am_util_stdio_printf("\nNema Configuration:\n");
    am_util_stdio_printf("-----------------------------------------------------------\n");
    am_util_stdio_printf("Master Bus:       %s\t\t\n", check_bits(config,  NEMA_CONF_MASK_AXIM,      NEMA_CONF_POS_AXIM      ) ? "AXI4"    : "ΑΗΒ32"    ) ;
    am_util_stdio_printf("Texfilter :       %s  \t\n", check_bits(config,  NEMA_CONF_MASK_TEXFILTER, NEMA_CONF_POS_TEXFILTER ) ? "enabled" : "disabled" ) ;
    am_util_stdio_printf("TSc6:             %s  \t\n", check_bits(config,  NEMA_CONF_MASK_TSC6,      NEMA_CONF_POS_TSC6      ) ? "enabled" : "disabled" ) ;
    am_util_stdio_printf("Blender:          %s  \t\n", check_bits(config,  NEMA_CONF_MASK_BLENDER,   NEMA_CONF_POS_BLENDER   ) ? "enabled" : "disabled" ) ;
    am_util_stdio_printf("Asynchronous:     %s  \t\n", check_bits(config,  NEMA_CONF_MASK_ASYNC,     NEMA_CONF_POS_ASYNC     ) ? "enabled" : "disabled" ) ;
    am_util_stdio_printf("Dirty Region:     %s  \t\n", check_bits(config,  NEMA_CONF_MASK_DIRTY,     NEMA_CONF_POS_DIRTY     ) ? "enabled" : "disabled" ) ;
    am_util_stdio_printf("Core Type:        %s\t\t\n", check_bits(config,  NEMA_CONF_MASK_TYPES,     NEMA_CONF_POS_TYPES     ) ? "small"   : "tiny" ) ;
    am_util_stdio_printf("MMU:              %s  \t\n", check_bits(config,  NEMA_CONF_MASK_MMU,       NEMA_CONF_POS_MMU       ) ? "enabled" : "disabled" ) ;
    am_util_stdio_printf("Z-Compression:    %s  \t\n", check_bits(config,  NEMA_CONF_MASK_ZCOMPR,    NEMA_CONF_POS_ZCOMPR    ) ? "enabled" : "disabled" ) ;
    am_util_stdio_printf("Vertex Processor: %s  \t\n", check_bits(config,  NEMA_CONF_MASK_VRX,       NEMA_CONF_POS_VRX       ) ? "enabled" : "disabled" ) ;
    am_util_stdio_printf("Z-Buffer:         %s  \t\n", check_bits(config,  NEMA_CONF_MASK_ZBUF,      NEMA_CONF_POS_ZBUF      ) ? "enabled" : "disabled" ) ;
    am_util_stdio_printf("TSc:              %s  \t\n", check_bits(config,  NEMA_CONF_MASK_TSC,       NEMA_CONF_POS_TSC       ) ? "enabled" : "disabled" ) ;
    am_util_stdio_printf("Clock-Gating:     %s  \t\n", check_bits(config,  NEMA_CONF_MASK_CG,        NEMA_CONF_POS_CG        ) ? "enabled" : "disabled" ) ;
    am_util_stdio_printf("Cores:            %lu\t\t\n", check_bits(config, NEMA_CONF_MASK_CORES,     NEMA_CONF_POS_CORES    ));
    am_util_stdio_printf("Threads:          %u\t\t\n", (0x1 << check_bits(config, NEMA_CONF_MASK_THREADS, NEMA_CONF_POS_THREADS )));
    am_util_stdio_printf("-----------------------------------------------------------\n");
    return 0;
}

void
test_dithering(void)
{
    int32_t i;

    int32_t x_min = 0;
    int32_t x_max = RESX - 1;   //!< RESX-1;

    nema_cmdlist_t sCL = nema_cl_create();
    nema_cl_bind(&sCL);

    nema_set_blend_fill( NEMA_BL_SRC );
    for ( i = 0; i < RESX; i++ )
    {
        nema_draw_line(x_min, i, x_max, i, OPAQUE(i << 16 | i << 8 | i << 0));
    }

    nema_cl_unbind();
    nema_cl_submit(&sCL);
    nema_cl_wait(&sCL);
    nemadc_set_layer(0, &g_sDCLayer);
    frame_transfer();
    nema_cl_destroy(&sCL);
}

void
blit_texture_part_tsc6a(void)
{
    nema_cmdlist_t sCL;

    sCL = nema_cl_create();
    nema_cl_bind(&sCL);
    nema_bind_dst_tex(g_sFB.bo.base_phys, g_sFB.w, g_sFB.h, g_sFB.format, g_sFB.stride);

    char *psram_greek_islend_100x100_tsc6a = psram_malloc(sizeof(g_ui8GreekIsland100x100TSC6A));
    memcpy(psram_greek_islend_100x100_tsc6a, g_ui8GreekIsland100x100TSC6A, sizeof(g_ui8GreekIsland100x100TSC6A));
    g_sGreekIslandTSC6A.bo.base_phys = (uintptr_t)psram_greek_islend_100x100_tsc6a;
    g_sGreekIslandTSC6A.bo.base_virt = (void*)g_sGreekIslandTSC6A.bo.base_phys;
    nema_bind_src_tex(g_sGreekIslandTSC6A.bo.base_phys,
                      g_sGreekIslandTSC6A.w,
                      g_sGreekIslandTSC6A.h,
                      g_sGreekIslandTSC6A.format,
                      g_sGreekIslandTSC6A.stride,
                      NEMA_FILTER_PS);
    nema_set_clip(0, 0, RESX, RESY);
    nema_set_blend_blit(NEMA_BL_SRC);
    nema_blit(100, 100);

    img_obj_t sObjMonkey48x48TSC6A = {{0}, 48, 48, -1, 0, NEMA_TSC6A, 0};
    sObjMonkey48x48TSC6A.bo.base_phys = (uintptr_t)g_ui8Monkeytrans48x48TSC6A;
    sObjMonkey48x48TSC6A.bo.base_virt = (void*)sObjMonkey48x48TSC6A.bo.base_phys;
    nema_bind_src_tex(sObjMonkey48x48TSC6A.bo.base_phys,
                      sObjMonkey48x48TSC6A.w,
                      sObjMonkey48x48TSC6A.h,
                      sObjMonkey48x48TSC6A.format,
                      sObjMonkey48x48TSC6A.stride,
                      NEMA_FILTER_PS);
    nema_set_clip(140, 140, 25, 15);
    nema_set_blend_blit(NEMA_BL_SIMPLE);
    nema_blit(125, 125);

    nema_cl_submit(&sCL);
    nema_cl_wait(&sCL);
    nemadc_set_layer(0, &g_sDCLayer);
    frame_transfer();

    nema_cl_destroy(&sCL);

    psram_free((void*)g_sGreekIslandTSC6A.bo.base_phys);

}

void
test_dirty(void)
{
    int32_t ui16MinX, ui16MinY, maxx, maxy;

    nema_cmdlist_t sCL;

    sCL = nema_cl_create();
    nema_cl_bind(&sCL);
    nema_bind_dst_tex(g_sFB.bo.base_phys, g_sFB.w, g_sFB.h, g_sFB.format, g_sFB.stride);

    nema_clear_dirty_region();

    g_sGreekIslandRGBA8888.bo.base_phys = (uintptr_t)g_ui8GreekIsland100x100RGBA;
    g_sGreekIslandRGBA8888.bo.base_virt = (void*)g_sGreekIslandRGBA8888.bo.base_phys;
    nema_bind_src_tex(g_sGreekIslandRGBA8888.bo.base_phys,
                      g_sGreekIslandRGBA8888.w,
                      g_sGreekIslandRGBA8888.h,
                      g_sGreekIslandRGBA8888.format,
                      g_sGreekIslandRGBA8888.stride,
                      NEMA_FILTER_PS);

    nema_set_blend_blit(NEMA_BL_SRC);
    nema_blit_rect_fit( 50,  50, 100, 50);
    nema_blit_rect_fit(200, 200, 100, 50);

    nema_cl_unbind();
    nema_cl_submit(&sCL);
    nema_cl_wait(&sCL);
    nemadc_set_layer(0, &g_sDCLayer);
    frame_transfer();

    nema_get_dirty_region(&ui16MinX, &ui16MinY, &maxx, &maxy);

    nema_cl_rewind(&sCL);
    nema_cl_bind(&sCL);
    nema_set_blend_fill(NEMA_BL_SRC);
    nema_draw_rect(ui16MinX, ui16MinY,  maxx - ui16MinX + 1, maxy - ui16MinY + 1, OPAQUE(RED));

    nema_cl_unbind();
    nema_cl_submit(&sCL);
    nema_cl_wait(&sCL);
    nemadc_set_layer(0, &g_sDCLayer);
    frame_transfer();
    nema_cl_destroy(&sCL);
}

void
test_edge(void)
{
    unsigned char matrix[3][3] =
    {
        {0xFF, 0xFF, 0xFF},
        {0x56, 0x56, 0x56},
        {0x56, 0x56, 0x56}
    };

    nema_cmdlist_t sCL;

    sCL = nema_cl_create();
    nema_cl_bind(&sCL);
    nema_bind_dst_tex(g_sFB.bo.base_phys, g_sFB.w, g_sFB.h, g_sFB.format, g_sFB.stride);

    img_obj_t obj_girl_tsc6 = {{0}, 256, 256, -1, 0, NEMA_TSC6, 0};
    obj_girl_tsc6.bo.base_phys = (uintptr_t)g_ui8GirlTSC6;
    obj_girl_tsc6.bo.base_virt = (void*)obj_girl_tsc6.bo.base_phys;
    nema_bind_src_tex(obj_girl_tsc6.bo.base_phys,
                      obj_girl_tsc6.w,
                      obj_girl_tsc6.h,
                      obj_girl_tsc6.format,
                      obj_girl_tsc6.stride,
                      NEMA_FILTER_PS);

    nema_blit_edge(matrix, 20, 20, 256, 256);

    nema_cl_unbind();
    nema_cl_submit(&sCL);
    nema_cl_wait(&sCL);
    nemadc_set_layer(0, &g_sDCLayer);
    frame_transfer();

    nema_cl_destroy(&sCL);
}

void
test_blur(void)
{
    unsigned char matrix[3][3] =
    {
        {0xFF, 0xFF, 0xFF},
        {0x56, 0x56, 0x56},
        {0x56, 0x56, 0x56}
    };

    nema_cmdlist_t sCL;

    sCL = nema_cl_create();
    nema_cl_bind(&sCL);
    nema_bind_dst_tex(g_sFB.bo.base_phys, g_sFB.w, g_sFB.h, g_sFB.format, g_sFB.stride);

    img_obj_t obj_girl_tsc6 = {{0}, 256, 256, -1, 0, NEMA_TSC6, 0};
    obj_girl_tsc6.bo.base_phys = (uintptr_t)g_ui8GirlTSC6;
    obj_girl_tsc6.bo.base_virt = (void*)obj_girl_tsc6.bo.base_phys;
    nema_bind_src_tex(obj_girl_tsc6.bo.base_phys,
                      obj_girl_tsc6.w,
                      obj_girl_tsc6.h,
                      obj_girl_tsc6.format,
                      obj_girl_tsc6.stride,
                      NEMA_FILTER_PS);

    nema_blit_blur(matrix, 20, 20, 256, 256);

    nema_cl_unbind();
    nema_cl_submit(&sCL);
    nema_cl_wait(&sCL);
    nemadc_set_layer(0, &g_sDCLayer);
    frame_transfer();

    nema_cl_destroy(&sCL);
}

img_obj_t g_sObjWatchfaceStar360x360TSC6A = {{0}, 360, 360, 360*3, 0, NEMA_TSC6A, 0};

void
test_watch(void)
{

#define PARTICAL_DISPALY_OFFSET          ((g_sDispCfg.ui16ResX - RESX)/2)

    nema_cmdlist_t sCL;

    nema_bind_dst_tex(g_sFB.bo.base_phys, g_sFB.w, g_sFB.h, g_sFB.format, g_sFB.stride);

    sCL = nema_cl_create_sized(0x1000);
    nema_cl_bind(&sCL);
    g_sObjWatchfaceStar360x360TSC6A.bo.base_phys = (uintptr_t)g_ui8WatchfaceStar360x360TSC6A;
    g_sObjWatchfaceStar360x360TSC6A.bo.base_virt = (void*)g_sObjWatchfaceStar360x360TSC6A.bo.base_phys;
    nema_bind_src_tex(g_sObjWatchfaceStar360x360TSC6A.bo.base_phys,
                      g_sObjWatchfaceStar360x360TSC6A.w,
                      g_sObjWatchfaceStar360x360TSC6A.h,
                      g_sObjWatchfaceStar360x360TSC6A.format,
                      g_sObjWatchfaceStar360x360TSC6A.stride,
                      NEMA_FILTER_PS);
    nema_set_clip(0, 0, 360, 360);
    nema_set_blend_blit(NEMA_BL_SRC);
    nema_blit(0, 0);

    nema_bind_font(&g_sArial48pt1b);
    nema_print("19:59", 0, 0, 360, 360, OPAQUE(BLACK), NEMA_ALIGNX_CENTER | NEMA_ALIGNY_CENTER);

    nema_cl_submit(&sCL);
    nema_cl_wait(&sCL);
    nemadc_set_layer(0, &g_sDCLayer);
    frame_transfer();
    nema_cl_destroy(&sCL);
//!< am_util_delay_ms(1000);
    nema_calculate_fps();

//!< fb_reload_tsc6_new_size(180, 60);

#if 1
    int32_t offset = 0; // just for test
    int32_t fb_start_x = 100;
    int32_t fb_start_y = 140;
    int32_t fb_size_x = 180;
    int32_t fb_size_y = 60;
    int32_t ui16MinX = 0;
    int32_t ui16MinY = 0;
    g_sDCLayer.startx        = -fb_start_x + offset;
    g_sDCLayer.starty        = -fb_start_y + offset;
    g_sDCLayer.resx          = fb_start_x + fb_size_x - offset;
    g_sDCLayer.resy          = fb_start_y + fb_size_y - offset;
    nemadc_timing(fb_size_x, 4, 10, 1,
                 fb_size_y, 10, 50, 1);
#endif

    while(1)
    {
        sCL = nema_cl_create();
        nema_cl_bind(&sCL);
        g_sObjWatchfaceStar360x360TSC6A.bo.base_phys = (uintptr_t)g_ui8WatchfaceStar360x360TSC6A;
        g_sObjWatchfaceStar360x360TSC6A.bo.base_virt = (void*)g_sObjWatchfaceStar360x360TSC6A.bo.base_phys;
        nema_bind_src_tex(g_sObjWatchfaceStar360x360TSC6A.bo.base_phys,
                          g_sObjWatchfaceStar360x360TSC6A.w,
                          g_sObjWatchfaceStar360x360TSC6A.h,
                          g_sObjWatchfaceStar360x360TSC6A.format,
                          g_sObjWatchfaceStar360x360TSC6A.stride,
                          NEMA_FILTER_PS);

        nema_set_clip(100, 140, 180, 60);
        nema_set_blend_blit(NEMA_BL_SRC);
        nema_blit(0, 0);

        nema_bind_font(&g_sArial48pt1b);
        nema_print("19:59", 0, 0, 360, 360, OPAQUE(BLACK), NEMA_ALIGNX_CENTER | NEMA_ALIGNY_CENTER);

        nema_cl_submit(&sCL);
        nema_cl_wait(&sCL);
        nemadc_layer_disable(0);
        nemadc_set_layer(0, &g_sDCLayer);
        am_devices_dc_dsi_raydium_set_region(fb_size_x,
                                          fb_size_y,
                                          fb_start_x + ui16MinX + g_sDispCfg.ui16Offset + PARTICAL_DISPALY_OFFSET / 2 * 2,
                                          fb_start_y + ui16MinY + PARTICAL_DISPALY_OFFSET / 2 * 2);
        frame_transfer();
        nema_cl_destroy(&sCL);
//!< am_util_delay_ms(1000);
        nema_calculate_fps();

        sCL = nema_cl_create();
        nema_cl_bind(&sCL);
        nema_bind_src_tex(g_sObjWatchfaceStar360x360TSC6A.bo.base_phys,
                          g_sObjWatchfaceStar360x360TSC6A.w,
                          g_sObjWatchfaceStar360x360TSC6A.h,
                          g_sObjWatchfaceStar360x360TSC6A.format,
                          g_sObjWatchfaceStar360x360TSC6A.stride,
                          NEMA_FILTER_PS);

        nema_set_clip(100, 140, 180, 60);
        nema_set_blend_blit(NEMA_BL_SRC);
        nema_blit(0, 0);

        nema_print("20:00", 0, 0, 360, 360, OPAQUE(BLACK), NEMA_ALIGNX_CENTER | NEMA_ALIGNY_CENTER);
        nema_cl_submit(&sCL);
        nema_cl_wait(&sCL);
        nemadc_set_layer(0, &g_sDCLayer);
        frame_transfer();
        nema_cl_destroy(&sCL);
//!< am_util_delay_ms(1000);
        nema_calculate_fps();
    }
}

int32_t
fillrect_grad()
{
    uint16_t ui16HeartRateTable[51];
    uint32_t i;

    int32_t x1, y1;
    color_var_t sCol0, sCol1, sCol2;

#define HEART_RATE_NUM      27

    for (i = 0; i < HEART_RATE_NUM; i++)
    {
        ui16HeartRateTable[i] = rand() % 80 + 120;
        // am_util_stdio_printf("ui16HeartRateTable[%d] = %d\n", i, ui16HeartRateTable[i]);
    }

    start_time = nema_get_time();

    nema_cmdlist_t cl0 = nema_cl_create();
    nema_cl_bind(&cl0);

    nema_enable_gradient(1);

    nema_set_blend_fill ( NEMA_BL_SRC );
    // Col 1
    // NEMA_P_RAST_EMULATION;
    x1 = 60; y1 = 100;
    sCol0.r = 150; sCol0.g = 0; sCol0.b = 0; sCol0.a = 255;
    sCol1.r = 150; sCol1.g = 0; sCol1.b = 0; sCol1.a = 255; // x gradient
    sCol2.r = 50; sCol2.g = 0; sCol2.b = 0; sCol2.a = 255; // y gradient
    nema_interpolate_rect_colors(x1, y1, 256, 256-50, &sCol0, &sCol1, &sCol2);
    nema_fill_rect(x1, y1, 256, 256-50, 0);

    nema_enable_gradient(0);

    for (i = 0; i < HEART_RATE_NUM; i++)
    {
        nema_fill_quad(60 + 10 * i, 100,
                       60 + 10 * (i + 1), 100,
                       60 + 10 * (i + 1), ui16HeartRateTable[i + 1],
                       60 + 10 * i, ui16HeartRateTable[i],
                       BLACK);
    }

    nema_enable_aa(1, 1, 1, 1);
    for (i = 0; i < HEART_RATE_NUM-1; i++)
    {
        nema_draw_line_aa(60 + 10 * i, ui16HeartRateTable[i],
                       60 + 10 * (i + 1), ui16HeartRateTable[i + 1],
                       3, RED);
    }

    nema_cl_submit(&cl0);
    nema_cl_wait(&cl0);

    end_time = nema_get_time();
    am_util_stdio_printf("heart beat rate used %f s\n", end_time - start_time);

    nemadc_set_layer(0, &g_sDCLayer);
    frame_transfer();

    nema_enable_gradient(0);

    nema_cl_unbind();

    return 0;
}

typedef struct
{
    uint16_t outer_x;
    uint16_t outer_y;
    uint32_t outer_color;
    float outer_start_degree;
    float outer_end_degree;

    uint16_t middle_x;
    uint16_t middle_y;
    uint32_t middle_color;
    float middle_start_degree;
    float middle_end_degree;

    uint16_t inner_x;
    uint16_t inner_y;
    uint32_t inner_color;
    float inner_start_degree;
    float inner_end_degree;
}t_scenario_arc;

t_scenario_arc g_scenario_arc = {RESX, RESY, 0x00FF9C33, 45, 270,
                                 RESX - 60,  RESY - 60,  0x00DAF15A, 45, 180,
                                 RESX - 120, RESY - 120, 0x006F54FF, 45, 225};

int32_t
draw_arc(t_scenario_arc *scenario_arc)
{

    float degree;
    int32_t x, y;
    uint16_t r;
    uint16_t width_common = 20;

    uint32_t gray_color = 0x00333333;

    start_time = nema_get_time();
    nema_cmdlist_t cl0 = nema_cl_create();
    nema_cl_bind(&cl0);

    nema_set_blend_fill ( NEMA_BL_SRC );

    // outer gray start
    r = scenario_arc->outer_x / 2 - width_common / 2;

    for (degree = 45; degree < 315; degree += 0.5)
    {
          x = (int32_t)(scenario_arc->outer_x / 2 + r * nema_sin(degree));
          y = (int32_t)(scenario_arc->outer_y / 2 + r * nema_cos(degree));
          nema_fill_circle(x, y, width_common / 2, gray_color);
    }
    nema_cl_submit(&cl0);
    nema_cl_wait(&cl0);
    nema_cl_rewind(&cl0);
    nema_set_blend_fill(NEMA_BL_SRC);

    // middle gray start
    r = scenario_arc->middle_x / 2 - width_common / 2;

    for (degree = 45; degree < 315; degree += 0.5)
    {
        x = (int32_t)(scenario_arc->middle_x / 2 + r * nema_sin(degree));
        x += 30;
        y = (int32_t)(scenario_arc->middle_y / 2 + r * nema_cos(degree));
        y += 30;
        nema_fill_circle(x, y, width_common / 2, gray_color);
    }
    nema_cl_submit(&cl0);
    nema_cl_wait(&cl0);
    nema_cl_rewind(&cl0);
    nema_set_blend_fill(NEMA_BL_SRC);

    // inner gray start
    r = scenario_arc->inner_x / 2 - width_common / 2;

    for (degree = 45; degree < 315; degree += 0.5)
    {
        x = (int32_t)(scenario_arc->inner_x / 2 + r * nema_sin(degree));
        x += 60;
        y = (int32_t)(scenario_arc->inner_y / 2 + r * nema_cos(degree));
        y += 60;
        nema_fill_circle(x, y, width_common / 2, gray_color);
    }
    nema_cl_submit(&cl0);
    nema_cl_wait(&cl0);
    nema_cl_rewind(&cl0);
    nema_set_blend_fill(NEMA_BL_SRC);

    // outer color start
    nema_cl_rewind(&cl0);
    nema_set_blend_fill ( NEMA_BL_SRC );
    r = scenario_arc->outer_x / 2 - width_common / 2;

    for (degree = scenario_arc->outer_start_degree; degree < scenario_arc->outer_end_degree; degree += 0.5)
    {
        x = (int32_t)(scenario_arc->outer_x / 2 + r * nema_sin(degree));
        //x+=2;
        y = (int32_t)(scenario_arc->outer_y / 2 + r * nema_cos(degree));
        nema_fill_circle(x, y, width_common / 2, scenario_arc->outer_color);
    }
    nema_cl_submit(&cl0);
    nema_cl_wait(&cl0);
    nema_cl_rewind(&cl0);
    nema_set_blend_fill ( NEMA_BL_SRC );

    // middle color start
    r = scenario_arc->middle_x / 2 - width_common / 2;

    for (degree = scenario_arc->middle_start_degree; degree < scenario_arc->middle_end_degree; degree += 0.5)
    {
        x = (int32_t)(scenario_arc->middle_x / 2 + r * nema_sin(degree));
        x += 30;
        y = (int32_t)(scenario_arc->middle_y / 2 + r * nema_cos(degree));
        y += 30;
        nema_fill_circle(x, y, width_common / 2, scenario_arc->middle_color);
    }
    nema_cl_submit(&cl0);
    nema_cl_wait(&cl0);
    nema_cl_rewind(&cl0);
    nema_set_blend_fill ( NEMA_BL_SRC );

    // inner color start
    r = scenario_arc->inner_x / 2 - width_common / 2;

    for (degree = scenario_arc->inner_start_degree; degree < scenario_arc->inner_end_degree; degree += 0.5)
    {
        x = (int32_t)(scenario_arc->inner_x / 2 + r * nema_sin(degree));
        x += 60;
        y = (int32_t)(scenario_arc->inner_y / 2 + r * nema_cos(degree));
        y += 60;
        nema_fill_circle(x, y, width_common / 2, scenario_arc->inner_color);
    }

    nema_cl_submit(&cl0);
    nema_cl_wait(&cl0);

    end_time = nema_get_time();
    am_util_stdio_printf("draw_arc used %f s\n", end_time - start_time);

    nema_cl_rewind(&cl0);
    img_obj_t sObjSport150x168RGBA565 = {{0}, 150, 168, -1, 0, NEMA_RGB565, 0};

    sObjSport150x168RGBA565.bo.base_phys = (uintptr_t)g_ui8Sport150x168RGBA565;
    sObjSport150x168RGBA565.bo.base_virt = (void*)sObjSport150x168RGBA565.bo.base_phys;
    nema_bind_src_tex(sObjSport150x168RGBA565.bo.base_phys,
                      sObjSport150x168RGBA565.w,
                      sObjSport150x168RGBA565.h,
                      sObjSport150x168RGBA565.format,
                      sObjSport150x168RGBA565.stride,
                      NEMA_FILTER_PS);
    nema_set_blend_blit(NEMA_BL_SRC);
    nema_blit(120, 110);
    nema_cl_submit(&cl0);
    nema_cl_wait(&cl0);

    nemadc_set_layer(0, &g_sDCLayer);
    frame_transfer();

    nema_cl_unbind();
    nema_cl_destroy(&cl0);

    return 0;
}

int32_t
blit()
{
    psram_malloc_init((void *)MSPI_XIPMM_BASE_ADDRESS, (uintptr_t)MSPI_XIPMM_BASE_ADDRESS, 64*1024*1024, 1);

    test_config();

//!< Texture SRAM to SRAM rgba8888
//blit_clear();
#if 0
    load_objects();

    fb_release();
#endif

    test_nema_malloc();

#if 0
        //!< PSRAM as frame buffer
        am_util_delay_ms(SHOW_INTERVAL);
        blit_clear();
        fb_reload_psram_tsc6();
        blit_font();
#endif

#if 0
        am_util_delay_ms(SHOW_INTERVAL);
        blit_clear();
        fb_reload_rgba8888();
        test_watch();
#endif

#if 0
    //!<  blit_clear_rgba8888();
    fb_reload_tsc6();
    blit_texture_100x100_tile();
    fb_release();
    am_util_delay_ms(SHOW_INTERVAL);
#endif

#if 0
    // blit_clear_rgba8888();
    fb_reload_tsc6();
    tsuite2d_tileblit();
    fb_release();
    am_util_delay_ms(SHOW_INTERVAL);
#endif

    blit_clear_rgba8888();
    fb_reload_rgba8888();
    blit_texture_100x100();
    fb_release();
    am_util_delay_ms(SHOW_INTERVAL);

    blit_clear_rgba8888();
    fb_reload_rgba565();
    blit_texture_360x360();
    fb_release();
    am_util_delay_ms(SHOW_INTERVAL);

    blit_clear_rgba8888();
    fb_reload_rgba8888();
    blit_texture_psram_100x100();
    fb_release();
    am_util_delay_ms(SHOW_INTERVAL);

    blit_clear_rgba8888();
    fb_reload_rgba565();
    blit_texture_psram_360x360();
    fb_release();
    am_util_delay_ms(SHOW_INTERVAL);

#if 0
    blit_clear_tsc6();
    fb_reload_tsc6();
    blit_texture_100x100();
    fb_release();
    am_util_delay_ms(SHOW_INTERVAL);

    blit_clear_tsc6();
    fb_reload_tsc6();
    blit_texture_200x200();
    fb_release();
    am_util_delay_ms(SHOW_INTERVAL);
#endif

    blit_clear_rgba8888();
    fb_reload_rgba8888();
    fillrect_grad();
    fb_release();
    am_util_delay_ms(SHOW_INTERVAL);

#if 0
    blit_clear_rgba8888();
    fb_reload_rgba8888();
    draw_arc(&g_scenario_arc);
    fb_release();
    am_util_delay_ms(SHOW_INTERVAL);
#endif

#if 0
    blit_clear_rgba8888();
    fb_reload_rgba8888();
    blit_font();
    fb_release();
    am_util_delay_ms(SHOW_INTERVAL);

    blit_clear_rgba8888();
    fb_reload_rgba565();
    blit_font();
    fb_release();
    am_util_delay_ms(SHOW_INTERVAL);
#endif
    // while(1);

#if 1
    //!< Texture PSRAM to SRAM rgba8888
    am_util_delay_ms(SHOW_INTERVAL);
    blit_clear_rgba8888();
    fb_reload_rgba8888();
    blit_texture_psram();
    fb_release();

//!< Texture PSRAM to SRAM tsc6

//    am_util_delay_ms(SHOW_INTERVAL);
//    blit_clear();
//    fb_reload_tsc6();
//    blit_texture_psram();
//    fb_release();

    //!< Font SRAM to SRAM rgba8888
    am_util_delay_ms(SHOW_INTERVAL);
    blit_clear_rgba8888();
    //load_objects_with_free();
    fb_reload_rgba8888();
    blit_font();
    fb_release();

    //!< Font PSRAM to SRAM rgba8888
    am_util_delay_ms(SHOW_INTERVAL);
    blit_clear_rgba8888();
    fb_reload_rgba8888();
    blit_font_psram();
    fb_release();

    //!< Texture PSRAM to SRAM rgba8888 with blend
    am_util_delay_ms(SHOW_INTERVAL);
    blit_clear_rgba8888();
    fb_reload_rgba8888();
    blit_texture_psram_blend();
    fb_release();

    //!< Texture PSRAM to SRAM rgba8888 with scale
    am_util_delay_ms(SHOW_INTERVAL);
    blit_clear_rgba8888();
    fb_reload_rgba8888();
    blit_texture_psram_scale();
    fb_release();

    //!< Texture PSRAM to SRAM with rotate
    am_util_delay_ms(SHOW_INTERVAL);
    blit_clear_rgba8888();
    fb_reload_rgba8888();
    blit_texture_psram_rotate_rgba8888();
    fb_release();

    am_util_delay_ms(SHOW_INTERVAL);
    blit_clear_rgba8888();
    fb_reload_rgba8888();
    blit_texture_psram_rotate_tsc6();
    fb_release();

    am_util_delay_ms(SHOW_INTERVAL);
    blit_clear_rgba8888();
    fb_reload_rgba8888();
    blit_texture_psram_rotate_tsc6a();
    fb_release();

    //!< fill const color
    am_util_delay_ms(SHOW_INTERVAL);
    blit_clear_rgba8888();
    fb_reload_rgba8888();
    fill_const();
    fb_release();

    am_util_delay_ms(SHOW_INTERVAL);
    blit_clear_rgba8888();
    fb_reload_rgba8888();
    test_dirty();
    fb_release();
#endif

    //!< Texture PSRAM to SRAM tsc6
    am_util_delay_ms(SHOW_INTERVAL);
    blit_clear();
    fb_reload_tsc6();
    blit_texture_psram();
    fb_release();

    //!< Font SRAM to SRAM tsc6
    am_util_delay_ms(SHOW_INTERVAL);
    blit_clear();
    fb_reload_tsc6();
    blit_font();
    fb_release();

    //!< Font PSRAM to SRAM tsc6
    am_util_delay_ms(SHOW_INTERVAL);
    blit_clear();
    fb_reload_tsc6();
    blit_font_psram();
    fb_release();

    //!< Texture PSRAM to SRAM tsc6 with scale
    am_util_delay_ms(SHOW_INTERVAL);
    blit_clear();
    fb_reload_tsc6();
    blit_texture_psram_scale();
    fb_release();

    am_util_delay_ms(SHOW_INTERVAL);
    blit_clear();
    fb_reload_tsc6();
    blit_texture_psram_rotate_rgba8888();
    fb_release();

    am_util_delay_ms(SHOW_INTERVAL);
    blit_clear();
    fb_reload_tsc6();
    blit_texture_psram_rotate_tsc6();
    fb_release();

    am_util_delay_ms(SHOW_INTERVAL);
    blit_clear();
    fb_reload_tsc6();
    blit_texture_psram_rotate_tsc6a();
    fb_release();

    am_util_delay_ms(SHOW_INTERVAL);
    blit_clear();
    fb_reload_tsc6();
    fill_const();
    fb_release();

    am_util_delay_ms(SHOW_INTERVAL);
    blit_clear();
    fb_reload_tsc6();
    test_dithering();
    fb_release();

    am_util_delay_ms(SHOW_INTERVAL);
    blit_clear();
    fb_reload_tsc6();
    test_blur();
    fb_release();

    am_util_delay_ms(SHOW_INTERVAL);
    blit_clear();
    fb_reload_tsc6();
    test_edge();
    fb_release();
    am_util_delay_ms(1000);

#if 1
    am_util_delay_ms(SHOW_INTERVAL);
    blit_clear();
    fb_reload_tsc6();
    blit_texture_part_tsc6a();
    fb_release();
#endif

#if 1
    /* rgb24 texture */
    am_util_delay_ms(SHOW_INTERVAL);
    blit_clear();
    fb_reload_rgb24();
    blit_rgb24_texture();
    fb_release();
#endif

#if 1
    /* PSRAM as frame buffer */
    am_util_delay_ms(SHOW_INTERVAL);
    blit_clear();
    fb_reload_tsc6();
    blit_font();
    fb_release();
#endif

    am_util_delay_ms(SHOW_INTERVAL);
    blit_clear();
    fb_reload_tsc6();
    fill_round_rect_test();
    fb_release();

    am_util_delay_ms(1000);

    am_util_delay_ms(SHOW_INTERVAL);
    blit_clear();
    fb_reload_tsc6();
    // blit_psram_clear();
    // fill_round_rect_test();
    test_watch();
    fb_release();

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
    uint8_t ui8LanesNum = g_sDispCfg.ui8NumLanes;
    uint8_t ui8DbiWidth = g_sDispCfg.eDbiWidth;
    uint32_t ui32FreqTrim = g_sDispCfg.eDsiFreq;
    pixel_format_t eFormat = FMT_RGB888;
    uint32_t ui32MipiCfg = MIPICFG_16RGB888_OPT0;    //!< default config
    uint16_t ui16MinX, ui16MinY;
    nemadc_initial_config_t sDCConfig;
    am_devices_dc_dsi_raydium_config_t sDisplayPanelConfig;
    //
    // External power on
    //
    am_bsp_external_pwr_on();
    am_util_delay_ms(100);
    am_bsp_low_power_init();
// #### INTERNAL BEGIN ####
    //uint32_t ui32Val0 = AM_REGVAL(0x40020080);
    //AM_REGVAL(0x40020080) = ui32Val0 + 24; //24; // 34 please increase trim value with a proper value, to increase VDDC to 750mV.
    //am_util_delay_ms(2000);
    ////mcuctrlpriv.ldoreg2.memldoactivetrim 0x40040088 bit5~0
    //ui32Val0 = AM_REGVAL(0x40020088);
    //AM_REGVAL(0x40020088) = ui32Val0 + 20; // please increase trim value with a proper value, to increase VDDF to 950mV.
    //am_util_delay_ms(100);
// #### INTERNAL END ####

    am_hal_gpio_pinconfig(22, am_hal_gpio_pincfg_output);
    am_hal_gpio_output_clear(22);
    am_hal_gpio_pinconfig(23, am_hal_gpio_pincfg_output);
    am_hal_gpio_output_clear(23);
    am_hal_gpio_pinconfig(24, am_hal_gpio_pincfg_output);
    am_hal_gpio_output_clear(24);
    am_hal_gpio_pinconfig(25, am_hal_gpio_pincfg_output);
    am_hal_gpio_output_clear(25);

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
        return 0;
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

    am_devices_dc_dsi_raydium_hardware_reset();
    sDCConfig.eInterface = DISP_INTERFACE_DBIDSI;
    sDCConfig.ui32PixelFormat = ui32MipiCfg;
    sDisplayPanelConfig.ui32PixelFormat = ui32MipiCfg;
    nemadc_configure(&sDCConfig);
    am_devices_dc_dsi_raydium_init(&sDisplayPanelConfig);
    //
    // Initialize the printf interface for ITM/SWO output.
    //
    am_bsp_itm_printf_enable();
    //
    // Clear the terminal and print the banner.
    //
    am_util_stdio_terminal_clear();
    //
    // Configure the MSPI and PSRAM Device.
    //
    uint32_t ui32Status = am_devices_mspi_psram_aps12808l_ddr_init(MSPI_PSRAM_MODULE, &MSPI_PSRAM_OctalCE1MSPIConfig, &g_pPSRAMHandle, &g_pMSPIHandle);
    if (AM_DEVICES_MSPI_PSRAM_STATUS_SUCCESS != ui32Status)
    {
        am_util_stdio_printf("Failed to configure the MSPI and PSRAM Device correctly!\n");
    }
    NVIC_SetPriority(mspi_interrupts[MSPI_PSRAM_MODULE], AM_IRQ_PRIORITY_DEFAULT);
    NVIC_EnableIRQ(mspi_interrupts[MSPI_PSRAM_MODULE]);

    //
    // Enable XIP mode.
    //
    ui32Status = am_devices_mspi_psram_aps12808l_ddr_enable_xip(g_pPSRAMHandle);
    if (AM_DEVICES_MSPI_PSRAM_STATUS_SUCCESS == ui32Status)
    {
        am_util_stdio_printf("Enable XIP mode in the MSPI!\n");
    }
    else
    {
        am_util_stdio_printf("Failed to enable XIP mode in the MSPI!\n");
    }
    //
    // Enable debug printing to the console.
    //
#ifdef AM_DEBUG_PRINTF
    am_bsp_debug_printf_enable();
#endif

    //
    // Global interrupt enable
    //
    am_hal_interrupt_master_enable();

    //
    // Initialize NemaGFX
    //
    nema_init();
#if 0
    TIMER->TMR0CMP0 = 0x1000000;
    TIMER->CTRL0 = 0x11;  // enable timer
#else
    am_hal_timer_config_t       sTimerConfig;
    am_hal_timer_default_config_set(&sTimerConfig);
    sTimerConfig.eInputClock = AM_HAL_TIMER_CLOCK_HFRC_DIV16;
    sTimerConfig.eFunction = AM_HAL_TIMER_FN_EDGE;

    am_hal_timer_config(0, &sTimerConfig);
    am_hal_timer_start(0);
#endif

    //
    // Initialize plotting interface.
    //
    am_util_debug_printf("watchface Example\n");

    blit();

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

