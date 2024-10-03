/**
 ****************************************************************************************
 *
 * @file gain.h
 *
 * @brief API of gain tuning algorithm.
 *
 * Copyright (c) 2024, Ambiq Micro, Inc.
 * All rights reserved.
 *
 ****************************************************************************************
 */

#ifndef _GAIN_H_
#define _GAIN_H_

#if defined __cplusplus
extern "C"
{
#endif

    /**
    ****************************************************************************************
    * @brief Gain tuning function for signed 16 bits audio PCM data.
    * @param gain Gain value(Q7,range(0,32767)).
    * @param in Pointer of input audio PCM data.
    * @param out Pointer of output audio PCM data.
    * @param len Sample number of input data. 16 bits per sample.
    ****************************************************************************************
    */
    void gain_s16(int gain, short *in, short *out, int len);

    /**
    ****************************************************************************************
    * @brief Gain tuning function for signed 32 bits audio PCM data.
    * @param gain Gain value(Q7,range(0,32767)).
    * @param in Pointer of input audio PCM data.
    * @param out Pointer of output audio PCM data.
    * @param len Sample number of input data. 32 bits per sample.
    ****************************************************************************************
    */
    void gain_s32(int gain, int *in, int *out, int len);

#if defined __cplusplus
}
#endif

#endif // _GAIN_H_
