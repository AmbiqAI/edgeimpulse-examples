//*****************************************************************************
//
//! @file nemagfx_vg_test_cases.c
//!
//! @brief NemaGFX vector graphics test cases.
//! Need to connect APS25616 PSRAM card to MSPI0.
//
//*****************************************************************************

//*****************************************************************************
//
// ${copyright}
//
// This is part of revision ${version} of the AmbiqSuite Development Package.
//
//*****************************************************************************
#include "nemagfx_test_common.h"
#include "tiger.tsvg.h"
#include "raw_path.h"
#include "ref.rgba.h"
#include "ume_ugo5_ttf.h"
#include "crc32.h"

//*****************************************************************************
//
// Macro definitions
//
//*****************************************************************************
#define FB_RESX 464
#define FB_RESY 464

#define FRAME_BUFFER_FORMAT NEMA_RGB24
#define DC_FORMAT           NEMADC_RGB24

#define TIGER_X 400
#define TIGER_Y 400

#define CENTER_POINT_X (FB_RESX/2)
#define CENTER_POINT_Y (FB_RESY/2)

#define DRAW_SHAPE_AREA (200)

//*****************************************************************************
//
// Frame buffer
//
//*****************************************************************************
img_obj_t fb = {{0}, FB_RESX, FB_RESY, -1, 0, FRAME_BUFFER_FORMAT, 0};

//*****************************************************************************
//
// Texture.
//
//*****************************************************************************
img_obj_t g_sRef = {{0}, 80, 100, -1,
                    0,   NEMA_RGBA8888, NEMA_FILTER_BL};

am_hal_pwrctrl_mcu_mode_e g_sPowerMode[2] = {AM_HAL_PWRCTRL_MCU_MODE_LOW_POWER, AM_HAL_PWRCTRL_MCU_MODE_HIGH_PERFORMANCE};

//*****************************************************************************
//
// Global variable used in tiger test case.
//
//*****************************************************************************
uint32_t g_ui32TotalRunTime[4] = 
{
    0, 0, 0, 0,
};

uint32_t g_ui32CrcResult[4] = 
{
    0, 0, 0, 0,
};

//*****************************************************************************
//
// Paint type list and shape type list used for shape_paint test case.
//
//*****************************************************************************
typedef enum
{
    PAINT_FILL,
    PAINT_LINEAR_GRADIENT,
    PAINT_CONIC_GRADIENT,
    PAINT_RADIAL_GRADIENT,
    PAINT_TEXTURE,
} paint_type_t;

paint_type_t paint_list[] =
{
    PAINT_FILL,
    PAINT_LINEAR_GRADIENT,
    PAINT_CONIC_GRADIENT,
    PAINT_RADIAL_GRADIENT,
    PAINT_TEXTURE,
};

typedef enum
{
    SHAPE_CIRCLE,
    SHAPE_ELLIPSE,
    SHAPE_RING,
    SHAPE_LINE,
    SHAPE_ROUNDED_RECT,
    SHAPE_RECT,
} shape_type_t;

shape_type_t shape_list[] = 
{
    SHAPE_CIRCLE,
    SHAPE_ELLIPSE,
    SHAPE_RING,
    SHAPE_LINE,
    SHAPE_ROUNDED_RECT,
    SHAPE_RECT,
};

const char* shape_list_string[] = 
{
    "circle",
    "ellipse",
    "ring",
    "line",
    "rounded_rectangle",
    "rectangle",
};

const char* paint_list_string[] = 
{
    "fill",
    "gradient_linear",
    "gradient_conic",
    "gradient_radial",
    "texture",
};

//*****************************************************************************
//
// CRC result.
//
//*****************************************************************************

#define TIGER_CRC 0x2F3F19D3UL
#define TIGER_CRC_ROTATE 0xA4B359CBUL

const uint32_t g_ui32ShapePaintCrc[5][6] = 
{
    {0xB9CF1301, 0x8A4A173F, 0x6B6403C9, 0xFFB40D4F, 0x6D20C136, 0xE5DC2DAF},
    {0xFCE4844F, 0x2755627C, 0x0EDF0F22, 0x84390749, 0x8E18F465, 0xD80AC044},
    {0xBEC1E1B2, 0x5F41F71B, 0x777C02A9, 0x894687FA, 0xDE8D79F4, 0x07EFBA52},
    {0x09A71EB4, 0xFDB39C4D, 0x64B96CDA, 0x6631EC31, 0x40887760, 0x6A4BF568},
    {0xA446A14A, 0x95E58598, 0x8F0DFA7A, 0xD76FADD6, 0xF7561A9A, 0xA05C5533},    
};

