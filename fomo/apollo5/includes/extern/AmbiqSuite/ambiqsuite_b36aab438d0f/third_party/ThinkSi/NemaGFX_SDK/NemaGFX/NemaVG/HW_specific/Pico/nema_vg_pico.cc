#ifndef NEMA_VG_PICO_CC__
#define NEMA_VG_PICO_CC__

#ifndef NEMAPVG

#define set_vertex_matrix_(...)
#define revert_vertex_matrix_(...)

#define SHADER_ADDR_SUB 7U

// must be power of 2
#define ADD_COLOR 0x10101010U

// HAAS is disabled by default
#ifndef HAAS_THRESHOLD
#define HAAS_THRESHOLD 1.f //1 pixel
#endif

#ifndef HOLD
#define HOLD 0xff000000U
#endif

#ifdef HAAS
static int update_clip(uint32_t *cmd_array, int32_t cmd_idx, int32_t clip_x, int32_t clip_y, int32_t clip_x1, int32_t clip_y1) {
    int32_t clip_x_max = nemavg_context->info.clip_x_orig + nemavg_context->info.clip_w_orig;
    int32_t clip_y_max = nemavg_context->info.clip_y_orig + nemavg_context->info.clip_h_orig;
    if ( clip_x < nemavg_context->info.clip_x_orig ) {
        clip_x = nemavg_context->info.clip_x_orig;
    } else if ( clip_x > clip_x_max ) {
        clip_x = clip_x_max;
    } else{
        //MISRA
    }

    if ( clip_x1 < nemavg_context->info.clip_x_orig ) {
        clip_x1 = nemavg_context->info.clip_x_orig;
    } else if ( clip_x1 > clip_x_max ) {
        clip_x1 = clip_x_max;
    } else{
        //MISRA
    }

    if ( clip_y < nemavg_context->info.clip_y_orig ) {
        clip_y = nemavg_context->info.clip_y_orig;
    } else if ( clip_y > clip_y_max) {
        clip_y = clip_y_max;
    } else{
        //MISRA
    }

    if ( clip_y1 < nemavg_context->info.clip_y_orig ) {
        clip_y1 = nemavg_context->info.clip_y_orig;
    } else if ( clip_y1 > clip_y_max) {
        clip_y1 = clip_y_max;
    } else{
        //MISRA
    }

    cmd_array[cmd_idx++] = NEMA_CLIPMIN;
    cmd_array[cmd_idx++] = ((unsigned)clip_y   << 16) | ((unsigned)clip_x  & 0xffffU);
    cmd_array[cmd_idx++] = NEMA_CLIPMAX;
    cmd_array[cmd_idx++] = ((unsigned)clip_y1  << 16) | ((unsigned)clip_x1  & 0xffffU);

    return cmd_idx;
}

static int reset_clip(uint32_t *cmd_array, int32_t cmd_idx) {
    int32_t clip_x_max = nemavg_context->info.clip_x_orig + nemavg_context->info.clip_w_orig;
    int32_t clip_y_max = nemavg_context->info.clip_y_orig + nemavg_context->info.clip_h_orig;
    int32_t clip_x     = nemavg_context->info.clip_x_orig;
    int32_t clip_y     = nemavg_context->info.clip_y_orig;
    int32_t clip_x1    = clip_x_max;
    int32_t clip_y1    = clip_y_max;

    if ( clip_x < 0 ) {
        clip_x = 0;
    } else if ( clip_x > stencil->width ) {
        clip_x = stencil->width;
    } else{
        //MISRA
    }

    if ( clip_x1 < 0 ) {
        clip_x1 = 0;
    } else if ( clip_x1 > stencil->width ) {
        clip_x1 = stencil->width;
    } else{
        //MISRA
    }

    if ( clip_y < 0 ) {
        clip_y = 0;
    } else if ( clip_y > stencil->height ) {
        clip_y = stencil->height;
    } else{
        //MISRA
    }

    if ( clip_y1 < 0 ) {
        clip_y1 = 0;
    } else if ( clip_y1 > stencil->height ) {
        clip_y1 = stencil->height;
    } else{
        //MISRA
    }

    cmd_array[cmd_idx++] = NEMA_CLIPMIN;
    cmd_array[cmd_idx++] = ((unsigned)clip_y   << 16) | ((unsigned)clip_x  & 0xffffU);
    cmd_array[cmd_idx++] = NEMA_CLIPMAX;
    cmd_array[cmd_idx++] = ((unsigned)clip_y1  << 16) | ((unsigned)clip_x1  & 0xffffU);


    return cmd_idx;
}

static void compute_aa_edge_clip(float dx, float dy, int32_t clip_min_x, int32_t clip_min_y, int32_t clip_max_x, int32_t clip_max_y, int32_t *clip_x, int32_t *clip_y, int32_t *clip_x1, int32_t *clip_y1) {
    int32_t clip_x_max = nemavg_context->info.clip_x_orig + nemavg_context->info.clip_w_orig;
    int32_t clip_y_max = nemavg_context->info.clip_y_orig + nemavg_context->info.clip_h_orig;
    if ( dx > dy ) {
        // horizontal
        *clip_x  = clip_min_x;
        *clip_y  = nemavg_context->info.clip_y_orig;
        *clip_x1 = clip_max_x;
        *clip_y1 = clip_y_max;
    } else {
        // vertical
        *clip_x  = nemavg_context->info.clip_x_orig;
        *clip_y  = clip_min_y;
        *clip_x1 = clip_x_max;
        *clip_y1 = clip_max_y;
    }
}

static int clip_non_aa_edges(uint32_t *cmd_array, int32_t cmd_idx, float dx, float dy, int32_t clip_min_x, int32_t clip_min_y, int32_t clip_max_x, int32_t clip_max_y) {
    uint8_t needs_clipping = 0U;
    int32_t clip_x  = 0;
    int32_t clip_y  = 0;
    int32_t clip_x1 = 0;
    int32_t clip_y1 = 0;

    int32_t clip_x_max = nemavg_context->info.clip_x_orig + nemavg_context->info.clip_w_orig;
    int32_t clip_y_max = nemavg_context->info.clip_y_orig + nemavg_context->info.clip_h_orig;
    if ( dx > dy ) {
        // horizontal
        clip_y  = nemavg_context->info.clip_y_orig;
        clip_y1 = clip_y_max;
        if (nemavg_context->info.fan_center.fanx0_f < (float)clip_min_x) {
            clip_x  = (int)nemavg_context->info.fan_center.fanx0_f;
            clip_x1 = clip_min_x;
            needs_clipping = 1U;
        } else if (nemavg_context->info.fan_center.fanx0_f >= (float)clip_max_x) {
            clip_x  = clip_max_x;
            clip_x1 = nema_ceil_(nemavg_context->info.fan_center.fanx0_f);
            needs_clipping = 1U;
        } else{
            //MISRA
        }
    } else {
        // vertical
        clip_x  = nemavg_context->info.clip_x_orig;
        clip_x1 = clip_x_max;
        if (nemavg_context->info.fan_center.fany0_f < (float)clip_min_y) {
            clip_y  = (int)nemavg_context->info.fan_center.fany0_f;
            clip_y1 = clip_min_y;
            needs_clipping = 1U;
        } else if (nemavg_context->info.fan_center.fany0_f >= (float)clip_max_y) {
            clip_y  = clip_max_y;
            clip_y1 = nema_ceil_(nemavg_context->info.fan_center.fany0_f);
            needs_clipping = 1U;
        } else{
            //MISRA
        }
    }

    if ( needs_clipping == 1U ) {
        cmd_idx = update_clip(cmd_array, cmd_idx, clip_x, clip_y, clip_x1, clip_y1);

        cmd_array[cmd_idx++] = NEMA_DRAW_CMD_NOHOLD | HOLD;
        cmd_array[cmd_idx++] = DRAW_TRIANGLE;
    }

    return cmd_idx;
}

