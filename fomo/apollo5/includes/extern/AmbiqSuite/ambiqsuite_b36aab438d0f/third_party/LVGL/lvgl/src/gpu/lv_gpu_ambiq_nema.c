//*****************************************************************************
//
//! @file lv_gpu_ambiq_nema.c
//!
//! @brief This is the GPU supporting file for LVGL.
//!
//
//*****************************************************************************

//*****************************************************************************
//
// Copyright (c) 2021, Ambiq Micro, Inc.
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// 1. Redistributions of source code must retain the above copyright notice,
// this list of conditions and the following disclaimer.
//
// 2. Redistributions in binary form must reproduce the above copyright
// notice, this list of conditions and the following disclaimer in the
// documentation and/or other materials provided with the distribution.
//
// 3. Neither the name of the copyright holder nor the names of its
// contributors may be used to endorse or promote products derived from this
// software without specific prior written permission.
//
// Third party software included in this distribution is subject to the
// additional license terms as defined in the /docs/licenses directory.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
// ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
// LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
// CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
// SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
// INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
// CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
// ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
// POSSIBILITY OF SUCH DAMAGE.
//
//*****************************************************************************
#include "lvgl.h"
#include "lv_gpu_ambiq_nema.h"

#include "am_mcu_apollo.h"

#include "nema_hal.h"
#include "nema_math.h"
#include "nema_core.h"
#include "nema_regs.h"
#include "nema_utils.h"
#include "nema_event.h"
#include "nema_raster.h"
#include "nema_graphics.h"
#include "nema_provisional.h"
#include "nema_error.h"
#include "nema_sys_defs.h"

#if !defined(AM_PART_APOLLO4L) && !defined(AM_PART_APOLLO4B)
    #include "nema_vg.h"
#endif

#include "lv_ambiq_misc.h"
#include "lv_ambiq_nema_hal.h"

//*****************************************************************************
//
// Internal function declaration.
//
//*****************************************************************************
extern void nema_cl_free(nema_cmdlist_t *cl);

extern uint32_t _lv_gpu_ambiq_nema_init(lv_gpu_ambiq_version_t* lib_ver);

extern uint32_t _lv_gpu_ambiq_nema_fill(const lv_area_t * disp_area,
                        lv_color_t * disp_buf,
                        const lv_area_t * draw_area,
                        lv_color_t color, lv_opa_t opa,
                        const lv_opa_t * mask, lv_draw_mask_res_t mask_res);
extern uint32_t _lv_gpu_ambiq_nema_map(const lv_area_t * disp_area, 
                        lv_color_t * disp_buf,
                        const lv_area_t * draw_area,
                        const lv_area_t * map_area, 
                        const lv_color_t * map_buf, 
                        lv_opa_t opa,
                        const lv_opa_t * mask, 
                        lv_draw_mask_res_t mask_res);
extern uint32_t _lv_gpu_ambiq_texture_recolor(const lv_img_dsc_t * img_src, 
                        const lv_img_dsc_t * img_recolor, 
                        nema_tex_format_t nema_cf, 
                        lv_color_t recolor, 
                        lv_opa_t recolor_opa);
extern uint32_t _lv_gpu_ambiq_nema_blit(const lv_area_t * coords,
                        const lv_area_t * clip_area,
                        const void * src,
                        const lv_draw_img_dsc_t * dsc,
                        nema_tex_format_t nema_cf);
extern uint32_t _lv_gpu_ambiq_nema_draw_letter(lv_coord_t pos_x, 
                        lv_coord_t pos_y, 
                        lv_font_glyph_dsc_t * g,
                        const lv_area_t * clip_area,
                        const uint8_t * map_p, 
                        lv_color_t color, lv_opa_t opa, lv_blend_mode_t blend_mode);
extern uint32_t _lv_gpu_ambiq_nema_draw_border_rounded(const lv_area_t * clip_area, 
                        const lv_area_t * outer_area, 
                        const lv_area_t * inner_area,
                        lv_coord_t rout, 
                        lv_coord_t rin, lv_color_t color, lv_opa_t opa, 
                        lv_blend_mode_t blend_mode);
extern uint32_t _lv_gpu_ambiq_nema_draw_border_rec(const lv_area_t * clip_area, 
                        const lv_area_t * outer_area, 
                        const lv_area_t * inner_area,
                        lv_color_t color, lv_opa_t opa, 
                        lv_blend_mode_t blend_mode);
extern uint32_t _lv_gpu_ambiq_nema_draw_bg(const lv_area_t * clip_area, 
                        const lv_area_t * draw_area,
                        lv_coord_t rout, lv_color_t color, lv_opa_t opa, 
                        lv_blend_mode_t blend_mode);
extern uint32_t _lv_gpu_ambiq_nema_draw_bg_img(lv_area_t * coords, const lv_area_t * mask,
                        const lv_draw_rect_dsc_t * dsc_rect,
                        const lv_draw_img_dsc_t * dsc_img,
                        nema_tex_format_t nema_cf);
extern uint32_t _lv_gpu_ambiq_nema_draw_line_normal(const lv_point_t * point1, 
                        const lv_point_t * point2,
                        const lv_draw_line_dsc_t * dsc);
extern uint32_t _lv_gpu_ambiq_nema_draw_line_dashed(float* line_coordinate,
                        const lv_draw_line_dsc_t * dsc, bool * run_again, float angle);
extern uint32_t _lv_gpu_ambiq_nema_draw_triangle(const lv_point_t points[], 
                        const lv_area_t * clip_area, 
                        const lv_draw_rect_dsc_t * draw_dsc);
extern uint32_t _lv_gpu_ambiq_nema_draw_quadrangle(const lv_point_t points[], 
                        const lv_area_t * clip_area, 
                        const lv_draw_rect_dsc_t * draw_dsc);
extern uint32_t _lv_gpu_ambiq_nema_draw_arc(lv_coord_t center_x, lv_coord_t center_y, 
                        uint16_t radius, uint16_t start_angle, 
                        uint16_t end_angle, const lv_area_t * clip_area, 
                        const lv_draw_arc_dsc_t * dsc,
                        lv_img_header_t bg_img_header,
                        nema_tex_format_t bg_img_nema_cf);

//*****************************************************************************
//
// Macro definitions
//
//*****************************************************************************
#define CL_SIZE_DEFAULT         (512)
#ifdef AM_PART_APOLLO4B
#define CL_SIZE_NORMAL_FILL     (256)
#else
#define CL_SIZE_NORMAL_FILL     (128)
#endif
#define CL_SIZE_MASK_FILL       (320)
#define CL_SIZE_MAP             (512)
#define CL_SIZE_BLIT            (448)
#define CL_SIZE_LABEL           (1024)
#define CL_SIZE_LETTER          (336)
#define CHAR_CMD_NUMBER         (30)
#define CL_SIZE_ROUNDED_RECT    (10*1024)
#define CL_SIZE_ARC             (10*1024)
#define CL_SIZE_BOARDER         (368)
#define CL_SIZE_LINE            (1024)
#define CL_SIZE_LINE_NORMAL     (512)
#define CL_SIZE_RECOLOR         (1024)


#if defined(AM_PART_APOLLO5B) //apollo5B uses hardware VG, less CL memory is acceptable
#define CL_SIZE_TSVG            (10*1024)
#else
#define CL_SIZE_TSVG            (40*1024)
#endif

#define COLOR_FORMAT_INVALID    (0xffffffffUL)
#define COLOR_FORMAT_TSVG       (0xfffffffeUL)

//*****************************************************************************
//
//! @brief command list management
//!
//! Manage the CLs used in this file. Every time we run the following APIs,
//! a dedicated CL is created to hold the GPU commands, this structure record
//! these CLs for GPU sync and garbage collection.
//*****************************************************************************
typedef struct
{
    //! Record the size of allocated buffer.
    uint32_t total_buffer;

    //! Record total CLs created.
    uint32_t total_cl;

    //! Last CL submittion id, used for cl_wait()
    int32_t last_cl_id;

    //! CL list head
    nema_cmdlist_t* cl_head;
}
CommandListManage_t;

//*****************************************************************************
//
// Static Variables
//
//*****************************************************************************
//!
static CommandListManage_t CLManage =
{
    .cl_head = NULL,
    .total_buffer = 0,
    .total_cl = 0,
};

//! CL used by label widget
static nema_cmdlist_t* pLabelCL;

// Allocate memory and create CL
static nema_cmdlist_t* create_cl(uint32_t cl_size, bool circular);

//Clear current bounded cl
static void clear_current_cl(void);

// Create cl, bind destination, bind clip
static nema_cmdlist_t* lv_gpu_ambiq_nema_cl_create_bind(img_obj_t* fb,
                                            const lv_area_t * clip_area,
                                            uint32_t cl_size,
                                            bool circular,
                                            bool clip_offset);

//Submit cl, link current node.
static void lv_gpu_ambiq_nema_cl_submit_link(nema_cmdlist_t* cl);

//Draw rounded rectangle boarder
static lv_res_t lv_gpu_ambiq_nema_draw_border_rounded(const lv_area_t * clip_area,
                                                  const lv_area_t * outer_area,
                                                  const lv_area_t * inner_area,
                                                  lv_coord_t rout,
                                                  lv_coord_t rin,
                                                  lv_color_t color,
                                                  lv_opa_t opa,
                                                  lv_blend_mode_t blend_mode);

