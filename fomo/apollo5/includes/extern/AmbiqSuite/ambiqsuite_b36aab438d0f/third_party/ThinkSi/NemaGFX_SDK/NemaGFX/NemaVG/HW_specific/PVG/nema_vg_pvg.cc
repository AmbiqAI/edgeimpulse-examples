#ifndef NEMA_VG_PVG_CC__
#define NEMA_VG_PVG_CC__

#ifdef NEMAPVG

// even_odd and non_zero are the same in NemaPVG
#define fill_path_non_zero fill_path_even_odd


#ifdef HW_VERTEX_MMUL
inline static void set_vertex_matrix_(nema_matrix3x3_t m) {
    nemavg_context->info.had_transformation = nemavg_context->info.has_transformation;
    if ( nemavg_context->info.has_transformation != 0U ) {
        set_vertex_matrix(m);
        nemavg_context->info.has_transformation = 0U;
    }
}
#else
#define set_vertex_matrix_(...)
#endif

#ifdef HW_VERTEX_MMUL
inline static void revert_vertex_matrix_(void) {
    if ( nemavg_context->info.had_transformation != 0U ) {
        bypass_vertex_matrix();
        nemavg_context->info.has_transformation = nemavg_context->info.had_transformation;
        nemavg_context->info.had_transformation = 0U;
    }
}
#else
#define revert_vertex_matrix_(...)
#endif

void
set_raster_point(uint32_t point, int32_t xfx, int32_t yfx)
{
    if ( point > 3U ) {
        set_bezier_fan_fx_( xfx, yfx );
    }
}

inline static void
set_frag_ptr_non_zero_tiger(void) {
    const uint32_t add_frag_ptr = PREFETCH_TEXEL | PRE_T1 | PRE_XY | PRE_TEX[NEMA_TEX3] | SHADER_ADDR_ADD;
    const uint32_t sub_frag_ptr = PREFETCH_TEXEL | PRE_T1 | PRE_XY | PRE_TEX[NEMA_TEX3] | SHADER_ADDR_SUB;

    nema_cl_add_cmd(NEMA_CODEPTR, add_frag_ptr);
    nema_cl_add_cmd(0x1e0U      , sub_frag_ptr);

}

static inline void
line_non_zero(nema_vg_vertex_t_* p1, nema_vg_vertex_t_* p2)
{
    raster_triangle_fan_(p1->x, p1->y, p2->x, p2->y, 0, nemavg_context->aa_flags);
}

static inline void
line_even_odd(nema_vg_vertex_t_* p1, nema_vg_vertex_t_* p2)
{
    raster_triangle_fan_(p1->x, p1->y, p2->x, p2->y, 0, nemavg_context->aa_flags);
}

static inline void
line_draw(nema_vg_vertex_t_* p1, nema_vg_vertex_t_* p2)
{
    if ( nemavg_context->info.do_fat_stroke == 0 ) {
        raster_triangle_fan_(p1->x, p1->y, p2->x, p2->y, 1, nemavg_context->aa_flags);
    }
    else {
        // Need to do fat stroke and apply bevel
        // Use SW tessellation
        raster_stroked_line_(*p1, *p2, 1);
    }
}

uint32_t
fill_cubic(nema_vg_info_t_* path_info)
{
    int draw_ = 0;
    if ( nemavg_context->fill_rule == NEMA_VG_STROKE ) {
        if ( nemavg_context->info.do_fat_stroke == 1 ) {
            // Need to do fat stroke and apply bevel
            // Use SW tessellation
            stroke_cubic_decasteljau(path_info->p1,
                                     path_info->ctrl0,
                                     path_info->ctrl1,
                                     path_info->p2);
            return NEMA_VG_ERR_NO_ERROR;
        }

        draw_ = 1;
    }

    raster_bezier_cubic_(path_info, draw_, nemavg_context->aa_flags);
    return NEMA_VG_ERR_NO_ERROR;
}

static inline uint32_t
fill_quad(nema_vg_info_t_* path_info)
{
    int draw_ = 0;
    if ( nemavg_context->fill_rule == NEMA_VG_STROKE ) {
        if ( nemavg_context->info.do_fat_stroke == 1 ) {
            // Need to do fat stroke and apply bevel
            // Use SW tessellation
            stroke_quadratic_decasteljau(path_info->p1,
                                         path_info->ctrl0,
                                         path_info->p2);
            return NEMA_VG_ERR_NO_ERROR;
        }

        draw_ = 1;
    }

    raster_bezier_quad_(path_info->p1.x   , path_info->p1.y,
                        path_info->ctrl0.x, path_info->ctrl0.y,
                        path_info->p2.x   , path_info->p2.y, draw_, nemavg_context->aa_flags);

    return NEMA_VG_ERR_NO_ERROR;
}

