/**
 ****************************************************************************************
 *
 * @file adev_drv_spk.h
 *
 * @brief Adev driver of amplifier TAS2560 on Audio Peripheral Card.
 *
 * Copyright (C) Ambiq Micro
 *
 *
 ****************************************************************************************
 */

#ifndef _ADEV_DRV_SPK_H_
#define _ADEV_DRV_SPK_H_

#include "adev_comm.h"

#if defined __cplusplus
extern "C"
{
#endif

    // ADEV Speaker configuration data structure
    typedef struct
    {
        adev_conf_t comm; // ADEV configuration common header
    } adev_conf_spk_t;

    // ADEV Speaker Driver
    extern const adev_drv_t adev_drv_spk;

#if defined __cplusplus
}
#endif
#endif /* _ADEV_DRV_SPK_H_ */