//Draw rectangle boarder.
static lv_res_t lv_gpu_ambiq_nema_draw_border_rec(const lv_area_t * clip_area,
                                                  const lv_area_t * outer_area,
                                                  const lv_area_t * inner_area,
                                                  lv_color_t color,
                                                  lv_opa_t opa,
                                                  lv_blend_mode_t blend_mode);
//Draw normal line
static lv_res_t lv_gpu_ambiq_nema_draw_line_normal(const lv_point_t * point1, 
                                        const lv_point_t * point2, 
                                        const lv_area_t * clip,
                                        const lv_draw_line_dsc_t * dsc);

//Draw dashed line
static lv_res_t lv_gpu_ambiq_nema_draw_line_dashed(const lv_point_t * point1, 
                                        const lv_point_t * point2, 
                                        const lv_area_t * clip,
                                        const lv_draw_line_dsc_t * dsc);

/**
* Reset and initialize NEMA_GFX device. This function should be called as a part
* of display init sequence.
*
* @return LV_RES_OK: NEMA_GFX init ok; LV_RES_INV: init error. See error log for more
*                    information.
*/
lv_res_t lv_gpu_ambiq_nema_init(void)
{
    LV_ASSERT(CLManage.cl_head == NULL);

    CLManage.cl_head = NULL;
    CLManage.total_buffer = 0;
    CLManage.total_cl = 0;

    lv_gpu_ambiq_version_t gpu_lib_ver = {0};
    uint32_t gpu_ret = _lv_gpu_ambiq_nema_init(&gpu_lib_ver);

    if((gpu_ret != NEMA_ERR_NO_ERROR) && (gpu_ret != NEMA_ERR_NO_INIT))
    {
        LV_LOG_ERROR("GPU error: %s", lv_ambiq_gpu_error_interpret(gpu_ret));
        return LV_RES_INV;
    }
    LV_LOG_INFO("GPU Lib Version: %d.%d.%d\n", gpu_lib_ver.s.Major, gpu_lib_ver.s.Minor, gpu_lib_ver.s.Revision);

    //Power on GPU for NemaSDK initialization.
    am_hal_pwrctrl_periph_enable(AM_HAL_PWRCTRL_PERIPH_GFX);

    //Initialize NemaGFX.
    nema_init();

    uint32_t ret = nema_get_error();
    if(ret != NEMA_ERR_NO_ERROR)
    {
        LV_LOG_ERROR("NemaGFX init failed!\n");
    }

#if !defined(AM_PART_APOLLO4L) && !defined(AM_PART_APOLLO4B)
    //Initialize NemaGFX.
    nema_vg_init(LV_AMBIQ_FB_RESX, LV_AMBIQ_FB_RESY);
    ret = nema_vg_get_error();
    if(ret != NEMA_VG_ERR_NO_ERROR)
    {
        LV_LOG_ERROR("NemaVG init failed!\n");
    }
    nema_vg_handle_large_coords(1U, 1U);
#endif

#ifdef NEMA_GFX_POWERSAVE
    // Power off GPU. It will keep in power down state until it is needed.
    am_hal_pwrctrl_periph_disable(AM_HAL_PWRCTRL_PERIPH_GFX);   

#if defined(NEMA_GFX_POWEROFF_END_CL)
    nemagfx_set_interrupt_callback(lv_ambiq_nema_gpu_isr_cb);
#endif

#endif

    return LV_RES_OK;
}

/**
* Disable NEMA_GFX device. Should be called during display deinit sequence.
*/
void lv_gpu_ambiq_nema_deinit(void)
{
    //clean command list buffer
    if(CLManage.cl_head != NULL)
    {
        nema_cl_destroy(CLManage.cl_head);
    }
}

/**
 * Can be used as `gpu_wait_cb` in display driver to
 * let the MCU run while the GPU is working
 */
void lv_gpu_ambiq_nema_wait(struct _lv_disp_drv_t * disp_drv)
{
    if(CLManage.cl_head != NULL)
    {
        //Wait GPU, CLManage.cl_head always points to the last CL that we have submitted.
        nema_cl_wait(CLManage.cl_head);
        LV_ASSERT(nema_get_error() == NEMA_ERR_NO_ERROR);

        //Destroy the allocated CL and its buffer.
        nema_cl_free(CLManage.cl_head);

        CLManage.cl_head = NULL;
    }
}

static nema_cmdlist_t* create_cl(uint32_t cl_size, bool circular)
{
   //Create CL
    nema_cmdlist_t* cl = (nema_cmdlist_t *)nema_host_malloc(sizeof(nema_cmdlist_t));

    // If malloc failed, try flush the pending CL
    if(cl == NULL)
    {
        // flush pending CL
        lv_gpu_ambiq_nema_wait(NULL);

        //try again
        cl = (nema_cmdlist_t *)nema_host_malloc(sizeof(nema_cmdlist_t));
    }

    if(cl == NULL)
    {
        LV_LOG_ERROR("CL create error. STOP!.\n");
        return NULL;
    }

    *cl = nema_cl_create_sized(cl_size);

    // If malloc failed, try flush the pending CL
    if(cl->bo.base_virt == NULL)
    {
        // flush cl
        lv_gpu_ambiq_nema_wait(NULL);

        // Try again.
        *cl = nema_cl_create_sized(cl_size);
    }

    //If there is still no enough memory, cut the size to half of it.
    if((cl->bo.base_virt == NULL) && circular)
    {
        while(cl_size >= 1024)
        {
            cl_size /= 2;

            *cl = nema_cl_create_sized(cl_size);

            if(cl->bo.base_virt != NULL)
            {
                break;
            }

        }
    }

    LV_ASSERT(NULL != cl->bo.base_virt);

    if(cl->bo.base_virt == NULL)
    {
        LV_LOG_ERROR("CL buffer allocation failed. STOP!.\n");
        nema_host_free(cl);

        return NULL;
    }

    return cl;
}

static void clear_current_cl(void)
{
    //Get current cl;
    nema_cmdlist_t* cur_cl = nema_cl_get_bound();

    //Unbind
    nema_cl_unbind();

#ifdef NEMA_GFX_POWERSAVE
    //Check and power off GPU
    NEMA_BUILDCL_END

#ifdef NEMA_GFX_POWEROFF_END_CL
    lv_ambiq_nema_gpu_check_busy_and_suspend();
#endif

#endif

    //Clear its memory space
    if(cur_cl != NULL)
    {
        cur_cl->next = NULL;
        nema_cl_free(cur_cl);
    }
}

static nema_cmdlist_t* lv_gpu_ambiq_nema_cl_create_bind(img_obj_t* fb,
                                                const lv_area_t * clip_area,
                                                uint32_t cl_size,
                                                bool circular,
                                                bool clip_offset)
{
    lv_area_t disp_area;
    void* disp_buf;
    int fb_w;
    int fb_h;
    nema_tex_format_t format;

    if(fb == NULL)
    {
        lv_disp_t * disp = _lv_refr_get_disp_refreshing();
        lv_disp_draw_buf_t * draw_buf = lv_disp_get_draw_buf(disp);

        disp_area = draw_buf->area;
        disp_buf = draw_buf->buf_act;

        fb_w = lv_area_get_width(&disp_area);
        fb_h = lv_area_get_height(&disp_area);
        img_obj_t * frame_buf = disp->driver->user_data;
        if(frame_buf != NULL)
        {
            format = frame_buf->format;
        }
        else
        {
            format = NEMA_BGRA8888;
            LV_LOG_ERROR("display driver user_data is NULL!\n");
        }
    }
    else
    {
        disp_area.x1 = 0;
        disp_area.y1 = 0;
        disp_area.x2 = disp_area.x1 + fb->w -1;
        disp_area.y2 = disp_area.y1 + fb->h -1;
        disp_buf = fb->bo.base_virt;
        fb_w = fb->w;
        fb_h = fb->h;
        format = fb->format;
    }

    //Create cl
    nema_cmdlist_t* cl = create_cl(cl_size, circular);
    if(cl == NULL)
    {
        return cl;
    }

#ifdef NEMA_GFX_POWERSAVE
    //Power up GPU
    NEMA_BUILDCL_START
    lv_res_t ret = lv_ambiq_nema_gpu_power_on();
    LV_ASSERT(ret == LV_RES_OK);
#endif

    //bind the CL
    if(circular)
    {
        nema_cl_bind_circular(cl);
    }
    else
    {
        nema_cl_bind(cl);
    }

    LV_ASSERT(nema_get_error() == NEMA_ERR_NO_ERROR);

#if defined(AM_PART_APOLLO5A) || defined(AM_PART_APOLLO5B)
    //Destination buffer should be in the SSRAM or psram.
    LV_ASSERT_MSG((((uintptr_t)disp_buf >= SSRAM_BASEADDR ) && 
    ( (uintptr_t)disp_buf <= (SSRAM_BASEADDR + SSRAM_MAX_SIZE))) || 
    (((uintptr_t)disp_buf >= MSPI0_APERTURE_START_ADDR ) && 
    ( (uintptr_t)disp_buf <= MSPI2_APERTURE_END_ADDR)), "Des buffer should be in SSRAM or PSRAM");
#else
    //Destination buffer should be in the SSRAM or psram.
    LV_ASSERT_MSG((((uintptr_t)disp_buf >= SSRAM0_BASEADDR ) && 
    ( (uintptr_t)disp_buf <= (SSRAM0_BASEADDR + NONTCM_MAX_SIZE))) || 
    (((uintptr_t)disp_buf >= MSPI0_APERTURE_START_ADDR ) && 
    ( (uintptr_t)disp_buf <= MSPI2_APERTURE_END_ADDR)), "Des buffer should be in SSRAM or PSRAM");
#endif

    LV_ASSERT_MSG(!((uintptr_t)disp_buf & 0x00000007), "Destination buffer should align to 64bits");

    // Bind framebuffer
    nema_bind_dst_tex((uintptr_t)disp_buf,
                      fb_w,
                      fb_h,
                      format,
                      -1);

    LV_ASSERT(nema_get_error() == NEMA_ERR_NO_ERROR);

    //Set clip
    lv_area_t temp_area;
    if(clip_area == NULL)
    {
        temp_area = disp_area;
    }
    else
    {
        temp_area = *clip_area;
    }

    if(clip_offset)
    {
        lv_area_move(&temp_area, -disp_area.x1, -disp_area.y1);
    }
    int clip_w = lv_area_get_width(&temp_area);
    int clip_h = lv_area_get_height(&temp_area);
    nema_set_clip(temp_area.x1, temp_area.y1, clip_w, clip_h);

    LV_ASSERT(nema_get_error() == NEMA_ERR_NO_ERROR);

    return cl;
}

