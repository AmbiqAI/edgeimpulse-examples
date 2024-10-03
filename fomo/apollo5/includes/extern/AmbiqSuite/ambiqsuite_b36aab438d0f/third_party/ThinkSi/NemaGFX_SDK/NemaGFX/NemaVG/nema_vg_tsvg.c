/* TSI 2023.gen */
// -----------------------------------------------------------------------------
// Copyright (c) 2008-23 Think Silicon Single Member PC
// Think Silicon Single Member PC Confidential Proprietary
// -----------------------------------------------------------------------------
//     All Rights reserved - Unpublished -rights reserved under
//         the Copyright laws of the European Union
//
//  This file includes the Confidential information of Think Silicon Single
//  Member PC The receiver of this Confidential Information shall not disclose
//  it to any third party and shall protect its confidentiality by using the
//  same degree of care, but not less then a reasonable degree of care, as the
//  receiver uses to protect receiver's own Confidential Information. The entire
//  notice must be reproduced on all authorized copies and copies may only be
//  made to the extent permitted by a licensing agreement from Think Silicon
//  Single Member PC.
//
//  The software/data is provided 'as is', without warranty of any kind,
//  expressed or implied, including but not limited to the warranties of
//  merchantability, fitness for a particular purpose and noninfringement. In no
//  event shall Think Silicon Single Member PC be liable for any claim, damages
//  or other liability, whether in an action of contract, tort or otherwise,
//  arising from, out of or in connection with the software.
//
//  For further information please contact:
//
//                    Think Silicon Single Member PC
//                    http://www.think-silicon.com
//                    Patras Science Park
//                    Rion Achaias 26504
//                    Greece
// -----------------------------------------------------------------------------
#include "nema_vg.h"
#include "nema_vg_font.h"
#include "nema_vg_tsvg.h"
#include "tsvg_reader.h"
#include "nema_matrix3x3.h"
#include "nema_vg_p.h"

static TLS_VAR nema_tsvg_info_t tsvg_info;

static void calc_text_bbox(Geometry_table_t geometry, float* w, float* h) {

    nema_vg_set_font_size((float)Text_size_get(Geometry_text(geometry)));
    nema_vg_string_get_bbox(Text_strText_get(Geometry_text(geometry)), w, h, tsvg_info.resx, NEMA_VG_ALIGNX_LEFT);
}

static void tsvg_draw_text(NEMA_VG_PAINT_HANDLE paint,Geometry_table_t geometry) {
    Text_table_t text_geometry = Geometry_text(geometry);

    nema_vg_set_font_size((float)Text_size_get(text_geometry));

    nema_vg_print(paint, Text_strText_get(text_geometry),
                         Text_x_get(text_geometry),
                         Text_y_get(text_geometry) - nema_vg_get_ascender_pt(), //y corresponds to the baseline, subtract the ascender
                         tsvg_info.resx, tsvg_info.resy, 0,
                         (float (*)[3])Text_matrix_get(text_geometry));
}

