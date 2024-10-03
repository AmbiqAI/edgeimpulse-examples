/**
 ****************************************************************************************
 *
 * @file iir_filter.h
 *
 * @brief Declaration of iir filter API.
 *
 * Copyright (c) 2024, Ambiq Micro, Inc.
 * All rights reserved.
 *
 ****************************************************************************************
 */

#ifndef _IIR_FILTER_H_
#define _IIR_FILTER_H_

#if defined __cplusplus
extern "C"
{
#endif

    void *IIR_Filter_Init(float *ptNUM, float *ptDen, short order);
    void  IIR_Filter(void *ptr, short *filter_fifo, int sample_n);
    void  IIR_Filter_uninit(void *ptr);

#if defined __cplusplus
}
#endif

#endif // _IIR_FILTER_H_
