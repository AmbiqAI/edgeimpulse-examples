//*****************************************************************************
//
//! @file am_util_draw_2d1bpp.h
//!
//! @brief High level 1bpp drawing routines for Ambiq Micro products.
//!
//! This file contains the high level, 1bpp drawing routines that are eventually
//! displayed on an LCD screen.  This routines draw into a framebuffer and
//! do not draw directly to the display.
//!
//! The framebuffer itself is allocated at a higher level, thus a pointer to
//! the framebuffer is required for each call.
//!
//! @addtogroup 2d1bpp 2D1LBPP - LBPP Drawing Routines
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
#ifndef AM_UTIL_DRAW_2D1BPP_H
#define AM_UTIL_DRAW_2D1BPP_H

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
// Macro definitions
//
//*****************************************************************************

//*****************************************************************************
// Drawing operations:
//
//  These ROPs apply only to am_util_draw_2d1bpp_bitmap_draw():
//      AM_UTIL_DRAW_ROPS_INVERT
//      AM_UTIL_DRAW_ROPS_MIRROR
//
//  These ROPs apply to all drawing functions:
//      AM_UTIL_DRAW_ROPS_COPY
//      AM_UTIL_DRAW_ROPS_XOR
//      AM_UTIL_DRAW_ROPS_OR
//
//*****************************************************************************
//
// AM_UTIL_DRAW_ROPS_COPY copies the bitmap pattern to the framebuffer.
// Note that AM_UTIL_DRAW_ROPS_NORMAL is deprecated.
//
#define AM_UTIL_DRAW_ROPS_COPY          0x0000
#define AM_UTIL_DRAW_ROPS_NORMAL        AM_UTIL_DRAW_ROPS_COPY

//
//! AM_UTIL_DRAW_ROPS_INVERT inverts the bitmap pattern then copies it to the
//! framebuffer.
//
#define AM_UTIL_DRAW_ROPS_INVERT        0x0001

//
//! AM_UTIL_DRAW_ROPS_MIRROR mirrors the bitmap pattern (rotating it 180 degrees)
//! then copies it to the framebuffer.
//
#define AM_UTIL_DRAW_ROPS_MIRROR        0x0002

//
//! AM_UTIL_DRAW_ROPS_XOR performs a Boolean operation (XOR, OR, AND) between
//! the bitmap pattern (including lines, rectanges, ellipses, etc) and the
//! existing framebuffer data.
//
#define AM_UTIL_DRAW_ROPS_XOR           0x0004
#define AM_UTIL_DRAW_ROPS_OR            0x0008

// #### INTERNAL BEGIN ####
// FUTURE OPTIONS, NOT YET IMPLEMENTED!
#define DISPLAY_BITMAP_ROTATE_90        0x0010
#define DISPLAY_BITMAP_ROTATE_270       0x0020
// #### INTERNAL END ####

//*****************************************************************************
//
//! Data structure used for display devices.
//!
//!  All members of this structure are initialized by the application using
//!   respective members of the display device structure, which is filled in
//!   by the call to the display driver init function.
//!  Therefore, this structure is intialized AFTER the display driver init
//!   function is called, but before calling any drawing function.
//
//*****************************************************************************
typedef struct
{

    //
    //! Framebuffer pointer.
    //! The framebuffer must be guaranteed to be 32-bit aligned.
    //
    uint8_t *pui8Framebuffer;

    //
    //! Display width (X dimension).
    //
    uint32_t ui32DrawWidth;

    //
    //! Display height (Y dimension).
    //
    uint32_t ui32DrawHeight;

    //
    //! The ui32DrawPolarity is set by the display driver and determines the
    //!  polarity used by the display for drawing the background.
    //! If 1, the display is cleared with all 0xFFs.
    //! If 0, the display is cleared with all 0x00s.
    //
    uint32_t ui32DrawPolarity;

    //*****************************************************************************
    //! Font selector is updated after a call to am_util_draw_2d1bpp_font_select().
    //*****************************************************************************
    //
    //! Font selector
    //
    uint32_t ui32FontSel;
}
am_util_draw_2d1bpp_context_t;

//*****************************************************************************
//
// External function definitions
//
//*****************************************************************************
//*****************************************************************************
//
//! @brief Return the value of a given pixel.
//!
//! @param psDrawContext - Pointer of the Drawing Context
//! @param ui32X - Coordinates of the desired pixel.
//! @param ui32Y - Coordinates of the desired pixel.
//!
//! @return Value of the requested device independent pixel.  Therefore
//! a 1 is returned if a pixel is on, 0 is returned if the pixel is off.
//
//*****************************************************************************
extern uint32_t am_util_draw_2d1bpp_pixel_get(
                            am_util_draw_2d1bpp_context_t *psDrawContext,
                            uint32_t ui32X, uint32_t ui32Y);

