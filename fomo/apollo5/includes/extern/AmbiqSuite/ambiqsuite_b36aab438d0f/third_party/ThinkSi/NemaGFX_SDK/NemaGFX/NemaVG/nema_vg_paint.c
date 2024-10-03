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
#include "nema_vg_p.h"
#include "nema_vg_paint.h"
#include "nema_vg_p.h"
#include "nema_blender.h"
#include "nema_graphics.h"
#include "nema_programHW.h"

#include "nema_math.h"
#include "nema_core.h"
#include "nema_sys_defs.h"


#define GET_GRAD \
    nema_vg_grad_t* _grad = (nema_vg_grad_t*)grad;

void
nema_vg_paint_clear(NEMA_VG_PAINT_HANDLE paint)
{
    if(paint == NULL){
        nema_vg_set_error(NEMA_VG_ERR_BAD_HANDLE);
        return;
    }

    GET_PAINT;
    _paint->tex.obj = NULL;
    _paint->tex.lut_palette_obj = NULL;
    _paint->tex.is_lut_tex = 0U;
    _paint->type = NEMA_VG_PAINT_COLOR;
    _paint->paint_color = 0xFF000000U; //black
    _paint->grad.sampling_mode = NEMA_TEX_CLAMP;
    _paint->grad.grad_obj = NULL;;
    _paint->opacity = 1.f;
    nema_mat3x3_load_identity(_paint->grad.m);
    nema_mat3x3_load_identity(_paint->tex.m);

    // by default, paint is locked to path
    // FORCE_FLAGS(_paint->flags, NEMA_VG_PAINT_TRANS_LOCKED_TO_PATH);
    FORCE_FLAGS(_paint->flags, NEMA_VG_PAINT_NO_FLAGS);
}

void
nema_vg_paint_destroy(NEMA_VG_PAINT_HANDLE paint)
{
    if(paint == NULL){
        nema_vg_set_error(NEMA_VG_ERR_BAD_HANDLE);
        return;
    }

    nema_vg_paint_clear(paint);
    nema_host_free(paint);
}

NEMA_VG_PAINT_HANDLE
nema_vg_paint_create(void)
{
    NEMA_VG_PAINT_HANDLE paint = (NEMA_VG_PAINT_HANDLE)(nema_host_malloc(sizeof(nema_vg_paint_t)));
    nema_vg_paint_clear(paint);
    return paint;
}

void
nema_vg_paint_set_type(NEMA_VG_PAINT_HANDLE paint, uint8_t type)
{
    if(paint == NULL){
        nema_vg_set_error(NEMA_VG_ERR_BAD_HANDLE);
        return;
    }

    if (type != NEMA_VG_PAINT_COLOR && type != NEMA_VG_PAINT_GRAD_LINEAR &&
        type != NEMA_VG_PAINT_TEXTURE && type != NEMA_VG_PAINT_GRAD_RADIAL &&
        type != NEMA_VG_PAINT_GRAD_CONICAL){

        nema_vg_set_error(NEMA_VG_ERR_INVALID_PAINT_TYPE);
        return;
    }

    GET_PAINT;
    _paint->type = type;
}

void
nema_vg_paint_lock_tran_to_path(NEMA_VG_PAINT_HANDLE paint, int locked)
{
    if(paint == NULL){
        nema_vg_set_error(NEMA_VG_ERR_BAD_HANDLE);
        return;
    }

    GET_PAINT;

    if ( locked == 0 ) {
        CLEAR_FLAG(_paint->flags, NEMA_VG_PAINT_TRANS_LOCKED_TO_PATH);
    }
    else {
        SET_FLAG(_paint->flags, NEMA_VG_PAINT_TRANS_LOCKED_TO_PATH);
    }
}

static inline
void
find_valid_pair(int* first_index, int* second_index, float *stops, int stops_count, int* valid_stops, uint8_t* valid, uint8_t* end){

    nema_vg_float_t prev = -65535.f;
    int i = 0;

    if (*second_index != 0 || *valid_stops >= 2){
        i = *second_index + 1 ;
        prev = stops[*second_index];
    }

    while( i < stops_count){
        if(stops[i] < prev){
            *valid = 0U;
            *end = 1U ;
            return;
        }else{
            //MISRA
        }
        if(stops[i] >= 0.f && stops[i] <= 1.f){
            //the first valid stop is not at 0, replicate the first one
            if(*valid_stops == 0 && stops[i] > 0.f){
                //First Pair Found
                (*valid_stops)+=2;
                *first_index = i;
                *second_index = i;
                return;
            }
            (*valid_stops)++;
            if((*valid_stops)>1){
                //New Pair Found
                *first_index = *second_index;
                *second_index = i;
                return;
            }
        }
        prev = stops[i];
        i++;
    }

    //There is at least one stop, but the last one is not at 1, replicate the last one
    //The last correct ramp stop lies into second_index
    if(*valid != 0U && *valid_stops >= 1 && stops[*second_index] < 1.f){
        (*valid_stops)++;
        *first_index = *second_index;
    }

    *end = 1U ;
    return;
}

