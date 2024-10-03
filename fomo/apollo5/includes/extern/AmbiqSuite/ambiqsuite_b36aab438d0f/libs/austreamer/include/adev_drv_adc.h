/**
 ****************************************************************************************
 *
 * @file adev_drv_adc.h
 *
 * @brief Declaration of the Audio Device ADC Driver API.
 *
 * Copyright (C) Ambiq Micro
 *
 *
 ****************************************************************************************
 */

#ifndef _ADEV_DRV_ADC_H_
#define _ADEV_DRV_ADC_H_

#include "adev_comm.h"

#if defined __cplusplus
extern "C"
{
#endif

    // ADEV ADC configuration data structure
    typedef struct
    {
        adev_conf_t comm; // ADEV confiuration common header
    } adev_conf_adc_t;

    // ADEV ADC Driver
    extern const adev_drv_t adev_drv_adc;

#if defined __cplusplus
}
#endif
#endif /* _ADEV_DRV_ADC_H_ */
