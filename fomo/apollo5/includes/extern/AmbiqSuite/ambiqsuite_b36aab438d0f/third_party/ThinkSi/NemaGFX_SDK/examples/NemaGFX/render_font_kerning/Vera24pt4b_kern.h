#ifndef VERA24PT4B_KERN_H
#define VERA24PT4B_KERN_H

/*
    Do this:
        #define NEMA_FONT_IMPLEMENTATION
    before you include this file in *one* C file to create the implementation

    OR

    add:
        Vera24pt4b_kern.c
    to your C_SRCS to be compiled
*/

#include "nema_font.h"

extern nema_font_t Vera24pt4b_kern;

#ifdef NEMA_FONT_IMPLEMENTATION
#include "Vera24pt4b_kern.c"
#endif // NEMA_FONT_IMPLEMENTATION

#endif // VERA24PT4B_KERN_H

