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

#ifndef NG_GITEM_IMAGE_H__
#define NG_GITEM_IMAGE_H__
#include "ng_gitem.h"
#include "ng_utils.h"

/** Image widget data struct */
typedef struct _gitem_image_t{
    BASE_STRUCT;       /**< Inherited attributes from gitem_base_t data struct*/
    img_obj_t *image;  /**< Pointer to image asset*/
} gitem_image_t;

/** \brief Draw function
 *
 * \param *git 	Pointer to image's base gitem (gitem_base_t data struct)
 * \param x_off Horizontal offset from its parent item
 * \param y_off Vertical offset from its parent item
 * \return void
 *
 */
DRAW_FUNC(ng_image_draw);

/** \brief Set image asset
 *
 * \param *git 	Pointer to target gitem (gitem_base_t data struct)
 * \param *asset_ptr Pointer to image asset (casted to img_obj_t internally)
 * \return void
 *
 */
void ng_image_set_asset(gitem_base_t *git, void *asset_ptr);

#define NG_IMAGE(object) SAFE_CAST((object), gitem_base_t *, gitem_image_t *) /**< Type caster from  base gitem_base_t struct to derived gitem_image_t struct*/

#endif //NG_GITEM_IMAGE_H__
