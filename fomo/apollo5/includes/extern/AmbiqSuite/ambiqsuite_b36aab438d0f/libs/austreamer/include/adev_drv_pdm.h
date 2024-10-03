/**
 ****************************************************************************************
 *
 * @file adev_drv_pdm.h
 *
 * @brief Declaration of the Audio Device PDM Driver API.
 *
 * Copyright (C) Ambiq Micro
 *
 *
 ****************************************************************************************
 */

#ifndef _ADEV_DRV_PDM_H_
#define _ADEV_DRV_PDM_H_

#include "adev_comm.h"

#if defined __cplusplus
extern "C"
{
#endif

    // PDM configuration data structure
    typedef struct
    {
        adev_conf_t comm; // ADEV confiuration common header

    } adev_conf_pdm_t;

    // ADEV PDM Driver
    extern const adev_drv_t adev_drv_pdm;

#if defined __cplusplus
}
#endif
#endif /* _ADEV_DRV_PDM_H_ */