NEMA_VG_GRAD_HANDLE
nema_vg_grad_create(void)
{
    nema_vg_grad_t *grad = (nema_vg_grad_t *)(nema_host_malloc(sizeof(nema_vg_grad_t)));
    grad->bo = nema_buffer_create_pool(NEMA_MEM_POOL_FB, GRAD_SIZE*4);
    return (NEMA_VG_GRAD_HANDLE) grad;
}

void
nema_vg_grad_destroy(NEMA_VG_GRAD_HANDLE grad)
{
    if (  grad == NULL ) {
        return;
    }

    GET_GRAD;
    nema_buffer_destroy(&_grad->bo);
    nema_host_free(grad);
}

static void
nema_interpolate_colors_f(float x0, float w, color_var_t* col0, color_var_t* col1) {
    float dist_r = col1->r-col0->r;
    float dist_g = col1->g-col0->g;
    float dist_b = col1->b-col0->b;
    float dist_a = col1->a-col0->a;

    float r_dx = dist_r/w;
    float g_dx = dist_g/w;
    float b_dx = dist_b/w;
    float a_dx = dist_a/w;

    float x_frac = x0 - (float)(int)x0;
    float x_dist_to_05;

    if(x_frac > 0.5f){
        x_dist_to_05 = 1.5f - x_frac;
    }
    else{
        x_dist_to_05 = 0.5f - x_frac;
    }

    col0->r += x_dist_to_05*r_dx;
    col0->g += x_dist_to_05*g_dx;
    col0->b += x_dist_to_05*b_dx;
    col0->a += x_dist_to_05*a_dx;

    nema_set_gradient(col0->r, col0->g, col0->b, col0->a, r_dx, 0.f, g_dx, 0.f, b_dx, 0.f, a_dx, 0.f);
}