/************************************************
 * Beziers are drawn with triangle fans
 * fan - p0 - bezier - p1
 * It's faster if you do the following:
 * 1. triangle fan->p0->p1
 * 2. bezier p0 - bezier - p1 (fan == p0)
 *
 * The other way would be to always draw bezier
 *   with fan center at original nemavg_context->info.fan_center.fanx0/nemavg_context->info.fan_center.fany0
 *************************************************/
static uint32_t
fill_path_non_zero(nema_vg_path_t* path) {
    PRINT_FUNC_ENTRY;

#if 0
    nemavg_context->info.fan_center.fanx0 = ((int32_t)stencil->dirty_area_p1.x+
                (int32_t)stencil->dirty_area_p2.x+
                (int32_t)stencil->dirty_area_p3.x+
                (int32_t)stencil->dirty_area_p4.x)/4;
    nemavg_context->info.fan_center.fany0 = ((int32_t)stencil->dirty_area_p1.y+
                (int32_t)stencil->dirty_area_p2.y+
                (int32_t)stencil->dirty_area_p3.y+
                (int32_t)stencil->dirty_area_p4.y)/4;
#else
    float fanx0f = (path->shape.bbox.transformed_min.x+path->shape.bbox.transformed_max.x)*0.5f;
    float fany0f = (path->shape.bbox.transformed_min.y+path->shape.bbox.transformed_max.y)*0.5f;
    nemavg_context->info.fan_center.fanx0 = (int)fanx0f;
    nemavg_context->info.fan_center.fany0 = (int)fany0f;
#endif


    nemavg_context->info.fan_center.fanx0fx = nema_i2fx(nemavg_context->info.fan_center.fanx0);
    nemavg_context->info.fan_center.fany0fx = nema_i2fx(nemavg_context->info.fan_center.fany0);

    set_bezier_fan_fx_(nemavg_context->info.fan_center.fanx0fx, nemavg_context->info.fan_center.fany0fx);

    nema_vg_info_t_ path_info = {0};
    path_info.no_move=1;
    path_info.no_close=1;
    path_info.new_path=1;

     nema_vbuf_t_ *vb = &path->shape;

    size_t idx_seg = 0;
    while (idx_seg < vb->seg_size) {

        uint8_t seg = (vb->seg)[idx_seg++];
        uint8_t seg_no_rel = (uint8_t)(seg & NEMA_VG_NO_REL_MASK); //remove rel from seg to avoid multi checking
        NEMA_VG_IF_ERROR(process_path_vertex(vb, seg, &path_info));

        if(path_info.new_path == 0U){
            if ( seg_no_rel == NEMA_VG_PRIM_BEZIER_CUBIC || seg_no_rel == NEMA_VG_PRIM_BEZIER_SCUBIC ) {
                // set_bezier_fan_(path_info.p1.x, path_info.p1.y);
                raster_bezier_cubic_pie_(nemavg_context->info.fan_center.fanx0fx, nemavg_context->info.fan_center.fany0fx, &path_info, nemavg_context->aa_flags);
                continue;
            }
            else if ( seg_no_rel == NEMA_VG_PRIM_LINE ) {
                raster_triangle_fffx(nemavg_context->info.fan_center.fanx0fx, nemavg_context->info.fan_center.fany0fx, &path_info.p1, &path_info.p2);
                continue;
            }
            else if ( seg_no_rel == NEMA_VG_PRIM_BEZIER_QUAD || seg_no_rel == NEMA_VG_PRIM_BEZIER_SQUAD ) {
                raster_bezier_quad_pie_(nemavg_context->info.fan_center.fanx0fx, nemavg_context->info.fan_center.fany0fx, &path_info, nemavg_context->aa_flags);
                continue;
            }
            else if ( (seg & NEMA_VG_PRIM_MASK) == NEMA_VG_PRIM_ARC ) {
                set_bezier_fan_(path_info.p1.x, path_info.p1.y);
                NEMA_VG_IF_ERROR(fill_arc(path, &path_info));
            }
            else if( seg_no_rel == NEMA_VG_PRIM_POLYGON || seg_no_rel == NEMA_VG_PRIM_POLYLINE ) {
                // The polygon/polyline segments are already handled inside process_path_vertex...
                continue;
            }
            else {
                // MISRA
            }

            // draw a triangle
            {
                // line_even_odd(&path_info.p1, &path_info.p2);
                raster_triangle_fffx(nemavg_context->info.fan_center.fanx0fx, nemavg_context->info.fan_center.fany0fx, &path_info.p1, &path_info.p2);
            }
        }
    }

    if(path_info.no_close == 1U){
        nema_vg_vertex_t_ implicit_p2 = find_implicit_vertices(&path_info);
        // line_even_odd(&path_info.p2, &implicit_p2);
        raster_triangle_fffx(nemavg_context->info.fan_center.fanx0fx, nemavg_context->info.fan_center.fany0fx, &path_info.p2, &implicit_p2);
    }

    return NEMA_VG_ERR_NO_ERROR;
}

