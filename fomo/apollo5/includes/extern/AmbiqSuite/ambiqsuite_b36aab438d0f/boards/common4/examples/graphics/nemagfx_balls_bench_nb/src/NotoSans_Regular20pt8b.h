//*****************************************************************************
//
//! @file NotoSans_Regular20pt8b.h
//!
//! @brief NemaGFX example.

//
//*****************************************************************************

//*****************************************************************************
//
// ${copyright}
//
// This is part of revision ${version} of the AmbiqSuite Development Package.
//
//*****************************************************************************

#ifndef NOTOSANS_REGULAR20PT8B_H
#define NOTOSANS_REGULAR20PT8B_H

/*
    Do this:
        #define NEMA_FONT_IMPLEMENTATION
    before you include this file in *one* C file to create the implementation

    OR

    add:
        NotoSans_Regular20pt8b.c
    to your C_SRCS to be compiled
*/

#include "nema_font.h"

extern nema_font_t NotoSans_Regular20pt8b;

#ifdef NEMA_FONT_IMPLEMENTATION
#include "NotoSans_Regular20pt8b.c"
#endif // NEMA_FONT_IMPLEMENTATION

#endif // NOTOSANS_REGULAR20PT8B_H
