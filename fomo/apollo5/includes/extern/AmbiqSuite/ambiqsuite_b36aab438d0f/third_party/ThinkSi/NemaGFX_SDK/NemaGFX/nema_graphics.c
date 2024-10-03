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
#include "nema_regs.h"
#include "nema_blender_intern.h"
#include "nema_programHW.h"
#include "nema_interpolators.h"
#include "nema_core.h"
#include "nema_rasterizer.h"
#include "nema_raster.h"
#include "nema_error.h"
#include "nema_error_intern.h"
#include "nema_provisional.h"
#include "nema_ringbuffer.h"
#include "nema_graphics.h"

#define NEMA_MAGICID  0x86362000U
#define TEX_BASE_OFFSET(x) (0x800U + ((x >> 4) * 8))

#define NEMA_CLIPMIN2         (0x158U)
#define NEMA_CLIPMAX2         (0x15CU)

static const nema_context_t zero_context = {0};


static void init_nema_regs(void)
{
    nema_reg_write(NEMA_CMDSTATUS, 0);
    nema_reg_write(NEMA_STATUS   , 0);
#ifdef RASTERIZER_BUG_WA
    nema_reg_write(NEMA_CGCTRL   , 4);
#endif
        // disable VMM
    nema_reg_write(NEMA_RAST_BYPASS, 1U);

    nema_set_interrupt_ctrl(0x0);
    nema_blender_init(); //preload imem
    nema_set_const_reg(0, 0);
    nema_set_const_reg(1, 0xffffffffU);

    nema_enable_depth(0);
    nema_enable_gradient(0);

    uint32_t confh = nema_readHwConfigH();
    if ( (confh & 0x200U) != 0U ) {
        nema_reg_write(0x374, 0);
        nema_reg_write(0x368, 0);
    }
}

int nema_init(void)
{
    //system powerup
    int dont_init = nema_sys_init();
    if (dont_init < 0) {
        nema_set_error(NEMA_ERR_SYS_INIT_FAILURE);
        return dont_init;
    }

    int err = nema_checkGPUPresence();
    if (err != 0) {
        return err;
    }

    nema_cl_unbind();
    nema_context = zero_context;

    if (dont_init == 0) {
        init_nema_regs();
    }

    return 0;
}

int nema_reinit(void)
{
    int err = nema_checkGPUPresence();
    if (err != 0) {
        return err;
    }

    nema_cl_unbind();
    nema_context = zero_context;

    nema_rb_reset();
    init_nema_regs();

    return 0;
}

int
nema_checkGPUPresence(void)
{
    // Check if Accelerator is there
    //-----------------------------------------------------
    uint32_t idreg = nema_reg_read(NEMA_IDREG);
    if(idreg != NEMA_MAGICID) {
        nema_set_error(NEMA_ERR_GPU_ABSENT);
        return -1;
    }

    return 0;
}

void
nema_set_error(uint32_t error)
{
    nema_context.nema_error |= error;
}

void
nema_reset_error(void)
{
    nema_context.nema_error = NEMA_ERR_NO_ERROR;
}

uint32_t
nema_get_error(void)
{
    uint32_t ret = nema_context.nema_error;
    nema_reset_error();

    return ret;
}


// ------------------------------ TEXTURES -------------------------------------
void
nema_bind_tex(  nema_tex_t texid,
                uintptr_t addr_gpu,
                uint32_t width,
                uint32_t height,
                nema_tex_format_t format,
                int32_t  stride,
                nema_tex_mode_t wrap_mode)
{
    if (texid <= NEMA_NOTEX){
        return;
    }

    int32_t stride_ = stride; //alias
    if (stride_ == -1) {
        stride_ = nema_stride_size((format & NEMA_FORMAT_MASK), wrap_mode, (int)width);
    }

    if (texid == NEMA_TEX0 ) {
        if ( ((format & NEMA_FORMAT_MASK) == NEMA_TSC4)  ||
             ((format & NEMA_FORMAT_MASK) == NEMA_TSC6)  ||
             ((format & NEMA_FORMAT_MASK) == NEMA_TSC6A)
             ) {
            //should check is HW tsc is available?
            nema_context.en_tscFB = 1;
            nema_enable_tiling(1);
        } else if ((format & NEMA_FORMAT_MASK) == NEMA_TSC12 ||
                   (format & NEMA_FORMAT_MASK) == NEMA_TSC12A
        ) {
            nema_context.en_tscFB = 1;
            nema_enable_tiling_2x2(1);
        } else {
            nema_enable_tiling(nema_context.en_ZCompr);
        }
    }

    nema_context.texs[texid].base = (uint32_t)addr_gpu;
    nema_context.texs[texid].w = (int)width;
    nema_context.texs[texid].h = (int)height;
    nema_context.texs[texid].fstride = ((unsigned)format << 24) | ((unsigned)wrap_mode << 16) | ((unsigned)stride_ & 0xffffU);
    nema_context.texs[texid].valid  = 1;
    nema_context.texs[texid].format = format;

    uint32_t tex_base_reg = /*NEMA_TEX0_BASE +*/ ((uint32_t)(int)texid)*16U;

#ifdef ADDR64BIT
    nema_multi_union_t *cmd_array = (nema_multi_union_t *)nema_cl_get_space(4);
#else
    nema_multi_union_t *cmd_array = (nema_multi_union_t *)nema_cl_get_space(3);
#endif
    if (cmd_array == NULL) {
        return;
    }

    cmd_array[ 0].u = tex_base_reg   ;  cmd_array[ 1].u = (uint32_t)addr_gpu;
    cmd_array[ 2].u = tex_base_reg+4U;  cmd_array[ 3].u = nema_context.texs[texid].fstride;
    cmd_array[ 4].u = tex_base_reg+8U;  cmd_array[ 5].u = (height<<16) | width;
#ifdef ADDR64BIT
    cmd_array[ 6].u = TEX_BASE_OFFSET(tex_base_reg) + 4U;  cmd_array[ 7].u = (uint32_t)(addr_gpu >> 32);
#endif
}

