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
#include "nema_core.h"
#include "nema_raster.h"
#include "nema_shaderSpecific.h"
#include "nema_blender_intern.h"
// #include "nema_programHW.h"


static void load_matrix_3x2(unsigned char matrix[2][3]) {
    uint32_t val;

    val  = matrix[0][0];  val <<=8;
    val |= matrix[0][1];  val <<=8;
    val |= matrix[0][2];
    nema_set_const_reg(1, val);

    val  = matrix[1][0];  val <<=8;
    val |= matrix[1][1];  val <<=8;
    val |= matrix[1][2];
    nema_set_const_reg(2, val);
}


static void load_matrix_3x3(unsigned char matrix[3][3]) {
    uint32_t val;

    val  = matrix[0][0];  val <<=8;
    val |= matrix[0][1];  val <<=8;
    val |= matrix[0][2];
    nema_set_const_reg(1, val);

    val  = matrix[1][0];  val <<=8;
    val |= matrix[1][1];  val <<=8;
    val |= matrix[1][2];
    nema_set_const_reg(2, val);

    val  = matrix[2][0];  val <<=8;
    val |= matrix[2][1];  val <<=8;
    val |= matrix[2][2];
    nema_set_const_reg(3, val);
}


static void load_matrix_4x4(unsigned char matrix[4][4]) {
    uint32_t val;

    val  = matrix[0][0];  val <<=8;
    val |= matrix[0][1];  val <<=8;
    val |= matrix[0][2];  val <<=8;
    val |= matrix[0][3];
    nema_set_const_reg(0, val);

    val  = matrix[1][0];  val <<=8;
    val |= matrix[1][1];  val <<=8;
    val |= matrix[1][2];  val <<=8;
    val |= matrix[1][3];
    nema_set_const_reg(1, val);

    val  = matrix[2][0];  val <<=8;
    val |= matrix[2][1];  val <<=8;
    val |= matrix[2][2];  val <<=8;
    val |= matrix[2][3];
    nema_set_const_reg(2, val);

    val  = matrix[3][0];  val <<=8;
    val |= matrix[3][1];  val <<=8;
    val |= matrix[3][2];  val <<=8;
    val |= matrix[3][3];
    nema_set_const_reg(3, val);
}

//@function nema_blit_yuv
//@brief Blit with an Image in yuv format
//@param int mode desc:source image surface type
//@param int src_addr_Y desc:base addr of Y component
//@param int src_addr_U desc:base addr of U component
//@param int src_addr_V desc:base addr of V component
//@param int src_xres desc:source x resolution
//@param int src_yres desc:source y resolution
//@param int src_stride desc:source stride
//@param int dst_x desc:destination x
//@param int dst_y desc:destination y
//@return void desc:void
//------------------------------------------------------------------------------------------------------------------------
void
nema_blit_yuv(uintptr_t src_addr_Y,
              uintptr_t src_addr_U,
              uintptr_t src_addr_V,
              uint32_t src_xres,
              uint32_t src_yres,
              int src_stride,
              int dst_x,
              int dst_y)
{
    //------------------------------------------------------------------------------------------
    uint32_t cmd_blityuv[10] = {  0x000010ebU, 0x00000000U,   // readtex.divtx2ty2 $v0, $v1, 1
                                  0x1000110bU, 0x00000000U,   // readtex.txty $v2, $v1, 2
                                  0x1800118bU, 0x0000100cU,   // { readtex.txty $v3, $v1, 3 ! radd.rgb $v0, $v0, $v2 }
                                  0x00001000U, 0x0000180cU,   // radd.rgb $v0, $v0, $v3
                                  0x08001002U, 0x90000010U};  // { pixout $v1, 0 ! yuv2rgb $v0, $v0 ! yield }


    nema_set_blend_blit(NEMA_BL_SRC); //setup matmult and ROB blender (if present) correctly
    nema_set_frag_ptr(((0x9400U | (PRELOAD_ADDR+1U-5U))<<16));
    nema_load_frag_shader(cmd_blityuv, 5, 0);
    //------------------------------------------------------------------------------------------
    nema_bind_tex(NEMA_TEX1, src_addr_Y, src_xres   , src_yres   , NEMA_BY, src_stride  , NEMA_FILTER_PS);
    nema_bind_tex(NEMA_TEX2, src_addr_U, src_xres/2U, src_yres/2U, NEMA_GU, src_stride/2, NEMA_FILTER_PS);
    nema_bind_tex(NEMA_TEX3, src_addr_V, src_xres/2U, src_yres/2U, NEMA_RV, src_stride/2, NEMA_FILTER_PS);
    //------------------------------------------------------------------------------------------
    nema_blit_rect(dst_x, dst_y, (int)src_xres, (int)src_yres);
}

//@function nema_blit_yuv10
//@brief Blit with an Image in yuv 10-bitformat
//@param int mode desc:source image surface type
//@param int src_addr_Y desc:base addr of Y component
//@param int src_addr_U desc:base addr of U component
//@param int src_addr_V desc:base addr of V component
//@param int src_xres desc:source x resolution
//@param int src_yres desc:source y resolution
//@param int src_stride desc:source stride
//@param int dst_x desc:destination x
//@param int dst_y desc:destination y
//@return void desc:void
//------------------------------------------------------------------------------------------------------------------------
void
nema_blit_yuv10(uintptr_t src_addr_Y,
              uintptr_t src_addr_U,
              uintptr_t src_addr_V,
              uint32_t src_xres,
              uint32_t src_yres,
              int src_stride,
              int dst_x,
              int dst_y)
{
    //------------------------------------------------------------------------------------------
    uint32_t cmd_blityuv[10] = {  0x000010ebU, 0x00000000U,   // readtex.divtx2ty2 $v0, $v1, 1
                                  0x1000110bU, 0x00000000U,   // readtex.txty $v2, $v1, 2
                                  0x1800118bU, 0x0000100cU,   // { readtex.txty $v3, $v1, 3 ! radd.rgb $v0, $v0, $v2 }
                                  0x00001000U, 0x0000180cU,   // radd.rgb $v0, $v0, $v3
                                  0x08001002U, 0x90000010U};  // { pixout $v1, 0 ! yuv2rgb $v0, $v0 ! yield }
    nema_set_blend_blit(NEMA_BL_SRC); //setup matmult and ROB blender (if present) correctly
    nema_set_frag_ptr(((0x9400U | (PRELOAD_ADDR+1U-5U))<<16));
    nema_load_frag_shader(cmd_blityuv, 5, 0);
    //------------------------------------------------------------------------------------------
    nema_bind_tex(NEMA_TEX1, src_addr_Y, src_xres   , src_yres   , NEMA_BY10, src_stride  , NEMA_FILTER_PS);
    nema_bind_tex(NEMA_TEX2, src_addr_U, src_xres/2U, src_yres/2U, NEMA_GU10, src_stride/2, NEMA_FILTER_PS);
    nema_bind_tex(NEMA_TEX3, src_addr_V, src_xres/2U, src_yres/2U, NEMA_RV10, src_stride/2, NEMA_FILTER_PS);
    //------------------------------------------------------------------------------------------
    nema_blit_rect(dst_x, dst_y, (int)src_xres, (int)src_yres);
}

