//*****************************************************************************
//
//! @file ai_callback.c
//!
//! @brief AI callback function example
//!
//*****************************************************************************

//*****************************************************************************
//
// ${copyright}
//
// This is part of revision ${version} of the AmbiqSuite Development Package.
//
//*****************************************************************************

#include <stdint.h>
#include <string.h>

#include "am_util_stdio.h"
#include "am_util_debug.h"

#include "ai_callback.h"
#include "seCntrlClass.h"
#include "ambiq_nnsp_const.h"
#include "nn_speech.h"
#include "arm_math.h"

seCntrlClass cntrl_inst;
NNSPClass *pt_nnsp;

int32_t nnse_init(void *para, uint32_t size)
{
    // Init NNSE
    am_util_debug_printf("Init NNSE\n");

    seCntrlClass_init(&cntrl_inst);
    seCntrlClass_reset(&cntrl_inst);
    pt_nnsp = (NNSPClass*) cntrl_inst.pt_nnsp;
    pt_nnsp->pt_params->pre_gain_q1 = 3;

    return 0;
}

int32_t nnse_process(int16_t *in, int16_t *out, uint32_t sample_num)
{
    seCntrlClass_exec(&cntrl_inst, in, out);

    return 0;
}

void nnse_deinit(void)
{

}
