/* TSI 2023.xmp */
/*******************************************************************************
 * Copyright (c) 2023 Think Silicon Single Member PC
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this file and/or associated documentation files to use, copy,
 * modify, merge, publish, distribute, sublicense, and/or sell copies of the
 * Materials, and to permit persons to whom the Materials are furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Materials.
 *
 * The software is provided 'as is', without warranty of any kind, express or
 * implied, including but not limited to the warranties of merchantability,
 * fitness for a particular purpose and noninfringement. In no event shall Think
 * Silicon Single Member PC be liable for any claim, damages or other liability,
 * whether in an action of contract, tort or otherwise, arising from, out of or
 * in connection with the software or the use or other dealings in the software.
 ******************************************************************************/

// STANDALONE: compile as standalone application.
// UNIFIED_MEMORY: CPU and GPU share address space (e.g. baremetal systems)
// ENABLE_LAYER_SCALING: enable scaling of layers
// ENABLE_LAYER_BLENDING: enable blending/transparency of layers

// #define STANDALONE
// #define UNIFIED_MEMORY
#define ENABLE_LAYER_SCALING
#define ENABLE_LAYER_BLENDING

#ifndef UNIFIED_MEMORY
#include "nema_core.h"
#include <string.h>
#endif

#include "nema_dc.h"

#include "layer0.rgba.h"
#include "layer1.rgba.h"

// Display resolution
#define RESX 800
#define RESY 600

// Framebuffer resolution
#define FB_RESX 300
#define FB_RESY 200

static size_t framebuffer_size = FB_RESX*FB_RESY*4;

static nemadc_layer_t dc_layer0 = {(void *)0, 0, FB_RESX, FB_RESY, FB_RESX*4, 0, 0, RESX, RESY, 0xff, NEMADC_BL_SRC, 0, NEMADC_RGBA8888, 0, 0, 0, 0, 0};
static nemadc_layer_t dc_layer1 = {(void *)0, 0, FB_RESX, FB_RESY, FB_RESX*4, 0, 0, RESX, RESY, 0xff, NEMADC_BL_SRC, 0, NEMADC_RGBA8888, 0, 0, 0, 0, 0};

int
load_objects(void)
{
#ifndef UNIFIED_MEMORY
    // Initialize NemaGFX
    // Used only for graphics (contiguous) memory allocations
    if ( nema_init() != 0 ) {
        return -1;
    }

    //---------------------------------------------------
    // Load framebuffers to Contiguous Memory
    //---------------------------------------------------

    // Allocate buffers
    nema_buffer_t bo0 = nema_buffer_create(framebuffer_size);
    nema_buffer_t bo1 = nema_buffer_create(framebuffer_size);

    // memcpy framebuffers to allocated buffers
    memcpy(bo0.base_virt, layer0_rgba, framebuffer_size);
    memcpy(bo1.base_virt, layer1_rgba, framebuffer_size);

    dc_layer0.baseaddr_phys = bo0.base_phys;
    dc_layer0.baseaddr_virt = bo0.base_virt;

    dc_layer1.baseaddr_phys = bo1.base_phys;
    dc_layer1.baseaddr_virt = bo1.base_virt;
#endif

    return 0;
}

#ifdef STANDALONE
int
main(int argc, char *argv[])
#else
int two_layers()
#endif
{
    //Initialize NemaDC
    if ( nemadc_init() != 0 ) {
        return -2;
    }

    // Load framebuffers to Contiguous Memory space
    if ( load_objects() != 0 ) {
        return -3;
    }

    // http://tinyvga.com/vga-timing/800x600@60Hz
    //Format        | Pixclock | RESX | FP | SYNC | BP | RESY | FP | SYNC | BP
    //800x600, 60Hz | 40.000   | 800  | 40 | 128  | 88 | 600  | 1  | 4    | 23
    nemadc_timing(800, 40, 128, 88, 600, 1, 4, 23);

    // set NemaDC's background color to dark blue
    // Background color is visible when the available layers are not
    // covering the entire display resolution
    nemadc_set_bgcolor(0x00004000);

    // set layer's top left corner position (coordinates)
    // (0, 0) is on the top left corner of the display
    dc_layer0.startx = 40;
    dc_layer0.starty = 20;

    dc_layer1.startx = 140;
    dc_layer1.starty = 150;

#ifdef ENABLE_LAYER_BLENDING
    // ------------------------------------------------------------
    // LAYER BLENDING
    // ignored if layer-blending not enabled in hardware
    // ------------------------------------------------------------
    // - set layer's global alpha/transparency
    dc_layer0.alpha = 0x80; // range 0-255
    dc_layer1.alpha = 0x40; // range 0-255
    // - enable blending for each layer
    dc_layer0.blendmode = NEMADC_BL_SIMPLE;
    dc_layer1.blendmode = NEMADC_BL_SIMPLE;
#endif // ENABLE_LAYER_BLENDING

#ifdef ENABLE_LAYER_SCALING
    // ------------------------------------------------------------
    // LAYER SCALING
    // ignored if layer-scaler not enabled in hardware
    // ------------------------------------------------------------
    // - set layer's size on the display (width/height in pixels)
    // - layer's resx/resy correspond to the original framebuffer's resolution
    // - layer's sizex/sizey correspond to final scaled dimensions in pixels
    dc_layer0.sizex = dc_layer0.resx*1.2f;
    dc_layer0.sizey = dc_layer0.resy*1.2f;
    dc_layer1.sizex = dc_layer1.resx*1.2f;
    dc_layer1.sizey = dc_layer1.resy*1.2f;
#endif // ENABLE_LAYER_BLENDING

    // program and enable layers
    nemadc_set_layer(0, &dc_layer0);
    nemadc_set_layer(1, &dc_layer1);

    // - Modulate (multiply) each pixel's ALPHA channel with layer's global
    //   alpha
    nemadc_reg_write( NEMADC_REG_LAYER_MODE(0), NEMADC_MODULATE_A );
    // - Force (replace) each pixel's ALPHA channel with layer's global
    //   alpha
    nemadc_reg_write( NEMADC_REG_LAYER_MODE(1), NEMADC_FORCE_A );;

    return 0;
}