static inline int extra_clipping_commands(float dx, float dy, int clip_min_x, int clip_min_y, int clip_max_x, int clip_max_y) {
//some cases require 3 additional commands (2 for clip and 1 for draw triangle)
    if ( dx > dy ) {
        if ((nemavg_context->info.fan_center.fanx0_f < (float)clip_min_x) ||  (nemavg_context->info.fan_center.fanx0_f >= (float)clip_max_x) ) {
            return 3;
        }
    } else {
        if ((nemavg_context->info.fan_center.fany0_f < (float)clip_min_y) || (nemavg_context->info.fan_center.fany0_f >= (float)clip_max_y) ) {
            return 3;
        }
    }

    return 0;
}

#endif //HAAS


inline static void
nema_raster_quad_f_(nema_vg_vertex_t_ p0,
                    nema_vg_vertex_t_ p1,
                    nema_vg_vertex_t_ p2,
                    nema_vg_vertex_t_ p3)
{
    nema_raster_quad_f( p0.x, p0.y,
                        p1.x, p1.y,
                        p2.x, p2.y,
                        p3.x, p3.y);
}

inline static int
set_raster_point_inline_(uint32_t *cmd_array, uint32_t point, int32_t xfx, int32_t yfx)
{
    int cmd_idx = 0;
    cmd_array[cmd_idx++] = NEMA_DRAW_PT0_X+0x10U*point;
    cmd_array[cmd_idx++] = *(uint32_t *)&xfx;
    cmd_array[cmd_idx++] = NEMA_DRAW_PT0_Y+0x10U*point;
    cmd_array[cmd_idx++] = *(uint32_t *)&yfx;
    return cmd_idx;
}

void
set_raster_point(uint32_t point, int32_t xfx, int32_t yfx)
{
    const int num_cmds = 2;
    if ( point > 3U ) {
        point = 0;
    }

    uint32_t *cmd_array = nema_cl_get_space(num_cmds);
    if (cmd_array == NULL) {
        return;
    }
    int cmd_idx = 0;
    cmd_array[cmd_idx++] = NEMA_DRAW_PT0_X+0x10U*point;
    cmd_array[cmd_idx++] = *(uint32_t *)&xfx;
    cmd_array[cmd_idx++] = NEMA_DRAW_PT0_Y+0x10U*point;
    cmd_array[cmd_idx++] = *(uint32_t *)&yfx;
}

