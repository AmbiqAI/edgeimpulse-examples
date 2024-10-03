//*****************************************************************************
//
//! @file test_common.c
//!
//! @brief Common procedure to run the test items.
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
#include "nemagfx_performance_test.h"

#if (DISP_CTRL_IP == DISP_CTRL_IP_MSPI)
#error "this example contains several pixel format that MSPI driving display couldn't support."
#endif

//
// Textures
//
#include "texture/img_468x468_tsc6.h"
#include "texture/boy_466_466_rgb24.h"
#include "texture/boy_466_466_rgba565.h"
// #include "texture/boy_468_466.z_rgb24.h"
// #include "texture/boy_468_466.z_rgba.h"
// #include "texture/color_wheel_466_466_rgba.h"
#include "texture/hands_20_250_morton.z_rgba.h"

//
// fonts
//
#include "texture/simhei34pt4b.h"
#include "texture/simhei30pt8b.h"
#include "texture/simhei30pt4b.h"

//
// vector texture/font
//
#include "texture/ume_ugo5_ttf.h"
#include "texture/tiger.tsvg.h"

//
// Display
//
#include "display.h"

//
// Test configuration, select one of them
//
#include "config_base.h"
//#include "config_scale.h"
//#include "config_rotate.h"
//#include "config_scale_rotate.h"
//#include "config_morton.h"
//#include "config_write_psram.h"
//#include "config_read_psram.h"

//*****************************************************************************
//
// Macro definitions
//
//*****************************************************************************
#define FB_RESX 468
#define FB_RESY 468


#define TEST_ITEM_TOTAL  (sizeof(g_sTestItemList)/sizeof(TestItem_t))

#define PSRAM_FB_ADDR (MSPI_XIP_BASE_ADDRESS)
#define PSRAM_FB_SIZE (FB_RESX * FB_RESY * 4)

#define PSRAM_TEX_ADDR (((PSRAM_FB_ADDR + PSRAM_FB_SIZE + 7) >> 3) << 3)

#define SSRAM_FB_ADDR (0x20100040)
#define SSRAM_FB_SIZE (FB_RESX * FB_RESY * 4)

#define SSRAM_TEX_ADDR (((SSRAM_FB_ADDR + SSRAM_FB_SIZE + 7) >> 3) << 3)

//
// Source/Destination location strings
//
const char* g_pui8LocationStr[] =
{
   "NO_USED", "SSRAM", "PSRAM",
};

//
// Test type strings
//
const char* g_pui8TestTypeStr[] =
{
    "Unknown", "Fill", "Copy", "Blend", "Blend_Part", "Blend_Global_Alpha", "Blend_Font", "Scale", "Rotate", "Scale_Rotate", "Perspective", "Gaussian_Blur", "Vector_Graphics", "Vector_Graphics_Font"
};

//*****************************************************************************
//
//! @brief Convert source/destination color format to string
//!
//! @return char pointer.
//
//*****************************************************************************
char *
format2string(nema_tex_format_t sFormat)
{
    char* ui8Str;

    switch (sFormat)
    {
        case NEMA_ARGB8888:
            ui8Str = "ARGB8888";
            break;
        case NEMA_RGBA8888:
            ui8Str = "RGBA8888";
            break;
        case NEMA_XRGB8888:
            ui8Str = "XRGB8888";
            break;
        case NEMA_RGB24:
            ui8Str = "RGB888";
            break;
        case NEMA_RGB565:
            ui8Str = "RGB565";
            break;
        case NEMA_TSC6:
            ui8Str = "TSC6";
            break;
        case NEMA_A4:
            ui8Str = "A4";
            break;
       case NEMA_A8:
            ui8Str = "A8";
            break;
        default:
            ui8Str = "unknown";
            break;
    }

    return ui8Str;
}

