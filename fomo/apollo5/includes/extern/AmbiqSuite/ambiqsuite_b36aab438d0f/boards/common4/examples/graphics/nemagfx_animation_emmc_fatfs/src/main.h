//*****************************************************************************
//
//! @file main.h
//!
//! @brief Global includes for the NemaGFX example.
//
//*****************************************************************************

//*****************************************************************************
//
// ${copyright}
//
// This is part of revision ${version} of the AmbiqSuite Development Package.
//
//*****************************************************************************

#ifndef MAIN_H
#define MAIN_H
#include "am_bsp.h"

//*****************************************************************************
//
// Macro definitions
//
//*****************************************************************************
#define FRAME_BUFFERS           2
#define IMG_BUF_SIZE_RGB565     415872  //  456*456*2 = 0.3966MB
#define IMG_BUF_SIZE_RGBA8888   831744  // 456*456*4 = 0.7931MB
#define IMG_BUF_SIZE_TSC6       155952  // 456*456*6/8 = 0.1487MB

#define LOAD_IMG_FROM_EMMC_DIRECTLY     //!< if defined the project will open the all related images(combined into one file) and only transmit one frame once;if undefined will load all the seperate images to PSRAM once,then transmit one frame to display once.

#ifdef LOAD_IMG_FROM_EMMC_DIRECTLY
#else
#define MAX_ANIMATION_IMAGES    128         //!< define animation images maximum count.
typedef struct
{
    //! animation images address arrays.
    void *img[MAX_ANIMATION_IMAGES];
    //! animation image format.
    uint32_t format;
    //! the size of a frame animation image.
    uint32_t size;
    //! animation images count.
    uint16_t cnt;
    //! actived animation images index.
    uint16_t idx;

}animationImage_t;
#endif //LOAD_IMG_FROM_EMMC_DIRECTLY

//*****************************************************************************
//
// External function definitions.
//
//*****************************************************************************
int32_t g_draw_watch_with_animation(bool isDualBuffer, uint8_t img, int32_t offset, uint8_t onestyle, uint8_t twostyle);
void g_load_objects(void);

#endif // MAIN_H
