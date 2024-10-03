//*****************************************************************************
//
//! @file am_util_fonts.h
//!
//! @brief Define Ambiq Micro fonts.
//!
//! This file defines available fonts for displaying on an LCD screen.
//!
//! @addtogroup fonts Fonts
//! @ingroup utils
//! @{
//
//*****************************************************************************

//*****************************************************************************
//
// ${copyright}
//
// This is part of revision ${version} of the AmbiqSuite Development Package.
//
//*****************************************************************************
#ifndef AM_UTIL_FONTS_H
#define AM_UTIL_FONTS_H

#ifdef __cplusplus
extern "C"
{
#endif

//*****************************************************************************
//
// C99
//
//*****************************************************************************
#include <stdint.h>
#include <stdbool.h>

//*****************************************************************************
//
//! Font definitions
//
//*****************************************************************************
typedef enum
{
    AM_FONT_SELECT_AMBIQ_NONE        = 0,
    AM_FONT_SELECT_AMBIQSIMPLE_5x8,
    AM_FONT_SELECT_PIXELDREAMS_11x28,
    AM_FONT_SELECT_PIXELDREAMS_09x20,
    AM_FONT_SELECT_PIXELDREAMS_08x17,

    AM_FONT_SELECT_VALID_MIN = AM_FONT_SELECT_AMBIQSIMPLE_5x8,
    AM_FONT_SELECT_VALID_MAX = AM_FONT_SELECT_PIXELDREAMS_08x17
} am_font_select_t;

#define AM_FONT_SELECT_INVALID  0xFFFFFFFF

#define AM_FONT_ATTR_ASCII              0x00000001  // ASCII character set

//*****************************************************************************
//
//! Font lookup structure
//! This structure provides information related to the selection of a font.
//
//
//*****************************************************************************
typedef struct
{

    //
    //! Identifier, a unique value for each font.
    //
    uint32_t ui32ID;

    //
    //! Pointer to the font data.
    //
    uint8_t *pu8Font;

    //
    //! Font width, height, and recommended spacing.
    //
    uint32_t ui32FontWidth;
    uint32_t ui32FontHeight;
    uint32_t ui32FontSpacingW;
    uint32_t ui32FontSpacingH;

    //
    //! Number of bytes required for each character in the table.
    //! This value can then be used to compute an offset to the character.
    //! Most of the font characters are bit padded to get to a 32-bit offset.
    //! For those characters, this value is computed as:
    //!      int((w*h)+31/32) * 4
    //! If the character is padded out to a byte, the formula is simply:
    //!      int((w*h)+7/8)
    //!
    //! Finally, note that this padding applies only to the entire bitmap of the
    //! character, not for each individual row in the bitmap.
    //
    uint32_t ui32BytesPerChar;

    //
    //! Full set of characters, or numerals only.
    //! This is used for fonts that may implement numerals only.
    //
    uint32_t ui32Attr;

    //
    //! Specify Unicode-sized range of valid chars.
    //
    uint16_t ui16RangeLo;
    uint16_t ui16RangeHi;
}
am_util_fonts_selector_t;

//*****************************************************************************
//
// Fonts
//
//*****************************************************************************
//
//! Font characteristics lookup table
//
extern const am_util_fonts_selector_t g_sFontSelectorTbl[];

//
//! @name Indexed font bitmap tables.
//! @{
//
extern const uint8_t am_util_font11x28[][40];
extern const uint8_t am_util_font8x17[][20];
extern const uint8_t am_util_font9x20[][24];
extern const uint8_t am_util_font5x8[][5];
extern const uint8_t am_util_font8x5[][5];
//! @}

#ifdef __cplusplus
}
#endif

#endif // AM_UTIL_FONTS_H

//*****************************************************************************
//
// End Doxygen group.
//! @}
//
//*****************************************************************************