#define RAW_PATCH_CRC_H_EVEN_ODD 0x20C2F338UL
#define RAW_PATCH_CRC_e_EVEN_ODD 0xCB924F1AUL
#define RAW_PATCH_CRC_H_NON_ZERO 0x20C2F338UL
#define RAW_PATCH_CRC_e_NON_ZERO 0xCB924F1AUL

#define TTF_CRC_EVEN_ODD         0x75DCC922UL
#define TTF_CRC_STROKE           0x2A9CF18EUL

//*****************************************************************************
//
// Optional setup/tear-down functions.
//
// These will be called before and after each test function listed below.
//
//*****************************************************************************
void
setUp(void)
{
    // Create frame buffer
    uint32_t size = nema_texture_size(fb.format, 0, fb.w, fb.h);

    fb.bo = nema_buffer_create_pool(NEMA_MEM_POOL_FB, size);
    if ( fb.bo.base_virt == NULL )
    {
      am_util_stdio_printf("Frame buffer Malloc failed!\n");
      return;
    }

    // Init VG
    nema_vg_init(fb.w, fb.h);
    if( nema_get_error() != NEMA_ERR_NO_ERROR)
    {
        am_util_stdio_printf("nema_init failed!\n");
        return; 
    }

    if( nema_vg_get_error() != NEMA_VG_ERR_NO_ERROR)
    {
        am_util_stdio_printf("nema_vg_init failed!\n");
        return; 
    }
}

void
tearDown(void)
{
    // Deinit VG
    nema_vg_deinit();

    // Free frame buffer
    nema_buffer_destroy(&fb.bo);
}

//*****************************************************************************
//
// Get frame buffer CRC value.
//
//*****************************************************************************
uint32_t get_frame_buffer_crc(void)
{
    //Invalidate the framebuffer area.
    am_hal_cachectrl_range_t Range;
    Range.ui32Size = fb.bo.size;
    Range.ui32StartAddr = fb.bo.base_phys;
    am_hal_cachectrl_dcache_invalidate(&Range, false);

    uint32_t size = nema_texture_size(fb.format, 0, fb.w, fb.h);
    uint32_t result = crc32(fb.bo.base_virt, size);
    am_util_stdio_printf("CRC value =0x%08X\n", result);
    return result;
}

//*****************************************************************************
//
// Clear frame buffer.
//
//*****************************************************************************
void frame_buffer_clear(nema_cmdlist_t* cl)
{
    nema_cl_bind(cl);
    nema_cl_rewind(cl);

    nema_bind_dst_tex(fb.bo.base_phys, fb.w, fb.h, fb.format, -1);

    nema_set_clip(0, 0, fb.w, fb.h);

    nema_clear(0xffffffff);

    nema_cl_submit(cl);
    nema_cl_wait(cl);
}

//*****************************************************************************
//
// Set SVG image rotation and scale.
//
//*****************************************************************************
void set_scale_rotate(unsigned char *image, float rotate_degree, float scale_x, float scale_y)
{
    uint32_t svg_width, svg_height;
    float svg_w, svg_h;

    //Get image resolution
    nema_vg_get_tsvg_resolution(image, &svg_width, &svg_height);

    if (svg_width != 0U && svg_height != 0U)
    {
        svg_w = (float)svg_width;
        svg_h = (float)svg_height;
    }
    else
    {
        svg_w = (float)498; 
        svg_h = (float)600;
    }

    // Default scale
    float origin_scale_x = TIGER_X / svg_w;
    float origin_scale_y = TIGER_Y / svg_h;

    // User specified scale
    float display_scale_x = origin_scale_x * scale_x;
    float display_scale_y = origin_scale_y * scale_y;

    // Set matrix
    nema_matrix3x3_t matrix;
    nema_mat3x3_load_identity(matrix);
    nema_mat3x3_translate(matrix, -svg_w * 0.5f, -svg_h * 0.5f);
    nema_mat3x3_rotate(matrix, rotate_degree);
    nema_mat3x3_scale(matrix, display_scale_x, display_scale_y);
    nema_mat3x3_translate(matrix, fb.w / 2, fb.h / 2);

    nema_vg_set_global_matrix(matrix);
}

