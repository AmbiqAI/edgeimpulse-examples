//*****************************************************************************
//
//! @file am_util_draw_2d1bpp.c
//!
//  High level 1bpp drawing routines for Ambiq Micro products.
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

//*****************************************************************************
//
// C99
//
//*****************************************************************************
#include <stdint.h>
#include <stdbool.h>

#include "am_util.h"
#include "am_devices_display.h"

//*****************************************************************************
//
// Global variables.
//
//*****************************************************************************
const uint8_t g_BMMask[8]  = { 0x80, 0x40, 0x20, 0x10, 0x08, 0x04, 0x02, 0x01 };
const uint8_t g_SetMask[8] = { 0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80 };
const uint8_t g_ClrMask[8] = { 0xFE, 0xFD, 0xFB, 0xF7, 0xEF, 0xDF, 0xBF, 0x7F };

//*****************************************************************************
//
//! @brief Static support function.
//!
//! modify_pixel_in_a_line() will modify a pixel in a given line of the
//! framebuffer.
//!
//! It was originally implemented as a function to replace individual macros
//! that wrote or XORed pixels. Implement as a macro.
//! Better performance, larger code size.
//!
//! @param pFrBufLine      Pointer to the beginning of the line of the FB.
//! @param ui32X           The X coordinate of the pixel.
//! @param ui32DrawingOps  A mask of the raster operations.
//! @param ui32DrawingOps  Drawing options/operations include:
//!                      AM_UTIL_DRAW_ROPS_COPY   - Set the pixel to ui32Color.
//!                      AM_UTIL_DRAW_ROPS_XOR    - XOR the pixel.
//!                      AM_UTIL_DRAW_ROPS_OR     - OR the pixel.
//! @param ui32Color       The color to be used in setting the pixel.
//
//*****************************************************************************
#define modify_pixel_in_a_line(pFrBufLine, ui32X, ui32DrawingOps,               \
                               ui32Color, ui32DrawPolarity)                     \
{                                                                               \
    uint8_t ui8FB, ui8SetMask;                                                  \
                                                                                \
    /*                                                                          \
    // Set current framebuffer byte and setmask.                                \
    */                                                                          \
    ui8FB = pFrBufLine[(ui32X / 8)];                                            \
    ui8SetMask = g_SetMask[ui32X & 7];                                          \
                                                                                \
    if ( !(ui32DrawingOps & (AM_UTIL_DRAW_ROPS_XOR | AM_UTIL_DRAW_ROPS_OR)) )   \
    {                                                                           \
        /*                                                                      \
        // Set the pixel according to the provided color.                       \
        */                                                                      \
        ui8FB &= g_ClrMask[ui32X & 7];                                          \
        ui8FB |= ui32Color & 0x01 ? ui8SetMask : 0x00;                          \
    }                                                                           \
    else if ( ui32DrawingOps & AM_UTIL_DRAW_ROPS_XOR )                          \
    {                                                                           \
        /*                                                                      \
        // XOR this pixel.                                                      \
        */                                                                      \
        if ( !ui32DrawPolarity )                                                \
        {                                                                       \
            ui8FB ^= ui8SetMask;                                                \
        }                                                                       \
    }                                                                           \
    else                                                                        \
    {                                                                           \
        /*                                                                      \
        // OR this pixel                                                        \
        */                                                                      \
        ui8FB |= ui8SetMask;                                                    \
    }                                                                           \
                                                                                \
    /*                                                                          \
    // Determine the required polarity.                                         \
    */                                                                          \
    if ( ui32DrawPolarity )                                                     \
    {                                                                           \
        ui8FB ^= ui8SetMask;                                                    \
    }                                                                           \
                                                                                \
    /*                                                                          \
    // Write the pixel back to the framebuffer                                  \
    */                                                                          \
    pFrBufLine[(ui32X / 8)] = ui8FB;                                            \
                                                                                \
} // modify_pixel_in_a_line()

//*****************************************************************************
//
// Static support functions.
//
// DrawHorzRun() and DrawVertRun() are support functions for line drawing.
// They draw a horizontal or vertical run, respectively, of pixels in the
// line draw algorithm.  DrawHorzRun is used for X-major lines, and similarly
// DrawVertRun is used for Y-major lines.
//
//*****************************************************************************
static void
DrawHorzRun(uint8_t *pFrBufLine, uint32_t ui32X1,
            uint32_t ui32RunLen, int32_t i32Xadvance,
            uint32_t ui32DrawingOps, uint32_t ui32Color,
            uint32_t ui32DrawPolarity)
{
    int32_t i32X;

    for ( i32X = 0; i32X < ui32RunLen; i32X++ )
    {
        modify_pixel_in_a_line(pFrBufLine,  ui32X1, ui32DrawingOps, ui32Color, ui32DrawPolarity);
        ui32X1 += i32Xadvance;
    }
} // DrawHorzRun()

static void
DrawVertRun(uint8_t *pFrBufLine, uint32_t ui32RunLen,
            uint32_t ui32X1, uint32_t ui32DisplayWidthBytes,
            uint32_t ui32DrawingOps, uint32_t ui32Color,
            uint32_t ui32DrawPolarity)
{
    uint32_t ui32Y;

    for ( ui32Y = 0; ui32Y < ui32RunLen; ui32Y++ )
    {
        modify_pixel_in_a_line((pFrBufLine + (ui32Y * ui32DisplayWidthBytes)),
                               ui32X1, ui32DrawingOps, ui32Color, ui32DrawPolarity);
    }
} // DrawVertRun()

