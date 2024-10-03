/**
 ****************************************************************************************
 *
 * @file fbaec.h
 *
 * @brief Declaration of Full-band Acoustic Echo Cancellation(AEC) API.
 *
 * Copyright (c) 2024, Ambiq Micro, Inc.
 * All rights reserved.
 *
 ****************************************************************************************
 */

#ifndef _FBAEC_H_
#define _FBAEC_H_

#include "pseudofloat.h"
#include "speex_types.h"

#ifdef __cplusplus
extern "C"
{
#endif

    /**
    ****************************************************************************************
    * @brief Initialization function.
    * @param  frame_size      Number of samples to process at one time(must be
    *64).
    * @param  aec_param       Configurable parameters,such as filter length,
    *fixdelay,etc...
    * @param  sampling_rate   sample rate of samples.
    * @return FB-AEC instance pointer.
    ****************************************************************************************
    */
    void *fbaec_init(int frame_size, short *aec_param, int sampling_rate);

    /**
    ****************************************************************************************
    * @brief Deinitialization function.
    * @param ptr FB-AEC instance pointer.
    ****************************************************************************************
    */
    void fbaec_deinit(void *ptr);

    /**
    ****************************************************************************************
    * @brief Performs echo cancellation. Data size is defined in fbaec_init().
    * Data size of input, reference and output are the same
    * @param ptr FB-AEC instance pointer.
    * @param in Input audio PCM data from microphone.
    * @param ref Reference data PCM data from playback.
    * @param out Output processed audio data PCM data.
    * @return FB-AEC double detection state.
    ****************************************************************************************
    */
    short fbaec_process(void *ptr, spx_int16_t *in, spx_int16_t *ref,
                        spx_int16_t *out);

    /**
    ****************************************************************************************
    * @brief Reset FB-AEC function. All the filter buffering data will be clean.
    * This API is called for tuning or debug.
    * @param ptr FB-AEC instance pointer.
    ****************************************************************************************
    */
    void fbaec_reset(void *ptr);

#ifdef __cplusplus
}
#endif

#endif // _FBAEC_H_
