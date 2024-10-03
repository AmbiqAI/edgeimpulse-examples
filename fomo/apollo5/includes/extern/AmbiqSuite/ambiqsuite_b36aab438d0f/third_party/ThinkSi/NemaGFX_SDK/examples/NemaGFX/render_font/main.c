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
#include "nema_core.h"
#include "nema_utils.h"
#ifndef DONT_USE_NEMADC
#include "nema_dc.h"
#endif

#include "nema_font.h"

// ------------------------ USER DEFINES ---------------------------------------

#ifndef USE_1BIT_FONT
#define USE_1BIT_FONT 1
#endif
#ifndef USE_2BIT_FONT
#define USE_2BIT_FONT 1
#endif
#ifndef USE_4BIT_FONT
#define USE_4BIT_FONT 1
#endif
#ifndef USE_8BIT_FONT
#define USE_8BIT_FONT 1
#endif

#ifndef RESX
#define RESX 800
#endif

#ifndef RESY
#define RESY 600
#endif

#define USE_BINS  0

#if USE_BINS
#define NEMA_FONT_LOAD_FROM_BIN
#endif

// ----------------------- INCLUDE FONTS -----------------------------------------

#define NEMA_FONT_IMPLEMENTATION
#if USE_1BIT_FONT != 0
    #include "DejaVuSerif8pt1b.h"
#endif
#if USE_2BIT_FONT != 0
    #include "DejaVuSerif8pt2b.h"
#endif
#if USE_4BIT_FONT != 0
    #include "DejaVuSerif8pt4b.h"
#endif
#if USE_8BIT_FONT != 0
    #include "DejaVuSerif8pt8b.h"
#endif
#undef NEMA_FONT_IMPLEMENTATION

// -------------------------------------------------------------------------------

static img_obj_t fb = {{0}, RESX, RESY, RESX*4, 0, NEMA_RGBA8888, 0};

#ifndef DONT_USE_NEMADC
static nemadc_layer_t dc_layer = {(void *)0, 0, RESX, RESY, -1, 0, 0, RESX, RESY, 0xff, (int)NEMADC_BL_SRC, 0, NEMADC_RGBA8888, 0, 0, 0, 0, 0};
#endif

static void
load_objects(void)
{
    //Load memory objects
    fb.bo = nema_buffer_create(fb.stride*(int)fb.h);
    (void)nema_buffer_map(&fb.bo);

#ifndef DONT_USE_NEMADC
    dc_layer.baseaddr_phys = fb.bo.base_phys;
    dc_layer.baseaddr_virt = fb.bo.base_virt;
#endif

#if USE_BINS

#if USE_1BIT_FONT != 0
    DejaVuSerif8pt1b.bo = nema_load_file("DejaVuSerif8pt1b.bin", -1, 0);
#endif
#if USE_2BIT_FONT != 0
    DejaVuSerif8pt2b.bo = nema_load_file("DejaVuSerif8pt2b.bin", -1, 0);
#endif
#if USE_4BIT_FONT != 0
    DejaVuSerif8pt4b.bo = nema_load_file("DejaVuSerif8pt4b.bin", -1, 0);
#endif
#if USE_8BIT_FONT != 0
    DejaVuSerif8pt8b.bo = nema_load_file("DejaVuSerif8pt8b.bin", -1, 0);
#endif

#else

#if USE_1BIT_FONT != 0
    DejaVuSerif8pt1b.bo = nema_buffer_create( DejaVuSerif8pt1b.bitmap_size );
    (void)nema_buffer_map(&DejaVuSerif8pt1b.bo);
    (void)nema_memcpy(DejaVuSerif8pt1b.bo.base_virt, DejaVuSerif8pt1b.bitmap, (size_t)DejaVuSerif8pt1b.bitmap_size);
#endif

#if USE_2BIT_FONT != 0
    DejaVuSerif8pt2b.bo = nema_buffer_create( DejaVuSerif8pt2b.bitmap_size );
    (void)nema_buffer_map(&DejaVuSerif8pt2b.bo);
    (void)nema_memcpy(DejaVuSerif8pt2b.bo.base_virt, DejaVuSerif8pt2b.bitmap, (size_t)DejaVuSerif8pt2b.bitmap_size);
#endif

#if USE_4BIT_FONT != 0
    DejaVuSerif8pt4b.bo = nema_buffer_create( DejaVuSerif8pt4b.bitmap_size );
    (void)nema_buffer_map(&DejaVuSerif8pt4b.bo);
    (void)nema_memcpy(DejaVuSerif8pt4b.bo.base_virt, DejaVuSerif8pt4b.bitmap, (size_t)DejaVuSerif8pt4b.bitmap_size);
#endif

#if USE_8BIT_FONT != 0
    DejaVuSerif8pt8b.bo = nema_buffer_create( DejaVuSerif8pt8b.bitmap_size );
    (void)nema_buffer_map(&DejaVuSerif8pt8b.bo);
    (void)nema_memcpy(DejaVuSerif8pt8b.bo.base_virt, DejaVuSerif8pt8b.bitmap, (size_t)DejaVuSerif8pt8b.bitmap_size);
#endif

#endif
}

