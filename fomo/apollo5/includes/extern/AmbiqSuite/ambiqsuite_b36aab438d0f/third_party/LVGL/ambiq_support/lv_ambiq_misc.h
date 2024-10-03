//*****************************************************************************
//
//  lv_ambiq_misc.h
//! @file
//!
//! @brief APIs called by ambiq gpu acceleration lib.
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


#ifndef LV_AMBIQ_MISC_H
#define LV_AMBIQ_MISC_H

#include "lvgl.h"

#ifdef __cplusplus
extern "C"
{
#endif

extern const char* lv_ambiq_gpu_error_interpret(uint32_t error_code);
extern lv_color_t lv_ambiq_gpu_get_chroma_key(void);
extern void lv_ambiq_cache_invalid_draw_buffer(lv_disp_t * disp);
extern void lv_ambiq_cache_clean_draw_buffer(lv_disp_t * disp);
extern void lv_ambiq_cache_invalid_and_clean_draw_buffer(lv_disp_t * disp);
extern void lv_ambiq_cache_clean_mask_buffer(const lv_opa_t* mask, const lv_area_t* area);
extern void lv_ambiq_cache_clean_map_buffer(const lv_color_t* map, const lv_area_t* area);
#ifdef __cplusplus
}
#endif

#endif // LV_AMBIQ_MISC_H