//*****************************************************************************
//
// Static support function.
//
// DrawOvalLine() is a support function for am_util_draw_2d1bpp_ellipse_draw()
// which used for drawing the lines of an ellipse, whether filled or not.
//
//*****************************************************************************
static void
DrawOvalLine(uint8_t *pui8Line, uint32_t ui32X1, uint32_t ui32R,
             uint32_t ui32DrawingOps, uint32_t ui32Color,
             uint32_t bFill, uint32_t ui32DrawPolarity)
{
    uint32_t ui32X, ui32Xincr;

    ui32Xincr = bFill ? 1 : ui32R * 2;
    for ( ui32X = (ui32X1 - ui32R); ui32X <= (ui32X1 + ui32R); ui32X += ui32Xincr)
    {
        modify_pixel_in_a_line(pui8Line,  ui32X, ui32DrawingOps, ui32Color, ui32DrawPolarity);
    }
}

//*****************************************************************************
//
//  Return the value of a given pixel.
//!
//! @param ui32X,ui32Y = Coordinates of the desired pixel.
//!
//! @return Value of the requested device independent pixel.  Therefore
//! a 1 is returned if a pixel is on, 0 is returned if the pixel is off.
//
//*****************************************************************************
uint32_t
am_util_draw_2d1bpp_pixel_get(am_util_draw_2d1bpp_context_t *psDrawContext,
                              uint32_t ui32X, uint32_t ui32Y)
{
    uint32_t ui32FrBufOffset, ui32Shift, ui32Ret;

    ui32FrBufOffset = (ui32Y * (psDrawContext->ui32DrawWidth)) + ui32X / 8;
    ui32Shift = ui32X & 0x7;

    ui32Ret = (psDrawContext->pui8Framebuffer[ui32FrBufOffset] >> ui32Shift) & 0x01;

    if ( psDrawContext->ui32DrawPolarity )
    {
        //
        // Background polarity is 1 to clear, 0 to set.
        // Invert the return value.
        //
        ui32Ret ^= 0x1;
    }

    return ui32Ret;
}

//*****************************************************************************
//
//  Set a given pixel value at the given coordinates
//
//*****************************************************************************
void
am_util_draw_2d1bpp_pixel_set(am_util_draw_2d1bpp_context_t *psDrawContext,
                              uint32_t ui32X, uint32_t ui32Y,
                              uint32_t ui32Color)
{
    uint32_t ui32Shift;
    uint8_t ui8Mask;

    //
    // Update the pixel
    //
    ui32Color &= 0x01;

    uint32_t ui32FrBufOffset;
    ui32FrBufOffset = (ui32Y * (psDrawContext->ui32DrawWidth / 8)) + ui32X / 8;

    ui32Shift = (ui32X & 0x7);
    ui8Mask = 1 << ui32Shift;

    psDrawContext->pui8Framebuffer[ui32FrBufOffset] &= ~ui8Mask;

    if ( psDrawContext->ui32DrawPolarity )
    {
        //
        // Background polarity is 1 to clear, 0 to set.
        //
        ui8Mask = ui32Color ? 0x00 : ui8Mask;
    }
    else
    {
        //
        // Background polarity is 0 to clear, 1 to set.
        //
        ui8Mask = ui32Color ? ui8Mask : 0x00;
    }

    psDrawContext->pui8Framebuffer[ui32FrBufOffset] |= ui8Mask;

} // am_util_draw_2d1bpp_pixel_set()

//*****************************************************************************
//
//  Write a bitmap into the display framebuffer.
//
//*****************************************************************************
bool
am_util_draw_2d1bpp_bitmap_draw(am_util_draw_2d1bpp_context_t *psDrawContext,
                                const uint8_t *pui8Bitmap,
                                uint32_t ui32Width, uint32_t ui32Height,
                                uint32_t ui32DstX,  uint32_t ui32DstY,
                                uint32_t ui32DrawingOps)
{
    uint8_t  ui8X, ui8Y, ui8FBbyte, ui8FBmask, ui8FBvalue;
    uint32_t ui32BMpixnum, ui32BMbitnum;
    uint32_t ui32FrBufOffset, ui32MirrorInc, ui32Bitnum_pxloff;
    bool     bInvert, bXOR, bBMbit, bOR;

    //
    // The bitmap must fit on the display
    //
    if ( ((ui32DstX + ui32Width)  > psDrawContext->ui32DrawWidth)    ||
         ((ui32DstY + ui32Height) > psDrawContext->ui32DrawHeight) )
    {
        return false;
    }

    if ( (ui32Width == 0)  ||  (ui32Height == 0) )
    {
        return true;
    }

    //
    // Determine background polarity of the display device
    //
    if ( psDrawContext->ui32DrawPolarity )
    {
        //
        // This display clears the screen with 0xFFs.
        // So to invert the image, a 1 bit in the bitmap should stay a 1 on
        // the display.
        //
        bInvert  = ui32DrawingOps & AM_UTIL_DRAW_ROPS_INVERT ? true : false;
    }
    else
    {
        //
        // This display clears the screen with 0x00s.
        // So to invert the image, a 1 bit in the bitmap should become a 0 on
        // the display.
        //
        bInvert  = ui32DrawingOps & AM_UTIL_DRAW_ROPS_INVERT ? false : true;
    }

    bXOR     = ui32DrawingOps & AM_UTIL_DRAW_ROPS_XOR    ? true : false;
    bOR      = ui32DrawingOps & AM_UTIL_DRAW_ROPS_OR     ? true : false;

    if ( bXOR  &&  bOR )
    {
        return false;
    }

    //
    // Initialize increment variables, which are used in the loop depending on
    // whether mirroring of the bitmap is required or not.
    //
    ui32MirrorInc     = ui32DrawingOps & AM_UTIL_DRAW_ROPS_MIRROR ? -1 : 1;
    ui32Bitnum_pxloff = ui32DrawingOps & AM_UTIL_DRAW_ROPS_MIRROR ?
                            ui32Width - 1 : 0;

    //
    // Display the bitmap line-by-line.
    //
    ui32BMpixnum = 0;
    for ( ui8Y = ui32DstY; ui8Y < ui32DstY + ui32Height; ui8Y++ )
    {
        ui32BMbitnum = ui32BMpixnum + ui32Bitnum_pxloff;

        //
        // Copy each bitmap line to the framebuffer.
        // This algorithm can handle any bmWidth.
        //
        for ( ui8X = ui32DstX; ui8X < ui32DstX + ui32Width; ui8X++ )
        {
            //
            // Get the appropriate byte out of the framebuffer and determine
            // a mask for the pixel.
            //
            ui32FrBufOffset = (ui8Y * (psDrawContext->ui32DrawWidth / 8)) + (ui8X / 8);
            ui8FBbyte = psDrawContext->pui8Framebuffer[ui32FrBufOffset];
            ui8FBmask = g_SetMask[ui8X & 7];

            bBMbit = (pui8Bitmap[ui32BMbitnum / 8] & g_BMMask[ui32BMbitnum & 7]) ? true : false;

            //
            // Get the pixel from the bitmap and set the FB pixel appropriately.
            // If bBMbit is 1 and noninvert, set a 0 in the FB to light the
            //  display pixel.  If inverting, set a 1 in the FB.
            //
            if ( bInvert )
            {

                ui8FBvalue = bBMbit ? ui8FBmask : 0;
            }
            else
            {
                ui8FBvalue = bBMbit ? 0 : ui8FBmask;
            }

            if ( bXOR )
            {
                ui8FBbyte ^= ui8FBvalue;
            }
            else if ( bOR )
            {
                ui8FBbyte |= ui8FBvalue;
            }
            else
            {
                //
                // COPY
                //
                ui8FBbyte &= ~ui8FBmask;
                ui8FBbyte |= ui8FBvalue;
            }

            psDrawContext->pui8Framebuffer[ui32FrBufOffset] = ui8FBbyte;

            ui32BMpixnum++;
            ui32BMbitnum += ui32MirrorInc;
        }
    }

    return true;

} // am_util_draw_2d1bpp_bitmap_draw()

