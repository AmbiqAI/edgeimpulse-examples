/**
 ****************************************************************************************
 *
 * @file fade.h
 *
 * @brief Declaration of Fade algorithm API.
 *
 * Copyright (c) 2024, Ambiq Micro, Inc.
 * All rights reserved.
 *
 ****************************************************************************************
 */

#ifndef _FADE_H_
#define _FADE_H_

#ifdef __cplusplus
extern "C"
{
#endif

    /**
    ****************************************************************************************
    * @brief Fade initialization function. Fade will use change gain from
    *gain_start to gain_end within fade_sample samples.
    * @param gain Fading gain
    * @param fade_sample Fading sample number. 16 bits per sample.
    * @return Fade instance pointer
    ****************************************************************************************
    */
    void *fade_init(float gain, int fade_sample);

    /**
    ****************************************************************************************
    * @brief Fade reset function.
    * @param ptr  Fade instance pointer
    * @param gain Fading gain
    * @param fade_sample Fading sample number. 16 bits per sample.
    ****************************************************************************************
    */
    void fade_reset(void *ptr, float gain, int fade_sample);

    /**
    ****************************************************************************************
    * @brief Fade processing function. Input and output are the same pointer.
    * @param ptr  Fade instance pointer
    * @param data Audio PCM data pointer. Input and output are the same pointer.
    * @param data_samples Data sample number. 16 bits per sample.
    ****************************************************************************************
    */
    void fade_process(void *ptr, short *data, short data_samples);

    /**
    ****************************************************************************************
    * @brief Fade deinitialization function.
    * @param ptr  Fade instance pointer
    ****************************************************************************************
    */
    void fade_uninit(void *ptr);

#ifdef __cplusplus
}
#endif

#endif // _FADE_H_
