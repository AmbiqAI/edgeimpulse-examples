//*****************************************************************************
//
//  lv_ambiq_nema_hal.h
//! @file
//!
//! @brief Support functions for nema_hal.c, these functions
//! are called from nema_hal.c to allocate heap memories for NemaSDK.
//!
//
//*****************************************************************************

//*****************************************************************************
//
// ${copyright}
//
// This is part of revision ${version} of the AmbiqSuite Development Package.
//
//*****************************************************************************
#ifndef LV_AMBIQ_NEMA_HAL_H
#define LV_AMBIQ_NEMA_HAL_H

#include "lvgl.h"

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * Malloc for NemaSDK
 */
extern void* lv_ambiq_nema_hal_malloc(int pool, uint32_t size_in_byte);

/**
 * Free for NemaSDK
 */
extern void lv_ambiq_nema_hal_free(void* ptr);

#define NEMA_CUSTOM_MALLOC(pool, size) lv_ambiq_nema_hal_malloc(pool, size)
#define NEMA_CUSTOM_FREE(ptr) lv_ambiq_nema_hal_free(ptr)

lv_res_t lv_ambiq_nema_gpu_power_on(void);
lv_res_t lv_ambiq_nema_gpu_check_busy_and_suspend(void);
void lv_ambiq_nema_gpu_isr_cb(int last_cl_id);

extern void nema_build_cl_start(void);
extern void nema_build_cl_end(void);

#define NEMA_BUILDCL_START                          nema_build_cl_start();
#define NEMA_BUILDCL_END                            nema_build_cl_end();

#ifdef __cplusplus
}
#endif

#endif // LV_AMBIQ_NEMA_HAL_H