//*****************************************************************************
//
//  Select a font for use in subsequent character drawing.
//
//*****************************************************************************
bool
am_util_draw_2d1bpp_font_select(am_util_draw_2d1bpp_context_t *psDrawContext,
                                uint32_t ui32FontSel)
{
    if ( ( ui32FontSel < AM_FONT_SELECT_VALID_MIN )                 ||
         ( ui32FontSel > AM_FONT_SELECT_VALID_MAX )                 ||
         ( g_sFontSelectorTbl[ui32FontSel].ui32ID != ui32FontSel ) )
    {
        psDrawContext->ui32FontSel = AM_FONT_SELECT_AMBIQ_NONE;
        return false;
    }

    psDrawContext->ui32FontSel = ui32FontSel;

    return true;

} // am_util_draw_2d1bpp_font_select()

//*****************************************************************************
//
//  Return the currently selected font dimensions.
//
//*****************************************************************************
uint32_t
am_util_draw_2d1bpp_fontsize_get(am_util_draw_2d1bpp_context_t *psDrawContext)
{
    uint32_t ui32FontSel = psDrawContext->ui32FontSel;

    if ( ( ui32FontSel < AM_FONT_SELECT_VALID_MIN )                 ||
         ( ui32FontSel > AM_FONT_SELECT_VALID_MAX )                 ||
         ( g_sFontSelectorTbl[ui32FontSel].ui32ID != ui32FontSel ) )
    {
        return 0x00000000;
    }

    return (g_sFontSelectorTbl[ui32FontSel].ui32FontHeight << 16) |
           (g_sFontSelectorTbl[ui32FontSel].ui32FontWidth  << 0);
}

//*****************************************************************************
//
//  Draw a character at the given coordinates
//
//*****************************************************************************
bool
am_util_draw_2d1bpp_char_draw(am_util_draw_2d1bpp_context_t *psDrawContext,
                              uint8_t ui16Char, uint32_t ui32X, uint32_t ui32Y,
                              uint32_t ui32Color)
{
    uint32_t ui32Attr, ui32Width, ui32Height;
    uint32_t ui32FontSel;
    uint32_t ui32DrawingOps = ui32Color                 ?
                              AM_UTIL_DRAW_ROPS_COPY    :
                              AM_UTIL_DRAW_ROPS_INVERT;

    //
    // Validate the font selector.
    //
    ui32FontSel = psDrawContext->ui32FontSel;
    if ( ( ui32FontSel < AM_FONT_SELECT_VALID_MIN )                 ||
         ( ui32FontSel > AM_FONT_SELECT_VALID_MAX ) )
    {
        return false;
    }

    //
    // Validate table entries
    //
    ui32Attr    = g_sFontSelectorTbl[ui32FontSel].ui32Attr;
    ui32Width   = g_sFontSelectorTbl[ui32FontSel].ui32FontWidth;
    ui32Height  = g_sFontSelectorTbl[ui32FontSel].ui32FontHeight;

    if ( ( g_sFontSelectorTbl[ui32FontSel].ui32ID != ui32FontSel )  ||
         ( ui16Char < g_sFontSelectorTbl[ui32FontSel].ui16RangeLo ) ||
         ( ui16Char > g_sFontSelectorTbl[ui32FontSel].ui16RangeHi ) )

    {
        return false;
    }

    //
    // One more (temporary) validation.
    //
    if ( ui32Attr != AM_FONT_ATTR_ASCII)
    {
        //
        // Temporary.  We will eventually support unicode.
        //
        return false;
    }

    //
    // Convert the character into an index value.
    //
    ui16Char -= g_sFontSelectorTbl[ui32FontSel].ui16RangeLo;

    //
    // By this point we've validated all of the parameters and converted
    // ui16Char into an index.
    // If the font bitmap can be turned into a pointer, this switch statement
    // can probably go away!
    //
    switch ( ui32FontSel )
    {
        case AM_FONT_SELECT_AMBIQSIMPLE_5x8:
            am_util_draw_2d1bpp_bitmap_draw(
                psDrawContext,
                (uint8_t*)am_util_font5x8[ui16Char],
                ui32Width, ui32Height, ui32X, ui32Y,  ui32DrawingOps);
            break;
        case AM_FONT_SELECT_PIXELDREAMS_11x28:
            am_util_draw_2d1bpp_bitmap_draw(
                psDrawContext,
                (uint8_t*)am_util_font11x28[ui16Char],
                ui32Width, ui32Height, ui32X, ui32Y,  ui32DrawingOps);
            break;
        case AM_FONT_SELECT_PIXELDREAMS_09x20:
            am_util_draw_2d1bpp_bitmap_draw(psDrawContext,
                (uint8_t*)am_util_font9x20[ui16Char],
                ui32Width, ui32Height, ui32X, ui32Y,  ui32DrawingOps);
            break;
        case AM_FONT_SELECT_PIXELDREAMS_08x17:
            am_util_draw_2d1bpp_bitmap_draw(psDrawContext,
                (uint8_t*)am_util_font8x17[ui16Char],
                ui32Width, ui32Height, ui32X, ui32Y,  ui32DrawingOps);
            break;
    }

    return true;

} // am_util_draw_2d1bpp_char_draw()