static void lv_gpu_ambiq_nema_cl_submit_link(nema_cmdlist_t* cl)
{
    //Start GPU, submit CL
    nema_cl_submit(cl);

#ifdef LV_GPU_AMBIQ_DEBUG_CL
    nema_cl_wait(cl);
#endif

#ifdef NEMA_GFX_POWERSAVE
    //Power down the GPU
    NEMA_BUILDCL_END

#ifdef NEMA_GFX_POWEROFF_END_CL
    lv_ambiq_nema_gpu_check_busy_and_suspend();
#endif

#endif

    //Check wether the core ring buffer in NemaSDK is full
    bool rb_ret = nema_rb_check_full();
    if(rb_ret)
    {
        //Wait for GPU.
        nema_cl_wait(CLManage.cl_head);
    }

    //Unbind the CL
    nema_cl_unbind();

    LV_ASSERT(nema_get_error() == NEMA_ERR_NO_ERROR);

    AM_CRITICAL_BEGIN

    //Insert current cl to cl_list
    cl->next = CLManage.cl_head;
    CLManage.cl_head = cl;

    AM_CRITICAL_END
}

lv_res_t lv_gpu_ambiq_nema_check_common(const lv_area_t *influenced_area, lv_blend_mode_t blend_mode)
{
    lv_disp_t * disp = _lv_refr_get_disp_refreshing();

    //If we have a preset px drawing API.
    if(disp->driver->set_px_cb && (disp->driver->user_data == NULL))
    {
        LV_LOG_WARN("GPU failed, set_px_cb is set!");
        return LV_RES_INV;
    }

    //TODO:We can't support draw with an underlying mask layer.
    if(influenced_area != NULL)
    {
        bool mask_any = lv_draw_mask_is_any(influenced_area);
        if(mask_any)
        {
            LV_LOG_WARN("GPU failed, underlying mask exists!");
            return LV_RES_INV;
        }
    }

    //TODO: Only normal blend mode is supported.
    if(blend_mode != LV_BLEND_MODE_NORMAL)
    {
        LV_LOG_WARN("GPU failed, blend mode is not supported!");
        return LV_RES_INV;
    }

    return LV_RES_OK;
}

/**
 * Fill an area with a color
 * @param disp_area the current display area (destination area)
 * @param disp_buf destination buffer
 * @param draw_area fill this area (relative to `disp_area`)
 * @param color fill color
 * @param opa overall opacity in 0x00..0xff range
 * @param mask a mask to apply on every pixel (uint8_t array with 0x00..0xff values).
 *                It fits into draw_area.
 * @param mask_res LV_MASK_RES_COVER: the mask has only 0xff values (no mask),
 *                 LV_MASK_RES_TRANSP: the mask has only 0x00 values (full transparent),
 *                 LV_MASK_RES_CHANGED: the mask has mixed values
 * @return LV_RES_OK, if we have drawn this object by GPU. LV_RES_INV, not supported.
 */
lv_res_t lv_gpu_ambiq_nema_fill(const lv_area_t * disp_area,
                            lv_color_t * disp_buf,
                            const lv_area_t * draw_area,
                            lv_color_t color, lv_opa_t opa,
                            const lv_opa_t * mask, lv_draw_mask_res_t mask_res)
{
    //Check limitation.
    //Disable the following minimum fill size limitation, as we may need to draw to a GPU-only frame buffer
    //CPU can not write to this kind of framebuffer, add this limitation will lower the GPU acceleration coverage.
    // if((lv_area_get_size(draw_area) <= LV_AMBIQ_NEMA_FILL_SIZE_LIMIT) &&
    //    (mask_res == LV_DRAW_MASK_RES_CHANGED) )
    // {
    //     return LV_RES_INV;
    // }

    nema_cmdlist_t* cl;

    if(mask_res == LV_DRAW_MASK_RES_CHANGED)
    {
        cl = lv_gpu_ambiq_nema_cl_create_bind(NULL, draw_area, CL_SIZE_MASK_FILL, false, false);

#if defined(AM_PART_APOLLO5A)  || defined(AM_PART_APOLLO5B)
        // Mask buffer should be allocated from SSRAM or PSRAM
        LV_ASSERT_MSG((((uintptr_t)mask >= SSRAM_BASEADDR ) && 
        ( (uintptr_t)mask <= (SSRAM_BASEADDR + SSRAM_MAX_SIZE))) || 
        (((uintptr_t)mask >= MSPI0_APERTURE_START_ADDR ) && 
        ( (uintptr_t)mask <= MSPI2_APERTURE_END_ADDR)), "Mask buffer should be allocated from SSRAM or PSRAM");  
#else
        // Mask buffer should be allocated from SSRAM or PSRAM
        LV_ASSERT_MSG((((uintptr_t)mask >= SSRAM0_BASEADDR ) && 
        ( (uintptr_t)mask <= (SSRAM0_BASEADDR + NONTCM_MAX_SIZE))) || 
        (((uintptr_t)mask >= MSPI0_APERTURE_START_ADDR ) && 
        ( (uintptr_t)mask <= MSPI2_APERTURE_END_ADDR)), "Mask buffer should be allocated from SSRAM or PSRAM");  
#endif			
    }
    else
    {
        cl = lv_gpu_ambiq_nema_cl_create_bind(NULL, draw_area, CL_SIZE_NORMAL_FILL, false, false);
    }

    if(cl == NULL)
    {
        return LV_RES_INV;
    }

    uint32_t gpu_ret = _lv_gpu_ambiq_nema_fill(disp_area, disp_buf, draw_area, color, opa, mask, mask_res);
    if(gpu_ret != NEMA_ERR_NO_ERROR)
    {
        LV_LOG_ERROR("GPU error: %s", lv_ambiq_gpu_error_interpret(gpu_ret));
        clear_current_cl();
        return LV_RES_INV;
    }
    
    lv_gpu_ambiq_nema_cl_submit_link(cl);

    if(mask_res == LV_DRAW_MASK_RES_CHANGED)
    {
        //Do the sync operation to prevent the mask content being updated.
        lv_gpu_ambiq_nema_wait(NULL);
    }

    return LV_RES_OK;
}

/**
 * Copy an image to an area
 * @param disp_area the current display area (destination area)
 * @param disp_buf destination buffer
 * @param map_area coordinates of the map (image) to copy. (absolute coordinates)
 * @param map_buf the pixel of the image
 * @param opa overall opacity in 0x00..0xff range
 * @param mask a mask to apply on every pixel (uint8_t array with 0x00..0xff values).
 *                It fits into draw_area.
 * @param mask_res LV_MASK_RES_COVER: the mask has only 0xff values (no mask),
 *                 LV_MASK_RES_TRANSP: the mask has only 0x00 values (full transparent),
 *                 LV_MASK_RES_CHANGED: the mask has mixed values
 * @return LV_RES_OK, if we have drawn this object by GPU. LV_RES_INV, not supported.
 */