//*****************************************************************************
//
//! @brief load texture to PSRAM or SSRAM
//!
//! @return uintptr_t pointer.
//
//*****************************************************************************
uintptr_t
load_texture(const TestItem_t* psTestItem)
{
    uintptr_t texture = (uintptr_t)NULL;

    switch(psTestItem->eTEXLocation)
    {
        case LT_NO_USED:
            break;
        case LT_SSRAM:
            texture = SSRAM_TEX_ADDR;
            break;
        case LT_PSRAM:
            texture = PSRAM_TEX_ADDR;
            break;
        default:
            break;

    }
    if ( texture == (uintptr_t)NULL )
    {
        return texture;
    }

    switch (psTestItem->sTexFormat)
    {
        case NEMA_RGBA8888:
            if ( (psTestItem->ui32SrcWidth == 466) && (psTestItem->ui32SrcHight == 466) )
            {
                if ( (psTestItem->enable_morton) )
                {
//                    nema_memcpy((void*)texture,boy_468_466_rgba,sizeof(boy_468_466_rgba));
                    memset((void*)texture, 0xff, 468*466*4);
                }
                else
                {
                    //nema_memcpy((void*)texture,color_wheel_466_466_rgba,sizeof(color_wheel_466_466_rgba));
                    memset((void*)texture, 0xff, 466*466*4);
                }
            }
            else if ( (psTestItem->ui32SrcWidth == 20) && (psTestItem->ui32SrcHight == 250) )
            {
                if ( (psTestItem->enable_morton) )
                {
                    nema_memcpy((void*)texture, hands_20_250_morton, sizeof(hands_20_250_morton));
                }
                else
                {
                    memset((void*)texture, 0xFF, 20 * 250 * 4);
                }
            }
            else
            {
                memset((void*)texture, 0xFF, psTestItem->ui32SrcWidth * psTestItem->ui32SrcHight * 4);
            }
            break;
        case NEMA_RGB24:
            if ( (psTestItem->ui32SrcWidth == 466) && (psTestItem->ui32SrcHight == 466) )
            {
                if ( (psTestItem->enable_morton) )
                {
                    //nema_memcpy((void*)texture,boy_468_466_rgb24,sizeof(boy_468_466_rgb24));
                    memset((void*)texture, 0xff, 468 * 466 * 3);
                }
                else
                {
                    nema_memcpy((void*)texture, boy_466_466_rgb24, sizeof(boy_466_466_rgb24));
                }
            }
            else
            {
                memset((void*)texture, 0xFF, psTestItem->ui32SrcWidth * psTestItem->ui32SrcHight*3);
            }
            break;
        case NEMA_RGB565:
            if ( (psTestItem->ui32SrcWidth == 466) && (psTestItem->ui32SrcHight == 466) )
            {
                if ( (psTestItem->enable_morton) )
                {
                    texture = (uintptr_t)NULL;
                }
                else
                {
                    nema_memcpy((void*)texture, boy_466_466_rgb565, sizeof(boy_466_466_rgb565));
                }
            }
            else
            {
                memset((void*)texture, 0xFF, psTestItem->ui32SrcWidth * psTestItem->ui32SrcHight*2);
            }
            break;
        case NEMA_TSC6:
            nema_memcpy((void*)texture, img_468x468_tsc6, sizeof(img_468x468_tsc6));
            break;
        default:
            texture = (uintptr_t)NULL;
            break;
    }

    //
    // flush cache for memory consistency.
    //
    am_hal_cachectrl_dcache_invalidate(NULL, true);

    return texture;
}

