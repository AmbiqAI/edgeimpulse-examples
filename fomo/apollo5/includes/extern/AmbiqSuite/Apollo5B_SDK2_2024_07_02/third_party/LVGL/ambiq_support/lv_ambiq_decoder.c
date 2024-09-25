//*****************************************************************************
//
//  lv_ambiq_decoder.c
//! @file
//!
//! @brief Ambiq format image texture decoder.
//!
//
//*****************************************************************************

//*****************************************************************************
//
// Copyright (c) 2024, Ambiq Micro, Inc.
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// 1. Redistributions of source code must retain the above copyright notice,
// this list of conditions and the following disclaimer.
//
// 2. Redistributions in binary form must reproduce the above copyright
// notice, this list of conditions and the following disclaimer in the
// documentation and/or other materials provided with the distribution.
//
// 3. Neither the name of the copyright holder nor the names of its
// contributors may be used to endorse or promote products derived from this
// software without specific prior written permission.
//
// Third party software included in this distribution is subject to the
// additional license terms as defined in the /docs/licenses directory.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
// ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
// LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
// CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
// SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
// INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
// CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
// ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
// POSSIBILITY OF SUCH DAMAGE.
//
// This is part of revision release_a5b_sdk2-748191cd0 of the AmbiqSuite Development Package.
//
//*****************************************************************************

#include "lvgl.h"


/*********************
 *      DEFINES
 *********************/
#define CF_BUILT_IN_FIRST LV_IMG_CF_AMBIQ_TSVG
#define CF_BUILT_IN_LAST LV_IMG_CF_AMBIQ_RGBA4444

/**
 * Get info about a built-in image
 * @param decoder the decoder where this function belongs
 * @param src the image source: pointer to an `lv_img_dsc_t` variable, a file path or a symbol
 * @param header store the image data here
 * @return LV_RES_OK: the info is successfully stored in `header`; LV_RES_INV: unknown format or other error.
 */
lv_res_t lv_img_decoder_ambiq_info(lv_img_decoder_t * decoder, const void * src, lv_img_header_t * header)
{
    (void)decoder; /*Unused*/

    lv_img_src_t src_type = lv_img_src_get_type(src);
    if(src_type == LV_IMG_SRC_VARIABLE) {
        lv_img_cf_t cf = ((lv_img_dsc_t *)src)->header.cf;
        if(cf < CF_BUILT_IN_FIRST || cf > CF_BUILT_IN_LAST) return LV_RES_INV;

        header->w  = ((lv_img_dsc_t *)src)->header.w;
        header->h  = ((lv_img_dsc_t *)src)->header.h;
        header->cf = ((lv_img_dsc_t *)src)->header.cf;
    }
    else if(src_type == LV_IMG_SRC_FILE) {
        /*We don't support "*.bin" files*/
        return LV_RES_INV;
    }
    else if(src_type == LV_IMG_SRC_SYMBOL) {
        /*The size depend on the font but it is unknown here. It should be handled outside of the
         *function*/
        header->w = 1;
        header->h = 1;
        /*Symbols always have transparent parts. Important because of cover check in the draw
         *function. The actual value doesn't matter because lv_draw_label will draw it*/
        header->cf = LV_IMG_CF_ALPHA_1BIT;
    }
    else {
        LV_LOG_WARN("Image get info found unknown src type");
        return LV_RES_INV;
    }
    return LV_RES_OK;
}

void _lv_img_decoder_ambiq_add(void)
{
    lv_img_decoder_t * decoder;

    /*Create a decoder for the built in color format*/
    decoder = lv_img_decoder_create();
    LV_ASSERT_MALLOC(decoder);
    if(decoder == NULL) {
        LV_LOG_WARN("lv_img_decoder_init: out of memory");
        return;
    }

    // We only set set_info_cb, no other API is needed, as LV_IMG_SRC_FILE is not supported. 
    lv_img_decoder_set_info_cb(decoder, lv_img_decoder_ambiq_info);
}
