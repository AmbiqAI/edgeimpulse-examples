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

#ifndef __NEMAGUI_WINDOW__
#define __NEMAGUI_WINDOW__
#include "ng_gitem.h"
#include "ng_gestures.h"

/** Window gestures data struct */
extern gitem_gestures_t gestures_window;

/** Window widget data struct */
typedef struct _gitem_window_t{
    BASE_STRUCT;
} gitem_window_t;

/** \brief Sets the source screen that the window displays.
 *
 * \param *window 	Pointer to the window's tree node
 * \param *source   Pointer to the source screen's tree node
 * \return void
 *
 */
void ng_window_set_source(tree_node_t *window, tree_node_t *source);

#define NG_WINDOW(object) SAFE_CAST((object), gitem_base_t *, gitem_window_t *) /**< Type caster from  base gitem_base_t struct to derived gitem_window_t struct*/

#endif //__NEMAGUI_WINDOW__
