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

#define NEMA_FONT_IMPLEMENTATION
#include "Vera16pt8b.h"
#undef NEMA_FONT_IMPLEMENTATION

#define RESX 800
#define RESY 600

img_obj_t fb = {{0}, RESX, RESY, RESX*4, 0, NEMA_RGBA8888, 0};

#define IMG_COUNT 37

const char img_names[IMG_COUNT][20] =
{
    "gui01",    // 0
    "gui02",    // 1
    "gui03",    // 2
    "gui04",    // 3
    "gui05",    // 4
    "gui06",    // 5
    "gui07",    // 6
    "gui08",    // 7
    "image01",  // 8
    "image02",  // 9
    "image03",  //10
    "image04",  //11
    "image05",  //12
    "kodim01",  //13
    "kodim02",  //14
    "kodim03",  //15
    "kodim04",  //16
    "kodim05",  //17
    "kodim06",  //18
    "kodim07",  //19
    "kodim08",  //20
    "kodim09",  //21
    "kodim10",  //22
    "kodim11",  //23
    "kodim12",  //24
    "kodim13",  //25
    "kodim14",  //26
    "kodim15",  //27
    "kodim16",  //28
    "kodim17",  //29
    "kodim18",  //30
    "kodim19",  //31
    "kodim20",  //32
    "kodim21",  //33
    "kodim22",  //34
    "kodim23",  //35
    "kodim24"   //36
};

img_obj_t imgs_rgba[IMG_COUNT];
img_obj_t imgs_tsc[IMG_COUNT];

void
load_objects(void)
{
    //Load memory objects
    fb.bo = nema_buffer_create(fb.stride*fb.h);
    nema_buffer_map(&fb.bo);
    printf("FB: V:%p P:0x%08lx\n", (void *)fb.bo.base_virt, fb.bo.base_phys);
    int i = 0;

    Vera16pt8b.bo = nema_load_file("Vera16pt8b.bin", -1, 0);

    for (i = 0; i < IMG_COUNT; ++i) {
        char tmp_filename[40];

        snprintf(tmp_filename, 40, "rgba/%s.rgba", img_names[i]);
        imgs_rgba[i].bo = nema_load_file(tmp_filename, -1, 0);
        imgs_rgba[i].format = NEMA_RGBA8888;

        snprintf(tmp_filename, 40, "tsc/%s.tsc4", img_names[i]);
        imgs_tsc[i].bo = nema_load_file(tmp_filename, -1, 0);
        imgs_tsc[i].format = NEMA_TSC4;

        if ( i < 8 ) {
            imgs_rgba[i].w = imgs_rgba[i].h = 320;
            imgs_tsc[i].w  = imgs_tsc[i].h = 320;
        } else if (i < 13 ) {
            imgs_rgba[i].w = imgs_rgba[i].h = 256;
            imgs_tsc[i].w  = imgs_tsc[i].h = 256;
        } else {
            imgs_rgba[i].w = imgs_rgba[i].h = 512;
            imgs_tsc[i].w  = imgs_tsc[i].h = 512;
        }
    }
}

#define Y_OFFSET 86

#ifndef DONT_USE_NEMADC
nemadc_layer_t dc_layer;
#endif

int
main()
{
    int ret = 0;

    //Initialize NemaGFX
    ret = nema_init();
    if (ret) return ret;
    //Initialize Nema|dc

#ifndef DONT_USE_NEMADC
    ret = nemadc_init();
    if (ret) return ret;

    nemadc_cursor_enable(0);
#endif

    load_objects();

#ifndef DONT_USE_NEMADC
    dc_layer.baseaddr_phys = fb.bo.base_phys;
    dc_layer.baseaddr_virt = fb.bo.base_virt;
    dc_layer.resx          = fb.w;
    dc_layer.resy          = fb.h;
    dc_layer.sizex         = fb.w;
    dc_layer.sizey         = fb.h;
    dc_layer.stride        = fb.stride;
    dc_layer.buscfg        = 0;
    dc_layer.format        = NEMADC_RGBA8888;
    dc_layer.startx        = 0;
    dc_layer.starty        = 0;
    dc_layer.alpha         = 0xff;
    dc_layer.blendmode     = NEMADC_BL_SRC;

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

    nema_fill_rect(0, 0, RESX, 70, nema_rgba(78, 72, 72, 255));
    nema_bind_font(&Vera16pt8b);
    int x = 0, y = 0;

    int cursor_x = 0;
    int cursor_y = 0;
    nema_print_to_position("Think Silicon TSC4 Image Compression Demo\n",
        &cursor_x, &cursor_y, x, y, RESX, 35, 0xffffffff, NEMA_ALIGNX_CENTER );
    x = 0;

    nema_print("Compressed", x, cursor_y, RESX/2, 35, 0xffffffff, NEMA_ALIGNX_CENTER);
    x = RESX/2;
    nema_print("Not Compressed", x, cursor_y, RESX/2, 35, 0xffffffff, NEMA_ALIGNX_CENTER);

    nema_cl_submit(&cl);
    nema_cl_wait(&cl);

    //image index
    int idx = 0;
    char c;

    do {
        nema_cl_rewind(&cl);

        //Set Blending Mode
        nema_set_blend_fill(NEMA_BL_SRC);
        //Fill Rectangle with Color
        nema_fill_rect(0, Y_OFFSET, RESX, RESY-Y_OFFSET, 0);

        //Set Blending Mode for Blitting
        nema_set_blend_blit(NEMA_BL_SRC);

        int tmp_w = imgs_rgba[idx].w > RESX/2 ? (RESX/2-8)&(~0x3) : imgs_rgba[idx].w;
        printf("%dx%d\n", tmp_w, imgs_rgba[idx].h);

        nema_bind_src_tex(imgs_tsc[idx].bo.base_phys, imgs_tsc[idx].w, imgs_tsc[idx].h, imgs_tsc[idx].format, -1, NEMA_FILTER_PS);
        nema_blit_rect(0, Y_OFFSET, tmp_w, imgs_tsc[idx].h);

        nema_bind_src_tex(imgs_rgba[idx].bo.base_phys, imgs_rgba[idx].w, imgs_rgba[idx].h, imgs_rgba[idx].format, -1, NEMA_FILTER_PS);
        nema_blit_rect(RESX/2, Y_OFFSET, tmp_w, imgs_rgba[idx].h);

        nema_cl_submit(&cl);
        nema_cl_wait(&cl);

        idx = (idx+1)%IMG_COUNT;
        c = getchar();
    } while (c != 'q');

    nema_cl_destroy(&cl);

    return 0;
}
