#ifndef DEJAVUSERIF8PT2B_H
#define DEJAVUSERIF8PT2B_H

/*
    Do this:
        #define NEMA_FONT_IMPLEMENTATION
    before you include this file in *one* C file to create the implementation

    OR

    add:
        DejaVuSerif8pt2b.c
    to your C_SRCS to be compiled
*/

#include "nema_font.h"

extern nema_font_t DejaVuSerif8pt2b;

#ifdef NEMA_FONT_IMPLEMENTATION
#include "DejaVuSerif8pt2b.c"
#endif // NEMA_FONT_IMPLEMENTATION

#endif // DEJAVUSERIF8PT2B_H

