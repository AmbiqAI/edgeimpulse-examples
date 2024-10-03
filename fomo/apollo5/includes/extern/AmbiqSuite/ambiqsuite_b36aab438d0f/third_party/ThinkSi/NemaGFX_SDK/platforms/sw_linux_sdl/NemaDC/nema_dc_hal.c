/* TSI 2023.oem */
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
//  This file can be modified by OEMs as specified in the license agreement.
//
//  For further information please contact:
//
//                    Think Silicon Single Member PC
//                    http://www.think-silicon.com
//                    Patras Science Park
//                    Rion Achaias 26504
//                    Greece
// -----------------------------------------------------------------------------

#include "interpose.h"
#include "nema_dc_hal.h"

#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/msg.h>
#include <errno.h>
#include <SDL2/SDL.h>


#define SCREEN_WIDTH   800
#define SCREEN_HEIGHT  600

SDL_Renderer *SDL_ren;

int32_t nemadc_sys_init (void)
{
	_FUNCTION_PREAMBLE

	uint32_t rval = 0;

	_FUNCTION_BODY

	_FUNCTION_POSTAMBLE

	return rval;
}

void nemadc_wait_vsync (void)
{
	// _FUNCTION_PREAMBLE

	_FUNCTION_BODY

	_FUNCTION_POSTAMBLE

}

uint32_t nemadc_reg_read (uint32_t reg)
{
	_FUNCTION_PREAMBLE

	uint32_t rval = 0;

	_FUNCTION_BODY

	_FUNCTION_POSTAMBLE

	return rval;
}

void nemadc_reg_write (uint32_t reg,uint32_t value)
{
	_FUNCTION_PREAMBLE

	_FUNCTION_BODY

	_FUNCTION_POSTAMBLE

}