//------------------------------------------------------------------------------------------------------------------------
static inline void interpolate_tx_ty(
    int x0, int y0, int tx0, int ty0,
    int x1, int y1, int tx1, int ty1,
    int x2, int y2, int tx2, int ty2,
    int tex_width, int tex_height )
{
    nema_interpolate_tx_ty(
        (float)x0, (float)y0, 1.f, (float)tx0, (float)ty0,
        (float)x1, (float)y1, 1.f, (float)tx1, (float)ty1,
        (float)x2, (float)y2, 1.f, (float)tx2, (float)ty2,
        tex_width, tex_height
    );
}
//------------------------------------------------------------------------------------------------------------------------
void
nema_blit_blur(unsigned char matrix[3][3], int x, int y, int w, int h)
{
    uint32_t cmd_blur[8] = {  0x1000109bU, 0x00000000U,  // readtex.inctx $v2, $v1, 1
                              0x1000109bU, 0x03046404U,  // { readtex.inctx $v2, $v1, 1 ! rmadd.rgb $v3, $v2, $cv1, $cv0 }
                              0x1000108bU, 0x0f549c45U,  // { readtex.txty $v2, $v1, 1 ! rmadd.rgb $v3, $v2, $cv2, $v3 ! amov.a $v3, $v2 }
                              0x00001002U, 0x8f7cdc65U}; // { pixout $v0, 0 ! rmadd.rgb $v3, $v2, $cv3, $v3 ! amov.a $v3, $v3 ! yield }

    nema_set_blend_blit(NEMA_BL_SRC); //setup matmult and ROB blender (if present) correctly
    nema_set_frag_ptr(((0x9400U | (PRELOAD_ADDR+1U-4U))<<16));
    nema_load_frag_shader(cmd_blur, 4, 0);
    //----------------------------------------------------------------------------
    load_matrix_3x3(matrix);
    //----------------------------------------------------------------------------

    int x_1 = x+1;
    nema_set_matrix_translate((float)x_1, (float)y);
    nema_raster_rect(x, y, w, h);
}

//------------------------------------------------------------------------------------------------------------------------
void
nema_blit_edge(unsigned char matrix[3][3], int x, int y, int w, int h)
{
    uint32_t cmd_edge[8] = {  0x1000109bU, 0x00000000U,  // readtex.inctx $v2, $v1, 1
                              0x1000109bU, 0x03046404U,  // { readtex.inctx $v2, $v1, 1 ! rmadd.rgb $v3, $v2, $cv1, $cv0 }
                              0x1000108bU, 0x0c500041U,  // { readtex.txty $v2, $v1, 1 ! amov.a $v3, $v2 }
                              0x00001002U, 0x8f7c5c75U}; // { pixout $v0, 0 ! rmsub.rgb $v3, $v2, $cv1, $v3 ! amov.a $v3, $v3 ! yield }
    nema_set_blend_blit(NEMA_BL_SRC); //setup matmult and ROB blender (if present) correctly
    nema_set_frag_ptr(((0x9400U | (PRELOAD_ADDR+1U-4U))<<16));
    nema_load_frag_shader(cmd_edge, 4, 0);
    //----------------------------------------------------------------------------
    load_matrix_3x3(matrix);
    //----------------------------------------------------------------------------

    int x_1 = x+1;
    nema_set_matrix_translate((float)x_1, (float)y);
    nema_raster_rect(x, y, w, h);
}

//------------------------------------------------------------------------------------------------------------------------
void
nema_blit_warp(uintptr_t warpBase, uint32_t warpW, uint32_t warpH, nema_tex_format_t warpMode, int warpStride, int x, int y)
{
    uint32_t cmd_map[4] = {  0x0000108bU, 0x00000000U,   // readtex.txty $v0, $v1, 1
                             0x08001002U, 0x80000009U};  // { pixout $v1, 0 ! rmov.rgb $v0, $v0 ! amov.a $v0, $v0 ! yield }

    nema_set_blend_blit(NEMA_BL_SRC); //setup matmult and ROB blender (if present) correctly
    nema_set_frag_ptr(0xA800U | ((0x9400U | (PRELOAD_ADDR+1U-2U))<<16));
    nema_load_frag_shader(cmd_map, 2, 0);
    //------------------------------------------------
    nema_bind_tex(NEMA_TEX2,
                  warpBase,
                  warpW,
                  warpH,
                  warpMode,
                  warpStride,
                  NEMA_FILTER_PS);
    //------------------------------------------------
    nema_blit(x, y);
}

//------------------------------------------------------------------------------------------------------------------------
void
nema_blit_mean(int x, int y, int w, int h)
{
    uint32_t cmd_mean[20] = {  0x1000109bU, 0x03046440U,   // readtex.inctx $v2, $v1, 1
                               0x100010abU, 0x03046444U,   // { readtex.incty $v2, $v1, 1 ! rmadd.rgb $v3, $v2, $cv1, $cv0 }
                               0x100010bbU, 0x03045c44U,   // { readtex.dectx $v2, $v1, 1 ! rmadd.rgb $v3, $v2, $cv1, $v3 }
                               0x100010bbU, 0x03049c44U,   // { readtex.dectx $v2, $v1, 1 ! rmadd.rgb $v3, $v2, $cv2, $v3 }
                               0x100010cbU, 0x03049c44U,   // { readtex.decty $v2, $v1, 1 ! rmadd.rgb $v3, $v2, $cv2, $v3 }
                               0x100010cbU, 0x03049c44U,   // { readtex.decty $v2, $v1, 1 ! rmadd.rgb $v3, $v2, $cv2, $v3 }
                               0x1000109bU, 0x03045c44U,   // { readtex.inctx $v2, $v1, 1 ! rmadd.rgb $v3, $v2, $cv1, $v3 }
                               0x1000109bU, 0x03049c44U,   // { readtex.inctx $v2, $v1, 1 ! rmadd.rgb $v3, $v2, $cv2, $v3 }
                               0x1000108bU, 0x03049c44U,   // { readtex.txty $v2, $v1, 1 ! rmadd.rgb $v3, $v2, $cv2, $v3 }
                               0x10001002U, 0x80049c45U};  // { pixout $v2, 0 ! rmadd.rgb $v0, $v2, $cv2, $v3 ! amov.a $v0, $v2 ! yield }
    nema_set_blend_blit(NEMA_BL_SRC); //setup matmult and ROB blender (if present) correctly
    nema_set_frag_ptr(0);
    nema_load_frag_shader(cmd_mean, 10, 0);
    //----------------------------------------------------------------------------
    uint8_t matrix[2][3]={ {0x1d,0x1d,0x1d},
                           {0x1c,0x1c,0x1c} };
    //----------------------------------------------------------------------------
    load_matrix_3x2(matrix);
    //----------------------------------------------------------------------------
    nema_blit_rect(x, y, w, h);
}