static void
nema_raster_quad_only123_(nema_vg_float_t x1, nema_vg_float_t y1, nema_vg_float_t x2, nema_vg_float_t y2, nema_vg_float_t x3, nema_vg_float_t y3)
{
    int x1fx = nema_f2fx(x1);
    int y1fx = nema_f2fx(y1);
    int x2fx = nema_f2fx(x2);
    int y2fx = nema_f2fx(y2);
    int x3fx = nema_f2fx(x3);
    int y3fx = nema_f2fx(y3);

    int num_cmds = 3*2+2;

    #ifdef HAAS

    float dx_1 = x1-nemavg_context->info.fan_center.fanx0_f;
    float dy_1 = y1-nemavg_context->info.fan_center.fany0_f;
    float dx_2 = x2-nemavg_context->info.fan_center.fanx0_f;
    float dy_2 = y2-nemavg_context->info.fan_center.fany0_f;
    float dx_3 = x3-nemavg_context->info.fan_center.fanx0_f;
    float dy_3 = y3-nemavg_context->info.fan_center.fany0_f;

    float dx_4 = x2-x1;
    float dy_4 = y2-y1;
    float dx_5 = x3-x2;
    float dy_5 = y3-y2;

    float a = nema_sqrt(dx_1*dx_1 + dy_1*dy_1);
    float b = nema_sqrt(dx_2*dx_2 + dy_2*dy_2);
    float c = nema_sqrt(dx_3*dx_3 + dy_3*dy_3);
    float d = nema_sqrt(dx_4*dx_4 + dy_4*dy_4);
    float e = nema_sqrt(dx_5*dx_5 + dy_5*dy_5);

    float edge_diff_1 = nema_absf( nema_absf(a-b) - d );
    float edge_diff_2 = nema_absf( nema_absf(b-c) - e );

    uint8_t haas_active  = 0U;

    if ( ( (nema_absf(dx_1) < HAAS_THRESHOLD) || (nema_absf(dx_2) < HAAS_THRESHOLD)) && ((nema_absf(dx_1 - dx_2) < HAAS_THRESHOLD)  || (nema_absf(dy_1 - dy_2) < HAAS_THRESHOLD)) ) {
            haas_active = 1U;
    } else if ( ( (nema_absf(dx_2) < HAAS_THRESHOLD) || (nema_absf(dx_3) < HAAS_THRESHOLD)) && ((nema_absf(dx_2 - dx_3) < HAAS_THRESHOLD)  || (nema_absf(dy_2 - dy_3) < HAAS_THRESHOLD)) ) {
        haas_active = 1U;
    } else if ( edge_diff_1 < HAAS_THRESHOLD ) {
        haas_active = 1U;
    } else if ( edge_diff_2 < HAAS_THRESHOLD ) {
        haas_active = 1U;
    } else {
        //MISRA
    }

    float dx  = nema_absf(x1 - x2);
    float dy  = nema_absf(y1 - y2);
    float dx2 = nema_absf(x2 - x3);
    float dy2 = nema_absf(y2 - y3);

    nema_vg_float_t clip_min_fx =  nema_min2(x1, x2);
    nema_vg_float_t clip_min_fy =  nema_min2(y1, y2);
    nema_vg_float_t clip_max_fx =  nema_max2(x1, x2) + 0.99f;
    nema_vg_float_t clip_max_fy =  nema_max2(y1, y2) + 0.99f;

    int32_t clip_min_x = (int32_t) clip_min_fx;
    int32_t clip_min_y = (int32_t) clip_min_fy;
    int32_t clip_max_x = (int32_t) clip_max_fx;
    int32_t clip_max_y = (int32_t) clip_max_fy;

    nema_vg_float_t clip_min_fx2 = nema_min2(x2, x3);
    nema_vg_float_t clip_min_fy2 = nema_min2(y2, y3);
    nema_vg_float_t clip_max_fx2 = nema_max2(x2, x3) + 0.99f;
    nema_vg_float_t clip_max_fy2 = nema_max2(y2, y3) + 0.99f;

    int32_t clip_min_x2 = (int32_t) clip_min_fx2;
    int32_t clip_min_y2 = (int32_t) clip_min_fy2;
    int32_t clip_max_x2 = (int32_t) clip_max_fx2;
    int32_t clip_max_y2 = (int32_t) clip_max_fy2;

    int32_t clip_x  = 0;
    int32_t clip_y  = 0;
    int32_t clip_x1 = 0;
    int32_t clip_y1 = 0;

    if ( haas_active == 1U) {
        compute_aa_edge_clip(dx, dy, clip_min_x, clip_min_y, clip_max_x, clip_max_y, &clip_x, &clip_y, &clip_x1, &clip_y1);
        num_cmds += 6;

        num_cmds += extra_clipping_commands(dx , dy , clip_min_x , clip_min_y , clip_max_x , clip_max_y );
        num_cmds += extra_clipping_commands(dx2, dy2, clip_min_x2, clip_min_y2, clip_max_x2, clip_max_y2);
    }

    #endif //HAAS

    uint32_t *cmd_array = nema_cl_get_space(num_cmds);
    if (cmd_array == NULL) {
        return;
    }

    int cmd_idx = 0;
    #ifdef HAAS
    if ( haas_active == 1U) {
        cmd_idx = update_clip(cmd_array, cmd_idx, clip_x, clip_y, clip_x1, clip_y1);
    }
    #endif //HAAS

    cmd_array[cmd_idx++] = NEMA_DRAW_PT2_X; cmd_array[cmd_idx++] = *(uint32_t*)&x1fx;
    cmd_array[cmd_idx++] = NEMA_DRAW_PT2_Y; cmd_array[cmd_idx++] = *(uint32_t*)&y1fx;
    cmd_array[cmd_idx++] = NEMA_DRAW_PT1_X; cmd_array[cmd_idx++] = *(uint32_t*)&x2fx;
    cmd_array[cmd_idx++] = NEMA_DRAW_PT1_Y; cmd_array[cmd_idx++] = *(uint32_t*)&y2fx;

    cmd_array[cmd_idx++] = NEMA_DRAW_CMD_NOHOLD | HOLD;
    cmd_array[cmd_idx++] = DRAW_TRIANGLE | nemavg_context->aa_flags;

    #ifdef HAAS
    if ( haas_active == 1U) {
        cmd_idx = clip_non_aa_edges(cmd_array, cmd_idx, dx, dy, clip_min_x, clip_min_y, clip_max_x, clip_max_y);

        compute_aa_edge_clip(dx2, dy2, clip_min_x2, clip_min_y2, clip_max_x2, clip_max_y2, &clip_x, &clip_y, &clip_x1, &clip_y1);
        cmd_idx = update_clip(cmd_array, cmd_idx, clip_x, clip_y, clip_x1, clip_y1);
    }
    #endif //HAAS

    cmd_array[cmd_idx++] = NEMA_DRAW_PT2_X; cmd_array[cmd_idx++] = *(uint32_t*)&x3fx;
    cmd_array[cmd_idx++] = NEMA_DRAW_PT2_Y; cmd_array[cmd_idx++] = *(uint32_t*)&y3fx;

    cmd_array[cmd_idx++] = NEMA_DRAW_CMD_NOHOLD | HOLD;
    cmd_array[cmd_idx++] = DRAW_TRIANGLE | nemavg_context->aa_flags;

    #ifdef HAAS
    if ( haas_active == 1U) {
        cmd_idx = clip_non_aa_edges(cmd_array, cmd_idx, dx2, dy2, clip_min_x2, clip_min_y2, clip_max_x2, clip_max_y2);
        //Reset clip
        (void) reset_clip(cmd_array, cmd_idx);
    }
    #endif //HAAS
    nemavg_context->info.last_vertex = 2;
}

