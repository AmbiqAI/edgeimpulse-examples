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
#include "nema_vg_context.h"
#include "nema_vg_p.h"
#include "nema_math.h"
#include "nema_vg_path.h"

#include <math.h>

static void
bbox_clear(nema_vbuf_t_* vb)
{
    vb->bbox.min.x = 65535.f;
    vb->bbox.min.y = 65535.f;
    vb->bbox.max.x = -65535.0f;
    vb->bbox.max.y = -65535.0f;
    vb->bbox.transformed_p1.y = 0.0f;
    vb->bbox.transformed_p1.x = 0.0f;
    vb->bbox.transformed_p2.y = 0.0f;
    vb->bbox.transformed_p2.x = 0.0f;
    vb->bbox.transformed_p3.y = 0.0f;
    vb->bbox.transformed_p3.x = 0.0f;
    vb->bbox.transformed_p4.y = 0.0f;
    vb->bbox.transformed_p4.x = 0.0f;
    vb->bbox.transformed_w = 0.0f;
    vb->bbox.transformed_h = 0.0f;
    vb->bbox.transformed_min.x = 65535.f;
    vb->bbox.transformed_min.y = 65535.f;
    vb->bbox.transformed_max.x = -65535.0f;
    vb->bbox.transformed_max.y = -65535.0f;
}

static inline void
calc_bbox(nema_vbuf_t_* vb, nema_vg_vertex_t_ *p)
{
    vb->bbox.min.x = nema_min2(p->x, vb->bbox.min.x);
    vb->bbox.min.y = nema_min2(p->y, vb->bbox.min.y);
    vb->bbox.max.x = nema_max2(p->x, vb->bbox.max.x);
    vb->bbox.max.y = nema_max2(p->y, vb->bbox.max.y);
}

/*********************************************************
 * Functions for finding axis-aligned bounding boxes     *
 * for elliptical arcs and quadratic/cubic bezier curves *
 *********************************************************/
#define NEMA_EPS (0.0000001f)

static inline
uint32_t ctrl_pt_in_bounds(nema_vg_float_t* ctrl_pt, nema_vg_float_t* bounds)
{
    if (ctrl_pt[0] >= bounds[0] && ctrl_pt[0] <= bounds[2] && ctrl_pt[1] >= bounds[1] && ctrl_pt[1] <= bounds[3]) {
        return 1U;
    } else {
        return 0U;
    }
}

static inline
float eval_quad_bezier(float t, float p0, float p1, float p2)
{
    float it = 1.0f-t;
    return it*it*p0 + 2.0f*it*t*p1 + t*t*p2;
}

static inline
float eval_cubic_bezier(float t, float p0, float p1, float p2, float p3)
{
    float it = 1.0f-t;
    return it*it*it*p0 + 3.0f*it*it*t*p1 + 3.0f*it*t*t*p2 + t*t*t*p3;
}

static void quadratic_aabb(nema_vbuf_t_* vb, nema_vg_info_t_* curve)
{
    float a, b, t, v;
    float v0[2], v1[2], v2[2]; // hold the curve end and control points
    float bounds[4];

    // Start by setting the curve end points as the bounding box (p1 already consumed at the first segment)
    vb->bbox.min.x = nema_min2(vb->bbox.min.x, curve->p2.x);
    vb->bbox.min.y = nema_min2(vb->bbox.min.y, curve->p2.y);
    vb->bbox.max.x = nema_max2(vb->bbox.max.x, curve->p2.x);
    vb->bbox.max.y = nema_max2(vb->bbox.max.y, curve->p2.y);

    bounds[0] = vb->bbox.min.x;
    bounds[1] = vb->bbox.min.y;
    bounds[2] = vb->bbox.max.x;
    bounds[3] = vb->bbox.max.y;

    v1[0] = curve->ctrl0.x;
    v1[1] = curve->ctrl0.y;

    // If control points are inside the bounding box created by the end points, we're done
    if (ctrl_pt_in_bounds(v1, bounds) == 1U) {
        return;
    }

    v0[0] = curve->p1.x;
    v0[1] = curve->p1.y;
    v2[0] = curve->p2.x;
    v2[1] = curve->p2.y;

    // Find roots of the first derivative of the Bezier curve (for X and Y)
    // Evaluate the Bezier at these points and update the bounding box
    for (size_t i = 0U; i < 2U; i++) {
        // More info for these equations https://pomax.github.io/bezierinfo/#extremities
        a = 2.0f * v1[i] - 2.0f * v0[i];
        b = 2.0f * v2[i] - 2.0f * v1[i];

        float ba = b - a;
        if (nema_absf(ba) > NEMA_EPS) { // do not divide by zero!!!
            t = -a / ba;
            if (t > NEMA_EPS && t < 1.0f - NEMA_EPS) {
                v = eval_quad_bezier(t, v0[i], v1[i], v2[i]);
                bounds[i] = nema_min2(bounds[i], v);
                bounds[2U+i] = nema_max2(bounds[2U+i], v);
            }
        }
    }

    vb->bbox.min.x = bounds[0];
    vb->bbox.min.y = bounds[1];
    vb->bbox.max.x = bounds[2];
    vb->bbox.max.y = bounds[3];
}