//*****************************************************************************
//
//  Draw a string of characters beginning at the given coordinates.
//
//*****************************************************************************
bool
am_util_draw_2d1bpp_string_draw(am_util_draw_2d1bpp_context_t *psDrawContext,
                                uint8_t *pui8Char, uint32_t ui32X, uint32_t ui32Y,
                                uint32_t ui32DrawingOps, uint32_t ui32Color)
{
    uint32_t ui32SpacingW, ui32SpacingH, ui32Width, ui32Height;
    uint32_t ui32X0, ui32DrawWidth, ui32FontSel;
    uint16_t ui16RangeLo, ui16RangeHi, ui16CharOffset;
    bool bLower;
    uint8_t ui8Char;

    ui32DrawingOps |= ui32Color                 ?
                      AM_UTIL_DRAW_ROPS_COPY    :
                      AM_UTIL_DRAW_ROPS_INVERT;

    //
    // Validate the font selector.
    //
    ui32FontSel = psDrawContext->ui32FontSel;
    if ( ( ui32FontSel < AM_FONT_SELECT_VALID_MIN )                 ||
         ( ui32FontSel > AM_FONT_SELECT_VALID_MAX ) )
    {
        return false;
    }

    //
    // Get table entries to locals for ease-of-handling
    //
    ui32Width      = g_sFontSelectorTbl[ui32FontSel].ui32FontWidth;
    ui32Height     = g_sFontSelectorTbl[ui32FontSel].ui32FontHeight;
    ui32SpacingW   = g_sFontSelectorTbl[ui32FontSel].ui32FontSpacingW;
    ui32SpacingH   = g_sFontSelectorTbl[ui32FontSel].ui32FontSpacingH;
    ui16RangeLo    = g_sFontSelectorTbl[ui32FontSel].ui16RangeLo;
    ui16RangeHi    = g_sFontSelectorTbl[ui32FontSel].ui16RangeHi;
    ui16CharOffset = ui16RangeLo;

    ui32X0 = ui32X;
    ui32DrawWidth = psDrawContext->ui32DrawWidth;

    while ( *pui8Char )
    {
        ui8Char = *pui8Char;

        if ( (ui8Char < ui16RangeLo)  ||  (ui8Char > ui16RangeHi) )
        {
            pui8Char++;
            ui32X += ui32SpacingW;
            continue;
        }

        bLower = ( ui8Char >= 'a' && ui8Char <= 'z' ) ? true : false;

        //
        // Everything looks good.  So now convert ui8Char into an index
        // into the font bitmap table.
        //
        ui8Char -= ui16CharOffset;

        switch ( ui32FontSel )
        {
            case AM_FONT_SELECT_AMBIQSIMPLE_5x8:
                am_util_draw_2d1bpp_bitmap_draw(
                        psDrawContext,
                        (uint8_t*)am_util_font5x8[ui8Char],
                        ui32Width, ui32Height, ui32X, ui32Y,  ui32DrawingOps);
                break;
            case AM_FONT_SELECT_PIXELDREAMS_11x28:
                am_util_draw_2d1bpp_bitmap_draw(
                        psDrawContext,
                        (uint8_t*)am_util_font11x28[ui8Char],
                        ui32Width, ui32Height, ui32X, ui32Y,  ui32DrawingOps);
                break;
            case AM_FONT_SELECT_PIXELDREAMS_09x20:
                //
                // We need to special case lower-case letters such that we end
                // up with properly descended letters.
                //
                if ( bLower )
                {
                    ui32Y += 3;
                }

                am_util_draw_2d1bpp_bitmap_draw(
                        psDrawContext,
                        (uint8_t*)am_util_font9x20[ui8Char],
                        ui32Width, ui32Height, ui32X, ui32Y,  ui32DrawingOps);

                if ( bLower )
                {
                    ui32Y -= 3;
                }
                break;
            case AM_FONT_SELECT_PIXELDREAMS_08x17:
                am_util_draw_2d1bpp_bitmap_draw(
                        psDrawContext,
                        (uint8_t*)am_util_font8x17[ui8Char],
                        ui32Width, ui32Height, ui32X, ui32Y,  ui32DrawingOps);
                break;
        }

        pui8Char++;
        ui32X += ui32SpacingW;

        if ( (ui32X + ui32Width) >= ui32DrawWidth )
        {
            ui32X = ui32X0;
            ui32Y += ui32SpacingH;
        }
    }

    return true;

} // am_util_draw_2d1bpp_string_draw()