static void
nema_raster_quad_only23_(nema_vg_float_t x1, nema_vg_float_t y1, nema_vg_float_t x2, nema_vg_float_t y2, nema_vg_float_t x3, nema_vg_float_t y3)
{
    int x2fx = nema_f2fx(x2);
    int y2fx = nema_f2fx(y2);
    int x3fx = nema_f2fx(x3);
    int y3fx = nema_f2fx(y3);

    int num_cmds = 2*2+2;

    #ifdef HAAS
    float dx  = nema_absf(x1 - x2);
    float dy  = nema_absf(y1 - y2);
    float dx2 = nema_absf(x2 - x3);
    float dy2 = nema_absf(y2 - y3);

    nema_vg_float_t clip_min_fx =  nema_min2(x1, x2);
    nema_vg_float_t clip_min_fy =  nema_min2(y1, y2);
    nema_vg_float_t clip_max_fx =  nema_max2(x1, x2) + 0.99f;
    nema_vg_float_t clip_max_fy =  nema_max2(y1, y2) + 0.99f;

    int32_t clip_min_x = (int32_t) clip_min_fx;
    int32_t clip_min_y = (int32_t) clip_min_fy;
    int32_t clip_max_x = (int32_t) clip_max_fx;
    int32_t clip_max_y = (int32_t) clip_max_fy;

    nema_vg_float_t clip_min_fx2 = nema_min2(x2, x3);
    nema_vg_float_t clip_min_fy2 = nema_min2(y2, y3);
    nema_vg_float_t clip_max_fx2 = nema_max2(x2, x3) + 0.99f;
    nema_vg_float_t clip_max_fy2 = nema_max2(y2, y3) + 0.99f;

    int32_t clip_min_x2 = (int32_t) clip_min_fx2;
    int32_t clip_min_y2 = (int32_t) clip_min_fy2;
    int32_t clip_max_x2 = (int32_t) clip_max_fx2;
    int32_t clip_max_y2 = (int32_t) clip_max_fy2;

    int32_t clip_x  = 0;
    int32_t clip_y  = 0;
    int32_t clip_x1 = 0;
    int32_t clip_y1 = 0;

    float dx_1 = x1-nemavg_context->info.fan_center.fanx0_f;
    float dy_1 = y1-nemavg_context->info.fan_center.fany0_f;
    float dx_2 = x2-nemavg_context->info.fan_center.fanx0_f;
    float dy_2 = y2-nemavg_context->info.fan_center.fany0_f;
    float dx_3 = x3-nemavg_context->info.fan_center.fanx0_f;
    float dy_3 = y3-nemavg_context->info.fan_center.fany0_f;

    float dx_4 = x2-x1;
    float dy_4 = y2-y1;
    float dx_5 = x3-x2;
    float dy_5 = y3-y2;

    float a = nema_sqrt(dx_1*dx_1 + dy_1*dy_1);
    float b = nema_sqrt(dx_2*dx_2 + dy_2*dy_2);
    float c = nema_sqrt(dx_3*dx_3 + dy_3*dy_3);
    float d = nema_sqrt(dx_4*dx_4 + dy_4*dy_4);
    float e = nema_sqrt(dx_5*dx_5 + dy_5*dy_5);

    float edge_diff_1 = nema_absf(nema_absf(a-b) - d);
    float edge_diff_2 = nema_absf(nema_absf(b-c) - e);


    uint8_t haas_active  = 0U;

    if ( ( (nema_absf(dx_1) < HAAS_THRESHOLD) || (nema_absf(dx_2) < HAAS_THRESHOLD)) && ((nema_absf(dx_1 - dx_2) < HAAS_THRESHOLD)  || (nema_absf(dy_1 - dy_2) < HAAS_THRESHOLD)) ) {
            haas_active = 1U;
    } else if (( ( (nema_absf(dx_2) < HAAS_THRESHOLD) || (nema_absf(dx_3) < HAAS_THRESHOLD))) && ((nema_absf(dx_2 - dx_3) < HAAS_THRESHOLD)  || (nema_absf(dy_2 - dy_3) < HAAS_THRESHOLD)) ) {
        haas_active = 1U;
    } else if ( edge_diff_1 < HAAS_THRESHOLD ) {
        haas_active = 1U;
    } else if ( edge_diff_2 < HAAS_THRESHOLD ) {
        haas_active = 1U;
    } else {
        //MISRA
    }

    if ( haas_active == 1U) {
        compute_aa_edge_clip(dx, dy, clip_min_x, clip_min_y, clip_max_x, clip_max_y, &clip_x, &clip_y, &clip_x1, &clip_y1);
        num_cmds += 6;

        num_cmds += extra_clipping_commands(dx , dy , clip_min_x , clip_min_y , clip_max_x , clip_max_y );
        num_cmds += extra_clipping_commands(dx2, dy2, clip_min_x2, clip_min_y2, clip_max_x2, clip_max_y2);
    }
    #endif //HAAS

    uint32_t *cmd_array = nema_cl_get_space(num_cmds);
    if (cmd_array == NULL) {
        return;
    }

    int cmd_idx = 0;

    if (nemavg_context->info.last_vertex == 1) {
        nemavg_context->info.last_vertex = 2;
    }
    else {
        nemavg_context->info.last_vertex = 1;
    }
    //triangle 012
    //Set clip for aa edge
    #ifdef HAAS
    if ( haas_active == 1U) {
        cmd_idx = update_clip(cmd_array, cmd_idx, clip_x, clip_y, clip_x1, clip_y1);
    }
    #endif //HAAS

    cmd_array[cmd_idx++] = NEMA_DRAW_PT0_X+0x10U*(*(uint32_t*)&nemavg_context->info.last_vertex);
    cmd_array[cmd_idx++] = *(uint32_t*)&x2fx;
    cmd_array[cmd_idx++] = NEMA_DRAW_PT0_Y+0x10U*(*(uint32_t*)&nemavg_context->info.last_vertex);
    cmd_array[cmd_idx++] = *(uint32_t*)&y2fx;

    cmd_array[cmd_idx++] = NEMA_DRAW_CMD_NOHOLD | HOLD;
    cmd_array[cmd_idx++] = DRAW_TRIANGLE | nemavg_context->aa_flags;

    #ifdef HAAS
    if ( haas_active == 1U) {
        cmd_idx = clip_non_aa_edges(cmd_array, cmd_idx, dx, dy, clip_min_x, clip_min_y, clip_max_x, clip_max_y);
    }
    #endif // HAAS

    if (nemavg_context->info.last_vertex == 1) {
        nemavg_context->info.last_vertex = 2;
    }
    else {
        nemavg_context->info.last_vertex = 1;
    }
    #ifdef HAAS
    if ( haas_active == 1U) {
        compute_aa_edge_clip(dx2, dy2, clip_min_x2, clip_min_y2, clip_max_x2, clip_max_y2, &clip_x, &clip_y, &clip_x1, &clip_y1);
        cmd_idx = update_clip(cmd_array, cmd_idx, clip_x, clip_y, clip_x1, clip_y1);
    }
    #endif //HAAS

    //triangle 023
    cmd_array[cmd_idx++] = NEMA_DRAW_PT0_X+0x10U*(*(uint32_t*)&nemavg_context->info.last_vertex);
    cmd_array[cmd_idx++] = *(uint32_t*)&x3fx;
    cmd_array[cmd_idx++] = NEMA_DRAW_PT0_Y+0x10U*(*(uint32_t*)&nemavg_context->info.last_vertex);
    cmd_array[cmd_idx++] = *(uint32_t*)&y3fx;

    cmd_array[cmd_idx++] = NEMA_DRAW_CMD_NOHOLD | HOLD;
    cmd_array[cmd_idx++] = DRAW_TRIANGLE | nemavg_context->aa_flags;

    #ifdef HAAS
    if ( haas_active == 1U) {
        cmd_idx = clip_non_aa_edges(cmd_array, cmd_idx, dx2, dy2, clip_min_x2, clip_min_y2, clip_max_x2, clip_max_y2);

        //Reset clip
        (void) reset_clip(cmd_array, cmd_idx);
    }
    #endif //HAAS
}

