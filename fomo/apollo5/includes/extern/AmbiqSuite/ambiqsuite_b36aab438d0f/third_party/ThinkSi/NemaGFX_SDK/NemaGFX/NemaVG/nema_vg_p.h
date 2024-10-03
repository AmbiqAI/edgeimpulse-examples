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
#ifndef NEMA_VG_P_H__
#define NEMA_VG_P_H__

#include "nema_core.h"
#include "nema_sys_defs.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifndef PRINT_F
#if 1
#define PRINT_F(...)
#else
#include <stdio.h>
#define PRINT_F printf
#endif
#endif

#ifndef MIN_COORD
#define MIN_COORD -511.f
#endif

#ifndef MAX_COORD
#define MAX_COORD  1535.f
#endif

#ifndef NEMA_MEM_POOL_FB
#define NEMA_MEM_POOL_FB 0
#endif

#ifndef NEMA_MEM_POOL_CLIPPED_PATH
#ifdef NEMA_MEM_POOL_CL
#define NEMA_MEM_POOL_CLIPPED_PATH NEMA_MEM_POOL_CL
#else
#define NEMA_MEM_POOL_CLIPPED_PATH 0
#endif
#endif

#define NEMA_SVG        0x1FU  /**< SVG (source only) */

#ifndef DECASTELJAU_CUBIC_FLATNESS
// 0.125 seems the best option
#define DECASTELJAU_CUBIC_FLATNESS 0.125f
#endif //DECASTELJAU_CUBIC_FLATNESS
#ifndef DECASTELJAU_QUAD_FLATNESS
// Tried 0.25, 0.5, 1
// 0.5 is better than 1
// 0.5 looks the same as 0.25
#define DECASTELJAU_QUAD_FLATNESS 0.5f
#endif //DECASTELJAU_QUAD_FLATNESS

#define NEMA_VG_PATH_NO_FLAGS                      (0x00000000U)
#define NEMA_VG_PATH_HAS_TRANSFORMATION            (0x00000001U)

#define NEMA_VG_CONTEXT_NO_FLAGS                   (0x00000000U)
#define NEMA_VG_CONTEXT_HAS_GLOBAL_TRANSFORMATION  (0x00000001U)
#define NEMA_VG_CONTEXT_STROKE_WIDTH_SCREEN_SPACE  (0x00000002U)
#define NEMA_VG_CONTEXT_ENABLE_HANDLE_LARGE_COORDS (0x00000004U)
#define NEMA_VG_CONTEXT_ALLOW_INTERNAL_ALLOC       (0x00000010U)

#define NEMA_VG_STENCIL_NO_FLAGS                   (0x00000000U)
#define NEMA_VG_STENCIL_ALLOC                      (0x00000001U)

#define NEMA_VG_PAINT_NO_FLAGS                     (0x00000000U)
#define NEMA_VG_PAINT_TRANS_LOCKED_TO_PATH         (0x00000001U)

#define NEMA_VG_ASSERT_ERRORS

#ifdef PROJECT_TIGER
#ifndef NEMAPVG
#define NEMAPVG
#endif //NEMAPVG
#endif //PROJECT_TIGER

#ifdef NEMAPVG
#ifdef HAAS
#undef HAAS
#endif //HAAS
#else
#ifndef HAAS
#define HAAS
#endif //HAAS
#endif //NEMAPVG

#ifndef NEMAPVG
#ifndef P_FORCE_EVEN_ODD
#define P_FORCE_EVEN_ODD 1 /**< If different than 0, force tsvg/font files to be drawn using even-odd fill rule */
#endif
#endif //NEMAPVG


#ifdef NEMA_VG_ASSERT_ERRORS
#define NEMA_VG_ASSERT(expr, retval) \
    if (!(expr)){ \
        nema_vg_set_error(retval); \
        return retval; \
    }

