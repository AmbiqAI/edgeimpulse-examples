/**
 ****************************************************************************************
 *
 * @file drc.h
 *
 * @brief Declaration of Dynamic Range Control(DRC) API.
 *
 * Copyright (c) 2024, Ambiq Micro, Inc.
 * All rights reserved.
 *
 ****************************************************************************************
 */

#ifndef _DRC_H_
#define _DRC_H_

#if defined __cplusplus
extern "C"
{
#endif

    // #define FIX_POINT //for fixpoint version
    // #define FLOAT     //for float version

#if defined(FIX_POINT) || defined(CEVA_BX)
    typedef int real;
#else
typedef float real;
#endif
    /**
    ****************************************************************************************
    * @brief Initialize the parameters of DRC
    * @param samplerate Audio sample rate
    * @param frame_len Audio frame length
    * @param knee_thrd Knee threshold
    * @param noise_gate Noise gate
    * @param slope Slope, Q10
    * @param attack_time Attack time
    * @param decay_time Decay time
    * @param channel_num channel number
    * @return DRC struct pointer
    ****************************************************************************************
    */
    void *drc_init(int samplerate, int frame_len, int knee_thrd, int noise_gate,
                   int slope, int attack_time, int decay_time, int channel_num);

    /**
    ****************************************************************************************
    * @brief DRC algorithm
    * @param ptr Pointer of DRC struct
    * @param in Pointer of audio input data buffer
    * @param out Pointer of audio output data buffer
    * @param sample_n Audio data sample number of per channel,non-interleaved
    *data format
    * @return None
    ****************************************************************************************
    */
    void drc_process(void *ptr, short *in, short *out, int sample_n);

    /**
    ****************************************************************************************
    * @brief Free memory of DRC struct
    * @param ptr Pointer of DRC struct
    * @return None
    ****************************************************************************************
    */
    void drc_uninit(void *ptr);

    /**
    ****************************************************************************************
    * @brief get DRC version info
    * @param None
    * @return DRC version info pointer
    ****************************************************************************************
    */
    char *drc_getversion();

#if defined __cplusplus
}
#endif

#endif // _DRC_H_
