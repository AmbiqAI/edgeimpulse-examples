//*****************************************************************************
//
//! @file main.c
//!
//! @brief NemaGFX example.
//! this example use one frame buffer demonstrate two picture trasition effect,
//! with Nema GPU support, the effect include
//! NEMA_TRANS_LINEAR_H,
//! NEMA_TRANS_CUBE_H,
//! NEMA_TRANS_INNERCUBE_H,
//! NEMA_TRANS_STACK_H,
//! NEMA_TRANS_LINEAR_V,
//! NEMA_TRANS_CUBE_V,
//! NEMA_TRANS_INNERCUBE_V,
//! NEMA_TRANS_STACK_V,
//! NEMA_TRANS_FADE,
//! NEMA_TRANS_FADE_ZOOM,
//! NEMA_TRANS_MAX,
//! NEMA_TRANS_NONE
//
//*****************************************************************************

//*****************************************************************************
//
// ${copyright}
//
// This is part of revision ${version} of the AmbiqSuite Development Package.
//
//*****************************************************************************

#include <string.h>

#include "am_bsp.h"
#include "am_util_delay.h"


#include "nema_core.h"
#include "nema_utils.h"
#include "nema_dc.h"
#include "nema_dc_mipi.h"

#include "nema_transitions.h"
#include "nema_utils.h"
#include "nemagfx_transition_effects.h"

#include "im00.rgba565.h"
#include "im01.rgba565.h"
#include "daxi_test_common.h"

static img_obj_t g_sScreen0 = {
                        {.size = IM00_RGBA565_LEN,
                         .base_virt = (void *)g_ui8Im00RGBA565,
                         .base_phys = (uintptr_t)g_ui8Im00RGBA565},
                         256, 256, -1, 0, (uint8_t)NEMA_RGB565, 0};
static img_obj_t g_sScreen1 = {
                        {.size = IM01_RGBA565_LEN,
                         .base_virt = (void *)g_ui8Im01RGBA565,
                         .base_phys = (uintptr_t)g_ui8Im01RGBA565},
                         256, 256, -1, 0, (uint8_t)NEMA_RGB565, 0};

#define FRAME_BUFFERS 1
static img_obj_t g_sFB[FRAME_BUFFERS];

