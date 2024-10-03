/**
 ****************************************************************************************
 *
 * @file agc.h
 *
 * @brief Declaration of Automatic Gain Control(AGC) API.
 *
 * Copyright (c) 2024, Ambiq Micro, Inc.
 * All rights reserved.
 *
 ****************************************************************************************
 */

#ifndef _AGC_H_
#define _AGC_H_

#if defined __cplusplus
extern "C"
{
#endif

    // #define FIX_POINT //for vs fixpoint project
    //  #define FLOAT     //for vs float project

    /**
    ****************************************************************************************
    * @brief Initialize the parameters of AGC
    * @param samplerate Audio sample rate
    * @param frame_len Audio frame length
    * @param target_level target amplitude value
    * @param attack_time Attack time
    * @param decay_time Decay time
    * @return AGC struct pointer
    ****************************************************************************************
    */
    void *agc_init(int samplerate, int frame_len, int target_level,
                   int attack_time, int decay_time);

    /**
    ****************************************************************************************
    * @brief AGC algorithm
    * @param ptr Pointer of AGC struct
    * @param in/out Pointer of audio input data buffer
    * @param sample_n Audio data sample number
    * @return None
    ****************************************************************************************
    */
    void agc_process(void *ptr, short *in, int sample_n, short dt_flag);

    /**
    ****************************************************************************************
    * @brief Free memory of AGC struct
    * @param ptr Pointer of AGC struct
    * @return None
    ****************************************************************************************
    */
    int agc_uninit(void *ptr);

    /**
    ****************************************************************************************
    * @brief get AGC version info
    * @param None
    * @return AGC version info pointer
    ****************************************************************************************
    */
    char *agc_getversion();

#if defined __cplusplus
}
#endif

#endif // _AGC_H_
