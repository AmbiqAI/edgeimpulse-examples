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

#include "nema_graphics.h"
#include "nema_interpolators.h"
#include "nema_raster.h"

#include "ng_draw.h"
#include "ng_globals.h"
#include "ng_animation.h"
#include "ng_typedefs.h"
#include "ng_gitem.h"

void ng_draw_tree_node(tree_node_t *node, int x_off, int y_off, int x_min, int y_min, int x_max, int y_max)
{
    if(node->this_->draw) {
        nema_set_clip(x_min, y_min, x_max-x_min, y_max-y_min);
        node->this_->draw(node->this_, x_off, y_off);
    }
}

//draws tree
void ng_draw_tree(tree_node_t *node, int x_off, int y_off, int x_min, int y_min, int x_max, int y_max)
{
    while (node && node->this_) {
        // a) Simple draw operations
        if ( (node->this_->flags & GITEMF_HIDDEN) == 0) {
            //draw current gitem
            ng_draw_tree_node(node, x_off, y_off, x_min, y_min, x_max, y_max);

            //draw children if this is a container of some kind
            if (node->first_child) {
                int cur_x_min = x_off     + node->this_->x;
                int cur_y_min = y_off     + node->this_->y;
                int cur_x_max = cur_x_min + node->this_->w;
                int cur_y_max = cur_y_min + node->this_->h;

                if (x_min > cur_x_min) cur_x_min = x_min;
                if (y_min > cur_y_min) cur_y_min = y_min;
                if (x_max < cur_x_max) cur_x_max = x_max;
                if (y_max < cur_y_max) cur_y_max = y_max;

                ng_draw_tree(node->first_child, x_off+node->this_->x, y_off+node->this_->y,
                             cur_x_min, cur_y_min,
                             cur_x_max, cur_y_max);
            }

        // b) Draw animated items (animated items are hidden and replaced by a back_buffer)
        } else if ( node->this_->flags & GITEMF_ANIMATED) {
            ng_animation_draw(node, x_min, y_min, x_max, y_max);
        }

        //got to next item at the same level;
        node = node->next;
    }
}


//Draws a tree_node and its children inside the destination buffer at its native resolution
void ng_draw_to_buffer(tree_node_t *node, int x_off, int y_off, int x_min, int y_min, int x_max, int y_max){
    //Draw target tree node
    if ( node ) {
        nema_set_clip(0, 0, node->this_->w, node->this_->h);
        // nema_set_clip(x_min, y_min, x_max-x_min, y_max-y_min);
        //Clear
        nema_set_blend_fill(NEMA_BL_SRC);
        ng_fill_rect(0, 0, node->this_->w, node->this_->h, 0U, 1);

        //Draw current node
        if(node->this_->draw){
            node->this_->draw(node->this_, x_off, y_off);
        }

        node = node->first_child;
    }

    //Draw its children
    while (node && node->this_) {
        if ( (node->this_->flags & GITEMF_HIDDEN) == 0) {
            //draw current gitem
            if (node->this_->draw) {
                x_min = x_min > 0 ? x_min : 0;
                y_min = y_min > 0 ? y_min : 0;
                // node->this_->color = node->this_->color_const;
                if (x_min < x_max && y_min < y_max) {
                    nema_set_clip(x_min, y_min, x_max-x_min, y_max-y_min);
                    node->this_->draw(node->this_, x_off, y_off);
                }
            }

            //draw children if this is a container of some kind
            if (node->first_child) {

                int cur_x_min = x_off     + node->this_->x;
                int cur_y_min = y_off     + node->this_->y;
                int cur_x_max = cur_x_min + node->this_->w;
                int cur_y_max = cur_y_min + node->this_->h;

                if (x_min > cur_x_min) cur_x_min = x_min;
                if (y_min > cur_y_min) cur_y_min = y_min;
                if (x_max < cur_x_max) cur_x_max = x_max;
                if (y_max < cur_y_max) cur_y_max = y_max;

                ng_draw_to_buffer(node->first_child, x_off + node->this_->x, y_off + node->this_->y, cur_x_min, cur_y_min, cur_x_max, cur_y_max);
            }
        }

        //go to next item at the same level
        node = node->next;
    }
}