//-------------------------------------------------------------------------------
void nema_set_tex_color(uint32_t color)
{
    nema_cl_add_cmd( NEMA_TEX_COLOR , color);
}

// ------------------------------ CONSTREGS ------------------------------------
void
nema_set_const_reg(int reg, uint32_t value)
{
    uint32_t hwreg = NEMA_C0_REG + (unsigned)reg*4U;
    if (hwreg > NEMA_CMAX_REG) {
        return;
    }
    nema_cl_add_cmd( hwreg, value);
}

void
nema_tri_cull(nema_tri_cull_t cull)
{
    nema_context.tri_cul = (uint32_t)cull & (uint32_t)NEMA_CULL_ALL;
    nema_context.draw_flags   = nema_context.surface_tile | nema_context.color_grad | nema_context.tri_cul | nema_context.aa;
}

// --------------------------------- CLIP --------------------------------------
void
nema_set_clip(int32_t x,
              int32_t y,
              uint32_t w,
              uint32_t h)
{
    nema_context.prev_clip_xy[0] = x;
    nema_context.prev_clip_xy[1] = y;
    nema_context.prev_clip_wh[0] = w;
    nema_context.prev_clip_wh[1] = h;

    int x1 = x+(int)w;
    int y1 = y+(int)h;

    int32_t x_ = x;
    int32_t y_ = y;

    if (x_ < 0) {
        x_ = 0;
    }
    if (y_ < 0) {
        y_ = 0;
    }

    nema_cl_add_cmd( NEMA_CLIPMIN , ((unsigned)y_ << 16) | ((unsigned)x_ & 0xffffU) );
    nema_cl_add_cmd( NEMA_CLIPMAX , ((unsigned)y1 << 16) | ((unsigned)x1 & 0xffffU) );
}

// declared in nema_rasterizer_intern.h
void
nema_set_clip2(int32_t x,
              int32_t y,
              uint32_t w,
              uint32_t h)
{
    int x1 = x+(int)w;
    int y1 = y+(int)h;

    int x_ = nema_max2(x, 0);
    int y_ = nema_max2(y, 0);

    nema_cl_add_cmd( NEMA_CLIPMIN2 , ((unsigned)y_ << 16) | ((unsigned)x_ & 0xffffU) );
    nema_cl_add_cmd( NEMA_CLIPMAX2 , ((unsigned)y1 << 16) | ((unsigned)x1 & 0xffffU) );
}

// declared in nema_rasterizer_intern.h
int nema_supports_clip2(void)
{
    if (nema_reg_read(NEMA_CLIPMAX2) != 0u){
        return 1;
    }else{
        return 0;
    }
}

void
nema_set_clip_temp(int32_t x,
              int32_t y,
              uint32_t w,
              uint32_t h)
{
    int x1 = x+(int)w;
    int y1 = y+(int)h;

    int x_ = x;
    int y_ = y;

    if (x_ < 0) {
        x_ = 0;
    }
    if (y_ < 0) {
        y_ = 0;
    }

    nema_cl_add_cmd( NEMA_CLIPMIN , ((unsigned)y_ << 16) | ((unsigned)x_ & 0xffffU) );
    nema_cl_add_cmd( NEMA_CLIPMAX , ((unsigned)y1 << 16) | ((unsigned)x1 & 0xffffU) );
}

void
nema_set_clip_pop(void) {
    nema_set_clip(nema_context.prev_clip_xy[0], nema_context.prev_clip_xy[1], nema_context.prev_clip_wh[0], nema_context.prev_clip_wh[1]);
}

// -------------------------- RASTERIZER (GEOM) --------------------------------

void
nema_enable_gradient(int enable)
{
    nema_context.color_grad   = enable != 0 ? RAST_GRAD : 0U;
    nema_context.draw_flags   = nema_context.surface_tile | nema_context.color_grad | nema_context.tri_cul | nema_context.aa;

}

//-------------------------------------------------------------------------------
void
nema_enable_depth(int enable) {
    uint32_t config = nema_readHwConfig();

    if ( (config & NEMA_CONF_MASK_ZBUF) == 0U) {
        if ( enable != 0 ) {
            nema_context.en_sw_depth = 1;
        } else {
            nema_context.en_sw_depth = 0;
        }
    }
    else {
        if (enable != 0) {
            nema_set_depth_ctrl(NEMA_Z_COMPARE_OP_LESS | Z_LATE);
            nema_context.en_ZCompr = (config & NEMA_CONF_MASK_ZCOMPR) != 0U ? (unsigned char)1 : (unsigned char)0;
        }
        else {
            nema_set_depth_ctrl(NEMA_Z_COMPARE_OP_ALWAYS );
            nema_context.en_ZCompr = 0;

        }
        unsigned char tmp = nema_context.en_tscFB | nema_context.en_ZCompr;
        nema_enable_tiling(tmp);
    }
}

// ------------------------------- DIRTY REGIONS ------------------------------------
void
nema_get_dirty_region(int *minx, int *miny, int *maxx, int *maxy)
{
    uint32_t min, max;

    min  = nema_reg_read(NEMA_DIRTYMIN);
    uint32_t tmp = min&0xFFFFU;
    *minx = (int)tmp;
             tmp = (min>>16)&0xFFFFU;
    *miny = (int)tmp;

    max = nema_reg_read(NEMA_DIRTYMAX);
             tmp = max&0xFFFFU;
    *maxx = (int)tmp;
             tmp = (max>>16)&0xFFFFU;
    *maxy = (int)tmp;

    nema_reg_write( NEMA_DIRTYMIN, 0x0);
}

void
nema_clear_dirty_region(void)
{
    nema_cl_add_cmd( NEMA_DIRTYMIN, 0x0);
}