//*****************************************************************************
//
// Draw SVG image.
//
//*****************************************************************************
uint32_t draw_svg_image(nema_cmdlist_t* cl, unsigned char *image)
{

    float start_time = nema_get_time();

    // Draw the SVG
    nema_cl_bind_circular(cl);
    nema_cl_rewind(cl);

    nema_vg_draw_tsvg(image);

    nema_cl_submit(cl);
    nema_cl_wait(cl);

    float end_time = nema_get_time();

    // Get errorcode
    uint32_t  nema_error_code = nema_vg_get_error();
    if(nema_error_code != NEMA_VG_ERR_NO_ERROR)
    {
        am_util_stdio_printf("NemaSDK failed, errorcode =%8X\n", nema_error_code);
    }

    //Get time
    uint32_t time_used = (uint32_t)(( end_time - start_time ) * 1000.0f);
    return time_used;
}

//*****************************************************************************
//
// Set CPU and GPU power mode.
//
//*****************************************************************************
void set_cpu_power_mode(am_hal_pwrctrl_mcu_mode_e cpu_mode)
{

    // Set CPU power mode
    am_hal_pwrctrl_mcu_mode_select(cpu_mode);

    // Check and print the CPU and GPU power mode
    am_hal_pwrctrl_mcu_mode_e mode_status;
    am_hal_pwrctrl_mcu_mode_status(&mode_status);
    am_util_stdio_printf("CPU frequency: %dMHz\n", 
                         (mode_status == AM_HAL_PWRCTRL_MCU_MODE_LOW_POWER) ? 96 : 250);
}


//*****************************************************************************
//
// Common procedure for tiger test case.
//
//*****************************************************************************
void draw_tiger_common(void* texture_location, float rotate_degree, float scale_x, float scale_y)
{
    // Copy texture from MRAM to specified location
    if(texture_location != NULL)
    {
        nema_memcpy(texture_location, tiger_tsvg, tiger_tsvg_length);
    }
    
    am_util_stdio_printf("Texture location: 0X%08X\n",  (uint32_t)texture_location);

    // Create CL
    nema_cmdlist_t cl = nema_cl_create_sized(16*1024);
    if(cl.bo.base_virt == NULL)
    {
        am_util_stdio_printf("CL buffer Malloc failed!\n");
        return;    
    }

    for(uint32_t i = 0; i < 2; i++)
    {
        am_util_stdio_printf("\n\n");

        set_cpu_power_mode(g_sPowerMode[i]);

        // Clear frame buffer
        frame_buffer_clear(&cl);

        // Set transformation matrix
        set_scale_rotate(tiger_tsvg, rotate_degree, scale_x, scale_y);
        am_util_stdio_printf("rotation: %.2f \n", rotate_degree);
        am_util_stdio_printf("scale_x: %.2f \n", scale_x);
        am_util_stdio_printf("scale_y: %.2f \n", scale_y);

        // Draw SVG
        uint32_t run_time = draw_svg_image(&cl, tiger_tsvg);
        am_util_stdio_printf("--------Run time: %d ms--------------\n", run_time);
        g_ui32TotalRunTime[i] = run_time;

        // Get frame buffer CRC value
        g_ui32CrcResult[i] = get_frame_buffer_crc();

    }

    // Free CL
    nema_cl_destroy(&cl);

}

//*****************************************************************************
//
// Set gradient.
//
//*****************************************************************************
void set_grad(NEMA_VG_GRAD_HANDLE  gradient)
{
    float       stops[5]  = {0.0f, 0.25f, 0.50f, 0.75f, 1.0f};
    color_var_t colors[5] =
    {
        {0, 0, 255, 255},     // blue
        {0, 255, 0, 255},     // green
        {255, 255, 255, 255}, // white
        {255, 165, 0, 255},   // orange
        {255, 0, 0, 255}      // red
    };

    nema_vg_grad_set(gradient, 5, stops, colors);
}