#define NEMA_VG_IF_ERROR(expr) \
    {   \
        uint32_t ret_ = (expr) ; \
        if (ret_ != NEMA_VG_ERR_NO_ERROR){ \
            enable_draw_on_fb(0); \
            enable_use_bevel_joins(0); \
            nema_vg_set_error(ret_); \
            return ret_; \
        } \
    }
#else

#define NEMA_VG_ASSERT(...)
#define NEMA_VG_IF_ERROR(expr) (expr)

#endif

#define LUT_SIZE 256
#define GRAD_SIZE 64

#define STACK_DEPTH 10

#define NEMA_VG_CUBIC_DATA_SIZE  6
#define NEMA_VG_SCUBIC_DATA_SIZE 4
#define NEMA_VG_QUAD_DATA_SIZE   4
#define NEMA_VG_SQUAD_DATA_SIZE  2
#define NEMA_VG_LINE_DATA_SIZE   2
#define NEMA_VG_VLINE_DATA_SIZE  1
#define NEMA_VG_HLINE_DATA_SIZE  1
#define NEMA_VG_MOVE_DATA_SIZE   2
#define NEMA_VG_CLOSE_DATA_SIZE  0
#define NEMA_VG_ARC_DATA_SIZE    5

// Needed in large coords, where strokes have to be handled differently than fills (eg. do not render strokes at all)
#define NEMA_VG_PRIM_NO_STROKE          (0x80U) /**< No stroke */
#define NEMA_VG_NO_REL_MASK             (~(NEMA_VG_REL | NEMA_VG_PRIM_NO_STROKE)) /**< No Rel segment*/

#define CLEAR_FLAG(flags_var,flag_bits)      (        (flags_var) &= ~(flag_bits)  )
#define SET_FLAG(flags_var,flag_bits)        (        (flags_var) |=  (flag_bits)  )
#define FORCE_FLAGS(flags_var,flag_bits)     (        (flags_var)  =  (flag_bits)  )
#define FLAG_IS_SET(flags_var,flag_bits)     ( 0U != ((flags_var)  &  (flag_bits)) )
#define FLAG_IS_NOT_SET(flags_var,flag_bits) ( 0U == ((flags_var)  &  (flag_bits)) )

#define GET_PAINT \
    nema_vg_paint_t* _paint = (nema_vg_paint_t*) paint;

#define GET_PATH \
    nema_vg_path_t* _path = (nema_vg_path_t*) path;

#define GET_CONTEXT \
	nemavg_context = nema_vg_get_context();

#define GET_STENCIL \
    stencil =  nema_vg_get_stencil(); \

#define GET_LUT \
    nema_buffer_t* lut =  nema_vg_get_lut(); \

// -------------------------------------------------------------------------------
//                           VERTEX BUFFER MANAGEMENT
// -------------------------------------------------------------------------------

typedef struct {
    nema_vg_float_t x; /**< X coordinate position */
    nema_vg_float_t y; /**< Y coordinate positionr */
} nema_vg_vertex_t_;

typedef struct {
    nema_buffer_t bo;
    int width;
    int height;
    uint32_t flags;
    nema_vg_vertex_t_ dirty_area_p1;
    nema_vg_vertex_t_ dirty_area_p2;
    nema_vg_vertex_t_ dirty_area_p3;
    nema_vg_vertex_t_ dirty_area_p4;
} nema_vg_stencil_t_;

typedef struct {
    nema_img_obj_t *mask_obj;
    float trans_x;
    float trans_y;
    uint8_t masking;
}nema_vg_masking_info_t_;

#define FB_TSC4     0x1U
#define FB_TSC6     0x2U
#define FB_TSC12    0x4U
#define FB_TSC_MASK 0x7U


typedef struct {
    nema_vg_vertex_t_ q0;
    nema_vg_vertex_t_ q1;
    nema_vg_vertex_t_ q2;
    nema_vg_vertex_t_ q3;
    float length;// we can check if the line is valid by checking length > 0.0f
    float dx, dy;
}stroked_line_segment_t_;

