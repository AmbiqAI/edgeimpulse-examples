/**
 ****************************************************************************************
 *
 * @file mbdrc.h
 *
 * @brief Declaration of Multi-Band Dynamic Range Control(MBDRC) API.
 *
 * Copyright (c) 2024, Ambiq Micro, Inc.
 * All rights reserved.
 *
 ****************************************************************************************
 */

#ifndef _MBDRC_H_
#define _MBDRC_H_

#include <stdbool.h>

#if defined __cplusplus
extern "C"
{
#endif

    /**
     * @brief Get the persistent RAM size of MBDRC, which is used for user to
     * allocate the memory for this instance.
     *
     * @return Persistent RAM size in bytes.
     */
    int mbdrc_get_persistent_ram_size(void);

    /**
    ****************************************************************************************
    * @brief Initialize the parameters of MBDRC
    *
    * @attention This function need to be called after
    *mbdrc_get_persistent_ram_size function, and user needs to allocate static
    *memory first for this instance before this function, the static memory size
    *must be above or equal than persistent RAM size.
    *
    * @param ptr Pointer to the persistent RAM allocated by the user.
    * @param samplerate Audio sample rate
    * @param band_num subband number, range(1,9]
    * @param BoundHigh Pointer of subband frequency table.
    * @param CompThrshold Pointer of subband compress threshold table.
    * @param CompSlop Pointer of subband compress slop table.
    * @param framelen Audio frame length, must be 128 samples
    *
    * @return true if initializing successfully, otherwise false.
    ****************************************************************************************
    */
    bool mbdrc_init(void *ptr, int samplerate, short band_num, int *BoundHigh,
                    short *CompThrshold, float *CompSlop, short framelen);

    /**
    ****************************************************************************************
    * @brief MBDRC algorithm
    * @param ptr Pointer of MBDRC struct
    * @param input Pointer of audio input data buffer
    * @param output Pointer of audio output data buffer
    * @param frame_len Audio data sample number
    * @param in_flag Type of input sample,0--time domain,1--freq domain
    * @param out_flag Type of output sample,0--time domain,1--freq domain
    * @return None
    ****************************************************************************************
    */
    void mbdrc_process(void *ptr, short *input, short *output, int frame_len,
                       short in_flag, short out_flag);

    /**
    ****************************************************************************************
    * @brief Get byte size of out
    * @param frame_len Audio data sample number
    * @param out_flag Type of output sample,0--time domain,1--freq domain
    * @return Byte size of out
    ****************************************************************************************
    */
    short mbdrc_get_out_bytesize(int frame_len, short out_flag);

    /**
    ****************************************************************************************
    * @brief Free memory of MBDRC struct
    * @param ptr Pointer of MBDRC struct
    * @return None
    ****************************************************************************************
    */
    void mbdrc_uninit(void *ptr);

    /**
    ****************************************************************************************
    * @brief get MBDRC version info
    * @param None
    * @return MBDRC version info pointer
    ****************************************************************************************
    */
    char *mbdrc_getversion();

#if defined __cplusplus
}
#endif

#endif // _MBDRC_H_
