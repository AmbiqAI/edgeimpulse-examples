//*****************************************************************************
//
//! @file paint.h
//!
//*****************************************************************************

//*****************************************************************************
//
// ${copyright}
//
// This is part of revision ${version} of the AmbiqSuite Development Package.
//
//*****************************************************************************

#ifndef __PAINT_H__
#define __PAINT_H__

#include "nema_vg.h"

#ifdef __cplusplus
extern "C" {
#endif

#define COLOR_STOPS     5
#define GRAD_SIZE       8
#define STAR_DIST_X     100
#define STAR_DIST_Y     150
#define SEG_SIZE_STAR   6
#define DATA_SIZE_STAR  12
#define REF_IMG_W       80
#define REF_IMG_H       100

float grad[GRAD_SIZE][4] =
{
    {10.0f , 100.0f, 75.0f , 150.0f}, // right down
    {75.0f , 150.0f, 10.0f , 100.0f}, // right up
    {90.0f , 100.0f, 25.0f , 150.0f}, // left down
    {25.0f , 150.0f, 90.0f , 100.0f}, // left up
    {10.0f , 100.0f, 90.0f , 100.0f}, // right
    {90.0f , 100.0f, 10.0f , 100.0f}, // left
    {50.0f , 50.0f , 50.0f , 150.0f}, // down
    {50.0f , 150.0f, 50.0f , 50.0f}   // up
};

float data_star_small[]   = {10.0f, 100.0f,
                             90.0f, 100.0f,
                             25.0f, 150.0f,
                             50.0f, 50.0f,
                             75.0f, 150.0f,
                             10.0f, 100.0f};

uint8_t cmds_star[] = {NEMA_VG_PRIM_MOVE,
                       NEMA_VG_PRIM_LINE,
                       NEMA_VG_PRIM_LINE,
                       NEMA_VG_PRIM_LINE,
                       NEMA_VG_PRIM_LINE,
                       NEMA_VG_PRIM_LINE};

float stops[COLOR_STOPS] = {0.0f, 0.25f, 0.50f, 0.75f, 1.0f};
color_var_t colors[COLOR_STOPS] = {{0  , 0  , 0  , 255}, //black
                                   {255, 0  , 0  , 255}, //red
                                   {0  , 255, 0  , 255}, //green
                                   {0  , 0  , 255, 255}, //blue
                                   {255, 255, 255, 255}}; //white

#ifdef __cplusplus
}
#endif

#endif //__PAINT_H__
