/**
 ****************************************************************************************
 *
 * @file ns.h
 *
 * @brief Declaration of Noise Suppression(NS) external API.
 *
 * Copyright (c) 2024, Ambiq Micro, Inc.
 * All rights reserved.
 *
 ****************************************************************************************
 */

#ifndef _NS_H_
#define _NS_H_

#ifdef __cplusplus
#if __cplusplus
extern "C"
{
#endif
#endif

    /**
    ****************************************************************************************
    * @brief Initialization function.
    * @param swSampleRate Sample rate.
    * @param ns_level noise suppression level, range[0,4].
    * @return NS instance pointer.
    ****************************************************************************************
    */
    void *ns_init(short swSampleRate, short ns_level);

    /**
    ****************************************************************************************
    * @brief Processing data. Frame sizes of input and output are the same.
    * @param ptr Sample rate.
    * @param in Input data pointer.
    * @param out Output data pointer.
    * @param frame_len Frame length. One frame is 16 bits.
    * @param dt_flag Frame double detection result.
    ****************************************************************************************
    */
    void ns_process(void *ptr, short *in, short *out, short frame_len,
                    short dt_flag);

    /**
    ****************************************************************************************
    * @brief Deinitialization function
    * @param ptr NS instance pointer.
    ****************************************************************************************
    */
    void ns_deinit(void *ptr);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif // _NS_H_