// -------------------------------- UTILS --------------------------------------
// Function that returns the size in bytes for each display mode
//-------------------------------------------------------------------------------
int
nema_format_size(nema_tex_format_t format)
{
    int ret = 4;

    switch((format & NEMA_FORMAT_MASK)) {
    // case NEMA_RGBX8888 :
    // case NEMA_RGBA8888 :
    // case NEMA_XRGB8888 :
    // case NEMA_ARGB8888 :
    // case NEMA_Z24_8    :
    default            : ret = 4; break;

    case NEMA_RGBA5650 :
    case NEMA_BGRA5650 :
    case NEMA_RGBA5551 :
    case NEMA_ARGB1555 :
    case NEMA_BGRA5551 :
    case NEMA_ABGR1555 :
    case NEMA_RGBA4444 :
    case NEMA_ARGB4444 :
    case NEMA_BGRA4444 :
    case NEMA_ABGR4444 :
    case NEMA_AL88     :
    case NEMA_UYVY     :
    case NEMA_RV10     :
    case NEMA_GU10     :
    case NEMA_BY10     :
    case NEMA_Z16      : ret = 2; break;
    case NEMA_RGB24    : ret = 3; break;
    case NEMA_BGR24    : ret = 3; break;
    case NEMA_RGBA0800 :
    case NEMA_L2       :
    case NEMA_L4       :
    case NEMA_L8       :
    case NEMA_RGB332   :
    case NEMA_RGBA2222 :
    case NEMA_ABGR2222 :
    case NEMA_BGRA2222 :
    case NEMA_ARGB2222 :
    case NEMA_AL44     :
    case NEMA_A1       :
    case NEMA_A2       :
    case NEMA_A1LE     :
    case NEMA_A2LE     :
    case NEMA_A4       :
    case NEMA_A8       :
    case NEMA_RV       :
    case NEMA_GU       :
    case NEMA_BY       : ret = 1; break;

    case NEMA_TSC4     :
    case NEMA_TSC6     :
    case NEMA_TSC6A    : ret = 1; break;
    case NEMA_TSC6AP   : ret = 1; break;
    case NEMA_TSC12    :
    case NEMA_TSC12A   : ret = 2; break;

    }

    return ret;
}

//-------------------------------------------------------------------------------
int
nema_stride_size(nema_tex_format_t format, nema_tex_mode_t wrap_mode, int width)
{
    int stride;
    int width_ = width;

    if ( (wrap_mode & NEMA_TEX_MORTON_ORDER) != 0U ) {
        // make width a multiple of 4
        width_ = ((width_ + 3)/4)*4;
    }

    if ( ((format & NEMA_FORMAT_MASK) == NEMA_TSC4)   ||
         ((format & NEMA_FORMAT_MASK) == NEMA_TSC6)   ||
         ((format & NEMA_FORMAT_MASK) == NEMA_TSC6A)  ||
         ((format & NEMA_FORMAT_MASK) == NEMA_TSC6AP)
    ) {
        // make width a multiple of 4
        width_ = ((width_ + 3)/4)*4;
    }

    if ( ((format & NEMA_FORMAT_MASK) == NEMA_TSC12)   ||
         ((format & NEMA_FORMAT_MASK) == NEMA_TSC12A)
    ) {
        // make width a multiple of 2
        width_ = ((width_ + 1)/2)*2;
    }

    switch((format & NEMA_FORMAT_MASK)) {
    case NEMA_L1       :
    case NEMA_L1LE     :
    case NEMA_A1LE     :
    case NEMA_A1       : stride = (width_  +7)/8;   break;
    case NEMA_L2LE     :
    case NEMA_A2LE     :
    case NEMA_A2       :
    case NEMA_L2       : stride = (width_*2+7)/8;   break;
    case NEMA_A4LE     :
    case NEMA_L4LE     :
    case NEMA_A4       :
    case NEMA_L4       : stride = (width_*4+7)/8;   break;
    case NEMA_TSC4     : stride = width_*2;         break; // ( (w/4)*8  ) 4x4 tiles, 8 bytes per tile
    case NEMA_TSC6     : stride = width_*3;         break; // ( (w/4)*12 ) 4x4 tiles, 12 bytes per tile
    case NEMA_TSC6A    : stride = width_*3;         break; // ( (w/4)*12 ) 4x4 tiles, 12 bytes per tile
    case NEMA_TSC6AP   : stride = width_*3;         break; // ( (w/4)*12 ) 4x4 tiles, 12 bytes per tile
    case NEMA_TSC12    :
    case NEMA_TSC12A   : stride = width_*3;         break; // ( (w/2)*6  ) 2x2 tiles, 6 bytes per tile
    default            : stride = width_*nema_format_size((format & NEMA_FORMAT_MASK)); break;

    }

    return stride;
}

//-------------------------------------------------------------------------------
int
nema_texture_size(nema_tex_format_t format, nema_tex_mode_t wrap_mode, int width, int height)
{
    int width_  = width;
    int height_ = height;

    if ((format & NEMA_FORMAT_MASK) == NEMA_TSC4) {
        width_  = ((width_+3)/4)*4;
        height_ = ((height_+3)/4)*4;
        return (width_*height_)/2;
    }

    if ( ((format & NEMA_FORMAT_MASK) == NEMA_TSC6)  ||
         ((format & NEMA_FORMAT_MASK) == NEMA_TSC6A) ||
         ((format & NEMA_FORMAT_MASK) == NEMA_TSC6AP)
    ){
        width_  = ((width_+3)/4)*4;
        height_ = ((height_+3)/4)*4;
        return ((width_*height_)/4)*3;
    }

    if ( ((format & NEMA_FORMAT_MASK) == NEMA_TSC12) ||
         ((format & NEMA_FORMAT_MASK) == NEMA_TSC12A)
    ){
        width_  = ((width_+1)/2)*2;
        height_ = ((height_+1)/2)*2;
        return ((width_*height_)/8)*12;
    }

    return nema_stride_size((format & NEMA_FORMAT_MASK), wrap_mode, width_)*height_;
}