static void
nema_raster_triangle_only12_(nema_vg_float_t x1, nema_vg_float_t y1, nema_vg_float_t x2, nema_vg_float_t y2)
{
    int x1fx = nema_f2fx(x1);
    int y1fx = nema_f2fx(y1);
    int x2fx = nema_f2fx(x2);
    int y2fx = nema_f2fx(y2);

    int num_cmds = 2*2+1;
    #ifdef HAAS
    float dx = nema_absf(x1 - x2);
    float dy = nema_absf(y1 - y2);

    nema_vg_float_t clip_min_fx =  nema_min2(x1, x2);
    nema_vg_float_t clip_min_fy =  nema_min2(y1, y2);
    nema_vg_float_t clip_max_fx =  (nema_vg_float_t)nema_ceil_(nema_max2(x1, x2));
    nema_vg_float_t clip_max_fy =  (nema_vg_float_t)nema_ceil_(nema_max2(y1, y2));

    int32_t clip_min_x = (int32_t) clip_min_fx;
    int32_t clip_min_y = (int32_t) clip_min_fy;
    int32_t clip_max_x = (int32_t) clip_max_fx;
    int32_t clip_max_y = (int32_t) clip_max_fy;

    //default values (vertical)
    int32_t clip_x  = 0;
    int32_t clip_y  = 0;
    int32_t clip_x1 = 0;
    int32_t clip_y1 = 0;

    float dx_1 = x1-nemavg_context->info.fan_center.fanx0_f;
    float dy_1 = y1-nemavg_context->info.fan_center.fany0_f;
    float dx_2 = x2-nemavg_context->info.fan_center.fanx0_f;
    float dy_2 = y2-nemavg_context->info.fan_center.fany0_f;
    float dx_3 = x2-x1;
    float dy_3 = y2-y1;

    float a = nema_sqrt(dx_1*dx_1 + dy_1*dy_1);
    float b = nema_sqrt(dx_2*dx_2 + dy_2*dy_2);
    float c = nema_sqrt(dx_3*dx_3 + dy_3*dy_3);

    float edge_diff = nema_absf(nema_absf(a-b) - c);
    uint8_t haas_active  = 0U;

    if ( ((nema_absf(dx_1) < HAAS_THRESHOLD) || (nema_absf(dx_2) < HAAS_THRESHOLD) ) && ( (nema_absf(dx_1 - dx_2) < HAAS_THRESHOLD)  || (nema_absf(dy_1 - dy_2) < HAAS_THRESHOLD)) ) {
        haas_active = 1U;
    } else if ( edge_diff < HAAS_THRESHOLD ) {
        haas_active = 1U;
    } else {
        //MISRA
    }

    if ( haas_active == 1U) {
        compute_aa_edge_clip(dx, dy, clip_min_x, clip_min_y, clip_max_x, clip_max_y, &clip_x, &clip_y, &clip_x1, &clip_y1);
        num_cmds += 4;
        num_cmds += extra_clipping_commands(dx, dy, clip_min_x, clip_min_y, clip_max_x, clip_max_y);
    }

    #endif //HAAS

    uint32_t *cmd_array = nema_cl_get_space(num_cmds);
    if (cmd_array == NULL) {
        return;
    }

    int cmd_idx = 0;

    //Set clip for aa edge
    #ifdef HAAS
    if ( haas_active == 1U) {
        cmd_idx = update_clip(cmd_array, cmd_idx, clip_x, clip_y, clip_x1, clip_y1);
    }
    #endif //HAAS
    //Set triangle
    cmd_array[cmd_idx++] = NEMA_DRAW_PT1_X;
    cmd_array[cmd_idx++] = *(uint32_t*)&x1fx;
    cmd_array[cmd_idx++] = NEMA_DRAW_PT1_Y;
    cmd_array[cmd_idx++] = *(uint32_t*)&y1fx;
    cmd_array[cmd_idx++] = NEMA_DRAW_PT2_X;
    cmd_array[cmd_idx++] = *(uint32_t*)&x2fx;
    cmd_array[cmd_idx++] = NEMA_DRAW_PT2_Y;
    cmd_array[cmd_idx++] = *(uint32_t*)&y2fx;

    cmd_array[cmd_idx++] = NEMA_DRAW_CMD_NOHOLD | HOLD;
    cmd_array[cmd_idx++] = DRAW_TRIANGLE | nemavg_context->aa_flags;
    nemavg_context->info.last_vertex = 2;

    #ifdef HAAS
    if ( haas_active == 1U) {
        cmd_idx = clip_non_aa_edges(cmd_array, cmd_idx, dx, dy, clip_min_x, clip_min_y, clip_max_x, clip_max_y);

        //Reset clip
        (void) reset_clip(cmd_array, cmd_idx);
    }
    #endif //HAAS
}

static void
nema_raster_triangle_only2_(nema_vg_float_t x1, nema_vg_float_t y1, nema_vg_float_t x2, nema_vg_float_t y2)
{
    int x2fx = nema_f2fx(x2);
    int y2fx = nema_f2fx(y2);

    int num_cmds = 2+1;

    #ifdef HAAS
    float dx = nema_absf(x1 - x2);
    float dy = nema_absf(y1 - y2);

    nema_vg_float_t clip_min_fx =  nema_min2(x1, x2);
    nema_vg_float_t clip_min_fy =  nema_min2(y1, y2);
    nema_vg_float_t clip_max_fx =  nema_max2(x1, x2) + 0.99f;
    nema_vg_float_t clip_max_fy =  nema_max2(y1, y2) + 0.99f;

    int32_t clip_min_x = (int32_t) clip_min_fx;
    int32_t clip_min_y = (int32_t) clip_min_fy;
    int32_t clip_max_x = (int32_t) clip_max_fx;
    int32_t clip_max_y = (int32_t) clip_max_fy;

    //default values (vertical)
    int32_t clip_x  = 0;
    int32_t clip_y  = 0;
    int32_t clip_x1 = 0;
    int32_t clip_y1 = 0;

    float dx_1 = x1-nemavg_context->info.fan_center.fanx0_f;
    float dy_1 = y1-nemavg_context->info.fan_center.fany0_f;
    float dx_2 = x2-nemavg_context->info.fan_center.fanx0_f;
    float dy_2 = y2-nemavg_context->info.fan_center.fany0_f;
    float dx_3 = x2-x1;
    float dy_3 = y2-y1;

    float a = nema_sqrt(dx_1*dx_1 + dy_1*dy_1);
    float b = nema_sqrt(dx_2*dx_2 + dy_2*dy_2);
    float c = nema_sqrt(dx_3*dx_3 + dy_3*dy_3);

    float edge_diff = nema_absf(nema_absf(a-b) - c);

    uint8_t haas_active  = 0U;

    if ( ((nema_absf(dx_1) < HAAS_THRESHOLD ) || (nema_absf(dx_2) < HAAS_THRESHOLD ) ) && ( (nema_absf(dx_1 - dx_2) < HAAS_THRESHOLD)  || (nema_absf(dy_1 - dy_2) < HAAS_THRESHOLD)) ) {
        haas_active = 1U;
    } else if ( edge_diff < HAAS_THRESHOLD ) {
        haas_active = 1U;
    } else {
        //MISRA
    }

    if ( haas_active == 1U) {
        compute_aa_edge_clip(dx, dy, clip_min_x, clip_min_y, clip_max_x, clip_max_y, &clip_x, &clip_y, &clip_x1, &clip_y1);
        num_cmds += 4;
        num_cmds += extra_clipping_commands(dx, dy, clip_min_x, clip_min_y, clip_max_x, clip_max_y);
    }
    #endif // HAAS

    uint32_t *cmd_array = nema_cl_get_space(num_cmds);
    if (cmd_array == NULL) {
        return;
    }

    int cmd_idx = 0;

    if (nemavg_context->info.last_vertex == 1) {
        nemavg_context->info.last_vertex = 2;
    }
    else {
        nemavg_context->info.last_vertex = 1;
    }

    //Set clip for aa edge
    #ifdef HAAS
    if ( haas_active == 1U) {
        cmd_idx = update_clip(cmd_array, cmd_idx, clip_x, clip_y, clip_x1, clip_y1);
    }
    #endif //HAAS
    //Set triangle
    cmd_array[cmd_idx++] = NEMA_DRAW_PT0_X+0x10U*(*(uint32_t*)&nemavg_context->info.last_vertex);
    cmd_array[cmd_idx++] = *(uint32_t*)&x2fx;
    cmd_array[cmd_idx++] = NEMA_DRAW_PT0_Y+0x10U*(*(uint32_t*)&nemavg_context->info.last_vertex);
    cmd_array[cmd_idx++] = *(uint32_t*)&y2fx;

    cmd_array[cmd_idx++] = NEMA_DRAW_CMD_NOHOLD | HOLD;
    cmd_array[cmd_idx++] = DRAW_TRIANGLE | nemavg_context->aa_flags;

    #ifdef HAAS
    if ( haas_active == 1U) {
        cmd_idx = clip_non_aa_edges(cmd_array, cmd_idx, dx, dy, clip_min_x, clip_min_y, clip_max_x, clip_max_y);

        //Reset clip
        (void) reset_clip(cmd_array, cmd_idx);
    }
    #endif // HAAS
}