static void tsvg_draw_predefined(NEMA_VG_PAINT_HANDLE paint, Geometry_table_t geometry) {
    Predefined_table_t predefined_geometry = Geometry_predefined(geometry);

    if ( Predefined_shape_get(predefined_geometry) == PredefinedShape_Rect ) {
        int rx_present = Predefined_rx_is_present(predefined_geometry);
        int ry_present = Predefined_ry_is_present(predefined_geometry);

        if(rx_present != 0 || ry_present != 0){

            nema_vg_draw_rounded_rect( Predefined_x_get(predefined_geometry)      ,
                                       Predefined_y_get(predefined_geometry)      ,
                                       Predefined_width_get(predefined_geometry)  ,
                                       Predefined_height_get(predefined_geometry) ,
                                       Predefined_rx_get(predefined_geometry)     ,
                                       Predefined_ry_get(predefined_geometry)     ,
                                       (float (*)[3])Predefined_matrix(predefined_geometry),
                                       paint);
        }else{
            nema_vg_draw_rect( Predefined_x_get(predefined_geometry)      ,
                               Predefined_y_get(predefined_geometry)      ,
                               Predefined_width_get(predefined_geometry)  ,
                               Predefined_height_get(predefined_geometry) ,
                               (float (*)[3])Predefined_matrix(predefined_geometry),
                              paint);
        }

    }else if(Predefined_shape_get(predefined_geometry) == PredefinedShape_Ellipse){

        nema_vg_draw_ellipse( Predefined_cx_get(predefined_geometry) ,
                              Predefined_cy_get(predefined_geometry) ,
                              Predefined_rx_get(predefined_geometry) ,
                              Predefined_ry_get(predefined_geometry) ,
                              (float (*)[3])Predefined_matrix(predefined_geometry),
                              paint);
    }else if(Predefined_shape_get(predefined_geometry) == PredefinedShape_Circle){

        nema_vg_draw_circle( Predefined_cx_get(predefined_geometry) ,
                             Predefined_cy_get(predefined_geometry) ,
                             Predefined_r_get(predefined_geometry)  ,
                             (float (*)[3])Predefined_matrix(predefined_geometry),
                            paint);
    }else if(Predefined_shape_get(predefined_geometry) == PredefinedShape_Line){

        nema_vg_draw_line( Predefined_x1_get(predefined_geometry) ,
                           Predefined_y1_get(predefined_geometry) ,
                           Predefined_x2_get(predefined_geometry) ,
                           Predefined_y2_get(predefined_geometry) ,
                           (float (*)[3])Predefined_matrix(predefined_geometry),
                           paint);
    }

}

static void tsvg_draw_path(NEMA_VG_PATH_HANDLE path, NEMA_VG_PAINT_HANDLE paint, Geometry_table_t geometry) {
    Path_table_t path_geometry = Geometry_path(geometry);

    if(Path_matrix_is_present(path_geometry) ) {
        nema_vg_path_set_matrix(path, (float (*)[3])Path_matrix(path_geometry));
    }


    nema_vg_path_set_shape_and_bbox( path,
                                     flatbuffers_uint8_vec_len(Path_seg(path_geometry)),
                                     Path_seg(path_geometry),
                                     flatbuffers_float_vec_len(Path_data(path_geometry)),
                                     Path_data(path_geometry),
                                     Path_bbox(path_geometry)
                                    );

    uint32_t err = nema_vg_draw_path(path, paint);
}

static inline void
set_grad_paint(NEMA_VG_PAINT_HANDLE paint, Grad_table_t grad, int istext, float w, float h)
{
    flatbuffers_uint32_vec_t grad_stops = Grad_stopColors_get(grad);
    size_t len = flatbuffers_uint32_vec_len(grad_stops);

    if(istext == 0)
    {
        nema_vg_paint_lock_tran_to_path(paint,1);
    }

    color_var_t stops[NEMA_VG_PAINT_MAX_GRAD_STOPS];
    for(int j = 0; j < (int)len; j++)
    {
        uint32_t stop_ = flatbuffers_uint32_vec_at(grad_stops, j );
        stops[j].r = stop_         & 0xff;
        stops[j].g = (stop_ >> 8)  & 0xff;
        stops[j].b = (stop_ >> 16) & 0xff;
        stops[j].a = (stop_ >> 24) & 0xff;
    }

    nema_vg_grad_set(tsvg_info.gradient,
                     len,
                     (float *)Grad_stops(grad),
                     stops);

    if(Grad_isLinear_get(grad)){
        nema_vg_paint_set_type(paint, NEMA_VG_PAINT_GRAD_LINEAR);
        nema_vg_paint_set_grad_linear(paint,
                                    tsvg_info.gradient,
                                    Grad_x1_get(grad),
                                    Grad_y1_get(grad),
                                    istext != 0 ? Grad_x1_get(grad) + w: Grad_x2_get(grad),
                                    istext != 0? Grad_y1_get(grad) + h: Grad_y2_get(grad),
                                    Grad_sampling(grad)
                                    ); //set grad paint

    }else{
        nema_vg_paint_set_type(paint, NEMA_VG_PAINT_GRAD_RADIAL);
        nema_vg_paint_set_grad_radial2(paint,
                                      tsvg_info.gradient,
                                      Grad_cx_get(grad),
                                      Grad_cy_get(grad),
                                      Grad_r_get(grad),
                                      Grad_ry_get(grad),
                                      Grad_sampling(grad)
                                      ); //set grad paint
    }
}