//-------------------------------------------------------------------------------

uint32_t nema_rgba(unsigned char R,
                   unsigned char G,
                   unsigned char B,
                   unsigned char A) {
    return ((uint32_t)R | ((uint32_t)G<<8) | ((uint32_t)B<<16) | ((uint32_t)A<<24));
}

uint32_t nema_premultiply_rgba(uint32_t rgba) {
    uint32_t r = (rgba      )&0xffU;
    uint32_t g = (rgba >>  8)&0xffU;
    uint32_t b = (rgba >> 16)&0xffU;
    uint32_t a = (rgba >> 24)     ;

    r =(r*a)/255U;
    g =(g*a)/255U;
    b =(b*a)/255U;

    return nema_rgba((unsigned char)r, (unsigned char)g, (unsigned char)b, (unsigned char)a);
}

// ------------------------------- CONTEXT -------------------------------------
void nema_bind_src_tex(uintptr_t baseaddr_phys,
                       uint32_t width, uint32_t height,
                       nema_tex_format_t format, int32_t stride, nema_tex_mode_t mode)
{
    nema_bind_tex(NEMA_TEX1,
                  baseaddr_phys,
                  width, height,
                  format, stride, mode);
}

//-------------------------------------------------------------------------------
void nema_bind_src2_tex(uintptr_t baseaddr_phys,
                        uint32_t width, uint32_t height,
                        nema_tex_format_t format, int32_t stride, nema_tex_mode_t mode)
{
    nema_bind_tex(NEMA_TEX2,
                  baseaddr_phys,
                  width, height,
                  format, stride, mode);
}

//-------------------------------------------------------------------------------
void nema_bind_dst_tex(uintptr_t baseaddr_phys,
                       uint32_t width, uint32_t height,
                       nema_tex_format_t format, int32_t stride)
{
    nema_bind_tex(NEMA_TEX0,
                  baseaddr_phys,
                  width, height,
                  format, stride, NEMA_FILTER_PS);
}

//-------------------------------------------------------------------------------
void nema_bind_depth_buffer(uintptr_t baseaddr_phys,
                            uint32_t width, uint32_t height)
{
    nema_bind_tex(NEMA_TEX3,
                  baseaddr_phys,
                  width, height,
                  NEMA_Z24_8, -1, NEMA_FILTER_PS);
}

//-------------------------------------------------------------------------------


void nema_bind_lut_tex( uintptr_t baseaddr_phys,
                        uint32_t width, uint32_t height,
                        nema_tex_format_t format, int32_t stride, nema_tex_mode_t mode,
                        uintptr_t palette_baseaddr_phys,
                        nema_tex_format_t palette_format)
{
    uint32_t lut_size;
    switch(format) {
        case NEMA_L1:
        case NEMA_L1LE:
            lut_size = 2U;
            break;
        case NEMA_L2:
        case NEMA_L2LE:
            lut_size = 4U;
            break;
        case NEMA_L4:
        case NEMA_L4LE:
            lut_size = 16U;
            break;
        // case NEMA_L8:
        default:
            lut_size = 256U;
            break;
    }

    // LUT/PALETTE
    nema_bind_tex(NEMA_TEX2,
                  palette_baseaddr_phys,
                  lut_size,
                  1,
                  palette_format,
                  0,
                  NEMA_TEX_REPEAT);

    // INDICES
    nema_bind_tex(NEMA_TEX1,
                  baseaddr_phys,
                  width,
                  height,
                  format,
                  stride,
                  mode);
}


//-------------------------------------------------------------------------------

// ------------------------------ GRADIENT -------------------------------------
void
nema_set_gradient(float r_init, float g_init, float b_init, float a_init,
                  float r_dx, float r_dy, float g_dx, float g_dy,
                  float b_dx, float b_dy, float a_dx, float a_dy)
{
    nema_set_gradient_fx(nema_f2fx(r_init), nema_f2fx(g_init), nema_f2fx(b_init), nema_f2fx(a_init),
                         nema_f2fx(r_dx), nema_f2fx(r_dy), nema_f2fx(g_dx), nema_f2fx(g_dy),
                         nema_f2fx(b_dx), nema_f2fx(b_dy), nema_f2fx(a_dx), nema_f2fx(a_dy));
}

// -------------------------------- DEPTH --------------------------------------
void
nema_set_depth(float start, float dx, float dy) {
    // input's range is [-1, 1]
    float    dx_ = dx;
    float    dy_ = dy;
    float start_ = start;
    // make input range [0, 0xffffff]
    start_ = 0.5f*start_ + 0.5f;
    double st1 = (double)start_*(double)0xffffff;
    dx_ *= 0.5f;
    dy_ *= 0.5f;
    double dx1 = (double)dx_*(double)0xffffff;
    double dy1 = (double)dy_*(double)0xffffff;

    st1 *= (double)0x1000;
    dx1 *= (double)0x1000;
    dy1 *= (double)0x1000;

    // convert to 32.12 fixed point
    int64_t st2 = (int64_t)st1; //(1U << (12));
    int64_t dx2 = (int64_t)dx1; //(1U << (12));
    int64_t dy2 = (int64_t)dy1; //(1U << (12));

    nema_set_depth_imm( (unsigned) (((unsigned long long)st2 & 0xfffU)<<20),
                        (unsigned) (((unsigned long long)st2 >> 12) & 0xffffffffU),
                        (unsigned) (((unsigned long long)dx2 & 0xfffU)<<20),
                        (unsigned) (((unsigned long long)dx2 >> 12) & 0xffffffffU),
                        (unsigned) (((unsigned long long)dy2 & 0xfffU)<<20),
                        (unsigned) (((unsigned long long)dy2 >> 12) & 0xffffffffU));
}

