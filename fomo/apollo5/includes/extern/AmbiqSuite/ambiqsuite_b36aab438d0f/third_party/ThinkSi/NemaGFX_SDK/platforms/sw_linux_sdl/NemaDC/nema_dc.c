/* TSI 2023.oem */
// -----------------------------------------------------------------------------
// Copyright (c) 2008-23 Think Silicon Single Member PC
// Think Silicon Single Member PC Confidential Proprietary
// -----------------------------------------------------------------------------
//     All Rights reserved - Unpublished -rights reserved under
//         the Copyright laws of the European Union
//
//  This file includes the Confidential information of Think Silicon Single
//  Member PC The receiver of this Confidential Information shall not disclose
//  it to any third party and shall protect its confidentiality by using the
//  same degree of care, but not less then a reasonable degree of care, as the
//  receiver uses to protect receiver's own Confidential Information. The entire
//  notice must be reproduced on all authorized copies and copies may only be
//  made to the extent permitted by a licensing agreement from Think Silicon
//  Single Member PC.
//
//  The software/data is provided 'as is', without warranty of any kind,
//  expressed or implied, including but not limited to the warranties of
//  merchantability, fitness for a particular purpose and noninfringement. In no
//  event shall Think Silicon Single Member PC be liable for any claim, damages
//  or other liability, whether in an action of contract, tort or otherwise,
//  arising from, out of or in connection with the software.
//
//  This file can be modified by OEMs as specified in the license agreement.
//
//  For further information please contact:
//
//                    Think Silicon Single Member PC
//                    http://www.think-silicon.com
//                    Patras Science Park
//                    Rion Achaias 26504
//                    Greece
// -----------------------------------------------------------------------------

#include <SDL2/SDL.h>

#include "interpose.h"
#include "nema_dc.h"

#include "nema_dc_hal.h"

// #define CONTINUOUS_UPDATE

typedef struct {
  SDL_Texture *tex;
  uint32_t *fb;
  SDL_Rect rect;
  uint32_t format;
} display_t;

typedef struct {
  SDL_Window *window;
  SDL_Renderer *renderer;
  display_t display;
} context_t;

static context_t ctx = {0};

#define DEFAULT_RESX 640
#define DEFAULT_RESY 480

static int sdl_init(void) {
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "SDL_Init() failed: %s\n", SDL_GetError());
        return -1;
    }

    return 0;
}

static int sdl_create_window(int w, int h) {
    ctx.display.rect.w = w;
    ctx.display.rect.h = h;

    if (ctx.window != 0) {
    	SDL_SetWindowSize(ctx.window, w, h);

    	return 0;
    } else {
	    SDL_Window *window = SDL_CreateWindow("Think Silicon - NEMA®|GFX",
	        SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, w, h, SDL_WINDOW_SHOWN);

	    if (window == NULL) {
	        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "SDL_CreateWindow() failed: %s\n", SDL_GetError());
	        return -1;
	    }

	    ctx.window = window;
	}

    SDL_Renderer *renderer = SDL_CreateRenderer(
      ctx.window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (renderer == NULL) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "SDL_CreateRenderer() failed: %s\n", SDL_GetError());

        return -1;
    }

    ctx.renderer = renderer;
    SDL_RendererInfo info;
   SDL_GetRendererInfo(renderer,
                        &info);

    return 0;
}


static uint32_t nema2sdl_format(uint32_t format) {
	switch(format) {

		case NEMADC_RGBA5551: return SDL_PIXELFORMAT_RGBA5551;
		case NEMADC_ABGR8888: return SDL_PIXELFORMAT_RGBA8888;
		case NEMADC_RGB332  : return SDL_PIXELFORMAT_RGB332;
		case NEMADC_RGB565  : return SDL_PIXELFORMAT_RGB565;
		case NEMADC_BGRA8888: return SDL_PIXELFORMAT_ARGB8888;
		case NEMADC_L8      : return SDL_PIXELFORMAT_INDEX8;
		case NEMADC_L1      : return SDL_PIXELFORMAT_INDEX1LSB;
		case NEMADC_L4      : return SDL_PIXELFORMAT_INDEX4LSB;
		case NEMADC_RGB24   : return SDL_PIXELFORMAT_RGB24;
		case NEMADC_ARGB8888: return SDL_PIXELFORMAT_BGRA8888;
		// case NEMADC_YUY2    : return SDL_PIXELFORMAT_YUY2;
		// case NEMADC_YUYV    : return SDL_PIXELFORMAT_UYVY;
		// case NEMADC_V_YUV420: return ;
		// case NEMADC_TLYUV420: return ;
		case NEMADC_RGBA8888:
		default:
		return SDL_PIXELFORMAT_ABGR8888;
	}

	return 0;
}

