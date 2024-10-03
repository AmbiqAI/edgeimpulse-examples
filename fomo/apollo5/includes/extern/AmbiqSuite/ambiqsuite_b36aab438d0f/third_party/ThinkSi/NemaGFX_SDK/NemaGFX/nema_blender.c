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
#include "nema_programHW.h"
#include "nema_blender_intern.h"
#include "nema_blender.h"
#include "nema_raster.h"
#include "nema_rasterizer.h"

static const uint32_t cmd_modulate[2] = {0x080c0000U, 0x00000004U};
static const uint32_t cmd_blendsrc[2] = {0x080c0000U, 0x0a002007U};
static const uint32_t cmd_blenddst[2] = {0x004e0002U, 0x80080206U};

// #define NEMAGFX_DISABLE_ROP

static uint32_t BG_PREFETCH = PREFETCH_TEXEL;

#ifndef NEMAGFX_DISABLE_ROP
static bool      rop_blender = false;
#endif

static bool debug_overdraws = false;

void
nema_blender_init(void)
{
    uint32_t cmd[6] = {   0U, 0U,
                          0x08000002U,
                          0x80000009U,
                          0U,
                          0U
                          };

    for (unsigned i = 0U; i < 7U; ++i) {
        nema_load_frag_shader(&cmd[0], 1, PRELOAD_ADDR-7U+i);
    }

    nema_load_frag_shader(&cmd[2], 1, PRELOAD_ADDR);

#ifndef NEMAGFX_DISABLE_ROP
    rop_blender = (nema_readHwConfig() & NEMA_CONF_MASK_ROP_BLENDER) != 0U;

    //set rop_blender to bypass
    if (rop_blender)
    {
        BG_PREFETCH = 0;
#endif
        nema_set_rop_blend_mode( NEMA_BL_SRC );
#ifndef NEMAGFX_DISABLE_ROP
    }
#endif
}

//#define DONT_PREFETCH_FG_TEX