//------------------------------------------------------------------------------------------------------------------------
void
nema_blit_gauss(int x, int y, int w, int h)
{
    uint32_t cmd_gauss[20] = {  0x1000109bU, 0x0304e440U,  // readtex.inctx $v2, $v1, 1
                                0x100010abU, 0x0304e444U,  // { readtex.incty $v2, $v1, 1 ! rmadd.rgb $v3, $v2, $cv3, $cv0 }
                                0x100010bbU, 0x03049c44U,  // { readtex.dectx $v2, $v1, 1 ! rmadd.rgb $v3, $v2, $cv2, $v3 }
                                0x100010bbU, 0x03045c44U,  // { readtex.dectx $v2, $v1, 1 ! rmadd.rgb $v3, $v2, $cv1, $v3 }
                                0x100010cbU, 0x03049c44U,  // { readtex.decty $v2, $v1, 1 ! rmadd.rgb $v3, $v2, $cv2, $v3 }
                                0x100010cbU, 0x03045c44U,  // { readtex.decty $v2, $v1, 1 ! rmadd.rgb $v3, $v2, $cv1, $v3 }
                                0x1000109bU, 0x03049c44U,  // { readtex.inctx $v2, $v1, 1 ! rmadd.rgb $v3, $v2, $cv2, $v3 }
                                0x1000109bU, 0x03045c44U,  // { readtex.inctx $v2, $v1, 1 ! rmadd.rgb $v3, $v2, $cv1, $v3 }
                                0x1000108bU, 0x03049c44U,  // { readtex.txty $v2, $v1, 1 ! rmadd.rgb $v3, $v2, $cv2, $v3 }
                                0x10001002U, 0x80045c45U}; // { pixout $v2, 0 ! rmadd.rgb $v0, $v2, $cv1, $v3 ! amov.a $v0, $v2 ! yield }
    nema_set_blend_blit(NEMA_BL_SRC); //setup matmult and ROB blender (if present) correctly
    nema_set_frag_ptr(0);
    nema_load_frag_shader(cmd_gauss, 10, 0);
    //----------------------------------------------------------------------------
    uint8_t matrix[3][3]={ {0x10, 0x10, 0x10},
                           {0x20, 0x20, 0x20},
                           {0x40, 0x40, 0x40} };

    load_matrix_3x3(matrix);
    //----------------------------------------------------------------------------
    nema_blit_rect(x, y, w, h);
}

//------------------------------------------------------------------------------------------------------------------------
void
nema_blit_sharpen_gauss(int x, int y, int w, int h, float sharpen)
{
    uint32_t cmd_sharpen_gauss[32] = {  0x000010abU, 0x02046000U,  // readtex.incty $v0, $v1, 1
                                        0x000010bbU, 0x02046004U,  // { readtex.dectx $v0, $v1, 1 ! rmadd.rgb $v2, $v0, $cv1, $cv0 }
                                        0x000010bbU, 0x03046004U,  // { readtex.dectx $v0, $v1, 1 ! rmadd.rgb $v3, $v0, $cv1, $cv0 }
                                        0x000010cbU, 0x02045004U,  // { readtex.decty $v0, $v1, 1 ! rmadd.rgb $v2, $v0, $cv1, $v2 }
                                        0x000010cbU, 0x03045804U,  // { readtex.decty $v0, $v1, 1 ! rmadd.rgb $v3, $v0, $cv1, $v3 }
                                        0x0000109bU, 0x02045004U,  // { readtex.inctx $v0, $v1, 1 ! rmadd.rgb $v2, $v0, $cv1, $v2 }
                                        0x0000109bU, 0x03045804U,  // { readtex.inctx $v0, $v1, 1 ! rmadd.rgb $v3, $v0, $cv1, $v3 }
                                        0x000010abU, 0x02045004U,  // { readtex.incty $v0, $v1, 1 ! rmadd.rgb $v2, $v0, $cv1, $v2 }
                                        0x000010bbU, 0x03045804U,  // { readtex.dectx $v0, $v1, 1 ! rmadd.rgb $v3, $v0, $cv1, $v3 }
                                        0x0000108bU, 0x0204a404U,  // { readtex.txty $v0, $v1, 1 ! rmadd.rgb $v2, $v2, $cv2, $cv0 }
                                        0x00001088U, 0x03045604U,  // rmadd.rgb $v3, $v3, $cv1, $v2
                                        0x00001088U, 0x03045804U,  // rmadd.rgb $v3, $v0, $cv1, $v3
                                        0x00001088U, 0x02075814U,  // rmsub.imm1 $v2, $v0, 1, $v3
                                        0x00001088U, 0x03074614U,  // rmsub.imm1 $v3, $v3, 1, $v0
                                        0x00001000U, 0x00075814U,  // rmsub.imm1 $v0, $v0, 1, $v3
                                        0x00001002U, 0x8000100dU}; // { pixout $v0, 0 ! radd.rgb $v0, $v0, $v2 ! amov.a $v0, $v0 ! yield }
    nema_set_blend_blit(NEMA_BL_SRC); //setup matmult and ROB blender (if present) correctly
    nema_set_frag_ptr(0);
    nema_load_frag_shader(cmd_sharpen_gauss, 16, 0);
    //----------------------------------------------------------------------------
    uint8_t matrix[2][3]={ {0x40, 0x40, 0x40},
                           {0x80, 0x80, 0x80} };
    //----------------------------------------------------------------------------
    load_matrix_3x2(matrix);
    //----------------------------------------------------------------------------
    int x_1 = x-1;
    nema_set_matrix_translate((float)x_1, (float)y);

    nema_blender_init();
}

