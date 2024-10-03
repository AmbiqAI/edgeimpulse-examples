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

#include "nemagfx_tsuite2d_test_vg.h"

#define tsi_print   am_util_stdio_printf

//*****************************************************************************
//
// Variables Definition.
//
//*****************************************************************************

nema_cmdlist_t context_cl;
nema_cmdlist_t clearblack_cl;
nema_cmdlist_t clearwhite_cl;

nema_buffer_t fb_bo  = {0};
nema_buffer_t zbuffer_bo  = {0};

#define RESX                640
#define RESY                480

#define GPU_FORMAT          NEMA_RGB24


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
       //optimizations
        nema_set_depth_imm(0, 0, 0, 0, 0, 0);

        int fb_size = nema_texture_size(GPU_FORMAT, 0, RESX, RESY);

        //Allocate Framebuffer - TEX0
        fb_bo = nema_buffer_create(fb_size);
//        nema_buffer_map(&fb_bo);
        tsi_print("Framebuffer\n");
        tsi_print("Virt: 0x%x\n", (unsigned)fb_bo.base_virt);
        tsi_print("Phys: 0x%x\n", (unsigned)fb_bo.base_phys);

 //      hw_config  = nema_readHwConfig();
 //      hw_configh = nema_readHwConfigH();

 //      //Allocate ZBuffer - TEX3
 //      zbuffer_bo = nema_buffer_create(RESX*RESY*4);
 //      nema_buffer_map(&zbuffer_bo);

    //Context CL
    //---------------------------------------------------------------------
    context_cl = nema_cl_create();
    tsi_print("Context CL: 0x%x\n", (unsigned)&context_cl);
    nema_cl_bind(&context_cl);
    nema_set_const_reg(0, 0);
    nema_enable_gradient(0);
    nema_enable_depth(0);

    nema_bind_dst_tex(fb_bo.base_phys, RESX, RESY, GPU_FORMAT, -1);
    //nema_bind_depth_buffer(zbuffer_bo.base_phys, RESX, RESY);

    nema_set_clip(0, 0, RESX, RESY);

    nema_cl_unbind();

    //Clear Black CL
    //---------------------------------------------------------------------
    clearblack_cl = nema_cl_create_sized(256);
    nema_cl_bind(&clearblack_cl);
    nema_cl_branch(&context_cl);
    nema_set_matrix_translate(0, 0);
    nema_clear(BLACK);
    nema_cl_unbind();

    //Clear White CL
    //---------------------------------------------------------------------
    clearwhite_cl = nema_cl_create_sized(256);
    nema_cl_bind(&clearwhite_cl);
    nema_cl_branch(&context_cl);
    nema_set_matrix_translate(0, 0);
    nema_clear(OWHITE);
    nema_cl_unbind();
}

//  Test NemaVG
//---------------------------------------------------------------------
void tsuite2d_test_vg()
{
#if 1//def USE_NEMAVG
    tsi_print("TB_SUITE: NemaVG\n");

    #define NUM_VERTICES 18
    #define NUM_SEG      7

    float coords[NUM_VERTICES] =
    {
        20.0f,  50.0f,  //move
        100.0f, 10.0f,  //quad ctrl
        150.0f, 200.0f, //quad
        20.0f,  260.0f, // line
        20.0f,  220.0f, // line
        150.0f, 300.0f, // line
        75.0f,  250.0f, //cubic ctrl 0
        125.0f, 350.0f, //cubic ctrl
        50.0f,  300.0f  //cubic
    };

    uint8_t cmds[NUM_SEG] =
    {
        NEMA_VG_PRIM_MOVE,
        NEMA_VG_PRIM_BEZIER_QUAD,
        NEMA_VG_PRIM_LINE,
        NEMA_VG_PRIM_LINE,
        NEMA_VG_PRIM_LINE,
        NEMA_VG_PRIM_BEZIER_CUBIC,
        NEMA_VG_PRIM_CLOSE
    };

    nema_matrix3x3_t m =  {  {2.f,  0.5f, 100.f},
                             {0.5f,  1.f, 100.f},
                             {0.f,  0.f, 1.f}
                          };


    nema_matrix3x3_t m2 = {  {1.5f,  0.2f, 300.f},
                             {0.f,  1.f, 20.f},
                             {0.f,  0.f, 1.f} // Projection Not supported in SW
                          };

    //---------------------------------------------------------------------
    nema_cl_submit(&clearblack_cl);
    nema_cl_wait(&clearblack_cl);
    nema_cmdlist_t cl = nema_cl_create();
    nema_cl_bind(&cl);
    nema_cl_branch(&context_cl);

    //---------------------------------------------------------------------
    nema_vg_init(RESX, RESY);

    NEMA_VG_PATH_HANDLE path = nema_vg_path_create();
    NEMA_VG_PAINT_HANDLE paint = nema_vg_paint_create();

    nema_vg_path_clear(path);
    nema_vg_path_set_shape(path, NUM_SEG, cmds, NUM_VERTICES, coords);

    nema_vg_set_fill_rule(NEMA_VG_FILL_EVEN_ODD);
    nema_vg_paint_set_type(paint, NEMA_VG_PAINT_FILL);
    nema_vg_paint_set_paint_color(paint, nema_rgba(0xff, 0x00, 0x00, 0xff));
    nema_vg_draw_path(path, paint);

    nema_vg_set_fill_rule(NEMA_VG_FILL_NON_ZERO);
    nema_vg_paint_set_type(paint, NEMA_VG_PAINT_FILL);
    nema_vg_paint_set_paint_color(paint, nema_rgba(0x33, 0x55, 0x77, 0xff));
    nema_vg_path_set_matrix(path, m);
    nema_vg_draw_path(path, paint);

    nema_vg_set_fill_rule(NEMA_VG_FILL_DRAW);
    nema_vg_paint_set_type(paint, NEMA_VG_PAINT_FILL);
    nema_vg_paint_set_paint_color(paint, nema_rgba(0xff, 0xff, 0x00, 0xff));
    nema_vg_path_set_matrix(path, m2);
    nema_vg_draw_path(path, paint);

    //---------------------------------------------------------------------
    nema_cl_unbind();
    nema_cl_submit(&cl);
    nema_cl_wait(&cl);
    nema_cl_destroy(&cl);
    nema_vg_paint_destroy(paint);
    nema_vg_path_destroy(path);
    nema_vg_deinit();
    //---------------------------------------------------------------------

    //snapshot_name("vg_curves",0);
    //cmp_image("vg_curves",0,0);
#else
  printf("Not supported\n");
#endif
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
    // Configure the board for low power operation.
    //
    am_bsp_low_power_init();

    //
    // Initialize the printf interface for ITM/SWO output.
    //
    am_bsp_itm_printf_enable();

    //
    // Clear the terminal and print the banner.
    //
    am_util_stdio_terminal_clear();

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
    //  Test NemaVG
    //
    tsuite2d_test_vg();

    //
    // Send FB to display
    //
    am_devices_display_transfer_frame(RESX, RESY, fb_bo.base_phys, NULL, NULL);

    while(1);
}

