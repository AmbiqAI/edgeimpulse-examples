//*****************************************************************************
//
//! @file render_vg_font.c
//!
//! @brief draw vg font.
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

#ifdef RUN_RENDER_VG_FONT
#include "DejaVuSans_ttf_kern.h"

//*****************************************************************************
//
// Macro definitions
//
//*****************************************************************************
#define VECTOR_FONT DejaVuSans_ttf_kern

#define RESX (452)
#define RESY (452)

#define SECTOR_SPACING (10)
#define SECTOR_WIDTH ( (RESX - SECTOR_SPACING) / 2 )
#define SECTOR_HIGHT ( (RESY - SECTOR_SPACING) / 2 )
#define SECTOR_BACKGROUND_COLOR (0x70707070U)

#define FONT_SIZE (14)


//*****************************************************************************
//
// Global Variables
//
//*****************************************************************************
//! Frame buffer
static nema_img_obj_t g_sFrameBuffer =
{
    {0}, RESX, RESY, RESX*4, NEMA_RGBA8888
};

//! DC layer
static nemadc_layer_t g_sDCLayer =
{
    (void *)0, 0, RESX, RESY, RESX*4, 0, 0, RESX, RESY, 0xff,
    NEMADC_BL_SRC, 0, NEMADC_RGBA8888, 0, 0, 0, 0, 0, 0, 0, 0
};

//*****************************************************************************
//
//! @brief Create frame buffer and load the font from MRAM to ssram
//!
//
//*****************************************************************************
void bufferCreate(void)
{
    //Create framebuffer in SSRAM
    g_sFrameBuffer.bo = nema_buffer_create_pool(NEMA_MEM_POOL_FB, g_sFrameBuffer.stride*g_sFrameBuffer.h);
    nema_buffer_map(&g_sFrameBuffer.bo);

    //Create buffer for font in SSRAM and load them from MRAM to ssram
    nema_buffer_t font_data = nema_buffer_create_pool(NEMA_MEM_POOL_ASSETS, DejaVuSans_ttf_kern.data_length*4);
    nema_memcpy(font_data.base_virt, DejaVuSans_ttf_kern.data, DejaVuSans_ttf_kern.data_length*4);
    DejaVuSans_ttf_kern.data = font_data.base_virt;

    //Set DC layer
    g_sDCLayer.baseaddr_phys = g_sFrameBuffer.bo.base_phys;
    g_sDCLayer.baseaddr_virt = g_sFrameBuffer.bo.base_virt;
}

