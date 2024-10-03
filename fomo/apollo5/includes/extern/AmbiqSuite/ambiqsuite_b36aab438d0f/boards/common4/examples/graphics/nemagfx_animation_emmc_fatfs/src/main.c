//*****************************************************************************
//
//! @file main.c
//!
//! @brief NemaGFX example.
//! main.c demonstrate two layers texture DC blend that one texture(FB) in SSRAM
//! render with mask by GPU and another texture(animations) in PSRAM/TCM
//! transmitter by DC.the original textures saved in eMMC.user load mass textures
//! to PSRAM once or only load one frame to TCM depending on needed.DC used
//! two layers when only demostrate one watchface with animations background.
//! at the same time,can use dual framebuffer to improve the performance.
//! when scrolling watchface it used four layers(total 4 layers) couldn't use dual
//! framebuffer because of insufficient memory.
//!
//
//*****************************************************************************

//*****************************************************************************
//
// ${copyright}
//
// This is part of revision ${version} of the AmbiqSuite Development Package.
//
//*****************************************************************************

// -----------------------------------------------------------------------------
// Copyright (c) 2019 Think Silicon S.A.
// Think Silicon S.A. Confidential Proprietary
// -----------------------------------------------------------------------------
//     All Rights reserved - Unpublished -rights reserved under
//         the Copyright laws of the European Union
//
//  This file includes the Confidential information of Think Silicon S.A.
//  The receiver of this Confidential Information shall not disclose
//  it to any third party and shall protect its confidentiality by
//  using the same degree of care, but not less than a reasonable
//  degree of care, as the receiver uses to protect receiver's own
//  Confidential Information. The entire notice must be reproduced on all
//  authorised copies and copies may only be made to the extent permitted
//  by a licensing agreement from Think Silicon S.A..
//
//  The software is provided 'as is', without warranty of any kind, express or
//  implied, including but not limited to the warranties of merchantability,
//  fitness for a particular purpose and noninfringement. In no event shall
//  Think Silicon S.A. be liable for any claim, damages or other liability, whether
//  in an action of contract, tort or otherwise, arising from, out of or in
//  connection with the software or the use or other dealings in the software.
//
//
//                    Think Silicon S.A.
//                    http://www.think-silicon.com
//                    Patras Science Park
//                    Rion Achaias 26504
//                    Greece
// -----------------------------------------------------------------------------
#include "main.h"
#include "watch.h"
#include "am_bsp.h"
#include "nema_core.h"
#include "nema_utils.h"
#include "am_hal_global.h"

#include "ff.h"
//*****************************************************************************
//
// Macro definitions
//
//*****************************************************************************

#define SECONDARYINDEX                               ((ui8PrimaryIndex + 1) % FRAME_BUFFERS)
#define DCINDEX                                      ((ui8GPUIndex + 1) % FRAME_BUFFERS)
#define IMAGEINDEX                                   ((ui8ImageIndex + 1) % FRAME_BUFFERS)

#define WATCHLAYER_1        3
#define WATCHLAYER_0        2
#define ANIMATIONLAYER_1    1
#define ANIMATIONLAYER_0    0

static uint8_t ui8ImageIndex = 0, ui8GPUIndex = 0, ui8PrimaryIndex = 0;

static img_obj_t    sFrameBuffer[FRAME_BUFFERS] =  {{{0}, RESOLUTION_X, RESOLUTION_Y, RESOLUTION_X * 2, 0, NEMA_RGBA5551, NEMA_FILTER_PS},\
                                                    {{0}, RESOLUTION_X, RESOLUTION_Y, RESOLUTION_X * 2, 0, NEMA_RGBA5551, NEMA_FILTER_PS}};

static nemadc_layer_t sWatchLayer[FRAME_BUFFERS] = {{(void *)0, 0, RESOLUTION_X, RESOLUTION_Y, -1, 0, 0, RESOLUTION_X, RESOLUTION_Y, 0xA0, \
                                                    NEMADC_BL_SIMPLE, 0, NEMADC_RGBA5551, 0, 0, 0, 0, 0, 0, 0, 0},\
                                                    {(void *)0, 0, RESOLUTION_X, RESOLUTION_Y, -1, 0, 0, RESOLUTION_X, RESOLUTION_Y, 0xA0, \
                                                    NEMADC_BL_SIMPLE, 0, NEMADC_RGBA5551, 0, 0, 0, 0, 0, 0, 0, 0}};

static nemadc_layer_t sAnimationLayer[FRAME_BUFFERS] = {{(void *)0, 0, RESOLUTION_X, RESOLUTION_Y, -1, 0, 0, RESOLUTION_X, RESOLUTION_Y, 0xff,\
                                                        NEMADC_BL_SIMPLE, 0, NEMADC_TSC6, 0, 0, 0, 0, 0, 0, 0, 0},\
                                                        {(void *)0, 0, RESOLUTION_X, RESOLUTION_Y, -1, 0, 0, RESOLUTION_X, RESOLUTION_Y, 0xff, \
                                                        NEMADC_BL_SIMPLE, 0, NEMADC_TSC6, 0, 0, 0, 0, 0, 0, 0, 0}};