static void
load_objects(void)
{
#ifdef FB_IN_PSRAM
    uint32_t ui32CurStartAddr;
    ui32CurStartAddr = MSPI_XIPMM_BASE_ADDRESS + IM00_RGBA565_LEN + IM01_RGBA565_LEN;
#endif
    for (int32_t i = 0; i < FRAME_BUFFERS; ++i)
    {
        g_sFB[i].w = RESX;
        g_sFB[i].h = RESY / SMALLFB_STRIPES;
        g_sFB[i].format = NEMA_RGB565;
        g_sFB[i].stride = RESX * 2;
#ifndef FB_IN_PSRAM
        g_sFB[i].bo = nema_buffer_create(g_sFB[i].stride * g_sFB[i].h);
        (void)nema_buffer_map(&g_sFB[i].bo);
#else
        g_sFB[i].bo.base_phys = ui32CurStartAddr + g_sFB[i].stride * g_sFB[i].h;
        g_sFB[i].bo.base_virt = (void *)g_sFB[i].bo.base_phys;
        ui32CurStartAddr = g_sFB[i].bo.base_phys;
#endif
    }

// #### INTERNAL BEGIN ####
//#define LOAD_FROM_DSP_SRAM
#if defined(LOAD_FROM_DSP_SRAM)
    //
    // Preload the textures from MRAM to DSP_SRAM
    // Set the textures address in DSP_SRAM for GPU
    //
    g_sScreen0.bo.base_phys = (uintptr_t)0x10160000;
    g_sScreen0.bo.base_virt = (void*)g_sScreen0.bo.base_phys;
    g_sScreen1.bo.base_phys = (uintptr_t)(0x10160000 + IM00_RGBA565_LEN);
    g_sScreen1.bo.base_virt = (void*)g_sScreen1.bo.base_phys;

    //g_sScreen0.bo = nema_buffer_create(IM00_RGBA565_LEN);
    memcpy((void*)g_sScreen0.bo.base_phys, g_ui8Im00RGBA565, IM00_RGBA565_LEN);
    //g_sScreen1.bo = nema_buffer_create(IM01_RGBA565_LEN);
    memcpy((void*)g_sScreen1.bo.base_phys, g_ui8Im01RGBA565, IM01_RGBA565_LEN);
#elif defined(LOAD_FROM_MRAM)
    //
    // Set the textures address in MRAM for GPU
    //
    g_sScreen0.bo.base_phys = (uintptr_t)g_ui8Im00RGBA565;
    g_sScreen0.bo.base_virt = (void*)g_sScreen0.bo.base_phys;
    g_sScreen1.bo.base_phys = (uintptr_t)g_ui8Im01RGBA565;
    g_sScreen1.bo.base_virt = (void*)g_sScreen1.bo.base_phys;
#elif defined(LOAD_FROM_PSRAM)
    g_sScreen0.bo.base_phys = (uintptr_t)MSPI_XIPMM_BASE_ADDRESS;
    g_sScreen0.bo.base_virt = (void*)g_sScreen0.bo.base_phys;
    g_sScreen1.bo.base_phys = (uintptr_t)(MSPI_XIPMM_BASE_ADDRESS + IM00_RGBA565_LEN);
    g_sScreen1.bo.base_virt = (void*)g_sScreen1.bo.base_phys;


    memcpy((void*)g_sScreen0.bo.base_phys, g_ui8Im00RGBA565, IM00_RGBA565_LEN);
    memcpy((void*)g_sScreen1.bo.base_phys, g_ui8Im01RGBA565, IM01_RGBA565_LEN);

#else
// #### INTERNAL END ####
    //
    // Preload the textures from MRAM to SSRAM
    // Set the textures address in SSRAM for GPU
    //

    g_sScreen0.bo = nema_buffer_create(IM00_RGBA565_LEN);
    memcpy((void*)g_sScreen0.bo.base_phys, g_ui8Im00RGBA565, IM00_RGBA565_LEN);
    g_sScreen1.bo = nema_buffer_create(IM01_RGBA565_LEN);
    memcpy((void*)g_sScreen1.bo.base_phys, g_ui8Im01RGBA565, IM01_RGBA565_LEN);

// #### INTERNAL BEGIN ####
#endif
// #### INTERNAL END ####
}

static nema_cmdlist_t g_sCLDrawEntireScene;
static nema_cmdlist_t g_sCL;

static int32_t
init(void)
{
    am_devices_disp_color_e eDispColor = COLOR_FORMAT_RGB565;

    if(eDispColor == COLOR_FORMAT_RGB565)
    {
       g_sDispCfg.eDsiFreq = AM_HAL_DSI_FREQ_TRIM_X13;
    }
    //
    // Initialize display
    //
    am_devices_display_init(RESX,
                            RESY,
                            eDispColor,
                            false);
    am_hal_pwrctrl_periph_enable(AM_HAL_PWRCTRL_PERIPH_GFX);
    //
    // Global interrupt enable
    //
    am_hal_interrupt_master_enable();

    //
    // Initialize NemaGFX
    //
    nema_init();

    load_objects();

    g_sCLDrawEntireScene  = nema_cl_create();
    g_sCL  = nema_cl_create();

    return 0;
}

static nema_transition_t g_eEffect = NEMA_TRANS_LINEAR_H;