//------------------------------------------------------------------------------------------------------------------------
void
nema_blit_sharpen_laplace(int x, int y, int w, int h, float sharpen)
{
    // uint32_t sharp = (uint32_t)(0xff * sharpen);
    uint32_t cmd_sharpen_laplace[34] = {  0x000010bbU, 0x03074400U, // readtex.dectx $v0, $v1, 1
                                          0x00001088U, 0x03074414U, // rmsub.imm1 $v3, $v2, 1, $v0
                                          0x00001088U, 0x02075014U, // rmsub.imm1 $v2, $v0, 1, $v2
                                          0x10001088U, 0x0200140cU, // radd.rgb $v2, $v0, $v2
                                          0x1000108bU, 0x03075c14U, // { readtex.txty $v2, $v1, 1 ! rmsub.imm1 $v3, $v2, 1, $v3 }
                                          0x10001088U, 0x02074414U, // rmsub.imm1 $v2, $v2, 1, $v0
                                          0x1000109bU, 0x03075614U, // readtex.inctx $v2, $v1, 1 { rmsub.imm1 $v3, $v3, 1, $v2 }
                                          0x100010c8U, 0x02075014U, // rmsub.imm1 $v2, $v0, 1, $v2
                                          0x1000108bU, 0x0300d40cU, // readtex.txty $v2, $v1, 1 { radd.rgb $v3, $v3, $v2 }
                                          0x10001088U, 0x02074414U, // rmsub.imm1 $v2, $v2, 1, $v0
                                          0x100010dbU, 0x03075614U, // readtex.divty2 $v2, $v1, 1 { rmsub.imm1 $v3, $v3, 1, $v2 }
                                          0x10001088U, 0x02075014U, // rmsub.imm1 $v2, $v0, 1, $v2
                                          0x1000108bU, 0x0300d40cU, // readtex.txty $v2, $v1, 1 { radd.rgb $v3, $v3, $v2 }
                                          0x10001088U, 0x02074414U, // rmsub.imm1 $v2, $v2, 1, $v0 }
                                          0x100010cbU, 0x03075614U, // readtex.decty $v2, $v1, 1 { rmsub.imm1 $v3, $v3, 1, $v2 }
                                          0x10001008U, 0x02075014U, // rmsub.imm1 $v2, $v0, 1, $v2
                                          0x1000100aU, 0x8000d00dU};// { pixout $v2, 0 ! radd.rgb $v0, $v3, $v2 ! amov.a $v0, $v0 ! yield };
    nema_set_blend_blit(NEMA_BL_SRC); //setup matmult and ROB blender (if present) correctly
    nema_set_frag_ptr(0xd000);
    nema_load_frag_shader(cmd_sharpen_laplace, 17, 0);
    //------------------------------------------------------------
    // nema_set_const_reg(1, (sharp<<16)|(sharp<<8)|(sharp<<0));
    //------------------------------------------------------------
    nema_blit_rect(x+1, y, w, h);
    nema_blender_init();
}

//------------------------------------------------------------------------------------------------------------------------
void
nema_blit_contrast_linear(int x, int y, int w, int h, uint8_t min, uint8_t max)
{
    uint8_t diff = max - min;
    uint8_t modulo;
    uint8_t iter;
    //-----------------------------------------------------------------------
    if(diff == 0u) {
        min = 0; modulo = 0; iter = 0;
    } else {
        float diff_rec_f = 255.f/(float)diff;
        uint8_t diff_rec_i = 255u / diff;
        float modulo_f = 255.f * ((diff_rec_f - (float)diff_rec_i) + 0.5f / 255.0f);
        modulo = (uint8_t)modulo_f;
        iter = 255u / diff - 1u;
    }
    //-----------------------------------------------------------------------
    int  size = 6 + (2 * (int)iter);
    uint32_t cmd_contrast_linear[32];
    //-----------------------------------------------------------------------
    cmd_contrast_linear[0] = 0x00001000U;
    cmd_contrast_linear[1] = 0x03076c54U;       // rmsub.imm1 $v3, $v2, 1, $cv1
    cmd_contrast_linear[2] = 0x00001000U;
    cmd_contrast_linear[3] = 0x0200d448U;       // rmov.rgb $v2, $v3
    for(uint16_t i = 0; i < iter; i++) {
        cmd_contrast_linear[4U+(2U*i)] = 0x00001000U;
        cmd_contrast_linear[5U+(2U*i)] = 0x0200d44cU; // radd.rgb $v2, $v3, $v2
    }
    cmd_contrast_linear[size-2] = 0x00001002U;
    cmd_contrast_linear[size-1] = 0x80049445U; // { pixout $v0, 0 ! rmadd.rgb $v0, $v2, $cv2, $v2 ! amov.a $v0, $v2 ! yield }
    //-----------------------------------------------------------------------
    nema_set_blend_blit(NEMA_BL_SRC); //setup matmult and ROB blender (if present) correctly
    nema_set_frag_ptr(0xc000); //Prefetch
    size /= 2;
    nema_load_frag_shader(cmd_contrast_linear, (uint32_t)size, 0);
    //-----------------------------------------------------------------------
    uint32_t val;

    val  = min;    val <<= 8;
    val += min;    val <<= 8;
    val += min;
    nema_set_const_reg(1, val);

    val  = modulo; val <<= 8;
    val += modulo; val <<= 8;
    val += modulo;
    nema_set_const_reg(2, val);
    //-----------------------------------------------------------------------
    nema_blit_rect(x, y, w, h);
}

//------------------------------------------------------------------------------------------------------------------------
void
nema_blit_color_correction(uint8_t matrix[3][3], int x, int y, int w, int h)
{
    uint32_t cmd_color_correction[10] = {  0x00001000U, 0x02046664U,    // rmadd.rgb $v2, $v3, $cv1, $cv0
                                           0x00001000U, 0x0306d668U,    // rmov.gbr $v3, $v3
                                           0x00001000U, 0x02049664U,    // rmadd.rgb $v2, $v3, $cv2, $v2
                                           0x00001000U, 0x0306d668U,    // rmov.gbr $v3, $v3
                                           0x00001002U, 0x8004d665U};   // { pixout $v0, 0 ! rmadd.rgb $v0, $v3, $cv3, $v2 ! amov.a $v0, $v3 ! yield }
    nema_set_blend_blit(NEMA_BL_SRC); //setup matmult and ROB blender (if present) correctly
    nema_set_frag_ptr(0xe000); //Prefetch
    nema_load_frag_shader(cmd_color_correction, 5, 0);
    //----------------------------------------------------------------------------
    load_matrix_3x3(matrix);
    //----------------------------------------------------------------------------
    nema_blit_rect(x, y, w, h);
}

