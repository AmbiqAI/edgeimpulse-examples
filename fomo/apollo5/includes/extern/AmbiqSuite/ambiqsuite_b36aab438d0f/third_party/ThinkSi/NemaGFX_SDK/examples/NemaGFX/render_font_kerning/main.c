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
#ifndef RESX
#define RESX 800
#endif

#ifndef RESY
#define RESY 600
#endif

#define NEMA_FONT_LOAD_FROM_BIN

// ----------------------- INCLUDE FONTS -----------------------------------------

#define NEMA_FONT_IMPLEMENTATION
#include "DejaVuSerif24pt4b.h"
#include "DejaVuSerif24pt4b_kern.h"
#include "Vera24pt4b.h"
#include "Vera24pt4b_kern.h"
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

#ifdef NEMA_FONT_LOAD_FROM_BIN
    DejaVuSerif24pt4b.bo = nema_load_file("DejaVuSerif24pt4b.bin", -1, 0);
    DejaVuSerif24pt4b_kern.bo = nema_load_file("DejaVuSerif24pt4b_kern.bin", -1, 0);

    Vera24pt4b.bo = nema_load_file("Vera24pt4b.bin", -1, 0);
    Vera24pt4b_kern.bo = nema_load_file("Vera24pt4b_kern.bin", -1, 0);