static nema_cmdlist_t sCL;

#ifdef LOAD_IMG_FROM_EMMC_DIRECTLY
extern uint16_t g_ui16Index[];
extern uint16_t g_ui16Count[];
extern uint8_t g_ui8TSC6img[][IMG_BUF_SIZE_TSC6];
extern uint8_t *g_loadOnefromEMMC(uint8_t order);
#else
extern animationImage_t g_sAImages[];

#endif
void
g_load_objects()
{
    g_load_watch_images();
    for ( uint8_t idx = 0; idx < FRAME_BUFFERS; idx++ )
    {
        sFrameBuffer[idx].bo = nema_buffer_create(sFrameBuffer[idx].stride * sFrameBuffer[idx].h);
    }
    sCL = nema_cl_create();
}
//*****************************************************************************
//
//! @brief draw watchface and animation images.
//!
//! @param isDualBuffer - set false when scrolling watchface and animation image.
//! @param img - struct of animationImage_t arrays index,value is 0 or 1.
//! @param offset - the scrolling effect's offset.
//! @param onestyle - the first type of watch hands.
//! @param twostyle - the second type of watch hands.
//!
//! when the application implement effects that only elaborate watchface with
//! animations(without scrolling watchface/animations) variable isDualBuffer
//! had better set true and at same time recommend set offset to zero,otherwise
//! set false with scrolling effect.
//!
//! @note recommend set isDualBuffer equivelent to true,when not scrolling watch.
//!
//! @return 0.
//
//*****************************************************************************
int32_t
g_draw_watch_with_animation(bool isDualBuffer, uint8_t img, int32_t offset,
                            uint8_t onestyle, uint8_t twostyle)
{
    float time = nema_get_time();
    ui8ImageIndex = img;
    nema_cmdlist_t *pCL = g_draw_watch_hands(time, onestyle);

    nema_cl_rewind(&sCL);
    nema_cl_bind(&sCL);
    //
    // GPU rasterize sFrameBuffer[ui8GPUIndex]
    //
    nema_bind_dst_tex(sFrameBuffer[ui8GPUIndex].bo.base_phys, sFrameBuffer[ui8GPUIndex].w, sFrameBuffer[ui8GPUIndex].h, sFrameBuffer[ui8GPUIndex].format, RESOLUTION_X*2);
    nema_cl_branch(pCL);
    nema_cl_submit(&sCL);
    if ( isDualBuffer )
    {
        //
        // concurrent operations: GPU rasterize sFrameBuffer[ui8GPUIndex],mcu operate sFrameBuffer[DCINDEX] related DC layer registers.
        //
        sWatchLayer[ui8PrimaryIndex].startx =  - offset;
        sWatchLayer[ui8PrimaryIndex].baseaddr_phys = sFrameBuffer[DCINDEX].bo.base_phys;
        sWatchLayer[ui8PrimaryIndex].baseaddr_virt = sFrameBuffer[DCINDEX].bo.base_virt;
        sAnimationLayer[ui8PrimaryIndex].startx =  - offset;
#ifdef LOAD_IMG_FROM_EMMC_DIRECTLY
        sAnimationLayer[ui8PrimaryIndex].baseaddr_virt = g_loadOnefromEMMC(ui8ImageIndex);
#else
        sAnimationLayer[ui8PrimaryIndex].format = g_sAImages[ui8ImageIndex].format;
        sAnimationLayer[ui8PrimaryIndex].baseaddr_virt = g_sAImages[ui8ImageIndex].img[g_sAImages[ui8ImageIndex].idx];
#endif
        sAnimationLayer[ui8PrimaryIndex].baseaddr_phys = (unsigned)(sAnimationLayer[ui8PrimaryIndex].baseaddr_virt);
        nemadc_set_layer(WATCHLAYER_0, &sWatchLayer[ui8PrimaryIndex]);
        nemadc_set_layer(ANIMATIONLAYER_0, &sAnimationLayer[ui8PrimaryIndex]);
        nemadc_layer_disable(WATCHLAYER_1);
        nemadc_layer_disable(ANIMATIONLAYER_1);

        am_devices_display_transfer_frame_start(NULL);
        am_devices_display_transfer_end_after_done();
#ifdef LOAD_IMG_FROM_EMMC_DIRECTLY
#else
        if ( ++g_sAImages[ui8ImageIndex].idx == g_sAImages[ui8ImageIndex].cnt )
        {
            g_sAImages[ui8ImageIndex].idx = 0;
        }
#endif
        //
        // wait GPU operate completion.
        //
        nema_cl_wait(&sCL);

        //
        // swap GPU and DC operated sFrameBuffers.
        //
        ui8GPUIndex = DCINDEX;
    }
    else
    {
        //
        // concurrent operations: GPU rasterize sFrameBuffer[ui8GPUIndex],mcu operate sFrameBuffer[DCINDEX] related DC layer registers.
        //
        sWatchLayer[SECONDARYINDEX].startx = RESOLUTION_X - offset;
        sWatchLayer[SECONDARYINDEX].baseaddr_phys = sFrameBuffer[DCINDEX].bo.base_phys;
        sWatchLayer[SECONDARYINDEX].baseaddr_virt = sFrameBuffer[DCINDEX].bo.base_virt;
        sAnimationLayer[SECONDARYINDEX].startx = RESOLUTION_X - offset;
#ifdef LOAD_IMG_FROM_EMMC_DIRECTLY
        sAnimationLayer[SECONDARYINDEX].baseaddr_virt = g_loadOnefromEMMC(IMAGEINDEX);
#else
        sAnimationLayer[SECONDARYINDEX].format = g_sAImages[IMAGEINDEX].format;
        sAnimationLayer[SECONDARYINDEX].baseaddr_virt = g_sAImages[IMAGEINDEX].img[g_sAImages[IMAGEINDEX].idx];
#endif
        sAnimationLayer[SECONDARYINDEX].baseaddr_phys = (unsigned)(sAnimationLayer[SECONDARYINDEX].baseaddr_virt);
        nemadc_set_layer(WATCHLAYER_1, &sWatchLayer[SECONDARYINDEX]);
        nemadc_set_layer(ANIMATIONLAYER_1, &sAnimationLayer[SECONDARYINDEX]);
        //
        // wait GPU operate completion.
        //
        nema_cl_wait(&sCL);
        //
        // GPU rasterize sFrameBuffer[DCINDEX]
        //
        pCL = g_draw_watch_hands(time, twostyle);
        nema_cl_rewind(&sCL);
        nema_cl_bind(&sCL);
        nema_bind_dst_tex(sFrameBuffer[DCINDEX].bo.base_phys, sFrameBuffer[DCINDEX].w, sFrameBuffer[DCINDEX].h, sFrameBuffer[DCINDEX].format, RESOLUTION_X*2);
        nema_cl_branch(pCL);
        nema_cl_submit(&sCL);
        //
        // concurrent operations: GPU rasterize sFrameBuffer[DCINDEX],mcu operate sFrameBuffer[ui8GPUIndex] related DC layer registers.
        //
        sWatchLayer[ui8PrimaryIndex].startx =  - offset;
        sWatchLayer[ui8PrimaryIndex].baseaddr_phys = sFrameBuffer[ui8GPUIndex].bo.base_phys;
        sWatchLayer[ui8PrimaryIndex].baseaddr_virt = sFrameBuffer[ui8GPUIndex].bo.base_virt;
        sAnimationLayer[ui8PrimaryIndex].startx =  - offset;
#ifdef LOAD_IMG_FROM_EMMC_DIRECTLY
        sAnimationLayer[ui8PrimaryIndex].baseaddr_virt = g_loadOnefromEMMC(ui8ImageIndex);
#else
        sAnimationLayer[ui8PrimaryIndex].format = g_sAImages[ui8ImageIndex].format;
        sAnimationLayer[ui8PrimaryIndex].baseaddr_virt = g_sAImages[ui8ImageIndex].img[g_sAImages[ui8ImageIndex].idx];
#endif
        sAnimationLayer[ui8PrimaryIndex].baseaddr_phys = (unsigned)(sAnimationLayer[ui8PrimaryIndex].baseaddr_virt);
        nemadc_set_layer(WATCHLAYER_0, &sWatchLayer[ui8PrimaryIndex]);
        nemadc_set_layer(ANIMATIONLAYER_0, &sAnimationLayer[ui8PrimaryIndex]);
#ifdef LOAD_IMG_FROM_EMMC_DIRECTLY
#else
        if ( ++g_sAImages[ui8ImageIndex].idx == g_sAImages[ui8ImageIndex].cnt )
        {
            g_sAImages[ui8ImageIndex].idx = 0;
        }

        if ( ++g_sAImages[IMAGEINDEX].idx == g_sAImages[IMAGEINDEX].cnt )
        {
            g_sAImages[IMAGEINDEX].idx = 0;
        }
#endif
        //
        // wait GPU operate completion.
        //
        nema_cl_wait(&sCL);

        am_devices_display_transfer_frame_start(NULL);
        am_devices_display_transfer_end_after_done();
    }
    nema_calculate_fps();
    return 0;
}


