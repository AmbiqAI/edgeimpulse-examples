//*****************************************************************************
//
//! @file path_data.h
//!
//*****************************************************************************

//*****************************************************************************
//
// ${copyright}
//
// This is part of revision ${version} of the AmbiqSuite Development Package.
//
//*****************************************************************************

#define RESX 452
#define RESY 452

#define COLOR_STOPS       5

nema_img_obj_t fb = {{0}, RESX, RESY, RESX*4, 0, NEMA_RGBA8888, 0};

nema_img_obj_t mask_a8 = {{0},  296,  154,  296    , 0, NEMA_A8, 0};
nema_img_obj_t mask_a4 = {{0},  296,  154,  296 / 2  , 0, NEMA_A4, 0};
nema_img_obj_t mask_a2 = {{0},  296,  154,  296 / 4  , 0, NEMA_A2, 0};
nema_img_obj_t mask_a1 = {{0},  296,  154,  296 / 8  , 0, NEMA_A1, 0};

nema_img_obj_t *A_masks[4];

uint8_t cmds_rectangle[] = {NEMA_VG_PRIM_MOVE, NEMA_VG_PRIM_LINE, NEMA_VG_PRIM_LINE, NEMA_VG_PRIM_LINE, NEMA_VG_PRIM_CLOSE};

float  coords_rectangle[] = {0.0f, 0.0f, 0.0f, 150.0f, 80.0f, 150.0f, 80.0f, 0.0f};

uint8_t fill_rules[] = {NEMA_VG_STROKE, NEMA_VG_FILL_EVEN_ODD, NEMA_VG_FILL_NON_ZERO};

float mask_stops[COLOR_STOPS] = {0.0f, 0.25f, 0.50f, 0.75f, 1.0f};

color_var_t mask_colors[COLOR_STOPS] = {{0  , 0  , 0  , 255}, //black
                                   {255, 0  , 0  , 255}, //red
                                   {0  , 255, 0  , 255}, //green
                                   {0  , 0  , 255, 255}, //blue
                                   {255, 255, 255, 255}}; //white
