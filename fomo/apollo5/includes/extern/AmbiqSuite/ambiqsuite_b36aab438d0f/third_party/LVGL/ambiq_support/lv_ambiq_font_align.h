//*****************************************************************************
//
//  lv_ambiq_font_align.h
//! @file
//!
//! @brief Ambiq font realignment.
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


#ifndef LV_AMBIQ_FONT_ALIGN_H
#define LV_AMBIQ_FONT_ALIGN_H

#include "lvgl.h"

#ifdef __cplusplus
extern "C"
{
#endif

//*****************************************************************************
//
//! @brief Get the font bitmap length after alignment.
//!
//! @param font - Font to be align.
//!
//! @return the bitmap length after alignment.
//
//*****************************************************************************
extern uint32_t lv_ambiq_bitmap_length_get(const lv_font_fmt_txt_dsc_t* font);

//*****************************************************************************
//
//! @brief Align the A4 format font bitmap
//!
//! @param font - Font to be align.
//! @param new_bitmap_addr - a buffer to hold the aligned bitmap, 
//!                         call lv_ambiq_bitmap_length_get to get the its length .
//!
//! @note This function will modify the glyph_bitmap pointer to the input new_bitmap_addr.
//!
//! @return None.
//
//*****************************************************************************
extern void lv_ambiq_A4_font_align(lv_font_fmt_txt_dsc_t* font, uint8_t* new_bitmap_addr);

#ifdef __cplusplus
}
#endif

#endif // LV_AMBIQ_FONT_ALIGN_H