//------------------------------------------------------------------------------------------------------------------------
void
nema_blit_rgb_to_ycbcr(int w, int h)
{
    uint32_t cmd_rgb_to_ycbcr[18] = {  0x00001000U, 0x01002664U,  // rmadd.rgb $v1, $v3, $v0, $cv0
                                       0x00001000U, 0x02074874U,  // rmsub.imm1 $v2, $v0, 1, $v1
                                       0x00001000U, 0x02045664U,  // rmadd.rgb $v2, $v3, $cv1, $v2
                                       0x00001000U, 0x03072668U,  // rmov.brg $v3, $v3
                                       0x00001000U, 0x01002664U,  // rmadd.rgb $v1, $v3, $v0, $cv0
                                       0x00001000U, 0x0204d664U,  // rmadd.rgb $v2, $v3, $cv3, $v2
                                       0x00001000U, 0x02074c74U,  // rmsub.imm1 $v2, $v2, 1, $v1
                                       0x00001000U, 0x03071668U,  // rmov.brg $v3, $v3
                                       0x00001002U, 0x80049665U}; // { pixout $v0, 0 ! rmadd.rgb $v0, $v3, $cv2, $v2 ! amov.a $v0, $v3 ! yield }
    nema_set_blend_blit(NEMA_BL_SRC); //setup matmult and ROB blender (if present) correctly
    nema_set_frag_ptr(0xe000); //Prefetch
    nema_load_frag_shader(cmd_rgb_to_ycbcr, 9, 0);
    //----------------------------------------------------------------------------
    uint8_t matrix[3][3]={ { 76, - 84+128, - 21+128},
                           {150,  127    ,  127    },
                           { 29, - 43+128, -106+128} };
    //----------------------------------------------------------------------------
    load_matrix_3x3(matrix);
    //----------------------------------------------------------------------------
    nema_set_raster_color(0x00808000); //Set T0
    //----------------------------------------------------------------------------
    nema_blit_rect(0, 0, w, h);
}

//------------------------------------------------------------------------------------------------------------------------
void
nema_blit_median(int w, int h)
{
    uint32_t cmd_median[16] = {  0x1000109bU, 0x03001040U,  // readtex.inctx $v2, $v1, 1
                                 0x100010c8U, 0x03001058U,  // rmax.rgb $v3, $v0, $v2
                                 0x100010dbU, 0x0000105cU,  // { readtex.divty2 $v2, $v1, 1 ! rmin.rgb $v0, $v0, $v2 }
                                 0x10001088U, 0x02001058U,  // rmax.rgb $v2, $v0, $v2
                                 0x10001088U, 0x0000d05cU,  // rmin.rgb $v0, $v3, $v2
                                 0x100010cbU, 0x0300d058U,  // { readtex.decty $v2, $v1, 1 ! rmax.rgb $v3, $v3, $v2 }
                                 0x10001008U, 0x00001058U,  // rmax.rgb $v0, $v0, $v2
                                 0x1000100aU, 0x8000185dU}; // { pixout $v2, 0 ! rmin.rgb $v0, $v0, $v3 ! amov.a $v0, $v2 ! yield }
    nema_set_blend_blit(NEMA_BL_SRC); //setup matmult and ROB blender (if present) correctly
    nema_set_frag_ptr(0x9000); //Prefetch
    nema_load_frag_shader(cmd_median, 8, 0);
    //---------------------------------------------------

    nema_set_matrix_translate(2.f, 0.f); //go TX-2
    nema_raster_rect(1, 0, w, h); //start X+1
}

//------------------------------------------------------------------------------------------------------------------------
void
nema_blit_hist_equalization(int w, int h, uint32_t histogram[256])
{
    nema_buffer_t bo = nema_buffer_create(256);
    (void)nema_buffer_map(&bo);
    uint8_t *Equalized = (uint8_t *) bo.base_virt;
    uintptr_t base = bo.base_phys;
    uint32_t sum = 0;
    // float CDP[256] = {0.0f}; //Cumulative Distribution Propabilities
    //--------------------------------------------------------------
    for(uint16_t i = 0U; i < 256U; i++) {
        unsigned j = ((unsigned)i>>4) | ((unsigned)i<<4);
        sum += histogram[i];
        int _wh = w * h;
        // CDP[i] = (float)sum / (float)_wh;
        float CDP = (float)sum / (float)_wh;
        CDP = 255.f * (CDP + 0.5f/255.f);
        Equalized[j] = (uint8_t)CDP;
    }
    //--------------------------------------------------------------
    uint32_t cmd_lutY[8] = {  0x00001108U, 0x10044609U, // and.v2   $v0, $v3, $cv1
                              0x00001108U, 0x01000668U, // rmov.rgb $v1, $v0
                              0x0000110bU, 0x00000660U, // readtex.txty $v0, $v1, 2
                              0x00001002U, 0x80000669U};// { pixout $v0, 0 ! rmov.rgb $v0, $v0 ! amov.a $v0, $v3 ! yield }
    nema_set_blend_blit(NEMA_BL_SRC); //setup matmult and ROB blender (if present) correctly
    nema_set_frag_ptr(0xe000); //Prefetch
    nema_load_frag_shader(cmd_lutY, 4, 0);
    //--------------------------------------------------------------
    nema_set_const_reg(1, 0x00000ff0); //ABGR -> TYTX (TX=GGR.R)
    //--------------------------------------------------------------
    nema_bind_tex(NEMA_TEX2, base, 256, 1, NEMA_L8, 1024, NEMA_FILTER_PS);
    //--------------------------------------------------------------
    nema_blit_rect(0, 0, w, h);

    nema_buffer_unmap(&bo);
    nema_buffer_destroy(&bo);
}