//*****************************************************************************
//
//! @brief Render vg font
//
//*****************************************************************************
int render_vg_font(void)
{
    //Test string
    char str[] = "AMBIQ - Hello World!!!\n"
                  "|----*******----!!!!!----------|\n"
                  "TEST TEST !\n"
                  "ABCDEFGhijklmnopqrestuvweeeeeeeeeeeeeeeeeeeeeeeeexyzd\n"
                  "H NemaGFX υποστηρίζει ΚΑΙ Unicode χαρακτήρες!!!";

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

    //Create buffer
    bufferCreate();

    //Set layer
    nemadc_set_layer(0, &g_sDCLayer);

    //Create Command List
    nema_cmdlist_t cl  =  nema_cl_create_sized(10*1024);
    //Bind Command List
    nema_cl_bind_circular(&cl);

    //Set Framebuffer
    nema_bind_dst_tex(g_sFrameBuffer.bo.base_phys,
                      g_sFrameBuffer.w,
                      g_sFrameBuffer.h,
                      g_sFrameBuffer.format,
                      g_sFrameBuffer.stride);

    //Set Clipping Rectangle
    nema_set_clip(0, 0, RESX, RESY);

    //Clear frame buffer
    nema_clear(0x00000000); //black

    //Create and set paint
    NEMA_VG_PAINT_HANDLE paint =  nema_vg_paint_create();
    nema_vg_paint_set_type(paint, NEMA_VG_PAINT_COLOR);
    nema_vg_stroke_set_width(2.f);

    //bind font
    nema_vg_bind_font(&VECTOR_FONT);

    //set font size
    nema_vg_set_font_size(FONT_SIZE);

    //blend mode
    nema_set_blend_fill(NEMA_BL_SIMPLE);

    //section offset
    int x_start;
    int y_start;

    //******************************LEFT TOP**********************************/
    x_start = 0;
    y_start = 0;

    //blend mode
    nema_set_blend_fill(NEMA_BL_SIMPLE);

    //Set backgound
    nema_set_clip(x_start, y_start, SECTOR_WIDTH, SECTOR_HIGHT);
    nema_fill_rect(x_start, y_start, SECTOR_WIDTH, SECTOR_HIGHT, SECTOR_BACKGROUND_COLOR);

    //Draw font
    nema_vg_paint_set_paint_color(paint, nema_rgba(255, 255, 0, 255 ));
    nema_vg_print(paint, str, x_start, y_start, SECTOR_WIDTH, SECTOR_HIGHT,
                  NEMA_VG_ALIGNX_LEFT | NEMA_VG_TEXT_WRAP | NEMA_VG_ALIGNY_TOP,
                  NULL);

    //******************************RIGHT TOP**********************************/
    x_start = SECTOR_WIDTH + SECTOR_SPACING / 2;
    y_start = 0;

    //blend mode
    nema_set_blend_fill(NEMA_BL_SIMPLE);

    //Set backgound
    nema_set_clip(x_start, y_start, SECTOR_WIDTH, SECTOR_HIGHT);
    nema_fill_rect(x_start, y_start, SECTOR_WIDTH, SECTOR_HIGHT, SECTOR_BACKGROUND_COLOR);

    //Draw font
    nema_vg_paint_set_paint_color(paint, nema_rgba(255, 0, 0, 255 ));
    nema_vg_print(paint, str, x_start, y_start, SECTOR_WIDTH, SECTOR_HIGHT,
                  NEMA_VG_ALIGNX_RIGHT | NEMA_VG_TEXT_WRAP | NEMA_VG_ALIGNY_BOTTOM,
                  NULL);

    //******************************LEFT BOTTOM**********************************/
    x_start = 0;
    y_start = SECTOR_HIGHT + SECTOR_SPACING / 2;

    //blend mode
    nema_set_blend_fill(NEMA_BL_SIMPLE);

    //Set backgound
    nema_set_clip(x_start, y_start, SECTOR_WIDTH, SECTOR_HIGHT);
    nema_fill_rect(x_start, y_start, SECTOR_WIDTH, SECTOR_HIGHT, SECTOR_BACKGROUND_COLOR);

    //Draw font
    nema_vg_paint_set_paint_color(paint, nema_rgba(0, 255, 255, 255 ));
    nema_vg_print(paint, str, x_start, y_start, SECTOR_WIDTH, SECTOR_HIGHT,
                  NEMA_VG_ALIGNX_CENTER | NEMA_VG_TEXT_WRAP | NEMA_VG_ALIGNY_CENTER,
                  NULL);

    //******************************RIGHT BOTTOM**********************************/
    x_start = SECTOR_WIDTH + SECTOR_SPACING / 2;
    y_start = SECTOR_HIGHT + SECTOR_SPACING / 2;

    //blend mode
    nema_set_blend_fill(NEMA_BL_SIMPLE);

    //Set backgound
    nema_set_clip(x_start, y_start, SECTOR_WIDTH, SECTOR_HIGHT);
    nema_fill_rect(x_start, y_start, SECTOR_WIDTH, SECTOR_HIGHT, SECTOR_BACKGROUND_COLOR);

    //Draw font
    nema_vg_paint_set_paint_color(paint, nema_rgba(0, 255, 0, 255 ));
    nema_vg_print(paint, str, x_start, y_start, SECTOR_WIDTH, SECTOR_HIGHT,
                  NEMA_VG_ALIGNX_JUSTIFY | NEMA_VG_TEXT_WRAP | NEMA_VG_ALIGNY_JUSTIFY,
                  NULL);

    //**************************************************************************/
    nema_cl_submit(&cl);
    nema_cl_wait(&cl);

    //Start DC
    display_refresh_start();
    //Wait DC complete interrupt.
    nemadc_wait_vsync();
    //Do follow-up operations required by hardware.
    display_refresh_end();

    //Destroy (free) Command List
    nema_cl_destroy(&cl);

    //de-initialize nema_vg
    nema_vg_paint_destroy(paint);
    nema_vg_deinit();

    return 0;
}

#endif
