#ifndef SIMHEI30PT4B_H
#define SIMHEI30PT4B_H

/*
    Do this:
        #define NEMA_FONT_IMPLEMENTATION
    before you include this file in *one* C file to create the implementation

    OR

    add:
        simhei30pt4b.c
    to your C_SRCS to be compiled
*/

#include "nema_font.h"

extern nema_font_t simhei30pt4b;

#ifdef NEMA_FONT_IMPLEMENTATION
#include "simhei30pt4b.c"
#endif // NEMA_FONT_IMPLEMENTATION

#endif // SIMHEI30PT4B_H

