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
#ifndef NEMA_RASTERIZER_INTERN_H__
#define NEMA_RASTERIZER_INTERN_H__

#ifdef __cplusplus
extern "C" {
#endif

// Drawing Engine Primitives
//-----------------------------------------------------------------------------------------------------------------------
#define DRAW_LINE         (0x01U)
#define DRAW_BOX          (0x02U)
#define DRAW_TRIANGLE     (0x04U)
#define DRAW_QUAD         (0x05U)
#define GL_DRAW_LINE      (0x21U)
#define GL_DRAW_BOX       (0x22U)
#define GL_DRAW_PIXEL     (0x23U)
#define GL_DRAW_TRIANGLE  (0x24U)
#define GL_DRAW_QUAD      (0x25U)

#define RAST_AA_E3          (0x00800000U)  //(1U<<23)
#define RAST_AA_E2          (0x01000000U)  //(1U<<24)
#define RAST_AA_E1          (0x02000000U)  //(1U<<25)
#define RAST_AA_E0          (0x04000000U)  //(1U<<26)
#define RAST_AA_MASK        (RAST_AA_E0 | RAST_AA_E1 | RAST_AA_E2 | RAST_AA_E3)
#define RAST_GRAD           (0x08000000U)  //(1U<<27)
#define RAST_SETUP_CULL_CW  (0x10000000U)  //(1U<<28)
#define RAST_SETUP_CULL_CCW (0x20000000U)  //(1U<<29)
#define RAST_TILE           (0x40000000U)  //(1U<<30)
#define RAST_TILE_2X2       (0xC0000000U)  //(1U<<31 | 1U<<30)
#define RAST_TILE_MASK      (0xC0000000U)

/** \brief Enables MSAA per edge
 *
 * \param aa A combination of the flags RAST_AA_E0, RAST_AA_E1, RAST_AA_E2, RAST_AA_E3
 * \return previous AA flags (may be ignored)
 *
 */
uint32_t nema_enable_aa_flags(uint32_t aa);

/** \brief Enables an additional clipping rectangle
 *
 * \param x Clip Window top-left x coordinate
 * \param y Clip Window minimum y
 * \param w Clip Window width
 * \param h Clip Window height
 *
 */
void nema_set_clip2(int32_t x, int32_t y, uint32_t w, uint32_t h);

/** \brief Returns if clip2 is supported in the hardware
 *
 * \return returns 1 if clip2 is supported otherwise returns 0
 *
 */
int nema_supports_clip2(void);

/** \brief Returns the number of steps required to draw a full circle
 *
 * \param r Radius
 * \return returns the number of steps
 *
 */
int calculate_steps_from_radius(float r);

int line_intersection_point(float xa0, float ya0, float xa1, float ya1, float xb0, float yb0, float xb1, float yb1, float *x_out, float *y_out);

#ifdef __cplusplus
}
#endif

#endif //NEMA_RASTERIZER_INTERN_H__