void
nema_vg_draw_tsvg(const void* buffer){


    if(buffer == NULL)
    {
        nema_vg_set_error(NEMA_VG_ERR_BAD_BUFFER);
        return;
    }

    if(Svg_geometries_is_present( Svg_as_root(buffer)) == 0)
    {
        nema_vg_set_error(NEMA_VG_ERR_EMPTY_TSVG);
        return;
    }

    nema_vg_context_t_* context =  nema_vg_get_context();

    uint8_t global_m_enabled = 0U;
    nema_matrix3x3_t prev_global;
    nema_matrix3x3_t _global_matrix;
    uint32_t prev_blend = context->blend;
    uint32_t prev_fill_rule = context->fill_rule;
    nema_vg_set_blend(NEMA_BL_SRC_OVER | NEMA_BLOP_SRC_PREMULT);

    if(is_identity_matrix((float (*)[3]) (Svg_matrix(Svg_as_root(buffer)))) == 0U){
        //Check for GLOBAL_MATRIX
        if(context->flags & NEMA_VG_CONTEXT_HAS_GLOBAL_TRANSFORMATION){
            global_m_enabled = 1U;

            //Keep Original Global matrix
            nema_mat3x3_copy(prev_global, context->global_m);

            // Calculate the temporary global_m * svg_matrix
            nema_mat3x3_copy(_global_matrix, context->global_m);
            nema_mat3x3_mul(_global_matrix,(float (*)[3]) (Svg_matrix(Svg_as_root(buffer))));

            //Set the temporary matrix
            nema_vg_set_global_matrix(_global_matrix);
        }
        else{// No Global matrix previously set, take only svg matrix into consideration
            // nema_mat3x3_copy(_global_matrix, (float (*)[3]) (Svg_matrix(Svg_as_root(buffer))));
            nema_vg_set_global_matrix((float (*)[3]) (Svg_matrix(Svg_as_root(buffer))));
        }
    }

    Geometry_vec_t geometries = Svg_geometries( Svg_as_root(buffer));
    for(int i = 0; i <  (int)Geometry_vec_len(geometries); ++i){

        Geometry_table_t geometry = Geometry_vec_at(geometries, i);
        int isPath    = Geometry_path_is_present(geometry);
        int isPredef  = Geometry_predefined_is_present(geometry);
        int isText    = Geometry_text_is_present(geometry);

        if(Geometry_fillPaint_is_present(geometry) != 0){ //has fill paint
            nema_vg_paint_clear(tsvg_info.paint);
            Paint_table_t fill_paint = Geometry_fillPaint(geometry);

            if(Paint_grad_is_present(fill_paint) == 0) { //doesn't have gradient
                nema_vg_paint_set_type(tsvg_info.paint, NEMA_VG_PAINT_COLOR);
                nema_vg_paint_set_paint_color(tsvg_info.paint, Paint_color_get(fill_paint)); //set fill paint
            }
            else{
                float w = 0.f;
                float h = 0.f;
                if(isText !=0)
                {
                    calc_text_bbox(geometry, &w, &h);
                }
                set_grad_paint(tsvg_info.paint, Paint_grad_get(fill_paint), isText, w, h);
            }
            nema_vg_paint_set_opacity(tsvg_info.paint, Paint_opacity_get(fill_paint));

            if ( Geometry_nonzero_is_present(geometry) ) {
                if ( Geometry_nonzero_get(geometry) ) {
                    #ifndef NEMAPVG
                        #if (P_FORCE_EVEN_ODD != 0)
                            nema_vg_set_fill_rule(NEMA_VG_FILL_EVEN_ODD);
                        #else
                            nema_vg_set_fill_rule(NEMA_VG_FILL_NON_ZERO);
                        #endif
                    #else
                        nema_vg_set_fill_rule(NEMA_VG_FILL_NON_ZERO);
                    #endif
                } else {
                    nema_vg_set_fill_rule(NEMA_VG_FILL_EVEN_ODD);
                }
            } else {
                nema_vg_set_fill_rule(NEMA_VG_FILL_EVEN_ODD);
            }

            if(isPath != 0) {
                nema_vg_path_clear(tsvg_info.path);
                tsvg_draw_path(tsvg_info.path, tsvg_info.paint, geometry);
            }
            else if ( isText != 0) {
                tsvg_draw_text(tsvg_info.paint, geometry);
            }else if( isPredef != 0) {
                tsvg_draw_predefined(tsvg_info.paint, geometry);
            }
        }

        if(Geometry_strokePaint_is_present(geometry) != 0){ ///has stroke
            nema_vg_paint_clear(tsvg_info.paint);
            Paint_table_t stroke_paint = Geometry_strokePaint(geometry);

            if(Paint_grad_is_present(stroke_paint) == 0){
                nema_vg_paint_set_paint_color(tsvg_info.paint, Paint_color_get(stroke_paint)); //stroke tsvg_info.paint
            }else{
                float w = 0.f;
                float h = 0.f;
                if(isText)
                {
                    calc_text_bbox(geometry, &w, &h);
                }
                set_grad_paint(tsvg_info.paint, Paint_grad_get(stroke_paint),isText, w, h);
            }

            nema_vg_paint_set_opacity(tsvg_info.paint, Paint_opacity_get(stroke_paint));
            nema_vg_stroke_set_width(Paint_strokeWidth_get(stroke_paint));
            nema_vg_set_fill_rule(NEMA_VG_STROKE);

            if(Geometry_stroke_is_present(geometry) != 0)
            {
                Stroke_table_t stroke = Geometry_stroke(geometry);
                if((tsvg_info.disabled_features & NEMA_VG_TSVG_DISABLE_JOINS) != NEMA_VG_TSVG_DISABLE_JOINS)
                {
                    nema_vg_stroke_set_join_style(Stroke_join_get(stroke));
                }
                if((tsvg_info.disabled_features & NEMA_VG_TSVG_DISABLE_CAPS) != NEMA_VG_TSVG_DISABLE_CAPS)
                {
                    uint8_t cap = Stroke_cap_get(stroke);
                    nema_vg_stroke_set_cap_style(cap, cap);
                }
                nema_vg_stroke_set_miter_limit(Stroke_miterLimit_get(stroke));
            }

            if( isPath != 0U) {
                nema_vg_path_clear(tsvg_info.path);
                tsvg_draw_path(tsvg_info.path, tsvg_info.paint, geometry);
            }
            else if ( isText != 0U) {
                tsvg_draw_text(tsvg_info.paint, geometry);
            }else if( isPredef != 0U) {
                tsvg_draw_predefined(tsvg_info.paint, geometry);
            }
        }
    }

    if(is_identity_matrix((float (*)[3]) (Svg_matrix(Svg_as_root(buffer)))) == 0U){
        if(global_m_enabled == 1U){
            //revert global matrix to original
            nema_vg_set_global_matrix(prev_global);
        }
        else{
            //Just erase the global matrix flag
            nema_vg_reset_global_matrix();
        }
    }

    nema_vg_set_blend(prev_blend);
    nema_vg_set_fill_rule(prev_fill_rule);
}

void set_tsvg_text_max_size(uint32_t width, uint32_t height) {
    tsvg_info.resx = width;
    tsvg_info.resy = height;
}

void nema_vg_get_tsvg_resolution(const void *buffer, uint32_t *width, uint32_t *height){
    //MISRA
    float width_temp  = Svg_width(Svg_as_root(buffer));
    float heigth_temp = Svg_height(Svg_as_root(buffer));
    *width  = (uint32_t) width_temp;
    *height = (uint32_t) heigth_temp;
}

void tsvg_init(void){
    tsvg_info.paint = nema_vg_paint_create();
    tsvg_info.path = nema_vg_path_create();
    tsvg_info.gradient = nema_vg_grad_create();
    tsvg_info.resx = 800;
    tsvg_info.resy = 600;
    tsvg_info.disabled_features = NEMA_VG_TSVG_DISABLE_NONE;
}

void tsvg_deinit(void){
    nema_vg_path_destroy(tsvg_info.path);
    nema_vg_paint_destroy(tsvg_info.paint);
    nema_vg_grad_destroy(tsvg_info.gradient);
}

void
nema_vg_tsvg_disable_feature(uint32_t feature)
{
    tsvg_info.disabled_features |= feature;
}