//*****************************************************************************
//
//! @brief Set a given pixel value at the given coordinates
//!        into the display framebuffer.
//!
//! @param psDrawContext - Pointer of the Drawing Context
//! @param ui32X - Coordinates of the desired pixel.
//! @param ui32Y - Coordinates of the desired pixel.
//! @param ui32Color   - Color to write to the given pixel.
//
//*****************************************************************************
extern void     am_util_draw_2d1bpp_pixel_set(
                            am_util_draw_2d1bpp_context_t *psDrawContext,
                            uint32_t ui32X, uint32_t ui32Y,
                            uint32_t ui32Color);

//*****************************************************************************
//
//! @brief Write a bitmap into the display framebuffer.
//!
//! @param psDrawContext     Pointer of the Drawing Context
//! @param pui8Bitmap        Pointer to the data array containing the bitmap.
//! @param ui32Width         Width of the bitmap in pixels.
//! @param ui32Height        Height of the bitmap in pixels.
//! @param ui32DstX          The screen location to place the bitmap.
//! @param ui32DstY          The screen location to place the bitmap.
//! @param ui32DrawingOps    Drawing options/operations include:
//!                       AM_UTIL_DRAW_ROPS_COPY   - Print the bitmap as-is
//!                       AM_UTIL_DRAW_ROPS_INVERT - Invert the color
//!                       AM_UTIL_DRAW_ROPS_MIRROR - Mirror the image (180)
//!                       AM_UTIL_DRAW_ROPS_XOR    - XOR the image with the
//!                                                    existing FB contents.
//!
//! Displays a monochrome bitmap to the display.
//!
//! Assumes the bitmap at pui8Bitmap[] is little-endian and thus MSB-oriented.
//! For example, bit7 of a bitmap byte will operate on pix0, bit6 of the bitmap
//! will operate on pix1, etc.  However, this behavior can be affected with the
//! AM_UTIL_DRAW_ROPS_MIRROR option.
//!
//! @return true if successfully, false otherwise.
//
//*****************************************************************************
extern bool am_util_draw_2d1bpp_bitmap_draw(
                            am_util_draw_2d1bpp_context_t *psDrawContext,
                            const uint8_t *pui8Bitmap,
                            uint32_t ui32Width, uint32_t ui32Height,
                            uint32_t ui32DstX,  uint32_t ui32DstY,
                            uint32_t ui32DrawingOps);

//*****************************************************************************
//
//! @brief Select a font for use in subsequent character drawing.
//!
//! @param psDrawContext - Pointer of the Drawing Context
//! @param ui32FontSel - Font Selection
//!
//! @return None.
//
//*****************************************************************************
extern bool am_util_draw_2d1bpp_font_select(
                            am_util_draw_2d1bpp_context_t *psDrawContext,
                            uint32_t ui32FontSel);

//*****************************************************************************
//
//! @brief Return the currently selected font dimensions.
//!
//! @param psDrawContext - Pointer of the Drawing Context
//!
//! @return High 16 bits - font height, low 16 bits = font width.
//
//*****************************************************************************
extern uint32_t am_util_draw_2d1bpp_fontsize_get(
                            am_util_draw_2d1bpp_context_t *psDrawContext);

//*****************************************************************************
//
//! @brief Draw a character at the given coordinates
//!        into the display framebuffer.
//!
//! @param psDrawContext - Pointer of the Drawing Context
//! @param ui16Char - character to print.
//! @param ui32X - Coordinates of the desired pixel.
//! @param ui32Y - Coordinates of the desired pixel.
//! @param ui32Color - Color to write to the given pixel.
//!
//! @return None.
//
//*****************************************************************************
extern bool am_util_draw_2d1bpp_char_draw(
                            am_util_draw_2d1bpp_context_t *psDrawContext,
                            uint8_t ui16Char, uint32_t ui32X, uint32_t ui32Y,
                            uint32_t ui32Color);

//*****************************************************************************
//
//! @brief Draw a string of characters beginning at the given coordinates.
//!
//! Each character of the string is printed and advanced to the next character
//! position.  If the length of the string exceeds the width of the screen, the
//! printing of the string is continued on the next line.  However the
//! characters are simply printed contiguously, that is when a character cannot
//! be printed on the current line, it is printed at the beginning of the next
//! line - thus words can be broken midword and not necessarily at a syllable.
//!
//! @param psDrawContext - Pointer of the Drawing Context
//! @param pui8Char - Pointer to the null-terminated string to be printed.
//! @param ui32X - Coordinates of the desired pixel.
//! @param ui32Y - Coordinates of the desired pixel.
//! @param ui32DrawingOps - AM_UTIL_DRAW_ROPS_COPY or AM_UTIL_DRAW_ROPS_XOR.
//! @param ui32Color - Color of the string of characters.
//!
//! @return true if successful.
//
//*****************************************************************************
extern bool am_util_draw_2d1bpp_string_draw(
                            am_util_draw_2d1bpp_context_t *psDrawContext,
                            uint8_t *pui8Char, uint32_t ui32X, uint32_t ui32Y,
                            uint32_t ui32DrawingOps,
                            uint32_t ui32Color );