typedef struct {
    float fanx0_f;
    float fany0_f;
    int32_t fanx0;
    int32_t fany0;
    int32_t fanx0fx;
    int32_t fany0fx;
}fan_center_info_t_;

typedef struct {
    int      do_fat_stroke;
    int      do_draw_on_fb;
    int      use_bevel_joins; //when we are in shapes or fonts we want bevel joins
    int      do_not_transform;
    int      last_vertex;
    uint32_t paint_color;
    uint32_t num_stroked_line_segments;
    float    stroke_w;    // the stroke as given by the user
    float    stroke_w_x;  // the stroke in the x dimension after applying the transform
    float    stroke_w_y;  // the stroke in the y dimension after applying the transform
    stroked_line_segment_t_ first_line; //this is the shape first line
    stroked_line_segment_t_ last_line; //this is the last drawn line
    stroked_line_segment_t_ end_line; //this the shape end line
    uint8_t has_transformation; // This is updated from entry points (draw_path, draw_rect etc)
    uint8_t had_transformation; // This is updated from entry points (draw_path, draw_rect etc)
    uint8_t pre_clip_transformation;
    uint8_t faster_quality;
    uint8_t has_cap;
    uint8_t draw_join;
    float global_sx_factor;
    float global_sy_factor;
    float tess_stack[STACK_DEPTH][6];
    int tess_stack_idx; // idx points to next free slot
    fan_center_info_t_ fan_center;
#ifndef NEMAPVG
#ifdef HAAS
    int32_t clip_x_orig;
    int32_t clip_y_orig;
    int32_t clip_w_orig;
    int32_t clip_h_orig;
#endif
#endif
} nema_vg_context_info_t;

typedef struct {
    int resx;
    int resy;
    NEMA_VG_PAINT_HANDLE paint;
    NEMA_VG_PATH_HANDLE path;
    NEMA_VG_GRAD_HANDLE gradient;
    uint32_t disabled_features;
} nema_tsvg_info_t;

typedef struct {
    float width; /**< Stroking width */
    float miter_limit;  /**< Mitter join length */
    uint8_t start_cap_style;   /**< Start Cap style */
    uint8_t end_cap_style;   /**< End Cap style */
    uint8_t join_style;  /**< Join style */
} nema_vg_stroke_t;

typedef struct {
    nema_buffer_t* clipped_segs ; /**< Clipped seg buffer */
    nema_buffer_t* clipped_data ; /**< Clipped data buffer */
    uint8_t fill_rule; /**< Fill rule*/
    uint8_t quality; /**< Quality*/
    uint8_t drawing_clipped_path;
    uint8_t fb_tsc; /**< Framebuffer format is TSC (TSC4, TSC6 or TSC12) */
    uint32_t error; /**< Error code */
    uint32_t blend; /**< Blending mode */
    uint32_t aa_flags; /**< AA flags */
    uint32_t aa_mask; /**< AA mask */
    uint32_t flags; /**< Context flags  */
    uint32_t clipped_segs_size_bytes ; /**< Clipped seg size in bytes */
    uint32_t clipped_data_size_bytes ; /**< Clipped seg size in bytes */
    nema_vg_masking_info_t_ masking_info;
    nema_matrix3x3_t global_m; /**< Grad matrix */
    nema_matrix3x3_t global_m_inv; /**< Grad matrix */
    nema_matrix3x3_t path_plus_global_m;
    nema_vg_context_info_t info;
    nema_vg_stroke_t stroke; /**< stroke */
} nema_vg_context_t_;

typedef struct {
    uint8_t no_move;
    uint8_t no_close;
    uint8_t new_path;
    uint8_t seg;
    size_t idx_data;
    size_t start_idx;
    nema_vg_vertex_t_ p1; //points after transformation
    nema_vg_vertex_t_ p2; //points after transformation
    nema_vg_vertex_t_ ctrl0; //ctrl point after transformation
    nema_vg_vertex_t_ ctrl1; //ctrl point after transformation
    nema_vg_vertex_t_ previous_data; //previous data
    nema_vg_vertex_t_ previous_ctrl; //previous ctrl
    nema_vg_vertex_t_ path_start_data; //Path Start data
    uint8_t previous_seg; //previous seg
} nema_vg_info_t_;