void
nema_vg_grad_set(NEMA_VG_GRAD_HANDLE grad, int stops_count,float *stops, color_var_t* colors)
{
    int stops_count_ = stops_count;

    if(stops_count_ > NEMA_VG_PAINT_MAX_GRAD_STOPS)
    {
        nema_vg_set_error(NEMA_VG_ERR_INVALID_GRAD_STOPS);
        stops_count_ = NEMA_VG_PAINT_MAX_GRAD_STOPS;
    }

    GET_GRAD;
    nema_set_clip_temp(0 , 0 , GRAD_SIZE, 1);
    nema_bind_tex(NEMA_TEX1, _grad->bo.base_phys,
                  GRAD_SIZE,
                  1,
                  NEMA_RGBA8888,
                  0, 0);

    nema_set_blend(NEMA_BL_SRC, NEMA_TEX1, NEMA_NOTEX, NEMA_NOTEX);

    nema_enable_gradient(1);

    //check stops
    uint8_t valid = 1U;
    uint8_t end = 0U;
    int valid_stops = 0;
    int previous_stops = 0;
    int first_index = 0;
    int second_index = 0;
    // Dry run in order to avoid re-writting to grad buffer
    while(end != 1U){
        find_valid_pair(&first_index, &second_index, stops, stops_count_, &valid_stops, &valid,&end);
        if(previous_stops == valid_stops) {
            break;
        }
        else{
            //MISRA
        }
        previous_stops = valid_stops;
    }

    if(!(valid == 0U || valid_stops == 0)){

        //Clear Variables from dry run
        valid = 1U;
        end = 0U;
        valid_stops = 0;
        previous_stops = 0;
        first_index = 0;
        second_index = 0;
        color_var_t first_color={0.0f,0.0f,0.0f,0.0f};
        color_var_t color1={0.0f,0.0f,0.0f,0.0f};
        color_var_t color2={0.0f,0.0f,0.0f,0.0f};
        uint8_t is_first = 1U;
        while(end != 1U){
            find_valid_pair(&first_index, &second_index, stops, stops_count_, &valid_stops, &valid,&end);
            if(previous_stops == valid_stops) {
                break;
            }
            else{
                //MISRA
            }
            previous_stops = valid_stops;
            float x0,x1;
            float x0_int,x1_int;
            int w;
            //Invalid First Stop
            if((first_index == second_index) && (valid_stops == 2)){
                color1 = colors[second_index];
                color2 = colors[second_index];
                x0 = 0.5f ;
                x1 = stops[second_index] * (float)(GRAD_SIZE-1) + 0.5f;
                //These are used for fill rect
                x1_int = stops[second_index] * (float)(GRAD_SIZE);
                x0_int = 0.f;
                w = (int)x1_int;
            }
            //Invalid Last Stop
            else if((first_index == second_index) && (valid_stops > 2)){
                color1 = colors[second_index];
                color2 = colors[second_index];
                x0 = stops[second_index] * (float)(GRAD_SIZE-1) + 0.5f;
                x1 = (float)GRAD_SIZE - 0.5f;
                //These are used for fill rect
                x0_int = stops[second_index] * (float)(GRAD_SIZE);
                w = GRAD_SIZE - (int)x0_int;
            }
            //Correct Pair
            else{
                color1 = colors[first_index];
                color2 = colors[second_index];
                x0 = stops[first_index]  * (float)(GRAD_SIZE-1) + 0.5f;
                x1 = stops[second_index] * (float)(GRAD_SIZE-1) + 0.5f;
                //These are used for fill rect
                x0_int = stops[first_index]  * (float)(GRAD_SIZE);
                x1_int = stops[second_index] * (float)(GRAD_SIZE);
                w = (int)x1_int - (int)x0_int;

            }
            float wf = x1 - x0;

            nema_interpolate_colors_f(x0, wf, &color1, &color2);
            nema_fill_rect((int)x0_int, 0, (int)w, 1, 0xffffffffU);

            if (is_first == 1U) {
                first_color = color1;
            }
            is_first = 0U;
        }
        nema_enable_gradient(0);
        //Ensure Correct first and last stops
        nema_fill_rect(0, 0, 1, 1, nema_rgba((unsigned char)first_color.r, (unsigned char)first_color.g,
                                             (unsigned char)first_color.b, (unsigned char)first_color.a));
        nema_fill_rect(63, 0, 1, 1, nema_rgba((unsigned char)color2.r, (unsigned char)color2.g,
                                              (unsigned char)color2.b, (unsigned char)color2.a));
    }
    else{ //there are no valid stops, add implicit stops
        color_var_t color1 = { 0.f,   0.f,     0.f, 255.f};
        color_var_t color2 = { 255.f, 255.f, 255.f, 255.f};
        color_var_t color3 = color2;
        nema_interpolate_rect_colors(0, 0, GRAD_SIZE, 1, &color1,  &color2, &color3);
        nema_fill_rect(0, 0, GRAD_SIZE, 1, 0xffffffffU);
    }

    nema_enable_gradient(0);
    nema_set_clip_pop();
}

void
nema_vg_paint_set_grad_linear(NEMA_VG_PAINT_HANDLE paint,
                              NEMA_VG_GRAD_HANDLE grad,
                              float x0, float y0,
                              float x1, float y1,
                              nema_tex_mode_t sampling_mode)
{
    nema_tex_mode_t sampling_mode_ = sampling_mode;

    if(paint == NULL){
        nema_vg_set_error(NEMA_VG_ERR_BAD_HANDLE);
        return;
    }

    GET_PAINT;
    if ( (sampling_mode_ & 0xcU) == NEMA_TEX_BORDER ) {
        // if wrapping is NEMA_TEX_BORDER
        // default to NEMA_TEX_CLAMP
        sampling_mode_ = sampling_mode_ & 0x1U;
    }
    _paint->grad.sampling_mode = sampling_mode_ & 0xfU;
    _paint->grad.grad_obj = grad;

    // Create grad matrix
    float dy = y1 - y0;
    float dx = x1 - x0;
    float length = nema_sqrt(dx*dx + dy*dy);

    if ( length != 0.f ) {
        float scale = (float)GRAD_SIZE/(float)length;
        float cosx = (float)dx/(float)length;
        float sinx = (float)dy/(float)length;

        nema_mat3x3_load_identity(_paint->grad.m);

        // translate->rotate->scale
        // x,y -> tx,ty
        _paint->grad.m[0][0] =  cosx*scale;
        _paint->grad.m[0][1] =  sinx*scale;
        _paint->grad.m[1][0] = -sinx*scale;
        _paint->grad.m[1][1] =  cosx*scale;
        _paint->grad.m[0][2] = (-x0*cosx - y0*sinx)*scale;
        _paint->grad.m[1][2] = ( x0*sinx - y0*cosx)*scale;
    }
    else {
        // points are coincident
        // make the gradient always "1"
        _paint->grad.m[0][0] = 0.f;
        _paint->grad.m[0][1] = 0.f;
        _paint->grad.m[0][2] = (float)GRAD_SIZE-0.5f;
        _paint->grad.m[1][0] = 0.f;
        _paint->grad.m[1][1] = 0.f;
        _paint->grad.m[1][2] = 0.f;
        _paint->grad.m[2][0] = 0.f;
        _paint->grad.m[2][1] = 0.f;
        _paint->grad.m[2][2] = 1.f;
    }

}

