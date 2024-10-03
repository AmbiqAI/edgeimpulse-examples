/**
 ****************************************************************************************
 *
 * @file asp.h
 *
 * @brief Declaration of Auditory Signal Processing(ASP) API. ASP integrates
 *AEC, NS, DRC and AGC together to reduce some duplicate processing
 *
 * Copyright (c) 2024, Ambiq Micro, Inc.
 * All rights reserved.
 *
 ****************************************************************************************
 */

#ifndef _ASP_H_
#define _ASP_H_

#if defined __cplusplus
extern "C"
{
#endif

    typedef struct
    {
        short AEC_Param[10];
        short NS_Param[10];
        short AGC_Param[10];
        short DRC_Param[10];
    } objASP_Param;

    /**
    ****************************************************************************************
    * @brief Initialize the modules of ASP
    * @param ASP_PARAM ASP parameter struct pointer
    * @param samplerate Audio sample rate
    * @return asp struct pointer
    ****************************************************************************************
    */
    void *asp_init(objASP_Param *ASP_PARAM, int samplerate);

    /**
    ****************************************************************************************
    * @brief ASP process a frame, include AEC,NS,AGC and DRC.
    * @param ptr asp struct pointer
    * @param mic_buf rec Signal from the microphone (near end + far end echo)
    * @param ref_buf play Signal played to the speaker(received from far end)
    * @param ref_buf Returns near-end signal with speech signal process
    * @param data_length Audio data sample number
    * @return None
    ****************************************************************************************
    */
    void asp_process(void *ptr, short *mic_buf, short *ref_buf, short *out_buf,
                     short data_length);

    /**
    ****************************************************************************************
    * @brief Free memory of asp struct
    * @param ptr Pointer of asp struct
    * @return None
    ****************************************************************************************
    */
    void asp_uninit(void *ptr);

#if defined __cplusplus
}
#endif

#endif // _ASP_H_
