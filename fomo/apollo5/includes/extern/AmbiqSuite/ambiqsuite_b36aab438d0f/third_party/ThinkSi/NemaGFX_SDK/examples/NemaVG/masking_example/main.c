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
#include "nema_vg.h"
#include "mask296x154_a8.h"
#include "mask296x154_a4.h"
#include "mask296x154_a2.h"
#include "mask296x154_a1.h"
#include "path_data.h"

#ifndef DONT_USE_NEMADC
#include "nema_dc.h"
#endif


#ifndef DONT_USE_NEMADC
nemadc_layer_t dc_layer = {(void *)0, 0, RESX, RESY, -1, 0, 0, RESX, RESY, 0xff, NEMADC_BL_SRC, 0, NEMADC_RGBA8888, 0, 0, 0, 0, 0};
#endif


void
load_objects(void)
{
    //Load memory objects
    fb.bo = nema_buffer_create_pool(NEMA_MEM_POOL_FB, fb.stride*fb.h);
    nema_buffer_map(&fb.bo);
    printf("FB: V:%p P:0x%08x\n", (void *)fb.bo.base_virt, fb.bo.base_phys);

#ifndef DONT_USE_NEMADC
    dc_layer.baseaddr_phys = fb.bo.base_phys;
    dc_layer.baseaddr_virt = fb.bo.base_virt;
#endif

    mask_a8.bo = nema_buffer_create(mask296x154_a8_length);
    mask_a4.bo = nema_buffer_create(mask296x154_a4_length);
    mask_a2.bo = nema_buffer_create(mask296x154_a2_length);
    mask_a1.bo = nema_buffer_create(mask296x154_a1_length);

    nema_memcpy(mask_a8.bo.base_virt, mask296x154_a8, mask296x154_a8_length);
    nema_memcpy(mask_a4.bo.base_virt, mask296x154_a4, mask296x154_a4_length);
    nema_memcpy(mask_a2.bo.base_virt, mask296x154_a2, mask296x154_a2_length);
    nema_memcpy(mask_a1.bo.base_virt, mask296x154_a1, mask296x154_a1_length);
}

int main()
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
    // Initialize memory objects
    load_objects();

#ifndef DONT_USE_NEMADC
    //Format        | Pixclock | RESX | FP | SYNC | BP | RESY | FP | SYNC | BP
    //800x600, 60Hz | 40.000   | 800  | 40 | 128  | 88 | 600  | 1  | 4    | 23
    nemadc_timing(800, 40, 128, 88, 600, 1, 4, 23);
    nemadc_set_layer(0, &dc_layer);
#endif

    nema_cmdlist_t cl  = nema_cl_create();
    nema_cl_bind(&cl);

    A_masks[0] = &mask_a8;
    A_masks[1] = &mask_a4;
    A_masks[2] = &mask_a2;
    A_masks[3] = &mask_a1;

    // Clear fb
    nema_bind_dst_tex(fb.bo.base_phys, fb.w, fb.h, fb.format, fb.stride);
    nema_set_clip(0, 0, RESX, RESY);

    nema_clear(nema_rgba(0xff,0xff,0xff,0xff));

    nema_matrix3x3_t m_path;

    // Init VG library
    nema_vg_init(RESX, RESY);

    // Create paint
    NEMA_VG_PAINT_HANDLE paint = nema_vg_paint_create();
    nema_vg_paint_set_type(paint, NEMA_VG_PAINT_GRAD_LINEAR);

    NEMA_VG_PATH_HANDLE path_rectangle = nema_vg_path_create();

    NEMA_VG_GRAD_HANDLE gradient = nema_vg_grad_create();
    nema_vg_grad_set(gradient, COLOR_STOPS, stops, colors);
    nema_vg_path_set_shape(path_rectangle, 5, cmds_rectangle, 8, coords_rectangle);

    // Set fill rule
    nema_vg_set_fill_rule(NEMA_VG_FILL_EVEN_ODD);

    //Enable Masking
    nema_vg_masking(1U);

    for(int i=0; i<4; i++){

        //for each iteration, set a different Mask, A8, A4, A2, A1
        nema_vg_set_mask(A_masks[i]);

        nema_mat3x3_load_identity(m_path);
        nema_mat3x3_translate(m_path,i*75 + 5,i*150);
        nema_vg_path_set_matrix(path_rectangle, m_path);

        // Set paint Gradient
        nema_vg_paint_set_grad_linear(paint, gradient, i*10.0f, i*200.0f, 310.0f + i*10, i*200.0f, NEMA_TEX_CLAMP | NEMA_FILTER_BL);

        // Set Mask x,y translation
        nema_vg_set_mask_translation(0,i*150);

        // Draw path (applies paint to path)
        nema_vg_draw_path(path_rectangle, paint);
    }

    // Disable Masking
    nema_vg_masking(0U);

    // Submit command list
    nema_cl_submit(&cl);
    nema_cl_wait(&cl);

    // Save Frame buffer to file
    //nema_save_file("fb.rgba",RESX*RESY*4, fb.bo.base_virt);

    // Destroy memory objects (Deallocate memory)
    nema_vg_path_destroy(path_rectangle);
    nema_vg_grad_destroy(gradient);
    nema_vg_paint_destroy(paint);
    nema_vg_deinit();

    nema_buffer_unmap(&fb.bo);
    nema_buffer_destroy(&fb.bo);

    nema_buffer_destroy(&mask_a8.bo);
    nema_buffer_destroy(&mask_a4.bo);
    nema_buffer_destroy(&mask_a2.bo);
    nema_buffer_destroy(&mask_a1.bo);

    nema_cl_destroy(&cl);

    return 0;
}
