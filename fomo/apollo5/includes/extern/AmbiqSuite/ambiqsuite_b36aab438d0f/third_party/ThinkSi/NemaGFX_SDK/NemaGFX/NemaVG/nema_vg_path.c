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
#include "nema_vg_path.h"
#include "nema_vg.h"
#include "nema_vg_p.h"

static inline void
clear_flag(nema_vg_path_t* path, uint32_t flag)
{
    path->flags &= ~flag;
}

static inline void
set_flag(nema_vg_path_t* path, uint32_t flag)
{
    path->flags |= flag;
}

static inline void
reset_flags(nema_vg_path_t* path)
{
    path->flags = NEMA_VG_PATH_NO_FLAGS;
}

uint8_t
is_identity_matrix(nema_matrix3x3_t m)
{
    for(int i = 0 ; i<3; i++){
        for(int j = 0 ; j<3; j++){
            if(i == j){
                if(m[i][j] != 1.f){
                    return 0;
                }

            }
            else{
                if(m[i][j] != 0.f){
                    return 0;
                }
            }
        }
    }
    return 1;
}

void
nema_vg_path_destroy(NEMA_VG_PATH_HANDLE path)
{
    if(path == NULL){
        nema_vg_set_error(NEMA_VG_ERR_BAD_HANDLE);
        return;
    }
    // call this to deallocate shape
    nema_vg_path_clear(path);
    nema_host_free(path);
}

void
nema_vg_path_clear(NEMA_VG_PATH_HANDLE path)
{
    if(path == NULL){
        nema_vg_set_error(NEMA_VG_ERR_BAD_HANDLE);
        return;
    }
    GET_PATH;

    nema_mat3x3_load_identity(_path->m);
    reset_flags(_path); // This is needed both here and in create fuction in case this is called from application to clear a path that has an implicit alloc
}

NEMA_VG_PATH_HANDLE
nema_vg_path_create(void)
{
    NEMA_VG_PATH_HANDLE path = (NEMA_VG_PATH_HANDLE)(nema_host_malloc(sizeof(nema_vg_path_t)));
    GET_PATH;

    reset_flags(_path);
    nema_vg_path_clear(_path);

    return path;
}

void
nema_vg_path_set_shape_and_bbox(NEMA_VG_PATH_HANDLE path, const size_t seg_size, const uint8_t* seg, const size_t data_size, const nema_vg_float_t* data, const nema_vg_float_t *bbox) {
    if(path == NULL){
        nema_vg_set_error(NEMA_VG_ERR_BAD_HANDLE);
        return;
    }
    if(seg == NULL){
        nema_vg_set_error(NEMA_VG_ERR_BAD_BUFFER);
        return;
    }
    if(data == NULL){
        nema_vg_set_error(NEMA_VG_ERR_BAD_BUFFER);
        return;
    }
    if(bbox == NULL){
        nema_vg_set_error(NEMA_VG_ERR_BAD_BUFFER);
        return;
    }
    if(seg_size <= 0U)
    {
        nema_vg_set_error(NEMA_VG_ERR_INVALID_ARGUMENTS);
        return;
    }
    if(data_size <= 0U)
    {
        nema_vg_set_error(NEMA_VG_ERR_INVALID_ARGUMENTS);
        return;
    }

    GET_PATH;

    nema_vbuf_t_ *vb = &_path->shape;
    if(vb == NULL)
    {
        nema_vg_set_error(NEMA_VG_ERR_BAD_BUFFER);
        return;
    }

    vb->seg = seg;
    vb->data = data;
    vb->seg_size = seg_size;
    vb->data_size = data_size;

    // bbox_clear(vb);

    vb->bbox.min.x = (nema_vg_float_t)nema_floor_(bbox[0]);
    vb->bbox.min.y = (nema_vg_float_t)nema_floor_(bbox[1]);
    vb->bbox.max.x = (nema_vg_float_t)nema_ceil_(bbox[2]);
    vb->bbox.max.y = (nema_vg_float_t)nema_ceil_(bbox[3]);
    vb->bbox.w = vb->bbox.max.x - vb->bbox.min.x;
    vb->bbox.h = vb->bbox.max.y - vb->bbox.min.y;
}

void
nema_vg_path_set_shape(NEMA_VG_PATH_HANDLE path, const size_t seg_size, const uint8_t* seg, const size_t data_size, const nema_vg_float_t* data)
{
    if(path == NULL){
        nema_vg_set_error(NEMA_VG_ERR_BAD_HANDLE);
        return;
    }

    GET_PATH;

    nema_vbuf_t_ *vb = &_path->shape;

    vb->seg = seg;
    vb->data = data;
    vb->seg_size = seg_size;
    vb->data_size = data_size;

    calculate_aabb_per_shape(path);
}

void
nema_vg_path_set_matrix(NEMA_VG_PATH_HANDLE path, nema_matrix3x3_t m)
{
    if(path == NULL){
        nema_vg_set_error(NEMA_VG_ERR_BAD_HANDLE);
        return;
    }

    GET_PATH;
    if (m == NULL) {
        clear_flag(_path, NEMA_VG_PATH_HAS_TRANSFORMATION);
        nema_vg_set_error(NEMA_VG_ERR_BAD_HANDLE);
        return;
    }

    for(int i = 0; i < 3; ++i ){
        for(int j = 0; j < 3; ++j ){
            _path->m[i][j] =  m[i][j];
        }
    }

    if(is_identity_matrix(_path->m) == (uint8_t)0){
        set_flag(_path, NEMA_VG_PATH_HAS_TRANSFORMATION);
    }else{
        clear_flag(_path, NEMA_VG_PATH_HAS_TRANSFORMATION);
    }
}

uint32_t
nema_vg_path_get_bbox(nema_vbuf_t_ *vb, nema_vg_float_t* xmin, nema_vg_float_t* ymin, nema_vg_float_t* xmax, nema_vg_float_t *ymax) {

    if (vb != NULL){
        *xmin = vb->bbox.min.x;
        *ymin = vb->bbox.min.y;
        *xmax = vb->bbox.max.x;
        *ymax = vb->bbox.max.y;
    }

    return NEMA_VG_ERR_NO_ERROR;
}
