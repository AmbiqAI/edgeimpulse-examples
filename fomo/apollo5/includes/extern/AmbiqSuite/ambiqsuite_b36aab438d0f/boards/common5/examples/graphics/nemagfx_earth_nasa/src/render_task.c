//*****************************************************************************
//
//! @file render_task.c
//!
//! @brief Task to render frame buffer
//! The task starts the display task after the rendering is complete.
//!
//!
//*****************************************************************************

//*****************************************************************************
//
// ${copyright}
//
// This is part of revision ${version} of the AmbiqSuite Development Package.
//
//*****************************************************************************

//*****************************************************************************
//
// Global includes for this project.
//
//*****************************************************************************
#include "nemagfx_earth_nasa.h"
#include "render_task.h"
#include "display_task.h"
#include "event_task.h"
#include "rtos.h"
#include "nema_core.h"
#include "nema_utils.h"
#include "nema_graphics.h"
#include "nema_matrix4x4.h"
#include "earth_nasa.h"
#include "earth_nasa_rgb24.h"

#ifdef GPU_FORMAT_RGB888
#define FB_COLOR_FORMAT (NEMA_RGB24)
#else
#define FB_COLOR_FORMAT (NEMA_RGB565)
#endif

//*****************************************************************************
//
// Render task handle.
//
//*****************************************************************************
TaskHandle_t RenderTaskHandle;


//*****************************************************************************
//
// earth nasa assert.
//
//*****************************************************************************
img_obj_t earth_img = {
    .bo             = {0},
    .w              = 1024,
    .h              = 1024,
    .stride         = 1024*3,
    .color          = 0,
    .format         = NEMA_RGB24,
    .sampling_mode  = NEMA_FILTER_BL
};
//*****************************************************************************
//
// Frame buffer.
//
//*****************************************************************************
img_obj_t g_sFrameBuffer[FRAME_BUFFERS] = 
{
    {.bo = {0}, .w = RESX, .h=RESY, .stride=-1, .color=0, .format=FB_COLOR_FORMAT, .sampling_mode = 0},
#if (FRAME_BUFFERS > 1)
    {.bo = {0}, .w = RESX, .h=RESY, .stride=-1, .color=0, .format=FB_COLOR_FORMAT, .sampling_mode = 0}
#endif
};

//*****************************************************************************
//
// Config buffers locate in SSRAM as non-cacheable.
//
//*****************************************************************************
static am_hal_mpu_attr_t sMPUAttr =
{
    .ui8AttrIndex = 0,
    .bNormalMem = true,
    .sOuterAttr = {.bNonTransient = false, .bWriteBack = true, .bReadAllocate = false, .bWriteAllocate = false},
    .sInnerAttr = {.bNonTransient = false, .bWriteBack = true, .bReadAllocate = false, .bWriteAllocate = false},
    .eDeviceAttr = 0
};

static am_hal_mpu_region_config_t sMPUCfg[] =
{
    // buffer for the event task
    {
        .ui32RegionNumber = 0,
        .ui32BaseAddress = (uint32_t)DMATCBBuffer,
        .eShareable = NON_SHARE,
        .eAccessPermission = RW_NONPRIV,
        .bExecuteNever = true,
        .ui32LimitAddress = (uint32_t)DMATCBBuffer + DMABUFFSIZE * 4 - 1,
        .ui32AttrIndex = 0,
        .bEnable = true
    },
    // buffer for the render task, the addresses will reconfigure dynamically.
    {
        .ui32RegionNumber = 0,
        .ui32BaseAddress = (uint32_t)DMATCBBuffer,
        .eShareable = NON_SHARE,
        .eAccessPermission = RW_NONPRIV,
        .bExecuteNever = true,
        .ui32LimitAddress = (uint32_t)DMATCBBuffer + DMABUFFSIZE * 4 - 1,
        .ui32AttrIndex = 0,
        .bEnable = true
    }
};
//*****************************************************************************
//
//! @brief Framebuffer init
//!
//! Allocated frame buffer.
//!
//! @return -1:failed, 0:OK..
//
//*****************************************************************************
int
framebuffer_create(img_obj_t* image)
{
    uint32_t size = nema_texture_size(image->format, 0, image->w, image->h);

    // Create buffer in SSRAM
    image->bo = nema_buffer_create_pool(NEMA_MEM_POOL_FB, size);
    if(image->bo.base_virt == NULL)
    {
        return -1;
    }
    else
    {
        return 0;
    }
}