static void create_conical_grad_matrix(nema_vg_paint_t* paint,
                                       float cx, float cy,
                                       float rot_init, float scale, int quadrant,
                                       nema_matrix3x3_t m)
{
    (void) paint;

    float f = 0.f;
    float tmp;
    float rot2 = rot_init;
    float rot  = 0.f;
    float cx_  = cx;
    float cy_  = cy;

    float lut_size = (float)(GRAD_SIZE);
    float A = (f+rot)*lut_size;
    float B = (0.25f + f + rot)*lut_size;
    float C = ((-f-rot)*cx_+(-f-0.25f-rot)*cy_)*lut_size;
    float G = 1.f;
    float H = 1.f;
    float I = (-cx_-cy_);

    if (quadrant == 2) {
        f  = 0.5f;
        A = -(f+rot)*lut_size;
        B = -(0.25f+f+rot)*lut_size;
        C = -((-f-rot)*cx_+(-f-0.25f-rot)*cy_)*lut_size;
        G = -G;
        H = -H;
        I = -I;
    }
    else if (quadrant == 1) {
        tmp = cx_;    cx_ = cy_;   cy_ = -tmp;
        f   = 0.25f;
        B = (f+rot)*lut_size;
        A = -(0.25f + f + rot)*lut_size;
        C = ((-f-rot)*cx_+(-f-0.25f-rot)*cy_)*lut_size;
        G = -1.f;
        H = 1.f;
        I = (-cx_-cy_);
    }
    else if (quadrant == 3) {
        f   = 0.75f;
        tmp = cx_;    cx_ = -cy_;   cy_ = tmp;
        B = -(f+rot)*lut_size;
        A = (0.25f+f+rot)*lut_size;
        C = ((-f-rot)*cx_+(-f-0.25f-rot)*cy_)*lut_size;
        G = 1.f;
        H = -1.f;
        I = (-cx_-cy_);
    }
    else {
        // MISRA
    }

    G*=scale;
    H*=scale;
    I*=scale;

    float add = 0.5f + rot2 * lut_size;
    A+=add*G;
    B+=add*H;
    C+=add*I;

    m[0][0] = A;
    m[0][1] = B;
    m[0][2] = C;
    m[1][0] = 0.f;
    m[1][1] = 0.f;
    m[1][2] = 0.f;
    m[2][0] = G;
    m[2][1] = H;
    m[2][2] = I;

}

void
nema_vg_paint_set_grad_conical(NEMA_VG_PAINT_HANDLE paint,
                               NEMA_VG_GRAD_HANDLE grad,
                               float cx, float cy,
                               nema_tex_mode_t sampling_mode)
{
    if(paint == NULL){
        nema_vg_set_error(NEMA_VG_ERR_BAD_HANDLE);
        return;
    }

    GET_PAINT;

    _paint->grad.grad_obj = grad;
    _paint->grad.sampling_mode = sampling_mode;

    // Create grad matrix
    create_conical_grad_matrix(_paint, cx, cy, 0.f, 1.f, 0, _paint->grad.m);
    create_conical_grad_matrix(_paint, cx, cy, 0.f, 1.f, 1, _paint->grad.m1);
    create_conical_grad_matrix(_paint, cx, cy, 0.f, 1.f, 2, _paint->grad.m2);
    create_conical_grad_matrix(_paint, cx, cy, 0.f, 1.f, 3, _paint->grad.m3);
    _paint->grad.cx = cx;
    _paint->grad.cy = cy;
}