// ------------------------------- DRAWING -------------------------------------
void nema_clear(uint32_t rgba8888)
{
    nema_set_blend_fill( NEMA_BL_SRC );
    nema_fill_rect(0, 0, nema_context.texs[NEMA_TEX0].w, nema_context.texs[NEMA_TEX0].h, rgba8888);
}

//-------------------------------------------------------------------------------
void nema_clear_depth(uint32_t val)
{
    nema_set_blend(NEMA_BL_SRC, NEMA_TEX3, NEMA_NOTEX, NEMA_NOTEX);
    nema_set_raster_color(val);
    nema_raster_rect(0, 0, nema_context.texs[NEMA_TEX3].w, nema_context.texs[NEMA_TEX3].h);
}

//-------------------------------------------------------------------------------
void
nema_draw_line(int x0, int y0, int x1, int y1,
               uint32_t rgba8888)
{
    nema_set_raster_color(rgba8888);
    nema_raster_line(x0, y0, x1, y1);
}

//-------------------------------------------------------------------------------
void
nema_draw_line_aa(float x0, float y0, float x1, float y1, float w,
               uint32_t rgba8888)
{
    nema_set_raster_color(rgba8888);
    float dX = x1-x0;
    float dY = y1-y0;
    float w_ = w;

    if (w_ < 1.f) {
        w_ = 1.f;
    }

#if 0
    // nema_sqrt is not accurate enough
    float L = nema_sqrt(1.f/(dX*dX+dY*dY));

    float dx =  dY*L*w*0.5f;
    float dy = -dX*L*w*0.5f;
#else
    float dx, dy;

    // if line is vertical
    if ( nema_absf(dX) < 0.5f ) {
        dy = 0.f;
        dx = w_*0.5f;
    }
    else {
        float l_tan = dY/dX;

        // nema_atan is more suitable than nema_sqrt
        float angle = nema_atan(l_tan);
        dx =  nema_sin(angle)*w_*0.5f;
        dy = -nema_cos(angle)*w_*0.5f;
    }
#endif

    float qx0 = x0+dx;
    float qx1 = x1+dx;
    float qx2 = x1-dx;
    float qx3 = x0-dx;
    float qy0 = y0+dy;
    float qy1 = y1+dy;
    float qy2 = y1-dy;
    float qy3 = y0-dy;

    uint32_t prev_aa = nema_enable_aa_flags(RAST_AA_MASK);

    nema_raster_quad_fx(nema_f2fx(qx0), nema_f2fx(qy0),
                        nema_f2fx(qx1), nema_f2fx(qy1),
                        nema_f2fx(qx2), nema_f2fx(qy2),
                        nema_f2fx(qx3), nema_f2fx(qy3) );

    (void)nema_enable_aa_flags(prev_aa);
}

//-------------------------------------------------------------------------------
void nema_draw_rounded_rect(int x0, int y0, int w, int h, int r, uint32_t rgba8888) {
    int r_ = r;
    if ( (w/2) < r_) {
        r_ = w/2;
    }
    if ( (h/2) < r_) {
        r_ = h/2;
    }

    if (r_ <= 0 || w <= 2 || h <= 2) {
        nema_draw_rect(x0, y0, w, h, rgba8888);
        return;
    }

    int x=0, y=r_;
    int d=3-(2*r_);

    nema_set_raster_color(rgba8888);

    int x_l = x0+r_;     int y_t = y0+r_;
    int x_r = x0+w-r_-1; int y_b = y0+h-r_-1;

    bool horizontal_lines_needed = (w != r_*2) ? true : false;
    bool vertical_lines_needed   = (h != r_*2) ? true : false;

    // Calculate cmds needed for the operation
    int cmd_needed = 1; // for nema_set_raster_color
    if(horizontal_lines_needed){
        cmd_needed += (int)NEMA_RASTER_LINE_SIZE *2;
    }
    if(vertical_lines_needed){
        cmd_needed += (int)NEMA_RASTER_LINE_SIZE *2;
    }

    if (d<0) {
        d += /*(4*x)+*/6;
    }
    else {
        d += (-4*(/*x-*/y))+10;
        y -= 1;
    }
    x++;

    int x_start = x, y_start = y, d_start = d;
    while(x<=y) {
        if (x != y) {
            cmd_needed += (int)NEMA_RASTER_PIXEL_SIZE * 4;
        }
        cmd_needed += (int)NEMA_RASTER_PIXEL_SIZE * 4;

        if (d<0) {
            d += (4*x)+6;
        }
        else {
            d += (4*(x-y))+10;
            y -= 1;
        }
        x++;
    }

    int ret = nema_cl_enough_space(cmd_needed);
    if(ret < 0){
        return ;
    }

    // Execute Operation if there is enpugh space left
    if (horizontal_lines_needed)
    {
        //top
        nema_raster_line(x_l,y0    ,x_r,y0    );
        //bottom
        nema_raster_line(x_l,y0+h-1,x_r,y0+h-1);
    }
    if (vertical_lines_needed)
    {
        //left
        nema_raster_line(x0    ,y_t,x0    ,y_b);
        //right
        nema_raster_line(x0+w-1,y_t,x0+w-1,y_b);
    }

    x = x_start;
    y = y_start;
    d = d_start;

    while(x<=y) {
        if (/*x != 0 &&*/ x != y) {
            nema_raster_pixel(x_l-x,y_t-y);
            nema_raster_pixel(x_r+y,y_t-x);
            nema_raster_pixel(x_l-y,y_b+x);
            nema_raster_pixel(x_r+x,y_b+y);
        }

        nema_raster_pixel(x_l-y,y_t-x);
        nema_raster_pixel(x_r+x,y_t-y);
        nema_raster_pixel(x_l-x,y_b+y);
        nema_raster_pixel(x_r+y,y_b+x);

        if (d<0) {
            d += (4*x)+6;
        }
        else {
            d += (4*(x-y))+10;
            y -= 1;
        }
        x++;
    }
}

