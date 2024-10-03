#include "nemagui_porting.h"

#ifndef BACK_BUFFERS
    #if defined(SCENARIO_A)
        #define BACK_BUFFERS 2
    #elif defined(SCENARIO_B)
        #define BACK_BUFFERS 2
    #elif defined(SCENARIO_C)
        #define BACK_BUFFERS 2
    #endif
#endif

#if defined(SCENARIO_A)
    #ifndef FRAME_BUFFER_FORMAT
        #ifdef AM_PART_APOLLO4L
            #define FRAME_BUFFER_FORMAT NEMA_RGB24
        #else
            #define FRAME_BUFFER_FORMAT NEMA_RGBA8888
        #endif
    #endif

    #ifndef BACK_BUFFER_FORMAT
        #define BACK_BUFFER_FORMAT NEMA_RGB565
    #endif
#elif defined(SCENARIO_B)
    #ifndef FRAME_BUFFER_FORMAT
        #define FRAME_BUFFER_FORMAT NEMA_RGB565
    #endif

    #ifndef BACK_BUFFER_FORMAT
        #define BACK_BUFFER_FORMAT NEMA_RGB565
    #endif
#elif defined(SCENARIO_C)
    #ifdef AM_PART_APOLLO4L
        #ifndef FRAME_BUFFER_FORMAT
            #define FRAME_BUFFER_FORMAT NEMA_RGB565
        #endif

        #ifndef BACK_BUFFER_FORMAT
            #define BACK_BUFFER_FORMAT NEMA_RGB565
        #endif
    #else
        #ifndef FRAME_BUFFER_FORMAT
            #define FRAME_BUFFER_FORMAT NEMA_TSC6
        #endif

        #ifndef BACK_BUFFER_FORMAT
            #define BACK_BUFFER_FORMAT NEMA_TSC6
        #endif
    #endif
#endif

#if FRAME_BUFFERS < 1
#error 0 && "FRAME_BUFFERS need to be at least 1"
#endif

#include "framebuffers.h"
#ifndef AM_PART_APOLLO4L
#include "nema_dc.h"
#endif
#include "nema_core.h"
#include "nema_utils.h"
#include "ng_globals.h"
#include "psram_malloc.h"
// #include "nema_graphics.h"

img_obj_t fb[FRAME_BUFFERS];
#ifndef AM_PART_APOLLO4L
nemadc_layer_t layer[FRAME_BUFFERS] = {0};
#endif

static img_obj_t back_buffer[BACK_BUFFERS];

#ifndef AM_PART_APOLLO4L
static inline int
format_to_dc(int format)
{
    switch(format)
    {
        case NEMA_RGBA5551: return NEMADC_RGBA5551;
        case NEMA_ABGR8888: return NEMADC_ABGR8888;
        case NEMA_RGB332  : return NEMADC_RGB332;
        case NEMA_RGB565  : return NEMADC_RGB565;
        case NEMA_BGRA8888: return NEMADC_BGRA8888;
        case NEMA_L8      : return NEMADC_L8;
        case NEMA_ARGB8888: return NEMADC_ARGB8888;
        case NEMA_TSC4    : return NEMADC_TSC4;
        case NEMA_TSC6    : return NEMADC_TSC6;
        default:
        case NEMA_RGBA8888: return NEMADC_RGBA8888;
    }
}
#endif

void load_framebuffers(void)
{
    // load frame buffers
    for (int id = 0; id < FRAME_BUFFERS; ++id)
    {
        fb[id].stride = nema_stride_size(FRAME_BUFFER_FORMAT, 0, RESX);
        fb[id].w = RESX;
        fb[id].h = RESY;
        fb[id].format = FRAME_BUFFER_FORMAT;

        if (FRAME_BUFFER_FORMAT == NEMA_TSC4)
        {
            fb[id].bo = nema_buffer_create(RESX * RESY / 2);
        }
        else if (FRAME_BUFFER_FORMAT == NEMA_TSC6)
        {
            fb[id].bo = nema_buffer_create(RESX * RESY * 3 / 4);
        }
        else
        {
            fb[id].bo = nema_buffer_create(fb[id].stride*fb[id].h);
        }

        nema_buffer_map(&fb[id].bo);

#ifndef AM_PART_APOLLO4L
        layer[id].sizex         = layer[id].resx = fb[id].w;
        layer[id].sizey         = layer[id].resy = fb[id].h;
        layer[id].stride        = -1;
        layer[id].format        = format_to_dc(FRAME_BUFFER_FORMAT);
        layer[id].blendmode     = NEMADC_BL_SRC;
        layer[id].baseaddr_phys = fb[id].bo.base_phys;
        layer[id].baseaddr_virt = fb[id].bo.base_virt;
        layer[id].flipx_en      = 0;
        layer[id].flipy_en      = 0;
#endif

    }

    // load back buffers
    for (int id = 0; id < BACK_BUFFERS; ++id)
    {
        back_buffer[id].stride = nema_stride_size(BACK_BUFFER_FORMAT, 0, RESX);
        back_buffer[id].w      = RESX;
        back_buffer[id].h      = RESY;
        back_buffer[id].format = BACK_BUFFER_FORMAT;

        if ( BACK_BUFFER_FORMAT == NEMA_TSC4 )
        {
            back_buffer[id].bo = nema_psram_buffer_create(RESX * RESY / 2);
        }
        else if ( BACK_BUFFER_FORMAT == NEMA_TSC6 )
        {
            back_buffer[id].bo = nema_psram_buffer_create(6 * RESX * RESY / 8);
        }
        else
        {
            back_buffer[id].bo = nema_psram_buffer_create(back_buffer[id].stride*back_buffer[id].h);
        }

        nema_buffer_map(&back_buffer[id].bo);
    }

#ifndef AM_PART_APOLLO4L
    ng_globals_register_framebuffers(FRAME_BUFFERS, fb, BACK_BUFFERS, back_buffer, layer);
#else
    ng_globals_register_framebuffers(FRAME_BUFFERS, fb, BACK_BUFFERS, back_buffer);
#endif

    ng_globals_set_resolution(RESX, RESY);
}
