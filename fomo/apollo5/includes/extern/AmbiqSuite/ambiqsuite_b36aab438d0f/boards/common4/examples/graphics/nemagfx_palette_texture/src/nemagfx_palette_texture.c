//*****************************************************************************
//
//! @file nemagfx_palette_texture.c
//!
//! @brief NemaGFX Palette Texture Example
//!
//! @addtogroup graphics_examples Graphics Examples
//!
//! @defgroup nemagfx_palette_texture NemaGFX Palette Texture Example
//! @ingroup graphics_examples
//! @{
//!
//! Purpose: This file demo the palette color format texture usage, the texture in the
//! palette color format will start with a palette section, which is followed
//! by a indexes section.
//*****************************************************************************

//*****************************************************************************
//
// ${copyright}
//
// This is part of revision ${version} of the AmbiqSuite Development Package.
//
//*****************************************************************************

#include "nemagfx_palette_texture.h"

#define CONCAT_PALETTE_INDICES

#ifdef CONCAT_PALETTE_INDICES
#include "texture/boy_indexed_1bits.h"
#include "texture/boy_indexed_2bits.h"
#include "texture/boy_indexed_4bits.h"
#include "texture/boy_indexed_8bits.h"
#else
#include "texture/boy_indices.256.gray.h"
#include "texture/boy_palette.256.rgba.h"

#include "texture/boy_indices.16.gray.h"
#include "texture/boy_palette.16.rgba.h"

#include "texture/boy_indices.4.gray.h"
#include "texture/boy_palette.4.rgba.h"

#include "texture/boy_indices.2.gray.h"
#include "texture/boy_palette.2.rgba.h"
#endif

//*****************************************************************************
//
// Macro definitions
//
//*****************************************************************************
#define RESX 452
#define RESY 452

#define TEXX 250
#define TEXY 250

//*****************************************************************************
//
// Global Variables
//
//*****************************************************************************
//! frame buffer
static img_obj_t fb = {{0}, RESX, RESY, RESX*3, 0, NEMA_RGB24, 0};

#ifdef CONCAT_PALETTE_INDICES
//! Texture
nema_img_obj_t boy_indexed_8bits = {{0}, TEXX, TEXY, -1, 0, NEMA_L8, NEMA_TEX_CLAMP};
nema_img_obj_t boy_indexed_4bits = {{0}, TEXX, TEXY, -1, 0, NEMA_L4, NEMA_TEX_CLAMP};
nema_img_obj_t boy_indexed_2bits = {{0}, TEXX, TEXY, -1, 0, NEMA_L2, NEMA_TEX_CLAMP};
nema_img_obj_t boy_indexed_1bits = {{0}, TEXX, TEXY, -1, 0, NEMA_L1, NEMA_TEX_CLAMP};
#else
//! Palette
nema_img_obj_t boy_palette8 = {{0}, 16, 16, -1, 0, NEMA_RGBA8888, 0};
nema_img_obj_t boy_palette4 = {{0}, 4,   4, -1, 0, NEMA_RGBA8888, 0};
nema_img_obj_t boy_palette2 = {{0}, 2,   2, -1, 0, NEMA_RGBA8888, 0};
nema_img_obj_t boy_palette1 = {{0}, 1,   2, -1, 0, NEMA_RGBA8888, 0};

//! Indices
nema_img_obj_t boy_indices8 = {{0}, TEXX, TEXY, -1, 0, NEMA_L8, NEMA_TEX_CLAMP};
nema_img_obj_t boy_indices4 = {{0}, TEXX, TEXY, -1, 0, NEMA_L4, NEMA_TEX_CLAMP};
nema_img_obj_t boy_indices2 = {{0}, TEXX, TEXY, -1, 0, NEMA_L2, NEMA_TEX_CLAMP};
nema_img_obj_t boy_indices1 = {{0}, TEXX, TEXY, -1, 0, NEMA_L1, NEMA_TEX_CLAMP};
#endif

void
load_objects(void)
{
    //Load memory objects
    fb.bo = nema_buffer_create(fb.stride*fb.h);
    nema_buffer_map(&fb.bo);

#ifdef CONCAT_PALETTE_INDICES
    boy_indexed_8bits.bo = nema_buffer_create(sizeof(boy_indexed_8bits_bin));
    memcpy(boy_indexed_8bits.bo.base_virt, boy_indexed_8bits_bin, sizeof(boy_indexed_8bits_bin));

    boy_indexed_4bits.bo = nema_buffer_create(sizeof(boy_indexed_4bits_bin));
    memcpy(boy_indexed_4bits.bo.base_virt, boy_indexed_4bits_bin, sizeof(boy_indexed_4bits_bin));

    boy_indexed_2bits.bo = nema_buffer_create(sizeof(boy_indexed_2bits_bin));
    memcpy(boy_indexed_2bits.bo.base_virt, boy_indexed_2bits_bin, sizeof(boy_indexed_2bits_bin));

    boy_indexed_1bits.bo = nema_buffer_create(sizeof(boy_indexed_1bits_bin));
    memcpy(boy_indexed_1bits.bo.base_virt, boy_indexed_1bits_bin, sizeof(boy_indexed_1bits_bin));
#else
    boy_palette8.bo = nema_buffer_create(sizeof(boy_palette_256_rgba));
    memcpy(boy_palette8.bo.base_virt, boy_palette_256_rgba, sizeof(boy_palette_256_rgba));
    boy_indices8.bo = nema_buffer_create(sizeof(boy_indices_256_gray));
    memcpy(boy_indices8.bo.base_virt, boy_indices_256_gray, sizeof(boy_indices_256_gray));

    boy_palette4.bo = nema_buffer_create(sizeof(boy_palette_16_rgba));
    memcpy(boy_palette4.bo.base_virt, boy_palette_16_rgba, sizeof(boy_palette_16_rgba));
    boy_indices4.bo = nema_buffer_create(sizeof(boy_indices_16_gray));
    memcpy(boy_indices4.bo.base_virt, boy_indices_16_gray, sizeof(boy_indices_16_gray));

    boy_palette2.bo = nema_buffer_create(sizeof(boy_palette_4_rgba));
    memcpy(boy_palette2.bo.base_virt, boy_palette_4_rgba, sizeof(boy_palette_4_rgba));
    boy_indices2.bo = nema_buffer_create(sizeof(boy_indices_4_gray));
    memcpy(boy_indices2.bo.base_virt, boy_indices_4_gray, sizeof(boy_indices_4_gray));

    boy_palette1.bo = nema_buffer_create(sizeof(boy_palette_2_rgba));
    memcpy(boy_palette1.bo.base_virt, boy_palette_2_rgba, sizeof(boy_palette_2_rgba));
    boy_indices1.bo = nema_buffer_create(sizeof(boy_indices_2_gray));
    memcpy(boy_indices1.bo.base_virt, boy_indices_2_gray, sizeof(boy_indices_2_gray));
#endif
}