#ifdef STANDALONE
int
main(int argc, char *argv[])
#else
int
render_font(void)
#endif
{
    //Initialize NemaGFX
    if ( nema_init() < 0 ) {
        return -1;
    }

#ifndef DONT_USE_NEMADC
    //Initialize NemaDC
    if ( nemadc_init() < 0 ) {
        return -2;
    }
#endif

    load_objects();

    // (void)nema_event_init(0, 0, 0, 0, 0);

#ifndef DONT_USE_NEMADC
    //Format        | Pixclock | RESX | FP | SYNC | BP | RESY | FP | SYNC | BP
    //800x600, 60Hz | 40.000   | 800  | 40 | 128  | 88 | 600  | 1  | 4    | 23
    nemadc_timing(800, 40, 128, 88, 600, 1, 4, 23);
    nemadc_set_layer(0, &dc_layer);
#endif

    nema_cmdlist_t cl;

    //Create Command Lists
    cl  = nema_cl_create();

    //Bind Command List
    nema_cl_bind(&cl);

    //Bind Framebuffer
    nema_bind_dst_tex(fb.bo.base_phys, fb.w, fb.h, fb.format, fb.stride);
    //Set Clipping Rectangle
    nema_set_clip(0, 0, RESX, RESY);
    nema_set_blend_fill(NEMA_BL_SRC);
    //Fill Rectangle with Color
    nema_fill_rect(0, 0, RESX, RESY, 0x10101010);

    char str[] = "0 bpp - Hello World!!!\n|-------------------------------|\nThe quick brown fox jumps     over the lazy dog!\n\
ThisIsAVeryVeryVeryVeryVeryVeryVeryVeryVeryVeryVeryVeryLongString\nH NemaGFX υποστηρίζει ΚΑΙ Unicode χαρακτήρες!!!";

    int w, h;

#if   USE_1BIT_FONT != 0
    nema_bind_font(&DejaVuSerif8pt1b);
#elif USE_2BIT_FONT != 0
    nema_bind_font(&DejaVuSerif8pt2b);
#elif USE_4BIT_FONT != 0
    nema_bind_font(&DejaVuSerif8pt4b);
#elif USE_8BIT_FONT != 0
    nema_bind_font(&DejaVuSerif8pt8b);
#endif

    const int spacing = 5;

    (void)nema_string_get_bbox(str, &w, &h, RESX/2-spacing*3, 1);
    h += spacing;    w += spacing;

    int x, y;

    const int xs[4] = {spacing, w+2*spacing, spacing, w+2*spacing};
    const int ys[4] = {spacing, spacing, h+2*spacing, h+2*spacing};
    int idx = 0;

#if USE_1BIT_FONT != 0
    x = xs[idx];
    y = ys[idx];
    ++idx;

    str[0] = '1';

    nema_set_blend_fill(NEMA_BL_SRC);
    nema_fill_rect(x, y, w, h, 0x70707070U);
    nema_bind_font(&DejaVuSerif8pt1b);
    nema_print(str, x, y, w, h, 0xff00ff80U, NEMA_ALIGNX_LEFT   |NEMA_TEXT_WRAP|NEMA_ALIGNY_TOP);

    nema_cl_submit(&cl);
    (void)nema_cl_wait(&cl);
    nema_cl_rewind(&cl);
#endif

#if USE_2BIT_FONT != 0
    x = xs[idx];
    y = ys[idx];
    ++idx;

    str[0] = '2';

    nema_set_blend_fill(NEMA_BL_SRC);
    nema_fill_rect(x, y, w, h, 0x70707070U);
    nema_bind_font(&DejaVuSerif8pt2b);
    nema_print(str, x, y, w, h, 0xff00ffffU, NEMA_ALIGNX_RIGHT  |NEMA_TEXT_WRAP|NEMA_ALIGNY_BOTTOM);

    nema_cl_submit(&cl);
    (void)nema_cl_wait(&cl);
    nema_cl_rewind(&cl);
#endif

#if USE_4BIT_FONT != 0
    x = xs[idx];
    y = ys[idx];
    ++idx;

    str[0] = '4';

    nema_set_blend_fill(NEMA_BL_SRC);
    nema_fill_rect(x, y, w, h, 0x70707070U);
    nema_bind_font(&DejaVuSerif8pt4b);
    nema_print(str, x, y, w, h, 0xff0080ffU, NEMA_ALIGNX_CENTER |NEMA_TEXT_WRAP|NEMA_ALIGNY_CENTER);

    nema_cl_submit(&cl);
    (void)nema_cl_wait(&cl);
    nema_cl_rewind(&cl);
#endif

#if USE_8BIT_FONT != 0
    x = xs[idx];
    y = ys[idx];
    ++idx;

    str[0] = '8';

    nema_set_blend_fill(NEMA_BL_SRC);
    nema_fill_rect(x, y, w, h, 0x70707070U);
    nema_bind_font(&DejaVuSerif8pt8b);
    nema_print(str, x, y, w, h, 0x808ff08fU, NEMA_ALIGNX_JUSTIFY|NEMA_TEXT_WRAP|NEMA_ALIGNY_JUSTIFY);

    nema_cl_submit(&cl);
    (void)nema_cl_wait(&cl);
#endif

    nema_cl_destroy(&cl);

#ifndef DONT_USE_NEMADC
    nemadc_set_layer(0, &dc_layer);
#endif

    // nema_save_file("render_font.rgba", fb.bo.size, (void *)fb.bo.base_virt);

    return 0;
}
