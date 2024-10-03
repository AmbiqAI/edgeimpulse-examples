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


// Uncomment this if SVG needs to be loaded from the filesystem
#define LOAD_SVG_FROM_FILESYSTEM

#include "nema_core.h"
#include "nema_utils.h"
#include "nema_vg_tsvg.h"
#include "nema_vg.h"
#include "Arial_ttf.h"
#include <stdlib.h>
#define VECTOR_FONT Arial_ttf

#ifndef DONT_USE_NEMADC
#include "nema_dc.h"
#endif


#ifndef LOAD_SVG_FROM_FILESYSTEM
#include "./tsvgs/tiger.tsvg.h"
#endif

#define RESX 800
#define RESY 600

#ifndef DEFAULT_FILENAME
#define DEFAULT_FILENAME  "tsvgs/tiger.tsvg"
#endif

nema_img_obj_t fb = {{0}, RESX, RESY, RESX*4, 0, NEMA_RGBX8888, 0};

#ifndef DONT_USE_NEMADC
nemadc_layer_t dc_layer = {(void *)0, 0, RESX, RESY, -1, 0, 0, RESX, RESY, 0xff, NEMADC_BL_SRC, 0, NEMADC_RGBA8888, 0, 0, 0, 0, 0};
#endif


#ifdef LOAD_SVG_FROM_FILESYSTEM

static void* load_binary(const char *filename)
{
    FILE* fp = NULL;
    size_t size;

    unsigned char* binary = NULL;
    fp = fopen(filename, "rb");
    if (!fp) goto error;
    fseek(fp, 0, SEEK_END);
    size = ftell(fp);
    fseek(fp, 0, SEEK_SET);
    binary = (unsigned char*)malloc(size+1);
    if (binary == NULL) goto error;
    if (fread(binary, 1, size, fp) != size) goto error;
    binary[size] = '\0';	// Must be null terminated.
    fclose(fp);

    return (void*)binary;

error:
    if (fp) fclose(fp);
    if (binary) free(binary);

    return NULL;
}

#endif

static void load_objects(void)
{
    //Load memory objects
    fb.bo = nema_buffer_create(fb.stride*fb.h);
    nema_buffer_map(&fb.bo);
    //printf("FB: V:%p P:0x%08x\n", (void *)fb.bo.base_virt, fb.bo.base_phys);

#ifndef DONT_USE_NEMADC
    dc_layer.baseaddr_phys = fb.bo.base_phys;
    dc_layer.baseaddr_virt = fb.bo.base_virt;
#endif

    //printf("FB: V:%p P:0x%08x\n", (void *)fb.bo.base_virt, fb.bo.base_phys);
}

static void destroy_objects(void)
{
    //Destroy memory objects
    nema_buffer_unmap(&fb.bo);
    nema_buffer_destroy(&fb.bo);
}

static void set_dc(void)
{
#ifndef DONT_USE_NEMADC
    //Format        | Pixclock | RESX | FP | SYNC | BP | RESY | FP | SYNC | BP
    //800x600, 60Hz | 40.000   | 800  | 40 | 128  | 88 | 600  | 1  | 4    | 23
    nemadc_timing(800, 40, 128, 88, 600, 1, 4, 23);
    nemadc_set_layer(0, &dc_layer);
#endif
}

int main (int argc, char *argv[])
{
    // Initialize NemaGFX
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
    set_dc();

#if 0
    // Use circular cl
    nema_cmdlist_t cl  = nema_cl_create_sized(1024);
    nema_cl_bind_circular(&cl);
#else
    nema_cmdlist_t cl  = nema_cl_create();
    nema_cl_bind(&cl);
#endif

    // Clear fb
    nema_bind_dst_tex(fb.bo.base_phys, fb.w, fb.h, fb.format, fb.stride);
    nema_set_clip(0, 0, RESX, RESY);
    nema_clear(0xffffffffU); //white

    // Init VG library
    nema_vg_init(RESX, RESY);

    // Bind one default font in case our tsvg contains text
    //---------------------------------------------
    nema_vg_bind_font(&VECTOR_FONT);

#if 0
    size_t data_size = 9000;
    size_t seg_size = 1000;
    nema_buffer_t seg = nema_buffer_create_pool(0, seg_size);
    nema_buffer_t data = nema_buffer_create_pool(0, data_size);
    nema_vg_bind_clip_coords_buf((void*)&seg, seg_size, (void*)&data, data_size);
#endif
    nema_vg_handle_large_coords(1U, 1U);

#ifdef LOAD_SVG_FROM_FILESYSTEM
    char* filename = DEFAULT_FILENAME;
    if (argc > 1) {
        filename = argv[1];
    }

    unsigned char* binary = load_binary(filename);
    nema_vg_draw_tsvg(binary);
#else
    nema_vg_draw_tsvg(tiger_tsvg);
#endif

    nema_cl_submit(&cl);
    nema_cl_wait(&cl);
    uint32_t error = nema_vg_get_error();

    if(error != NEMA_VG_ERR_NO_ERROR)
    {
        printf("error = %x\n", error);
    }
    nema_save_file("fb.rgba",RESX*RESY*4, fb.bo.base_virt);

    //---------------------------------------------

    nema_vg_deinit();

    nema_buffer_unmap(&fb.bo);
    nema_buffer_destroy(&fb.bo);

#ifdef LOAD_SVG_FROM_FILESYSTEM
    free(binary);
#endif

    nema_cl_destroy(&cl);


    return 0;
}