static void
display(float step)
{
    static bool bFirstFrame = true;
    uint8_t ui8CurrentIndex = 0;
    for (int32_t stripe = 0; stripe < SMALLFB_STRIPES; ++stripe)
    {
        nema_cl_bind(&g_sCLDrawEntireScene);
        nema_cl_rewind(&g_sCLDrawEntireScene);
        nema_bind_tex(NEMA_TEX1,
                  g_sScreen0.bo.base_phys + stripe * g_sScreen0.bo.size / SMALLFB_STRIPES,
                  g_sScreen0.w,
                  g_sScreen0.h,
                  g_sScreen0.format,
                  g_sScreen0.stride,
                  NEMA_FILTER_BL | NEMA_TEX_BORDER);

        nema_bind_tex(NEMA_TEX2,
                  g_sScreen1.bo.base_phys + stripe * g_sScreen1.bo.size / SMALLFB_STRIPES,
                  g_sScreen1.w,
                  g_sScreen1.h,
                  g_sScreen1.format,
                  g_sScreen1.stride,
                  NEMA_FILTER_BL | NEMA_TEX_BORDER);

        nema_set_tex_color(0);

        nema_transition(g_eEffect,
                        NEMA_TEX1,
                        NEMA_TEX2,
                        NEMA_BL_SRC,
                        step,
                        RESX,
                        RESY / SMALLFB_STRIPES);

        nema_cl_bind(&g_sCL);
        nema_cl_rewind(&g_sCL);
        //
        // Bind Framebuffer
        //
        nema_bind_dst_tex(g_sFB[ui8CurrentIndex].bo.base_phys,
                          g_sFB[ui8CurrentIndex].w,
                          g_sFB[ui8CurrentIndex].h,
                          g_sFB[ui8CurrentIndex].format,
                          g_sFB[ui8CurrentIndex].stride);

        //
        // Set Clipping Rectangle
        //
        nema_set_clip(0,
                      0,
                      RESX,
                      RESY / SMALLFB_STRIPES);

        nema_cl_jump(&g_sCLDrawEntireScene);

        //
        // make sure display transfer is completed before submit next cl.
        //
        if(bFirstFrame == false)
        {
            am_devices_display_wait_transfer_done();
        }
        nema_cl_submit(&g_sCL);
#if (FRAME_BUFFERS == 1)
        //
        //wait GPU render completed before transfer frame when using singlebuffer.
        //
        nema_cl_wait(&g_sCL);
#endif
#if (SMALLFB_STRIPES != 1)
        //
        // Reposition the display area if necessary
        //
        am_devices_display_set_region(RESX,
                                      RESY / SMALLFB_STRIPES,
                                      PANEL_OFFSET,
                                      PANEL_OFFSET + RESY / SMALLFB_STRIPES * stripe);
#endif
        //
        // transfer frame to the display
        //
        am_devices_display_transfer_frame(g_sFB[ui8CurrentIndex].w,
                                          g_sFB[ui8CurrentIndex].h,
                                          g_sFB[ui8CurrentIndex].bo.base_phys,
                                          NULL,
                                          NULL);
        bFirstFrame = false;
        ui8CurrentIndex = (ui8CurrentIndex +1) % FRAME_BUFFERS;
#if (FRAME_BUFFERS > 1)
        //
        //wait GPU render completed after transfer frame when using dualbuffers.
        //
        nema_cl_wait(&g_sCL);
#endif

    }
}

static void
next_effect(void)
{
#if (SMALLFB_STRIPES != 1)
    if(g_eEffect == NEMA_TRANS_LINEAR_H)
    {
        g_eEffect = NEMA_TRANS_LINEAR_V;
    }
    else
    {
        g_eEffect = NEMA_TRANS_LINEAR_H;
    }
#else
    g_eEffect = (nema_transition_t)(((int32_t)g_eEffect + 1) % NEMA_TRANS_MAX);
#endif
}

static void
loop(void)
{
#define MAX_FRAMES        30
    float step = MIN_STEP;
    float step_step = ANIMATION_STEP_0_1;
    uint32_t i;
    for (i = 0; i < MAX_FRAMES; i++)
    {
        display(step);

        if (step <= MIN_STEP)
        {
            step = MIN_STEP;
            step_step = ANIMATION_STEP_0_1;
            next_effect();
        }
        else if (step >= MAX_STEP)
        {
            step = MAX_STEP;
            step_step = -ANIMATION_STEP_0_1;
            next_effect();
        }
        step += step_step;
            // nema_calculate_fps();
    }
}

