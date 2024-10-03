// -----------------------------------------------------------------------------
// Copyright (c) 2021 Think Silicon S.A.
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

#include "am_bsp.h"
#include "nema_core.h"
#include "nema_event.h"
#include "nema_utils.h"

#include "ng_globals.h"
#define EXTERN
#include "ng_display.h"
#undef EXTERN
#include "ng_draw.h"
#include "ng_draw_prim.h"
#include "ng_tree.h"
#include "ng_screen_trans.h"
#include "nema_programHW.h"
#include "am_util.h"

#define CURRENT_SCREEN_NODE     NG_CUR_SCREEN_GROUP_NODES[NG_CUR_SCREEN_NODE_INDEX]
#define LIB_FRAME_BUFFERS       2

static int display_mode = DISPLAY_SCREEN;
static ng_event_base_t *screen_transition_event_ = NULL;
static int fb_locked_flags   = 0;
static img_obj_t *cur_fb_img = NULL;
static uintptr_t cur_fb_base_phys;
static bool clear_display = false;

nema_transition_t global_screen_trans_effect = 0;


static int32_t i32LastFB = -1;
static int32_t i32CurFB = 0;

static int32_t i32LastBackFB = -1;
static int32_t i32CurBackFB = 0;

#if !defined(AM_PART_APOLLO4L)
extern nemadc_layer_t layer[];
#endif

extern img_obj_t fb[];

static uint8_t g_flag_transmode = 1;

extern volatile uint8_t g_ui8_fbs;

static void
swap_buffers(void)
{
    i32LastFB = i32CurFB;
    i32CurFB = (i32CurFB + 1) % LIB_FRAME_BUFFERS;
    cur_fb_base_phys = fb[i32CurFB].bo.base_phys;
}

static void
swap_back_buffers(void)
{
    i32LastBackFB = i32CurBackFB;
    i32CurBackFB = (i32CurBackFB + 1) % LIB_FRAME_BUFFERS;
    cur_fb_base_phys = fb[i32CurBackFB].bo.base_phys;
}

static inline img_obj_t* ng_display_get_cur_fb_img(void) {
#if !defined(AM_PART_APOLLO4L)
    for (int l = 0; l < NG_FRAMEBUFFER_COUNT; ++l) {
        if ( NG_DC_LAYER[l].baseaddr_phys == cur_fb_base_phys ) {
            return &NG_FRAMEBUFFER[l];
        }
    }
#else
            return &NG_FRAMEBUFFER[0];
#endif

    return 0;
}

static inline void
ng_display_bind_fb_img(img_obj_t *fb_img) {
    nema_bind_dst_tex(fb_img->bo.base_phys, fb_img->w, fb_img->h, fb_img->format, -1);
}

void
ng_display_screen_node_to_fb(img_obj_t *fb_img, tree_node_t *screen_node, int x_off, int y_off) {
    PROC_ENTRY;

    //draw screen backtround
    nema_cl_rewind(&cl_screen);
    nema_cl_bind(&cl_screen);
    ng_display_bind_fb_img(fb_img);
    ng_draw_tree_node(screen_node, x_off, y_off, 0, 0, NG_RESX, NG_RESY);
    nema_cl_submit_no_irq(&cl_screen);
    nema_cl_branch(&cl_screen);

    //draw screen gitems
    nema_cl_rewind(&cl);
    nema_cl_bind(&cl);
    ng_display_bind_fb_img(fb_img);
    nema_enable_tiling(1);
    ng_draw_tree(screen_node->first_child, x_off, y_off, 0, 0, NG_RESX, NG_RESY);
    nema_cl_submit(&cl);
    nema_cl_branch(&cl_screen);

    static bool flag_output = false;
    if (flag_output == false)
    {
        flag_output = true;
        am_util_stdio_printf("g_ui8_fbs = %d\n", g_ui8_fbs);
    }

    if (g_ui8_fbs > 1)
    {
        if (i32LastFB >= 0)
        {
        
#ifndef AM_PART_APOLLO4L
            nemadc_set_layer(0, &layer[i32LastFB]);
#endif
            am_devices_display_transfer_frame(fb[i32LastFB].w,
                                              fb[i32LastFB].h,
                                              fb[i32LastFB].bo.base_phys,
                                              NULL, NULL);
            am_devices_display_wait_transfer_done();
        }
        else
        {
            // first show up
            nema_cl_wait(&cl);
            static uint8_t flag_show_only_once = 0;
            if (flag_show_only_once == 0)
            {
                flag_show_only_once = 1;
#ifndef AM_PART_APOLLO4L
                nemadc_set_layer(0, &layer[0]);
#endif
                am_devices_display_transfer_frame(fb[0].w,
                                                  fb[0].h,
                                                  fb[0].bo.base_phys,
                                                  NULL, NULL);
                am_devices_display_wait_transfer_done();
            }
        }
        nema_cl_wait(&cl);
        swap_buffers();

        i32LastBackFB = -1;
    }
    else
    {
        nema_cl_wait(&cl);
    }
}