//*****************************************************************************
//
// Set paint parameter.
//
//*****************************************************************************
void set_paint(NEMA_VG_PAINT_HANDLE paint, NEMA_VG_GRAD_HANDLE  gradient, paint_type_t paint_type)
{
    nema_matrix3x3_t m_paint;

    //
    // Clear the parameters of a paint object.
    //
    nema_vg_paint_clear(paint);

    //
    // Set up paint parameter
    //
    switch (paint_type)
    {
        case PAINT_FILL:
            nema_vg_paint_set_type(paint, NEMA_VG_PAINT_COLOR);
            nema_vg_paint_set_paint_color(paint,
                                          nema_rgba(0xff, 0x00, 0xff, 0x80));
            break;
        case PAINT_LINEAR_GRADIENT:
            nema_vg_paint_set_type(paint, NEMA_VG_PAINT_GRAD_LINEAR);
            nema_vg_paint_set_grad_linear(
                paint, gradient, 0, CENTER_POINT_Y - DRAW_SHAPE_AREA / 2,
                0, CENTER_POINT_Y + DRAW_SHAPE_AREA / 2,
                NEMA_TEX_CLAMP | NEMA_FILTER_BL);

            break;
        case PAINT_CONIC_GRADIENT:
            nema_vg_paint_set_type(paint, NEMA_VG_PAINT_GRAD_CONICAL);
            nema_vg_paint_set_grad_conical(paint, gradient,
                                           CENTER_POINT_X, CENTER_POINT_Y,
                                           NEMA_TEX_CLAMP | NEMA_FILTER_BL);
            break;
        case PAINT_RADIAL_GRADIENT:
            nema_vg_paint_set_type(paint, NEMA_VG_PAINT_GRAD_RADIAL);
            nema_vg_paint_set_grad_radial(paint, gradient, CENTER_POINT_X,
                                          CENTER_POINT_Y, DRAW_SHAPE_AREA / 2,
                                          NEMA_TEX_CLAMP | NEMA_FILTER_BL);
            break;
        case PAINT_TEXTURE:
            nema_mat3x3_load_identity(m_paint);
            nema_mat3x3_translate(m_paint, CENTER_POINT_X - g_sRef.w / 2.f,
                                  CENTER_POINT_Y - g_sRef.h / 2.f);

            nema_vg_paint_set_type(paint, NEMA_VG_PAINT_TEXTURE);
            nema_vg_paint_set_tex(paint, &g_sRef);
            nema_vg_paint_set_tex_matrix(paint, m_paint);
            break;
        default:
            break;
    }    
}

//*****************************************************************************
//
// Draw shape.
//
//*****************************************************************************
void draw_shape(shape_type_t shape, NEMA_VG_PAINT_HANDLE paint)
{
    switch(shape)
    {
        case SHAPE_CIRCLE:
            nema_vg_draw_circle(CENTER_POINT_X, CENTER_POINT_Y, 
                                DRAW_SHAPE_AREA / 2, NULL, paint);
        break;
        case SHAPE_ELLIPSE:
            nema_vg_draw_ellipse(CENTER_POINT_X, CENTER_POINT_Y, DRAW_SHAPE_AREA / 2,
                                DRAW_SHAPE_AREA / 4, NULL, paint);
        break;
        case SHAPE_RING:
            nema_vg_stroke_set_width(15.0f);
            nema_vg_draw_ring(CENTER_POINT_X, CENTER_POINT_Y, (DRAW_SHAPE_AREA / 2 - 30),
                              45, 340, paint);
        break;
        case SHAPE_LINE:
            nema_vg_stroke_set_width(15.0f);
            nema_vg_draw_line(CENTER_POINT_X - DRAW_SHAPE_AREA / 2 + 15, 
                              CENTER_POINT_Y - DRAW_SHAPE_AREA / 2 + 15, 
                              CENTER_POINT_X + DRAW_SHAPE_AREA / 2 - 15,
                              CENTER_POINT_Y + DRAW_SHAPE_AREA / 2 - 15,
                               NULL, paint);
        break;
        case SHAPE_ROUNDED_RECT:
            nema_vg_draw_rounded_rect(CENTER_POINT_X - DRAW_SHAPE_AREA / 2, 
                                      CENTER_POINT_Y - DRAW_SHAPE_AREA / 2  + 25,
                                      DRAW_SHAPE_AREA,
                                      DRAW_SHAPE_AREA - 50, 
                                      15,
                                      15, NULL, paint);
        break;
        case SHAPE_RECT:
            nema_vg_draw_rect(CENTER_POINT_X - DRAW_SHAPE_AREA / 2, 
                              CENTER_POINT_Y - DRAW_SHAPE_AREA / 2  + 25,
                              DRAW_SHAPE_AREA,
                              DRAW_SHAPE_AREA - 50, NULL, paint);
        break;
    }
}