//*****************************************************************************
//
//  Draw a rectangle into the display framebuffer.
//
//*****************************************************************************
bool
am_util_draw_2d1bpp_rectangle_draw(am_util_draw_2d1bpp_context_t *psDrawContext,
                                   uint32_t ui32DstX,  uint32_t ui32DstY,
                                   uint32_t ui32Width, uint32_t ui32Height,
                                   uint32_t ui32DrawingOps,
                                   uint32_t ui32Color, bool bFill )
{
    uint32_t ui32X, ui32Y, ui32FrBufOffset, ui32XIncr, ui32DrawPolarity;
    uint8_t *pui8FrmBuf;

    //
    // Check for invalid cases including any that would draw off the screen,
    // or a 1 pixel (width or height) unfilled rectangle.
    //
    if ( ((ui32DstX + ui32Width)  > psDrawContext->ui32DrawWidth)     ||
         ((ui32DstY + ui32Height) > psDrawContext->ui32DrawHeight)    ||
         ( ((ui32Height == 1) || (ui32Width == 1)) && !bFill ) )
    {
        return false;
    }

    if ( (ui32Width == 0)  ||  (ui32Height == 0) )
    {
        return true;
    }

    ui32DrawPolarity = psDrawContext->ui32DrawPolarity;

    pui8FrmBuf = psDrawContext->pui8Framebuffer;
    for ( ui32Y = ui32DstY; ui32Y < ui32DstY + ui32Height; ui32Y++ )
    {
        //
        // Update each line of the rectangle in the framebuffer.
        //
        ui32XIncr = 1;
        if ( !bFill )
        {
            //
            // Don't fill this rectangle, make an outline instead.
            // The first and last lines need a complete line.
            // Intermediate lines only need first and last pixels.
            //
            if ( (ui32Y != ui32DstY) && (ui32Y != (ui32DstY + (ui32Height-1))) )
            {
                ui32XIncr = ui32Width - 1;
            }
        }

        ui32FrBufOffset = ui32Y * (psDrawContext->ui32DrawWidth / 8);
        for ( ui32X = ui32DstX; ui32X < ui32DstX + ui32Width; ui32X += ui32XIncr )
        {
            //
            // Set or clear the pixel.
            // For this display, a 1 in the framebuffer is a cleared pixel on
            //  the screen.  Likewise, clearing the bit in the framebuffer will
            //  set the pixel on the screen.
            //
            modify_pixel_in_a_line((pui8FrmBuf + ui32FrBufOffset), ui32X,
                                   ui32DrawingOps, ui32Color,
                                   ui32DrawPolarity);
        }
    }

    return true;

} // am_util_draw_2d1bpp_rectangle_draw

