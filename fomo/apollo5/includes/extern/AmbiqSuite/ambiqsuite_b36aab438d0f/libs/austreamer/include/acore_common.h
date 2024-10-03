/**
 ******************************************************************************
 *
 * @file acore_common.h
 *
 * @brief Declaration Audio core common definition.
 *
 * Copyright (C) Ambiq Micro
 *
 *
 ******************************************************************************
 */

#ifndef _ACORE_COMMON_H_
#define _ACORE_COMMON_H_

#if defined __cplusplus
extern "C"
{
#endif

// ACORE error code
#define ACORE_OK                  0
#define ACORE_ERR                 1
#define ACORE_ERR_DRING_OVER_RUN  2
#define ACORE_ERR_DRING_UNDER_RUN 3

    /**
     * Message Endpoint
     */
    enum
    {
        MSG_EP_AGENT_DSP = (0x01 << 2),
        MSG_EP_AGENT_MCU = (0x02 << 2),
        MSG_EP_AGENT_PC  = (0x03 << 2),
    };

    /**
     * Object id
     */
    enum
    {
        OBJ_INVALID = 0,
        OBJ_USER,
        OBJ_ACORE_TOOL,
        OBJ_ACORE_CODER_SRC,
        OBJ_ACORE_CODER_SINK,
        OBJ_ACORE_DSP_SRC,
        OBJ_ACORE_DSP_SINK,
        OBJ_ACORE_DSP_GAIN,
        OBJ_ACORE_DSP_AGC,
        OBJ_ACORE_DSP_DRC_UL,
        OBJ_ACORE_DSP_DRC_DL,
        OBJ_ACORE_DSP_PEQ_UL,
        OBJ_ACORE_DSP_PEQ_DL,
        OBJ_ACORE_DSP_AEC,
        OBJ_ACORE_DSP_NS,
        OBJ_ACORE_DSP_ASP,
        OBJ_ACORE_DSP,
        OBJ_ACORE_DSP_MBDRC,
        OBJ_ACORE_DSP_IIR_FILTER,
        OBJ_ACORE_DSP_MIXER,
        OBJ_ACORE_DSP_NNSE,
        OBJ_ACORE_DSP_VAD,
        OBJ_ACORE_DSP_WNR,

        OBJ_ACORE_MAX
    };

#if defined __cplusplus
}
#endif
#endif /* _ACORE_COMMON_H_ */