lv_res_t lv_gpu_ambiq_nema_map(const lv_area_t * disp_area, lv_color_t * disp_buf,
                           const lv_area_t * draw_area,
                           const lv_area_t * map_area, const lv_color_t * map_buf, lv_opa_t opa,
                           const lv_opa_t * mask, lv_draw_mask_res_t mask_res)
{
    //Disable the following minimum fill size limitation, as we may need to draw to a GPU-only frame buffer
    //CPU can not write to this kind of framebuffer, add this limitation will lower the GPU acceleration coverage.
    // if((lv_area_get_size(draw_area) <= LV_AMBIQ_NEMA_FILL_SIZE_LIMIT) &&
    //    (mask_res == LV_DRAW_MASK_RES_CHANGED) )
    // {
    //     return LV_RES_INV;
    // }

    nema_cmdlist_t* cl = lv_gpu_ambiq_nema_cl_create_bind(NULL, draw_area, CL_SIZE_MAP, false, false);
    if(cl == NULL)
    {
        return LV_RES_INV;
    }

    if(mask_res == LV_DRAW_MASK_RES_CHANGED)
    {
#if defined(AM_PART_APOLLO5A)  || defined(AM_PART_APOLLO5B)
        LV_ASSERT_MSG((((uintptr_t)mask >= SSRAM_BASEADDR ) && 
        ( (uintptr_t)mask <= (SSRAM_BASEADDR + SSRAM_MAX_SIZE))) || 
        (((uintptr_t)mask >= MSPI0_APERTURE_START_ADDR ) && 
        ( (uintptr_t)mask <= MSPI2_APERTURE_END_ADDR)), "Src buffer should be in SSRAM or PSRAM");    			
#else
        LV_ASSERT_MSG((((uintptr_t)mask >= SSRAM0_BASEADDR ) && 
        ( (uintptr_t)mask <= (SSRAM0_BASEADDR + NONTCM_MAX_SIZE))) || 
        (((uintptr_t)mask >= MSPI0_APERTURE_START_ADDR ) && 
        ( (uintptr_t)mask <= MSPI2_APERTURE_END_ADDR)), "Src buffer should be in SSRAM or PSRAM");    			
#endif
    }

#if defined(AM_PART_APOLLO5A)  || defined(AM_PART_APOLLO5B)
    LV_ASSERT_MSG((((uintptr_t)map_buf >= SSRAM_BASEADDR ) && 
    ( (uintptr_t)map_buf <= (SSRAM_BASEADDR + SSRAM_MAX_SIZE))) || 
    (((uintptr_t)map_buf >= MSPI0_APERTURE_START_ADDR ) && 
    ( (uintptr_t)map_buf <= MSPI2_APERTURE_END_ADDR)), "Src buffer should be in SSRAM or PSRAM");
#else
    LV_ASSERT_MSG((((uintptr_t)map_buf >= SSRAM0_BASEADDR ) && 
    ( (uintptr_t)map_buf <= (SSRAM0_BASEADDR + NONTCM_MAX_SIZE))) || 
    (((uintptr_t)map_buf >= MSPI0_APERTURE_START_ADDR ) && 
    ( (uintptr_t)map_buf <= MSPI2_APERTURE_END_ADDR)), "Src buffer should be in SSRAM or PSRAM");
#endif

    uint32_t gpu_ret = _lv_gpu_ambiq_nema_map(disp_area, disp_buf, draw_area, map_area, map_buf, opa, mask, mask_res);
    if(gpu_ret != NEMA_ERR_NO_ERROR)
    {
        LV_LOG_ERROR("GPU error: %s", lv_ambiq_gpu_error_interpret(gpu_ret));
        clear_current_cl();
        return LV_RES_INV;
    }

    lv_gpu_ambiq_nema_cl_submit_link(cl);

    if(mask_res == LV_DRAW_MASK_RES_CHANGED)
    {
        //Do the sync operation to prevent the mask content being updated.
        lv_gpu_ambiq_nema_wait(NULL);
    }

    return LV_RES_OK;
}

lv_res_t lv_gpu_ambiq_nema_gpu_only_fb(lv_disp_t* disp)
{
    img_obj_t * frame_buf = disp->driver->user_data;
    lv_res_t ret = LV_RES_INV;

    if(frame_buf != NULL)
    {
        nema_tex_format_t format = frame_buf->format;
        if(format !=  NEMA_BGRA8888)
        {
            ret = LV_RES_OK;
        }
    }

    return ret;
}

static inline nema_tex_format_t texture_color_format_map(lv_img_cf_t lvgl_cf)
{
    switch(lvgl_cf)
    {
        case LV_IMG_CF_AMBIQ_ARGB8888:
            return NEMA_ARGB8888;
        case LV_IMG_CF_AMBIQ_BGRA8888:
            return NEMA_BGRA8888;
        case LV_IMG_CF_AMBIQ_RGB565:
            return NEMA_RGB565;
        case LV_IMG_CF_AMBIQ_TSC6:
            return NEMA_TSC6;
        case LV_IMG_CF_AMBIQ_TSC6A:
            return NEMA_TSC6A;
        case LV_IMG_CF_AMBIQ_TSC4:
            return NEMA_TSC4;
        case LV_IMG_CF_AMBIQ_RGB24:
            return NEMA_RGB24;
        case LV_IMG_CF_AMBIQ_RGBA4444:
            return NEMA_RGBA4444;
        case LV_IMG_CF_TRUE_COLOR:
        case LV_IMG_CF_TRUE_COLOR_CHROMA_KEYED:
            return NEMA_BGR24;
        case LV_IMG_CF_TRUE_COLOR_ALPHA:
            return NEMA_BGRA8888;
        case LV_IMG_CF_INDEXED_1BIT:
            return NEMA_L1;
        case LV_IMG_CF_INDEXED_2BIT:
            return NEMA_L2;
        case LV_IMG_CF_INDEXED_4BIT:
            return NEMA_L4;
        case LV_IMG_CF_INDEXED_8BIT:
            return NEMA_L8;
        case LV_IMG_CF_ALPHA_1BIT:
            return NEMA_A1;
        case LV_IMG_CF_ALPHA_2BIT:
            return NEMA_A2;
        case LV_IMG_CF_ALPHA_4BIT:
            return NEMA_A4;
        case LV_IMG_CF_ALPHA_8BIT:
            return NEMA_A8;
        case LV_IMG_CF_AMBIQ_TSVG:
#if defined(AM_PART_APOLLO4P) || defined(AM_PART_BRONCO) || defined(AM_PART_APOLLO5A)  || defined(AM_PART_APOLLO5B)
            return COLOR_FORMAT_TSVG;
#else
            return COLOR_FORMAT_INVALID;
#endif
        default:
            return COLOR_FORMAT_INVALID;
    }
}

static inline nema_tex_format_t frame_buffer_color_format_map(lv_img_cf_t lvgl_cf)
{
    switch(lvgl_cf)
    {
        case LV_IMG_CF_AMBIQ_ARGB8888:
            return NEMA_ARGB8888;
        case LV_IMG_CF_AMBIQ_BGRA8888:
            return NEMA_BGRA8888;
        case LV_IMG_CF_AMBIQ_RGB565:
            return NEMA_RGB565;
        case LV_IMG_CF_AMBIQ_RGB24:
            return NEMA_RGB24;
        case LV_IMG_CF_TRUE_COLOR:
        case LV_IMG_CF_TRUE_COLOR_CHROMA_KEYED:
            return NEMA_BGR24;
        case LV_IMG_CF_TRUE_COLOR_ALPHA:
            return NEMA_BGRA8888;
        case LV_IMG_CF_ALPHA_1BIT:
            return NEMA_L1;
        case LV_IMG_CF_ALPHA_2BIT:
            return NEMA_L2;
        case LV_IMG_CF_ALPHA_4BIT:
            return NEMA_L4;
        case LV_IMG_CF_ALPHA_8BIT:
            return NEMA_L8;
        default:
            return COLOR_FORMAT_INVALID;
    }
}

lv_res_t lv_gpu_ambiq_nema_fb_support_format(lv_img_cf_t lvgl_cf)
{
    switch(lvgl_cf)
    {
        case LV_IMG_CF_AMBIQ_ARGB8888:
        case LV_IMG_CF_AMBIQ_BGRA8888:
        case LV_IMG_CF_AMBIQ_RGB565:
        case LV_IMG_CF_AMBIQ_RGB24:
        case LV_IMG_CF_TRUE_COLOR:
        case LV_IMG_CF_TRUE_COLOR_CHROMA_KEYED:
        case LV_IMG_CF_TRUE_COLOR_ALPHA:
        case LV_IMG_CF_ALPHA_1BIT:
        case LV_IMG_CF_ALPHA_2BIT:
        case LV_IMG_CF_ALPHA_4BIT:
        case LV_IMG_CF_ALPHA_8BIT:
            return LV_RES_OK;
        default:
            return LV_RES_INV;
    }
}

uint32_t lv_gpu_ambiq_nema_fb_size_needed(lv_img_cf_t cf, lv_coord_t w, lv_coord_t h)
{
    nema_tex_format_t nema_cf =  frame_buffer_color_format_map(cf);
    uint32_t size;
    if(nema_cf != COLOR_FORMAT_INVALID)
        size = nema_texture_size(nema_cf, 0, w, h);
    else
        size = 0;

    return size;
}

lv_res_t lv_gpu_ambiq_nema_fb_clear(void* fb)
{
    nema_cmdlist_t* cl = lv_gpu_ambiq_nema_cl_create_bind(fb, NULL, CL_SIZE_NORMAL_FILL, false, false);
    if(cl == NULL)
    {
        return LV_RES_INV;
    }

    nema_clear(0x0);

    lv_gpu_ambiq_nema_cl_submit_link(cl);

    return LV_RES_OK;
}