//*****************************************************************************
//
//! @brief load font to PSRAM or SSRAM
//!
//! @return nema_font_t pointer.
//
//*****************************************************************************
nema_font_t*
load_font(const TestItem_t* psTestItem)
{
    nema_font_t* psFont = NULL;
    uintptr_t texture = (uintptr_t)NULL;

    switch(psTestItem->eTEXLocation)
    {
        case LT_NO_USED:
            break;
        case LT_SSRAM:
            texture = SSRAM_TEX_ADDR;
            break;
        case LT_PSRAM:
            texture = PSRAM_TEX_ADDR;
            break;
        default:
            break;

    }
    if ( texture == (uintptr_t)NULL )
    {
        return psFont;
    }
    switch(psTestItem->sTexFormat)
    {
        case NEMA_A4:
            if ( psTestItem->ui32DesHight == 36 )
            {
                nema_memcpy((void*)texture, simhei30pt4b.bitmap, simhei30pt4b.bitmap_size);
                simhei30pt4b.bo.base_phys = texture ;
                simhei30pt4b.bo.base_virt = (void*)simhei30pt4b .bo.base_phys;
                psFont = &simhei30pt4b ;
            }
            else if ( psTestItem->ui32DesHight == 40 )
            {
                nema_memcpy((void*)texture, simhei34pt4b.bitmap, simhei34pt4b.bitmap_size);
                simhei34pt4b.bo.base_phys = texture ;
                simhei34pt4b.bo.base_virt = (void*)simhei34pt4b .bo.base_phys;
                psFont = &simhei34pt4b ;
            }
            break;
        case NEMA_A8:
            nema_memcpy((void*)texture, simhei30pt8b.bitmap, simhei30pt8b.bitmap_size);
            simhei30pt8b.bo.base_phys = texture ;
            simhei30pt8b.bo.base_virt = (void*)simhei30pt8b .bo.base_phys;
            psFont = &simhei30pt8b ;
            break;
        default:
            break;
    }
    return psFont;
}

//*****************************************************************************
//
//! @brief load vector image to PSRAM or SSRAM
//!
//! @return uintptr_t pointer.
//
//*****************************************************************************
uintptr_t
load_vg_svg(const TestItem_t* psTestItem)
{
    uintptr_t texture = (uintptr_t)NULL;

    switch(psTestItem->eTEXLocation)
    {
        case LT_NO_USED:
            break;
        case LT_SSRAM:
            texture = SSRAM_TEX_ADDR;
            break;
        case LT_PSRAM:
            texture = PSRAM_TEX_ADDR;
            break;
        default:
            break;

    }
    if ( texture == (uintptr_t)NULL )
    {
        return texture;
    }

    nema_memcpy((void*)texture, tiger_tsvg, tiger_tsvg_length);

    return texture;
}

//*****************************************************************************
//
//! @brief load TTF font to PSRAM or SSRAM
//!
//! @return nema_font_t pointer.
//
//*****************************************************************************
nema_vg_font_t*
load_vg_font(const TestItem_t* psTestItem)
{
    nema_vg_font_t* psFont = NULL;
    uintptr_t texture = (uintptr_t)NULL;

    switch(psTestItem->eTEXLocation)
    {
        case LT_NO_USED:
            break;
        case LT_SSRAM:
            texture = SSRAM_TEX_ADDR;
            break;
        case LT_PSRAM:
            texture = PSRAM_TEX_ADDR;
            break;
        default:
            break;

    }
    if ( texture == (uintptr_t)NULL )
    {
        return psFont;
    }

    nema_memcpy((void*)texture, ume_ugo5_ttf.data, ume_ugo5_ttf.data_length*sizeof(float));
    ume_ugo5_ttf.data = (void*)texture;

    psFont = &ume_ugo5_ttf;
    return psFont;
}