//*****************************************************************************
//
//! @brief Initialize sources
//!
//! This function copys textures or other resources from MRAM to PSRAM
//!
//! @return -1:failed, 0:OK..
//
//*****************************************************************************
int
texture_load(img_obj_t* image, const unsigned char *ptr)
{
    uint32_t size = nema_texture_size(image->format, 0, image->w, image->h);

    image->bo.base_phys = MSPI_XIP_BASE_ADDRESS;
    image->bo.base_virt= (void *)image->bo.base_phys;
    image->bo.size = size;
    // Do memcpy and cache flush.
    nema_memcpy((void*)image->bo.base_phys, (void *)ptr, size);
    return 0;
}

float angle_dx = 0.f;
float angle_dy = 0.f;
float trans_dz = 0.f;
bool bDrawWire = false;

static float angle_x = 0.f;
static float angle_y = 0.f;
static float trans_z = 0.f;
static bool draw_wireframe = false;

void 
draw_earth() 
{
    nema_tri_cull(NEMA_CULL_CW);
    nema_bind_src_tex(earth_img.bo.base_phys, earth_img.w, earth_img.h, earth_img.format, earth_img.stride, earth_img.sampling_mode);
    nema_set_blend_blit(NEMA_BL_SRC);

    nema_matrix4x4_t mvp;
    nema_mat4x4_load_perspective(mvp, 45.f, (float)RESX/(float)RESY, 0.1f, 1000.f);

    nema_matrix4x4_t proj;
    nema_mat4x4_load_identity(proj);
    nema_mat4x4_scale(proj, 0.1f, -0.1f, -0.1f);
    nema_mat4x4_rotate_Y(proj, angle_y);
    nema_mat4x4_rotate_X(proj, angle_x);
    nema_mat4x4_translate(proj, 0.f, 0.f, 200.f+trans_z);

    nema_mat4x4_mul(mvp, mvp, proj);

    for (int idx_id = 0; idx_id < EARTH_INDICES_NUM; ) 
    {
        int idx0 = earth_indices[idx_id++];
        int idx1 = earth_indices[idx_id++];
        int idx2 = earth_indices[idx_id++];

        float *v0 = &earth_vertices[idx0*3];
        float *v1 = &earth_vertices[idx1*3];
        float *v2 = &earth_vertices[idx2*3];

        float *uv0 = &earth_uv[idx0*2];
        float *uv1 = &earth_uv[idx1*2];
        float *uv2 = &earth_uv[idx2*2];

        float vv0[4] = {
               v0[0],
               v0[1],
               v0[2],
                1.f
        };
        float vv1[4] = {
               v1[0],
               v1[1],
               v1[2],
                 1.f
        };
        float vv2[4] = {
               v2[0],
               v2[1],
               v2[2],
                 1.f
        };

        int clip;
        clip = nema_mat4x4_obj_to_win_coords(mvp, 0.f, 0.f, RESX, RESY, 0.1f, 1000.f, &vv0[0], &vv0[1], &vv0[2], &vv0[3]);
        if (clip) continue;
        clip = nema_mat4x4_obj_to_win_coords(mvp, 0.f, 0.f, RESX, RESY, 0.1f, 1000.f, &vv1[0], &vv1[1], &vv1[2], &vv1[3]);
        if (clip) continue;
        clip = nema_mat4x4_obj_to_win_coords(mvp, 0.f, 0.f, RESX, RESY, 0.1f, 1000.f, &vv2[0], &vv2[1], &vv2[2], &vv2[3]);
        if (clip) continue;

        float area2 = 0.f;
        {
            area2 =
            vv0[0] * ( vv1[1] - vv2[1]) +
            vv1[0] * ( vv2[1] - vv0[1]) +
            vv2[0] * ( vv0[1] - vv1[1]);

            if ( area2 >= 0.f ) continue;
        }

        if ( draw_wireframe ) 
        {
            // if draw_wireframe, we need to set the blending function on each triangle
            nema_set_blend_blit(NEMA_BL_SRC);
        }
        nema_blit_tri_uv(
            vv0[0], vv0[1], vv0[2],
            vv1[0], vv1[1], vv1[2],
            vv2[0], vv2[1], vv2[2],
            uv0[0]*earth_img.w, uv0[1]*earth_img.h,
            uv1[0]*earth_img.w, uv1[1]*earth_img.h,
            uv2[0]*earth_img.w, uv2[1]*earth_img.h
        );

        if ( draw_wireframe ) 
        {
            nema_set_blend_fill(NEMA_BL_SRC);
            nema_draw_line(vv0[0], vv0[1], vv1[0], vv1[1], 0xff0000ffU);
            nema_draw_line(vv1[0], vv1[1], vv2[0], vv2[1], 0xff0000ffU);
            nema_draw_line(vv2[0], vv2[1], vv0[0], vv0[1], 0xff0000ffU);
        }
    }

    nema_tri_cull(NEMA_CULL_NONE);
}
//*****************************************************************************
//
//! @brief Render the earth
//!
//! This function implements the zoom-in/out, rotating, and wire effects on the
//! earth
//!
//
//*****************************************************************************
void
render_earth(nema_img_obj_t* fb, nema_cmdlist_t* cl)
{
    //
    // bind the CL
    //
    nema_cl_bind_circular(cl);
    nema_cl_rewind(cl);

    //Bind Framebuffer
    nema_bind_dst_tex(fb->bo.base_phys, fb->w, fb->h, fb->format, fb->stride);
    //Set Clipping Rectangle
    nema_set_clip(0, 0, RESX, RESY);
    //nema_clear(0x00000000);
    //Set Blending Mode
    nema_set_blend_fill(NEMA_BL_SRC);
    //Fill Rectangle with Black (Clear)
    nema_fill_rect(0, 0, RESX, RESY, nema_rgba(0x00, 0x00, 0, 0xFF));

    draw_earth();
    nema_cl_submit(cl);

    if (bDrawWire) 
    {
        draw_wireframe = !draw_wireframe;
    }
    else
    {
        draw_wireframe = false;
    }

    angle_x += angle_dx;
    angle_y += angle_dy;
    trans_z += trans_dz;
    trans_z = nema_clamp(trans_z, -50.f, 750.f);
}

