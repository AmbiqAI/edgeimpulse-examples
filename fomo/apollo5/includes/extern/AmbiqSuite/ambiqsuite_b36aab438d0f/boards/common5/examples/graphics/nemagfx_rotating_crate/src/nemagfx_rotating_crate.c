//*****************************************************************************
//
//! @file nemagfx_rotating_crate.c
//!
//! @brief Example of the app running nemagfx rotating_crate.
//! this example shows a rotating crate with texture rending support. with Nema
//! GPU support, it can significantly reduce the general CPU effort to calculate
//! the data inside the frame buffer
//!
//! AM_DEBUG_PRINTF
//! If enabled, debug messages will be sent over ITM.
//
//*****************************************************************************

//*****************************************************************************
//
// ${copyright}
//
// This is part of revision ${version} of the AmbiqSuite Development Package.
//
//*****************************************************************************

#include "nemagfx_rotating_crate.h"
#include "crate_bgr24.h"

#define CREATE_SIZE     180
//*****************************************************************************
//
// Variables Definition.
//
//*****************************************************************************

nema_cmdlist_t          g_sCL;
img_obj_t               g_sFB[FRAME_BUFFERS];
img_obj_t               g_sCrate = {{0}, CREATE_SIZE, CREATE_SIZE, -1, 0, CRATE_GPU_FORMAT, 0};

//*****************************************************************************
//
//! @brief initialize framebuffer(s)
//!
//! This function initialize framebuffer for GPU,layers for DC if have. alse load
//! texture to SSRAM.
//!
//! @return None.
//
//*****************************************************************************

void
load_objects(void)
{
    for (uint8_t i = 0; i < FRAME_BUFFERS; ++i)
    {
        g_sFB[i].w = RESX;
        g_sFB[i].h = RESY;
        g_sFB[i].format = CRATE_GPU_FORMAT;
        g_sFB[i].stride = g_sFB[i].w * nema_format_size(g_sFB[i].format);
        g_sFB[i].bo = nema_buffer_create_pool(NEMA_MEM_POOL_FB, g_sFB[i].stride * g_sFB[i].h);
        if ( g_sFB[i].bo.base_virt == NULL )
        {
            am_util_stdio_printf("Failed to create FB!\n");
            while(1);
        }
        nema_buffer_map(&g_sFB[i].bo);
    }

    g_sCrate.bo = nema_buffer_create_pool(NEMA_MEM_POOL_ASSETS, sizeof(crate_bgr24));
    if ( g_sCrate.bo.base_virt == NULL )
    {
        am_util_stdio_printf("Failed to create FB!\n");
        while(1);
    }
    nema_memcpy(g_sCrate.bo.base_virt, crate_bgr24, sizeof(crate_bgr24));
}

void
draw_cube_side(float *v, int32_t v0, int32_t v1, int32_t v2, int32_t v3, uint32_t col)
{
#if COLOR_CUBE
    //
    // fill with color
    //
    nema_fill_quad(v[v0 * 3], v[v0 * 3 + 1],
                   v[v1 * 3], v[v1 * 3 + 1],
                   v[v2 * 3], v[v2 * 3 + 1],
                   v[v3 * 3], v[v3 * 3 + 1], col);
#else
    //
    // blit
    //
    nema_blit_quad_fit(v[v0 * 3], v[v0 * 3 + 1],
                   v[v1 * 3], v[v1 * 3 + 1],
                   v[v2 * 3], v[v2 * 3 + 1],
                   v[v3 * 3], v[v3 * 3 + 1]);
#endif
}