static void sdl_texture_update(int w, int h, uint32_t format) {
	if (ctx.display.tex != 0) {
		SDL_DestroyTexture(ctx.display.tex);
		ctx.display.tex = 0;
	}


    SDL_Texture *texture = SDL_CreateTexture(ctx.renderer,
            nema2sdl_format(format), SDL_TEXTUREACCESS_STREAMING, w, h);
    if (texture == NULL) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "SDL_CreateTexture() failed: %s\n", SDL_GetError());
    }

    ctx.display.tex = texture;
}

static void sdl_display_update(void) {
    if (ctx.display.fb != NULL) {
        SDL_UpdateTexture(ctx.display.tex, NULL, ctx.display.fb, nemadc_stride_size(ctx.display.format, ctx.display.rect.w));
        SDL_RenderCopy(ctx.renderer, ctx.display.tex, NULL, NULL);
        SDL_RenderPresent(ctx.renderer);
    }
}

#ifdef CONTINUOUS_UPDATE
static int sdl_display_loop(void *arg) {
    do {
        sdl_display_update();

        SDL_Delay(1000.f/30.f);
    } while(1);

    return 0;
}
#endif

static int
nemadc_format_size(uint32_t format)
{
    switch(format&0xff) {
    case NEMADC_RGBA8888:
    case NEMADC_ARGB8888:
    case NEMADC_ABGR8888:
    case NEMADC_BGRA8888: return 4;
    case NEMADC_RGB24   : return 3;
    case NEMADC_RGBA5551:
    case NEMADC_RGB565  : return 2;
    case NEMADC_RGB332  :
    case NEMADC_L8      :
    case NEMADC_L1      :
    case NEMADC_L4      : return 1;
    default             : return 0;
    }
}

//-------------------------------------------------------------------------------
int
nemadc_stride_size(uint32_t format, int width)
{
    int stride;
    switch(format) {
    case NEMADC_L1     : stride=(width  +7)/8;   break;
    case NEMADC_L4     : stride=(width*4+7)/8;   break;
    default            : stride=width*nemadc_format_size(format); break;

    }
    return stride;
}


int nemadc_init (void)
{
	_FUNCTION_PREAMBLE

    int ret = 0;
    ret = nemadc_sys_init();
    if (ret != 0) {
    	return ret;
    }

    ret = sdl_init();
    if (ret != 0) {
    	// exit(ret);
    	return ret;
    }

    ret = sdl_create_window(DEFAULT_RESX, DEFAULT_RESY);
    if (ret != 0) {
    	return ret;
    }

    //continuous update
#ifdef CONTINUOUS_UPDATE
    SDL_Thread *thread = SDL_CreateThread(sdl_display_loop, "display loop", (void *)NULL);

    if (NULL == thread) {
        printf("\nSDL_CreateThread failed: %s\n", SDL_GetError());
    }
#endif

	_FUNCTION_POSTAMBLE

	return 0;
}

uint32_t nemadc_get_config (void)
{
	_FUNCTION_PREAMBLE

	uint32_t rval = 0;

	_FUNCTION_BODY

	_FUNCTION_POSTAMBLE

	return rval;
}

uint32_t nemadc_get_crc (void)
{
	_FUNCTION_PREAMBLE

	uint32_t rval = 0;

	_FUNCTION_BODY

	_FUNCTION_POSTAMBLE

	return rval;
}

void nemadc_set_bgcolor (uint32_t rgba)
{
	_FUNCTION_PREAMBLE

	_FUNCTION_BODY

	_FUNCTION_POSTAMBLE

}

void nemadc_timing (int resx,int fpx,int blx,int bpx,int resy,int fpy,int bly,int bpy)
{
	_FUNCTION_PREAMBLE

	_FUNCTION_BODY

	SDL_SetWindowSize(ctx.window, resx, resy);

	_FUNCTION_POSTAMBLE

}

void nemadc_clkdiv (int div,int div2,int dma_prefetch, int phase)
{
	_FUNCTION_PREAMBLE

	_FUNCTION_BODY

	_FUNCTION_POSTAMBLE

}

