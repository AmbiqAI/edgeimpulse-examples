/**
 ****************************************************************************************
 *
 * @file peq.h
 *
 * @brief Declaration of Parametric Equalizer(PEQ) API.
 *
 * Copyright (c) 2024, Ambiq Micro, Inc.
 * All rights reserved.
 *
 ****************************************************************************************
 */

#ifndef _PEQ_H_
#define _PEQ_H_

#if defined __cplusplus
extern "C"
{
#endif

    // #define FIX_POINT
    // #define FLOAT_POINT

    /**
    ****************************************************************************************
    * @brief Initialize the parameters of EQ
    * @param channel_num channel number(1 or 2)
    * @param total_gain gain factor for all band.
    * @param center_freq Pointer of center frequency table.
    * @param qfactor Pointer of center quality factor table.
    * @param dBgain Pointer of center gain factor table.
    * @param samplerate Audio sample rate.
    * @param band_num center frequency number.
    * @return EQ struct pointer
    ****************************************************************************************
    */
    void *peq_init(int channel_num, int total_gain, int *center_freq,
                   float *qfactor, int *dBgain, int samplerate, int band_num);

    /**
    ****************************************************************************************
    * @brief EQ process algorithm
    * @param ptr Pointer of EQ struct
    * @param filter_fifo in/out Pointer of audio input data buffer
    * @param sample_n Audio data sample number
    * @return None
    ****************************************************************************************
    */
    void peq_process(void *ptr, short *filter_fifo, int sample_n);

    /**
    ****************************************************************************************
    * @brief Free memory of EQ struct
    * @param ptr Pointer of EQ struct
    * @return None
    ****************************************************************************************
    */
    int peq_uninit(void *ptr);

    /**
    ****************************************************************************************
    * @brief get EQ version info
    * @param None
    * @return EQ version info pointer
    ****************************************************************************************
    */
    char *peq_getversion();

#if defined __cplusplus
}
#endif

#endif // _PEQ_H_