lv_res_t lv_gpu_ambiq_nema_fb_set(void* fb_s, lv_img_cf_t cf, lv_coord_t w, lv_coord_t h,
                                  void* buf, uint32_t buff_size)
{
    img_obj_t* frame_buffer = fb_s;

    nema_tex_format_t nema_cf = frame_buffer_color_format_map(cf);
    if(nema_cf == COLOR_FORMAT_INVALID)
    {
        return LV_RES_INV;
    }

    frame_buffer->bo.base_phys = (uintptr_t)buf;
    frame_buffer->bo.base_virt = (void*)buf;
    frame_buffer->bo.size = buff_size;
    frame_buffer->w = w;
    frame_buffer->h = h;
    frame_buffer->format = nema_cf;
    frame_buffer->stride = nema_stride_size(nema_cf, 0, w);

    return LV_RES_OK;
}

static lv_res_t texture_recolor(const lv_img_dsc_t * img_src, lv_img_dsc_t * img_recolor, nema_tex_format_t nema_cf, 
                                lv_color_t recolor, lv_opa_t recolor_opa)
{
    uint32_t w = img_src->header.w;
    uint32_t h = img_src->header.h;

    //Allocate memory to hold the texture after recolor
    uint32_t size = nema_texture_size(NEMA_BGRA8888, 0, w, h);
    void* buffer_ptr = lv_mem_ssram_alloc(size);
    if(buffer_ptr == NULL)
    {
        buffer_ptr = lv_mem_external_alloc(size);
    }

    if(buffer_ptr == NULL)
    {
        return LV_RES_INV;
    }

    // Des buffer should align to 64 bits
    LV_ASSERT_MSG(!((uintptr_t)buffer_ptr & 0x00000007), "Destination buffer should align to 64bits");

    // Set the header info of the recolored image
    img_recolor->header.w = w;
    img_recolor->header.h = h;
    img_recolor->header.cf = LV_IMG_CF_AMBIQ_BGRA8888;
    img_recolor->header.morton_order = 0;
    img_recolor->data_size = size;
    img_recolor->data = buffer_ptr;

    img_obj_t fb;
    fb.bo.base_phys = (uintptr_t)buffer_ptr;
    fb.bo.base_virt = (void*)buffer_ptr;
    fb.bo.size = size;
    fb.w = w;
    fb.h = h;
    fb.format = NEMA_BGRA8888;
    fb.stride = nema_stride_size(NEMA_BGRA8888, 0, w);

    //Create and bind.
    nema_cmdlist_t* cl = lv_gpu_ambiq_nema_cl_create_bind(&fb, NULL, CL_SIZE_RECOLOR, false, false);
    if(cl == NULL)
    {
        lv_mem_free(buffer_ptr);
        return LV_RES_INV;
    }

    uint32_t gpu_ret = _lv_gpu_ambiq_texture_recolor(img_src, img_recolor, nema_cf, recolor, recolor_opa);
    if(gpu_ret != NEMA_ERR_NO_ERROR)
    {
        LV_LOG_ERROR("GPU error: %s", lv_ambiq_gpu_error_interpret(gpu_ret));
        lv_mem_free(buffer_ptr);
        clear_current_cl();
        return LV_RES_INV;  
    }

    //submit the cl
    lv_gpu_ambiq_nema_cl_submit_link(cl);

    //Do the sync operation.
    nema_cl_wait(CLManage.cl_head);

    return LV_RES_OK;
}

lv_res_t lv_gpu_ambiq_nema_blit(const lv_area_t * coords,
                            const lv_area_t * clip_area,
                            const void * src,
                            const lv_draw_img_dsc_t * dsc)
{
    lv_res_t ret = LV_RES_OK;
    uint32_t gpu_ret = 0;
    nema_tex_format_t nema_cf;
    lv_img_dsc_t img_recolor;
    lv_img_dsc_t * img_src = NULL;

    //Only support viariable type img_src
    if(lv_img_src_get_type(src) != LV_IMG_SRC_VARIABLE)
    {
        LV_LOG_WARN("GPU failed, LV_IMG_SRC_FILE is used!");
        ret = LV_RES_INV;
        goto exit;
    }

    img_src = (lv_img_dsc_t *)src;
    nema_cf = texture_color_format_map(img_src->header.cf);

    if(nema_cf == COLOR_FORMAT_INVALID)
    {
        LV_LOG_WARN("GPU failed, not supported color format!");
        ret = LV_RES_INV;
        goto exit;
    }

    if(lv_gpu_ambiq_nema_check_common(clip_area, dsc->blend_mode) != LV_RES_OK)
    {
        ret = LV_RES_INV;
        goto exit;
    }

#if defined(AM_PART_APOLLO5A)  || defined(AM_PART_APOLLO5B)
    // Check source location
    LV_ASSERT_MSG((((uintptr_t)img_src->data >= SSRAM_BASEADDR ) && 
    ( (uintptr_t)img_src->data <= (SSRAM_BASEADDR + SSRAM_MAX_SIZE))) || 
    (((uintptr_t)img_src->data >= MSPI0_APERTURE_START_ADDR ) && 
    ( (uintptr_t)img_src->data <= MSPI2_APERTURE_END_ADDR)), "Src buffer should be in SSRAM or PSRAM");
#else
    // Check source location
    LV_ASSERT_MSG((((uintptr_t)img_src->data >= SSRAM0_BASEADDR ) && 
    ( (uintptr_t)img_src->data <= (SSRAM0_BASEADDR + NONTCM_MAX_SIZE))) || 
    (((uintptr_t)img_src->data >= MSPI0_APERTURE_START_ADDR ) && 
    ( (uintptr_t)img_src->data <= MSPI2_APERTURE_END_ADDR)), "Src buffer should be in SSRAM or PSRAM");
#endif

    //Recolor
    if(dsc->recolor_opa != LV_OPA_TRANSP)
    {
        if(texture_recolor(img_src, &img_recolor, nema_cf, dsc->recolor, dsc->recolor_opa) != LV_RES_OK)
        {
            LV_LOG_WARN("GPU failed, image recolor failed!");
            ret = LV_RES_INV;
            goto exit;
        }

        img_src = &img_recolor;
        nema_cf = NEMA_BGRA8888;
    }

    nema_cmdlist_t* cl;
    if(nema_cf == COLOR_FORMAT_TSVG)
    {
        cl = lv_gpu_ambiq_nema_cl_create_bind(NULL, clip_area, CL_SIZE_TSVG, true, true);
    }
    else
    {
        cl = lv_gpu_ambiq_nema_cl_create_bind(NULL, clip_area, CL_SIZE_BLIT, false, true);
    }

    if(cl == NULL)
    {
        ret = LV_RES_INV;
        goto clean_exit;
    }

    gpu_ret = _lv_gpu_ambiq_nema_blit(coords, clip_area, img_src, dsc, nema_cf);
    if(gpu_ret != NEMA_ERR_NO_ERROR)
    {
        LV_LOG_ERROR("GPU error: %s", lv_ambiq_gpu_error_interpret(gpu_ret));
        ret = LV_RES_INV;
        clear_current_cl();
        goto clean_exit;
    }

    //submit the cl
    lv_gpu_ambiq_nema_cl_submit_link(cl);

#if LV_AMBIQ_NEMA_IMAGE_SYNC
    //Do the sync operation to prevent the image content being updated.
    nema_cl_wait(CLManage.cl_head);
#endif

    if(dsc->recolor_opa != LV_OPA_TRANSP)
    {
        //Wait for CL complete if recolor is used.
        nema_cl_wait(CLManage.cl_head);
        LV_ASSERT(nema_get_error() == NEMA_ERR_NO_ERROR);
    }

clean_exit:
    if(dsc->recolor_opa != LV_OPA_TRANSP)
    {
        //Release the recolor temporary buffer
        lv_mem_free((void*)img_src->data);
    }

exit:
    return ret;
}

lv_res_t lv_gpu_ambiq_nema_draw_label_pre(const lv_area_t * clip_area, lv_blend_mode_t blend_mode)
{
    LV_ASSERT(pLabelCL == NULL);

    lv_res_t check_ret = lv_gpu_ambiq_nema_check_common(clip_area, blend_mode);
    if(check_ret != LV_RES_OK)
    {
        return LV_RES_INV;
    }

    pLabelCL = lv_gpu_ambiq_nema_cl_create_bind(NULL, clip_area, CL_SIZE_LABEL, false, true);
    if(pLabelCL == NULL)
    {
        return LV_RES_INV;
    }

    return LV_RES_OK;
}

void lv_gpu_ambiq_nema_draw_label_post(void)
{
    if(pLabelCL != NULL)
    {
        lv_gpu_ambiq_nema_cl_submit_link(pLabelCL);

        pLabelCL = NULL;
    }
}

