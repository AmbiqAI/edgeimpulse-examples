//*****************************************************************************
//
//! @file joins_caps.c
//!
//! @brief Demo of different join and cap styles in stroke path. 
//!
//
//*****************************************************************************

//*****************************************************************************
//
// ${copyright}
//
// This is part of revision ${version} of the AmbiqSuite Development Package.
//
//*****************************************************************************

#include "nemagfx_vg_test.h"

#ifdef RUN_JOINS_CAPS



//*****************************************************************************
//
// Macro definitions
//
//*****************************************************************************
#define RESX 452
#define RESY 452


//*****************************************************************************
//
// Global Variables
//
//*****************************************************************************
//! framebuffrer
nema_img_obj_t g_sFrameBuffer =
{
    {0}, RESX, RESY, RESX*3, 0, NEMA_RGB24, 0
};


//! DC layer
nemadc_layer_t g_sDCLayer =
{
    (void *)0, 0, RESX, RESY, -1, 0, 0, RESX, RESY, 0xff,
    NEMADC_BL_SRC, 0, NEMADC_RGB24, 0, 0, 0, 0, 0, 0, 0, 0
};

float path_line_data[] =
{
    0.f, 0.f,
    50.f, 0.f,
};

uint8_t line_cmds[] = {NEMA_VG_PRIM_MOVE,
                       NEMA_VG_PRIM_LINE_REL,
};

float path_angle_data[] =
{
    0.f, 0.f,
    50.f, 50.f,
    50.f, -50.f,
};

uint8_t angle_cmds[] = {NEMA_VG_PRIM_MOVE,
                       NEMA_VG_PRIM_LINE_REL,
                       NEMA_VG_PRIM_LINE_REL,
};



//*****************************************************************************
//
//! @brief Create buffer, load image from MRAM to ssram
//!
//
//*****************************************************************************
static void bufferCreate(void)
{
    //Load memory objects
    g_sFrameBuffer.bo = nema_buffer_create_pool(NEMA_MEM_POOL_FB, g_sFrameBuffer.stride*g_sFrameBuffer.h);
    nema_buffer_map(&g_sFrameBuffer.bo);
    printf("FB: V:%p P:0x%08x\n", (void *)g_sFrameBuffer.bo.base_virt, g_sFrameBuffer.bo.base_phys);

    //set dc layer memory
    g_sDCLayer.baseaddr_phys = g_sFrameBuffer.bo.base_phys;
    g_sDCLayer.baseaddr_virt = g_sFrameBuffer.bo.base_virt;

    printf("FB: V:%p P:0x%08x\n", (void *)g_sFrameBuffer.bo.base_virt, g_sFrameBuffer.bo.base_phys);
}



//*****************************************************************************
//
//! @brief Destroy buffers
//!
//
//*****************************************************************************
static void bufferDestroy(void)
{
    nema_buffer_unmap(&g_sFrameBuffer.bo);
    nema_buffer_destroy(&g_sFrameBuffer.bo);
}