static void
ng_display_popup(void) {
    ng_display_screen_node_to_fb(cur_fb_img, CURRENT_SCREEN_NODE, 0, 0);

    nema_cl_rewind(&cl);
    nema_cl_bind(&cl);
    //Bind Framebuffer
    ng_display_bind_fb_img(cur_fb_img);

    nema_set_clip (0, 0, NG_RESX, NG_RESY);
    ng_fill_rect(0, 0, NG_RESX, NG_RESY, 0xd0000000U, 0);

    ng_draw_tree(popup_node, popup_off_x, popup_off_y, 0, 0, NG_RESX, NG_RESY);

    nema_cl_submit(&cl);

#if (FRAME_BUFFERS > 1)
#else
        nema_cl_wait(&cl);
#endif

}

void
ng_display_screen_clear(int wait) {
    nema_cl_rewind(&cl_screen);
    nema_cl_bind(&cl_screen);
        //Bind Framebuffer
        ng_display_bind_fb_img(cur_fb_img);

        nema_set_clip (0, 0, NG_RESX, NG_RESY);
        ng_fill_rect(0, 0, NG_RESX, NG_RESY, 0xff000000U, 0);

    if (wait) {
        nema_cl_submit(&cl_screen);
        nema_cl_wait(&cl_screen);
    } else {
        nema_cl_submit_no_irq(&cl_screen);
    }
}

extern bool goto_right_screen;

static void
ng_display_screen_transition(void) {
    // PROC_ENTRY;

    // use animation buffers
    if ( NG_BACKBUFFER_COUNT >= 2 ) {
        nema_cl_rewind(&cl);
        nema_cl_bind(&cl);
        //Bind Framebuffer
        ng_display_bind_fb_img(cur_fb_img);

        nema_set_clip (0, 0, NG_RESX, NG_RESY);

        float progress = goto_right_screen ? NG_TRANSITION(screen_transition_event_)->progress : 1.f - NG_TRANSITION(screen_transition_event_)->progress;

        nema_transition(global_screen_trans_effect, NEMA_TEX1, NEMA_TEX2, NEMA_BL_SRC, nema_ez_linear(progress), NG_FRAMEBUFFER[0].w, NG_FRAMEBUFFER[0].h);

        nema_cl_submit(&cl);

        if (g_ui8_fbs > 1)
        {
            if (i32LastBackFB >= 0)
            {
#ifndef AM_PART_APOLLO4L
                nemadc_set_layer(0, &layer[i32LastBackFB]);
#endif
                am_devices_display_transfer_frame(fb[0].w,
                                                  fb[0].h,
                                                  fb[0].bo.base_phys,
                                                  NULL, NULL);
                am_devices_display_wait_transfer_done();
                nema_cl_wait(&cl);
            }
            else
            {
                // first show up
            }

            swap_back_buffers();

            i32LastFB = -1;
        }
    }
    // no animation buffers available
    // do linear transition
    else {
        float progress = goto_right_screen ? NG_TRANSITION(screen_transition_event_)->progress : 1.f - NG_TRANSITION(screen_transition_event_)->progress;

        bool is_vert = NG_LAYOUT_PER_GROUP[NG_CUR_SCREEN_GROUP_INDEX] == 1;

        int res = is_vert ? cur_fb_img->h : cur_fb_img->w;
        int off = progress*res;

        if ( !goto_right_screen ) {
            ng_display_screen_node_to_fb(cur_fb_img, NG_CUR_SCREEN_GROUP_NODES[NG_CUR_SCREEN_NODE_INDEX+1],
                !is_vert ? off : 0,
                 is_vert ? off : 0);
            ng_display_screen_node_to_fb(cur_fb_img, CURRENT_SCREEN_NODE,
                !is_vert ? off-NG_RESX : 0,
                 is_vert ? off-NG_RESY : 0);
        }
        else {
            ng_display_screen_node_to_fb(cur_fb_img, CURRENT_SCREEN_NODE,
                !is_vert ? off : 0,
                 is_vert ? off : 0);
            ng_display_screen_node_to_fb(cur_fb_img, NG_CUR_SCREEN_GROUP_NODES[NG_CUR_SCREEN_NODE_INDEX-1],
                !is_vert ? off-NG_RESX : 0,
                 is_vert ? off-NG_RESY : 0);
        }
    }
}