static uint32_t
stroke_thin_path(nema_vg_path_t* path) {
    PRINT_FUNC_ENTRY;

    size_t idx_seg = 0;

    nema_vg_info_t_ path_info = {0};
    path_info.no_move = 1;
    path_info.new_path = 1;
     nema_vbuf_t_ *vb = &path->shape;

    while (idx_seg < vb->seg_size)  {
        uint8_t seg = (vb->seg)[idx_seg++];
        uint8_t seg_no_rel = (uint8_t)(seg & NEMA_VG_NO_REL_MASK); //remove rel from seg to avoid multi checking
        NEMA_VG_IF_ERROR(process_path_vertex(vb, seg, &path_info));

        if( (path_info.new_path == 0U) && ((seg & NEMA_VG_PRIM_NO_STROKE) == 0U) ) {
            if ( seg_no_rel == NEMA_VG_PRIM_BEZIER_QUAD || seg_no_rel == NEMA_VG_PRIM_BEZIER_SQUAD ) {
                NEMA_VG_IF_ERROR(fill_quad(&path_info));
            }
            else if ( seg_no_rel == NEMA_VG_PRIM_BEZIER_CUBIC || seg_no_rel == NEMA_VG_PRIM_BEZIER_SCUBIC ) {
                NEMA_VG_IF_ERROR(fill_cubic(&path_info));
            }
            else if ( (seg & NEMA_VG_PRIM_MASK) == NEMA_VG_PRIM_ARC ) {
                NEMA_VG_IF_ERROR(fill_arc(path, &path_info));
            }
            else if( seg_no_rel == NEMA_VG_PRIM_POLYGON || seg_no_rel == NEMA_VG_PRIM_POLYLINE ) {
                // The polygon/polyline segments are already handled inside process_path_vertex...
            }
            else {
                line_draw(&path_info.p1, &path_info.p2);
            }
        }
    }

    return NEMA_VG_ERR_NO_ERROR;
}


inline static uint32_t
stencil_draw(nema_vg_path_t* path, nema_vg_paint_t* paint)
{
    // TEX3 is bound as L8
    uint32_t error = NEMA_VG_ERR_NO_ERROR;

    uint32_t tex3_format = ((unsigned)0x2f << 24); // NonZero

    // Now draw the path
    switch (nemavg_context->fill_rule) {
        case NEMA_VG_STROKE: {
                uint32_t blending_mode = NEMA_BL_SRC | NEMA_BLOP_NO_USE_ROPBL;
                uint32_t raster_color  = 0xffffffffU;
                nema_set_blend(blending_mode, NEMA_TEX3, NEMA_NOTEX, NEMA_NOTEX);
                nema_set_raster_color(raster_color);
            if ( nemavg_context->info.do_fat_stroke == 0 ) {
                NEMA_VG_IF_ERROR(stroke_thin_path(path));
            }
            else {
                NEMA_VG_IF_ERROR(stroke_path(path));
            }
            nema_cl_add_cmd(NEMA_TEX3_FSTRIDE, ((unsigned)NEMA_A8 << 24) | ((unsigned)stencil->width & 0xffffU));
         }
            break;
        case NEMA_VG_FILL_EVEN_ODD:
            tex3_format = ((unsigned)0x2e << 24);
            set_frag_ptr_non_zero_tiger();
            nema_set_rop_blend_mode( NEMA_BL_SRC | 0x10000U /* (1U<<16) */ );
            NEMA_VG_IF_ERROR(fill_path_non_zero(path));
            nema_cl_add_cmd(NEMA_TEX3_FSTRIDE, tex3_format | ((unsigned)stencil->width & 0xffffU));
            break;
        case NEMA_VG_FILL_NON_ZERO:
            set_frag_ptr_non_zero_tiger();
            nema_set_rop_blend_mode( NEMA_BL_SRC | 0x10000U /* (1U<<16) */ );
            NEMA_VG_IF_ERROR(fill_path_non_zero(path));
            nema_cl_add_cmd(NEMA_TEX3_FSTRIDE, tex3_format | ((unsigned)stencil->width & 0xffffU));
            break;
        default:
            PRINT_F("%s: Invalid fill rule!!\n", __func__);
            error =  NEMA_VG_ERR_INVALID_FILL_RULE;
            nema_vg_set_error(error);
            break;
    }
    return error;
}

#endif // NEMAPVG

#endif // NEMA_VG_PVG_CC__
