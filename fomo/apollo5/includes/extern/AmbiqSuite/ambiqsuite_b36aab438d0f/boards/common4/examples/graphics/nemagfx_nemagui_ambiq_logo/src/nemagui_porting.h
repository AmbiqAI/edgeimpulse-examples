//*****************************************************************************
//
// @file nemagui_porting.h
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

#include <stdint.h>

#if (defined(SCENARIO_A) || defined(SCENARIO_B))
    #define RESX        390
    #define RESY        390
#elif defined(SCENARIO_C)
    #define RESX        456
    #define RESY        456
#endif


#if defined(SCENARIO_A)
    #define FRAME_BUFFERS       1
#elif defined(SCENARIO_B)
    #define FRAME_BUFFERS       2
#elif defined(SCENARIO_C)
    #ifndef AM_PART_APOLLO4L
        #define FRAME_BUFFERS       2
    #else 
        #define FRAME_BUFFERS       1
    #endif
#endif

#endif