//*****************************************************************************
//
// Common procedure to draw raw path.
//
//*****************************************************************************
uint32_t draw_raw_path_common(char char_to_draw, uint32_t fill_rule)
{

    am_util_stdio_printf("Draw character %c with %s\n", char_to_draw, 
                        (fill_rule == NEMA_VG_FILL_EVEN_ODD) ? "EVEN_ODD" : "NON_ZERO");

    //Create a large command list to hold all the command.
    nema_cmdlist_t cl  =  nema_cl_create_sized(128 * 1024);
    if(cl.bo.base_virt == NULL)
    {
        am_util_stdio_printf("CL buffer Malloc failed!\n");
        return 0;    
    }

    //
    // Clear frame buffer
    //
    frame_buffer_clear(&cl);

    //Bind Command List
    nema_cl_bind(&cl);
    nema_cl_rewind(&cl);  

    // Create PATH
    NEMA_VG_PATH_HANDLE path = nema_vg_path_create();
    const uint8_t* segs;
    const float* points;
    uint32_t num_segs;
    uint32_t num_points;
    if(char_to_draw == 'H')
    {
        segs = segs_H;
        points = points_H;
        num_segs = NUM_SEGS_H;
        num_points = NUM_COORDS_H;
    }
    else if(char_to_draw == 'e')
    {
        segs = segs_e;
        points = points_e;
        num_segs = NUM_SEGS_e;
        num_points = NUM_COORDS_e;
    }
    else
    {
        am_util_stdio_printf("Not supported char: %c\n", char_to_draw);
        return 0;
    }
    nema_vg_path_set_shape(path, num_segs, segs, num_points, points);

    // Set fill rule
    nema_vg_set_fill_rule(fill_rule);

    // Set quality
    nema_vg_set_quality(NEMA_VG_QUALITY_MAXIMUM);

    // Set blend mode
    nema_vg_set_blend(NEMA_BL_SRC_OVER);

    // Create paint
    NEMA_VG_PATH_HANDLE paint = nema_vg_paint_create();
    nema_vg_paint_set_type(paint, NEMA_VG_PAINT_COLOR);
    nema_vg_paint_set_paint_color(paint, 0xffffff00);

    // Draw character
    nema_matrix3x3_t matrix;

    float start_time = nema_get_time();

    for (int x = 0; x < 20; x++) {
      nema_mat3x3_load_identity(matrix);
      nema_mat3x3_translate(matrix, x * 10, 0);

      for (int i = 0; i < 10; i++) {
        nema_mat3x3_translate(matrix, 0, 20);
        nema_vg_set_global_matrix(matrix);
        nema_vg_draw_path(path, paint);
      }


        // Submit
        nema_cl_submit(&cl);

        //Wait and measure GPU time
        nema_cl_wait(&cl);

        nema_cl_rewind(&cl);

    }
    float end_time = nema_get_time();

    uint32_t nema_error_code = nema_vg_get_error();
    if(nema_error_code != NEMA_VG_ERR_NO_ERROR)
    {
        am_util_stdio_printf("NemaSDK failed, errorcode =0x%08X\n", nema_error_code);
    }

    uint32_t time_used = (uint32_t)(( end_time - start_time ) * 1000.0f);
    am_util_stdio_printf("Run time: %dms\n", time_used);

    // Clean up
    nema_vg_paint_destroy(paint);
    nema_vg_path_destroy(path);
    nema_cl_destroy(&cl);

    return time_used;
}

