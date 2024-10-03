/**
 ****************************************************************************************
 *
 * @file adev_drv_clk.h
 *
 * @brief Declaration of the Audio Device clk Driver API.
 *
 * Copyright (C) Ambiq Micro
 *
 *
 ****************************************************************************************
 */

#ifndef _ADEV_DRV_CLK_H_
#define _ADEV_DRV_CLK_H_

#include "adev_comm.h"

#if defined __cplusplus
extern "C"
{
#endif

    // ADEV Clock configuration data structure
    typedef struct
    {
        adev_conf_t comm; // ADEV confiuration common header
    } adev_conf_clk_t;

    // ADEV Clock Driver
    extern const adev_drv_t adev_drv_clk;

#if defined __cplusplus
}
#endif
#endif /* _ADEV_DRV_CLK_H_ */
