#ifndef DEJAVUSERIF24PT4B_H
#define DEJAVUSERIF24PT4B_H

/*
    Do this:
        #define NEMA_FONT_IMPLEMENTATION
    before you include this file in *one* C file to create the implementation

    OR

    add:
        DejaVuSerif24pt4b.c
    to your C_SRCS to be compiled
*/

#include "nema_font.h"

extern nema_font_t DejaVuSerif24pt4b;

#ifdef NEMA_FONT_IMPLEMENTATION
#include "DejaVuSerif24pt4b.c"
#endif // NEMA_FONT_IMPLEMENTATION

#endif // DEJAVUSERIF24PT4B_H

