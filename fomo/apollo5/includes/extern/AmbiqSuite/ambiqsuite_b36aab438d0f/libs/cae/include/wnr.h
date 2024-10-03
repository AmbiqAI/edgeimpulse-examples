/**
 ****************************************************************************************
 *
 * @file wnr.h
 *
 * @brief Declaration of Noise Suppression(NS) external API.
 *
 * Copyright (c) 2024, Ambiq Micro, Inc.
 * All rights reserved.
 *
 ****************************************************************************************
 */

#ifndef _WNR_H_
#define _WNR_H_

#include <stdbool.h>

#ifdef __cplusplus
#if __cplusplus
extern "C"
{
#endif
#endif

    /**
     * @brief Get the persistent RAM size of WNR, which is used for user to
     * allocate the memory for this instance.
     *
     * @return Persistent RAM size in bytes.
     */
    int wnr_get_persistent_ram_size(void);

    /**
    ****************************************************************************************
    * @brief Initialization function.
    *
    * @attention This function need to be called after
    *wnr_get_persistent_ram_size function, and user needs to allocate static
    *memory first for this instance before this function, the static memory size
    *must be above or equal than persistent RAM size.
    *
    * @param ptr Pointer to the persistent RAM allocated by the user.
    * @param swSampleRate Sample rate.
    * @param ns_level noise suppression level, range[0,4].
    *
    * @return true if initializing successfully, otherwise false.
    ****************************************************************************************
    */
    bool wnr_init(void *ptr, short swSampleRate, short ns_level);

    /**
    ****************************************************************************************
    * @brief Processing data. Frame sizes of input and output are the same.
    * @param ptr Sample rate.
    * @param in Input data pointer.
    * @param out Output data pointer.
    * @param frame_len Frame length. One frame is 16 bits.
    * @param dt_flag Frame double detection result.
    * @param in_flag Type of input sample,0--time domain,1--freq domain
    * @param out_flag Type of output sample,0--time domain,1--freq domain
    ****************************************************************************************
    */
    void wnr_process(void *ptr, short *in, short *out, short frame_len,
                     short in_flag, short out_flag, short dt_flag);

    /**
    ****************************************************************************************
    * @brief Get byte size of out
    * @param frame_len Audio data sample number
    * @param out_flag Type of output sample,0--time domain,1--freq domain
    * @return Byte size of out
    ****************************************************************************************
    */
    short wnr_get_out_bytesize(int frame_len, short out_flag);

    /**
    ****************************************************************************************
    * @brief Deinitialization function
    * @param ptr NS instance pointer.
    ****************************************************************************************
    */
    void wnr_deinit(void *ptr);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif // _NS_H_
