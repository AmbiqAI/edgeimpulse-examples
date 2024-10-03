// -----------------------------------------------------------------------------
// Copyright (c) 2019 Think Silicon S.A.
// Think Silicon S.A. Confidential Proprietary
// -----------------------------------------------------------------------------
//     All Rights reserved - Unpublished -rights reserved under
//         the Copyright laws of the European Union
//
//  This file includes the Confidential information of Think Silicon S.A.
//  The receiver of this Confidential Information shall not disclose
//  it to any third party and shall protect its confidentiality by
//  using the same degree of care, but not less than a reasonable
//  degree of care, as the receiver uses to protect receiver's own
//  Confidential Information. The entire notice must be reproduced on all
//  authorised copies and copies may only be made to the extent permitted
//  by a licensing agreement from Think Silicon S.A..
//
//  The software is provided 'as is', without warranty of any kind, express or
//  implied, including but not limited to the warranties of merchantability,
//  fitness for a particular purpose and noninfringement. In no event shall
//  Think Silicon S.A. be liable for any claim, damages or other liability, whether
//  in an action of contract, tort or otherwise, arising from, out of or in
//  connection with the software or the use or other dealings in the software.
//
//
//                    Think Silicon S.A.
//                    http://www.think-silicon.com
//                    Patras Science Park
//                    Rion Achaias 26504
//                    Greece
// -----------------------------------------------------------------------------

#ifndef __COVER_FLOW_H__
#define __COVER_FLOW_H__

//*****************************************************************************
//
// third-party/devices include files.
//
//*****************************************************************************
#include "am_bsp.h"
#include "nema_core.h"
#include "nema_matrix4x4.h"
#include "nema_math.h"
#include "nema_utils.h"
#include "nema_graphics.h"
#include "nema_easing.h"
#include "nema_programHW.h"

#include "am_util_stdio.h"
#ifndef RESX
#define RESX 320
#endif

#ifndef RESY
#define RESY 320
#endif

//
// if defined this macro,the application will initialize psram,otherwise,no use psram.
//
//#define LOAD_FROM_PSRAM

//
// please check SoC's datasheet to conform it could support hex pins operation
// PSRAM, of course used BSP should support hex pins at the same time.
// generally speaking,Apollo4P or later series SoC support x16 pin mode.
// if defined this macro it include x16 psram driver,otherwise,include x8 psram driver
//
//#define USE_HEX_MODE_PSRAM

#ifdef LOAD_FROM_PSRAM

#define MSPI_PSRAM_MODULE              0

#if (MSPI_PSRAM_MODULE == 0)
#define MSPI_XIP_BASE_ADDRESS 0x14000000
#elif (MSPI_PSRAM_MODULE == 1)
#define MSPI_XIP_BASE_ADDRESS 0x18000000
#elif (MSPI_PSRAM_MODULE == 2)
#define MSPI_XIP_BASE_ADDRESS 0x1C000000
#endif // MSPI_PSRAM_MODULE == 0

#define MSPI_XIPMM_BASE_ADDRESS MSPI_XIP_BASE_ADDRESS

#endif // LOAD_FROM_PSRAM


int32_t cover_flow(void);

#endif