//------------------------------------------------------------------------------------------------------------------------
void
nema_blit_gamma(int w, int h, nema_buffer_t *bo)
{
    uint8_t *Gamma = (uint8_t *) bo->base_virt;
    uintptr_t base = bo->base_phys;
    //--------------------------------------------------------------------------------------------------------------------
    // Lookup Table for gamma = 2.2
    uint8_t gamma[256] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01,
                          0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02,
                          0x03, 0x03, 0x03, 0x03, 0x03, 0x04, 0x04, 0x04, 0x04, 0x05, 0x05, 0x05, 0x05, 0x06, 0x06, 0x06,
                          0x06, 0x07, 0x07, 0x07, 0x08, 0x08, 0x08, 0x09, 0x09, 0x09, 0x0a, 0x0a, 0x0b, 0x0b, 0x0b, 0x0c,
                          0x0c, 0x0d, 0x0d, 0x0d, 0x0e, 0x0e, 0x0f, 0x0f, 0x10, 0x10, 0x11, 0x11, 0x12, 0x12, 0x13, 0x13,
                          0x14, 0x14, 0x15, 0x16, 0x16, 0x17, 0x17, 0x18, 0x19, 0x19, 0x1a, 0x1a, 0x1b, 0x1c, 0x1c, 0x1d,
                          0x1e, 0x1e, 0x1f, 0x20, 0x21, 0x21, 0x22, 0x23, 0x23, 0x24, 0x25, 0x26, 0x27, 0x27, 0x28, 0x29,
                          0x2a, 0x2b, 0x2b, 0x2c, 0x2d, 0x2e, 0x2f, 0x30, 0x31, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37,
                          0x38, 0x39, 0x3a, 0x3b, 0x3c, 0x3d, 0x3e, 0x3f, 0x40, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47,
                          0x49, 0x4a, 0x4b, 0x4c, 0x4d, 0x4e, 0x4f, 0x51, 0x52, 0x53, 0x54, 0x55, 0x57, 0x58, 0x59, 0x5a,
                          0x5b, 0x5d, 0x5e, 0x5f, 0x61, 0x62, 0x63, 0x64, 0x66, 0x67, 0x69, 0x6a, 0x6b, 0x6d, 0x6e, 0x6f,
                          0x71, 0x72, 0x74, 0x75, 0x77, 0x78, 0x79, 0x7b, 0x7c, 0x7e, 0x7f, 0x81, 0x82, 0x84, 0x85, 0x87,
                          0x89, 0x8a, 0x8c, 0x8d, 0x8f, 0x91, 0x92, 0x94, 0x95, 0x97, 0x99, 0x9a, 0x9c, 0x9e, 0x9f, 0xa1,
                          0xa3, 0xa5, 0xa6, 0xa8, 0xaa, 0xac, 0xad, 0xaf, 0xb1, 0xb3, 0xb5, 0xb6, 0xb8, 0xba, 0xbc, 0xbe,
                          0xc0, 0xc2, 0xc4, 0xc5, 0xc7, 0xc9, 0xcb, 0xcd, 0xcf, 0xd1, 0xd3, 0xd5, 0xd7, 0xd9, 0xdb, 0xdd,
                          0xdf, 0xe1, 0xe3, 0xe5, 0xe7, 0xea, 0xec, 0xee, 0xf0, 0xf2, 0xf4, 0xf6, 0xf8, 0xfb, 0xfd, 0xff};
    //--------------------------------------------------------------------------------------------------------------------
    for(uint16_t i = 0U; i < 256U; i++) {
        unsigned j = ((unsigned)i>>4)|((unsigned)i<<4);
        Gamma[j] = gamma[i];
        // Gamma[j] = 255 * (pow((float)i / 255.0f, gamma) + 0.5f / 255.0f);
    }
    //----------------------------------------------------------------------------
    uint32_t cmd_lutRGB[30] = {  0x00001108U, 0x01061268U, // rmov.r    $v1, $v3
                                 0x00001108U, 0x10140209U, // and.v2  $v0, $v3, $cv0
                                 0x00001108U, 0x01001268U, // rmov.rgb  $v1, $v0
                                 0x0000110bU, 0x02045060U, // readtex.txty  $v0, $v1, 2
                                 0x00001108U, 0x02045064U, // rmadd.rgb $v2, $v0, $cv1, $v2
                                 0x00001108U, 0x01065268U, // rmov.g    $v1, $v3
                                 0x00001108U, 0x10140209U, // and.v2  $v0, $v3, $cv0
                                 0x00001108U, 0x01001268U, // rmov.rgb  $v1, $v0
                                 0x0000110bU, 0x02049060U, // readtex.txty  $v0, $v1, 2
                                 0x00001108U, 0x02049064U, // rmadd.rgb $v2, $v0, $cv2, $v2
                                 0x00001108U, 0x01069268U, // rmov.b    $v1, $v3
                                 0x00001108U, 0x10140209U, // and.v2  $v0, $v3, $cv0
                                 0x00001108U, 0x01001268U, // rmov.rgb  $v1, $v0
                                 0x0000110bU, 0x0004d060U, // readtex.txty  $v0, $v1, 2
                                 0x00001002U, 0x8004d065U};// { pixout $v0, 0 ! rmadd.rgb   $v0, $v0, $cv3, $v2 ! amov.a $v0, $v3 ! yield };
    nema_set_blend_blit(NEMA_BL_SRC); //setup matmult and ROB blender (if present) correctly
    nema_set_frag_ptr(0xe000); //Prefetch
    nema_load_frag_shader(cmd_lutRGB, 15, 0);
    //----------------------------------------------------------------------------
    uint8_t matrix[3][3]={ {0xff, 0x00, 0x00},
                           {0x00, 0xff, 0x00},
                           {0x00, 0x00, 0xff} };
    //----------------------------------------------------------------------------
    nema_set_const_reg(0, 0x00000ff0); //ABGR -> TYTX (TX=GGR.R)
    load_matrix_3x3(matrix);
    //----------------------------------------------------------------------------
    nema_bind_tex(NEMA_TEX2, base, 256, 1, NEMA_L8, 1024, NEMA_FILTER_PS);
    //----------------------------------------------------------------------------
    nema_blit_rect_fit(0, 0, w, h);

    nema_blender_init();
}

//------------------------------------------------------------------------------------------------------------------------
void
nema_blit_binary(int x, int y, int w, int h, float threshold)
{
    threshold += 0.5f;
    uint8_t t = (uint8_t)threshold;
    uint8_t ct = 255U - t; //complementary
    uint32_t cmd_binary[2] = {  0x003a1002U, 0x8000300dU  };    // { pixout $v0, 0 ! cmp.less.rgb $v0, $cv1 ! radd.rgb $v0, $v0, $cv2 ! amov.a $v0, $v0 ! yield }
    //----------------------------------------------------
    nema_set_blend_blit(NEMA_BL_SRC); //setup matmult and ROB blender (if present) correctly
    nema_set_frag_ptr(0x8000); //Prefetch
    nema_load_frag_shader(cmd_binary, 1, 0);
    //----------------------------------------------------
    nema_set_const_reg(1, ((unsigned)t  << 16)|((unsigned)t  << 8)|((unsigned)t  ));
    nema_set_const_reg(2, ((unsigned)ct << 16)|((unsigned)ct << 8)|((unsigned)ct ));
    //----------------------------------------------------
    nema_blit_rect(x, y, w, h);
}