//*****************************************************************************
//
// Draw a line into the display framebuffer.
//
//*****************************************************************************
bool
am_util_draw_2d1bpp_line_draw(am_util_draw_2d1bpp_context_t *psDrawContext,
                              uint32_t ui32X1,  uint32_t ui32Y1,
                              uint32_t ui32X2,  uint32_t ui32Y2,
                              uint32_t ui32LineWidth,
                              uint32_t ui32DrawingOps, uint32_t ui32Color )
{
    uint32_t ui32FrBufOffset;
    int32_t i32Xadvance, i32ErrorTerm, i32WholeStep, ix;
    int32_t i32Xdelta, i32Ydelta, i32AdjUp, i32AdjDn;
    uint32_t ui32X, ui32Y, ui32InitialPixelCount, ui32FinalPixelCount, ui32RunLen;
    uint32_t ui32tmp, ui32DrawPolarity;
    uint32_t ui32DisplayWidthBytes = psDrawContext->ui32DrawWidth / 8;
    uint8_t *pui8FrmBuf;

    pui8FrmBuf = psDrawContext->pui8Framebuffer;

    //
    // Validate coordinates and other inputs.
    // Line width is a future feature - if not 1, fail for now.
    //
    if ( ( ui32X2 >= psDrawContext->ui32DrawWidth )     ||
         ( ui32Y2 >= psDrawContext->ui32DrawHeight )    ||
         ( ui32LineWidth != 1 ) )
    {
        return false;
    }

    //
    // Begin by making sure we're always drawing the line top to bottom.
    // This will eliminate having to handle multiple octants and it will always
    //  draw the same line for the same given endpoints.
    //
    if ( ui32Y1 > ui32Y2 )
    {
        ui32tmp = ui32Y1;
        ui32Y1 = ui32Y2;
        ui32Y2 = ui32tmp;
        ui32tmp = ui32X1;
        ui32X1 = ui32X2;
        ui32X2 = ui32tmp;
    }

    ui32DrawPolarity = psDrawContext->ui32DrawPolarity;

    //
    // Determine X direction, we want left to right.
    //
    i32Xdelta = ui32X2 - ui32X1;
    if ( i32Xdelta < 0 )
    {
        i32Xadvance = -1;
        i32Xdelta = -i32Xdelta;
    }
    else
    {
        i32Xadvance = 1;
    }

    //
    // Determine Y distance (we already know we're going top to bottom).
    //
    i32Ydelta = ui32Y2 - ui32Y1;

    //
    // Handle the special cases of horizontal, vertical, and diagonal lines.
    //
    if ( i32Xdelta == 0 )
    {
        //
        // It's a vertical line.
        //
        for ( ui32Y = ui32Y1; ui32Y <= ui32Y2; ui32Y++ )
        {
            ui32FrBufOffset = ui32Y * ui32DisplayWidthBytes;
            modify_pixel_in_a_line((pui8FrmBuf + ui32FrBufOffset), ui32X1,
                                   ui32DrawingOps, ui32Color,
                                   ui32DrawPolarity);
        }
        return true;
    }

    if ( i32Ydelta == 0 )
    {
        //
        // It's a horizontal line.
        //
        ui32FrBufOffset = ui32Y1 * ui32DisplayWidthBytes;
        i32Xdelta = ui32X2 - ui32X1;
        if ( i32Xdelta < 0 )
        {
            ui32tmp = ui32X1;
            ui32X1 = ui32X2;
            ui32X2 = ui32tmp;
        }

        for ( ui32X = ui32X1; ui32X <= ui32X2; ui32X++ )
        {
#if 1
            modify_pixel_in_a_line((pui8FrmBuf + ui32FrBufOffset), ui32X,
                                   ui32DrawingOps, ui32Color,
                                   ui32DrawPolarity );
#else
            if ( bXOR )
            {
                XOR_PIXEL_IN_LINE((pui8FrmBuf + ui32FrBufOffset), ui32X, ui32Color);
            }
            else
            {
                WRITE_PIXEL_IN_LINE((pui8FrmBuf + ui32FrBufOffset), ui32X, ui32Color);
            }
#endif
        }
        return true;
    }

    if ( i32Xdelta == i32Ydelta )
    {
        //
        // It's a diagonal line.
        //
        ui32X = ui32X1;
        for ( ui32Y = ui32Y1; ui32Y <= ui32Y2; ui32Y++ )
        {
            ui32FrBufOffset = ui32Y * ui32DisplayWidthBytes;
            modify_pixel_in_a_line((pui8FrmBuf + ui32FrBufOffset), ui32X,
                                   ui32DrawingOps, ui32Color,
                                   ui32DrawPolarity );
            ui32X += i32Xadvance;
        }
        return true;
    }

    //
    // The special (easy) cases are done.
    // Determine if the line is X or Y major
    //
    if ( i32Xdelta >= i32Ydelta )
    {
        //
        // X major line
        // Compute the minimum # of pixels in a run in this line.
        //
        i32WholeStep = i32Xdelta / i32Ydelta;

        //
        // Error term adjust each time Y steps by 1, used to tell when one extra
        // pixel should be drawn as part of a run to account for fractional steps
        // along the X axis per 1-pixel steps along Y.
        //
        i32AdjUp = (i32Xdelta % i32Ydelta) * 2;

        //
        // Error term adjust when the error term turns over, used to factor out the
        // X step made at that time.
        //
        i32AdjDn = i32Ydelta * 2;

        //
        // Initial error term - reflects an initial step of 1/2 along the Y axis.
        //
        i32ErrorTerm = (i32Xdelta % i32Ydelta) - (i32Ydelta * 2);

        //
        // The initial and last runs are partial because Y advances only 1/2 for
        // these runs rather than 1.  Divide one full run plus the initial pixel
        // between the initial and last runs.
        //
        ui32InitialPixelCount = (i32WholeStep / 2) + 1;
        ui32FinalPixelCount   = ui32InitialPixelCount;

        //
        // If the run length is even and there's no fractional advance, we have a
        // pixel that could go to either the initial or last partial run, which
        // we'll arbitrarily allocate to the last run.
        //
        if ( (i32AdjUp == 0) && ( (i32WholeStep & 0x01) == 0) )
        {
            ui32InitialPixelCount--;
        }

        //
        // If there are an odd number of pixels per run, we have 1 pixel that
        // can't be allocated to either the initial or last partial run, so
        // we'll add 1/2 to error term so this pixel will be handled by the
        // normal full-run loop.
        //
        if ( i32WholeStep & 0x01 )
        {
            i32ErrorTerm += i32Ydelta;
        }

        //
        // Draw the first, partial run of pixels.
        //
        ui32FrBufOffset = ui32Y1 * ui32DisplayWidthBytes;
        ui32X = ui32X1;

        DrawHorzRun(pui8FrmBuf + ui32FrBufOffset, ui32X, ui32InitialPixelCount,
                    i32Xadvance, ui32DrawingOps, ui32Color, ui32DrawPolarity);
        ui32X = ( i32Xadvance < 0 ) ?
                    ui32X - ui32InitialPixelCount :
                    ui32X + ui32InitialPixelCount;

        //
        // Draw all full runs.
        //
        for ( ui32Y = ui32Y1 + 1; ui32Y < ui32Y1 + (i32Ydelta - 1); ui32Y++ )
        {
            ui32RunLen = i32WholeStep;

            //
            // Advance the error term and add an extra pixel if the error term so
            // indicates.
            //
            i32ErrorTerm += i32AdjUp;
            if ( i32ErrorTerm > 0 )
            {
                ui32RunLen++;
                i32ErrorTerm -= i32AdjDn;
            }

            //
            // Draw this run.
            //
            ui32FrBufOffset = ui32Y * ui32DisplayWidthBytes;
            DrawHorzRun(pui8FrmBuf + ui32FrBufOffset, ui32X, ui32RunLen,
                        i32Xadvance, ui32DrawingOps, ui32Color, ui32DrawPolarity);
            ui32X = ( i32Xadvance < 0 ) ? ui32X - ui32RunLen : ui32X + ui32RunLen;
        }

        //
        // Draw the final run of pixels.
        //
        ui32FrBufOffset += ui32DisplayWidthBytes;
        DrawHorzRun(pui8FrmBuf + ui32FrBufOffset, ui32X, ui32FinalPixelCount,
                    i32Xadvance, ui32DrawingOps, ui32Color, ui32DrawPolarity);
    }
    else
    {
        //
        // Y major line
        // Compute the minimum # of pixels in a run in this line.
        //
        i32WholeStep = i32Ydelta / i32Xdelta;

        //
        // Error term adjust each time X steps by 1, used to tell when one extra
        // pixel should be drawn as part of a run to account for fractional steps
        // along the Y axis per 1-pixel steps along X.
        //
        i32AdjUp = (i32Ydelta % i32Xdelta) * 2;

        //
        // Error term adjust when the error term turns over, used to factor out the
        // Y step made at that time.
        //
        i32AdjDn = i32Xdelta * 2;

        //
        // Initial error term - reflects an initial step of 1/2 along the X axis.
        //
        i32ErrorTerm = (i32Ydelta % i32Xdelta) - (i32Xdelta * 2);

        //
        // The initial and last runs are partial because X advances only 1/2 for
        // these runs rather than 1.  Divide one full run plus the initial pixel
        // between the initial and last runs.
        //
        ui32InitialPixelCount = (i32WholeStep / 2) + 1;
        ui32FinalPixelCount   = ui32InitialPixelCount;

        //
        // If the basic run length is even and there's no fractional advance, we
        // have a pixel that could go to either the initial or last partial run,
        // which we'll arbitrarily allocate to the last run.
        //
        if ( (i32AdjUp == 0) && ( (i32WholeStep & 0x01) == 0) )
        {
            ui32InitialPixelCount--;
        }

        //
        // If there are an odd number of pixels per run, we have 1 pixel that
        // can't be allocated to either the initial or last partial run, so
        // we'll add 1/2 to error term so this pixel will be handled by the
        // normal full-run loop.
        //
        if ( i32WholeStep & 0x01 )
        {
            i32ErrorTerm += i32Xdelta;
        }

        //
        // Draw the first, partial run of pixels.
        // Then draw all full runs.
        //
        ui32Y = ui32Y1;
        ui32X = ui32X1;
        ui32FrBufOffset = ui32Y * ui32DisplayWidthBytes;
        DrawVertRun(pui8FrmBuf + ui32FrBufOffset, ui32InitialPixelCount, ui32X,
                    ui32DisplayWidthBytes, ui32DrawingOps, ui32Color, ui32DrawPolarity);
        ui32X += i32Xadvance;
        ui32Y += ui32InitialPixelCount;

        //
        // Draw all full runs
        //
        for ( ix = 0; ix < i32Xdelta - 1; ix++ )
        {
            ui32RunLen = i32WholeStep;

            //
            // Advance the error term and add an extra pixel if the error term so
            // indicates.
            //
            i32ErrorTerm += i32AdjUp;
            if ( i32ErrorTerm > 0 )
            {
                ui32RunLen++;
                i32ErrorTerm -= i32AdjDn;
            }

            //
            // Draw this run.
            //
            ui32FrBufOffset = ui32Y * ui32DisplayWidthBytes;
            DrawVertRun(pui8FrmBuf + ui32FrBufOffset, ui32RunLen, ui32X,
                        ui32DisplayWidthBytes, ui32DrawingOps, ui32Color, ui32DrawPolarity);
            ui32X += i32Xadvance;
            ui32Y += ui32RunLen;
        }

        //
        // Draw the final run of pixels.
        //
        ui32FrBufOffset = ui32Y * ui32DisplayWidthBytes;
        DrawVertRun(pui8FrmBuf + ui32FrBufOffset, ui32FinalPixelCount, ui32X,
                    ui32DisplayWidthBytes, ui32DrawingOps, ui32Color, ui32DrawPolarity);
    }

    return true;

} // am_util_draw_2d1bpp_line_draw()

