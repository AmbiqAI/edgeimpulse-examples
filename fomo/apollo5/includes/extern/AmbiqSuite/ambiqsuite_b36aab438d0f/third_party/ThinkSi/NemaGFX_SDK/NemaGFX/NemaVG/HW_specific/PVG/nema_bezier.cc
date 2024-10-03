#ifndef NEMA_BEZIER_CC__
#define NEMA_BEZIER_CC__

#include "nema_regs.h"
#include "nema_vg_regs.h"
#include "nema_rasterizer_intern.h"

#ifndef HOLD
#define HOLD 0xff000000U
#endif

//-----------------------------

#define SHADER_ADDR_ADD 22U
#define SHADER_ADDR_SUB 23U

#define PVG_AA_FLAGS (RAST_AA_E0 | RAST_AA_E1 | RAST_AA_E2 | RAST_AA_E3)
#define PVG_IMR_AA_FLAGS ( RAST_AA_E0 )

static inline uint32_t
calc_aa_flags_for_bezier(int draw, uint32_t do_aa) {
    uint32_t aa_flags = 0U;

    // Thin stroke without AA
    if ( do_aa == 0U ) {
        aa_flags = 0U;
    }
    else if ( nemavg_context->info.do_draw_on_fb == 0  ) {
        aa_flags = draw == 0 ? PVG_AA_FLAGS : RAST_AA_MASK;
    }
    else {
        aa_flags = draw == 0 ? PVG_IMR_AA_FLAGS : RAST_AA_MASK;
    }

    aa_flags &= nemavg_context->aa_mask;

    if ( nema_context.en_tscFB == 1U ) {
        aa_flags |= (nema_context.surface_tile & RAST_TILE_MASK);
    }

    return aa_flags;
}

static void
bezier_init_shaders(void) {
    static const uint32_t cmd_add[] = {0x080c0182U, 0x8a9f5a24U};
    static const uint32_t cmd_sub[] = {0x080c0182U, 0x8a9f5a34U};

    // SHADER
    nema_load_frag_shader(&cmd_add[0], 1, SHADER_ADDR_ADD);
    nema_load_frag_shader(&cmd_sub[0], 1, SHADER_ADDR_SUB);
}

//-----------------------------
static inline void
set_bezier_fan_fx_( int32_t fan_x, int32_t fan_y ) {
    const int num_cmds = 2;

    uint32_t *cmd_array = nema_cl_get_space(num_cmds);
    if (cmd_array == NULL) {
        return;
    }

    int cmd_idx = 0;

    cmd_array[cmd_idx++] = NEMA_DRAW_FAN_X; cmd_array[cmd_idx++] = *(uint32_t *)&fan_x;
    cmd_array[cmd_idx++] = NEMA_DRAW_FAN_Y; cmd_array[cmd_idx++] = *(uint32_t *)&fan_y;

}

#ifndef CL_GET_SPACE_U
#define UNDEFINE_THIS
#define CL_GET_SPACE_U(size)                      \
        cmd_array = (nema_multi_union_t *)nema_cl_get_space(size);    \
        if (cmd_array == NULL) {                   \
            return;                             \
        }
#endif

inline static void
set_bezier_fan_( float fan_x, float fan_y ) {
    nema_multi_union_t *cmd_array;
    CL_GET_SPACE_U(2);

    int cmd_idx = 0;

    cmd_array[cmd_idx++].u = NEMA_DRAW_FAN_X_F; cmd_array[cmd_idx++].f = fan_x;
    cmd_array[cmd_idx++].u = NEMA_DRAW_FAN_Y_F; cmd_array[cmd_idx++].f = fan_y;
}