//-------------------------------------------------------------------------------
void nema_draw_circle(int x, int y, int r, uint32_t rgba8888){
    nema_draw_rounded_rect(x-r, y-r, 2*r, 2*r, r, rgba8888);
}

//-------------------------------------------------------------------------------
void nema_draw_circle_aa(float x, float y, float r, float w, uint32_t rgba8888) {
    nema_set_raster_color(rgba8888);
    nema_raster_stroked_circle_aa(x, y, r, w);
}

//-------------------------------------------------------------------------------
void nema_draw_rect(int x, int y, int w, int h, uint32_t rgba8888) {

    // pt0 _______ pt1
    //    |       |
    //    |       |
    // pt3|_______|pt2

    int x0 = x    ; int y0 = y;
    int x1 = x+w-1; int y1 = y;
    int x2 = x+w-1; int y2 = y+h-1;
    int x3 = x    ; int y3 = y+h-1;

    nema_set_raster_color(rgba8888);

    if (w <= 2 || h <= 2 ) {
        if (w <= 0 || h <= 0) {
            return;
        }
        nema_raster_rect(x0, y0, w, h);
        return;
    }

    // Calculate cmds needed for the operation
    int cmd_needed = (int)NEMA_RASTER_LINE_SIZE * 4;
    int ret = nema_cl_enough_space(cmd_needed);
    if(ret < 0){
        return ;
    }

    // Execute oparation if there is enough space left
    nema_raster_line(x0, y0, x1, y1);
    nema_raster_line(x3, y3, x2, y2);
    ++y0;
    ++y1;
    --y2;
    --y3;
    nema_raster_line(x0, y0, x3, y3);
    nema_raster_line(x1, y1, x2, y2);
}

//-------------------------------------------------------------------------------
void
nema_fill_triangle(int x0, int y0,
                   int x1, int y1,
                   int x2, int y2,
                   uint32_t rgba8888) {
    nema_set_raster_color(rgba8888);
    nema_raster_triangle_fx(nema_i2fx(x0), nema_i2fx(y0),
                            nema_i2fx(x1), nema_i2fx(y1),
                            nema_i2fx(x2), nema_i2fx(y2));
}

//-------------------------------------------------------------------------------
void nema_fill_rounded_rect(int x0, int y0, int w, int h, int r, uint32_t rgba8888) {
    nema_set_raster_color(rgba8888);
    nema_raster_rounded_rect(x0, y0, w, h, r);
}

//-------------------------------------------------------------------------------
void nema_fill_circle(int x, int y, int r, uint32_t rgba8888) {
    nema_fill_rounded_rect(x-r, y-r, 2*r, 2*r, r, rgba8888);
}

//-------------------------------------------------------------------------------
void nema_fill_circle_aa(float x, float y, float r, uint32_t rgba8888) {
    nema_set_raster_color(rgba8888);
    nema_raster_circle_aa(x, y, r);
}

//-------------------------------------------------------------------------------
void nema_fill_rect(int x, int y, int w, int h , uint32_t rgba8888) {
    nema_set_raster_color(rgba8888);
    nema_raster_rect(x, y, w, h);
}

//-------------------------------------------------------------------------------
void
nema_fill_quad(int x0, int y0,
               int x1, int y1,
               int x2, int y2,
               int x3, int y3,
               uint32_t rgba8888) {
    nema_set_raster_color(rgba8888);
    nema_raster_quad_fx(nema_i2fx(x0), nema_i2fx(y0),
                        nema_i2fx(x1), nema_i2fx(y1),
                        nema_i2fx(x2), nema_i2fx(y2),
                        nema_i2fx(x3), nema_i2fx(y3));
}


void
nema_fill_rect_f(float x, float y,
                 float w, float h,
                 uint32_t rgba8888) {

    nema_set_raster_color(rgba8888);
    nema_raster_rect_f(x, y, w, h);
}


void
nema_fill_quad_f(float x0, float y0,float x1, float y1,
                 float x2, float y2,float x3, float y3,
                 uint32_t rgba8888) {

    nema_set_raster_color(rgba8888);
    nema_raster_quad_f( x0, y0, x1, y1,
                        x2, y2, x3, y3);
}


void
nema_fill_triangle_f(float x0, float y0,
                     float x1, float y1,
                     float x2, float y2,
                     uint32_t rgba8888) {

    nema_set_raster_color(rgba8888);
    nema_raster_triangle_f(x0, y0, x1, y1, x2, y2);
}

// ------------------------------- BLITTING ------------------------------------
void nema_blit_rect(int x, int y, int w, int h)
{
    nema_set_matrix_translate((float)x,(float)y);
    nema_raster_rect(x, y, w, h);
}

void nema_blit_subrect(int dst_x, int dst_y, int w, int h, int src_x, int src_y)
{
    int w1 = dst_x-src_x;
    int h1 = dst_y-src_y;
    nema_set_matrix_translate((float)w1, (float)h1);
    nema_raster_rect(dst_x, dst_y, w, h);
}

//-------------------------------------------------------------------------------
void nema_blit(int x, int y)
{
    nema_set_matrix_translate((float)x, (float)y);
    nema_raster_rect(x, y, nema_context.texs[NEMA_TEX1].w, nema_context.texs[NEMA_TEX1].h);
}

void nema_blit_rounded(int x, int y, int r)
{
    nema_set_matrix_translate((float)x,(float)y);
    nema_raster_rounded_rect(x, y, nema_context.texs[NEMA_TEX1].w, nema_context.texs[NEMA_TEX1].h, r);
}

//-------------------------------------------------------------------------------
void nema_blit_rect_fit(int x, int y, int w, int h)
{
    nema_set_matrix_scale((float)x, (float)y, (float)w, (float)h,
                            0.f, 0.f, (float)nema_context.texs[NEMA_TEX1].w, (float)nema_context.texs[NEMA_TEX1].h);
    nema_raster_rect(x, y, w, h);
}