//*****************************************************************************
//
// Draw an ellipse into the display framebuffer.
//
//*****************************************************************************
bool
am_util_draw_2d1bpp_ellipse_draw(am_util_draw_2d1bpp_context_t *psDrawContext,
                                 uint32_t ui32X,    uint32_t ui32Y,
                                 uint32_t ui32Xrad, uint32_t ui32Yrad,
                                 uint32_t ui32DrawingOps,
                                 uint32_t ui32Color, bool bFill )
{
    int32_t i32WorkingX, i32WorkingY;
    int32_t i32Threshold, i32XAdjust, i32YAdjust;
    int32_t XradSquared = ui32Xrad * ui32Xrad;
    int32_t YradSquared = ui32Yrad * ui32Yrad;
    uint8_t *pui8FrmBufLine;
    uint8_t *pui8FrmBuf = psDrawContext->pui8Framebuffer;
    uint32_t ui32DrawPolarity;
    uint32_t ui32DisplayWidthBytes = psDrawContext->ui32DrawWidth / 8;
    bool bWorkingYchgd;

    //
    // Validate that this oval will fit on our screen.
    //
    if ( ((ui32X + ui32Xrad) >= psDrawContext->ui32DrawWidth)   ||
         ((ui32X - ui32Xrad) & 0x80000000)                      ||
         ((ui32Y + ui32Yrad) >= psDrawContext->ui32DrawHeight)  ||
         ((ui32Y - ui32Yrad) & 0x80000000)                      ||
         (ui32X == 0)                                           ||
         (ui32Y == 0) )
    {
        return false;
    }

    ui32DrawPolarity = psDrawContext->ui32DrawPolarity;

    //
    // Draw the four symmetric arcs for which X advances faster (that is,
    // for which X is the major axis).
    //
    // Begin by drawing the initial top & bottom points.
    //
    pui8FrmBufLine = pui8FrmBuf + (ui32Y + ui32Yrad) * ui32DisplayWidthBytes;
    modify_pixel_in_a_line(pui8FrmBufLine, ui32X, ui32DrawingOps, ui32Color, ui32DrawPolarity);
    pui8FrmBufLine = pui8FrmBuf + (ui32Y - ui32Yrad) * ui32DisplayWidthBytes;
    modify_pixel_in_a_line(pui8FrmBufLine, ui32X, ui32DrawingOps, ui32Color, ui32DrawPolarity);

    //
    // And start the loop to draw the four symetric arcs.
    //
    i32WorkingX = 0;
    i32WorkingY = ui32Yrad;
    i32XAdjust = 0;
    i32YAdjust = XradSquared * 2 * ui32Yrad;
    i32Threshold = XradSquared / 4 - XradSquared * ui32Yrad;

    bWorkingYchgd = true;
    for ( ; ; )
    {
        //
        // Advance the threshold to the value for the next X point
        // to be drawn.
        //
        i32Threshold += i32XAdjust + YradSquared;

        //
        // If the threshold has passed 0, then the Y coordinate has
        // advanced more than halfway to the next pixel and it's time
        // to advance the Y coordinate by 1 and set the next threshold
        // accordingly.
        //
        if ( i32Threshold >= 0 )
        {
            i32YAdjust -= XradSquared * 2;
            i32Threshold -= i32YAdjust;
            i32WorkingY--;
            bWorkingYchgd = true;
        }

        //
        // Advance the X coordinate by 1
        //
        i32XAdjust += YradSquared * 2;
        i32WorkingX++;

        //
        // Stop if X is no longer the major axis (the arc has passed the
        // point of a 45-degree slope).
        //
        if ( i32XAdjust >  i32YAdjust )
        {
            break;
        }

        //
        // Draw the 4 symmetries of the current point.
        // When filling, we only want to fill a line once for any particular
        // value of Y, otherwise we're redrawing pixels that have already been
        // drawn (not only inefficient, but XOR doesn't work). So when filling,
        // we'll track whether the Y line number has changed or not and only
        // draw a line when actually needed.
        // If not filling, we may very well need to draw more than 1 pixel (for
        // various X values) per line if that segment of the slope happens to be
        // flat, so we don't need to track whether Y changes or not.
        //
        pui8FrmBufLine = pui8FrmBuf + (ui32Y-i32WorkingY)*ui32DisplayWidthBytes;
        if ( bFill )
        {
            if ( bWorkingYchgd )
            {
                bWorkingYchgd = false;
                DrawOvalLine(pui8FrmBufLine, ui32X, i32WorkingX, ui32DrawingOps, ui32Color, true, ui32DrawPolarity);
                pui8FrmBufLine += (2*i32WorkingY)*ui32DisplayWidthBytes;
                DrawOvalLine(pui8FrmBufLine, ui32X, i32WorkingX, ui32DrawingOps, ui32Color, true, ui32DrawPolarity);
            }
        }
        else
        {
            DrawOvalLine(pui8FrmBufLine, ui32X, i32WorkingX, ui32DrawingOps, ui32Color, false, ui32DrawPolarity);
            pui8FrmBufLine += (2*i32WorkingY)*ui32DisplayWidthBytes;
            DrawOvalLine(pui8FrmBufLine, ui32X, i32WorkingX, ui32DrawingOps, ui32Color, false, ui32DrawPolarity);
        }
    }

    //
    // Draw the four symmetric arcs for which Y advances faster (that is,
    // for which Y is the major axis).
    // Draw the initial left & right points.
    //
    pui8FrmBufLine = pui8FrmBuf + ui32Y * ui32DisplayWidthBytes;
    DrawOvalLine(pui8FrmBufLine, ui32X, ui32Xrad, ui32DrawingOps, ui32Color, bFill, ui32DrawPolarity);

    // Draw the four arcs; set draw parameters for initial point (ui32Xrad,0).
    i32WorkingX = ui32Xrad;
    i32WorkingY = 0;
    i32XAdjust = YradSquared * 2 * ui32Xrad;
    i32YAdjust = 0;
    i32Threshold = YradSquared / 4 - YradSquared * ui32Xrad;

    for ( ; ; )
    {
        //
        // Advance the threshold to the value for the next Y point
        // to be drawn.
        //
        i32Threshold += i32YAdjust + XradSquared;

        //
        // If the threshold has passed 0, then the X coordinate has
        // advanced more than halfway to the next pixel and it's time
        // to advance the X coordinate by 1 and set the next threshold
        // accordingly.
        //
        if ( i32Threshold >= 0 )
        {
            i32XAdjust -= YradSquared * 2;
            i32Threshold -= i32XAdjust;
            i32WorkingX--;
        }

        //
        // Advance the Y coordinate by 1
        //
        i32YAdjust += XradSquared * 2;
        i32WorkingY++;

        //
        // Stop if Y is no longer the major axis (the arc has passed the
        // point of a 45-degree slope).
        //
        if ( i32YAdjust >  i32XAdjust )
        {
            break;
        }

        //
        // Draw the 4 symmetries of the current point
        //
        pui8FrmBufLine = pui8FrmBuf + (ui32Y-i32WorkingY) * ui32DisplayWidthBytes;
        DrawOvalLine(pui8FrmBufLine, ui32X, i32WorkingX, ui32DrawingOps, ui32Color, bFill, ui32DrawPolarity);
        pui8FrmBufLine = pui8FrmBuf + (ui32Y + i32WorkingY) * ui32DisplayWidthBytes;
        DrawOvalLine(pui8FrmBufLine, ui32X, i32WorkingX, ui32DrawingOps, ui32Color, bFill, ui32DrawPolarity);
    }

    return true;

} // am_util_draw_2d1bpp_ellipse_draw()

//*****************************************************************************
//
// End Doxygen group.
//! @}
//
//*****************************************************************************