inline static void
nema_raster_quad_f_(nema_vg_vertex_t_ p0,
                    nema_vg_vertex_t_ p1,
                    nema_vg_vertex_t_ p2,
                    nema_vg_vertex_t_ p3)
{
    nema_multi_union_t *cmd_array;
    CL_GET_SPACE_U(9);

    cmd_array[ 0].u = NEMA_DRAW_PT0_X_F; cmd_array[ 1].f = p0.x;
    cmd_array[ 2].u = NEMA_DRAW_PT0_Y_F; cmd_array[ 3].f = p0.y;
    cmd_array[ 4].u = NEMA_DRAW_PT1_X_F; cmd_array[ 5].f = p1.x;
    cmd_array[ 6].u = NEMA_DRAW_PT1_Y_F; cmd_array[ 7].f = p1.y;
    cmd_array[ 8].u = NEMA_DRAW_PT2_X_F; cmd_array[ 9].f = p2.x;
    cmd_array[10].u = NEMA_DRAW_PT2_Y_F; cmd_array[11].f = p2.y;
    cmd_array[12].u = NEMA_DRAW_PT3_X_F; cmd_array[13].f = p3.x;
    cmd_array[14].u = NEMA_DRAW_PT3_Y_F; cmd_array[15].f = p3.y;
    cmd_array[16].u = NEMA_DRAW_CMD | HOLD  ;
    cmd_array[17].u = nema_context.draw_flags | DRAW_QUAD;
}

static inline void
raster_triangle_fffx(int32_t x0fx, int32_t y0fx, nema_vg_vertex_t_* p1, nema_vg_vertex_t_* p2)
{
    nema_multi_union_t *cmd_array;
    CL_GET_SPACE_U(7);

    uint32_t aa_flags = PVG_AA_FLAGS;
    aa_flags &= nemavg_context->aa_mask;

    if ( nema_context.en_tscFB == 1U ) {
        aa_flags |= (nema_context.surface_tile & RAST_TILE_MASK);
    }

    cmd_array[ 0].u = NEMA_DRAW_PT0_X_F; cmd_array[ 1].f = p1->x;
    cmd_array[ 2].u = NEMA_DRAW_PT0_Y_F; cmd_array[ 3].f = p1->y;
    cmd_array[ 4].u = NEMA_DRAW_PT1_X_F; cmd_array[ 5].f = p2->x;
    cmd_array[ 6].u = NEMA_DRAW_PT1_Y_F; cmd_array[ 7].f = p2->y;
    cmd_array[ 8].u = NEMA_DRAW_PT2_X  ; cmd_array[ 9].i = x0fx;
    cmd_array[10].u = NEMA_DRAW_PT2_Y  ; cmd_array[11].i = y0fx;
    cmd_array[12].u = NEMA_DRAW_CMD | HOLD;
    cmd_array[13].u = aa_flags | DRAW_TRIANGLE;
}

static inline void
raster_bezier_quad_( float x0,  float y0,
                     float cx,  float cy,
                     float x1,  float y1, int draw, uint32_t do_aa) {
    const int num_cmds = 3*2+1;

    uint32_t cmd = 6U;
    if ( draw != 0 ) {
        cmd = 8U;
    }

    uint32_t aa_flags = calc_aa_flags_for_bezier(draw, do_aa);

    nema_multi_union_t *cmd_array;
    CL_GET_SPACE_U(num_cmds);

    int cmd_idx = 0;

    cmd_array[cmd_idx++].u = NEMA_DRAW_PT0_X_F; cmd_array[cmd_idx++].f = x0;
    cmd_array[cmd_idx++].u = NEMA_DRAW_PT0_Y_F; cmd_array[cmd_idx++].f = y0;
    cmd_array[cmd_idx++].u = NEMA_DRAW_PT1_X_F; cmd_array[cmd_idx++].f = cx;
    cmd_array[cmd_idx++].u = NEMA_DRAW_PT1_Y_F; cmd_array[cmd_idx++].f = cy;
    cmd_array[cmd_idx++].u = NEMA_DRAW_PT2_X_F; cmd_array[cmd_idx++].f = x1;
    cmd_array[cmd_idx++].u = NEMA_DRAW_PT2_Y_F; cmd_array[cmd_idx++].f = y1;
    cmd_array[cmd_idx++].u = NEMA_DRAW_CMD_NOHOLD | HOLD;
    cmd_array[cmd_idx++].u = cmd | aa_flags; /*| RAST_AA_E1*/; // quadratic bezier
}


