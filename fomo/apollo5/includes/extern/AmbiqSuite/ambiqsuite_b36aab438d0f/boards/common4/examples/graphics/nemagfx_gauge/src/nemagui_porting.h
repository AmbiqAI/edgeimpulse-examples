//*****************************************************************************
//
//! @file nemagui_porting.h
//
//*****************************************************************************

//*****************************************************************************
//
// ${copyright}
//
// This is part of revision ${version} of the AmbiqSuite Development Package.
//
//*****************************************************************************

#ifndef _NEMAGUI_PORTING_H
#define _NEMAGUI_PORTING_H

#include "stdint.h"

#define RESX        456
#define RESY        456

#ifndef FRAME_BUFFERS
    #ifdef AM_PART_APOLLO4L
        #define FRAME_BUFFERS 1
    #else
        #define FRAME_BUFFERS 2
    #endif
#endif

#ifndef BACK_BUFFERS
    #define BACK_BUFFERS 2
#endif

#endif