typedef struct {
    size_t new_seg_size;
    size_t new_data_size;
    size_t idx_new_data;
    size_t idx_new_seg;
    nema_vg_float_t* new_data;
    nema_vg_vertex_t_ new_start_data;
    uint8_t* new_segs;
    uint8_t save_start_data;
    uint8_t has_transformation;
    uint32_t prev_cmd_is_move;
    uint32_t error;
    int _dry_run;
    float _min_coord;
    float _max_coord;
    nema_matrix3x3_t path_plus_global_m;
} nema_vg_culled_info_t_;

typedef struct {
    nema_buffer_t bo;
} nema_vg_grad_t;

typedef struct {
    nema_vg_grad_t* grad_obj;
    nema_matrix3x3_t m; /**< Grad matrix */
    nema_matrix3x3_t m1; /**< Grad matrix for conical quadrant 2*/
    nema_matrix3x3_t m2; /**< Grad matrix for conical quadrant 3*/
    nema_matrix3x3_t m3; /**< Grad matrix for conical quadrant 4*/
    nema_vg_float_t cx;
    nema_vg_float_t cy;
    nema_tex_mode_t sampling_mode;
} nema_vg_paint_grad_t;

typedef struct {
    nema_img_obj_t *obj; /**< Paint texture */
    nema_img_obj_t *lut_palette_obj; /**< Lut indices */
    nema_matrix3x3_t m; /**< Tex matrix */
    uint8_t is_lut_tex;
} nema_vg_paint_tex_t;

typedef struct {
    nema_vg_paint_tex_t tex; /**< Paint texture */
    nema_vg_paint_grad_t grad; /**< Paint gradient */
    float opacity;
    uint32_t flags;
    uint8_t type; /**< Paint type */
    uint32_t paint_color; /**< Paint color */
} nema_vg_paint_t;

typedef struct {
    nema_vg_vertex_t_ min; /**< Minimum x,y point */
    nema_vg_vertex_t_ max; /**< Maximum x,y point */
    nema_vg_vertex_t_ transformed_p1; /**<  x1 x,y point after matrix transformation. Contains also the paint stroke width*/
    nema_vg_vertex_t_ transformed_p2; /**<  x1 x,y point after matrix transformation. Contains also the paint stroke width*/
    nema_vg_vertex_t_ transformed_p3; /**<  x1 x,y point after matrix transformation. Contains also the paint stroke width*/
    nema_vg_vertex_t_ transformed_p4; /**<  x1 x,y point after matrix transformation. Contains also the paint stroke width*/
    nema_vg_vertex_t_ transformed_min; /**< Minimum x,y point after matrix transformation. Contains also the paint stroke width */
    nema_vg_vertex_t_ transformed_max; /**< Maximum x,y point after matrix transformation. Contains also the paint stroke width */
    float w; /**<  Path width */
    float h; /**<  Path height */
    float transformed_w; /**<  Path rotated width. Contains also the paint stroke width */
    float transformed_h; /**<  Path rotated height. Contains also the paint stroke width */
} nema_vg_path_bbox_t;

typedef struct {
    size_t                 seg_size;
    size_t                 data_size;
    const uint8_t*         seg;
    const nema_vg_float_t* data;
    nema_vg_path_bbox_t    bbox;  /**< Path bounding box */
} nema_vbuf_t_;

typedef struct {
    nema_vbuf_t_ shape;  /**< Path shape */
    nema_matrix3x3_t m; /**< Path transformation matrix */
    uint32_t flags; /**< Path flags  */
} nema_vg_path_t;