//
//          1                   //
//          /\---\              //
//         /  \   --\ bezier    //
//        /    \     |          //
//       /      \    |          //
//      /        \   /          //
//     /__________\ /           //
//    0            2 (idx)      //

// idx is last (more recent) idx of triangle
// x2 is last (more recent) vertex of triangle
// x1 is previous x2
// x0 is fan center

// shared edges must not be antialiased:
//   - edge x1-x2 is shared between triangle and bezier
//   - edges between 2 concecutive triangles in a triangle fan

// A typical rasterization would involve the indices as follows:
// a) 0, 0, 1 (in case there is a Bezier between 0 and 1)
// b) 0, 1, 2
// c) 0, 2, 3
// d) 0, 3, 4
// ..... etc
//
//

//rasterize a quadratic bezier via tesselation (fixed segment count) - draw with lines
static void
fill_quadratic_decasteljau( nema_vg_vertex_t_ p0,
                            nema_vg_vertex_t_ c,
                            nema_vg_vertex_t_ p1)
{
    int iter = 1;
    do  {
        // midpoint
        nema_vg_vertex_t_ m = {(p0.x + c.x*2.f + p1.x)*0.25f, (p0.y + c.y*2.f + p1.y)*0.25f};
        // versus directly drawn line
        nema_vg_float_t dx = (p0.x + p1.x)*0.5f - m.x;
        nema_vg_float_t dy = (p0.y + p1.y)*0.5f - m.y;

        nema_vg_float_t flatness = dx*dx+dy*dy;

        if( flatness <= DECASTELJAU_QUAD_FLATNESS )
        {
            --iter;

            // We draw a quad including midpoint (mx, my)
            // This improves rendering quality
            if (nemavg_context->info.last_vertex == -1) {
                nema_raster_quad_only123_(
                                            p0.x, p0.y,
                                            m.x, m.y,
                                            p1.x, p1.y);
            }
            else {
                nema_raster_quad_only23_(
                                            p0.x, p0.y,
                                            m.x, m.y,
                                            p1.x, p1.y);
            }
            tess_stack_pop_vertex(&p0, &c, &p1);
            continue;
        }

        // Calculate all the mid-points of the line segments
        //----------------------
        nema_vg_vertex_t_ p01 = {( p0.x + c.x ) * 0.5f, ( p0.y + c.y ) * 0.5f};
        nema_vg_vertex_t_ p12 = {( c.x + p1.x ) * 0.5f, ( c.y + p1.y ) * 0.5f};

        // Continue subdivision
        //----------------------
        tess_stack_push_vertex(m, p12, p1);
        c  = p01;
        p1 = m;

        ++iter;
    } while ( iter != 0 );
}

static inline float _f2fxx(float f) {
    int i = nema_f2fx(f);

    uint32_t *ui = (uint32_t *)&i;
    *ui = *ui&(~0xfffU);
    return (float)i;
}

static int
triangle_area2( nema_vg_float_t p0x,  nema_vg_float_t p0y,
                nema_vg_float_t p1x,  nema_vg_float_t p1y,
                nema_vg_float_t p2x,  nema_vg_float_t p2y) {

    p0x = _f2fxx( p0x );
    p1x = _f2fxx( p1x );
    p2x = _f2fxx( p2x );
    p0y = _f2fxx( p0y );
    p1y = _f2fxx( p1y );
    p2y = _f2fxx( p2y );

    nema_vg_float_t area2 = p0x * (p1y - p2y) + p1x * (p2y - p0y) + p2x * (p0y - p1y);
    return (int)area2;
}

static int
set_frag_ptr_non_zero(int area2) {
    //
    static const uint32_t prev_frag_ptr = 0xdeadU;
    const uint32_t add_frag_ptr = PREFETCH_TEXEL | PRE_T1 | PRE_XY | PRE_TEX[NEMA_TEX3];
    const uint32_t sub_frag_ptr = PREFETCH_TEXEL | PRE_T1 | PRE_XY | PRE_TEX[NEMA_TEX3] | SHADER_ADDR_SUB;


    if (area2 < 0) {
        if ( prev_frag_ptr != sub_frag_ptr) {
            nema_set_frag_ptr(sub_frag_ptr);
        }
        return 0;
    }
    else {
        if ( prev_frag_ptr != add_frag_ptr){
            nema_set_frag_ptr(add_frag_ptr);
        }
        return 0;
    }
}

//rasterize a cubic bezier via tesselation (fixed segment count) - draw with lines
static void
fill_cubic_decasteljau( nema_vg_vertex_t_ p0,
                        nema_vg_vertex_t_ c0,
                        nema_vg_vertex_t_ c1,
                        nema_vg_vertex_t_ p1)
{
    int iter = 1;
    int force_tess = 1;
    nema_vg_vertex_t_ p0_ = p0;
    do  {
        // Calculate all the mid-points of the line segments
        //----------------------
        nema_vg_vertex_t_ p01 = {( p0.x + c0.x ) * 0.5f, ( p0.y + c0.y ) * 0.5f};
        nema_vg_vertex_t_ p12 = {( c0.x + c1.x ) * 0.5f, ( c0.y + c1.y ) * 0.5f};
        nema_vg_vertex_t_ p23 = {( c1.x + p1.x ) * 0.5f, ( c1.y + p1.y ) * 0.5f};

        nema_vg_vertex_t_ p012 = {( p01.x + p12.x ) * 0.5f, ( p01.y + p12.y ) * 0.5f};
        nema_vg_vertex_t_ p123 = {( p12.x + p23.x ) * 0.5f, ( p12.y + p23.y ) * 0.5f};

        nema_vg_vertex_t_ m = {( p012.x + p123.x ) * 0.5f, ( p012.y + p123.y ) * 0.5f};

        // versus directly drawn line
        nema_vg_float_t dx = (p0.x + p1.x)*0.5f - m.x;
        nema_vg_float_t dy = (p0.y + p1.y)*0.5f - m.y;

        nema_vg_float_t flatness = dx*dx+dy*dy;
        if( flatness <= DECASTELJAU_CUBIC_FLATNESS && force_tess != 1)
        {
            --iter;
                int area = triangle_area2(p0_.x, p0_.y,
                                          p0.x, p0.y,
                                          p1.x, p1.y);
            if(nemavg_context->fill_rule == NEMA_VG_FILL_NON_ZERO)
            {


               (void)set_frag_ptr_non_zero(area);

            }

            nema_raster_triangle_only12_(p0.x, p0.y, p1.x, p1.y);

            p0 = p1;
            tess_stack_pop_vertex(&c0, &c1, &p1);

            continue;
        }

        force_tess = 0;
        // Continue subdivision
        //----------------------
        tess_stack_push_vertex(p123, p23, p1);
        ++iter;

        c0 = p01;
        c1 = p012;
        p1 = m;

    } while ( iter != 0 );
}