static void cubic_aabb(nema_vbuf_t_* vb, nema_vg_info_t_* curve)
{
    size_t count;
    float roots[2] = {0.0f, 0.0f};
    float a, b, c, b2ac, t, v;
    float v0[2], v1[2], v2[2], v3[2]; // hold the curve end and control points
    float bounds[4];

    // Start by setting the curve end points as the bounding box (p1 already consumed at the first segment)
    vb->bbox.min.x = nema_min2(vb->bbox.min.x, curve->p2.x);
    vb->bbox.min.y = nema_min2(vb->bbox.min.y, curve->p2.y);
    vb->bbox.max.x = nema_max2(vb->bbox.max.x, curve->p2.x);
    vb->bbox.max.y = nema_max2(vb->bbox.max.y, curve->p2.y);

    bounds[0] = vb->bbox.min.x;
    bounds[1] = vb->bbox.min.y;
    bounds[2] = vb->bbox.max.x;
    bounds[3] = vb->bbox.max.y;

    v1[0] = curve->ctrl0.x;
    v1[1] = curve->ctrl0.y;
    v2[0] = curve->ctrl1.x;
    v2[1] = curve->ctrl1.y;

    // If control points are inside the bounding box created by the end points, we're done
    if (ctrl_pt_in_bounds(v1, bounds) == 1U) {
        if (ctrl_pt_in_bounds(v2, bounds) == 1U) {
            return;
        }
    }

    v0[0] = curve->p1.x;
    v0[1] = curve->p1.y;
    v3[0] = curve->p2.x;
    v3[1] = curve->p2.y;

    // Find roots of the first derivative of the Bezier curve (for X and Y)
    // Evaluate the Bezier at these points and update the bounding box
    for (size_t i = 0U; i < 2U; i++) {
        // More info for these equations https://pomax.github.io/bezierinfo/#extremities
        a = -3.0f * v0[i] + 9.0f * v1[i] - 9.0f * v2[i] + 3.0f * v3[i];
        b = 6.0f * v0[i] - 12.0f * v1[i] + 6.0f * v2[i];
        c = 3.0f * v1[i] - 3.0f * v0[i];
        count = 0U;

        if (nema_absf(a) < NEMA_EPS) {
            // The quadratic polynomial at^2+bt+c if a is zero, is degenarated to a linear one
            // with simpler root computation (same as in quadratic_aabb function)
            if (nema_absf(b) > NEMA_EPS) { // do not divide by zero!!!
                t = -c / b;
                if (t > NEMA_EPS && t < 1.0f - NEMA_EPS) {
                    roots[count] = t; count++;
                }
            }
        } else {
            // Find roots of the quadratic polynomial
            b2ac = b*b - 4.0f*c*a;
            if (b2ac > NEMA_EPS) {
                float sq_b2ac = sqrtf(b2ac);

                t = (-b + sq_b2ac) / (2.0f * a);
                if (t > NEMA_EPS && t < 1.0f - NEMA_EPS) {
                    roots[count] = t; count++;
                }

                t = (-b - sq_b2ac) / (2.0f * a);
                if (t > NEMA_EPS && t < 1.0f - NEMA_EPS) {
                    roots[count] = t; count++;
                }
            }
        }

        for (size_t j = 0U; j < count; j++) {
            v = eval_cubic_bezier(roots[j], v0[i], v1[i], v2[i], v3[i]);
            bounds[i] = nema_min2(bounds[i], v);
            bounds[2U+i] = nema_max2(bounds[2U+i], v);
        }
    }

    vb->bbox.min.x = bounds[0];
    vb->bbox.min.y = bounds[1];
    vb->bbox.max.x = bounds[2];
    vb->bbox.max.y = bounds[3];
}

