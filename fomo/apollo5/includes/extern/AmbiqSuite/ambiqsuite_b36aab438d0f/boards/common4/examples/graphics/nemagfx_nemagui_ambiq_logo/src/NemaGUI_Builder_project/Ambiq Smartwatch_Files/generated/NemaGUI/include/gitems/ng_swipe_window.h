/*******************************************************************************
 * Copyright (c) 2021 Think Silicon S.A.
 *
   Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this header file and/or associated documentation files to use, copy,
 * modify, merge, publish, distribute, sublicense, and/or sell copies of the
 * Materials, and to permit persons to whom the Materials are furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Materials.
 *
 * MODIFICATIONS TO THIS FILE MAY MEAN IT NO LONGER ACCURATELY REFLECTS
 * NEMAGUI API.
 *
 *  The software is provided 'as is', without warranty of any kind, express or
 *  implied, including but not limited to the warranties of merchantability,
 *  fitness for a particular purpose and noninfringement. In no event shall
 *  Think Silicon S.A. be liable for any claim, damages or other liability, whether
 *  in an action of contract, tort or otherwise, arising from, out of or in
 *  connection with the software or the use or other dealings in the software.
 ******************************************************************************/

#ifndef NG_GITEM_SWIPE_WINDOW_H__
#define NG_GITEM_SWIPE_WINDOW_H__
#include "ng_gitem.h"
#include "ng_globals.h"
#include "ng_event_transition.h"
#include "ng_tuples.h"
#include "ng_tree.h"

/** Swipe window gestures data struct */
extern gitem_gestures_t gestures_swipe_window;

/** Swipe widnow widget data struct */
typedef struct _gitem_swipe_window_t{
    BASE_STRUCT;                         /**< Inherited attributes from gitem_base_t data struct*/
    tree_node_t *indicators_parent;      /**< Pointer to the tree node placeholder of the indicators (if applicable)*/
    uint16_t cur_page_index;             /**< Curretn page index*/
    uint16_t page_count;                 /**< Total page count*/
    uint16_t spacing;                    /**< Spacing between pages*/
    uint8_t layout;                      /**< Layout (horizontal or vertical)*/ 
    ng_transition_t *animation;          /**< Pointer to a transition event (used for animating the swipe window)*/
    ng_git_int_int_ez_t animation_data;  /**< Animation data (start point, end point and easing function)*/
} gitem_swipe_window_t;


#define NG_SWIPE_WINDOW(object) SAFE_CAST((object), gitem_base_t *, gitem_swipe_window_t *) /**< Type caster from  base gitem_base_t struct to derived gitem_swipe_window_t struct*/


#endif //NG_GITEM_SWIPE_WINDOW_H__