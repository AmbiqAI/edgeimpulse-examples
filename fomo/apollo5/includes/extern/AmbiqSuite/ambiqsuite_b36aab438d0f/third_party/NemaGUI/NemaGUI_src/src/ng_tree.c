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

#include "ng_tree.h"
#include "ng_globals.h"
#include "ng_gestures.h"
#include "ng_event.h"
#include "ng_display.h"

static bool is_inside_window = false;
static tree_node_t  *clicked_window = NULL;

void ng_tree_set_current_screen(tree_node_t *node)
{
    for(int i = 0; i < NG_SCREEN_GROUPS_COUNT; ++i){
        NG_CUR_SCREEN_GROUP_NODES = NG_NODES_PER_GROUP[i];

        for(int j = 0; j < NG_SCREENS_PER_GROUP[i]; ++j){
            if ( NG_CUR_SCREEN_GROUP_NODES[j] == node ) {
                NG_CUR_SCREEN_GROUP_INDEX  = i;
                NG_CUR_SCREEN_NODE_INDEX   = j;
                
                if ( node->this_->flags & GITEMF_FILL_IMAGE ) {
                    ng_display_set_clear(true);
                }
                
                if ( node->this_->event ) {
                    ng_event_handle(node->this_->event, EV_TRIGGER_SCREEN_ENTERED);
                }
                return;
            }
        }
    }
}

void ng_tree_set_current_popup(tree_node_t *node)
{
    gitem_base_t *git = node->this_;

    if ( git != NULL ) {
        popup_node   = node;
        popup_off_x  = (NG_RESX-git->w)/2;
        popup_off_y  = (NG_RESY-git->h)/2;
    }
}

tree_node_t*
ng_tree_get_node_under_cursor(tree_node_t *node, uint32_t gesture, int x, int y, int x_off, int y_off, int *click_x, int *click_y) {
    if( (ng_gestures_is_inside_popup(x ,y) == true) && (node == NULL) ){
        node = popup_node;
        is_inside_window = false;
    }else if(node == NULL){
        node = (NG_CUR_SCREEN_GROUP_NODES[NG_CUR_SCREEN_NODE_INDEX]);
        is_inside_window = false;
    }

    tree_node_t *curr_node = NULL;
    
    while (node && node->this_) {
        if (x >= node->this_->x + x_off &&
            y >= node->this_->y + y_off &&
            x <  node->this_->x + x_off + node->this_->w &&
            y <  node->this_->y + y_off + node->this_->h) {

            if ( (node->this_->type == GITEM_WINDOW) || (node->this_->type == GITEM_SWIPE_WINDOW) ) {
                is_inside_window = true;
                clicked_window = node;
            }    

            //first explore children
            if ( ( (node->this_->flags & GITEMF_STOP_RECUR) == 0 ) && ( node->first_child ) ) {
                node->first_child->parent = node;
                curr_node = ng_tree_get_node_under_cursor(node->first_child, gesture, x, y, x_off+node->this_->x, y_off+node->this_->y, click_x, click_y);
            }

            //if no children is selected, check if current node supports gesture
            if(curr_node == NULL){
                if (node->this_->flags & gesture ) {
                    curr_node = node;
                    *click_x = x - (node->this_->x + x_off);
                    *click_y = y - (node->this_->y + y_off);
                }
            }
        }

        //go to next item at the same level;
        if (node->next){
            node->next->parent = node->parent;
        }

        node = node->next;
    }

    if ( (is_inside_window == true) && (curr_node) && (curr_node->this_->type == GITEM_MAIN_SCREEN) ) {
        curr_node = clicked_window;
    }

    return curr_node;
}

tree_node_t* ng_tree_get_current_screen(){
    return  (NG_CUR_SCREEN_GROUP_NODES[NG_CUR_SCREEN_NODE_INDEX]);
}