// Compute the normalized angle (interval [0.0, 2.0*NEMA_PI])between the vector (1,0) x-axis and the vector (bx, by)
static inline
float getAnglef(float bx, float by)
{
    return fmodf(2.0f * NEMA_PI + (by > 0.0f ? 1.0f : -1.0f) * acosf( bx / sqrtf(bx * bx + by * by) ), 2.0f * NEMA_PI);
}

static void arc_aabb(nema_vbuf_t_* vb, nema_vg_info_t_* path_info)
{
    float rx = path_info->ctrl0.x;
    float ry = path_info->ctrl0.y;

    // Ensure radii are non-zero (else treat this as a straight line)
    if (nema_float_is_zero(rx) || nema_float_is_zero(ry)) {
        calc_bbox(vb, &path_info->p2);
        return;
    }

    // Ensure radii are positive
    rx = nema_absf(rx);
    ry = nema_absf(ry);

    float cx, cy;
    float rot_deg = path_info->ctrl1.x; // rotation on x-axis in degrees
    float x0p = path_info->previous_ctrl.x;
    float y0p = path_info->previous_ctrl.y;
    float x1p = path_info->previous_data.x;
    float y1p = path_info->previous_data.y;

    // Conversion from endpoint to center parameterization
    // The following code illustrates the process of computing the ellipse centers
    // more at https://www.khronos.org/registry/OpenVG/specs/openvg-1.1.pdf (Appendix A: Mathematics of Ellipses)

    // Convert rotation angle from degrees to radians
    float rot_rad = nema_deg_to_rad(rot_deg);
    // Pre-compute rotation matrix entries
    float cosine = cosf(rot_rad);
    float sine = sinf(rot_rad);
    // Transform (x0, y0) and (x1, y1) into unit space
    // using (inverse) rotate, followed by (inverse) scale
    float x0 = ( x0p*cosine + y0p*sine  )/rx;
    float y0 = (-x0p*sine   + y0p*cosine)/ry;
    float x1 = ( x1p*cosine + y1p*sine  )/rx;
    float y1 = (-x1p*sine   + y1p*cosine)/ry;

    // Compute differences and averages
    float dx = x0 - x1;
    float dy = y0 - y1;
    float xm = (x0 + x1)*0.5f;
    float ym = (y0 + y1)*0.5f;

    // Solve for intersecting unit circles
    float dsq = dx*dx + dy*dy;
    if (dsq == 0.f) {
        return; // Points are coincident!
    }

    float disc = 1.f/dsq - 1.f/4.f;
    if (disc < 0.f) {
        disc = 0.f;
        float scale = sqrtf(dsq/4.f);
        rx *= scale;
        ry *= scale;

        x0 = ( x0p*cosine + y0p*sine  )/rx;
        y0 = (-x0p*sine   + y0p*cosine)/ry;
        x1 = ( x1p*cosine + y1p*sine  )/rx;
        y1 = (-x1p*sine   + y1p*cosine)/ry;
        dx = x0 - x1;
        dy = y0 - y1;
        xm = (x0 + x1)*0.5f;
        ym = (y0 + y1)*0.5f;
    }

    float s = sqrtf(disc);
    float sdx = s*dx;
    float sdy = s*dy;

    float ucx = xm + sdy;
    float ucy = ym - sdx;

    // Find the angle between the endpoints of the arc path
    float theta00_rad = atan2f(y0-ucy, x0-ucx);
    float theta01_rad = atan2f(y1-ucy, x1-ucx);
    float theta00_deg = nema_rad_to_deg(theta00_rad);
    float theta01_deg = nema_rad_to_deg(theta01_rad);

    float dtheta0_deg = theta01_deg - theta00_deg;
    if (dtheta0_deg < 0.f) {
        dtheta0_deg+=360.f;
    }

    uint32_t sweep, large;
    if ((path_info->seg & NEMA_VG_ARC_CW) != 0U) {
        sweep = 1U;
    } else {
        sweep = 0U;
    }

    if ((path_info->seg & NEMA_VG_ARC_LARGE) != 0U) {
        large = 1U;
    } else {
        large = 0U;
    }

    // condition taken from findUnitCircles function
    if (! (((dtheta0_deg <  180.f) && ((sweep ^ large) != 0U)) ||
           ((dtheta0_deg >= 180.f) && ((sweep ^ large) == 0U)))) {
        ucx = xm - sdy;
        ucy = ym + sdx;
    }

    ucx *= rx;
    ucy *= ry;

    // Here we have computed the ellipse centers
    cx = ucx*cosine - ucy*sine;
    cy = ucx*sine   + ucy*cosine;

    nema_vg_vertex_t_ min_coord;
    nema_vg_vertex_t_ max_coord;

#ifdef ARC_LOOSE_AABB // Not defined by default
    // this is the non-tight aabb which contains the whole ellipse (not just the arc)
    // we create a square aabb subtracting from the center of the ellipse the
    // major radius - can we use it for lower cpu util?
    float major_radius = nema_max2(rx, ry);
    min_coord.x = cx - major_radius;
    min_coord.y = cy - major_radius;
    max_coord.x = cx + major_radius;
    max_coord.y = cy + major_radius;
#else
    // More at http://fridrich.blogspot.com/2011/06/bounding-box-of-svg-elliptical-arc.html
    // Now that we have found the ellipse center we have to found the extremes of the ellipse and which of them are inside the
    // angle created between the starting and ending points
    // First we have to compute for which theta the ellipse reaches its local extremes and which are these extremes points
    // The next step is to find which of the extreme points corresponding to xmin, xmax, ymin or ymax lie on the arc created
    // by the starting and ending ponts and which not
    // if a point of the extreme ones lie on the arc, this will be the extreme of the arc and if not the new extremes will be
    // the starting and ending points

    float xmin, ymin, xmax, ymax;
    float txmin, txmax, tymin, tymax; // theta for which the ellipse reaches its local extremes

    // Special handling when the rotation is 0, pi/2, 3*pi/2
    // as the extreme points are minor/major radius away from the center ellipse
    if (rot_deg == 0.f || rot_deg == 180.f) {
        xmin = cx - rx;
        txmin = getAnglef(-rx, 0.0f);
        xmax = cx + rx;
        txmax = getAnglef(rx, 0.0f);
        ymin = cy - ry;
        tymin = getAnglef(0.0f, -ry);
        ymax = cy + ry;
        tymax = getAnglef(0.0f, ry);
    } else if (rot_deg == 90.f || rot_deg == 270.f) {
        xmin = cx - ry;
        txmin = getAnglef(-ry, 0.0f);
        xmax = cx + ry;
        txmax = getAnglef(ry, 0.0f);
        ymin = cy - rx;
        tymin = getAnglef(0.0f, -rx);
        ymax = cy + rx;
        tymax = getAnglef(0.0f, rx);
    } else {
        float tmp1, tmp2;

        // two solutions for x
        txmin = -atanf(ry*tanf(rot_rad)/rx);
        txmax = NEMA_PI - atanf (ry*tanf(rot_rad)/rx);

        // compute the extremes of the ellipse based on the two solutions
        tmp1 = rx*cosf(txmin);
        tmp1 *= cosf(rot_rad);
        tmp2 = ry*sinf(txmin);
        tmp2 *= sinf(rot_rad);
        xmin = cx + tmp1 - tmp2;

        tmp1 = rx*cosf(txmax);
        tmp1 *= cosf(rot_rad);
        tmp2 = ry*sinf(txmax);
        tmp2 *= sinf(rot_rad);
        xmax = cx + tmp1 - tmp2;

        if (xmin > xmax) {
            float temp = xmin;
            xmin = xmax;
            xmax = temp;

            temp = txmin;
            txmin = txmax;
            txmax = temp;
        }

        // compute the angles with the x-axis of the lines going through the center of the ellipse and our extreme points
        tmp1 = rx*cosf(txmin);
        tmp1 *= sinf(rot_rad);
        tmp2 = ry*sinf(txmin);
        tmp2 *= cosf(rot_rad);
        float tmpY = cy + tmp1 + tmp2;
        txmin = getAnglef(xmin - cx, tmpY - cy);

        tmp1 = rx*cosf(txmax);
        tmp1 *= sinf(rot_rad);
        tmp2 = ry*sinf(txmax);
        tmp2 *= cosf(rot_rad);
        tmpY = cy + tmp1 + tmp2;
        txmax = getAnglef(xmax - cx, tmpY - cy);

        // two solutions for y
        tymin = atanf(ry/(tanf(rot_rad)*rx));
        tymax = atanf(ry/(tanf(rot_rad)*rx))+NEMA_PI;

        // compute the extremes of the ellipse based on the two solutions
        tmp1 = rx*cosf(tymin);
        tmp1 *= sinf(rot_rad);
        tmp2 = ry*sinf(tymin);
        tmp2 *= cosf(rot_rad);
        ymin = cy + tmp1 + tmp2;

        tmp1 = rx*cosf(tymax);
        tmp1 *= sinf(rot_rad);
        tmp2 = ry*sinf(tymax);
        tmp2 *= cosf(rot_rad);
        ymax = cy + tmp1 + tmp2;

        if (ymin > ymax) {
            float temp = ymin;
            ymin = ymax;
            ymax = temp;

            temp = tymin;
            tymin = tymax;
            tymax = temp;
        }

        // Compute the angles with the x-axis of the lines going through the center of the ellipse and our extreme points
        tmp1 = rx*cosf(tymin);
        tmp1 *= cosf(rot_rad);
        tmp2 = ry*sinf(tymin);
        tmp2 *= sinf(rot_rad);
        float tmpX = cx + tmp1 - tmp2;
        tymin = getAnglef(tmpX - cx, ymin - cy);

        tmp1 = rx*cosf(tymax);
        tmp1 *= cosf(rot_rad);
        tmp2 = ry*sinf(tymax);
        tmp2 *= sinf(rot_rad);
        tmpX = cx + tmp1 - tmp2;
        tymax = getAnglef(tmpX - cx, ymax - cy);
    }

    // Once we know the angles of the extremes, we still need to calculate the angles of the starting and the ending points
    float angle1 = getAnglef(x0p - cx, y0p - cy);
    float angle2 = getAnglef(x1p - cx, y1p - cy);

    if (sweep == 0U) {
        float temp = angle1;
        angle1 = angle2;
        angle2 = temp;
    }

    uint32_t otherArc = 0U;
    if (angle1 > angle2) {
        float temp = angle1;
        angle1 = angle2;
        angle2 = temp;
        otherArc = 1U;
    }

    // Check which extreme thetas are contained in the angle of the starting and ending points (lie on the arc and not only on the ellipse)
    if (((otherArc == 0U) && (angle1 > txmin || angle2 < txmin)) || ((otherArc != 0U) && !(angle1 > txmin || angle2 < txmin))) {
        xmin = x0p < x1p ? x0p : x1p;
    }
    if (((otherArc == 0U) && (angle1 > txmax || angle2 < txmax)) || ((otherArc != 0U) && !(angle1 > txmax || angle2 < txmax))) {
        xmax = x0p > x1p ? x0p : x1p;
    }
    if (((otherArc == 0U) && (angle1 > tymin || angle2 < tymin)) || ((otherArc != 0U) && !(angle1 > tymin || angle2 < tymin))) {
        ymin = y0p < y1p ? y0p : y1p;
    }
    if (((otherArc == 0U) && (angle1 > tymax || angle2 < tymax)) || ((otherArc != 0U) && !(angle1 > tymax || angle2 < tymax))) {
        ymax = y0p > y1p ? y0p : y1p;
    }
    min_coord.x = xmin;
    min_coord.y = ymin;
    max_coord.x = xmax;
    max_coord.y = ymax;
#endif //ARC_TIGHT_AABB

    calc_bbox(vb, &min_coord);
    calc_bbox(vb, &max_coord);
}
#undef NEMA_EPS
#undef NEMA_PI
/*** End of aabb functions ***/