/* Rounded rect points
*   A____________B
*   (            )
*  (  AH      BC  ) C
* H|              |
*  |              |
*  |              |
*  |              |
* G|              |D
*  ( FG        DE )
*  F(____________)E
*/
typedef struct {
    nema_vg_vertex_t_ A;
    nema_vg_vertex_t_ B;
    nema_vg_vertex_t_ C;
    nema_vg_vertex_t_ D;
    nema_vg_vertex_t_ E;
    nema_vg_vertex_t_ F;
    nema_vg_vertex_t_ G;
    nema_vg_vertex_t_ H;

    nema_vg_vertex_t_ AH;
    nema_vg_vertex_t_ AH_ctrl0;
    nema_vg_vertex_t_ AH_ctrl1;
    nema_vg_vertex_t_ BC;
    nema_vg_vertex_t_ BC_ctrl0;
    nema_vg_vertex_t_ BC_ctrl1;
    nema_vg_vertex_t_ DE;
    nema_vg_vertex_t_ DE_ctrl0;
    nema_vg_vertex_t_ DE_ctrl1;
    nema_vg_vertex_t_ FG;
    nema_vg_vertex_t_ FG_ctrl0;
    nema_vg_vertex_t_ FG_ctrl1;
} rounded_rect_points;

/*
* the rect shape points.
* These are for the stroked points.
* if we have fill instead, use the *up_* points
*/
typedef struct {
    nema_vg_vertex_t_ p1_up_left;
    nema_vg_vertex_t_ p1_down_right;

    nema_vg_vertex_t_ p2_up_right;
    nema_vg_vertex_t_ p2_down_left;

    nema_vg_vertex_t_ p3_up_left;
    nema_vg_vertex_t_ p3_down_right;

    nema_vg_vertex_t_ p4_up_right;
    nema_vg_vertex_t_ p4_down_left;
}rect_points;

/*
* the ellipse/circle points
* Basically these are 4 cubic beziers
*/
typedef struct {
    nema_vg_vertex_t_ right_point;
    nema_vg_vertex_t_ right_down_ctrl0_point;
    nema_vg_vertex_t_ right_down_ctrl1_point;
    nema_vg_vertex_t_ down_point;
    nema_vg_vertex_t_ down_left_ctrl1_point;
    nema_vg_vertex_t_ down_left_ctrl0_point;
    nema_vg_vertex_t_ left_point;
    nema_vg_vertex_t_ left_up_ctrl0_point;
    nema_vg_vertex_t_ left_up_ctrl1_point;
    nema_vg_vertex_t_ up_point;
    nema_vg_vertex_t_ up_right_ctrl1_point;
    nema_vg_vertex_t_ up_right_ctrl0_point;
}ellipse_points;

extern TLS_VAR nema_vg_context_t_* nemavg_context;
extern nema_vg_stencil_t_* stencil;

void context_reset(void);
// Acces context information
nema_vg_context_t_* nema_vg_get_context(void);

// Access stencil object
nema_vg_stencil_t_* nema_vg_get_stencil(void);

void nema_vg_set_error(uint32_t error);

nema_buffer_t stencil_buffer_create(int w, int h, int pool);
void stencil_buffer_set(int w, int h, nema_buffer_t bo);
void stencil_buffer_destroy(void);
void stencil_buffer_set_prealloc(void);
#ifndef NEMAPVG
void lut_buffer_create(void);
void lut_buffer_destroy(void);
// Access lut object
nema_buffer_t* nema_vg_get_lut(void);
#endif
uint32_t nema_vg_path_get_bbox(nema_vbuf_t_ *vb, nema_vg_float_t* xmin, nema_vg_float_t* ymin, nema_vg_float_t* xmax, nema_vg_float_t *ymax);

void set_raster_point(uint32_t point, int32_t xfx, int32_t yfx);
int vertex_2fx(float a);
uint32_t fill_cubic( nema_vg_info_t_* path_info);

void set_fan_center(float fanx, float fany);