lv_res_t lv_gpu_ambiq_nema_draw_letter_pre(const lv_area_t * clip_area, lv_blend_mode_t blend_mode)
{
    LV_ASSERT(pLabelCL == NULL);

    lv_res_t check_ret = lv_gpu_ambiq_nema_check_common(clip_area, blend_mode);
    if(check_ret != LV_RES_OK)
    {
        return LV_RES_INV;
    }

    pLabelCL = lv_gpu_ambiq_nema_cl_create_bind(NULL, clip_area, CL_SIZE_LETTER, false, true);
    if(pLabelCL == NULL)
    {
        return LV_RES_INV;
    }

    return LV_RES_OK;
}

void lv_gpu_ambiq_nema_draw_letter_post(void)
{
    if(pLabelCL != NULL)
    {
        lv_gpu_ambiq_nema_cl_submit_link(pLabelCL);

        pLabelCL = NULL;
    }
}

lv_res_t lv_gpu_ambiq_nema_draw_letter(lv_coord_t pos_x, lv_coord_t pos_y, lv_font_glyph_dsc_t * g,
                                             const lv_area_t * clip_area,
                                             const uint8_t * map_p, lv_color_t color, lv_opa_t opa, lv_blend_mode_t blend_mode)
{
    //pLabelCL must have been set.
    if(pLabelCL == NULL)
    {
        return LV_RES_INV;
    }

    //Check CL left space
    if((pLabelCL->size - pLabelCL->offset) < CHAR_CMD_NUMBER*2)
    {
        //Submit and link the full cl.
        lv_gpu_ambiq_nema_cl_submit_link(pLabelCL);

        //Create a new empty one.
        pLabelCL = lv_gpu_ambiq_nema_cl_create_bind(NULL, clip_area, CL_SIZE_LABEL, false, true);
        if(pLabelCL == NULL)
        {
            return LV_RES_INV;
        }
    }

    //bind the CL
    if(nema_cl_get_bound() != pLabelCL)
        nema_cl_bind(pLabelCL);

    LV_ASSERT(nema_get_error() == NEMA_ERR_NO_ERROR);

#if defined(AM_PART_APOLLO5A)  || defined(AM_PART_APOLLO5B)
    //src buffer should be allocated from SSRAM or PSRAM
    LV_ASSERT_MSG((((uintptr_t)map_p >= SSRAM_BASEADDR ) && 
    ( (uintptr_t)map_p <= (SSRAM_BASEADDR + SSRAM_MAX_SIZE))) || 
    (((uintptr_t)map_p >= MSPI0_APERTURE_START_ADDR ) && 
    ( (uintptr_t)map_p <= MSPI2_APERTURE_END_ADDR)), "Src buffer should be in SSRAM or PSRAM");
#else
    //src buffer should be allocated from SSRAM or PSRAM
    LV_ASSERT_MSG((((uintptr_t)map_p >= SSRAM0_BASEADDR ) && 
    ( (uintptr_t)map_p <= (SSRAM0_BASEADDR + NONTCM_MAX_SIZE))) || 
    (((uintptr_t)map_p >= MSPI0_APERTURE_START_ADDR ) && 
    ( (uintptr_t)map_p <= MSPI2_APERTURE_END_ADDR)), "Src buffer should be in SSRAM or PSRAM");
#endif

    uint32_t gpu_ret = _lv_gpu_ambiq_nema_draw_letter(pos_x, pos_y, g, clip_area, map_p, color, opa, blend_mode);
    if(gpu_ret != NEMA_ERR_NO_ERROR)
    {
        LV_LOG_ERROR("GPU error: %s", lv_ambiq_gpu_error_interpret(gpu_ret));
        clear_current_cl();
        pLabelCL = NULL;
        return LV_RES_INV;
    }

    //Unbind current cl
    nema_cl_unbind();
    LV_ASSERT(nema_get_error() == NEMA_ERR_NO_ERROR);

    return LV_RES_OK;
}

lv_res_t lv_gpu_ambiq_nema_draw_border(const lv_area_t * clip_area, const lv_area_t * outer_area, const lv_area_t * inner_area,
                         lv_coord_t rout, lv_coord_t rin, const lv_draw_rect_dsc_t * dsc)
{
    lv_res_t check_ret = lv_gpu_ambiq_nema_check_common(outer_area, dsc->blend_mode);
    if(check_ret != LV_RES_OK)
    {
        return LV_RES_INV;
    }

    if((rout == 0) || (rin == 0))
    {
        return lv_gpu_ambiq_nema_draw_border_rec(clip_area, outer_area, inner_area,
                                             dsc->border_color, dsc->border_opa, dsc->blend_mode);
    }
    else if((dsc->border_side & LV_BORDER_SIDE_FULL) == LV_BORDER_SIDE_FULL)
    {
        return lv_gpu_ambiq_nema_draw_border_rounded(clip_area, outer_area, inner_area, rout, rin,
                                            dsc->border_color, dsc->border_opa, dsc->blend_mode);
    }
    else
    {
        LV_LOG_WARN("GPU failed, only LV_BORDER_SIDE_FULL supported!");
        return LV_RES_INV;
    }
}

lv_res_t lv_gpu_ambiq_nema_draw_outline(const lv_area_t * clip_area, const lv_area_t * outer_area, const lv_area_t * inner_area,
                         lv_coord_t rout, lv_coord_t rin, const lv_draw_rect_dsc_t * dsc)
{
    lv_res_t check_ret = lv_gpu_ambiq_nema_check_common(outer_area, dsc->blend_mode);
    if(check_ret != LV_RES_OK)
    {
        return LV_RES_INV;
    }

    if((rout == 0) || (rin == 0))
    {
        lv_gpu_ambiq_nema_draw_border_rec(clip_area, outer_area, inner_area,
                                             dsc->outline_color, dsc->outline_opa, dsc->blend_mode);
    }
    else
    {
        lv_gpu_ambiq_nema_draw_border_rounded(clip_area, outer_area, inner_area, rout, rin,
                                            dsc->outline_color, dsc->outline_opa, dsc->blend_mode);
    }
    return LV_RES_OK;
}

static lv_res_t lv_gpu_ambiq_nema_draw_border_rounded(const lv_area_t * clip_area, const lv_area_t * outer_area, const lv_area_t * inner_area,
                         lv_coord_t rout, lv_coord_t rin, lv_color_t color, lv_opa_t opa, lv_blend_mode_t blend_mode)
{
#ifdef LV_GPU_AMBIQ_DEBUG_CL
    nema_cmdlist_t* cl = lv_gpu_ambiq_nema_cl_create_bind(NULL, clip_area, CL_SIZE_ROUNDED_RECT, false, true);
#else
    nema_cmdlist_t* cl = lv_gpu_ambiq_nema_cl_create_bind(NULL, clip_area, CL_SIZE_ROUNDED_RECT, true, true);
#endif
    if(cl == NULL)
    {
        return LV_RES_INV;
    }

    uint32_t gpu_ret = _lv_gpu_ambiq_nema_draw_border_rounded(clip_area, outer_area, inner_area, rout, rin, color, opa, blend_mode);
    if(gpu_ret != NEMA_ERR_NO_ERROR)
    {
        LV_LOG_ERROR("GPU error: %s", lv_ambiq_gpu_error_interpret(gpu_ret));
        clear_current_cl();
        return LV_RES_INV;
    }

    //Submit and link.
    lv_gpu_ambiq_nema_cl_submit_link(cl);

    return LV_RES_OK;
}

static lv_res_t lv_gpu_ambiq_nema_draw_border_rec(const lv_area_t * clip_area, const lv_area_t * outer_area, const lv_area_t * inner_area,
        lv_color_t color, lv_opa_t opa, lv_blend_mode_t blend_mode)
{
    nema_cmdlist_t* cl = lv_gpu_ambiq_nema_cl_create_bind(NULL, clip_area, CL_SIZE_BOARDER, false, true);
    if(cl == NULL)
    {
        return LV_RES_INV;
    }

    uint32_t gpu_ret = _lv_gpu_ambiq_nema_draw_border_rec(clip_area, outer_area, inner_area, color, opa, blend_mode);
    if(gpu_ret != NEMA_ERR_NO_ERROR)
    {
        LV_LOG_ERROR("GPU error: %s", lv_ambiq_gpu_error_interpret(gpu_ret));
        clear_current_cl();
        return LV_RES_INV;
    }

    //Submit and link.
    lv_gpu_ambiq_nema_cl_submit_link(cl);

    return LV_RES_OK;
}

lv_res_t lv_gpu_ambiq_nema_draw_bg(const lv_area_t * clip_area, const lv_area_t * draw_area,
                         lv_coord_t rout, lv_color_t color, lv_opa_t opa, lv_blend_mode_t blend_mode)
{
    //Underlying mask has already handled by upper layer, no need to check it here.
    lv_res_t check_ret = lv_gpu_ambiq_nema_check_common(NULL, blend_mode);
    if(check_ret != LV_RES_OK)
    {
        return LV_RES_INV;
    }

    nema_cmdlist_t* cl;

    if(rout != 0)
    {
#ifdef LV_GPU_AMBIQ_DEBUG_CL
        cl = lv_gpu_ambiq_nema_cl_create_bind(NULL, clip_area, CL_SIZE_ROUNDED_RECT, false, true);
#else
        cl = lv_gpu_ambiq_nema_cl_create_bind(NULL, clip_area, CL_SIZE_ROUNDED_RECT, true, true);
#endif
    }
    else
    {
        cl = lv_gpu_ambiq_nema_cl_create_bind(NULL, clip_area, CL_SIZE_NORMAL_FILL, false, true);
    }

    if(cl == NULL)
    {
        return LV_RES_INV;
    }

    uint32_t gpu_ret = _lv_gpu_ambiq_nema_draw_bg(clip_area, draw_area, rout, color, opa, blend_mode);
    if(gpu_ret != NEMA_ERR_NO_ERROR)
    {
        LV_LOG_ERROR("GPU error: %s", lv_ambiq_gpu_error_interpret(gpu_ret));
        clear_current_cl();
        return LV_RES_INV;
    }

    //Submit and link.
    lv_gpu_ambiq_nema_cl_submit_link(cl);

    return LV_RES_OK;
}