void
nema_vg_paint_set_grad_radial2( NEMA_VG_PAINT_HANDLE paint,
                                NEMA_VG_GRAD_HANDLE grad,
                                float x0, float y0,
                                float rx, float ry,
                                nema_tex_mode_t sampling_mode)
{
    if(paint == NULL){
        nema_vg_set_error(NEMA_VG_ERR_BAD_HANDLE);
        return;
    }

    GET_PAINT;

    _paint->grad.grad_obj = grad;
    _paint->grad.sampling_mode = sampling_mode;

    if(!(nema_float_is_zero(rx) || nema_float_is_zero(ry))){
        float scalex = (float)GRAD_SIZE/rx;
        float scaley = (float)GRAD_SIZE/ry;

        //Create grad matrix
        _paint->grad.m[0][0] = -scalex;
        _paint->grad.m[0][1] =  0.f;
        _paint->grad.m[0][2] =  x0*scalex;
        _paint->grad.m[1][0] =  0.f;
        _paint->grad.m[1][1] = -scaley;
        _paint->grad.m[1][2] =  y0*scaley;

        _paint->grad.m[2][0] =  0.f;
        _paint->grad.m[2][1] =  0.f;
        _paint->grad.m[2][2] =  1.f;
    }
    else {
        // points are coincident
        // make the gradient always "1"
        _paint->grad.m[0][0] = 0.f;
        _paint->grad.m[0][1] = 0.f;
        _paint->grad.m[0][2] = (float)GRAD_SIZE - 0.5f;
        _paint->grad.m[1][0] = 0.f;
        _paint->grad.m[1][1] = 0.f;
        _paint->grad.m[1][2] = 0.f;
        _paint->grad.m[2][0] = 0.f;
        _paint->grad.m[2][1] = 0.f;
        _paint->grad.m[2][2] = 1.f;
    }
}

void
nema_vg_paint_set_grad_radial(NEMA_VG_PAINT_HANDLE paint,
                              NEMA_VG_GRAD_HANDLE grad,
                              float x0, float y0,
                              float r,
                              nema_tex_mode_t sampling_mode)
{
    nema_vg_paint_set_grad_radial2(paint, grad, x0, y0, r, r, sampling_mode);
}

void
nema_vg_paint_set_paint_color(NEMA_VG_PAINT_HANDLE paint, uint32_t rgba)
{
    if(paint == NULL){
        nema_vg_set_error(NEMA_VG_ERR_BAD_HANDLE);
        return;
    }
    GET_PAINT;
    _paint->paint_color = rgba;
}

void nema_vg_paint_set_opacity(NEMA_VG_PAINT_HANDLE paint, float opacity)
{
    if(paint == NULL){
        nema_vg_set_error(NEMA_VG_ERR_BAD_HANDLE);
        return;
    }
    GET_PAINT;
    _paint->opacity = opacity;
}

void
nema_vg_paint_set_stroke_width(NEMA_VG_PAINT_HANDLE paint, float stroke_width)
{
    (void) paint;
    nema_vg_stroke_set_width(stroke_width);
}

void
nema_vg_paint_set_tex_matrix(NEMA_VG_PAINT_HANDLE paint, nema_matrix3x3_t m)
{
    if(paint == NULL){
        nema_vg_set_error(NEMA_VG_ERR_BAD_HANDLE);
        return;
    }
    GET_PAINT;
    for(int i = 0; i < 3; ++i){
        for(int j = 0; j < 3; ++j){
            _paint->tex.m[i][j] =  m[i][j];
        }
    }

    if ( nema_mat3x3_invert(_paint->tex.m) != 0 ) {
        nema_vg_set_error(NEMA_VG_ERR_NON_INVERTIBLE_MATRIX);
    }
}

void
nema_vg_paint_set_tex(NEMA_VG_PAINT_HANDLE paint, nema_img_obj_t* tex)
{
    if(paint == NULL){
        nema_vg_set_error(NEMA_VG_ERR_BAD_HANDLE);
        return;
    }
    GET_PAINT;
    _paint->tex.obj = tex;
    _paint->tex.is_lut_tex = 0U;

}