//-------------------------------------------------------------------------------
void nema_blit_subrect_fit( int dst_x, int dst_y, int dst_w, int dst_h,
                            int src_x, int src_y, int src_w, int src_h)
{
    nema_set_matrix_scale(  (float)dst_x, (float)dst_y,
                            (float)dst_w, (float)dst_h,
                            (float)src_x, (float)src_y,
                            (float)src_w, (float)src_h);
    nema_raster_rect(dst_x, dst_y, dst_w, dst_h);
}

//-------------------------------------------------------------------------------
void nema_blit_rotate_partial(int sx,
                              int sy,
                              int sw,
                              int sh,
                              int x,
                              int y,
                              uint32_t rotation)
{
    int tmp;
    int sw_ = sw;
    int sh_ = sh;

    nema_matrix3x3_t m;
    m[2][0] =  0.f;   m[2][1] =  0.f;     m[2][2] = 1.f;

    //TODO: optimize with LUT
    switch(rotation) {
    default:
    case NEMA_ROT_000_CCW :
    case NEMA_ROT_180_CCW | NEMA_MIR_HOR | NEMA_MIR_VERT:
        //--------------------------------------------------
        m[0][0] =  1.f;   m[0][1] =  0.f;     m[0][2] = -(float)x;
        m[1][0] =  0.f;   m[1][1] =  1.f;     m[1][2] = -(float)y;
        //--------------------------------------------------
        break;
    case NEMA_ROT_090_CCW :
    case NEMA_ROT_270_CCW | NEMA_MIR_HOR | NEMA_MIR_VERT:
        //--------------------------------------------------
                                                         tmp = y+sw_;
        m[0][0] =  0.f;   m[0][1] = -1.f;     m[0][2] =  (float)tmp;
        m[1][0] =  1.f;   m[1][1] =  0.f;     m[1][2] = -(float)x;
        //--------------------------------------------------
        tmp  = sw_;
        sw_  = sh_;
        sh_  = tmp;
        break;
    case NEMA_ROT_180_CCW :
    case NEMA_MIR_HOR | NEMA_MIR_VERT:
        //--------------------------------------------------
                                                         tmp = x+sw_;
        m[0][0] = -1.f;   m[0][1] =  0.f;     m[0][2] =  (float)tmp;
                                                         tmp = y+sh_;
        m[1][0] =  0.f;   m[1][1] = -1.f;     m[1][2] =  (float)tmp;
        //--------------------------------------------------
        break;
    case NEMA_ROT_270_CCW :
    case NEMA_ROT_090_CCW | NEMA_MIR_HOR | NEMA_MIR_VERT:
        //--------------------------------------------------
        m[0][0] =  0.f;   m[0][1] =  1.f;     m[0][2] = -(float)y;
                                                         tmp = x+sh_;
        m[1][0] = -1.f;   m[1][1] =  0.f;     m[1][2] =  (float)tmp;
        //--------------------------------------------------
        tmp = sw_; sw_ = sh_; sh_ = tmp;
        break;
    case NEMA_MIR_HOR:
    case NEMA_ROT_180_CCW | NEMA_MIR_VERT:
        //--------------------------------------------------
                                                         tmp = x+sw_;
        m[0][0] = -1.f;   m[0][1] =  0.f;     m[0][2] =  (float)tmp;
        m[1][0] =  0.f;   m[1][1] =  1.f;     m[1][2] = -(float)y;
        //--------------------------------------------------
        break;
    case NEMA_ROT_090_CCW | NEMA_MIR_HOR:
    case NEMA_ROT_270_CCW | NEMA_MIR_VERT:
        //--------------------------------------------------
                                                         tmp = y+sw_;
        m[0][0] =  0.f;   m[0][1] = -1.f;     m[0][2] =  (float)tmp;
                                                         tmp = x+sh_;
        m[1][0] = -1.f;   m[1][1] =  0.f;     m[1][2] =  (float)tmp;
        //--------------------------------------------------
        tmp = sw_; sw_ = sh_; sh_ = tmp;
        break;
    case NEMA_ROT_180_CCW | NEMA_MIR_HOR:
    case NEMA_MIR_VERT:
        //--------------------------------------------------
        m[0][0] =  1.f;   m[0][1] =  0.f;     m[0][2] = -(float)x;
                                                         tmp = y+sh_;
        m[1][0] =  0.f;   m[1][1] = -1.f;     m[1][2] =  (float)tmp;
        //--------------------------------------------------
        break;
    case NEMA_ROT_270_CCW | NEMA_MIR_HOR:
    case NEMA_ROT_090_CCW | NEMA_MIR_VERT:
        //--------------------------------------------------
        m[0][0] =  0.f;   m[0][1] =  1.f;     m[0][2] = -(float)y;
        m[1][0] =  1.f;   m[1][1] =  0.f;     m[1][2] = -(float)x;
        //--------------------------------------------------
        tmp = sw_; sw_ = sh_; sh_ = tmp;
        break;
    }

    m[0][2] += (float)sx;
    m[1][2] += (float)sy;

    nema_set_matrix(m);
    nema_raster_rect(x, y, sw_, sh_);
}

//-------------------------------------------------------------------------------
void nema_blit_rotate(int x,
                      int y,
                      uint32_t rotation)
{
    int w = nema_context.texs[NEMA_TEX1].w;
    int h = nema_context.texs[NEMA_TEX1].h;

    nema_blit_rotate_partial(0, 0, w, h, x, y, rotation);
}

