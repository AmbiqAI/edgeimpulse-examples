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

#ifndef NG_GITEM_TOGGLE_BUTTON_H__
#define NG_GITEM_TOGGLE_BUTTON_H__
#include "ng_gitem.h"

/**
 * @file
 * @brief Toggle button widget interface
 *
 */

/** Toggle button gestures data struct */
extern gitem_gestures_t gestures_toggle_button;

/** Toggle button widget data struct */
typedef struct _gitem_toggle_button_t{
    BASE_STRUCT;            /**< Inherited attributes from gitem_base_t data struct*/
    uint16_t cur_state;     /**< Current state's index*/
    uint16_t max_state;     /**< State count*/
    img_obj_t**    images;  /**< Array of pointers to image assets, each image corresponds to each state*/
} gitem_toggle_button_t;

/** \brief Draw function
 *
 * \param *git 	Pointer to toggle button's base gitem (gitem_base_t data struct)
 * \param x_off Horizontal offset from its parent item
 * \param y_off Vertical offset from its parent item
 * \return void
 *
 */
DRAW_FUNC(ng_toggle_button_draw);

#define NG_TOGGLE_BUTTON(object) SAFE_CAST((object), gitem_base_t *, gitem_toggle_button_t *) /**< Type caster from  base gitem_base_t struct to derived gitem_toggle_button_t struct*/

#endif //NG_GITEM_TOGGLE_BUTTON_H__