void
nema_vg_paint_set_lut_tex(NEMA_VG_PAINT_HANDLE paint, nema_img_obj_t* lut_palette, nema_img_obj_t* lut_indices)
{
    if(paint == NULL){
        nema_vg_set_error(NEMA_VG_ERR_BAD_HANDLE);
        return;
    }
    GET_PAINT;
    //Check For LUT Palette and Indices
    if(lut_palette == NULL || lut_indices == NULL){
        nema_vg_set_error(NEMA_VG_ERR_BAD_BUFFER);
        return;
    }
    //Check For LUT Indices Format
    if(!(lut_indices->format == NEMA_L8   || lut_indices->format == NEMA_L4   ||
         lut_indices->format == NEMA_L2   || lut_indices->format == NEMA_L1   ||
         lut_indices->format == NEMA_L4LE || lut_indices->format == NEMA_L2LE ||
         lut_indices->format == NEMA_L1LE )){
        nema_vg_set_error(NEMA_VG_ERR_INVALID_LUT_IDX_FORMAT);
        return;
    }

    _paint->tex.obj = lut_indices;
    _paint->tex.lut_palette_obj = lut_palette;
    _paint->tex.is_lut_tex = 1U;
}

/*
***   WARNING!!! If radial is not supported, this function will change paint to fill type  ****
*/
void bind_tex1_according_paint(nema_vg_paint_t *paint)
{
    switch (paint->type)
    {
        case NEMA_VG_PAINT_GRAD_LINEAR:
        case NEMA_VG_PAINT_GRAD_CONICAL:
        {
            nema_bind_tex(NEMA_TEX1, paint->grad.grad_obj->bo.base_phys,
                        GRAD_SIZE,
                        1,
                        NEMA_RGBA8888,
                        0, paint->grad.sampling_mode | NEMA_FILTER_BL);
            break;
        }
        case NEMA_VG_PAINT_GRAD_RADIAL:
        {
            if ((nema_readHwConfigH() & NEMA_CONF_MASK_RADIAL) == 0U){
            // if radial is not enabled on HW do a paint fill
                nema_vg_set_error(NEMA_VG_ERR_NO_RADIAL_ENABLED);
                paint->type = NEMA_VG_PAINT_COLOR;
            }
            else{
                nema_bind_tex(NEMA_TEX1, paint->grad.grad_obj->bo.base_phys,
                            1,
                            GRAD_SIZE,
                            NEMA_RGBA8888,
                            4, paint->grad.sampling_mode | (0x1U << 7) | NEMA_FILTER_BL);
            }
            break;
        }
        case NEMA_VG_PAINT_TEXTURE:
        {
            if(paint->tex.obj != NULL){
                if(paint->tex.is_lut_tex == 0U){
                    nema_bind_tex(NEMA_TEX1, paint->tex.obj->bo.base_phys,
                                paint->tex.obj->w,
                                paint->tex.obj->h,
                                paint->tex.obj->format,
                                paint->tex.obj->stride,
                                paint->tex.obj->sampling_mode);
                }
                //LUT TEX
                else{
                    if(paint->tex.lut_palette_obj != NULL){
                        nema_bind_lut_tex( paint->tex.obj->bo.base_phys, paint->tex.obj->w, paint->tex.obj->h,
                        paint->tex.obj->format, paint->tex.obj->stride, paint->tex.obj->sampling_mode, paint->tex.lut_palette_obj->bo.base_phys, paint->tex.lut_palette_obj->format);
                    }
                    else{
                        nema_vg_set_error(NEMA_VG_ERR_BAD_BUFFER);
                    }
                }
            }
            else{
                nema_vg_set_error(NEMA_VG_ERR_BAD_BUFFER);
            }
            break;
        }
        default:
        {
            break;
        }
    }
}