lv_res_t lv_gpu_ambiq_nema_draw_bg_img(const lv_area_t * coords, const lv_area_t * mask,
                                       const lv_draw_rect_dsc_t * dsc_rect,
                                       const lv_draw_img_dsc_t * dsc_img)
{
    lv_img_dsc_t * img_src = (lv_img_dsc_t *)dsc_rect->bg_img_src;

    //Only support viariable type img_src
    if(lv_img_src_get_type(img_src) != LV_IMG_SRC_VARIABLE)
    {
        LV_LOG_WARN("GPU failed, LV_IMG_SRC_FILE is used!");
        return LV_RES_INV;
    }

    //Check color format
    nema_tex_format_t nema_cf = texture_color_format_map(img_src->header.cf);
    if(nema_cf == COLOR_FORMAT_INVALID)
    {
        LV_LOG_WARN("GPU failed, not supported color format!");
        return LV_RES_INV;
    }

    if(dsc_rect->bg_img_tiled == true)
    {
        //For tiled image background, the source texture width and hight must be a power of two.
        uint32_t w = img_src->header.w;
        uint32_t h = img_src->header.h;
        if(((w & (w-1)) != 0) || ((h & (h-1))!=0))
        {
            LV_LOG_WARN("GPU failed, tiled background image is not accelerated!");
            return LV_RES_INV;
        }
    }

#if defined(AM_PART_APOLLO5A)  || defined(AM_PART_APOLLO5B)
    //src buffer should be allocated from SSRAM or PSRAM
    LV_ASSERT_MSG((((uintptr_t)img_src->data >= SSRAM_BASEADDR ) && 
    ( (uintptr_t)img_src->data <= (SSRAM_BASEADDR + SSRAM_MAX_SIZE))) || 
    (((uintptr_t)img_src->data >= MSPI0_APERTURE_START_ADDR ) && 
    ( (uintptr_t)img_src->data <= MSPI2_APERTURE_END_ADDR)), "Src buffer should be in SSRAM or PSRAM");
#else
    //src buffer should be allocated from SSRAM or PSRAM
    LV_ASSERT_MSG((((uintptr_t)img_src->data >= SSRAM0_BASEADDR ) && 
    ( (uintptr_t)img_src->data <= (SSRAM0_BASEADDR + NONTCM_MAX_SIZE))) || 
    (((uintptr_t)img_src->data >= MSPI0_APERTURE_START_ADDR ) && 
    ( (uintptr_t)img_src->data <= MSPI2_APERTURE_END_ADDR)), "Src buffer should be in SSRAM or PSRAM");
#endif

    lv_area_t coords_bg;
    lv_area_copy(&coords_bg, coords);

    /*If the border fully covers make the bg area 1px smaller to avoid artifacts on the corners*/
    if(dsc_rect->border_width > 1 && dsc_rect->border_opa >= LV_OPA_MAX && dsc_rect->radius != 0)
    {
        coords_bg.x1 += (dsc_rect->border_side & LV_BORDER_SIDE_LEFT) ? 1 : 0;
        coords_bg.y1 += (dsc_rect->border_side & LV_BORDER_SIDE_TOP) ? 1 : 0;
        coords_bg.x2 -= (dsc_rect->border_side & LV_BORDER_SIDE_RIGHT) ? 1 : 0;
        coords_bg.y2 -= (dsc_rect->border_side & LV_BORDER_SIDE_BOTTOM) ? 1 : 0;
    }

    /*Get clipped fill area which is the real draw area.
     *It is always the same or inside `fill_area`*/
    lv_area_t draw_area;
    if(!_lv_area_intersect(&draw_area, &coords_bg, mask)) 
        return LV_RES_OK;

    /*Check common*/
    lv_res_t check_ret = lv_gpu_ambiq_nema_check_common(&coords_bg, dsc_rect->blend_mode);
    if(check_ret != LV_RES_OK)
    {
        return LV_RES_INV;
    }

    /*Get the real radius. Can't be larger than the half of the shortest side */
    lv_coord_t coords_w = lv_area_get_width(&coords_bg);
    lv_coord_t coords_h = lv_area_get_height(&coords_bg);
    int32_t short_side = LV_MIN(coords_w, coords_h);
    int32_t rout = LV_MIN(dsc_rect->radius, short_side >> 1);

    nema_cmdlist_t* cl;
    if(rout == 0)
    {
        cl = lv_gpu_ambiq_nema_cl_create_bind(NULL, &draw_area, CL_SIZE_BLIT, false, true);
    }
    else
    {
#ifdef LV_GPU_AMBIQ_DEBUG_CL
        cl = lv_gpu_ambiq_nema_cl_create_bind(NULL, &draw_area, CL_SIZE_ROUNDED_RECT, false, true);
#else
        cl = lv_gpu_ambiq_nema_cl_create_bind(NULL, &draw_area, CL_SIZE_ROUNDED_RECT, true, true);
#endif
    }

    if(cl == NULL)
    {
        return LV_RES_INV;
    }

    uint32_t gpu_ret = _lv_gpu_ambiq_nema_draw_bg_img(&coords_bg, mask, dsc_rect, dsc_img, nema_cf);
    if(gpu_ret != NEMA_ERR_NO_ERROR)
    {
        LV_LOG_ERROR("GPU error: %s", lv_ambiq_gpu_error_interpret(gpu_ret));
        clear_current_cl();
        return LV_RES_INV;
    }

    //submit the cl
    lv_gpu_ambiq_nema_cl_submit_link(cl);

    return LV_RES_OK;
}

lv_res_t lv_gpu_ambiq_nema_draw_line(const lv_point_t * point1, const lv_point_t * point2, const lv_area_t * clip,
                                        const lv_draw_line_dsc_t * dsc)
{
    lv_res_t check_ret = lv_gpu_ambiq_nema_check_common(clip, dsc->blend_mode);
    if(check_ret != LV_RES_OK)
    {
        return LV_RES_INV;
    }

    bool dashed = dsc->dash_gap && dsc->dash_width ? true : false;

    if(!dashed)
    {
        return lv_gpu_ambiq_nema_draw_line_normal(point1, point2, clip, dsc);
    }
    else
    {
        return lv_gpu_ambiq_nema_draw_line_dashed(point1, point2, clip, dsc);
    }
}

static lv_res_t lv_gpu_ambiq_nema_draw_line_normal(const lv_point_t * point1,
                                        const lv_point_t * point2,
                                        const lv_area_t * clip,
                                        const lv_draw_line_dsc_t * dsc)
{
    nema_cmdlist_t* cl = lv_gpu_ambiq_nema_cl_create_bind(NULL, clip, CL_SIZE_LINE_NORMAL, false, true);
    if(cl == NULL)
    {
        return LV_RES_INV;
    }

    uint32_t gpu_ret = _lv_gpu_ambiq_nema_draw_line_normal(point1, point2, dsc);
    if(gpu_ret != NEMA_ERR_NO_ERROR)
    {
        LV_LOG_ERROR("GPU error: %s", lv_ambiq_gpu_error_interpret(gpu_ret));
        clear_current_cl();
        return LV_RES_INV;
    }

    lv_gpu_ambiq_nema_cl_submit_link(cl);

    return LV_RES_OK;
}