//*****************************************************************************
//
// Common procedure to draw ttf font.
//
//*****************************************************************************
uint32_t draw_ttf_common(uint32_t fill_rule)
{
    //UTF8 encoded string for "龍龍龍龍龍"
    const char str[16] = {0xe9, 0xbe, 0x8d, 
                           0xe9, 0xbe, 0x8d,
                           0xe9, 0xbe, 0x8d,
                           0xe9, 0xbe, 0x8d,
                           0xe9, 0xbe, 0x8d,
                           0x0};

    am_util_stdio_printf("Draw TTF font with %s\n", 
                        (fill_rule == NEMA_VG_FILL_EVEN_ODD) ? "EVEN_ODD" : "STROKE");

    //Create a large command list to hold all the command.
    nema_cmdlist_t cl  =  nema_cl_create_sized(256 * 1024);
    if(cl.bo.base_virt == NULL)
    {
        am_util_stdio_printf("CL buffer Malloc failed!\n");
        return 0;    
    }

    //
    // Clear frame buffer
    //
    frame_buffer_clear(&cl);

    //Bind Command List
    nema_cl_bind(&cl);
    nema_cl_rewind(&cl);  

    // Set fill rule
    nema_vg_set_fill_rule(fill_rule);

    // Set quality
    nema_vg_set_quality(NEMA_VG_QUALITY_MAXIMUM);

    // Set blend mode
    nema_vg_set_blend(NEMA_BL_SRC_OVER);

    // Create paint
    NEMA_VG_PATH_HANDLE paint = nema_vg_paint_create();
    nema_vg_paint_set_type(paint, NEMA_VG_PAINT_COLOR);
    nema_vg_paint_set_paint_color(paint, 0xFFFFFF00);
    nema_vg_stroke_set_width(2.f);

    // Reset global matrix
    nema_vg_reset_global_matrix();

    //Bind font
    nema_vg_bind_font(&ume_ugo5_ttf);

    // Set font size
    nema_vg_set_font_size(20);

    // Draw ttf
    nema_vg_print(paint, str, 100, 100, 300, 300,
                  NEMA_VG_ALIGNX_LEFT | NEMA_VG_TEXT_WRAP | NEMA_VG_ALIGNY_TOP,
                  NULL);

    // Submit
    nema_cl_submit(&cl);

    //Wait and measure GPU time
    float start_time = nema_get_time();
    nema_cl_wait(&cl);
    float end_time = nema_get_time();

    uint32_t nema_error_code = nema_vg_get_error();
    if(nema_error_code != NEMA_VG_ERR_NO_ERROR)
    {
        am_util_stdio_printf("NemaSDK failed, errorcode =%8X\n", nema_error_code);
    }

    uint32_t time_used = (uint32_t)(( end_time - start_time ) * 1000.0f);
    am_util_stdio_printf("Run time: %dms\n", time_used);

    // Clean up
    nema_vg_paint_destroy(paint);
    nema_cl_destroy(&cl);

    return time_used;
}


void 
nemagfx_vg_tiger_psram_test(void)
{
#ifdef USE_PSRAM

    am_util_stdio_printf("\n\n\n");
    am_util_stdio_printf("***********************************************************\n");
    am_util_stdio_printf("Draw tiger with texture located in PSRAM.\n");

    //
    // Copy texture from MRAM to PSRAM
    //
    draw_tiger_common((void*)MSPI_XIP_BASE_ADDRESS, 0.f, 1.f, 1.f);

    //
    // Pixel compare
    //
    TEST_ASSERT( g_ui32CrcResult[0] == TIGER_CRC );
    TEST_ASSERT( g_ui32CrcResult[1] == TIGER_CRC );
    TEST_ASSERT( g_ui32CrcResult[2] == TIGER_CRC );
    TEST_ASSERT( g_ui32CrcResult[3] == TIGER_CRC );

    //
    // Run time compare
    //

    //
    // Banner
    //
    am_util_stdio_printf("\n***********************************************************\n"); 

#else
    TEST_IGNORE();
#endif
}


void 
nemagfx_vg_tiger_ssram_test(void)
{
    am_util_stdio_printf("\n\n\n");
    am_util_stdio_printf("***********************************************************\n");
    am_util_stdio_printf("Draw tiger with texture located in SSRAM.\n");

    //
    // Copy texture from MRAM to SSRAM
    //
    nema_buffer_t tiger_bo = nema_buffer_create_pool(NEMA_MEM_POOL_ASSETS, tiger_tsvg_length);
    draw_tiger_common(tiger_bo.base_virt, 0.f, 1.f, 1.f);

    //
    // Content compare
    //
    TEST_ASSERT( g_ui32CrcResult[0] == TIGER_CRC );
    TEST_ASSERT( g_ui32CrcResult[1] == TIGER_CRC );
    TEST_ASSERT( g_ui32CrcResult[2] == TIGER_CRC );
    TEST_ASSERT( g_ui32CrcResult[3] == TIGER_CRC );

    //
    // Criteria
    //

    //
    // Free buffer
    //
    nema_buffer_destroy(&tiger_bo);

    //
    // Banner
    //
    am_util_stdio_printf("\n***********************************************************\n"); 
}