//*****************************************************************************
//
//! @brief run the render_joins_caps demo
//!
//
//*****************************************************************************
int render_joins_caps()
{
    //Initialize NemaGFX
    if ( nema_init() != 0 )
    {
        return -1;
    }

    //Init NemaVG
    nema_vg_init(RESX, RESY);

    //Initialize NemaDC
    if ( display_setup(RESX, RESY) != 0 )
    {
        return -2;
    }

    bufferCreate();

    nemadc_set_layer(0, &g_sDCLayer);

    // Use circular cl
    nema_cmdlist_t cl  = nema_cl_create_sized(10*1024);
    nema_cl_bind_circular(&cl);


    // Clear frame buffer
    nema_bind_dst_tex(g_sFrameBuffer.bo.base_phys, g_sFrameBuffer.w, g_sFrameBuffer.h, g_sFrameBuffer.format, g_sFrameBuffer.stride);
    nema_set_clip(0, 0, RESX, RESY);
    nema_clear(nema_rgba(25, 128, 230, 0xff)); //yellow

    // Set context
    nema_vg_set_fill_rule(NEMA_VG_STROKE);
    nema_vg_set_blend(NEMA_BL_SRC_OVER);

    // Set paint
    NEMA_VG_PAINT_HANDLE paint_stroke =  nema_vg_paint_create();
    nema_vg_paint_set_type(paint_stroke, NEMA_VG_PAINT_COLOR);
    nema_vg_paint_set_paint_color(paint_stroke, nema_rgba(255, 128, 0, 128));

    NEMA_VG_PAINT_HANDLE paint_thin =  nema_vg_paint_create();
    nema_vg_paint_set_type(paint_thin, NEMA_VG_PAINT_COLOR);
    nema_vg_paint_set_paint_color(paint_thin, nema_rgba(255, 128, 0, 255));  

    // Set path matrix
    nema_matrix3x3_t m;
    nema_mat3x3_load_identity(m);

    // Set shape
    NEMA_VG_PATH_HANDLE path_line =  nema_vg_path_create();
    nema_vg_path_set_shape(path_line, sizeof(line_cmds)/sizeof(uint8_t), line_cmds, sizeof(path_line_data)/sizeof(float), path_line_data);

    NEMA_VG_PATH_HANDLE path_angle =  nema_vg_path_create();
    nema_vg_path_set_shape(path_angle, sizeof(angle_cmds)/sizeof(uint8_t), angle_cmds, sizeof(path_angle_data)/sizeof(float), path_angle_data);   

    //
    // Fist line with BUTT cap 
    //

    //Set path position
    nema_mat3x3_translate(m, 100, 100);
    nema_vg_path_set_matrix(path_line, m);

    //Set stroke cap and darw the stoke line
    nema_vg_stroke_set_width(20);
    nema_vg_stroke_set_cap_style(NEMA_VG_CAP_BUTT, NEMA_VG_CAP_BUTT);
    nema_vg_draw_path(path_line, paint_stroke);

    //Draw the thin line
    nema_vg_stroke_set_width(1);
    nema_vg_draw_path(path_line, paint_thin);    

    //
    // Second line with ROUND cap 
    //

    //Set path position
    nema_mat3x3_translate(m, 0, 100);
    nema_vg_path_set_matrix(path_line, m);

    //Set stroke cap and darw the stoke line
    nema_vg_stroke_set_width(20);
    nema_vg_stroke_set_cap_style(NEMA_VG_CAP_ROUND, NEMA_VG_CAP_ROUND);
    nema_vg_draw_path(path_line, paint_stroke);

    //Draw the thin line
    nema_vg_stroke_set_width(1);
    nema_vg_draw_path(path_line, paint_thin);  

    //
    // Third line with SQUARE cap 
    //

    //Set path position
    nema_mat3x3_translate(m, 0, 100);
    nema_vg_path_set_matrix(path_line, m);

    //Set stroke cap and darw the stoke line
    nema_vg_stroke_set_width(20);
    nema_vg_stroke_set_cap_style(NEMA_VG_CAP_SQUARE, NEMA_VG_CAP_SQUARE);
    nema_vg_draw_path(path_line, paint_stroke);

    //Draw the thin line
    nema_vg_stroke_set_width(1);
    nema_vg_draw_path(path_line, paint_thin);  


    // Restore transformation matrix 
    nema_mat3x3_load_identity(m);

    //
    // First angle with BEVEL cap 
    //

    //Set line position
    nema_mat3x3_translate(m, 200, 100);
    nema_vg_path_set_matrix(path_angle, m);

    //Set stroke cap and darw the stoke line
    nema_vg_stroke_set_width(20);
    nema_vg_stroke_set_join_style(NEMA_VG_JOIN_BEVEL);
    nema_vg_draw_path(path_angle, paint_stroke);

    //Draw the thin line
    nema_vg_stroke_set_width(1);
    nema_vg_draw_path(path_angle, paint_thin);  

    //
    // Second angle with MITER cap 
    //

    //Set line position
    nema_mat3x3_translate(m, 0, 100);
    nema_vg_path_set_matrix(path_angle, m);

    //Set stroke cap and darw the stoke line
    nema_vg_stroke_set_width(20);
    nema_vg_stroke_set_join_style(NEMA_VG_JOIN_ROUND);
    nema_vg_draw_path(path_angle, paint_stroke);

    //Draw the thin line
    nema_vg_stroke_set_width(1);
    nema_vg_draw_path(path_angle, paint_thin);  

    //
    // Third angle with BEVEL cap 
    //

    //Set line position
    nema_mat3x3_translate(m, 0, 100);
    nema_vg_path_set_matrix(path_angle, m);

    //Set stroke cap and darw the stoke line
    nema_vg_stroke_set_width(20);
    nema_vg_stroke_set_join_style(NEMA_VG_JOIN_MITER);
    nema_vg_draw_path(path_angle, paint_stroke);

    //Draw the thin line
    nema_vg_stroke_set_width(1);
    nema_vg_draw_path(path_angle, paint_thin);  

    nema_cl_submit(&cl);
    nema_cl_wait(&cl);

    //Start DC
    display_refresh_start();
    //Wait DC complete interrupt.
    nemadc_wait_vsync();
    //Do follow-up operations required by hardware.
    display_refresh_end();

    nema_vg_paint_destroy(paint_stroke);
    nema_vg_paint_destroy(paint_thin);
    nema_vg_path_destroy(path_line);
    nema_vg_path_destroy(path_angle);   
    nema_vg_deinit();
    bufferDestroy();

    return 0;
}

#endif