#else
    DejaVuSerif24pt4b.bo = nema_buffer_create( DejaVuSerif24pt4b.bitmap_size );
    (void)nema_buffer_map(&DejaVuSerif24pt4b.bo);
    (void)nema_memcpy(DejaVuSerif24pt4b.bo.base_virt, DejaVuSerif24pt4b.bitmap, (size_t)DejaVuSerif24pt4b.bitmap_size);

    DejaVuSerif24pt4b_kern.bo = nema_buffer_create( DejaVuSerif24pt4b_kern.bitmap_size );
    (void)nema_buffer_map(&DejaVuSerif24pt4b_kern.bo);
    (void)nema_memcpy(DejaVuSerif24pt4b_kern.bo.base_virt, DejaVuSerif24pt4b_kern.bitmap, (size_t)DejaVuSerif24pt4b_kern.bitmap_size);

    Vera24pt4b.bo = nema_buffer_create( Vera24pt4b.bitmap_size );
    (void)nema_buffer_map(&Vera24pt4b.bo);
    (void)nema_memcpy(Vera24pt4b.bo.base_virt, Vera24pt4b.bitmap, (size_t)Vera24pt4b.bitmap_size);

    Vera24pt4b_kern.bo = nema_buffer_create( Vera24pt4b_kern.bitmap_size );
    (void)nema_buffer_map(&Vera24pt4b_kern.bo);
    (void)nema_memcpy(Vera24pt4b_kern.bo.base_virt, Vera24pt4b_kern.bitmap, (size_t)Vera24pt4b_kern.bitmap_size);
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

    nema_font_t *kern_font   = &DejaVuSerif24pt4b_kern;
    nema_font_t *simple_font = &DejaVuSerif24pt4b;

    char *str = "AV";
    nema_fill_rect(0, 0, 100, 50, 0x50505050);
    nema_fill_rect(0, 50, 100, 50, 0x50505050);

    nema_bind_font(simple_font);
    nema_print(str, 0, 0, 100, 50, 0xff00ff80U, NEMA_ALIGNX_CENTER|NEMA_ALIGNY_CENTER);
    nema_bind_font(kern_font);
    nema_print(str, 0, 50, 100, 50, 0xff00ff80U, NEMA_ALIGNX_CENTER|NEMA_ALIGNY_CENTER);

    str = "FA";
    nema_set_blend_fill(NEMA_BL_SRC);
    nema_fill_rect(105, 0, 100, 50, 0x50505050);
    nema_fill_rect(105, 50, 100, 50, 0x50505050);

    nema_bind_font(simple_font);
    nema_print(str, 105, 0, 100, 50, 0xff00ff80U, NEMA_ALIGNX_CENTER|NEMA_ALIGNY_CENTER);
    nema_bind_font(kern_font);
    nema_print(str, 105, 50, 100, 50, 0xff00ff80U, NEMA_ALIGNX_CENTER|NEMA_ALIGNY_CENTER);

    str = "Wö";
    nema_set_blend_fill(NEMA_BL_SRC);
    nema_fill_rect(210, 0, 100, 50, 0x50505050);
    nema_fill_rect(210, 50, 100, 50, 0x50505050);

    nema_bind_font(simple_font);
    nema_print(str, 210, 0, 100, 50, 0xff00ff80U, NEMA_ALIGNX_CENTER|NEMA_ALIGNY_CENTER);
    nema_bind_font(kern_font);
    nema_print(str, 210, 50, 100, 50, 0xff00ff80U, NEMA_ALIGNX_CENTER|NEMA_ALIGNY_CENTER);

    str = "Yu";
    nema_set_blend_fill(NEMA_BL_SRC);
    nema_fill_rect(315, 0, 100, 50, 0x50505050);
    nema_fill_rect(315, 50, 100, 50, 0x50505050);

    nema_bind_font(simple_font);
    nema_print(str, 315, 0, 100, 50, 0xff00ff80U, NEMA_ALIGNX_CENTER|NEMA_ALIGNY_CENTER);
    nema_bind_font(kern_font);
    nema_print(str, 315, 50, 100, 50, 0xff00ff80U, NEMA_ALIGNX_CENTER|NEMA_ALIGNY_CENTER);

    str = "VA";
    nema_set_blend_fill(NEMA_BL_SRC);
    nema_fill_rect(420, 0, 100, 50, 0x50505050);
    nema_fill_rect(420, 50, 100, 50, 0x50505050);

    nema_bind_font(simple_font);
    nema_print(str, 420, 0, 100, 50, 0xff00ff80U, NEMA_ALIGNX_CENTER|NEMA_ALIGNY_CENTER);
    nema_bind_font(kern_font);
    nema_print(str, 420, 50, 100, 50, 0xff00ff80U, NEMA_ALIGNX_CENTER|NEMA_ALIGNY_CENTER);

    str = "AT";
    nema_set_blend_fill(NEMA_BL_SRC);
    nema_fill_rect(525, 0, 100, 50, 0x50505050);
    nema_fill_rect(525, 50, 100, 50, 0x50505050);

    nema_bind_font(simple_font);
    nema_print(str, 525, 0, 100, 50, 0xff00ff80U, NEMA_ALIGNX_CENTER|NEMA_ALIGNY_CENTER);
    nema_bind_font(kern_font);
    nema_print(str, 525, 50, 100, 50, 0xff00ff80U, NEMA_ALIGNX_CENTER|NEMA_ALIGNY_CENTER);

    str = "LT";
    nema_set_blend_fill(NEMA_BL_SRC);
    nema_fill_rect(630, 0, 100, 50, 0x50505050);
    nema_fill_rect(630, 50, 100, 50, 0x50505050);

    nema_bind_font(simple_font);
    nema_print(str, 630, 0, 100, 50, 0xff00ff80U, NEMA_ALIGNX_CENTER|NEMA_ALIGNY_CENTER);
    nema_bind_font(kern_font);
    nema_print(str, 630, 50, 100, 50, 0xff00ff80U, NEMA_ALIGNX_CENTER|NEMA_ALIGNY_CENTER);

    str = "Ya";
    nema_set_blend_fill(NEMA_BL_SRC);
    nema_fill_rect(0, 105, 100, 50, 0x50505050);
    nema_fill_rect(0, 155, 100, 50, 0x50505050);

    nema_bind_font(simple_font);
    nema_print(str, 0, 105, 100, 50, 0xff00ff80U, NEMA_ALIGNX_CENTER|NEMA_ALIGNY_CENTER);
    nema_bind_font(kern_font);
    nema_print(str, 0, 155, 100, 50, 0xff00ff80U, NEMA_ALIGNX_CENTER|NEMA_ALIGNY_CENTER);

    str = "Wa";
    nema_set_blend_fill(NEMA_BL_SRC);
    nema_fill_rect(105, 105, 100, 50, 0x50505050);
    nema_fill_rect(105, 155, 100, 50, 0x50505050);

    nema_bind_font(simple_font);
    nema_print(str, 105, 105, 100, 50, 0xff00ff80U, NEMA_ALIGNX_CENTER|NEMA_ALIGNY_CENTER);
    nema_bind_font(kern_font);
    nema_print(str, 105, 155, 100, 50, 0xff00ff80U, NEMA_ALIGNX_CENTER|NEMA_ALIGNY_CENTER);


    str = "Ye";
    nema_set_blend_fill(NEMA_BL_SRC);
    nema_fill_rect(210, 105, 100, 50, 0x50505050);
    nema_fill_rect(210, 155, 100, 50, 0x50505050);

    nema_bind_font(simple_font);
    nema_print(str, 210, 105, 100, 50, 0xff00ff80U, NEMA_ALIGNX_CENTER|NEMA_ALIGNY_CENTER);
    nema_bind_font(kern_font);
    nema_print(str, 210, 155, 100, 50, 0xff00ff80U, NEMA_ALIGNX_CENTER|NEMA_ALIGNY_CENTER);

    str = "FE";
    nema_set_blend_fill(NEMA_BL_SRC);
    nema_fill_rect(315, 105, 100, 50, 0x50505050);
    nema_fill_rect(315, 155, 100, 50, 0x50505050);

    nema_bind_font(simple_font);
    nema_print(str, 315, 105, 100, 50, 0xff00ff80U, NEMA_ALIGNX_CENTER|NEMA_ALIGNY_CENTER);
    nema_bind_font(kern_font);
    nema_print(str, 315, 155, 100, 50, 0xff00ff80U, NEMA_ALIGNX_CENTER|NEMA_ALIGNY_CENTER);

    str = "AD";
    nema_set_blend_fill(NEMA_BL_SRC);
    nema_fill_rect(420, 105, 100, 50, 0x50505050);
    nema_fill_rect(420, 155, 100, 50, 0x50505050);

    nema_bind_font(simple_font);
    nema_print(str, 420, 105, 100, 50, 0xff00ff80U, NEMA_ALIGNX_CENTER|NEMA_ALIGNY_CENTER);
    nema_bind_font(kern_font);
    nema_print(str, 420, 155, 100, 50, 0xff00ff80U, NEMA_ALIGNX_CENTER|NEMA_ALIGNY_CENTER);


    str = "AF";
    nema_set_blend_fill(NEMA_BL_SRC);
    nema_fill_rect(525, 105, 100, 50, 0x50505050);
    nema_fill_rect(525, 155, 100, 50, 0x50505050);

    nema_bind_font(simple_font);
    nema_print(str, 525, 105, 100, 50, 0xff00ff80U, NEMA_ALIGNX_CENTER|NEMA_ALIGNY_CENTER);
    nema_bind_font(kern_font);
    nema_print(str, 525, 155, 100, 50, 0xff00ff80U, NEMA_ALIGNX_CENTER|NEMA_ALIGNY_CENTER);

    str = "Væ";
    nema_set_blend_fill(NEMA_BL_SRC);
    nema_fill_rect(630, 105, 100, 50, 0x50505050);
    nema_fill_rect(630, 155, 100, 50, 0x50505050);

    nema_bind_font(simple_font);
    nema_print(str, 630, 105, 100, 50, 0xff00ff80U, NEMA_ALIGNX_CENTER|NEMA_ALIGNY_CENTER);
    nema_bind_font(kern_font);
    nema_print(str, 630, 155, 100, 50, 0xff00ff80U, NEMA_ALIGNX_CENTER|NEMA_ALIGNY_CENTER);

    str = "Testing Text Eng Sample";
    nema_set_blend_fill(NEMA_BL_SRC);
    nema_fill_rect(0, 215, 800, 60, 0x50505050);
    nema_fill_rect(0, 280, 800, 60, 0x50505050);

    nema_bind_font(simple_font);
    nema_print(str, 0, 215, 800, 60, 0xff00ff80U, NEMA_ALIGNX_CENTER|NEMA_ALIGNY_TOP);
    nema_bind_font(kern_font);
    nema_print(str, 0, 280, 800, 60, 0xff00ff80U, NEMA_ALIGNX_CENTER|NEMA_ALIGNY_CENTER);

    str = "Δοκιμη με Ελληνικά";
    nema_set_blend_fill(NEMA_BL_SRC);
    nema_fill_rect(0, 345, 800, 60, 0x50505050);
    nema_fill_rect(0, 410, 800, 60, 0x50505050);

    nema_bind_font(simple_font);
    nema_print(str, 0, 345, 800, 60, 0xff00ff80U, NEMA_ALIGNX_CENTER|NEMA_ALIGNY_CENTER);
    nema_bind_font(kern_font);
    nema_print(str, 0, 410, 800, 60, 0xff00ff80U, NEMA_ALIGNX_CENTER|NEMA_ALIGNY_CENTER);

    //Mixed latin-greek characters
    str = "ΑV, AV, Wα, Wa, Wo, Wο";
    nema_set_blend_fill(NEMA_BL_SRC);
    nema_fill_rect(0, 475, 800, 60, 0x50505050);

    nema_bind_font(kern_font);
    nema_print(str, 0, 475, 800, 60, 0xff00ff80U, NEMA_ALIGNX_CENTER|NEMA_ALIGNY_CENTER);

    nema_cl_submit(&cl);
    nema_cl_destroy(&cl);

#ifndef DONT_USE_NEMADC
    nemadc_set_layer(0, &dc_layer);
#endif

    return 0;
}