//*****************************************************************************
//
//! @brief Init framebuffer and dc layer
//!
//! @return int32_t.
//
//*****************************************************************************
int32_t
init_framebuffer_layer(const TestItem_t* psTestItem, nemadc_layer_t* psLayer, img_obj_t* psFB)
{
    psFB->w = FB_RESX;
    psFB->h = FB_RESY;
    psFB->format = psTestItem->sFBFormat;

    //It is safe to ignore the wrap parameter for frame buffer in this example.
    psFB->stride = nema_stride_size(psFB->format, 0, psFB->w);


    //
    // Alloc frame buffer space
    //
    switch (psTestItem->eFBLocation)
    {
        case LT_SSRAM:
            psFB->bo.base_phys = SSRAM_FB_ADDR;
            psFB->bo.base_virt = (void*)psFB->bo.base_phys;
            psFB->bo.size = nema_texture_size(psFB->format, 0, psFB->w, psFB->h);
            break;
        case LT_PSRAM:
            psFB->bo.base_phys = PSRAM_FB_ADDR;
            psFB->bo.base_virt = (void*)psFB->bo.base_phys;
            psFB->bo.size = nema_texture_size(psFB->format, 0, psFB->w, psFB->h);
            break;
        default:
            am_util_stdio_printf("Unkown frame buffer location!\n");
            psFB->bo.base_phys = (uintptr_t)NULL;
            psFB->bo.base_virt = (void*)psFB->bo.base_phys;
            break;
    }

    if ((void*)psFB->bo.base_phys == NULL)
    {
        am_util_stdio_printf("frame buffer malloc failed!\n");
        return -1;
    }

    psLayer->startx        = 0;
    psLayer->sizex         = psFB->w;
    psLayer->resx          = psFB->w;
    psLayer->starty        = 0;
    psLayer->sizey         = psFB->h;
    psLayer->resy          = psFB->h;
    psLayer->stride        = psFB->stride;
    psLayer->format        = psTestItem->ui32DCFormat;
    psLayer->blendmode     = NEMADC_BL_SRC;
    psLayer->baseaddr_phys = psFB->bo.base_phys;
    psLayer->baseaddr_virt = psFB->bo.base_virt;
    psLayer->buscfg        = 0;
    psLayer->alpha         = 0xff;
    psLayer->flipx_en      = 0;
    psLayer->flipy_en      = 0;

    return 0;
}