static inline void
raster_bezier_quad_pie_(
                    int32_t fanx0fx,
                    int32_t fany0fx,
                    nema_vg_info_t_* path_info,
                     uint32_t do_aa) {
    // this is only for filling
    // don't use when drawing
    // don't use whn drawing directly on FB

    const int num_cmds = 14;
    const uint32_t cmd = 6U;

    uint32_t aa_flags = do_aa != 0U ? PVG_AA_FLAGS : 0U; //calc_aa_flags_for_bezier(0, do_aa);

    if ( nema_context.en_tscFB == 1U ) {
        aa_flags |= (nema_context.surface_tile & RAST_TILE_MASK);
    }

    nema_multi_union_t *cmd_array;
    CL_GET_SPACE_U(num_cmds);

    int cmd_idx = 0;

    cmd_array[cmd_idx++].u = NEMA_DRAW_FAN_X_F; cmd_array[cmd_idx++].f = path_info->p1.x;
    cmd_array[cmd_idx++].u = NEMA_DRAW_FAN_Y_F; cmd_array[cmd_idx++].f = path_info->p1.y;

    cmd_array[cmd_idx++].u = NEMA_DRAW_PT0_X_F; cmd_array[cmd_idx++].f = path_info->p1.x;
    cmd_array[cmd_idx++].u = NEMA_DRAW_PT0_Y_F; cmd_array[cmd_idx++].f = path_info->p1.y;
    cmd_array[cmd_idx++].u = NEMA_DRAW_PT1_X_F; cmd_array[cmd_idx++].f = path_info->p2.x;
    cmd_array[cmd_idx++].u = NEMA_DRAW_PT1_Y_F; cmd_array[cmd_idx++].f = path_info->p2.y;
    cmd_array[cmd_idx++].u = NEMA_DRAW_PT2_X  ; cmd_array[cmd_idx++].i = fanx0fx;
    cmd_array[cmd_idx++].u = NEMA_DRAW_PT2_Y  ; cmd_array[cmd_idx++].i = fany0fx;
    cmd_array[cmd_idx++].u = NEMA_DRAW_CMD | HOLD;
    cmd_array[cmd_idx++].u = aa_flags | DRAW_TRIANGLE;

    // cmd_array[cmd_idx++].u = NEMA_DRAW_PT0_X_F; cmd_array[cmd_idx++].f = path_info->p1.x;
    // cmd_array[cmd_idx++].u = NEMA_DRAW_PT0_Y_F; cmd_array[cmd_idx++].f = path_info->p1.y;
    cmd_array[cmd_idx++].u = NEMA_DRAW_PT1_X_F; cmd_array[cmd_idx++].f = path_info->ctrl0.x;
    cmd_array[cmd_idx++].u = NEMA_DRAW_PT1_Y_F; cmd_array[cmd_idx++].f = path_info->ctrl0.y;
    cmd_array[cmd_idx++].u = NEMA_DRAW_PT2_X_F; cmd_array[cmd_idx++].f = path_info->p2.x;
    cmd_array[cmd_idx++].u = NEMA_DRAW_PT2_Y_F; cmd_array[cmd_idx++].f = path_info->p2.y;
    cmd_array[cmd_idx++].u = NEMA_DRAW_CMD_NOHOLD | HOLD;
    cmd_array[cmd_idx++].u = cmd | aa_flags; /*| RAST_AA_E1*/; // quadratic bezier
}


