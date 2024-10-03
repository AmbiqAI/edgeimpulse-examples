//*****************************************************************************
//
//! @file lv_ambiq_touch.h
//!
//! @brief APIs for touch feature on LVGL.
//!
//!
//*****************************************************************************

//*****************************************************************************
//
// ${copyright}
//
// This is part of revision ${version} of the AmbiqSuite Development Package.
//
//*****************************************************************************

//*****************************************************************************
//
// Global includes for this project.
//
//*****************************************************************************
#ifndef LV_AMBIQ_TOUCH_H
#define LV_AMBIQ_TOUCH_H

#ifdef __cplusplus
extern "C"
{
#endif

#include "lvgl.h"

//*****************************************************************************
//
// External function definitions
//
//*****************************************************************************
extern void lv_ambiq_touch_read(lv_indev_drv_t * indev_drv, lv_indev_data_t * data);
extern void lv_ambiq_touch_init(void);

#ifdef __cplusplus
}
#endif

#endif // LV_AMBIQ_TOUCH_H
