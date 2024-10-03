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

#ifndef NG_GITEM_WATCHFACE_H__
#define NG_GITEM_WATCHFACE_H__
#include "ng_gitem.h"

/** Watchface widget data struct */
typedef struct _gitem_watchface_t{
    BASE_STRUCT;              /**< Inherited attributes from gitem_base_t data struct*/
    img_obj_t    *image;      /**< Pointer to image asset*/
    gitem_base_t *hour;       /**< Pointer to its hours hand child item (gitem_needle_t)*/
    gitem_base_t *minute;     /**< Pointer to its minutes hand child item (gitem_needle_t)*/
    gitem_base_t *sec;        /**< Pointer to its seconds hand child item (gitem_needle_t)*/
    uint16_t      pen_width;
} gitem_watchface_t;

/** \brief Draw function
 *
 * \param *git 	Pointer to watchface's base gitem (gitem_base_t data struct)
 * \param x_off Horizontal offset from its parent item
 * \param y_off Vertical offset from its parent item
 * \return void
 *
 */
DRAW_FUNC(ng_watchface_draw);

/** \brief Updates the watchface's hand angles according to the current time. System's wall time is used by default.
 *
 * In order to use a different time update method (not the system's wall time), the define WALL_TIME_CLOCKS (defined in the 
 * compiler flags of the generated Makefile) needs to be undefined and the time needs to be updated inside the #else segment of this function.
 * 
 * \param *git 	Pointer to target gitem (gitem_base_t data struct)
 * \return void
 *
 */
void ng_watchface_update(gitem_base_t *git);

/** Watchface gestures data struct (placeholder)*/
extern const gitem_gestures_t watchface_gestures;

#define NG_WATCHFACE(object) SAFE_CAST((object), gitem_base_t *, gitem_watchface_t *) /**< Type caster from  base gitem_base_t struct to derived gitem_watchface_t struct*/

#endif //NG_GITEM_SCREEN_H__