static lv_res_t lv_gpu_ambiq_nema_draw_line_dashed(const lv_point_t * point1, const lv_point_t * point2, const lv_area_t * clip,
                                        const lv_draw_line_dsc_t * dsc)
{


    lv_disp_t * disp = _lv_refr_get_disp_refreshing();
    lv_disp_draw_buf_t * draw_buf = lv_disp_get_draw_buf(disp);
    const lv_area_t * disp_area = &draw_buf->area;

    float line_coordinate[4];

    //Get the relative coordinate of the line points.
    line_coordinate[0] = (float)point1->x - disp_area->x1;
    line_coordinate[1] = (float)point1->y - disp_area->y1;
    line_coordinate[2] = (float)point2->x - disp_area->x1;
    line_coordinate[3] = (float)point2->y - disp_area->y1;

    float angle;

    //From point 0 to point 1
    if(point2->y == point1->y)
    {
        angle = 0.0f;

        if(point1->x > point2->x)
        {
            angle = 180.0f;
        }
    }
    else if(point2->x == point1->x)
    {
        angle = 90.0f;

        if(point1->y > point2->y)
        {
            angle = -90.0f;
        }
    }
    else
    {
        float diff_X = (float)point2->x - (float)point1->x;
        float diff_Y = (float)point2->y - (float)point1->y;
        angle = nema_atan(diff_Y/diff_X);
    }

    nema_cmdlist_t* cl;
    bool run_again;
    uint32_t gpu_ret;

    while(1)
    {
        cl = lv_gpu_ambiq_nema_cl_create_bind(NULL, clip, CL_SIZE_LINE, false, true);
        if(cl == NULL)
        {
            return LV_RES_INV;
        }

        gpu_ret = _lv_gpu_ambiq_nema_draw_line_dashed(line_coordinate,
                                                    dsc,
                                                    &run_again,
                                                    angle);

        //If we encounter an error, stop here
        if(gpu_ret != NEMA_ERR_NO_ERROR)
        {
            LV_LOG_ERROR("GPU error: %s", lv_ambiq_gpu_error_interpret(gpu_ret));
            clear_current_cl();
            return LV_RES_INV;
        }
        else
        {
            lv_gpu_ambiq_nema_cl_submit_link(cl);
        }

        //If there's not enough space, need to create another cl
        if(!run_again)
        {
            break;
        }
    }

    return LV_RES_OK;
}

static lv_res_t lv_gpu_ambiq_nema_draw_triangle(const lv_point_t points[], const lv_area_t * clip_area, const lv_draw_rect_dsc_t * draw_dsc)
{
    nema_cmdlist_t* cl =  lv_gpu_ambiq_nema_cl_create_bind(NULL, clip_area, CL_SIZE_DEFAULT, false, true);
    if(cl == NULL)
    {
        return LV_RES_INV;
    }

    uint32_t gpu_ret = _lv_gpu_ambiq_nema_draw_triangle(points, clip_area, draw_dsc);
    if(gpu_ret != NEMA_ERR_NO_ERROR)
    {
        LV_LOG_ERROR("GPU error: %s", lv_ambiq_gpu_error_interpret(gpu_ret));
        clear_current_cl();
        return LV_RES_INV;
    }

    lv_gpu_ambiq_nema_cl_submit_link(cl);

    return LV_RES_OK;
}

static lv_res_t lv_gpu_ambiq_nema_draw_quadrangle(const lv_point_t points[], const lv_area_t * clip_area, const lv_draw_rect_dsc_t * draw_dsc)
{
    nema_cmdlist_t* cl =  lv_gpu_ambiq_nema_cl_create_bind(NULL, clip_area, CL_SIZE_DEFAULT, false, true);
    if(cl == NULL)
    {
        return LV_RES_INV;
    }

    uint32_t gpu_ret = _lv_gpu_ambiq_nema_draw_quadrangle(points, clip_area, draw_dsc);
    if(gpu_ret != NEMA_ERR_NO_ERROR)
    {
        LV_LOG_ERROR("GPU error: %s", lv_ambiq_gpu_error_interpret(gpu_ret));
        clear_current_cl();
        return LV_RES_INV;
    }

    lv_gpu_ambiq_nema_cl_submit_link(cl);

    return LV_RES_OK;
}

lv_res_t lv_gpu_ambiq_nema_draw_polygon(const lv_point_t points[],
                                    uint16_t point_cnt,
                                    const lv_area_t * clip_area,
                                    const lv_draw_rect_dsc_t * draw_dsc)
{
    //Underlying mask is checked in the upper layer, no need to check it here.
    lv_res_t check_ret = lv_gpu_ambiq_nema_check_common(clip_area, draw_dsc->blend_mode);
    if(check_ret != LV_RES_OK)
    {
        return LV_RES_INV;
    }

    //Only triangle or rectangle
    if( (point_cnt != 3) && (point_cnt != 4) )
    {
        return LV_RES_INV;
    }

    //No rounded corner
    if( (draw_dsc->radius) != 0)
    {
        return LV_RES_INV;
    }

    lv_res_t ret;

    //No gradient
    if(draw_dsc->bg_grad_dir != LV_GRAD_DIR_NONE)
    {
        return LV_RES_INV;
    }

    //No image background
    if(draw_dsc->bg_img_src != NULL)
    {
        return LV_RES_INV;
    }

    //No boarder, this is hard to support and not commonly used.
    if(draw_dsc->border_side != LV_BORDER_SIDE_NONE)
    {
        return LV_RES_INV;
    }

    //No shadow, this is hard to support and not commonly used.
    if(draw_dsc->shadow_width != 0)
    {
        return LV_RES_INV;
    }

    //No outline, this is hard to support and not commonly used.
    if(draw_dsc->outline_width != 0)
    {
        return LV_RES_INV;
    }

    //For quadrangle, we support pure color fill.
    //TODO: support gradient color background
    //TODO: support image background
    if(point_cnt == 4)
    {
        ret = lv_gpu_ambiq_nema_draw_quadrangle(points, clip_area, draw_dsc);
        return ret;
    }

    //For triangle, we only support pure color fill operation
    //TODO: support gradient color background
    //TODO: support image background
    if(point_cnt == 3)
    {
        ret = lv_gpu_ambiq_nema_draw_triangle(points, clip_area, draw_dsc);
        return ret;
    }

    //Should never got here.
    return LV_RES_INV;
}

lv_res_t lv_gpu_ambiq_nema_draw_arc(lv_coord_t center_x, lv_coord_t center_y,
                                    uint16_t radius, uint16_t start_angle,
                                    uint16_t end_angle, const lv_area_t * clip_area,
                                    const lv_draw_arc_dsc_t * dsc)
{

    lv_res_t check_ret = lv_gpu_ambiq_nema_check_common(clip_area, dsc->blend_mode);
    if(check_ret != LV_RES_OK)
    {
        return LV_RES_INV;
    }

    lv_img_header_t bg_img_header;
    nema_tex_format_t bg_img_nema_cf = NEMA_BGRA8888;

    //If we use the background image, run the image format check.
    if(dsc->img_src)
    {
        const lv_img_dsc_t* image = dsc->img_src;

        lv_img_src_t src_type = lv_img_src_get_type(dsc->img_src);

        if(src_type == LV_IMG_SRC_VARIABLE)
        {

            lv_res_t res = lv_img_decoder_get_info(dsc->img_src, &bg_img_header);
            if(res != LV_RES_OK) 
            {
                LV_LOG_WARN("Couldn't read the background image");
                return LV_RES_INV;
            }

            bg_img_nema_cf = texture_color_format_map(bg_img_header.cf);

            if(bg_img_nema_cf == COLOR_FORMAT_INVALID)
            {
                LV_LOG_WARN("GPU failed, not supported color format!");
                return LV_RES_INV;
            }
        }
        //We can support SYMBOL, but there is better way to support this.
        // else if (src_type == LV_IMG_SRC_SYMBOL) 
        // {

        // }
        else
        {
            LV_LOG_WARN("GPU failed, only LV_IMG_SRC_VARIABLE can be used as the background image! ");
            return LV_RES_INV;
        }

#if defined(AM_PART_APOLLO5A)  || defined(AM_PART_APOLLO5B)
        //src buffer should be allocated from SSRAM or PSRAM
        LV_ASSERT_MSG((((uintptr_t)image->data >= SSRAM_BASEADDR ) && 
        ( (uintptr_t)image->data <= (SSRAM_BASEADDR + SSRAM_MAX_SIZE))) || 
        (((uintptr_t)image->data >= MSPI0_APERTURE_START_ADDR ) && 
        ( (uintptr_t)image->data <= MSPI2_APERTURE_END_ADDR)), "Src buffer should be in SSRAM or PSRAM");       
#else
        //src buffer should be allocated from SSRAM or PSRAM
        LV_ASSERT_MSG((((uintptr_t)image->data >= SSRAM0_BASEADDR ) && 
        ( (uintptr_t)image->data <= (SSRAM0_BASEADDR + NONTCM_MAX_SIZE))) || 
        (((uintptr_t)image->data >= MSPI0_APERTURE_START_ADDR ) && 
        ( (uintptr_t)image->data <= MSPI2_APERTURE_END_ADDR)), "Src buffer should be in SSRAM or PSRAM");       
#endif
    }

#ifdef LV_GPU_AMBIQ_DEBUG_CL
    nema_cmdlist_t* cl = lv_gpu_ambiq_nema_cl_create_bind(NULL, clip_area, CL_SIZE_ARC, false, true);
#else
    nema_cmdlist_t* cl = lv_gpu_ambiq_nema_cl_create_bind(NULL, clip_area, CL_SIZE_ARC, true, true);
#endif
    if(cl == NULL)
    {
        return LV_RES_INV;
    }

    uint32_t gpu_ret = _lv_gpu_ambiq_nema_draw_arc(center_x, center_y, radius, start_angle, end_angle, clip_area, dsc, bg_img_header, bg_img_nema_cf);
    if(gpu_ret != NEMA_ERR_NO_ERROR)
    {
        LV_LOG_ERROR("GPU error: %s", lv_ambiq_gpu_error_interpret(gpu_ret));
        clear_current_cl();
        return LV_RES_INV;
    }

    lv_gpu_ambiq_nema_cl_submit_link(cl);

    return LV_RES_OK;
}

