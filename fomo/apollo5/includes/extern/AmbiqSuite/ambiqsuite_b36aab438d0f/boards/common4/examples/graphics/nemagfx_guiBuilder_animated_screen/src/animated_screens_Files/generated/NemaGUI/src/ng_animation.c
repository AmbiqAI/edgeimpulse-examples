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

#include "nema_easing.h"
#include "ng_animation.h"
#include "ng_globals.h"
#include "ng_draw.h"
#include "ng_draw_prim.h"
#include "ng_display.h"
#include "ng_tuples.h"

static ng_event_base_t *cur_animation = NULL;
static void                 *cur_data = NULL;

static inline void set_blending_mode(uint8_t opacity) {
    if ( opacity == 0xff ) {
        nema_set_blend_blit(NEMA_BL_SIMPLE);
    } else {
        nema_set_blend_blit(NEMA_BL_SIMPLE|NEMA_BLOP_MODULATE_A);
        nema_set_const_color((opacity << 24));
    }
}

void ng_animation_callback(ng_event_base_t *event, void *data) {
    ng_animation_data_t *anim_data = NG_ANIMATION_DATA(data);
    tree_node_t  *animated_node    = anim_data->node;

    if ( ng_event_check_status_flag(event, EV_STATUS_FIRST_RUN) == true ) {
        //if no back-buffer is available, go to final state (instant show/hide)
        if ( ng_animation_init(anim_data) == false ) {
            event->stop(event, true);
        }
    } else if ( ng_event_check_status_flag(event, EV_STATUS_LAST_RUN) == true ) {

        if (anim_data->action == NG_SHOW ) {
            ng_gitem_unset_flag(animated_node->this_, GITEMF_HIDDEN);
            ng_gitem_unset_flag(animated_node->this_, GITEMF_ANIMATED);
        } else {
            // ng_gitem_set_flag(animated_node->this_, GITEMF_HIDDEN);
            ng_gitem_unset_flag(animated_node->this_, GITEMF_ANIMATED);
        }
        //free the occupied backbuffer
        ng_back_buffer_unlock(anim_data->back_buffer_index);

        cur_animation = NULL;
        cur_data      = NULL;
    } else {
        //Set current animation for this execution
        cur_animation = event;
        cur_data      = data;
    }
}

//draw the data->node and its children inside a free backbuffer
bool ng_animation_init(ng_animation_data_t *data) {
    bool ret = false;

    tree_node_t *node = data->node;

    if ( (node == NULL) || ( node->this_->flags & GITEMF_ANIMATED )) {
        return ret;
    }

    //Search for a free back buffer
    for ( int i = 0; i < NG_BACKBUFFER_COUNT; ++i ) {
        if ( ng_back_buffer_is_locked(i) == true ) {
            continue;
        } else {
            ng_back_buffer_lock(i);
            ng_gitem_set_flag(node->this_, GITEMF_HIDDEN);
            ng_gitem_set_flag(node->this_, GITEMF_ANIMATED);
            data->back_buffer_index = i;

            int w = node->this_->w;
            int h = node->this_->h;

            //Round resolution to multiple of 4
            if ( (NG_BACKBUFFER[i].format == NEMA_TSC4) || (NG_BACKBUFFER[i].format == NEMA_TSC6) ) {
                if ( (w % 4) != 0 ) {
                    w = round_up(node->this_->w, 4);
                }

                if ( (h % 4) != 0 ) {
                    h = round_up(node->this_->h, 4);
                }
            }

            //Draw git and its children inside NG_BACKBUFFERS
            nema_cl_rewind(&cl);
            nema_cl_bind(&cl);

            nema_bind_dst_tex(NG_BACKBUFFER[i].bo.base_phys, NG_BACKBUFFER[i].w, NG_BACKBUFFER[i].h, NG_BACKBUFFER[i].format, NG_BACKBUFFER[i].stride);
            // use x,y offets to draw the gitem in the beggining of the backbuffer
            int x_offset = -node->this_->x;
            int y_offset = -node->this_->y;
            ng_draw_to_buffer(node, x_offset, y_offset, 0, 0, w, h);

            nema_cl_submit(&cl);
            nema_cl_wait(&cl);
            return true;
        }
    }

    return ret;
}

