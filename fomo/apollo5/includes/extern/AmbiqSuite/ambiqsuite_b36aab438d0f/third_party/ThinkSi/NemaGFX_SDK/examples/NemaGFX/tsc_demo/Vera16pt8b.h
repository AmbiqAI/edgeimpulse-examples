#ifndef VERA16PT8B_H
#define VERA16PT8B_H

/*
    Do this:
        #define NEMA_FONT_IMPLEMENTATION
    before you include this file in *one* C file to create the implementation

    OR

    add:
        Vera16pt8b.c
    to your C_SRCS to be compiled
*/

#include "nema_font.h"

extern nema_font_t Vera16pt8b;

#ifdef NEMA_FONT_IMPLEMENTATION
#include "Vera16pt8b.c"
#endif // NEMA_FONT_IMPLEMENTATION

#endif // VERA16PT8B_H

