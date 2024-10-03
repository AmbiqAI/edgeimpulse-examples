#ifndef PROCESS_PATH_VERTEX_CC__
#define PROCESS_PATH_VERTEX_CC__

#define A m[0][0]
#define B m[0][1]
#define C m[0][2]
#define D m[1][0]
#define E m[1][1]
#define F m[1][2]
#define G m[2][0]
#define H m[2][1]
#define X m[2][2]

static inline void
nema_mat3x3_mul_vec_(nema_matrix3x3_t m,
                    float *x,
                    float *y)
{
#if 0
    float x0 = *x;
    float y0 = *y;
    float divisor = G*x0+H*y0+X;

    *x = (A*x0+B*y0+C)/divisor;
    *y = (D*x0+E*y0+F)/divisor;
#else
    float x0 = *x;
    float y0 = *y;

    *x = (A*x0+B*y0+C);
    *y = (D*x0+E*y0+F);
#endif
}

#ifndef HW_VERTEX_MMUL
inline static void
transform_vertex(nema_matrix3x3_t m, nema_vg_vertex_t_* coord)
{
    nema_mat3x3_mul_vec_(m, &(coord->x), &(coord->y));
}
#else
#define transform_vertex(...)
#endif

#if 0
//checks if next coordinates are available
static inline uint32_t
check_next_data_available(nema_vbuf_t_ *vb, size_t index, uint32_t size)
{
    if ( index + size > vb->data_size){
        nema_vg_set_error(NEMA_VG_ERR_INVALID_VERTEX_DATA);
        return NEMA_VG_ERR_INVALID_VERTEX_DATA;
    }
    else{
        return NEMA_VG_ERR_NO_ERROR;
    }
}

#define CHECK_DATA(vg, index, size) NEMA_VG_IF_ERROR(check_next_data_available((vb), (index), (size)))

#else
// #define check_next_data_available(...) NEMA_VG_ERR_NO_ERROR
#define CHECK_DATA(...)
#endif

static inline nema_vg_vertex_t_
find_implicit_vertices(nema_vg_info_t_* path_info)
{
    nema_vg_vertex_t_ implicit_p2 = {0.f, 0.f};
    if(path_info->no_move == 0U){
        implicit_p2.x = path_info->path_start_data.x;
        implicit_p2.y = path_info->path_start_data.y;
    }else{
        implicit_p2.x = 0.f;
        implicit_p2.y = 0.f;
    }
#ifndef HW_VERTEX_MMUL
    if (nemavg_context->info.has_transformation != 0U)  {
        transform_vertex(nemavg_context->path_plus_global_m, &implicit_p2);
    }
#endif
    return implicit_p2;
}