void nemadc_clkctrl (uint32_t ctrl)
{
	_FUNCTION_PREAMBLE

	_FUNCTION_BODY

	_FUNCTION_POSTAMBLE

}

void nemadc_set_mode (int mode)
{
	_FUNCTION_PREAMBLE

	_FUNCTION_BODY

	_FUNCTION_POSTAMBLE

}

uint32_t nemadc_get_status (void)
{
	_FUNCTION_PREAMBLE

	uint32_t rval = 0;

	_FUNCTION_BODY

	_FUNCTION_POSTAMBLE

	return rval;
}

void nemadc_request_vsync_non_blocking (void)
{
//	_FUNCTION_PREAMBLE

	_FUNCTION_BODY

	_FUNCTION_POSTAMBLE

}

extern SDL_Renderer *SDL_ren;


void nemadc_set_layer (int layer_no,nemadc_layer_t * layer)
{
	// _FUNCTION_PREAMBLE

	ctx.display.fb = layer->baseaddr_virt;
	ctx.display.format = layer->format;

	if ((int)layer->resx != ctx.display.rect.w ||
		(int)layer->resy != ctx.display.rect.h || !ctx.display.tex) {


		sdl_create_window(layer->resx, layer->resy);
		sdl_texture_update(layer->resx, layer->resy, layer->format);
		SDL_Delay(60);
	}


#ifndef CONTINUOUS_UPDATE
    sdl_display_update();
#endif

	_FUNCTION_POSTAMBLE

}

void nemadc_set_layer_addr (int layer_no,uintptr_t addr)
{
	if (addr != 0U) {
		ctx.display.fb = (uint32_t *)addr;
	}

#ifndef CONTINUOUS_UPDATE
    sdl_display_update();
#endif

}

void nemadc_set_layer_gamma_lut (int layer,int index,int colour)
{
	_FUNCTION_PREAMBLE

	_FUNCTION_BODY

	_FUNCTION_POSTAMBLE

}

int nemadc_get_layer_gamma_lut (int layer,int index)
{
	_FUNCTION_PREAMBLE

	int rval = 0;

	_FUNCTION_BODY

	_FUNCTION_POSTAMBLE

	return rval;
}

void nemadc_set_palette (uint32_t index,uint32_t colour)
{
	_FUNCTION_PREAMBLE

	_FUNCTION_BODY

	_FUNCTION_POSTAMBLE

}

int nemadc_get_palette (uint32_t index)
{
	_FUNCTION_PREAMBLE

	int rval = 0;

	_FUNCTION_BODY

	_FUNCTION_POSTAMBLE

	return rval;
}

void nemadc_layer_disable (int layer_no)
{
	_FUNCTION_PREAMBLE

	_FUNCTION_BODY

	_FUNCTION_POSTAMBLE

}

void nemadc_layer_enable (int layer_no)
{
	_FUNCTION_PREAMBLE

	_FUNCTION_BODY

	_FUNCTION_POSTAMBLE

}

void nemadc_cursor_enable (int enable)
{
	_FUNCTION_PREAMBLE

	_FUNCTION_BODY

	_FUNCTION_POSTAMBLE

}

void nemadc_cursor_xy (int x,int y)
{
	_FUNCTION_PREAMBLE

	_FUNCTION_BODY

	_FUNCTION_POSTAMBLE

}

void nemadc_set_cursor_img (unsigned char * img)
{
	_FUNCTION_PREAMBLE

	_FUNCTION_BODY

	_FUNCTION_POSTAMBLE

}

void nemadc_set_cursor_lut (uint32_t index,uint32_t colour)
{
	_FUNCTION_PREAMBLE

	_FUNCTION_BODY

	_FUNCTION_POSTAMBLE

}

unsigned char nemadc_check_config (uint32_t flag)
{
	_FUNCTION_PREAMBLE

	unsigned char rval = 0;

	_FUNCTION_BODY

	_FUNCTION_POSTAMBLE

	return rval;
}

uint32_t nemadc_get_col_mode (void)
{
	_FUNCTION_PREAMBLE

	uint32_t rval = 0;

	_FUNCTION_BODY

	_FUNCTION_POSTAMBLE

	return rval;
}

int nemadc_get_layer_count (void)
{
	_FUNCTION_PREAMBLE

	int rval = 0;

	_FUNCTION_BODY

	_FUNCTION_POSTAMBLE

	return rval;
}
