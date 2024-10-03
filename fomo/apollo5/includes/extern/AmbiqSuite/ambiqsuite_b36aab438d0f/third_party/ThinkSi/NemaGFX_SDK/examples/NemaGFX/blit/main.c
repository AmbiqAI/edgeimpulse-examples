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

#define RESX 800
#define RESY 600

img_obj_t fb = {{0}, RESX, RESY, RESX*4, 0, NEMA_RGBA8888, 0};
img_obj_t TSi_logo = {{0}, 301, 154, 301*4, 0, NEMA_RGBA8888, 0};

#ifndef DONT_USE_NEMADC
nemadc_layer_t dc_layer = {(void *)0, 0, RESX, RESY, -1, 0, 0, RESX, RESY, 0xff, NEMADC_BL_SRC, 0, NEMADC_RGBA8888, 0, 0, 0, 0, 0};
#endif

#include "ThinkSiliconLogo301x154.rgba.h"

void
load_objects(void)
{
    //Load memory objects
    fb.bo = nema_buffer_create(fb.stride*fb.h);
    nema_buffer_map(&fb.bo);
    printf("FB: V:%p P:0x%08x\n", (void *)fb.bo.base_virt, fb.bo.base_phys);

#ifndef DONT_USE_NEMADC
    dc_layer.baseaddr_phys = fb.bo.base_phys;
    dc_layer.baseaddr_virt = fb.bo.base_virt;
#endif

    printf("FB: V:%p P:0x%08x\n", (void *)fb.bo.base_virt, fb.bo.base_phys);

    // TSi_logo.bo = nema_load_file("ThinkSiliconLogo301x154.rgba", -1, 0);
    TSi_logo.bo = nema_buffer_create(ThinkSiliconLogo301x154_rgba_len);
    nema_memcpy(TSi_logo.bo.base_virt, ThinkSiliconLogo301x154_rgba, ThinkSiliconLogo301x154_rgba_len);
}

#ifdef STANDALONE
int main()
#else
int blit()
#endif
{
    //Initialize NemaGFX
    if ( nema_init() != 0 ) {
        return -1;
    }

#ifndef DONT_USE_NEMADC
    //Initialize NemaDC
    if ( nemadc_init() != 0 ) {
        return -2;
    }
#endif

    load_objects();

#ifndef DONT_USE_NEMADC
    //Format        | Pixclock | RESX | FP | SYNC | BP | RESY | FP | SYNC | BP
    //800x600, 60Hz | 40.000   | 800  | 40 | 128  | 88 | 600  | 1  | 4    | 23
    nemadc_timing(800, 40, 128, 88, 600, 1, 4, 23);
    nemadc_set_layer(0, &dc_layer);
#endif

    //Create Command Lists
    nema_cmdlist_t cl  = nema_cl_create();

    //Bind Command List
    nema_cl_bind(&cl);

    //Bind Framebuffer
    nema_bind_dst_tex(fb.bo.base_phys, fb.w, fb.h, fb.format, fb.stride);
    //Set Clipping Rectangle
    nema_set_clip(0, 0, RESX, RESY);
    //Set Blending Mode
    nema_set_blend_fill(NEMA_BL_SRC);
    //Fill Rectangle with Color
    nema_fill_rect(0, 0, RESX, RESY, 0);

    nema_bind_src_tex(TSi_logo.bo.base_phys, TSi_logo.w, TSi_logo.h, TSi_logo.format, TSi_logo.stride, NEMA_FILTER_BL);
    nema_set_blend_blit(NEMA_BL_SRC);

    //blit entire TSi_logo
    nema_blit(0, 0);

    //blit part of TSi_logo (80x80)
    nema_blit_rect(320, 0, 80, 80);

    //blit TSi_logo to fit in a 80x80 rectangle
    nema_blit_rect_fit(420, 0, 80, 80);

    nema_bind_src_tex(TSi_logo.bo.base_phys, TSi_logo.w, TSi_logo.h, TSi_logo.format, TSi_logo.stride, NEMA_FILTER_BL);
    //blit TSi_logo to fit in a quadrilateral
    nema_blit_quad_fit(300, 200, 450, 300, 300, 400, 150, 300);

    int x = 400;
    int y = 400;

    nema_blit_rounded(x, y, 20);

    nema_cl_submit(&cl);
    nema_cl_wait(&cl);

    nema_cl_destroy(&cl);

    return 0;
}