void 
nemagfx_vg_tiger_rotation_test(void)
{
    am_util_stdio_printf("\n\n\n");
    am_util_stdio_printf("***********************************************************\n");
    am_util_stdio_printf("Draw tiger with rotation effect.\n");

    //
    // Copy texture from MRAM to SSRAM
    //
    nema_buffer_t tiger_bo = nema_buffer_create_pool(NEMA_MEM_POOL_ASSETS, tiger_tsvg_length);
    draw_tiger_common(tiger_bo.base_virt, 45.f, 1.f, 1.f);

    //
    // Content compare
    //
    TEST_ASSERT( g_ui32CrcResult[0] == TIGER_CRC_ROTATE );
    TEST_ASSERT( g_ui32CrcResult[1] == TIGER_CRC_ROTATE );
    TEST_ASSERT( g_ui32CrcResult[2] == TIGER_CRC_ROTATE );
    TEST_ASSERT( g_ui32CrcResult[3] == TIGER_CRC_ROTATE );

    //
    // Criteria
    //

    //
    // Free buffer
    //
    nema_buffer_destroy(&tiger_bo);

    //
    // Banner
    //
    am_util_stdio_printf("\n***********************************************************\n"); 
}

//Test all the predefined shapes and all the paint types
void 
nemagfx_vg_shape_paint_test(void)
{
    float start_time;
    float end_time;
    uint32_t  nema_error_code;
    uint32_t time_used;

    am_util_stdio_printf("\n\n\n");
    am_util_stdio_printf("***********************************************************\n");
    am_util_stdio_printf("Draw VG predefined shapes with different paint type!\n");


    //Create a large command list to hold all the command.
    nema_cmdlist_t cl  =  nema_cl_create_sized(128 * 1024);
    if(cl.bo.base_virt == NULL)
    {
        am_util_stdio_printf("CL buffer Malloc failed!\n");
        return;    
    }

    // Alloc textures buffer in SSRAM
    g_sRef.bo = nema_buffer_create_pool(NEMA_MEM_POOL_ASSETS, ref_rgba_len);
    // load image data
    nema_memcpy(g_sRef.bo.base_virt, ref_rgba, ref_rgba_len);

    //
    // Create paint and gradient
    //
    NEMA_VG_PAINT_HANDLE paint = nema_vg_paint_create();
    NEMA_VG_GRAD_HANDLE  gradient = nema_vg_grad_create();

    for(uint32_t i=0; i<sizeof(paint_list)/sizeof(paint_list[0]); i++)
    {
        

        for(uint32_t j=0; j<sizeof(shape_list)/sizeof(shape_list[0]); j++)
        {
            am_util_stdio_printf("\n\n");
            am_util_stdio_printf("shape: %s \n", shape_list_string[j]);
            am_util_stdio_printf("paint: %s \n", paint_list_string[i]);

            //
            // Clear frame buffer
            //
            frame_buffer_clear(&cl);

            nema_cl_bind(&cl);
            nema_cl_rewind(&cl);

            //
            // Set vg context
            //
            nema_vg_set_fill_rule(NEMA_VG_FILL_EVEN_ODD);
            nema_vg_set_blend(NEMA_BL_SRC_OVER);

            //
            // Set gradient parameter
            //
            set_grad(gradient);

            //
            // Set paint parameter
            //            
            set_paint(paint, gradient, paint_list[i]);

            //
            // Draw shape
            //              
            draw_shape(shape_list[j], paint);

            nema_cl_submit(&cl);

            start_time = nema_get_time();
            nema_cl_wait(&cl);
            end_time = nema_get_time();

            nema_error_code = nema_vg_get_error();
            if(nema_error_code != NEMA_VG_ERR_NO_ERROR)
            {
                am_util_stdio_printf("NemaSDK failed, errorcode =%8X\n", nema_error_code);
            }

            time_used = (uint32_t)(( end_time - start_time ) * 1000.0f);
            am_util_stdio_printf("Run time: %dms\n", time_used);

            //
            // Content compare
            //
            TEST_ASSERT(get_frame_buffer_crc() == g_ui32ShapePaintCrc[i][j]);
        
            //
            // Criteria
            //           

        }

        
    }

    nema_vg_grad_destroy(gradient);
    nema_vg_paint_destroy(paint);
    nema_cl_destroy(&cl);

    //
    // Banner
    //
    am_util_stdio_printf("\n***********************************************************\n"); 
}