#define CALC_BBOX

// define HW_VERTEX_MMUL
// so that we don't do transformations inside process_path_vertex
#ifdef HW_VERTEX_MMUL
#define HW_VERTEX_MMUL_WAS_DEFINED
#else
#define HW_VERTEX_MMUL
#endif

#include "process_path_vertex.cc"

#ifndef HW_VERTEX_MMUL_WAS_DEFINED
#undef HW_VERTEX_MMUL
#endif

#undef CALC_BBOX

// Public Interface
void calculate_aabb_per_shape(NEMA_VG_PATH_HANDLE path)
{
    GET_PATH;
    nema_vbuf_t_ *vb = &_path->shape;

    bbox_clear(vb);
    nema_vg_info_t_ path_info = {0};

    path_info.no_move=1;
    path_info.no_close=1;
    path_info.new_path=1;

    // in case NEMA_VG_PRIM_MOVE is not explicitly set as the first segment use the 0,0 as the starting point
    if ((vb->seg_size > 0U) && ((vb->seg[0] & NEMA_VG_NO_REL_MASK) != NEMA_VG_PRIM_MOVE)) {
        nema_vg_vertex_t_ p = {0.0f, 0.0f};
        calc_bbox(vb, &p);
    }

    for(size_t i = 0U; i < vb->seg_size; ++i)
    {
        if ( process_path_vertex(vb, vb->seg[i], &path_info) != NEMA_VG_ERR_NO_ERROR ) {
#ifndef DO_NOT_SET_ERROR
            nema_vg_set_error(NEMA_VG_ERR_INVALID_VERTEX_DATA);
#endif
            return;
        }

        uint8_t seg_no_rel = (uint8_t)(vb->seg[i] & NEMA_VG_NO_REL_MASK); //remove rel from seg

        if(seg_no_rel == NEMA_VG_PRIM_BEZIER_CUBIC  || seg_no_rel == NEMA_VG_PRIM_BEZIER_SCUBIC ){
            cubic_aabb(vb, &path_info);
        }
        else if ( seg_no_rel == NEMA_VG_PRIM_MOVE) {
            calc_bbox(vb, &path_info.p2); //p2
        }
        else if( seg_no_rel == NEMA_VG_PRIM_LINE) {
            calc_bbox(vb, &path_info.p2); //p2
        }
        else if ( seg_no_rel == NEMA_VG_PRIM_VLINE ) {
            calc_bbox(vb, &path_info.p2); //p2
        }
        else if ( seg_no_rel == NEMA_VG_PRIM_HLINE ) {
            calc_bbox(vb, &path_info.p2); //p2
        }
        else if(seg_no_rel == NEMA_VG_PRIM_BEZIER_QUAD || seg_no_rel == NEMA_VG_PRIM_BEZIER_SQUAD ){
            quadratic_aabb(vb, &path_info);
        }
        else if ( (seg_no_rel & NEMA_VG_PRIM_MASK) == NEMA_VG_PRIM_ARC ) {
            arc_aabb(vb, &path_info);
        }
        else if(seg_no_rel == NEMA_VG_PRIM_POLYGON){
            // bbox has been calculated in process_path_vertex
        }
        else if(seg_no_rel == NEMA_VG_PRIM_POLYLINE){
            // bbox has been calculated in process_path_vertex
        }
        else{
            //MISRA
        }
    }

    vb->bbox.min.x = (float)nema_floor_(vb->bbox.min.x); // vb->min.x>>16;         //
    vb->bbox.min.y = (float)nema_floor_(vb->bbox.min.y); // vb->min.y>>16;         //
    vb->bbox.max.x = (float)nema_ceil_(vb->bbox.max.x); //(vb->max.x+0xffff)>>16; //
    vb->bbox.max.y = (float)nema_ceil_(vb->bbox.max.y); //(vb->max.y+0xffff)>>16; //
    vb->bbox.w = vb->bbox.max.x - vb->bbox.min.x;
    vb->bbox.h = vb->bbox.max.y - vb->bbox.min.y;
}