//*****************************************************************************
//
// Render task
//
//*****************************************************************************
void
RenderTask(void *pvParameters)
{
    //
    // Enable GPU here for load frame buffer.
    //
    am_hal_pwrctrl_periph_enable(AM_HAL_PWRCTRL_PERIPH_GFX);

    //
    // Initialize NemaSDK
    //
    nema_init();
    //nema_vg_init(RESX, RESY); 

    //
    // Prepare command list 
    //
    nema_cmdlist_t cl = nema_cl_create_sized(4 * 1024);

    //
    // Set command list region non-cacheable temporarily. We will fix this limitation in the next release.
    //
    sMPUCfg[1].ui32BaseAddress = (uint32_t)cl.bo.base_phys;
    sMPUCfg[1].ui32LimitAddress = (uint32_t)cl.bo.base_phys + cl.bo.size - 1;

    //
    // Set up the attributes.
    //
    am_hal_mpu_attr_configure(&sMPUAttr, sizeof(sMPUAttr) / sizeof(am_hal_mpu_attr_t));

    //
    // Clear the MPU regions.
    //
    am_hal_mpu_region_clear();

    //
    // Set up the regions.
    //
    am_hal_mpu_region_configure(sMPUCfg, sizeof(sMPUCfg) / sizeof(am_hal_mpu_region_config_t));

    //
    // Invalidate and clear DCACHE, this is required by CM55 TRF.
    //
    am_hal_cachectrl_dcache_invalidate(NULL, true);

    //
    // MPU enable
    //
    am_hal_mpu_enable(true, true);

    //
    // Load texture to SSRAM
    //
    texture_load(&earth_img, earth_nasa_rgb24);

    //
    // Prepare frame buffer
    //
    framebuffer_create(&g_sFrameBuffer[0]);
#if (FRAME_BUFFERS > 1)
    framebuffer_create(&g_sFrameBuffer[1]);
#endif
    //
    // Power off the GPU after rendering.
    //
    am_hal_pwrctrl_periph_disable(AM_HAL_PWRCTRL_PERIPH_GFX);
    while (1)
    {
        xSemaphoreTake(g_semDisplayEnd, portMAX_DELAY);

        //
        // Power on GPU
        //
        am_hal_pwrctrl_periph_enable(AM_HAL_PWRCTRL_PERIPH_GFX);

        //
        // Reinit NemaSDK and NemaVG
        //
        nema_reinit();
        nema_reset_last_cl_id();

        //nema_vg_reinit();    
        //
        // Render earth
        //
        render_earth(&g_sFrameBuffer[0], &cl);

        //
        // Wait rendering complete
        //
        nema_cl_wait(&cl);
        //
        // Power off the GPU after rendering.
        //
        am_hal_pwrctrl_periph_disable(AM_HAL_PWRCTRL_PERIPH_GFX);

        //
        // Release semphore
        //
        xSemaphoreGive(g_semDisplayStart);
  
    }

}