//*****************************************************************************
//
//! @brief Run performance test
//!
//! @return 0: success, -1: fail.
//
//*****************************************************************************
int32_t
nemagfx_performance_test(void)
{
    int32_t i;
    static img_obj_t sFB;
    static nemadc_layer_t sLayer = {0};
    const TestItem_t* psTestItem;
    float time_start;
    float time_end;
    float time_submit;
    double time_used;
    double speed_pps;
    uint32_t ui32StartX;
    uint32_t ui32StartY;

    char ui8Str[] = "龍龍龍龍龍";
    uint8_t ui8Alpha = 0x77;


    static nema_cmdlist_t sCL;
    static nema_cmdlist_t sCL_clear;
    static nema_cmdlist_t sCL_NON_VG;
    nema_cmdlist_t *pCurrentCL;

    NEMA_VG_PAINT_HANDLE paint = NULL;

    //
    // Create CL
    //
    sCL = nema_cl_create_sized(16*1024);
    sCL_clear = nema_cl_create_sized(128);
    sCL_NON_VG = nema_cl_create_sized(512);

    //
    // init vector graphics
    //
    nema_vg_init(FB_RESX, FB_RESY);

    //
    // Set up display
    //
    uint32_t ui32Status = display_setup(FB_RESX, FB_RESY);
    if ( ui32Status != 0 )
    {
        am_util_stdio_printf("Failed to set up display!\n");
    }


    am_util_stdio_printf("Type,Texture_Location,FB_Location,Tex_Format,FB_Format,Src_Burst,Des_Burst,SamplingMode,Src_Width,Src_Hight,Des_Width,Des_Hight,Des_Angle,time(ms),speed_MP/s\n");

    //
    // run the test
    //
    for (i = 0; i < TEST_ITEM_TOTAL; i++)
    {
        psTestItem = &g_sTestItemList[i];

        //
        // Init framebuffer and layer
        //
        init_framebuffer_layer(psTestItem, &sLayer, &sFB);

        //
        // Set DC layer
        //
        nemadc_set_layer(0, &sLayer);

        //
        // rewind and bind the CL
        //
        nema_cl_bind(&sCL_clear);
        nema_cl_rewind(&sCL_clear);

        //
        // bind framebuffer as destination texture
        //
        nema_bind_dst_tex(sFB.bo.base_phys, FB_RESX, FB_RESY, sFB.format, sFB.stride);

        //
        // set clip
        //
        nema_set_clip(0, 0, FB_RESX, FB_RESY);

        //
        // clear the frambuffer
        //
        nema_clear(0);

        //
        // submit the CL
        //
        nema_cl_submit(&sCL_clear);

        //
        // wait for clean complete
        //
        nema_cl_wait(&sCL_clear);
        nema_cl_unbind();

        //
        // rewind and bind the CL
        //
        if ( (psTestItem->eTestType == TEST_VG_FONT) || (psTestItem->eTestType == TEST_VG) )
        {
            //
            // We should treat font as texture when test vector graphics font.
            //
            if ( psTestItem->eSrcType == ST_FONT )
            {
                am_util_stdio_printf("we should treat font as textrue when test vector graphics font.\n");
                return -1;
            }

            //
            // Bind CL
            //
            pCurrentCL = &sCL;
            nema_cl_bind_circular(pCurrentCL);
        }
        else
        {
            //
            // Bind CL
            //
            pCurrentCL = &sCL_NON_VG;
            nema_cl_bind(pCurrentCL);

        }

        //
        // Rewind CL
        //
        nema_cl_rewind(pCurrentCL);

        //
        // bind framebuffer as destination
        //
        nema_bind_dst_tex(sFB.bo.base_phys, FB_RESX, FB_RESY, sFB.format, sFB.stride);

        //
        // set clip
        //
        nema_set_clip(0, 0, FB_RESX, FB_RESY);

        nema_font_t *psFont = NULL;
        nema_vg_font_t *psVgFont = NULL;
        uintptr_t texture = (uintptr_t)NULL;

        //
        // bind source texture
        //
        switch (psTestItem->eSrcType)
        {
            case ST_TEXTURE:
                texture = load_texture(psTestItem);
                if (texture == (uintptr_t)NULL)
                {
                   am_util_stdio_printf("Can't find source texture!\n");
                   return -1;
                }
                else
                {
                    if ( psTestItem->ui8SamplingMode != NEMA_FILTER_PS && psTestItem->ui8SamplingMode != NEMA_FILTER_BL )
                    {
                        am_util_stdio_printf("invalid sampling mode!\n");
                        return -1;
                    }
                    nema_tex_mode_t mode = (psTestItem->enable_morton) ? (psTestItem->ui8SamplingMode | NEMA_TEX_MORTON_ORDER) : (psTestItem->ui8SamplingMode);

                    uint32_t width = psTestItem->ui32SrcWidth;
                    uint32_t hight = psTestItem->ui32SrcHight;
                    if (psTestItem->enable_morton)
                    {
                        width = ((width + 3 ) >> 2 ) << 2;
                    }
                    else if ( psTestItem->sTexFormat == NEMA_TSC6 )
                    {
                        width = ((width + 3 ) >> 2 ) << 2;
                        hight = ((hight + 3 ) >> 2 ) << 2;
                    }

                    nema_bind_src_tex(texture,
                                      width,
                                      hight,
                                      psTestItem->sTexFormat,
                                      -1,
                                      mode);

                    nema_enable_tiling(psTestItem->enable_tiling);
                }

                break;
            case ST_FONT:
                psFont = load_font(psTestItem);

                if (psFont == NULL)
                {
                   am_util_stdio_printf("Cann't find Font!\n");
                   return -1;
                }

                //
                // Bind font to be used for the context
                //
                nema_bind_font(psFont);
                break;
            case ST_VG_FONT:
                psVgFont = load_vg_font(psTestItem);
                if (psVgFont == NULL)
                {
                   am_util_stdio_printf("Cann't find Font!\n");
                   return -1;
                }

                //
                //Bind font
                //
                nema_vg_bind_font(&ume_ugo5_ttf);
                if ( psTestItem->ui32DesHight == 40 )
                {
                    nema_vg_set_font_size(16.f);
                }
                else if ( psTestItem->ui32DesHight == 50 )
                {
                    nema_vg_set_font_size(18.f);
                }
                else
                {
                    am_util_stdio_printf("please adjust vg font size.\n");
                    return -1;
                }
                break;

            case ST_VG_SVG:
                texture = load_vg_svg(psTestItem);
            default:
                break;
        }

        //
        // Set GPU read/write MSPI burst length
        //
        uint32_t burst_size_reg_fb  = 0x1UL << psTestItem->eDesBurstSize;
        uint32_t burst_size_reg_tex = 0x1UL << psTestItem->eSrcBurstSize;
        uint32_t burst_size_reg_val = 0x0UL | (psTestItem->eDesBurstSize << 4) | (psTestItem->eSrcBurstSize);
        nema_cl_add_cmd(NEMA_BURST_SIZE, burst_size_reg_val);

        //
        // start point
        //
        if ( FB_RESX < psTestItem->ui32DesWidth )
        {
            ui32StartX = 0;
        }
        else
        {
            ui32StartX = (FB_RESX-psTestItem->ui32DesWidth) / 2;
        }
        if ( FB_RESY < psTestItem->ui32DesWidth )
        {
            ui32StartY = 0;
        }
        else
        {
            ui32StartY = (FB_RESY-psTestItem->ui32DesHight) / 2;
        }

        //
        // start time record
        //
        time_start = nema_get_time();

        switch(psTestItem->eTestType)
        {
            case TEST_UNKNOWN:
            case TEST_FILL:
                nema_set_blend_fill(NEMA_BL_SRC);
                nema_fill_rect(ui32StartX,
                               ui32StartY,
                               psTestItem->ui32DesWidth,
                               psTestItem->ui32DesHight,
                               0xFF0A59F7);
                break;

            case TEST_COPY:
                nema_set_blend_blit(NEMA_BL_SRC);
                nema_blit(ui32StartX, ui32StartY);
                break;

            case TEST_BLEND:
                nema_set_blend_blit( NEMA_BL_SIMPLE);
                nema_blit(ui32StartX, ui32StartY);
                break;

            case TEST_BLEND_PART:
                nema_set_blend_blit( NEMA_BL_SIMPLE);
                nema_blit_subrect_fit(49, 49, 300, 300, 50, 50, 300, 300);
                break;

            case TEST_BLEND_GLB_A:
                nema_set_blend_blit(NEMA_BL_SIMPLE | NEMA_BLOP_MODULATE_A);
                nema_set_const_color(ui8Alpha << 24 | 0x00FFFFFF);
                nema_blit(ui32StartX, ui32StartY);
                break;

            case TEST_BLEND_FONT:
                ui32StartX = 100;
                nema_print(ui8Str,
                           ui32StartX,
                           ui32StartY,
                           psTestItem->ui32DesWidth * 5,
                           psTestItem->ui32DesHight,
                           0xff0a59f7U,
                           NEMA_ALIGNX_LEFT | NEMA_TEXT_WRAP | NEMA_ALIGNY_TOP);
                break;

            case TEST_SCALE:
                nema_set_blend_blit(NEMA_BL_SIMPLE);
                nema_blit_rect_fit(ui32StartX,
                                   ui32StartY,
                                   psTestItem->ui32DesWidth,
                                   psTestItem->ui32DesHight);
                break;

            case TEST_ROTATE:
                nema_set_blend_blit(NEMA_BL_SIMPLE);
                nema_blit_rotate_pivot( FB_RESX / 2,
                                        FB_RESY / 2,
                                        psTestItem->ui32SrcWidth / 2,
                                        psTestItem->ui32SrcHight / 2,
                                        psTestItem->ui32Angle);
                break;

            case TEST_SCALE_ROTATE:
                {
                    float x0, y0, x1, y1, x2, y2, x3, y3;

                    nema_set_blend_blit(NEMA_BL_SIMPLE);
                    x0 = -(psTestItem->ui32SrcWidth * 0.5f);
                    y0 = -(psTestItem->ui32SrcHight * 0.5f);
                    x1 = x0 + psTestItem->ui32SrcWidth;
                    y1 = y0;
                    x2 = x0 + psTestItem->ui32SrcWidth;
                    y2 = y0 + psTestItem->ui32SrcHight;
                    x3 = x0;
                    y3 = y0 + psTestItem->ui32SrcHight;

                    nema_matrix3x3_t m;
                    float scale_x = psTestItem->ui32DesWidth * 1.0f / psTestItem->ui32SrcWidth;
                    float scale_y = psTestItem->ui32DesHight * 1.0f / psTestItem->ui32SrcHight;
                    nema_mat3x3_load_identity(m);
                    nema_mat3x3_scale(m, scale_x, scale_y);
                    nema_mat3x3_rotate(m, psTestItem->ui32Angle);
                    nema_mat3x3_translate(m, FB_RESX / 2, FB_RESY / 2);
                    nema_mat3x3_mul_vec(m, &x0, &y0);
                    nema_mat3x3_mul_vec(m, &x1, &y1);
                    nema_mat3x3_mul_vec(m, &x2, &y2);
                    nema_mat3x3_mul_vec(m, &x3, &y3);
                    nema_blit_quad_fit(x0, y0,
                                       x1, y1,
                                       x2, y2,
                                       x3, y3);
                }
                break;

            case TEST_PERSPECTIVE:
                {
                    float x0, y0, x1, y1, x2, y2, x3, y3;

                    nema_set_blend_blit(NEMA_BL_SIMPLE);
                    x0 = -(psTestItem->ui32SrcWidth * 0.5f);
                    y0 = -(psTestItem->ui32SrcHight * 0.5f);
                    x1 = x0 + psTestItem->ui32SrcWidth;
                    y1 = y0;
                    x2 = x0 + psTestItem->ui32SrcWidth;
                    y2 = y0 + psTestItem->ui32SrcHight;
                    x3 = x0;
                    y3 = y0 + psTestItem->ui32SrcHight;

                    nema_matrix3x3_t m;
                    nema_mat3x3_load_identity(m);
                    nema_mat3x3_rotate(m, 180);
                    nema_mat3x3_translate(m, FB_RESX / 2, FB_RESY / 2);
                    m[2][0] = 1.0e-3;
                    m[2][1] = 2.0e-3;
                    m[2][2] = 1.0;
                    nema_mat3x3_mul_vec(m, &x0, &y0);
                    nema_mat3x3_mul_vec(m, &x1, &y1);
                    nema_mat3x3_mul_vec(m, &x2, &y2);
                    nema_mat3x3_mul_vec(m, &x3, &y3);
                    nema_blit_quad_fit(x0, y0,
                                       x1, y1,
                                       x2, y2,
                                       x3, y3);
                }
                break;
            case TEST_GAUSSIAN_BLUR:
                //Not supported!
                break;

            case TEST_VG_FONT:
                ui32StartX = 100;

                //blend mode
                nema_set_blend_fill(NEMA_BL_SRC);
                //Create and set paint
                paint =  nema_vg_paint_create();
                nema_vg_paint_set_type(paint, NEMA_VG_PAINT_COLOR);
                nema_vg_stroke_set_width(2.f);
                //Draw font
                nema_vg_paint_set_paint_color(paint, nema_rgba(0x0a, 0x59, 0xf7, 0xff ));
                nema_vg_print(paint,
                              ui8Str,
                              ui32StartX,
                              ui32StartY,
                              psTestItem->ui32DesWidth*5,
                              psTestItem->ui32DesHight,
                              NEMA_VG_ALIGNX_LEFT | NEMA_VG_TEXT_WRAP | NEMA_VG_ALIGNY_TOP,
                              NULL);
                break;
            case TEST_VG:
                {
                    float scalex, scaley;
                    uint32_t svg_width, svg_height;
                    float svg_w, svg_h;

                    nema_vg_get_tsvg_resolution((void *)texture, &svg_width, &svg_height);

                    if (svg_width != 0U && svg_height != 0U)
                    {
                        svg_w = (float)svg_width;
                        svg_h = (float)svg_height;
                    }
                    else
                    {
                        svg_w = (float)498; //(float)svg_width;
                        svg_h = (float)600; //(float)svg_height;
                    }

                    scalex = psTestItem->ui32DesWidth / svg_w;
                    scaley = psTestItem->ui32DesHight / svg_h;
                    nema_matrix3x3_t matrix;
                    nema_mat3x3_load_identity(matrix);
                    nema_mat3x3_translate(matrix, -svg_w * 0.5f, -svg_h * 0.5f);
                    nema_mat3x3_rotate(matrix, psTestItem->ui32Angle);
                    nema_mat3x3_scale(matrix, scalex, scaley);
                    nema_mat3x3_translate(matrix, FB_RESX / 2, FB_RESY / 2);

                    nema_vg_set_global_matrix(matrix);
                    //nema_vg_set_quality(NEMA_VG_QUALITY_BETTER);
                    nema_vg_set_quality(NEMA_VG_QUALITY_FASTER);
                    //nema_vg_set_quality(NEMA_VG_QUALITY_NON_AA);


                    nema_vg_draw_tsvg((void *)texture);
                }
                break;
            default:
                break;
        }
        //
        // submit the CL
        //
        nema_cl_submit(pCurrentCL);

#ifdef PIN_MEASURE
        am_hal_gpio_output_set(PIN_MEASURE);
#endif

        //
        // Record CL submit time
        //
        time_submit = nema_get_time();

        //
        // wait for GPU
        //
        nema_cl_wait(pCurrentCL);

        //
        //Record end time
        //
        time_end = nema_get_time();

#ifdef PIN_MEASURE
        am_hal_gpio_output_clear(PIN_MEASURE);
#endif

        nema_cl_unbind();

        //
        // Display refresh start
        //
        display_refresh_start();

        //
        // Wait refresh end
        //
        display_refresh_wait();

        //
        // calculate result and output
        //

        if ( (psTestItem->eTestType == TEST_VG) || (psTestItem->eTestType == TEST_VG_FONT) )
        {
            //For vector graphics, we should include both the CPU time and GPU time.
            time_used = (double)(time_end - time_start) * 1000.0;
        }
        else
        {
            //For rater graphics, we only need to include the GPU execution time.
            time_used = (double)(time_end - time_submit) * 1000.0;
        }

        if ( (psTestItem->eTestType == TEST_VG_FONT) || (psTestItem->eTestType == TEST_BLEND_FONT) )
        {
            //
            // Time should be divided by 5 because of draw 5 chinese character.
            //
            time_used /= 5;
        }
        speed_pps = (psTestItem->ui32DesWidth * psTestItem->ui32DesHight) / time_used / 1000.0;

        if (psTestItem->eTestType != TEST_UNKNOWN)
        {
          am_util_stdio_printf("%s,%s,%s,%s,%s,%d,%d,%s,%d,%d,%d,%d,%d,%.3f,%.3f\n",
                              g_pui8TestTypeStr[psTestItem->eTestType],
                              g_pui8LocationStr[psTestItem->eTEXLocation],
                              g_pui8LocationStr[psTestItem->eFBLocation],
                              format2string(psTestItem->sTexFormat),
                              format2string(psTestItem->sFBFormat),
                              burst_size_reg_tex,
                              burst_size_reg_fb,
                              (psTestItem->ui8SamplingMode == NEMA_FILTER_PS ? "Point" : "Bilinear"),
                              psTestItem->ui32SrcWidth,
                              psTestItem->ui32SrcHight,
                              psTestItem->ui32DesWidth,
                              psTestItem->ui32DesHight,
                              psTestItem->ui32Angle,
                              time_used,
                              speed_pps);
        }

        //
        // Deinitialization, free memory space
        //
        if ( psTestItem->eTestType == TEST_VG_FONT )
        {
            nema_vg_paint_destroy(paint);
        }
        else if ( psTestItem->eTestType == TEST_VG )
        {
            nema_vg_reset_global_matrix();
        }
    }
    return 0;
}