// main display function
void ng_display(void) {
    // am_util_stdio_printf("g_ui8_fbs = %d\n", g_ui8_fbs);

    if ( clear_display == true ) {
        ng_display_screen_clear(0);
    }

    if ( (display_mode == DISPLAY_SCREEN_TRANSITION) && (screen_transition_event_ != NULL)) {
        ng_display_screen_transition();
        if (g_ui8_fbs > 1)
            g_flag_transmode = 0;
        //am_util_stdio_printf("ng_display_screen_transition\n");
    } else if (display_mode == DISPLAY_POPUP) {
        ng_display_popup();
    } else {
        ng_display_screen_node_to_fb(cur_fb_img, CURRENT_SCREEN_NODE, 0, 0);
    }

    if (g_ui8_fbs > 1)
    {
        clear_display    = false;
        cur_fb_img       = ng_display_get_cur_fb_img();
    }
    else
    {
        cur_fb_img       = ng_display_get_cur_fb_img();
#ifndef AM_PART_APOLLO4L
        nemadc_set_layer(0, &layer[0]);
#endif
        am_devices_display_transfer_frame(fb[0].w,
                                          fb[0].h,
                                          fb[0].bo.base_phys,
                                          NULL, NULL);

        am_devices_display_wait_transfer_done();

        clear_display    = false;
    }

    #ifdef CALCULATE_FPS
    nema_calculate_fps();
    #endif
}

void ng_display_bind_transition_buffers(void) {
    if ( NG_BACKBUFFER_COUNT >=2 ) {
        nema_cl_rewind(&cl);
        nema_cl_bind(&cl);
        nema_bind_tex(1, NG_BACKBUFFER[0].bo.base_phys, NG_BACKBUFFER[0].w, NG_BACKBUFFER[0].h, NG_BACKBUFFER[0].format, -1, NEMA_FILTER_BL);
        nema_bind_tex(2, NG_BACKBUFFER[1].bo.base_phys, NG_BACKBUFFER[1].w, NG_BACKBUFFER[1].h, NG_BACKBUFFER[1].format, -1, NEMA_FILTER_BL);
        nema_cl_submit(&cl);
        nema_cl_wait(&cl);
    }
}

void
ng_display_init(){
#if !defined(AM_PART_APOLLO4L)
    uintptr_t l0 = NG_DC_LAYER[0].baseaddr_phys;
    uintptr_t l1 = 2 > NG_FRAMEBUFFER_COUNT ? 0U : NG_DC_LAYER[1].baseaddr_phys;
    uintptr_t l2 = 3 > NG_FRAMEBUFFER_COUNT ? 0U : NG_DC_LAYER[2].baseaddr_phys;
#else
    uintptr_t l0 = fb[0].bo.base_phys;
    uintptr_t l1 = fb[1].bo.base_phys;
    uintptr_t l2 = fb[2].bo.base_phys;
#endif

    cur_fb_base_phys = nema_init_triple_fb(0, l0,
                                              l1,
                                              l2);

    cur_fb_img = ng_display_get_cur_fb_img();

    cl  = nema_cl_create();
    cl_screen = nema_cl_create();

    // //Bind Command List
    nema_cl_bind(&cl);

    // //Set Clipping Rectangle
    nema_set_clip(0, 0, NG_RESX, NG_RESY);

    nema_cl_submit(&cl);
    nema_wait_irq();

#ifndef AM_PART_APOLLO4L
    nemadc_cursor_enable(1);
#endif

    ng_display();
}

void ng_display_set_event(ng_event_base_t *event) {
    screen_transition_event_ = event;
}

void ng_display_set_mode(int mode) {
    display_mode = mode;
}

int ng_display_get_mode() {
    return display_mode;
}

void ng_display_set_clear(bool clear) {
    clear_display = clear;
}

bool ng_back_buffer_is_locked(int index) {
    bool val = true;
    int mask = (1 << index);

    if ( (fb_locked_flags & mask) == 0 ) {
        val = false;
    }

    return val;
}

void ng_back_buffer_lock(int index) {
    int bit_to_set = (1 << index);
    fb_locked_flags |= bit_to_set;
}

void ng_back_buffer_unlock(int index) {
    int bit_to_set = (1 << index);
    fb_locked_flags &= ~bit_to_set;
}

void ng_display_set_popup(tree_node_t *node) {
    display_mode = DISPLAY_POPUP;
    ng_tree_set_current_popup(node);
}
