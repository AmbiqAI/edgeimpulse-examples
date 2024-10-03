/**
 ****************************************************************************************
 *
 * @file adev_pvt_def.h
 *
 * @brief Audio Device Private Define.
 *
 * Copyright (C) Ambiq Micro
 *
 *
 ****************************************************************************************
 */

#ifndef _ADEV_PVT_DEF_H_
#define _ADEV_PVT_DEF_H_

#include "adev_comm.h"
#include "adev_drv_i2s.h"
#include "adev_drv_pdm.h"
#include "adev_drv_adc.h"
#include "adev_drv_clk.h"

#if defined __cplusplus
extern "C"
{
#endif

// Default duration for all the endpoints
#define DEFAULT_DURATION 4000 // 4ms

// Convert device ID to endpoint ID
#define ID_TO_EP(id, type) (type + id)

// Convert endpoint ID to device ID
#define EP_TO_ID(ep, type) (ep - type)

// Check API run in ADEV inactive state
#define CHECK_OP_INACTIVE                                                      \
    do                                                                         \
    {                                                                          \
        if (adev.state != ADEV_INACTIVE)                                       \
            return ADEV_ERR_OP_INACTIVE;                                       \
    } while (0)

// Check API run in ADEV active state
#define CHECK_OP_ACTIVE                                                        \
    do                                                                         \
    {                                                                          \
        if (adev.state != ADEV_ACTIVE)                                         \
            return ADEV_ERR_OP_ACTIVE;                                         \
    } while (0)

// Check API run in ADEV uninitialized state
#define CHECK_OP_NOT_INIT                                                      \
    do                                                                         \
    {                                                                          \
        if (adev.state != ADEV_NOT_INIT)                                       \
            return;                                                            \
    } while (0)

// Check endpoint ID is in the range of adev_ep_id_t
#define CHECK_EP_ID(ep)                                                        \
    do                                                                         \
    {                                                                          \
        if (ep >= ADEV_EP_NUM)                                                 \
            return ADEV_ERR_EP_ID;                                             \
    } while (0)

// Check endpoint ID is in the range of I2S endpoints
#define CHECK_I2S_EP_ID(ep)                                                    \
    do                                                                         \
    {                                                                          \
        if (ep != ADEV_EP_I2S_0 && ep != ADEV_EP_I2S_1)                        \
            return ADEV_ERR_EP_ID;                                             \
    } while (0)

// Check endpoint ID is in the range of PDM endpoints
#define CHECK_PDM_EP_ID(ep)                                                    \
    do                                                                         \
    {                                                                          \
        if (ep != ADEV_EP_PDM_0)                                               \
            return ADEV_ERR_EP_ID;                                             \
    } while (0)

    // ADEV state definition
    typedef enum
    {
        ADEV_NOT_INIT,
        ADEV_INACTIVE,
        ADEV_ACTIVE
    } adev_state_t;

    // Configuration instances
    typedef struct
    {
        adev_conf_i2s_t conf_i2s[I2S_EP_NUM]; // I2S driver configuration
        adev_conf_pdm_t conf_pdm[PDM_EP_NUM]; // PDM driver configuration
        adev_conf_adc_t conf_adc[ADC_EP_NUM]; // ADC driver configuration
        adev_conf_clk_t
            conf_clk; // Clock driver configuration. Clock isn't endpoint.
        adev_conf_t *confs[ADEV_EP_NUM]; // List of all endpoint confiurations.
    } adev_devs_conf_t;

    // ADEV private data structure
    typedef struct
    {
        adev_state_t     state; // ADEV state.
        adev_devs_conf_t conf;  // ADEV configurations.
    } adev_pvt_t;

    // Private instance shared with adev.c and adev_setting.c
    extern adev_pvt_t adev;

#if defined __cplusplus
}
#endif
#endif /* _ADEV_PVT_DEF_H_ */
