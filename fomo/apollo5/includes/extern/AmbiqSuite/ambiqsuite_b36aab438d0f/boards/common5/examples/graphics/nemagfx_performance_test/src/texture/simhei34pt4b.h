//*****************************************************************************
//
//! @file simhei34pt4b.h
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

#ifndef SIMHEI34PT4B_H
#define SIMHEI34PT4B_H

/*
    Do this:
        #define NEMA_FONT_IMPLEMENTATION
    before you include this file in *one* C file to create the implementation

    OR

    add:
        simhei34pt4b.c
    to your C_SRCS to be compiled
*/

#include "nema_font.h"

extern nema_font_t simhei34pt4b;

#ifdef NEMA_FONT_IMPLEMENTATION
#include "simhei34pt4b.c"
#endif // NEMA_FONT_IMPLEMENTATION

#endif // SIMHEI34PT4B_H