void ng_animation_draw(tree_node_t *node, int x_min, int y_min, int x_max, int y_max){
    if ( (cur_animation != NULL) && (cur_data != NULL ) ) {
        ng_animation_data_t  *data = NG_ANIMATION_DATA(cur_data);
        int index = data->back_buffer_index;

        nema_set_clip(x_min, y_min, x_max-x_min, y_max-y_min);
        nema_bind_src_tex(NG_BACKBUFFER[index].bo.base_phys, node->this_->w, node->this_->h,
                          NG_BACKBUFFER[index].format, NG_BACKBUFFER[index].stride,
                          NG_BACKBUFFER[index].sampling_mode);

        cur_animation->action->act_animptr(data);
    }
}

void ng_animation_fade(ng_animation_data_t *data){
    ng_transition_t *animation = NG_TRANSITION(cur_animation);
    float progress = animation->progress;
    tree_node_t *node = data->node;

    uint8_t opacity_final = (node->this_->color & 0xff000000) >> 24;
    uint8_t opacity;

    if ( data->action == NG_SHOW ) {
        opacity = (uint8_t)(nema_ez(0,  (float)(opacity_final), 1.f, progress, data->ez_func));
    } else {
        progress = 1.f - animation->progress;
        opacity = (int)(nema_ez( (float)(opacity_final), 0, 1.f, progress, data->ez_func));
    }

    set_blending_mode(opacity);

    nema_blit_rect_fit(node->this_->x, node->this_->y, node->this_->w, node->this_->h);
}

void ng_animation_fly(ng_animation_data_t *data){
    ng_transition_t  *animation = NG_TRANSITION(cur_animation);
    tree_node_t           *node = data->node;
    ng_point_t    *origin_point = (ng_point_t*)(data->ext_data); //exteded data must be ng_point_t
    float              progress = animation->progress;

    int x, y, w, h;
    if ( data->action == NG_SHOW ) {
        w = nema_ez(0              , node->this_->w, 1.f, progress, data->ez_func);
        h = nema_ez(0              , node->this_->h, 1.f, progress, data->ez_func);
        x = nema_ez(origin_point->x, node->this_->x, 1.f, progress, data->ez_func);
        y = nema_ez(origin_point->y, node->this_->y, 1.f, progress, data->ez_func);
    } else {
        w = nema_ez(node->this_->w, 0              , 1.f , progress, data->ez_func);
        h = nema_ez(node->this_->h, 0              , 1.f , progress, data->ez_func);
        x = nema_ez(node->this_->x, origin_point->x, 1.f , progress, data->ez_func);
        y = nema_ez(node->this_->y, origin_point->y, 1.f , progress, data->ez_func);
    }

    uint8_t opacity = (node->this_->color & 0xff000000) >> 24;
    set_blending_mode(opacity);

    nema_blit_rect_fit(x, y, w, h);
}

void ng_animation_fade_zoom(ng_animation_data_t *data){
    ng_transition_t *animation = NG_TRANSITION(cur_animation);
    float progress = animation->progress;
    tree_node_t *node = data->node;
    gitem_base_t *git = node->this_;

    uint8_t opacity_final = (node->this_->color & 0xff000000) >> 24;
    uint8_t opacity;
    int x,y,w,h;

    if ( data->action == NG_SHOW ) {
        opacity = (uint8_t)(nema_ez(0,  (float)(opacity_final), 1.f, progress, data->ez_func));
        x = nema_ez(git->x + 0.5*git->w, git->x, 1.f, progress, data->ez_func);
        y = nema_ez(git->y + 0.5*git->h, git->y, 1.f, progress, data->ez_func);
        w = nema_ez(0, git->w, 1.f, progress, data->ez_func);
        h = nema_ez(0, git->h, 1.f, progress, data->ez_func);

    } else {
        progress = 1.f - animation->progress;
        opacity = (int)(nema_ez( (float)(opacity_final), 0.f, 1.f, progress, data->ez_func));
        x = nema_ez(git->x, git->x + 0.5*git->w, 1.f, progress, data->ez_func);
        y = nema_ez(git->y, git->y + 0.5*git->h, 1.f, progress, data->ez_func);
        w = nema_ez(git->w, 0, 1.f, progress, data->ez_func);
        h = nema_ez(git->h, 0, 1.f, progress, data->ez_func);
    }

    set_blending_mode(opacity);

     nema_blit_rect_fit(x, y, w, h);
}