static inline void
line_non_zero(nema_vg_vertex_t_* p1, nema_vg_vertex_t_* p2)
{
    int area2 = triangle_area2((nema_vg_float_t)nemavg_context->info.fan_center.fanx0, (nema_vg_float_t)nemavg_context->info.fan_center.fany0, p1->x, p1->y, p2->x, p2->y);
    (void)set_frag_ptr_non_zero(area2);

    nema_raster_triangle_only12_(
        (p1->x), (p1->y),
        (p2->x), (p2->y));
}

static inline void
line_even_odd(nema_vg_vertex_t_* p1, nema_vg_vertex_t_* p2)
{
    if (nemavg_context->info.last_vertex == -1) {
        nema_raster_triangle_only12_(
            (p1->x), (p1->y),
            (p2->x), (p2->y));
    }
    else {
        nema_raster_triangle_only2_(p1->x, p1->y, p2->x, p2->y);
    }
}

static inline void
line_draw(nema_vg_vertex_t_* p1, nema_vg_vertex_t_* p2)
{
    raster_stroked_line_( *p1, *p2, 1);
}

uint32_t
fill_cubic(nema_vg_info_t_* path_info)
{
    if(nemavg_context->fill_rule == NEMA_VG_FILL_DRAW){
        stroke_cubic_decasteljau(path_info->p1,
                                 path_info->ctrl0,
                                 path_info->ctrl1,
                                 path_info->p2);
    }
    else if ( nemavg_context->fill_rule == NEMA_VG_FILL_EVEN_ODD ) {
        fill_cubic_decasteljau(path_info->p1,
                               path_info->ctrl0,
                               path_info->ctrl1,
                               path_info->p2);
    }
    else{// NEMA_VG_FILL_NON_ZERO


#ifdef HAAS
        int32_t fanx_prev = nemavg_context->info.fan_center.fanx0;
        int32_t fany_prev = nemavg_context->info.fan_center.fany0;
        set_fan_center(path_info->p1.x, path_info->p1.y);
#endif //HAAS


        set_raster_point(0, nema_f2fx(path_info->p1.x), nema_f2fx(path_info->p1.y));
        fill_cubic_decasteljau(path_info->p1,
                               path_info->ctrl0,
                               path_info->ctrl1,
                               path_info->p2);


#ifdef HAAS
        nema_vg_float_t fanx_prev_f = (nema_vg_float_t) fanx_prev;
        nema_vg_float_t fany_prev_f = (nema_vg_float_t) fany_prev;
        set_fan_center(fanx_prev_f, fany_prev_f);
#endif //HAAS
    }



    return NEMA_VG_ERR_NO_ERROR;
}

static uint32_t
fill_quad(nema_vg_info_t_* path_info)
{
    if(nemavg_context->fill_rule == NEMA_VG_STROKE){
        stroke_quadratic_decasteljau(path_info->p1,
                                     path_info->ctrl0,
                                     path_info->p2);
    }
    else if(nemavg_context->fill_rule == NEMA_VG_FILL_EVEN_ODD) {
        fill_quadratic_decasteljau(path_info->p1,
                                   path_info->ctrl0,
                                   path_info->p2);
    }
    else {
        int area_ = triangle_area2( path_info->p1.x, path_info->p1.y,
                                    path_info->ctrl0.x, path_info->ctrl0.y,
                                    path_info->p2.x, path_info->p2.y);

        (void)set_frag_ptr_non_zero(area_);
        fill_quadratic_decasteljau(path_info->p1,
                                   path_info->ctrl0,
                                   path_info->p2);
    }
    return NEMA_VG_ERR_NO_ERROR;
}

static uint32_t
fill_path_even_odd(nema_vg_path_t* path) {
    PRINT_FUNC_ENTRY;

    nema_vbuf_t_ *vb = &path->shape;

    nema_vg_float_t new_fanx0 = (stencil->dirty_area_p1.x+
                                 stencil->dirty_area_p2.x+
                                 stencil->dirty_area_p3.x+
                                 stencil->dirty_area_p4.x )/4.f;

    nema_vg_float_t new_fany0 = (stencil->dirty_area_p1.y+
                                 stencil->dirty_area_p2.y+
                                 stencil->dirty_area_p3.y+
                                 stencil->dirty_area_p4.y)/4.f;

    set_fan_center(new_fanx0, new_fany0);

    set_raster_point(4, nemavg_context->info.fan_center.fanx0fx, nemavg_context->info.fan_center.fany0fx);

    // last vertex that was updated
    // this is a new path, so set it to -1 (no last vertex)

    nema_vg_info_t_ path_info = {0};

    path_info.no_move=1;
    path_info.no_close=1;
    path_info.new_path=1;

    size_t idx_seg = 0;
    while (idx_seg < vb->seg_size) {

        uint8_t seg = (vb->seg)[idx_seg++];
        uint8_t seg_no_rel = (uint8_t)(seg & NEMA_VG_NO_REL_MASK); //remove rel from seg to avoid multi checking
        NEMA_VG_IF_ERROR(process_path_vertex(vb, seg, &path_info));

        if(path_info.new_path == 0U){
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
            // draw a triangle
                line_even_odd(&path_info.p1, &path_info.p2);
            }
        }
#ifdef DEBUG_VG
    PRINT_F("\n");
    PRINT_F("p1: %f, %f \n", path_info.p1.x, path_info.p1.y);
    PRINT_F("p2: %f, %f \n", path_info.p2.x, path_info.p2.y);
    PRINT_F("\n");
#endif
    }


    if(path_info.no_close == 1U){
        nema_vg_vertex_t_ implicit_p2 = find_implicit_vertices(&path_info);
        line_even_odd(&path_info.p2, &implicit_p2);
    }
    return NEMA_VG_ERR_NO_ERROR;
}