//------------------------------------------------------------------------------------------------------------------------
void
nema_blit_debayer(int w, int h)
{
//  a)    B         b)    R         c)  B G B       d)  R G R
//      R G R           B G B           G R G           G B G
//        B               R             B G B           R G R
    //----------------------------------------------------------------------------
    uint32_t cmd_debayer[34] = {/*a,b*/0x1800109bU, 0x00044600U, // readtex.inctx $v3, $v1, 1
                                       0x0000109bU, 0x00044600U, // readtex.inctx $v0, $v1, 1
                                       0x180010bbU, 0x00044604U, // readtex.dectx $v3, $v1, 1 { rmadd.rgb $v0, $v3, $cv1, $v0 }
                                       0x180010c8U, 0x00044604U, // rmadd.rgb $v0, $v3, $cv1, $v0
                                       0x180010dbU, 0x00044600U, // readtex.divty2 $v3, $v1, 1
                                       0x180010cbU, 0x00044604U, // readtex.decty $v3, $v1, 1 { rmadd.rgb $v0, $v3, $cv1, $v0 }
                                       0x1800100aU, 0x80044605U, // { pixout $v3, 0 ! rmadd.rgb $v0, $v3, $cv1, $v0 ! amov.a $v0, $v0 ! yield }
                                /*c,d*/0x1800109bU, 0x00048660U, // readtex.inctx $v3, $v1, 1
                                       0x1800109bU, 0x00048664U, // { readtex.inctx $v3, $v1, 1 ! rmadd.rgb $v0, $v3, $cv2, $v0 }
                                       0x180010abU, 0x00048664U, // { readtex.incty $v3, $v1, 1 ! rmadd.rgb $v0, $v3, $cv2, $v0 }
                                       0x180010abU, 0x00048664U, // { readtex.incty $v3, $v1, 1 ! rmadd.rgb $v0, $v3, $cv2, $v0 }
                                       0x180010bbU, 0x00048664U, // { readtex.dectx $v3, $v1, 1 ! rmadd.rgb $v0, $v3, $cv2, $v0 }
                                       0x180010bbU, 0x00048664U, // { readtex.dectx $v3, $v1, 1 ! rmadd.rgb $v0, $v3, $cv2, $v0 }
                                       0x180010cbU, 0x00048664U, // { readtex.decty $v3, $v1, 1 ! rmadd.rgb $v0, $v3, $cv2, $v0 }
                                       0x1800109bU, 0x00048664U, // { readtex.inctx $v3, $v1, 1 ! rmadd.rgb $v0, $v3, $cv2, $v0 }
                                       0x1800108bU, 0x00048664U, // { readtex.txty $v3, $v1, 1 ! rmadd.rgb $v0, $v3, $cv2, $v0 }
                                       0x1800100aU, 0x8000c66dU};// { pixout $v3, 0 ! radd.rgb $v0, $v3, $v0 ! amov.a $v0, $v3 ! yield }
    nema_load_frag_shader(cmd_debayer, 17, 0);
    //----------------------------------------------------------------------------
    uint8_t matrix[2][3]={ {0x80, 0x80, 0x80},
                           {0x40, 0x40, 0x40}};
    //----------------------------------------------------------------------------
    load_matrix_3x2(matrix);
    //----------------------------------------------------------------------------
    nema_matrix3x3_t m;
    m[0][0] = 2.0f;     m[0][1] = 0.0f;     m[0][2] = 0.0f;
    m[1][0] = 0.0f;     m[1][1] = 2.0f;     m[1][2] = 0.0f;
    m[2][0] = 0.0f;     m[2][1] = 0.0f;     m[2][2] = 1.0f;
    //----------------------------------------------------------------------------
    //a
    nema_set_blend_blit(NEMA_BL_SRC); //setup matmult and ROB blender (if present) correctly
    nema_set_frag_ptr(0);

    m[0][2] = -2.0f;
    m[1][2] =  0.0f;
    nema_set_matrix(m);
    nema_raster_rect(0, 0, w, h);

    //----------------------------------------------------------------------------
    //b
    nema_set_frag_ptr(0);

    m[0][2] = -1.0f;
    m[1][2] = -1.0f;
    nema_set_matrix(m);
    nema_raster_rect(0, 0, w, h);

    //----------------------------------------------------------------------------
    //c
    nema_set_frag_ptr(7);

    nema_raster_rect(0, 0, w, h);
    //----------------------------------------------------------------------------
    m[0][2] = -2.0f;
    m[1][2] = -2.0f;
    nema_set_matrix(m);
    nema_raster_rect(0, 0, w, h);

    nema_blender_init();
}