//-------------------------------------------------------------------------------
void
nema_blit_rotate_pivot( float cx, float cy, float px, float py, float degrees_cw )
{
    float w = (float)nema_context.texs[NEMA_TEX1].w;
    float h = (float)nema_context.texs[NEMA_TEX1].h;

    float x0 = -px;
    float y0 = -py;
    float x1 = x0+w;
    float y1 = y0;
    float x2 = x0+w;
    float y2 = y0+h;
    float x3 = x0;
    float y3 = y0+h;

    nema_matrix3x3_t m;
    nema_mat3x3_load_identity(m);
    nema_mat3x3_rotate(m, degrees_cw);
    nema_mat3x3_translate(m, cx, cy);
    nema_mat3x3_mul_vec(m, &x0, &y0);
    nema_mat3x3_mul_vec(m, &x1, &y1);
    nema_mat3x3_mul_vec(m, &x2, &y2);
    nema_mat3x3_mul_vec(m, &x3, &y3);
    nema_blit_quad_fit(x0, y0,
                       x1, y1,
                       x2, y2,
                       x3, y3);
}

//-------------------------------------------------------------------------------

void nema_blit_tri_fit (float dx0, float dy0, int v0,
                        float dx1, float dy1, int v1,
                        float dx2, float dy2, int v2)
{
    if((v0 > 3 || v0 < 0) ||
       (v1 > 3 || v1 < 0) ||
       (v2 > 3 || v2 < 0))
    {
        return;
    }

    float tx[4] = {0.f,      (float)nema_context.texs[NEMA_TEX1].w, (float)nema_context.texs[NEMA_TEX1].w, 0.f};
    float ty[4] = {0.f, 0.f, (float)nema_context.texs[NEMA_TEX1].h, (float)nema_context.texs[NEMA_TEX1].h};

    nema_interpolate_tx_ty(dx0, dy0, 1.f, tx[v0], ty[v0],
                           dx1, dy1, 1.f, tx[v1], ty[v1],
                           dx2, dy2, 1.f, tx[v2], ty[v2],
                           nema_context.texs[NEMA_TEX1].w, nema_context.texs[NEMA_TEX1].h);


    nema_raster_triangle_fx(nema_f2fx(dx0), nema_f2fx(dy0),
                            nema_f2fx(dx1), nema_f2fx(dy1),
                            nema_f2fx(dx2), nema_f2fx(dy2));
}

//-------------------------------------------------------------------------------

void nema_blit_tri_uv  (float dx0, float dy0, float dw0,
                        float dx1, float dy1, float dw1,
                        float dx2, float dy2, float dw2,
                        float sx0, float sy0,
                        float sx1, float sy1,
                        float sx2, float sy2
                        )
{
    nema_interpolate_tx_ty(dx0, dy0, dw0, sx0, sy0,
                           dx1, dy1, dw1, sx1, sy1,
                           dx2, dy2, dw2, sx2, sy2,
                           nema_context.texs[NEMA_TEX1].w, nema_context.texs[NEMA_TEX1].h);


    nema_raster_triangle_fx(nema_f2fx(dx0), nema_f2fx(dy0),
                            nema_f2fx(dx1), nema_f2fx(dy1),
                            nema_f2fx(dx2), nema_f2fx(dy2));
}

//-------------------------------------------------------------------------------

void nema_blit_quad_fit(float dx0, float dy0,
                        float dx1, float dy1,
                        float dx2, float dy2,
                        float dx3, float dy3)
{
    nema_matrix3x3_t m;

    if ( nema_mat3x3_quad_to_rect(nema_context.texs[NEMA_TEX1].w,
                                  nema_context.texs[NEMA_TEX1].h,
                                  dx0, dy0,
                                  dx1, dy1,
                                  dx2, dy2,
                                  dx3, dy3,
                                  m) != 0) {
        return;
    }

    nema_set_matrix(m);

    nema_raster_quad_fx(
        nema_f2fx(dx0), nema_f2fx(dy0),
        nema_f2fx(dx1), nema_f2fx(dy1),
        nema_f2fx(dx2), nema_f2fx(dy2),
        nema_f2fx(dx3), nema_f2fx(dy3));
}

void nema_blit_subrect_quad_fit(float dx0, float dy0,
                                float dx1, float dy1,
                                float dx2, float dy2,
                                float dx3, float dy3,
                                int sx, int sy,
                                int sw, int sh)
{
    nema_matrix3x3_t m;

    if ( nema_mat3x3_quad_to_rect(sw , sh,
                                  dx0, dy0,
                                  dx1, dy1,
                                  dx2, dy2,
                                  dx3, dy3,
                                  m) != 0) {
        return;
    }

    nema_mat3x3_translate(m, (float)sx, (float)sy);
    nema_set_matrix(m);

    nema_raster_quad_fx(
        nema_f2fx(dx0), nema_f2fx(dy0),
        nema_f2fx(dx1), nema_f2fx(dy1),
        nema_f2fx(dx2), nema_f2fx(dy2),
        nema_f2fx(dx3), nema_f2fx(dy3));
}

void nema_fill_triangle_strip_f(float* vertices, int num_vertices, int stride, uint32_t rgba8888)
{
    if(num_vertices < 3){
        return;
    }

    nema_set_raster_color(rgba8888);
    nema_raster_triangle_strip_f(vertices, num_vertices, stride);
}

void nema_fill_triangle_fan_f(float* vertices, int num_vertices, int stride, uint32_t rgba8888)
{
    if(num_vertices < 3){
        return;
    }

    nema_set_raster_color(rgba8888);
    nema_raster_triangle_fan_f(vertices, num_vertices, stride);
}

void nema_blit_quad_m(float dx0, float dy0,
                      float dx1, float dy1,
                      float dx2, float dy2,
                      float dx3, float dy3, nema_matrix3x3_t m)
{
    nema_set_matrix(m);
    nema_raster_quad_fx(
        nema_f2fx(dx0), nema_f2fx(dy0),
        nema_f2fx(dx1), nema_f2fx(dy1),
        nema_f2fx(dx2), nema_f2fx(dy2),
        nema_f2fx(dx3), nema_f2fx(dy3));
}

const char* nema_get_sw_device_name(void)
{
#ifdef NEMAPVG
    return "NemaPVG";
#else
    return "NemaP";
#endif
}