static inline uint32_t
process_path_vertex( nema_vbuf_t_ *vb, const uint8_t seg,
                    nema_vg_info_t_* path_info)
{

    path_info->p1 = path_info->p2;

    //next data available are checked before this call

    uint8_t rel_coordinates = 0U;
    if((seg & NEMA_VG_REL) != 0U){
        rel_coordinates = 1U;
    }

    uint8_t seg_no_rel = (uint8_t)(seg & NEMA_VG_NO_REL_MASK); //remove rel from seg
    uint8_t previous_seg_no_rel = (uint8_t)(path_info->previous_seg & NEMA_VG_NO_REL_MASK);
    if ( seg_no_rel == NEMA_VG_PRIM_MOVE ) {
        CHECK_DATA(vb, path_info->idx_data, 2);// check if next point is available
#ifndef CALC_BBOX
        if(path_info->no_close == 1U && path_info->idx_data > 0U && nemavg_context->fill_rule != NEMA_VG_STROKE){ // there was no close
            nema_vg_vertex_t_ implicit_p2 = find_implicit_vertices(path_info);
    #ifdef NEMAPVG
            raster_triangle_fffx(nemavg_context->info.fan_center.fanx0fx, nemavg_context->info.fan_center.fany0fx, &path_info->p1, &implicit_p2);
    #else
            render_line(&path_info->p1, &implicit_p2);
    #endif
        }

        reset_last_vertex();
#else
#ifndef CLIPPING
        if(path_info->no_close == 1U && path_info->idx_data > 0U ) {
            nema_vg_vertex_t_ implicit_p2 = find_implicit_vertices(path_info);

            calc_bbox(vb, &implicit_p2);

        }
#else
        // Doing clipping
        // Implicit close
        if(path_info->no_close == 1U && path_info->idx_data > 0U ) {
            nema_vg_vertex_t_ implicit_p2 = find_implicit_vertices(path_info);
            culled_info.prev_cmd_is_move = IMPLICIT_CLOSE_;
            NEMA_VG_IF_ERROR(calc_line_geometry(path_info->p2, implicit_p2));
        }

#endif // CLIPPING
#endif // CALC_BBOX

        path_info->no_move = 0U;
        path_info->new_path = 1;
        path_info->start_idx = path_info->idx_data;

        if (rel_coordinates != 0U ){
            path_info->p2.x = path_info->previous_data.x + (vb->data)[path_info->idx_data]; path_info->idx_data++;
            path_info->p2.y = path_info->previous_data.y + (vb->data)[path_info->idx_data]; path_info->idx_data++;
        }
        else{
            path_info->p2.x = (vb->data)[path_info->idx_data]; path_info->idx_data++;
            path_info->p2.y = (vb->data)[path_info->idx_data]; path_info->idx_data++;
        }
        path_info->previous_data.x =  path_info->p2.x;
        path_info->previous_data.y =  path_info->p2.y;
        //start_idx is not valid for rel coordinates so we keep the starting point
        path_info->path_start_data.x = path_info->p2.x;
        path_info->path_start_data.y = path_info->p2.y;
    }
    else if((seg == NEMA_VG_PRIM_CLOSE) || (seg == (/*NEMA_VG_PRIM_CLOSE |*/ NEMA_VG_PRIM_NO_STROKE))){
        if(path_info->no_move == 1U){
            path_info->p2.x = 0.0f;
            path_info->p2.y = 0.0f;
        }
        else{
            path_info->p2.x = path_info->path_start_data.x;
            path_info->p2.y = path_info->path_start_data.y;
        }

        path_info->previous_data.x =  path_info->p2.x;
        path_info->previous_data.y =  path_info->p2.y;
        path_info->new_path = 0U;
        path_info->no_close = 0U;
    }
    else { // (v,h)line, (s)quad , (s)cubic or arc here perform draw

        //find control points if needed
        if(path_info->idx_data == 0U){ //if first command is not a move

#ifndef CALC_BBOX
            reset_last_vertex();
#endif // CALC_BBOX

            path_info->no_move = 1U;
            //perform transformation if any
#ifndef HW_VERTEX_MMUL
            if (nemavg_context->info.has_transformation != 0U)  {
                transform_vertex(nemavg_context->path_plus_global_m, &path_info->p1);
            }
#endif
        }
        path_info->new_path = 0U;

        if(seg_no_rel == NEMA_VG_PRIM_BEZIER_CUBIC){
            CHECK_DATA(vb, path_info->idx_data, 4);// check if next points are available

            path_info->ctrl0.x = (vb->data)[path_info->idx_data]; (path_info->idx_data)++;
            path_info->ctrl0.y = (vb->data)[path_info->idx_data]; (path_info->idx_data)++;
            path_info->ctrl1.x = (vb->data)[path_info->idx_data]; (path_info->idx_data)++;
            path_info->ctrl1.y = (vb->data)[path_info->idx_data]; (path_info->idx_data)++;

            if (rel_coordinates != 0U ){
                path_info->ctrl0.x += path_info->previous_data.x;
                path_info->ctrl0.y += path_info->previous_data.y;
                path_info->ctrl1.x += path_info->previous_data.x;
                path_info->ctrl1.y += path_info->previous_data.y;
            }
            path_info->previous_ctrl.x =  path_info->ctrl1.x;
            path_info->previous_ctrl.y =  path_info->ctrl1.y;
#ifndef HW_VERTEX_MMUL
            if (nemavg_context->info.has_transformation != 0U)  {
                transform_vertex(nemavg_context->path_plus_global_m, &path_info->ctrl0);
                transform_vertex(nemavg_context->path_plus_global_m, &path_info->ctrl1);
            }
#endif
        }
        else if(seg_no_rel == NEMA_VG_PRIM_BEZIER_QUAD){
            CHECK_DATA(vb, path_info->idx_data, 2);// check if next point is available
            path_info->ctrl0.x = (vb->data)[path_info->idx_data]; (path_info->idx_data)++;
            path_info->ctrl0.y = (vb->data)[path_info->idx_data]; (path_info->idx_data)++;
            if (rel_coordinates != 0U ){
                path_info->ctrl0.x += path_info->previous_data.x;
                path_info->ctrl0.y += path_info->previous_data.y;
            }

            path_info->previous_ctrl.x =  path_info->ctrl0.x;
            path_info->previous_ctrl.y =  path_info->ctrl0.y;
#ifndef HW_VERTEX_MMUL
            if (nemavg_context->info.has_transformation != 0U)  {
                transform_vertex(nemavg_context->path_plus_global_m, &path_info->ctrl0);
            }
#endif

        }
        else if(seg_no_rel == NEMA_VG_PRIM_BEZIER_SQUAD ){
#ifndef C1_SMOOTHNESS
            if( previous_seg_no_rel == NEMA_VG_PRIM_BEZIER_QUAD || previous_seg_no_rel == NEMA_VG_PRIM_BEZIER_SQUAD
            || previous_seg_no_rel == NEMA_VG_PRIM_BEZIER_CUBIC || previous_seg_no_rel == NEMA_VG_PRIM_BEZIER_SCUBIC){
                path_info->ctrl0.x = 2.f *  path_info->previous_data.x -  path_info->previous_ctrl.x; //ctrl x
                path_info->ctrl0.y = 2.f *  path_info->previous_data.y -  path_info->previous_ctrl.y; //ctrl y

            }
            else {
                // if previous segment is not BEZIER
                // Use p1 as control point
                path_info->ctrl0.x = path_info->previous_data.x;
                path_info->ctrl0.y = path_info->previous_data.y;
            }
#else
            if( previous_seg_no_rel == NEMA_VG_PRIM_BEZIER_QUAD || previous_seg_no_rel == NEMA_VG_PRIM_BEZIER_SQUAD){
                path_info->ctrl0.x = 2.f *  path_info->previous_data.x -  path_info->previous_ctrl.x; //ctrl x
                path_info->ctrl0.y = 2.f *  path_info->previous_data.y -  path_info->previous_ctrl.y; //ctrl y
            }
            else if(previous_seg_no_rel == NEMA_VG_PRIM_BEZIER_CUBIC || previous_seg_no_rel == NEMA_VG_PRIM_BEZIER_SCUBIC){
                path_info->ctrl0.x = (5.f *  path_info->previous_data.x - 3.f *  path_info->previous_ctrl.x)/2.f; //ctrl x
                path_info->ctrl0.y = (5.f *  path_info->previous_data.y - 3.f *  path_info->previous_ctrl.y)/2.f; //ctrl y
            }
            else {
                // if previous segment is not BEZIER
                // Use p1 as control point
                path_info->ctrl0.x = path_info->previous_data.x;
                path_info->ctrl0.y = path_info->previous_data.y;
            }
#endif
            path_info->previous_ctrl.x =  path_info->ctrl0.x;
            path_info->previous_ctrl.y =  path_info->ctrl0.y;
#ifndef HW_VERTEX_MMUL
            if (nemavg_context->info.has_transformation != 0U)  {
                transform_vertex(nemavg_context->path_plus_global_m, &path_info->ctrl0);
            }
#endif
        }
        else if(seg_no_rel == NEMA_VG_PRIM_BEZIER_SCUBIC){
#ifndef C1_SMOOTHNESS
            if(previous_seg_no_rel == NEMA_VG_PRIM_BEZIER_QUAD || previous_seg_no_rel == NEMA_VG_PRIM_BEZIER_SQUAD
               || previous_seg_no_rel == NEMA_VG_PRIM_BEZIER_CUBIC || previous_seg_no_rel == NEMA_VG_PRIM_BEZIER_SCUBIC){
                path_info->ctrl0.x = 2.f * path_info->previous_data.x  - path_info->previous_ctrl.x; //ctrl x
                path_info->ctrl0.y = 2.f * path_info->previous_data.y  - path_info->previous_ctrl.y; //ctrl y
            }
            else {
                // if previous segment is not BEZIER
                // Use p1 as control point
                path_info->ctrl0.x = path_info->previous_data.x ;
                path_info->ctrl0.y = path_info->previous_data.y ;
            }
#else
            if(previous_seg_no_rel == NEMA_VG_PRIM_BEZIER_QUAD || previous_seg_no_rel == NEMA_VG_PRIM_BEZIER_SQUAD)
            {
                path_info->ctrl0.x = (5.f * path_info->previous_data.x - 2.f * path_info->previous_ctrl.x)/3.f; //ctrl x
                path_info->ctrl0.y = (5.f * path_info->previous_data.y - 2.f * path_info->previous_ctrl.y)/3.f; //ctrl y
            }
            else if(previous_seg_no_rel == NEMA_VG_PRIM_BEZIER_CUBIC|| previous_seg_no_rel == NEMA_VG_PRIM_BEZIER_SCUBIC){
                path_info->ctrl0.x = 2.f * path_info->previous_data.x - path_info->previous_ctrl.x; //ctrl x
                path_info->ctrl0.y = 2.f * path_info->previous_data.y - path_info->previous_ctrl.y; //ctrl y
            }
            else {
                // if previous segment is not BEZIER
                // Use p1 as control point
                path_info->ctrl0.x = path_info->previous_data.x;
                path_info->ctrl0.y = path_info->previous_data.y;
            }
#endif

            CHECK_DATA(vb, path_info->idx_data, 2); //check if next point is available
            path_info->ctrl1.x = (vb->data)[path_info->idx_data]; (path_info->idx_data)++;
            path_info->ctrl1.y = (vb->data)[path_info->idx_data]; (path_info->idx_data)++;

            if (rel_coordinates != 0U ){
                path_info->ctrl1.x += path_info->previous_data.x;
                path_info->ctrl1.y += path_info->previous_data.y;
            }

            path_info->previous_ctrl.x =  path_info->ctrl1.x;
            path_info->previous_ctrl.y =  path_info->ctrl1.y;
#ifndef HW_VERTEX_MMUL
            if (nemavg_context->info.has_transformation != 0U)  {
                transform_vertex(nemavg_context->path_plus_global_m, &path_info->ctrl0);
                transform_vertex(nemavg_context->path_plus_global_m, &path_info->ctrl1);
            }
#endif
        }
        else if ( (seg_no_rel & NEMA_VG_PRIM_MASK) == NEMA_VG_PRIM_ARC ) {
            CHECK_DATA(vb, path_info->idx_data, 3);// check if next points are available
            path_info->ctrl0.x = (vb->data)[path_info->idx_data]; (path_info->idx_data)++;// rx
            path_info->ctrl0.y = (vb->data)[path_info->idx_data]; (path_info->idx_data)++;// ry
            path_info->ctrl1.x = (vb->data)[path_info->idx_data]; (path_info->idx_data)++;// rot

#ifndef CALC_BBOX
            float transform_rot = ATAN2_(nemavg_context->path_plus_global_m[1][0], nemavg_context->path_plus_global_m[0][0]);
            path_info->ctrl1.x += transform_rot;
#endif

            // previous_data contains current p1
            path_info->previous_ctrl.x =  path_info->previous_data.x;
            path_info->previous_ctrl.y =  path_info->previous_data.y;

            path_info->seg     = seg;
        }
        else{
            //MISRA
        }

        //find p2
        if(seg_no_rel == NEMA_VG_PRIM_VLINE){
            CHECK_DATA(vb, path_info->idx_data, 1);// check if next point is available
            if (rel_coordinates != 0U ){
                path_info->p2.x = path_info->previous_data.x;
                path_info->p2.y = path_info->previous_data.y + (vb->data)[path_info->idx_data]; path_info->idx_data++;
            }
            else{
                path_info->p2.x = path_info->previous_data.x;
                path_info->p2.y = (vb->data)[path_info->idx_data]; path_info->idx_data++;
            }

        }
        else if(seg_no_rel == NEMA_VG_PRIM_HLINE){
            CHECK_DATA(vb, path_info->idx_data, 1);// check if next point is available
            if (rel_coordinates != 0U ){
                path_info->p2.x = path_info->previous_data.x + (vb->data)[path_info->idx_data]; path_info->idx_data++;
                path_info->p2.y = path_info->previous_data.y;
            }
            else{
                path_info->p2.x =  (vb->data)[path_info->idx_data]; path_info->idx_data++;
                path_info->p2.y = path_info->previous_data.y;
            }
        }
        else if (seg_no_rel == NEMA_VG_PRIM_POLYGON){
            CHECK_DATA(vb, path_info->idx_data, 1);// check if next point is available
            int num_of_coords = (int)(vb->data)[path_info->idx_data]; path_info->idx_data++;
            CHECK_DATA(vb, path_info->idx_data, (uint32_t)num_of_coords);

#ifdef CALC_BBOX
#ifndef CLIPPING
            calc_bbox(vb, &path_info->p1);
#else
            if( path_info->no_move == 1U ) {
                PRINT_F("# IMPLICIT MOVE, %f, %f\n", path_info->p1.x, path_info->p1.y);
                NEMA_VG_IF_ERROR(calc_move_geometry(path_info->p1));
                path_info->no_move = 0U;
            }
#endif //CLIPPING
#endif //CALC_BBOX

            for(int i=0; i<num_of_coords/2; i++){
                if (rel_coordinates != 0U ){
                    path_info->p2.x =  (vb->data)[path_info->idx_data] + path_info->previous_data.x; path_info->idx_data++;
                    path_info->p2.y =  (vb->data)[path_info->idx_data] + path_info->previous_data.y; path_info->idx_data++;
                }
                else{
                    path_info->p2.x = (vb->data)[path_info->idx_data]; path_info->idx_data++;
                    path_info->p2.y = (vb->data)[path_info->idx_data]; path_info->idx_data++;
                }
                path_info->previous_data.x = path_info->p2.x;
                path_info->previous_data.y = path_info->p2.y;
                path_info->previous_seg = seg;
#ifndef HW_VERTEX_MMUL
                //perform transformation if any
                if (nemavg_context->info.has_transformation != 0U)  {
                    transform_vertex(nemavg_context->path_plus_global_m, &path_info->p2);
                }
#endif

#ifdef CALC_BBOX
#ifdef CLIPPING
                NEMA_VG_IF_ERROR(calc_line_geometry(path_info->p1, path_info->p2));
#endif //CLIPPING
#endif //CALC_BBOX

#ifndef CALC_BBOX
#ifdef NEMAPVG
                if(nemavg_context->fill_rule != NEMA_VG_STROKE){
                    raster_triangle_fffx(nemavg_context->info.fan_center.fanx0fx, nemavg_context->info.fan_center.fany0fx, &path_info->p1, &path_info->p2);
                }
                else{
                    render_line(&path_info->p1, &path_info->p2);
                }
#else
                render_line(&path_info->p1, &path_info->p2);
#endif
#else
#ifndef CLIPPING
                calc_bbox(vb, &path_info->p2);
#else

#endif //CLIPPING
#endif // CALC_BBOX

                path_info->p1 = path_info->p2;
            }
            //check for path closure
            if(path_info->previous_data.x - path_info->path_start_data.x != 0.0f ||
               path_info->previous_data.y - path_info->path_start_data.y != 0.0f){
                //perform transformation if any
                path_info->p1 = path_info->p2;
                path_info->p2.x = path_info->path_start_data.x;
                path_info->p2.y = path_info->path_start_data.y;
                path_info->previous_data.x = path_info->p2.x;
                path_info->previous_data.y = path_info->p2.y;
#ifndef HW_VERTEX_MMUL
                if (nemavg_context->info.has_transformation != 0U)  {
                    transform_vertex(nemavg_context->path_plus_global_m, &path_info->p2);
                }
#endif

#ifdef CALC_BBOX
#ifdef CLIPPING
                NEMA_VG_IF_ERROR(calc_line_geometry(path_info->p1, path_info->p2));
#endif //CLIPPING
#endif //CALC_BBOX

#ifndef CALC_BBOX
#ifdef NEMAPVG
                if(nemavg_context->fill_rule != NEMA_VG_STROKE){
                    raster_triangle_fffx(nemavg_context->info.fan_center.fanx0fx, nemavg_context->info.fan_center.fany0fx, &path_info->p1, &path_info->p2);
                }
                else{
                    render_line(&path_info->p1, &path_info->p2);
                }
#else
                render_line(&path_info->p1, &path_info->p2);
#endif
#else
                // probably not needed
#ifndef CLIPPING
                calc_bbox(vb, &path_info->p2);
#endif //CLIPPING
#endif // CALC_BBOX
            }
            path_info->no_close = 0U;

            return NEMA_VG_ERR_NO_ERROR;
        }
        else if (seg_no_rel == NEMA_VG_PRIM_POLYLINE){
            CHECK_DATA(vb, path_info->idx_data, 1);// check if next point is available
            int num_of_coords = (int)(vb->data)[path_info->idx_data]; path_info->idx_data++;
            CHECK_DATA(vb, path_info->idx_data, (uint32_t)num_of_coords);

#ifdef CALC_BBOX
#ifndef CLIPPING
            calc_bbox(vb, &path_info->p1);
#else
            if( path_info->no_move == 1U ) {
                PRINT_F("# IMPLICIT MOVE, %f, %f\n", path_info->p1.x, path_info->p1.y);
                NEMA_VG_IF_ERROR(calc_move_geometry(path_info->p1));
                path_info->no_move = 0U;
            }
#endif //CLIPPING
#endif // CALC_BBOX

            for(int i=0; i<num_of_coords/2; i++){
                if (rel_coordinates != 0U ){
                    path_info->p2.x =  (vb->data)[path_info->idx_data] + path_info->previous_data.x; path_info->idx_data++;
                    path_info->p2.y =  (vb->data)[path_info->idx_data] + path_info->previous_data.y; path_info->idx_data++;
                }
                else{
                    path_info->p2.x = (vb->data)[path_info->idx_data]; path_info->idx_data++;
                    path_info->p2.y = (vb->data)[path_info->idx_data]; path_info->idx_data++;
                }
                path_info->previous_data.x = path_info->p2.x;
                path_info->previous_data.y = path_info->p2.y;
                path_info->previous_seg = seg;
                //perform transformation if any
#ifndef HW_VERTEX_MMUL
                if (nemavg_context->info.has_transformation != 0U)  {
                    transform_vertex(nemavg_context->path_plus_global_m, &path_info->p2);
                }
#endif

#ifdef CALC_BBOX
#ifdef CLIPPING
                NEMA_VG_IF_ERROR(calc_line_geometry(path_info->p1, path_info->p2));
#endif //CLIPPING
#endif //CALC_BBOX

#ifndef CALC_BBOX
#ifdef NEMAPVG
                if(nemavg_context->fill_rule != NEMA_VG_STROKE){
                    raster_triangle_fffx(nemavg_context->info.fan_center.fanx0fx, nemavg_context->info.fan_center.fany0fx, &path_info->p1, &path_info->p2);
                }
                else{
                    render_line(&path_info->p1, &path_info->p2);
                }
#else
                render_line(&path_info->p1, &path_info->p2);
#endif
#else
#ifndef CLIPPING
                calc_bbox(vb, &path_info->p2);
#endif // CLIPPING
#endif // CALC_BBOX

                path_info->p1 = path_info->p2;
            }
#ifndef CALC_BBOX
            if(nemavg_context->fill_rule != NEMA_VG_STROKE) //in stroking this need to be always set to no close
            {                                        // so bevel join won't draw
                //check for path closure
                if( path_info->previous_data.x - path_info->path_start_data.x == 0.0f &&
                    path_info->previous_data.y - path_info->path_start_data.y == 0.0f){
                    path_info->no_close = 0U;
                }
                else{
                    path_info->no_close = 1;
                }
            }
            else
            {
                    path_info->no_close = 1;
            }
#endif
            return NEMA_VG_ERR_NO_ERROR;
        }
        else{ //line, (s)quad, (s)cubic or arc
            CHECK_DATA(vb, path_info->idx_data, 2);// check if next point is available
            if (rel_coordinates != 0U ){
                path_info->p2.x =  (vb->data)[path_info->idx_data] + path_info->previous_data.x; path_info->idx_data++;
                path_info->p2.y =  (vb->data)[path_info->idx_data] + path_info->previous_data.y; path_info->idx_data++;
            }
            else{
                path_info->p2.x = (vb->data)[path_info->idx_data]; path_info->idx_data++;
                path_info->p2.y = (vb->data)[path_info->idx_data]; path_info->idx_data++;
            }
        }

#ifndef CALC_BBOX
        if(nemavg_context->fill_rule != NEMA_VG_STROKE) //in stroking this need to be always set to no close
        {
            //check for path closure
            if(path_info->p2.x - path_info->path_start_data.x == 0.0f &&
                path_info->p2.y - path_info->path_start_data.y == 0.0f){
                path_info->no_close = 0U;
            }
            else{
                path_info->no_close = 1;
            }
        }else{
                path_info->no_close = 1;
        }
#endif
    }

    path_info->previous_data.x = path_info->p2.x;
    path_info->previous_data.y = path_info->p2.y;
    path_info->previous_seg = seg;
#ifndef HW_VERTEX_MMUL
    //perform transformation if any
    if (nemavg_context->info.has_transformation != 0U)  {
        transform_vertex(nemavg_context->path_plus_global_m, &path_info->p2);
    }
#endif

    return NEMA_VG_ERR_NO_ERROR;
}

#endif // PROCESS_PATH_VERTEX_CC__