uint32_t get_paint_type(NEMA_VG_PAINT_HANDLE paint);
float get_stroke_width(void);

 void fill_rounded_cap(const nema_vg_vertex_t_ *cap_center,
                       float radius, float angle_cos_theta, float angle_sin_theta,
                       uint8_t is_end);

#ifdef HAAS
void init_clip_from_context(void);
void init_clip(int32_t x_orig, int32_t y_orig, int32_t w_orig, int32_t h_orig);
#else
#define init_clip_from_context(...)
#define init_clip(...)
#endif

void reset_clip2_or_temp(int clip2);
void set_clip2_or_temp(int clip2, int32_t x, int32_t y, uint32_t w, uint32_t h);
void set_bezier_quality(uint8_t quality);
void reset_last_vertex(void);

uint8_t is_identity_matrix(nema_matrix3x3_t m);

// this function needs to be called to update
// some static variables inside nema_vg.c.
// m matrix will scale the stroke according to m scale
// Needs to be called if you call "nema_raster_line_, fill_cubic etc"
// Also gets as input the paint_color which might modulate.
// Returns a new paint color(modulated or the same as paint_color)
uint32_t set_stroked_line_width(float stroke, nema_matrix3x3_t m, uint32_t color);

// rasters a line by taking into account stroke width.
// stroke width must have been previously set with set_stroked_line_width
//make sure to call flush lines accordingly
void raster_stroked_line_(nema_vg_vertex_t_ p0,  nema_vg_vertex_t_ p1, int is_join);

// when we have stroke width greater than 1,
// line segments are drawn in a different way.
// For example, the first and last segments are not drawn
// until we call this function.
// seg is the current segment type. Only useful when it's NO_STROKE
void flush_lines_(int is_close, uint8_t seg);

// bind the corresponding texture according to paint
/*
***   WARNING!!! If radial is not supported, this function will change paint to fill type  ****
*/
void bind_tex1_according_paint(nema_vg_paint_t *paint);
// sets the paint matrix. conic_quad is ignored if paint is not set to conical gradient
void set_matrix_according_paint(nema_vg_paint_t *paint, nema_matrix3x3_t path_m, int conic_quad);

// enable drawing directkly on FB
void enable_draw_on_fb(int enable);

// enable using bevel joins
void enable_use_bevel_joins(int enable);

void enable_do_not_transform(uint8_t enable);

/*
* math functions. Definition in nema_vg_path.c
*/

void tsvg_init(void);
void tsvg_deinit(void);
void set_tsvg_text_max_size(uint32_t width, uint32_t height);

static inline int nema_floor_(float a){
    int a_i = (int)a;
    float a_if = (float)a_i;
    int a_i2 = 0;
    if (a_if > a){
        a_i2 = 1;

    }else{
        //MISRA
    }

    return a_i - a_i2;
}

static inline int nema_ceil_(float a){
    int a_i = (int)a;
    float a_if = (float)a_i;
    int a_i2 = 0;
    if (a_if < a){
        a_i2 = 1;

    }else{
        //MISRA
    }

    return a_i + a_i2;
}

static inline int nema_truncf_(float x){
    int res = (x < 0.0f) ? nema_ceil_(x) : nema_floor_(x);
    return res;
}

static inline float nema_fmod_(float x, float y){
    return (x - ((float)nema_truncf_(x / y) * y));
}

static inline float nema_abs_(float a){
    if(a < 0.f ) {
        return -a;
    } else {
        return a;
    }
}

/** \brief Get Fill rule
 *
 * \return fill_rule
 *
 */
uint8_t get_fill_rule(void);


/** \brief Set stroke width to be in screen space
 *
 *  stroke width will NOT be affected by transformation
 *  This is an internal function
 */
void enable_screen_space_stroking(void);

/** \brief Reset stroke width from screen space to object space
 *
 *  stroke width will be affected by transformation
 *  This is an internal function
 */
void disable_screen_space_stroking(void);

uint32_t draw_caps(void);