//-----------------------------------------------------------------------------------------------------------------------
void
nema_set_blend(uint32_t blending_mode,
                nema_tex_t dst_tex,
                nema_tex_t fg_tex,
                nema_tex_t bg_tex)
{

    uint32_t blending_mode_ = blending_mode;
    nema_tex_t fg_tex_      = fg_tex;
    nema_tex_t bg_tex_      = bg_tex;
    nema_tex_t dst_tex_     = dst_tex;

    if (debug_overdraws) {
        blending_mode_ = NEMA_BL_ADD;
        nema_set_raster_color(0x20202020U);
        fg_tex_ = NEMA_NOTEX;
        bg_tex_ = NEMA_NOTEX;
        nema_enable_gradient(0);
    }

    uint32_t src_bf       =  blending_mode_&0xfU;
    uint32_t dst_bf       = (blending_mode_>>8)&0xfU;
    bool     dst_ckey_neg = (blending_mode_ & NEMA_BLOP_DST_CKEY_NEG ) != 0U;
    bool     dst_ckey     = (blending_mode_ & NEMA_BLOP_DST_CKEY     ) != 0U;
    bool     stencil_txty = (blending_mode_ & NEMA_BLOP_STENCIL_TXTY ) != 0U;
    bool     stencil_xy   = (blending_mode_ & NEMA_BLOP_STENCIL_XY   ) != 0U;
    bool     lut          = (blending_mode_ & NEMA_BLOP_LUT          ) != 0U;
    bool     blop         = (blending_mode_ & NEMA_BLOP_MASK         ) != 0U;
    bool     recolor      = (blending_mode_ & NEMA_BLOP_RECOLOR     ) != 0U;

    if ( (dst_tex_ < NEMA_TEX0) || ((int)dst_tex_ > (int)NEMA_TEX3) ) {
        dst_tex_ = NEMA_TEX0;
    }
    if ( (fg_tex_ < NEMA_TEX0) || ((int)fg_tex_ > (int)NEMA_TEX3) ) {
        fg_tex_ = NEMA_NOTEX;
    }

    nema_tex_t lut_tex = bg_tex_;

    if ( (bg_tex_ < NEMA_TEX0) || ((int)bg_tex_ > (int)NEMA_TEX3) ) {
        bg_tex_ = dst_tex_;
        lut_tex = NEMA_TEX2;
    }

#ifdef NEMAGFX_DISABLE_ROP
    // if we disable the HW blender, we can'd do recolor atm
    recolor = false;
#else
    if ( rop_blender ) {

        uint32_t rop_blop = 0;
        if (dst_ckey) {
            rop_blop |= NEMA_BLOP_DST_CKEY;
            dst_ckey  = false;
            blending_mode_ = blending_mode_ & ~NEMA_BLOP_DST_CKEY;
        }

        if (dst_ckey_neg) {
            rop_blop |= NEMA_BLOP_DST_CKEY_NEG;
            dst_ckey_neg  = false;
            blending_mode_ = blending_mode_ & ~NEMA_BLOP_DST_CKEY_NEG;
        }

        if ( ((blending_mode_ & NEMA_BLOP_NO_USE_ROPBL) != 0U) || (bg_tex_ != dst_tex_) ) {
            nema_set_rop_blend_mode( NEMA_BL_SRC | rop_blop );
            // if we disable the HW blender, we can'd do recolor atm
            recolor = false;
        } else {
            nema_set_rop_blend_mode( nema_blending_mode(src_bf, dst_bf, rop_blop));

            src_bf = NEMA_BF_ONE;
            dst_bf = NEMA_BF_ZERO;
            blending_mode_ = nema_blending_mode(src_bf, dst_bf, blending_mode_ & NEMA_BLOP_MASK);
        }
    }
#endif

    dst_ckey = dst_ckey || dst_ckey_neg;

#ifndef DONT_PREFETCH_FG_TEX
    if ( blending_mode_ == NEMA_BL_SRC && nema_context.en_sw_depth == 0u && dst_tex_ == NEMA_TEX0 ) {
        if (fg_tex_ < NEMA_TEX0) {
            nema_matmul_bypass(1);
            nema_set_frag_ptr(PRELOAD_ADDR                                    | ((BG_PREFETCH | PRE_IMG0 | PRE_XY | PRELOAD_ADDR) << 16) );
        } else {
            nema_matmul_bypass(0);
            nema_set_frag_ptr(PREFETCH_TEXEL | PRE_TEX[fg_tex_] | PRELOAD_ADDR | ((BG_PREFETCH | PRE_IMG0 | PRE_XY | PRELOAD_ADDR) << 16) );
        }

        return;
    }
#endif

    bool bypass_matmult = true;

    unsigned i=0;
    bool src_ckey    = (blending_mode_ & NEMA_BLOP_SRC_CKEY    ) != 0U;
    bool colorize    = (blending_mode_ & NEMA_BLOP_MODULATE_RGB) != 0U;
    bool src_premult = (blending_mode_ & NEMA_BLOP_SRC_PREMULT ) != 0U;
    bool modulateA   = (blending_mode_ & NEMA_BLOP_MODULATE_A  ) != 0U;
    bool forceA      = (blending_mode_ & NEMA_BLOP_FORCE_A     ) != 0U;
    bool read_dst    = src_bf >= NEMA_BF_DESTALPHA || dst_ckey || dst_bf != NEMA_BF_ZERO;

    uint32_t P_OPCODE    = (read_dst           ) ? (uint32_t)P_READTEX : (uint32_t)P_NOP;
    uint32_t C_SRCKEY_OP = (src_ckey           ) ? (uint32_t)C_CMP_EQ  : (uint32_t)C_NOP;
    uint32_t A_MODULATE  = (forceA             ) ? (uint32_t)A_MOV     : (uint32_t)A_MUL;
    uint32_t C_DSTKEY_OP = (dst_ckey_neg       ) ? (uint32_t)C_CMP_EQ  : ( dst_ckey    ? (uint32_t)C_CMP_NEQ : (uint32_t)C_NOP );
    uint32_t R1_MODULATE = ( recolor           ) ? (uint32_t)R1_C3AAA  : ( colorize    ? (uint32_t)R1_C1RGB  : ( src_premult ? (uint32_t)R1_T0AAA  : (uint32_t)R1_111) );
    uint32_t A1_MODULATE = (modulateA || forceA) ? (uint32_t)A1_C1A    : (uint32_t)A1_1;
    //--------------------------------------------------------------------------------------
    //SRC COLOR is read at T0
    //DST COLOR is read at T1
    //SRC CALC  is written in T2
    //FINAL RESULT is written in T0
    //--------------------------------------------------------------------------------------

    unsigned code_ptr     =  0;

    uint32_t cmd[8][2];
    cmd[0][0] = 0; //lint

    //Prefetch if needed
    if ( nema_context.en_sw_depth != 0u ) {
        code_ptr = PREFETCH_TEXEL | PRE_IMG3 | PRE_XY | PRE_T3;
        //compare depth value and pixout new depth value
        cmd[i][0] = 0x00f40182U;
        cmd[i][1] = 0x02009408U;
        ++i;

        if (fg_tex_ >= NEMA_TEX0) {
            cmd[i][0] = 0x000c100bU | ((uint32_t)(int)fg_tex_ << 7);
            cmd[i][1] = 0x00002000U;
            bypass_matmult = false;
            ++i;
        }
    }
    else {
        if (fg_tex_ < NEMA_TEX0) {
            if (read_dst) {
                code_ptr = PREFETCH_TEXEL | PRE_TEX[bg_tex_] | PRE_XY | PRE_T1;
                read_dst = false;
                P_OPCODE = P_NOP;
            }
            else {
                code_ptr = 0;
            }
        }
        else {
#ifndef DONT_PREFETCH_FG_TEX
            code_ptr = PREFETCH_TEXEL | PRE_TEX[fg_tex_] /*| PRE_TXTY | PRE_T0*/;
            bypass_matmult = false;
#else
            cmd[i][0] = 0x000c100bU | (fg_tex_ << 7);
            cmd[i][1] = 0x00002000U;
            bypass_matmult = false;
            ++i;
#endif
        }

        if (stencil_txty) {
            // SRC color is in T0
            // Load MASK in T2
            cmd[i][0] = 0x100c118bU;
            cmd[i][1] = 0x00002000U;

            ++i;
            // Multiply
            cmd[i][0] = 0x000c0000U;
            cmd[i][1] = 0x0081a042U;
            ++i;
        }
        else if (stencil_xy) {
            // SRC color is in T0
            // Load MASK in T2

            cmd[i][0] = 0x100c1183U;
            cmd[i][1] = 0x00002000U;

            ++i;
            // Multiply
            cmd[i][0] = 0x000c0000U;
            cmd[i][1] = 0x0081a042U;
            ++i;
        }
        else{
            // MISRA
        }

        if (lut && fg_tex_ != NEMA_NOTEX) {
            code_ptr = PREFETCH_TEXEL | PRE_TEX[fg_tex_] /* | PRE_TXTY */ | PRE_T1;
            cmd[i][0] = 0x0000100bU | ((uint32_t)(int)lut_tex << 7);
            cmd[i][1] = 0x00000000U;
            ++i;
        }
    }

        //                              ZERO    ONE     SRC_C     INVSRC_C   SRC_A     INVSRC_A   DST_A     INV_DST_A  DST_C     INVDST_C   CONST_C   CONST_A
    static const uint32_t bl_r1[] = {R1_000, R1_111, R1_T0RGB, R1_iT0RGB, R1_T0AAA, R1_iT0AAA, R1_T1AAA, R1_iT1AAA, R1_T1RGB, R1_iT1RGB, R1_C1RGB, R1_C1AAA};
    static const uint32_t bl_a1[] = {A1_0  , A1_1  , A1_T0A  , A1_iT0A  , A1_T0A  , A1_iT0A  , A1_T1A  , A1_iT1A  , A1_T1A  , A1_iT1A  , A1_C1A  , A1_C1A};

    // Get destination - Modulate
    //--------------------------------------------------------------------------------------
    bool do_blend = true;

    if ( dst_bf <= NEMA_BF_ONE && !blop && fg_tex_ < NEMA_TEX0 && bg_tex_ == dst_tex_ ) {
        do_blend = false;

        uint32_t   R2_DSTBLEND = dst_bf == NEMA_BF_ZERO ? (uint32_t)R2_C0RGB : (uint32_t)R2_T1RGB;
        uint32_t   A2_DSTBLEND = dst_bf == NEMA_BF_ZERO ? (uint32_t)A2_C0A   : (uint32_t)A2_T1A;

        cmd[i][1] = 0x8a000006U | (bl_r1[src_bf] << 14) | (R2_DSTBLEND << 11) | (bl_a1[src_bf] << 5) | (A2_DSTBLEND << 21);
        cmd[i][0] = cmd_blendsrc[0] | P_PIXOUT | ((uint32_t)(int)dst_tex_ << 7);

        ++i;
        C_SRCKEY_OP = C_NOP;
        P_OPCODE    = P_NOP;

        src_ckey    = false;
        read_dst    = false;
    }
    else if ( src_bf >= NEMA_BF_DESTALPHA || recolor || colorize || src_premult || modulateA || forceA ) {
        uint32_t R2_MODULATE = recolor ? R2_C3RGB : R2_C0RGB;
        if (P_OPCODE == P_NOP && dst_bf == NEMA_BF_ZERO && !dst_ckey && src_bf == NEMA_BF_ONE && !src_ckey) {
            cmd[i][1] = cmd_modulate[1] | (0x80000000U) | (R1_MODULATE << 14) | (A1_MODULATE << 5) | (A_MODULATE) | (R2_MODULATE << 11);
            cmd[i][0] = cmd_modulate[0] | (P_PIXOUT) | ((uint32_t)(int)dst_tex_ << 7) | (C_SRCKEY_OP << 20);
            do_blend = false;
            ++i;

        } else if (R1_MODULATE != R1_111 || A1_MODULATE != A1_1 || P_OPCODE != P_NOP || C_SRCKEY_OP != C_NOP) {
            cmd[i][1] = cmd_modulate[1] | (R1_MODULATE << 14) | (A1_MODULATE << 5) | (A_MODULATE) | (R2_MODULATE << 11);
            cmd[i][0] = cmd_modulate[0] | (P_OPCODE) | ((uint32_t)(int)bg_tex_ << 7) | (C_SRCKEY_OP << 20);
            ++i;
        } else {
            //misra
        }

        C_SRCKEY_OP = C_NOP;
        P_OPCODE    = P_NOP;

        src_ckey    = false;
        read_dst    = false;
    } else {
        //misra
    }

    if (do_blend) {
        uint32_t   R2_DSTBLEND = R2_T2RGB;
        uint32_t   A2_DSTBLEND = A2_T2A;
        nema_tex_t    P_IMG = bg_tex_;
        uint32_t firstCmdYield = 0U;
        bool needSrcBlend = true;
        if (dst_bf == NEMA_BF_ZERO && !dst_ckey) {
            P_OPCODE      = P_PIXOUT;
            P_IMG         = dst_tex_;
            firstCmdYield = 0x80000000U;
        } else if ( !src_ckey && !read_dst ) {
            if (src_bf == NEMA_BF_ZERO) {
                needSrcBlend = false;
                R2_DSTBLEND = R2_C0RGB;
                A2_DSTBLEND = A2_C0A;
            }
            else if (src_bf == NEMA_BF_ONE) {
                needSrcBlend = false;
                R2_DSTBLEND = R2_T0RGB;
                A2_DSTBLEND = A2_T0A;
            } else {
                //misra
            }
        } else {
            //misra
        }

        if (needSrcBlend) {
            cmd[i][1] = cmd_blendsrc[1] | (firstCmdYield) | (bl_r1[src_bf] << 14) | (bl_a1[src_bf] << 5);
            cmd[i][0] = cmd_blendsrc[0] | (P_OPCODE) | ((uint32_t)(int)P_IMG << 7) | (C_SRCKEY_OP << 20);
            ++i;
        }
        if (firstCmdYield == 0U) {
            cmd[i][1] = cmd_blenddst[1] | (bl_r1[dst_bf] << 14) | (R2_DSTBLEND << 11)| (bl_a1[dst_bf] << 5) | (A2_DSTBLEND << 21);
            cmd[i][0] = cmd_blenddst[0] | ((uint32_t)(int)dst_tex_ << 7) | (C_DSTKEY_OP << 20);
            ++i;
        }
    }
    //--------------------------------------------------------------------------------------
    nema_matmul_bypass(bypass_matmult ? (int)1 : (int)0);

    nema_load_frag_shader(&cmd[0][0], i, 0U);

    nema_set_frag_ptr(code_ptr | ((BG_PREFETCH | PRE_TEX[bg_tex_] | PRE_XY /*| PRE_T0*/ | (PRELOAD_ADDR+1U-i)) << 16));
}

void nema_set_dst_color_key(uint32_t rgba) {
#ifndef NEMAGFX_DISABLE_ROP
    if ( rop_blender) {
        nema_set_rop_dst_color_key(rgba);
    }
    else
#endif
    {
        nema_set_const_reg(3, rgba);
    }
}

void nema_set_src_color_key(uint32_t rgba) {
    nema_set_const_reg(2, rgba);
    nema_context.src_ckey = rgba;
}

void nema_set_const_color(uint32_t rgba) {
    nema_set_const_reg(1, rgba);

#ifndef NEMAGFX_DISABLE_ROP
    if (rop_blender) {
        nema_set_rop_const_color(rgba);
    }
#endif
}

void nema_set_recolor_color(uint32_t rgba) {
#ifndef NEMAGFX_DISABLE_ROP
    if ( rop_blender) {
        uint32_t rgba_premult = nema_premultiply_rgba(rgba);
        // invert alpha so that we do (Csrc * (1-Cconst))
        rgba_premult ^= 0xff000000U;
        nema_set_const_reg(3, rgba_premult);
    }
#endif
}

void nema_debug_overdraws(uint32_t enable) {
    debug_overdraws = enable != 0U;
}