void set_matrix_according_paint(nema_vg_paint_t *paint, nema_matrix3x3_t path_m, int conic_quad)
{
    switch(paint->type)
    {
        case NEMA_VG_PAINT_COLOR:
        {
            break;
        }
        case NEMA_VG_PAINT_GRAD_LINEAR:
        case NEMA_VG_PAINT_GRAD_RADIAL:
        {
            nema_vg_context_t_* context_var = nema_vg_get_context();
            if ( FLAG_IS_SET(paint->flags, NEMA_VG_PAINT_TRANS_LOCKED_TO_PATH) ) {
                uint8_t has_transf_ = 0U;
                if ( path_m != NULL ) {
                    has_transf_ = 1U;
                } else {
                    has_transf_ = (uint8_t)(context_var->flags & NEMA_VG_CONTEXT_HAS_GLOBAL_TRANSFORMATION );
                }

                if ( has_transf_ == 0U ) {
                    nema_set_matrix(paint->grad.m);
                }
                else {
                    nema_matrix3x3_t p_plus_g_m_inv;
                    nema_mat3x3_load_identity(p_plus_g_m_inv);
                    if ( (context_var->flags & NEMA_VG_CONTEXT_HAS_GLOBAL_TRANSFORMATION   ) != 0U ) {
                        nema_mat3x3_mul(p_plus_g_m_inv, context_var->global_m);
                    }
                    if ( path_m != NULL ) {
                        nema_mat3x3_mul(p_plus_g_m_inv, path_m);
                    }

                    if ( nema_mat3x3_invert(p_plus_g_m_inv) != 0 ) {
                        nema_vg_set_error(NEMA_VG_ERR_NON_INVERTIBLE_MATRIX);
                        return;
                    }

                    nema_matrix3x3_t m;
                    nema_mat3x3_copy(m, paint->grad.m);
                    nema_mat3x3_mul(m, p_plus_g_m_inv);
                    nema_set_matrix(m);
                }
            }
            else {
                if ( (  context_var->flags & NEMA_VG_CONTEXT_HAS_GLOBAL_TRANSFORMATION ) == 0U ) {
                    nema_set_matrix(paint->grad.m);
                }
                else {
                    nema_matrix3x3_t m;
                    nema_mat3x3_copy(m, paint->grad.m);
                    nema_mat3x3_mul(m, context_var->global_m_inv);
                    nema_set_matrix(m);
                }
            }
            break;
        }
        case NEMA_VG_PAINT_TEXTURE:
        {
            nema_vg_context_t_* context_var = nema_vg_get_context();
            if ( FLAG_IS_SET(paint->flags, NEMA_VG_PAINT_TRANS_LOCKED_TO_PATH) ) {
                uint8_t has_transf_ = 0U;
                if ( path_m != NULL ) {
                    has_transf_ = 1U;
                } else {
                    has_transf_ = (uint8_t) (context_var->flags & NEMA_VG_CONTEXT_HAS_GLOBAL_TRANSFORMATION );
                }

                if ( has_transf_ == 0U ) {
                    nema_set_matrix(paint->tex.m);
                }
                else {
                    nema_matrix3x3_t p_plus_g_m_inv;
                    nema_mat3x3_load_identity(p_plus_g_m_inv);
                    if ( (context_var->flags & NEMA_VG_CONTEXT_HAS_GLOBAL_TRANSFORMATION   ) != 0U ) {
                        nema_mat3x3_mul(p_plus_g_m_inv, context_var->global_m);
                    }
                    if ( path_m != NULL ) {
                        nema_mat3x3_mul(p_plus_g_m_inv, path_m);
                    }

                    if ( nema_mat3x3_invert(p_plus_g_m_inv) != 0 ) {
                        nema_vg_set_error(NEMA_VG_ERR_NON_INVERTIBLE_MATRIX);
                        return;
                    }

                    nema_matrix3x3_t m;
                    nema_mat3x3_copy(m, paint->tex.m);
                    nema_mat3x3_mul(m, p_plus_g_m_inv);
                    nema_set_matrix(m);
                }
            }
            else {
                if ( (  context_var->flags & NEMA_VG_CONTEXT_HAS_GLOBAL_TRANSFORMATION ) == 0U ) {
                    nema_set_matrix(paint->tex.m);
                }
                else {
                    nema_matrix3x3_t m;
                    nema_mat3x3_copy(m, paint->tex.m);
                    nema_mat3x3_mul(m, context_var->global_m_inv);
                    nema_set_matrix(m);
                }
            }
            break;
        }
        case NEMA_VG_PAINT_GRAD_CONICAL:
        {
            if (conic_quad == 0)
            {
                nema_set_matrix(paint->grad.m);
            }
            if (conic_quad == 1)
            {
                nema_set_matrix(paint->grad.m1);
            }
            if (conic_quad == 2)
            {
                nema_set_matrix(paint->grad.m2);
            }
            if (conic_quad == 3)
            {
                nema_set_matrix(paint->grad.m3);
            }
            break;
        }
        default:
        {
            break;
        }
    }
}

uint32_t
get_paint_type(NEMA_VG_PAINT_HANDLE paint)
{
    GET_PAINT;
    return _paint->type;
}