void
inner_cube(int32_t i32AngleX, int32_t i32AngleY, int32_t i32AngleZ)
{
    float box_size_2 = 0.2f;
    float FoV = 28.0724869359f;
    float w = 1.0f;
    float v[] =
    {   //x     y    z
        -box_size_2, -box_size_2,  box_size_2,  //0  0
         box_size_2, -box_size_2,  box_size_2,  //1  3
         box_size_2,  box_size_2,  box_size_2,  //2  6
        -box_size_2,  box_size_2,  box_size_2,  //3  9
        -box_size_2, -box_size_2, -box_size_2,  //4  12
         box_size_2, -box_size_2, -box_size_2,  //5  15
         box_size_2,  box_size_2, -box_size_2,  //6  18
        -box_size_2,  box_size_2, -box_size_2   //7  21
    };
    //
    // projection
    //
    nema_matrix4x4_t mvp;

    nema_mat4x4_load_perspective(mvp, FoV, (float)RESX / RESY, 0.2f, 100.f);

    nema_matrix4x4_t proj;
    nema_mat4x4_load_identity(proj);
    nema_mat4x4_rotate_X(proj, i32AngleX);
    nema_mat4x4_rotate_Y(proj, i32AngleY);
    nema_mat4x4_rotate_Z(proj, i32AngleZ);
    nema_mat4x4_translate(proj, 0, 0, 2.f-box_size_2);

    nema_mat4x4_mul(mvp, mvp, proj);


    for (int32_t i = 0; i < 24; i += 3)
    {
        nema_mat4x4_obj_to_win_coords(mvp, 0.f, 0.f, RESX, RESY,
                                      1.f, 100.f,
                                      &v[i  ], &v[i + 1], &v[i + 2], &w);
    }
    //
    // blend color with background
    //
#if COLOR_CUBE
    nema_set_blend_fill(NEMA_BL_SIMPLE);
#else
    nema_set_blend_blit(NEMA_BL_SRC);
#endif
    nema_bind_src_tex(g_sCrate.bo.base_phys,
                      g_sCrate.w,
                      g_sCrate.h,
                      g_sCrate.format,
                      g_sCrate.stride,
                      NEMA_FILTER_BL);
    //
    // remove this to draw back sides also
    //
    nema_tri_cull(NEMA_CULL_CW);
    draw_cube_side(v, 0, 1, 2, 3, 0x60ffffff); //!< front
    draw_cube_side(v, 4, 0, 3, 7, 0x600000ff); //!< left
    draw_cube_side(v, 1, 5, 6, 2, 0x60ff00ff); //!< right
    draw_cube_side(v, 4, 5, 1, 0, 0x60ff0000); //!< top
    draw_cube_side(v, 3, 2, 6, 7, 0x6000ff00); //!< bottom
    draw_cube_side(v, 5, 4, 7, 6, 0x60808080); //!< back
    nema_tri_cull(NEMA_CULL_NONE);
}

//*****************************************************************************
//
//! @brief render the rotating crate
//!
//! This function render the 6  faces of  crate.
//!
//! @return None.
//
//*****************************************************************************
void
gfx_display_function()
{
    static int32_t i32AngleX = 0.0f, i32AngleY = 0.0f, i32AngleZ = 80.f;
    static uint8_t ui8CurrentIndex = 0;
    //
    // transfer frame to the display
    //
    am_devices_display_transfer_frame(g_sFB[ui8CurrentIndex].w,
                                      g_sFB[ui8CurrentIndex].h,
                                      g_sFB[ui8CurrentIndex].bo.base_phys,
                                      NULL, NULL);

    ui8CurrentIndex = (ui8CurrentIndex + 1) % FRAME_BUFFERS;

    i32AngleX = (i32AngleX + 1) % 360;
    i32AngleY = (i32AngleY + 2) % 360;
    i32AngleZ = (i32AngleZ + 2) % 360;

    nema_cl_rewind(&g_sCL);

    nema_bind_dst_tex(g_sFB[ui8CurrentIndex].bo.base_phys,
                      g_sFB[ui8CurrentIndex].w,
                      g_sFB[ui8CurrentIndex].h,
                      g_sFB[ui8CurrentIndex].format,
                      g_sFB[ui8CurrentIndex].stride);

    nema_set_clip(0, 0, RESX, RESY);

    nema_clear(0);

    inner_cube(i32AngleX, i32AngleY, i32AngleZ);

    nema_cl_submit(&g_sCL);

    //
    // wait GPU render completed after transfer frame when using dualbuffers.
    //
    nema_cl_wait(&g_sCL);
    //
    // wait transfer done
    //
    am_devices_display_wait_transfer_done();

}

//*****************************************************************************
//
// Main Function
//
//*****************************************************************************
int
main(void)
{
    //
    // External power on
    //
    am_bsp_external_pwr_on();
    am_util_delay_ms(100);

    //
    // Configure the board for low power operation.
    //
    am_bsp_low_power_init();

    //
    //  Enable the I-Cache and D-Cache.
    //
    am_hal_cachectrl_icache_enable();
    am_hal_cachectrl_dcache_enable(true);

    //
    // Initialize the printf interface for ITM/SWO output.
    //
    am_bsp_itm_printf_enable();

    //
    // Clear the terminal and print the banner.
    //
    am_util_stdio_terminal_clear();

#ifdef AM_DEBUG_PRINTF
    //
    // Initialize debug printf interface for ITM output
    //
    am_bsp_debug_printf_enable();
#endif

    //
    // Initialize display
    //
    am_devices_display_init(RESX,
                            RESY,
                            COLOR_FORMAT_RGB888,
                            false);


    //
    // Global interrupt enable
    //
    am_hal_interrupt_master_enable();

    //
    // Power up GPU
    //
    am_hal_pwrctrl_periph_enable(AM_HAL_PWRCTRL_PERIPH_GFX);

    //
    // Initialize NemaGFX
    //
    nema_init();

    //
    // Create framebuffer and loading texture.
    //
    load_objects();

    //
    // Create and bind command lists
    //
    g_sCL  = nema_cl_create();
    nema_cl_bind(&g_sCL);

    //
    // Show the rotating crate
    //
    while (1)
    {
        gfx_display_function();
        nema_calculate_fps();
    }

}

