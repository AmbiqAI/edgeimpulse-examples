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

#ifndef NG_TYPEDEFS_H__
#define NG_TYPEDEFS_H__

//GITEM
typedef struct _gitem_base_t gitem_base_t;

//TREE
typedef struct _tree_node_t tree_node_t;

//GESTURES
typedef struct _gitem_gestures_t gitem_gestures_t;

//TIMER
// typedef struct _ng_timer_t ng_timer_t;

//EVENTS
typedef struct _ng_event_base_t ng_event_base_t;

//Function pointers
typedef float (*easing_f)(float val);                                       /**< Typedef to easing function pointer */
typedef void (*draw_f)(struct _gitem_base_t* gitem, int x_off, int y_off);  /**< Typedef to draw function pointer (used by the gitem_base_t struct) */
typedef void (*callback_f)(struct _ng_event_base_t* event, void *data);     /**< Typedef to callback function pointer (used by the g_event_base_t struct) */

#endif