static inline void
raster_bezier_cubic_pie_(
                    int32_t fanx0fx,
                    int32_t fany0fx,
                    nema_vg_info_t_* path_info,
                     uint32_t do_aa) {
    // this is only for filling
    // don't use when drawing
    // don't use whn drawing directly on FB

    const int num_cmds = 4*2+1 + 7;
    const uint32_t cmd = 7U;
    uint32_t aa_flags = do_aa != 0U ? PVG_AA_FLAGS : 0U; //calc_aa_flags_for_bezier(0, do_aa);

    if ( nema_context.en_tscFB == 1U ) {
        aa_flags |= (nema_context.surface_tile & RAST_TILE_MASK);
    }

    nema_multi_union_t *cmd_array;
    CL_GET_SPACE_U(num_cmds);

    int cmd_idx = 0;

    cmd_array[cmd_idx++].u = NEMA_DRAW_FAN_X_F; cmd_array[cmd_idx++].f = path_info->p1.x;
    cmd_array[cmd_idx++].u = NEMA_DRAW_FAN_Y_F; cmd_array[cmd_idx++].f = path_info->p1.y;

    cmd_array[cmd_idx++].u = NEMA_DRAW_PT0_X_F; cmd_array[cmd_idx++].f = path_info->p1.x;
    cmd_array[cmd_idx++].u = NEMA_DRAW_PT0_Y_F; cmd_array[cmd_idx++].f = path_info->p1.y;
    cmd_array[cmd_idx++].u = NEMA_DRAW_PT1_X_F; cmd_array[cmd_idx++].f = path_info->p2.x;
    cmd_array[cmd_idx++].u = NEMA_DRAW_PT1_Y_F; cmd_array[cmd_idx++].f = path_info->p2.y;
    cmd_array[cmd_idx++].u = NEMA_DRAW_PT2_X  ; cmd_array[cmd_idx++].i = fanx0fx;
    cmd_array[cmd_idx++].u = NEMA_DRAW_PT2_Y  ; cmd_array[cmd_idx++].i = fany0fx;
    cmd_array[cmd_idx++].u = NEMA_DRAW_CMD | HOLD;
    cmd_array[cmd_idx++].u = aa_flags | DRAW_TRIANGLE;

    // cmd_array[cmd_idx++].u = NEMA_DRAW_PT0_X_F; cmd_array[cmd_idx++].f = path_info->p1.x;
    // cmd_array[cmd_idx++].u = NEMA_DRAW_PT0_Y_F; cmd_array[cmd_idx++].f = path_info->p1.y;
    cmd_array[cmd_idx++].u = NEMA_DRAW_PT1_X_F; cmd_array[cmd_idx++].f = path_info->ctrl0.x;
    cmd_array[cmd_idx++].u = NEMA_DRAW_PT1_Y_F; cmd_array[cmd_idx++].f = path_info->ctrl0.y;
    cmd_array[cmd_idx++].u = NEMA_DRAW_PT2_X_F; cmd_array[cmd_idx++].f = path_info->ctrl1.x;
    cmd_array[cmd_idx++].u = NEMA_DRAW_PT2_Y_F; cmd_array[cmd_idx++].f = path_info->ctrl1.y;
    cmd_array[cmd_idx++].u = NEMA_DRAW_PT3_X_F; cmd_array[cmd_idx++].f = path_info->p2.x;
    cmd_array[cmd_idx++].u = NEMA_DRAW_PT3_Y_F; cmd_array[cmd_idx++].f = path_info->p2.y;
    cmd_array[cmd_idx++].u = NEMA_DRAW_CMD_NOHOLD | HOLD;
    cmd_array[cmd_idx++].u = cmd | aa_flags; /*| RAST_AA_E1*/; // quadratic bezier
}

static inline void
raster_bezier_cubic_(nema_vg_info_t_* path_info,
                     int draw, uint32_t do_aa) {
    const int num_cmds = 4*2+1;

    uint32_t cmd = 7U;
    if ( draw != 0 ) {
        cmd = 9U;
    }

    uint32_t aa_flags = calc_aa_flags_for_bezier(draw, do_aa);

    nema_multi_union_t *cmd_array;
    CL_GET_SPACE_U(num_cmds);

    int cmd_idx = 0;

    cmd_array[cmd_idx++].u = NEMA_DRAW_PT0_X_F; cmd_array[cmd_idx++].f = path_info->p1.x;
    cmd_array[cmd_idx++].u = NEMA_DRAW_PT0_Y_F; cmd_array[cmd_idx++].f = path_info->p1.y;
    cmd_array[cmd_idx++].u = NEMA_DRAW_PT1_X_F; cmd_array[cmd_idx++].f = path_info->ctrl0.x;
    cmd_array[cmd_idx++].u = NEMA_DRAW_PT1_Y_F; cmd_array[cmd_idx++].f = path_info->ctrl0.y;
    cmd_array[cmd_idx++].u = NEMA_DRAW_PT2_X_F; cmd_array[cmd_idx++].f = path_info->ctrl1.x;
    cmd_array[cmd_idx++].u = NEMA_DRAW_PT2_Y_F; cmd_array[cmd_idx++].f = path_info->ctrl1.y;
    cmd_array[cmd_idx++].u = NEMA_DRAW_PT3_X_F; cmd_array[cmd_idx++].f = path_info->p2.x;
    cmd_array[cmd_idx++].u = NEMA_DRAW_PT3_Y_F; cmd_array[cmd_idx++].f = path_info->p2.y;
    cmd_array[cmd_idx++].u = NEMA_DRAW_CMD_NOHOLD | HOLD;
    cmd_array[cmd_idx++].u = cmd | aa_flags; /*| RAST_AA_E1*/; // quadratic bezier
}