static uint32_t
fill_path_non_zero(nema_vg_path_t* path) {
    PRINT_FUNC_ENTRY;

    nema_vbuf_t_ *vb = &path->shape;

    float new_fanx0 = (stencil->dirty_area_p1.x+
                       stencil->dirty_area_p2.x+
                       stencil->dirty_area_p3.x+
                       stencil->dirty_area_p4.x)/4.f;

    float new_fany0 = (stencil->dirty_area_p1.y+
                       stencil->dirty_area_p2.y+
                       stencil->dirty_area_p3.y+
                       stencil->dirty_area_p4.y)/4.f;


    new_fanx0 = nema_clamp(new_fanx0, 0.0f, (float)stencil->width);
    new_fany0 = nema_clamp(new_fany0, 0.0f, (float)stencil->width);

    set_fan_center(new_fanx0, new_fany0);

    set_raster_point(0, nemavg_context->info.fan_center.fanx0fx, nemavg_context->info.fan_center.fany0fx);

    nema_vg_info_t_ path_info = {0};
    path_info.no_move=1;
    path_info.no_close=1;
    path_info.new_path=1;

    size_t idx_seg = 0;
    while (idx_seg < vb->seg_size) {

        uint8_t seg = (vb->seg)[idx_seg++];
        uint8_t seg_no_rel = (uint8_t)(seg & NEMA_VG_NO_REL_MASK); //remove rel from seg to avoid multi checking
        NEMA_VG_IF_ERROR(process_path_vertex(vb, seg, &path_info));

        if(path_info.new_path == 0U){

            uint32_t flags = nemavg_context->aa_flags;

            if ( seg_no_rel == NEMA_VG_PRIM_BEZIER_QUAD || seg_no_rel == NEMA_VG_PRIM_BEZIER_SQUAD ) {
#ifdef HAAS
                int32_t fanx_prev = nemavg_context->info.fan_center.fanx0;
                int32_t fany_prev = nemavg_context->info.fan_center.fany0;
                set_fan_center(path_info.p1.x, path_info.p1.y);
#endif //HAAS

                set_raster_point(0, nema_f2fx(path_info.p1.x), nema_f2fx(path_info.p1.y));
                NEMA_VG_IF_ERROR(fill_quad(&path_info));

#ifdef HAAS
                float fanx_prev_f = (float) fanx_prev;
                float fany_prev_f = (float) fany_prev;
                set_fan_center(fanx_prev_f, fany_prev_f);
#endif //HAAS

                set_raster_point(0, nemavg_context->info.fan_center.fanx0fx, nemavg_context->info.fan_center.fany0fx);
                nemavg_context->aa_flags = 0U;
            }
            else if ( seg_no_rel == NEMA_VG_PRIM_BEZIER_CUBIC || seg_no_rel == NEMA_VG_PRIM_BEZIER_SCUBIC ) {
                NEMA_VG_IF_ERROR(fill_cubic(&path_info));

                set_raster_point(0, nemavg_context->info.fan_center.fanx0fx, nemavg_context->info.fan_center.fany0fx);
                nemavg_context->aa_flags = 0U;
            }
            else if ( (seg & NEMA_VG_PRIM_MASK) == NEMA_VG_PRIM_ARC ) {
                NEMA_VG_IF_ERROR(fill_arc(path, &path_info));
                set_raster_point(0, nemavg_context->info.fan_center.fanx0fx, nemavg_context->info.fan_center.fany0fx);
                nemavg_context->aa_flags = 0U;
            }
            else if( seg_no_rel == NEMA_VG_PRIM_POLYGON || seg_no_rel == NEMA_VG_PRIM_POLYLINE ) {
                // The polygon/polyline segments are already handled inside process_path_vertex...
            }
            else {
                // MISRA
            }

            // draw a triangle
            if( seg_no_rel != NEMA_VG_PRIM_POLYGON && seg_no_rel != NEMA_VG_PRIM_POLYLINE ) {
                line_non_zero(&path_info.p1, &path_info.p2);
            }
            nemavg_context->aa_flags = flags;
        }
    }

    if(path_info.no_close == 1U){
        nema_vg_vertex_t_ implicit_p2 = find_implicit_vertices(&path_info);
        line_non_zero(&path_info.p2, &implicit_p2);
    }

    return NEMA_VG_ERR_NO_ERROR;
}

static inline void
AA_nz_stencil(void)
{
    // UV/STENCIL is in TEX3
    // LUT is in TEX2
    // PIXOUT to TEX3 (STENCIL)

    const uint32_t cmd_map[4] = { 0x0000110bU, 0x00000000U,   // Read LUT based on UV/STENCIL
                                  0x08001182U, 0x80000009U }; // Pixout to STENCIL
    nema_set_blend_blit(NEMA_BL_SRC); //setup matmult and ROB blender (if present) correctly
    nema_set_frag_ptr( PREFETCH_TEXEL | PRE_T1 | PRE_XY | PRE_IMG3 );
    nema_load_frag_shader(cmd_map, 2, 0);

    nema_raster_quad_f(stencil->dirty_area_p1.x, stencil->dirty_area_p1.y,
                       stencil->dirty_area_p2.x, stencil->dirty_area_p2.y,
                       stencil->dirty_area_p3.x, stencil->dirty_area_p3.y,
                       stencil->dirty_area_p4.x, stencil->dirty_area_p4.y );
}


static uint32_t
stencil_draw(nema_vg_path_t* path, nema_vg_paint_t* paint)
{
    uint32_t error = NEMA_VG_ERR_NO_ERROR;

    uint32_t blending_mode = 0U;
    uint32_t raster_color = 0U;

    switch (nemavg_context->fill_rule) {
        case NEMA_VG_STROKE:
            blending_mode = nema_blending_mode(NEMA_BF_ONE, 0, NEMA_BLOP_NO_USE_ROPBL);
            raster_color  = 0xffffffffU;
            break;
        case NEMA_VG_FILL_EVEN_ODD:
            blending_mode = nema_blending_mode(NEMA_BF_INVDESTCOLOR, 0, NEMA_BLOP_NO_USE_ROPBL);
            raster_color  = 0xffffffffU;
            break;
        case NEMA_VG_FILL_NON_ZERO:
            blending_mode = nema_blending_mode(NEMA_BF_ONE, NEMA_BF_ONE, NEMA_BLOP_NO_USE_ROPBL);
            raster_color  = ADD_COLOR;
            break;
        default:
            PRINT_F("%s: Invalid fill rule!!\n", __func__);
            error =  NEMA_VG_ERR_INVALID_FILL_RULE;
            nema_vg_set_error(error);
            break;
    }

    // SHADER
    nema_set_blend(blending_mode, NEMA_TEX3, NEMA_NOTEX, NEMA_NOTEX);
    nema_set_raster_color(raster_color);

    // Now draw the path
    switch (nemavg_context->fill_rule) {
        case NEMA_VG_STROKE:
            NEMA_VG_IF_ERROR(stroke_path(path));
            break;
        case NEMA_VG_FILL_EVEN_ODD:
            NEMA_VG_IF_ERROR(fill_path_even_odd(path));
            break;
        case NEMA_VG_FILL_NON_ZERO:

#ifndef NEMAPVG
    {
        //subtract src from dst
        const uint32_t cmd_sub_src_from_dst[] = {0x080c0182U, 0x8a2743d6U};
        nema_load_frag_shader(cmd_sub_src_from_dst, 1, SHADER_ADDR_SUB);
    }
#endif

            NEMA_VG_IF_ERROR(fill_path_non_zero(path));
            break;
        default:
            PRINT_F("%s: Invalid fill rule!!\n", __func__);
            error =  NEMA_VG_ERR_INVALID_FILL_RULE;
            nema_vg_set_error(error);
            break;
    }

    if (error == NEMA_VG_ERR_NO_ERROR && nemavg_context->fill_rule == NEMA_VG_FILL_NON_ZERO){
        AA_nz_stencil();
    }

    // Leave TEX3 ready to be read by next stages
    nema_cl_add_cmd(NEMA_TEX3_FSTRIDE, ((unsigned)NEMA_A8 << 24) | ((unsigned)stencil->width & 0xffffU));

    return error;
}

#endif // ifndef NEMAPVG

#endif
