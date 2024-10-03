//*****************************************************************************
//
//  lv_ambiq_decoder.h
//! @file
//!
//! @brief Ambiq format image texture decoder.
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


#ifndef LV_AMBIQ_DECODER_H
#define LV_AMBIQ_DECODER_H

#include "lvgl.h"

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * Initialize the ambiq image decoder module
 */
extern void _lv_img_decoder_ambiq_add(void);

#ifdef __cplusplus
}
#endif

#endif // LV_AMBIQ_DECODER_H