//------------------------------------------------------------------------------------------------------------------------
void
nema_blit_ycbcr_to_rgb(int w, int h)
{
    uint32_t cmd_ycbcr_to_rgb[32] = {  0x00001083U, 0x0ab4a948U,  // { readtex.xy $v0, 1 ! rmov.rgb $v2, $cv2 }
                                       0x00001080U, 0x0bb76955U,  // { rmsub.imm1 $v3, $v0, 1, $cv1 ! amov.r $v2, $v0 }
                                       0x00001080U, 0x0bb4df44U,  // rmadd.rgb $v3, $v3, $cv3, $v3
                                       0x00001080U, 0x0bb71f48U,  // rmov.brg $v3, $v3
                                       0x00001080U, 0x05b19d2cU,  // radd.a $v1, $v2, $v3
                                       0x00001080U, 0x07b74535U,  // { rmsub.imm1 $v3, $v2, 1, $v0 ! amov.g $v1, $v0 }
                                       0x00001080U, 0x07b4de27U,  // { rmadd.rgb $v3, $v3, $cv3, $v3 ! amul.a $v1, $v1, $cv1 }
                                       0x00001080U, 0x0b371b09U,  // { rmov.brg $v3, $v3 ! amov.b $v2, $v0 }
                                       0x00001080U, 0x09375bb6U,  // { rmsub.imm1 $v1, $v1, 1, $v3 ! amadd.c2 $v2, $v2, $cv2, $v1 }
                                       0x00001080U, 0x0a319ba8U,  // rmov.a $v2, $v2
                                       0x00001080U, 0x0b3517a8U,  // rmov.a $v3, $cv0
                                       0x00001080U, 0x0b3757b4U,  // rmsub.imm1 $v3, $v3, 1, $v2
                                       0x00001080U, 0x083417b4U,  // rmsub.rgb $v0, $v3, $cv0, $v2
                                       0x00001080U, 0x0b341db4U,  // rmsub.rgb $v3, $v2, $cv0, $v3
                                       0x00001000U, 0x01375a14U,  // rmsub.imm1 $v1, $v1, 1, $v3
                                       0x00001002U, 0x8030420dU}; // { pixout $v0, 0 ! radd.rgb $v0, $v1, $v0 ! amov.a $v0, $v0 ! yield }
    nema_set_blend_blit(NEMA_BL_SRC); //setup matmult and ROB blender (if present) correctly
    nema_set_frag_ptr(0);
    nema_load_frag_shader(cmd_ycbcr_to_rgb, 16, 0);
    //-----------------------------------------------------------------------------------------------
    uint8_t matrix[4][4]={ {  0, 255,   0, 136},
                           {255, 128, 128,  44},
                           {  0, 128, 128,  92},
                           {  0, 195, 102,   0} };
    //-----------------------------------------------------------------------------------------------
    load_matrix_4x4(matrix);
    //-----------------------------------------------------------------------------------------------
    nema_blit_rect(0, 0, w, h);

    nema_blender_init();
}

//-----------------------------------------------------------------------------------------------------------------------
void
nema_blit_bayer_L8_to_RGB(int w, int h)
{
    // IMG3 -> IMG0
    uint32_t cmd_L8_to_RGB[6] = {  0x0000100aU, 0x8004a665U,    // { pixout $v0, 0 ! rmadd.rgb $v0, $v3, $cv2, $cv0 ! amov.a $v0, $v3 ! yield }
                                   0x0000100aU, 0x80046665U,    // { pixout $v0, 0 ! rmadd.rgb $v0, $v3, $cv1, $cv0 ! amov.a $v0, $v3 ! yield }
                                   0x0000100aU, 0x8004e665U};   // { pixout $v0, 0 ! rmadd.rgb $v0, $v3, $cv3, $cv0 ! amov.a $v0, $v3 ! yield }
    nema_load_frag_shader(cmd_L8_to_RGB, 3, 0);
    //----------------------------------------------------------------------------
    nema_matrix3x3_t m;
    m[0][0] =  2.f;    m[0][1] =  0.f;    m[0][2] =  0.f;
    m[1][0] =  0.f;    m[1][1] =  2.f;    m[1][2] =  0.f;
    m[2][0] =  0.f;    m[2][1] =  0.f;    m[2][2] =  1.f;
    //----------------------------------------------------------------------------
    uint8_t matrix[3][3]={ {0xff, 0x00, 0x00},
                           {0x00, 0xff, 0x00},
                           {0x00, 0x00, 0xff} };
    //----------------------------------------------------------------------------
    load_matrix_3x3(matrix);
    //----------------------------------------------------------------------------
    //a
    nema_set_blend_blit(NEMA_BL_SRC); //setup matmult and ROB blender (if present) correctly
    nema_set_frag_ptr(0xec00);

    m[0][2] =  0.0f;
    m[1][2] = -1.0f;
    nema_set_matrix(m);
    nema_raster_rect(0, 0, w, h);

    //----------------------------------------------------------------------------
    m[0][2] = -1.0f;
    m[1][2] =  0.0f;
    nema_set_matrix(m);
    nema_raster_rect(0, 0, w, h);

    //----------------------------------------------------------------------------
    //c
    nema_set_frag_ptr(0xec01);

    m[0][2] = -1.0f;
    m[1][2] = -1.0f;
    nema_set_matrix(m);
    nema_raster_rect(0, 0, w, h);

    //----------------------------------------------------------------------------
    //d
    nema_set_frag_ptr(0xec02);

    m[0][2] =  0.0f;
    m[1][2] =  0.0f;
    nema_set_matrix(m);
    nema_raster_rect(0, 0, w, h);
}

//-----------------------------------------------------------------------------------------------------------------------
void
nema_blit_RGB_to_3L8(int w, int h)
{
    //IMG0 -> IMG1, IMG2, IMG3
    uint32_t cmd_to_3L8[6] = {  0x00001102U, 0x00064008U,   // { pixout $v0, 2 ! rmov.g $v0, $v3 }
                                0x00001182U, 0x00068008U,   // { pixout $v0, 3 ! rmov.b $v0, $v3 }
                                0x00001082U, 0x80060008U};  // { pixout $v0, 1 ! rmov.r $v0, $v3 ! yield }
    nema_set_blend_blit(NEMA_BL_SRC); //setup matmult and ROB blender (if present) correctly
    nema_set_frag_ptr(0xe400);
    nema_load_frag_shader(cmd_to_3L8, 3, 0);
    //--------------------------------------------------
    nema_blit_rect(0, 0, w, h);
}

//-----------------------------------------------------------------------------------------------------------------------
void
nema_blit_3L8_to_RGB(int w, int h)
{
    //IMG0, IMG1, IMG3 -> IMG2
    uint32_t cmd_to_RGB[6] = {  0x1000118bU, 0x00046004U,   // { readtex.txty $v2, $v1, 3 ! rmadd.rgb $v0, $v0, $cv1, $cv0 }
                                0x1800108bU, 0x0004c404U,   // { readtex.txty $v3, $v1, 1 ! rmadd.rgb $v0, $v2, $cv3, $v0 }
                                0x18001102U, 0x80048604U};  // { pixout $v3, 2 ! rmadd.rgb $v0, $v3, $cv2, $v0 ! yield }
    nema_set_blend_blit(NEMA_BL_SRC); //setup matmult and ROB blender (if present) correctly
    nema_set_frag_ptr(0x8400);
    nema_load_frag_shader(cmd_to_RGB, 3, 0);
    //----------------------------------------------------------------------------
    uint8_t matrix[3][3]={ {0xff, 0x00, 0x00},
                           {0x00, 0xff, 0x00},
                           {0x00, 0x00, 0xff} };
    //----------------------------------------------------------------------------
    load_matrix_3x3(matrix);
    //----------------------------------------------------------------------------
    nema_blit_rect(0, 0, w, h);
}