//Draw raw path, the char 'H' and 'e' in raw segment format
void
nemagfx_vg_raw_path_H_even_odd_test(void)
{
    am_util_stdio_printf("\n\n\n");
    am_util_stdio_printf("***********************************************************\n");

    //
    draw_raw_path_common('H', NEMA_VG_FILL_EVEN_ODD);

    //
    // Content compare
    //
    TEST_ASSERT(get_frame_buffer_crc() == RAW_PATCH_CRC_H_EVEN_ODD);
    
    //
    // Criteria
    //

    //
    // Banner
    //
    am_util_stdio_printf("\n***********************************************************\n"); 
}

//Draw raw path, the char 'H' and 'e' in raw segment format
void
nemagfx_vg_raw_path_e_even_odd_test(void)
{
    am_util_stdio_printf("\n\n\n");
    am_util_stdio_printf("***********************************************************\n");

    //
    draw_raw_path_common('e', NEMA_VG_FILL_EVEN_ODD);

    //
    // Content compare
    //
    TEST_ASSERT(get_frame_buffer_crc() == RAW_PATCH_CRC_e_EVEN_ODD);
    
    //
    // Criteria
    // 

    //
    // Banner
    //
    am_util_stdio_printf("\n***********************************************************\n");
}

//Draw raw path, the char 'H' and 'e' in raw segment format
void
nemagfx_vg_raw_path_H_non_zero_test(void)
{
    am_util_stdio_printf("\n\n\n");
    am_util_stdio_printf("***********************************************************\n");

    //
    draw_raw_path_common('H', NEMA_VG_FILL_NON_ZERO);

    //
    // Content compare
    //
    TEST_ASSERT(get_frame_buffer_crc() == RAW_PATCH_CRC_H_NON_ZERO);
    
    //
    // Criteria
    // 

    //
    // Banner
    //
    am_util_stdio_printf("\n***********************************************************\n");
}

//Draw raw path, the char 'H' and 'e' in raw segment format
void
nemagfx_vg_raw_path_e_non_zero_test(void)
{
    am_util_stdio_printf("\n\n\n");
    am_util_stdio_printf("***********************************************************\n");

    //
    draw_raw_path_common('e', NEMA_VG_FILL_NON_ZERO);

    //
    // Content compare
    //
    TEST_ASSERT(get_frame_buffer_crc() == RAW_PATCH_CRC_e_NON_ZERO);
    
    //
    // Criteria
    // 

    //
    // Banner
    //
    am_util_stdio_printf("\n***********************************************************\n");
}



//Test TTF font, the character 
void
nemagfx_vg_ttf_dragon_fill_test(void)
{
    am_util_stdio_printf("\n\n\n");
    am_util_stdio_printf("***********************************************************\n");

    //
    draw_ttf_common(NEMA_VG_FILL_EVEN_ODD);

    //
    // Content compare
    //
    TEST_ASSERT(get_frame_buffer_crc() == TTF_CRC_EVEN_ODD);
    
    //
    // Criteria
    // 

    //
    // Banner
    //
    am_util_stdio_printf("\n***********************************************************\n");
}

//Test TTF font, the character
void  
nemagfx_vg_ttf_dragon_stroke_test(void)
{
    am_util_stdio_printf("\n\n\n");
    am_util_stdio_printf("***********************************************************\n");

    //
    draw_ttf_common(NEMA_VG_STROKE);

    //
    // Content compare
    //
    TEST_ASSERT(get_frame_buffer_crc() == TTF_CRC_STROKE);
    
    //
    // Criteria
    // 

    //
    // Banner
    //
    am_util_stdio_printf("\n***********************************************************\n");
}