bool
transition_effects(void)
{
    uint32_t i, j, ui32Status;
    bool bPass = true;
#ifdef DAXI_TEST
    daxi_test_cfg_init();
#endif
#ifdef CACHE_TEST
    cache_test_cfg_init();
    for (j = 0; j < sizeof(g_sCacheCfg) / sizeof(cache_enable_config_t); j++)
    {
        if (g_sCacheCfg[j].bCacheEnable == false)
        {
            ui32Status = am_hal_cachectrl_disable();
            if (ui32Status != AM_HAL_STATUS_SUCCESS)
            {
                am_util_stdio_printf("Failed to disable cache!");
            }
        }
        else
        {
            ui32Status = am_hal_cachectrl_config(&(g_sCacheCfg[j].sCacheCtrl));
            if (ui32Status != AM_HAL_STATUS_SUCCESS)
            {
                am_util_stdio_printf("Failed to configure cache!");
            }
            ui32Status = am_hal_cachectrl_enable();
            if (ui32Status != AM_HAL_STATUS_SUCCESS)
            {
                am_util_stdio_printf("Failed to enable cache!");
            }
        }
#endif
#ifdef DAXI_TEST
        for (i = 0; i < sizeof(g_sDaxiCfg) / sizeof(am_hal_daxi_config_t); i++)
        {
            if (g_sDaxiCfg[i].bDaxiPassThrough == true)
            {
                ui32Status = am_hal_daxi_control(AM_HAL_DAXI_CONTROL_DISABLE, NULL);
                if (ui32Status != AM_HAL_STATUS_SUCCESS)
                {
                    am_util_stdio_printf("Failed to disable DAXI!");
                }
            }
            else
            {
                ui32Status = am_hal_daxi_config(&g_sDaxiCfg[i]);
                if (ui32Status != AM_HAL_STATUS_SUCCESS)
                {
                    am_util_stdio_printf("Failed to configure DAXI!");
                }
            }
#endif
            am_util_stdio_printf("\ntransition_effects with cache config - %d (CPU->CACHECFG is 0x%08X), daxi config - %d (CPU->DAXICFG is 0x%08X)...\n", j, CPU->CACHECFG, i, CPU->DAXICFG);
            float fExecTime;
#ifndef USE_STIMER
            float fStartTime, fStopTime;
            fStartTime = nema_get_time();
#else
            uint32_t ui32StartTime, ui32StopTime;
            ui32StartTime = am_hal_stimer_counter_get();
            am_hal_stimer_config(AM_HAL_STIMER_HFRC_6MHZ | AM_HAL_STIMER_CFG_RUN);
#endif
            g_eEffect = NEMA_TRANS_LINEAR_H;
            init();
            loop();
            nema_buffer_destroy(&(g_sScreen0.bo));
            nema_buffer_destroy(&(g_sScreen1.bo));
            nema_cl_destroy(&g_sCLDrawEntireScene);
            nema_cl_destroy(&g_sCL);
#ifndef USE_STIMER
            fStopTime = nema_get_time();
            fExecTime = fStopTime - fStartTime;
            am_util_stdio_printf("\nCache config: %d, DAXI config:  %d, Execution Time: %.02f\n", j, i, fExecTime);

#else
            ui32StopTime = am_hal_stimer_counter_get();
            fExecTime = ((float)ui32StopTime - (float)ui32StartTime) / 1500000.0f;
            am_util_stdio_printf("\nCache config: %d, DAXI config:  %d, Execution Time: %.10f seconds\n", j, i, fExecTime);
            am_hal_stimer_counter_clear();
#endif
#ifdef APOLLO4_FPGA
            if (fExecTime > 0.7f)
            {
                am_util_stdio_printf("------ Timeout! ------\n");
                bPass = false;;
            }
#endif
            TEST_ASSERT_TRUE(bPass);
#ifdef DAXI_TEST
        }
#endif
#ifdef CACHE_TEST
    }
#endif

    return bPass;
}

