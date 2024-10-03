//*****************************************************************************
//
//! @file simhei30pt8b.h
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

#ifndef SIMHEI30PT8B_H
#define SIMHEI30PT8B_H

/*
    Do this:
        #define NEMA_FONT_IMPLEMENTATION
    before you include this file in *one* C file to create the implementation

    OR

    add:
        simhei30pt8b.c
    to your C_SRCS to be compiled
*/

#include "nema_font.h"

extern nema_font_t simhei30pt8b;

#ifdef NEMA_FONT_IMPLEMENTATION
#include "simhei30pt8b.c"
#endif // NEMA_FONT_IMPLEMENTATION

#endif // SIMHEI30PT8B_H