/** \brief Calculate the tight axis-aligned bounding box of the shape of the path
 *
 * \param path Pointer to Path
 * \return void
 *
 */
void calculate_aabb_per_shape(NEMA_VG_PATH_HANDLE path);

void reenable_global_matrix(void);

uint32_t draw_clipped_path(NEMA_VG_PATH_HANDLE path, NEMA_VG_PAINT_HANDLE paint);

/** \brief Calculate the segment and data count of the clipped geometry (path)
 *
 * \param path Pointer (handle) to the path that will be drawn
 * \param new_seg_size Pointer to the variable that is overwritten with the number of the segments of the clipped path
 * \param new_data_size Pointer to the variable that is overwritten with the number of the data of the clipped path
 *
 * \return Error code. See NEMA_VG_ERR_* defines in "nema_vg_context.h" header file for the error codes.
 */
uint32_t calc_clipped_path_size(NEMA_VG_PATH_HANDLE path, NEMA_VG_PAINT_HANDLE paint,
                                        size_t* new_seg_size,
                                        size_t* new_data_size);

/** \brief Calculate the clipped geometry (path)
 *
 *  \param path Pointer (handle) to the path that will be drawn
 *  \param paint Pointer (handle) to the paint object that wil be used for drawing
 *  \param new_seg_size Pointer to the variable with the count of segments of the clipped path (preferably calculated by calc_clipped_path_size)
 *  \param new_segs Pointer to the segment buffer that contains the clipped path segments (can be allocated by clipped_path_alloc)
 *  \param new_data_size Pointer to the variable with the count of data of the clipped path (preferably calculated by calc_clipped_path_size)
 *  \param new_data Pointer to the data buffer that contains the clipped path data (can be allocated by clipped_path_alloc)
 *
 *  \return Error code. See NEMA_VG_ERR_* defines in "nema_vg_context.h" header file for the error codes.
 */
 uint32_t calc_clipped_path(NEMA_VG_PATH_HANDLE path, NEMA_VG_PAINT_HANDLE paint,
                                    size_t *new_seg_size, uint8_t* new_segs,
                                    size_t *new_data_size, nema_vg_float_t* new_data);

/** \brief Convenience function: creates the buffers needed to store the clipped path
 * (segment and data buffers)
 *
 *  \param seg_size Segments size (count)
 *  \param seg_buffer Pointer to the segment buffer that should be created
 *  \param data_size Data size (count)
 *  \param data_buffer Pointer to the data buffer that should be created
 *
 * \return Error code. If no error occurs, NEMA_VG_ERR_NO_ERROR otherwise NEMA_VG_ERR_BAD_BUFFER
 */
uint32_t clipped_path_alloc(int seg_size, nema_buffer_t *seg_buffer, int data_size, nema_buffer_t *data_buffer);

/** \brief Convenience function: destroys the buffers allocated for the clipped path
 * (segment and data buffers)
 *
 *  \param seg_buffer Pointer to the segment buffer
 *  \param data_buffer Pointer to the data buffer
 *
 */
void clipped_path_free(nema_buffer_t *seg_buffer, nema_buffer_t *data_buffer);

void transform_arc( nema_vg_vertex_t_ *v, float cosine, float sine, float rx, float ry, float cx, float cy );

#ifndef ENABLE_CLIPPED_PATH2
#define ENABLE_CLIPPED_PATH2 0
#endif

#if ENABLE_CLIPPED_PATH2 != 0
uint32_t calc_clipped_path2(NEMA_VG_PATH_HANDLE path, NEMA_VG_PAINT_HANDLE paint,
                                   size_t *new_seg_size, uint8_t* new_segs,
                                   size_t *new_data_size, nema_vg_float_t* new_data,
                                   float min, float max);

uint32_t calc_clipped_path_size2(NEMA_VG_PATH_HANDLE path,
                                        size_t* new_seg_size, size_t* new_data_size,
                                        float min, float max);
#endif


#ifdef __cplusplus
}
#endif

#endif //NEMA_VG_P_H__