//*****************************************************************************
//
//! @brief Draw a rectangle into the display framebuffer.
//!
//! @param psDrawContext - Pointer of the Drawing Context
//! @param ui32DstX - The screen location to place the bitmap.
//! @param ui32DstY - The screen location to place the bitmap.
//! @param ui32Width - Width of the rectangle, 1-96.
//! @param ui32Height - Height of the rectangle, 1-96.
//! @param ui32DrawingOps - AM_UTIL_DRAW_ROPS_COPY or AM_UTIL_DRAW_ROPS_XOR.
//! @param ui32Color - Color of the rectangle.
//! @param bFill - If true, fill the rectangle.
//!
//! @return true if successful, false otherwise.
//
//*****************************************************************************
extern bool am_util_draw_2d1bpp_rectangle_draw(
                            am_util_draw_2d1bpp_context_t *psDrawContext,
                            uint32_t ui32DstX,  uint32_t ui32DstY,
                            uint32_t ui32Width, uint32_t ui32Height,
                            uint32_t ui32DrawingOps,
                            uint32_t ui32Color, bool bFill );

//*****************************************************************************
//
//! @brief Draw a line into the display framebuffer.
//!
//! This function is a run-length slice implementation of the Bresenham line
//!  drawing algorithm.  The run-length slice algorithm basically steps along
//!  the minor axis (as opposed to the standard algorithm, which steps along
//!  the major axis), thereby minimizing the error-term calculations.
//! While extremely efficient, this method requires separate first and last
//!  runs in order to balance the line as the standard Bresenham algorithm does.
//! Some special cases (horizontal, vertical, and diagonal lines) are also
//!  handled for optimzation purposes (not to mention to avoid those pesky
//!  divide by zero situations and other boundary conditions).
//!
//! @param psDrawContext - Pointer of the Drawing Context
//! @param ui32X1 - One endpoint of the line.
//! @param ui32Y1 - One endpoint of the line.
//! @param ui32X2 - The other endpoint of the line.
//! @param ui32Y2 - The other endpoint of the line.
//! @param ui32LineWidth - width of the line.
//! @param ui32DrawingOps - AM_UTIL_DRAW_ROPS_COPY or AM_UTIL_DRAW_ROPS_XOR.
//! @param ui32Color  - Color of the line.
//!
//! @return true if successful, false otherwise.
//
//*****************************************************************************
extern bool am_util_draw_2d1bpp_line_draw(
                            am_util_draw_2d1bpp_context_t *psDrawContext,
                            uint32_t ui32X1,  uint32_t ui32Y1,
                            uint32_t ui32X2,  uint32_t ui32Y2,
                            uint32_t ui32LineWidth,
                            uint32_t ui32DrawingOps, uint32_t ui32Color );

//*****************************************************************************
//
//! @brief Draw an ellipse into the display framebuffer.
//!
//! This function draws an ellipse of given size and shape.
//! A circle is drawn if ui32Xrad = ui32Yrad.
//!
//! Note that this is a relatively simple ellipse algorithm and is not
//! particularly optimized.
//!
//! @param psDrawContext - Pointer of the Drawing Context
//! @param ui32X - Coordinates of the desired pixel.
//! @param ui32Y - Coordinates of the desired pixel.
//! @param ui32Xrad - Radius of the X axis.
//! @param ui32Yrad - Radius of the Y axis.
//! @param ui32DrawingOps - AM_UTIL_DRAW_ROPS_COPY or AM_UTIL_DRAW_ROPS_XOR.
//! @param ui32Color    - Color of the line.
//! @param bFill    - Filled if true, otherwise an outline.
//!
//! @return true if successful, false otherwise.
//
//*****************************************************************************
extern bool am_util_draw_2d1bpp_ellipse_draw(
                            am_util_draw_2d1bpp_context_t *psDrawContext,
                            uint32_t ui32X,    uint32_t ui32Y,
                            uint32_t ui32Xrad, uint32_t ui32Yrad,
                            uint32_t ui32DrawingOps,
                            uint32_t ui32Color, bool bFill );

#ifdef __cplusplus
}
#endif

#endif // AM_UTIL_DRAW_2D1BPP_H

//*****************************************************************************
//
// End Doxygen group.
//! @}
//
//*****************************************************************************