void draw_palette_texture()
{
    //Initialize NemaGFX
    if ( nema_init() != 0 )
    {
        am_util_stdio_printf("GPU init failed!\n");;
    }

    // load objects
    load_objects();

    am_devices_display_init(RESX, RESY, COLOR_FORMAT_RGB888, true);

    //Create Command List
    nema_cmdlist_t cl  = nema_cl_create();
    //Bind Command List
    nema_cl_bind(&cl);

    float rot = 0.f;

    while(1)
    {
        nema_cl_rewind(&cl);

        //Bind Framebuffer
        nema_bind_dst_tex(fb.bo.base_phys, fb.w, fb.h, fb.format, fb.stride);
        //Set Clipping Rectangle
        nema_set_clip(0, 0, RESX, RESY);
        nema_clear(0x80000080);

        // Set Blending Mode
        nema_set_blend_fill(NEMA_BL_SRC);
        // Fill Rectangle with Black (Clear)
        nema_fill_rect(0, 0, RESX, RESY, 0);

        //setup matmult and ROB blender (if present) correctly
        nema_set_blend_blit(NEMA_BL_SIMPLE | NEMA_BLOP_LUT);

#ifdef CONCAT_PALETTE_INDICES
        nema_bind_lut_tex(  boy_indexed_8bits.bo.base_phys + 256*4, TEXX, TEXY, NEMA_L8, -1, 0,
                            boy_indexed_8bits.bo.base_phys, NEMA_RGBA8888);
        nema_blit_rotate_pivot(RESX/4, RESY/4, TEXX/2, TEXY/2, rot);

        nema_bind_lut_tex(  boy_indexed_4bits.bo.base_phys + 16*4, TEXX, TEXY, NEMA_L4, -1, 0,
                            boy_indexed_4bits.bo.base_phys, NEMA_RGBA8888);
        nema_blit_rotate_pivot(3*RESX/4, RESY/4, TEXX/2, TEXY/2, rot);

        nema_bind_lut_tex(  boy_indexed_2bits.bo.base_phys + 4*4, TEXX, TEXY, NEMA_L2, -1, 0,
                            boy_indexed_2bits.bo.base_phys, NEMA_RGBA8888);
        nema_blit_rotate_pivot(RESX/4, 3*RESY/4, TEXX/2, TEXY/2, rot);

        nema_bind_lut_tex(  boy_indexed_1bits.bo.base_phys + 2*4, TEXX, TEXY, NEMA_L1, -1, 0,
                            boy_indexed_1bits.bo.base_phys, NEMA_RGBA8888);
        nema_blit_rotate_pivot(3*RESX/4, 3*RESY/4, TEXX/2, TEXY/2, rot);
#else
        nema_bind_lut_tex(  boy_indices8.bo.base_phys, TEXX, TEXY, NEMA_L8, -1, 0,
                            boy_palette8.bo.base_phys, NEMA_RGBA8888);
        nema_blit_rotate_pivot(RESX/4, RESY/4, TEXX/2, TEXY/2, rot);

        nema_bind_lut_tex(  boy_indices4.bo.base_phys, TEXX, TEXY, NEMA_L4, -1, 0,
                            boy_palette4.bo.base_phys, NEMA_RGBA8888);
        nema_blit_rotate_pivot(3*RESX/4, RESY/4, TEXX/2, TEXY/2, rot);

        nema_bind_lut_tex(  boy_indices2.bo.base_phys, TEXX, TEXY, NEMA_L2, -1, 0,
                            boy_palette2.bo.base_phys, NEMA_RGBA8888);
        nema_blit_rotate_pivot(RESX/4, 3*RESY/4, TEXX/2, TEXY/2, rot);

        nema_bind_lut_tex(  boy_indices1.bo.base_phys, TEXX, TEXY, NEMA_L1, -1, 0,
                            boy_palette1.bo.base_phys, NEMA_RGBA8888);
        nema_blit_rotate_pivot(3*RESX/4, 3*RESY/4, TEXX/2, TEXY/2, rot);
#endif

        //Submit Command List
        nema_cl_submit(&cl);
        //Wait for submitted Command List to finish
        nema_cl_wait(&cl);

        // Transfer frame
        am_devices_display_transfer_frame(fb.w, fb.h,
                                  fb.bo.base_phys,
                                  NULL, NULL);

        //Wait done.
        am_devices_display_wait_transfer_done();

        rot += 0.1;
        if(rot >= 360.f)
        {
            rot = 0.f;
        }
    }
}

//*****************************************************************************
//
// End Doxygen group.
//! @}
//
//*****************************************************************************