static inline void
raster_triangle_fan_(float x0,  float y0,
                     float x1,  float y1, int draw, uint32_t do_aa) {
    const int num_cmds = 2*2+1;

    uint32_t cmd = 0xbU;
    if ( draw != 0 ) {
        cmd = 0xaU;
    }

    uint32_t aa_flags = calc_aa_flags_for_bezier(draw, do_aa);

    nema_multi_union_t *cmd_array;
    CL_GET_SPACE_U(num_cmds);

    int cmd_idx = 0;

    cmd_array[cmd_idx++].u = NEMA_DRAW_PT0_X_F; cmd_array[cmd_idx++].f = x0;
    cmd_array[cmd_idx++].u = NEMA_DRAW_PT0_Y_F; cmd_array[cmd_idx++].f = y0;
    cmd_array[cmd_idx++].u = NEMA_DRAW_PT1_X_F; cmd_array[cmd_idx++].f = x1;
    cmd_array[cmd_idx++].u = NEMA_DRAW_PT1_Y_F; cmd_array[cmd_idx++].f = y1;
    cmd_array[cmd_idx++].u = NEMA_DRAW_CMD_NOHOLD | HOLD;
    cmd_array[cmd_idx++].u = cmd | aa_flags; /*| RAST_AA_E1*/; // quadratic bezier
}

static void
set_vertex_matrix(nema_matrix3x3_t m)
{
    int i = 0;

    nema_multi_union_t *cmd_array;
    // #define EPSILON     (0.0000001f)
    // #define COMPARE_2_ZERO(a) ((a) < (EPSILON) && (a) > -(EPSILON) )
    // if ( COMPARE_2_ZERO(m[2][0]) && COMPARE_2_ZERO(m[2][1]) ) {
    //     if ( !COMPARE_2_ZERO(m[2][0] - 1.0f) ) {
    //         m[0][0] /= m[2][2];
    //         m[0][1] /= m[2][2];
    //         m[0][2] /= m[2][2];
    //         m[1][0] /= m[2][2];
    //         m[1][1] /= m[2][2];
    //         m[1][2] /= m[2][2];
    //     }

        CL_GET_SPACE_U(7);

        cmd_array[i++].u = NEMA_MMV12; cmd_array[i++].f = m[1][2];
        cmd_array[i++].u = NEMA_MMV02; cmd_array[i++].f = m[0][2];
    // } else {
    //     CL_GET_SPACE_U(9);

    //     cmd_array[i++].u = NEMA_MMV12; cmd_array[i++].f = m[1][2];
    //     cmd_array[i++].u = NEMA_MMV02; cmd_array[i++].f = m[0][2];

    //     cmd_array[i++].u = NEMA_MMV20; cmd_array[i++].f = m[2][0];
    //     cmd_array[i++].u = NEMA_MMV21; cmd_array[i++].f = m[2][1];
    //     cmd_array[i++].u = NEMA_MMV22; cmd_array[i++].f = m[2][2];
    // }
    // #undef EPSILON
    // #undef COMPARE_2_ZERO

    cmd_array[i++].u = NEMA_MMV00; cmd_array[i++].f = m[0][0];
    cmd_array[i++].u = NEMA_MMV01; cmd_array[i++].f = m[0][1];
    cmd_array[i++].u = NEMA_MMV10; cmd_array[i++].f = m[1][0];
    cmd_array[i++].u = NEMA_MMV11; cmd_array[i++].f = m[1][1];
    // enable VMM and Rast Tess
    cmd_array[i++].u = 0x388U    ; cmd_array[i++].u = 0U;

    // nema_cl_add_cmd(0x118U, 0x90000000U);
}

static void bypass_vertex_matrix(void)
{
    // disable VMM and Rast Tess
    nema_cl_add_cmd(0x388U, 0x1U);
}



#ifdef UNDEFINE_THIS
#undef UNDEFINE_THIS
#undef CL_GET_SPACE_U
#endif

#endif // NEMA_BEZIER_CC__
