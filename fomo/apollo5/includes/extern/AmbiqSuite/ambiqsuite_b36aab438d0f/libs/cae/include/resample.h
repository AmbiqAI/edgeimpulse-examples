/**
 ****************************************************************************************
 *
 * @file resample.h
 *
 * @brief API of sample rate converter(SRC) algorithm.
 *
 * Copyright (c) 2024, Ambiq Micro, Inc.
 * All rights reserved.
 *
 ****************************************************************************************
 */

#ifndef _RESAMPLE_H_
#define _RESAMPLE_H_

#ifdef __cplusplus
extern "C"
{
#endif

    /**
    ****************************************************************************************
    * @brief SRC initialization function.
    * @param samplerate_in Sample rate of input data.
    * @param samplerate_out Sample rate of output data.
    * @return SRC instance pointer.
    ****************************************************************************************
    */
    void *resample_init(int samplerate_in, int samplerate_out);

    /**
    ****************************************************************************************
    * @brief SRC processing function.
    * @param ptr SRC instance pointer.
    * @param in Pointer of input audio PCM data
    * @param out Pointer of output audio PCM data
    * @param frame_length_in Sample number of input data. 16 bits per sample.
    ****************************************************************************************
    */
    void resample_process(void *ptr, short *in, short *out,
                          short frame_length_in);

    /**
    ****************************************************************************************
    * @brief SRC deinitialization function.
    * @param ptr SRC instance pointer.
    * @return Success: return 0, Failure: other value.
    ****************************************************************************************
    */
    int resample_uninit(void *ptr);

#ifdef __cplusplus
}
#endif

#endif // _RESAMPLE_H_