void ng_animation_cube_face(ng_animation_data_t *data){
    ng_transition_t *animation = NG_TRANSITION(cur_animation);
    float             progress = animation->progress;
    tree_node_t          *node = data->node;
    gitem_base_t          *git = node->this_;

    int    *direction_ptr = (int*)(data->ext_data);
    int         direction = *direction_ptr;

    float box_size_2 = 0.5f;
    float FoV        = 1.1301023542f;
                   //x     y    z
    float v[18] = {-box_size_2, -box_size_2,-box_size_2,   //0
                    box_size_2, -box_size_2,-box_size_2,   //1
                    box_size_2,  box_size_2,-box_size_2,   //2
                   -box_size_2,  box_size_2,-box_size_2,   //3
                    box_size_2, -box_size_2, box_size_2,   //4
                    box_size_2,  box_size_2, box_size_2};  //5

    nema_matrix4x4_t mvp;
    nema_mat4x4_load_perspective(mvp, FoV, 1, 1.f, 100.f);
    mvp[0][0] = mvp[1][1] = 2.f;

    nema_matrix4x4_t proj;
    nema_mat4x4_load_identity(proj);
    float angle = 0.f;

    if ( data->action == NG_SHOW ) {
        if ( direction == NG_DIRECTION_LEFT ) {
            angle = 90.f*(progress -1.f);
            nema_mat4x4_rotate_Y(proj, angle);
        }else if ( direction == NG_DIRECTION_RIGHT ) {
            angle = 90.f*(1.f-progress);
            nema_mat4x4_rotate_Y(proj, angle);
        }else if ( direction == NG_DIRECTION_TOP ) {
            angle = 90.f*(1.f-progress);
            nema_mat4x4_rotate_X(proj, angle);
        }else if ( direction == NG_DIRECTION_BOTTOM ) {
            angle = 90.f*(progress -1.f);
            nema_mat4x4_rotate_X(proj, angle);
        }

    } else {
        if ( direction == NG_DIRECTION_LEFT ) {
            angle = 90.f*progress;
            nema_mat4x4_rotate_Y(proj, angle);
        } else if ( direction == NG_DIRECTION_RIGHT ) {
            angle = -90.f*progress;
            nema_mat4x4_rotate_Y(proj, angle);
        } else if ( direction == NG_DIRECTION_TOP ) {
            angle = -90.f*progress;
            nema_mat4x4_rotate_X(proj, angle);
        } else if ( direction == NG_DIRECTION_BOTTOM ) {
            angle = 90.f*progress;
            nema_mat4x4_rotate_X(proj, angle);
        }
    }

    if ( (angle > 70.f) || (angle < -70.f) ) {
        return;
    }

    nema_mat4x4_translate(proj, 0, 0, 1.5);
    nema_mat4x4_mul(mvp, mvp, proj);

    float w = 1.f;
    int i;
    for(i = 0; i < 18; i+=3) {
        nema_mat4x4_obj_to_win_coords(mvp, (float)git->x, (float)git->y, (float)git->w, (float)git->h,
                                      1.f, 100.f,
                                      &v[i  ], &v[i+1], &v[i+2], &w);
    }

    v[ 0] = (int)v[ 0]; v[ 1] = (int)v[ 1];
    v[ 3] = (int)v[ 3]; v[ 4] = (int)v[ 4];
    v[ 6] = (int)v[ 6]; v[ 7] = (int)v[ 7];
    v[ 9] = (int)v[ 9]; v[10] = (int)v[10];

    uint8_t opacity = (git->color & 0xff000000) >> 24;
    set_blending_mode(opacity);

    nema_blit_quad_fit(v[ 0], v[ 1],
                       v[ 3], v[ 4],
                       v[ 6], v[ 7],
                       v[ 9], v[10]);
}

void ng_animation_flip(ng_animation_data_t *data) {
    ng_transition_t *animation = NG_TRANSITION(cur_animation);
    float             progress = animation->progress;
    tree_node_t          *node = data->node;
    gitem_base_t          *git = node->this_;

    float progr   = progress*0.5f;
    float hor_off = git->w*progr;
    float x0, x1, x2, x3, y0, y1, y2, y3;

     if ( data->action == NG_SHOW ) {
        x0 = git->x + 0.5*git->w - hor_off;
        x1 = git->x + 0.5*git->w + hor_off;
        x2 = x1;
        x3 = x0;

        y0 = git->y;
        y1 = y0;
        y2 = git->y + git->h;
        y3 = y2;
     } else {
        x0 = git->x + hor_off;
        x1 = git->x + git->w - hor_off;
        x2 = x1;
        x3 = x0;

        y0 = git->y;
        y1 = y0;
        y2 = git->y + git->h;
        y3 = y2;
     }

    uint8_t opacity = (git->color & 0xff000000) >> 24;
    set_blending_mode(opacity);

    nema_blit_quad_fit(x0, y0,
                       x1, y1,
                       x2, y2,
                       x3, y3);
}
