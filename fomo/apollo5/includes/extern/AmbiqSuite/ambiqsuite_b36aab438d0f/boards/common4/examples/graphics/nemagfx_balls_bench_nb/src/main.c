//*****************************************************************************
//
//! @file main.c
//!
//! @brief NemaGFX example.

//
//*****************************************************************************

//*****************************************************************************
//
// ${copyright}
//
// This is part of revision ${version} of the AmbiqSuite Development Package.
//
//*****************************************************************************

#include "am_bsp.h"
#include "nema_core.h"
#include "nema_utils.h"
#include "am_hal_global.h"
#include "stdlib.h"
#include "am_util_stdio.h"
#include "pretzel_rgba.h"
#include "string.h"
#include "main.h"
#include "NotoSans_Regular20pt8b.h"

#ifndef DONT_USE_NEMADC
#include "nema_dc.h"
#include "nema_dc_mipi.h"
#endif

#include "am_devices_dsi_rm67162.h"
#include "am_devices_nemadc_rm67162.h"

#ifndef BAREMETAL
    #include "FreeRTOS.h"
    #include "task.h"
    #include "semphr.h"
#endif

#define RESX ((g_sDispCfg[g_eDispType].ui32PanelResX / 64) * 64)
#define RESY ((g_sDispCfg[g_eDispType].ui32PanelResY / 64) * 64)

#define FRAME_BUFFERS               2
#define PRINT_LOG_INTERVAL          360

#define COLOR_RED                   0xFF0000FF
#define COLOR_WHITE                 0xFFFFFFFF

typedef enum
{
    STATE_FIRST_ROUND,
    STATE_SECOND_ROUND,
    STATE_THIRD_ROUND,
    STATE_DURING_WAIT_TE,
    STATE_DURING_WAIT_TE_COMPLETE,
    STATE_DURING_WAIT_VSYNC,
    STATE_DURING_WAIT_VSYNC_COMPLETE,
    STATE_DURING_WAIT_CL,
    STATE_DURING_WAIT_CL_COMPLETE,
    STATE_COMPLETE
} am_app_state_machine;

static img_obj_t g_sFBs[FRAME_BUFFERS];
static nemadc_layer_t sLayer[FRAME_BUFFERS] = {0};

img_obj_t g_sPretzel_rgba = {{0}, 120, 120, -1, 0, NEMA_RGBA8888, 0};

float start_time;
float end_time;
uint32_t ui32PrintTimer = 0;

static int i32CurFB = 0;
static int i32LastFB = -1;

nema_cmdlist_t sCLCircles;

am_app_state_machine g_eState;

#ifndef BAREMETAL
TaskHandle_t g_sTaskRending;
TaskHandle_t g_sTaskFlush;

static SemaphoreHandle_t g_sSemRenderComplete = NULL;
static SemaphoreHandle_t g_sSemFlushComplete = NULL;
#endif

static void
fb_reload_rgb565_2(void)
{
    for (int i = 0; i < FRAME_BUFFERS; ++i)
    {
        g_sFBs[i].w = RESX;
        g_sFBs[i].h = RESY;
        g_sFBs[i].format = NEMA_RGB565;
        g_sFBs[i].stride = RESX * 2;
        g_sFBs[i].bo = nema_buffer_create(g_sFBs[i].stride * g_sFBs[i].h);
        (void)nema_buffer_map(&g_sFBs[i].bo);

        sLayer[i].startx = sLayer[i].starty = 0;
        sLayer[i].sizex = sLayer[i].resx = g_sFBs[i].w;
        sLayer[i].sizey = sLayer[i].resy = g_sFBs[i].h;
        sLayer[i].stride = g_sFBs[i].stride;
        sLayer[i].format = (uint32_t)(NEMADC_RGB565 | (uint32_t)NEMADC_MODULATE_A);

        sLayer[i].blendmode = NEMADC_BL_SIMPLE;
        sLayer[i].alpha     = 0x80;
        sLayer[i].flipx_en      = 0;
        sLayer[i].flipy_en      = 0;
        sLayer[i].baseaddr_phys = g_sFBs[i].bo.base_phys;
        sLayer[i].baseaddr_virt = g_sFBs[i].bo.base_virt;
    }
}

static void
swap_fb(void)
{
    i32LastFB = i32CurFB;
    i32CurFB = (i32CurFB + 1) % FRAME_BUFFERS;
}

#define MAX_CIRCLE_NUM                (15)             // 32 will overflow
#define MAX_PICTURE_NUM               (15)

typedef struct
{
    int i32LastX, i32LastY;
    int i32CenterX, i32CenterY;
    int i32Radius;
    int speed_x, speed_y;
    unsigned char red, green, blue, aplha;
    unsigned char isFilled;
    char direction;
}
Circle_t;

Circle_t g_sCircle[MAX_CIRCLE_NUM];
Circle_t g_sPicture[MAX_PICTURE_NUM];

void
update_circle(void)
{
    uint16_t i;
    int16_t i16CurrentY;
    int16_t i16CurrentX;

    for (i = 0; i < MAX_CIRCLE_NUM; i++)
    {
        i16CurrentY = g_sCircle[i].i32CenterY + g_sCircle[i].speed_y;
        if (i16CurrentY > RESY - g_sCircle[i].i32Radius || i16CurrentY < g_sCircle[i].i32Radius)
        {
            if (g_sCircle[i].speed_y > 0)
            {
                g_sCircle[i].i32CenterY = RESY - g_sCircle[i].i32Radius;
                g_sCircle[i].speed_y = -g_sCircle[i].speed_y;
            }
            else
            {
                g_sCircle[i].i32CenterY = g_sCircle[i].i32Radius;
                g_sCircle[i].speed_y = abs(g_sCircle[i].speed_y);
            }
        }
        else
        {
            g_sCircle[i].i32CenterY = i16CurrentY;
        }
    }

    for (i = 0; i < MAX_CIRCLE_NUM; i++)
    {
        i16CurrentX = g_sCircle[i].i32CenterX + g_sCircle[i].speed_x;
        if (i16CurrentX > RESX - g_sCircle[i].i32Radius || i16CurrentX < g_sCircle[i].i32Radius)
        {
            if (g_sCircle[i].speed_x > 0)
            {
                g_sCircle[i].i32CenterX = RESX - g_sCircle[i].i32Radius;
                g_sCircle[i].speed_x = -g_sCircle[i].speed_x;
            }
            else
            {
                g_sCircle[i].i32CenterX = g_sCircle[i].i32Radius;
                g_sCircle[i].speed_x = abs(g_sCircle[i].speed_x);
            }
        }
        else
        {
            g_sCircle[i].i32CenterX = i16CurrentX;
        }
    }

    for (i = 0; i < MAX_CIRCLE_NUM; i++)
    {
        nema_set_blend_fill(NEMA_BL_SIMPLE);
        //am_util_stdio_printf("update_circle nema_cl_almost_full %d s\n", nema_cl_almost_full(&sCLCircles));
        // vTaskDelay(100);
        nema_fill_circle(g_sCircle[i].i32CenterX, g_sCircle[i].i32CenterY, g_sCircle[i].i32Radius,
                         nema_rgba(g_sCircle[i].red, g_sCircle[i].green, g_sCircle[i].blue, g_sCircle[i].aplha));
    }
    //am_util_stdio_printf("******************************************************\n");
}

void
update_picture(void)
{
    uint16_t i;
    int16_t i16CurrentY;
    int16_t i16CurrentX;

    for (i = 0; i < MAX_PICTURE_NUM; i++)
    {
        i16CurrentY = g_sPicture[i].i32CenterY + g_sPicture[i].speed_y;
        if (i16CurrentY > RESY - g_sPicture[i].i32Radius || i16CurrentY < g_sPicture[i].i32Radius)
        {
            if (g_sPicture[i].speed_y > 0)
            {
                g_sPicture[i].i32CenterY = RESY - g_sPicture[i].i32Radius;
                g_sPicture[i].speed_y = -g_sPicture[i].speed_y;
            }
            else
            {
                g_sPicture[i].i32CenterY = g_sPicture[i].i32Radius;
                g_sPicture[i].speed_y = abs(g_sPicture[i].speed_y);
            }
        }
        else
        {
            g_sPicture[i].i32CenterY = i16CurrentY;
        }
    }

    for (i = 0; i < MAX_PICTURE_NUM; i++)
    {
        i16CurrentX = g_sPicture[i].i32CenterX + g_sPicture[i].speed_x;
        if (i16CurrentX > RESX - g_sPicture[i].i32Radius || i16CurrentX < g_sPicture[i].i32Radius)
        {
            if (g_sPicture[i].speed_x > 0)
            {
                g_sPicture[i].i32CenterX = RESX - g_sPicture[i].i32Radius;
                g_sPicture[i].speed_x = -g_sPicture[i].speed_x;
            }
            else
            {
                g_sPicture[i].i32CenterX = g_sPicture[i].i32Radius;
                g_sPicture[i].speed_x = abs(g_sPicture[i].speed_x);
            }
        }
        else
        {
            g_sPicture[i].i32CenterX = i16CurrentX;
        }
    }

    for (i = 0; i < MAX_PICTURE_NUM; i++)
    {
        nema_set_blend_blit(NEMA_BL_SIMPLE);
        //am_util_stdio_printf("update_picture nema_cl_almost_full %d s\n", nema_cl_almost_full(&sCLCircles));
        nema_blit_rect_fit(g_sPicture[i].i32CenterX - g_sPicture[i].i32Radius,
                           g_sPicture[i].i32CenterY - g_sPicture[i].i32Radius,
                           g_sPicture[i].i32Radius*2,
                           g_sPicture[i].i32Radius*2);
    }
    //am_util_stdio_printf("******************************************************\n");
}

void state_transformation(void)
{
    switch (g_eState)
    {
        case STATE_FIRST_ROUND:
            g_eState = STATE_SECOND_ROUND;
        break;

        case STATE_DURING_WAIT_TE:
            if (nemadc_get_te() == AM_HAL_STATUS_SUCCESS)
            {
                g_eState = STATE_DURING_WAIT_TE_COMPLETE;
            }
        break;

        case STATE_DURING_WAIT_TE_COMPLETE:
            g_eState = STATE_THIRD_ROUND;
        break;

        case STATE_DURING_WAIT_VSYNC:
            if (nemadc_get_vsync() == AM_HAL_STATUS_SUCCESS)
            {
                g_eState = STATE_DURING_WAIT_VSYNC_COMPLETE;
            }
        break;

        case STATE_DURING_WAIT_CL:
            if (nema_get_cl_status(sCLCircles.submission_id) == AM_HAL_STATUS_SUCCESS)
            {
                g_eState = STATE_DURING_WAIT_CL_COMPLETE;
            }
        break;

        default: break;
    }
}

void balls_parameter_init(void)
{
    uint32_t i;

    srand(1);

    g_sPretzel_rgba.bo = nema_buffer_create(sizeof(pretzel_rgba));
    memcpy(g_sPretzel_rgba.bo.base_virt, pretzel_rgba, sizeof(pretzel_rgba));

    for (i = 0; i < MAX_CIRCLE_NUM; i++)
    {
        g_sCircle[i].i32Radius = rand() % 100;
        g_sCircle[i].i32CenterX = rand() % (RESX - g_sCircle[i].i32Radius * 2) + g_sCircle[i].i32Radius;
        g_sCircle[i].i32CenterY = rand() % (RESY - g_sCircle[i].i32Radius * 2) + g_sCircle[i].i32Radius;

        g_sCircle[i].red = rand() % 256;
        g_sCircle[i].green = rand() % 256;
        g_sCircle[i].blue = rand() % 256;
        g_sCircle[i].aplha = rand() % 256;

        g_sCircle[i].speed_x = rand() % 5 + 1;
        g_sCircle[i].speed_y = rand() % 5 + 1;
    }

    for (i = 0; i < MAX_PICTURE_NUM; i++)
    {
        g_sPicture[i].i32Radius = rand() % 100;
        g_sPicture[i].i32CenterX = rand() % (RESX - g_sPicture[i].i32Radius * 2) + g_sPicture[i].i32Radius;
        g_sPicture[i].i32CenterY = rand() % (RESY - g_sPicture[i].i32Radius * 2) + g_sPicture[i].i32Radius;

        g_sPicture[i].speed_x = rand() % 5 + 1;
        g_sPicture[i].speed_y = rand() % 5 + 1;
    }
}

static float get_fps(void)
{
    static int   frame = 0;
    static float fps = 0;
    ++frame;

    if ( frame % 100 == 0 )
    {
        static float start_time = 0.f;
        static float stop_time  = 0.f;

        stop_time = nema_get_time();
        // am_util_stdio_printf("\nfps: %.02f\n", 100.f / (stop_time - start_time));
        fps = 100.f / (stop_time - start_time);
        start_time = stop_time;
    }

    return fps;
}

void num2char(char *str, double number, uint8_t g, uint8_t l)
{
    char table[] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9'};

    uint8_t i;
    int temp = (int)(number / 1);
    double t2 = 0.0;
    for (i = 1; i <= g; i++)
    {
        if (temp == 0)
        {
            str[g - i] = table[0];
        }
        else
        {
            str[g - i] = table[temp % 10];
        }
        temp = temp / 10;
    }
    *(str + g) = '.';
    temp = 0;
    t2 = number;
    for (i = 1; i <= l; i++ )
    {
        temp = (int)(t2 * 10);
        str[g + i] = table[temp % 10];
        t2 = t2 * 10;
    }
    *(str + g + l + 1) = '\0';
}

void update_fps_print(void)
{
    char buf[16] = {0};

    // am_util_stdio_sprintf(buf, "FPS: %3.2f", get_fps());
    // snprintf(buf, 16, "FPS: %3.2f", get_fps());
    memcpy(buf, "FPS: ", 5);
    num2char(&buf[5], get_fps(), 2, 2);
    nema_bind_font(&NotoSans_Regular20pt8b);

    nema_print(buf, RESX / 2 - 60, 50, 200, 40, COLOR_WHITE, NEMA_ALIGNX_LEFT);
}

void rendering(void)
{
    nema_cl_bind(&sCLCircles);
    nema_bind_dst_tex(g_sFBs[i32CurFB].bo.base_phys, g_sFBs[i32CurFB].w, g_sFBs[i32CurFB].h, g_sFBs[i32CurFB].format, g_sFBs[i32CurFB].stride);
    nema_bind_src_tex(g_sPretzel_rgba.bo.base_phys,
              g_sPretzel_rgba.w,
              g_sPretzel_rgba.h,
              g_sPretzel_rgba.format,
              g_sPretzel_rgba.stride,
              NEMA_FILTER_PS);
    nema_set_clip(0, 0, RESX, RESY);
    nema_clear(nema_rgba(0x00, 0x00, 0x00, 0xff));

    start_time = nema_get_time();
    update_picture();
    update_circle();
    update_fps_print();
    end_time = nema_get_time();
    if (ui32PrintTimer % PRINT_LOG_INTERVAL == 0)
    {
        am_util_stdio_printf("rendering used %f s\n", end_time - start_time);
    }
}

void
test_blit_balls(void)
{
    balls_parameter_init();

    sCLCircles = nema_cl_create();

    while (1)
    {
        ui32PrintTimer++;

        if (g_eState == STATE_FIRST_ROUND ||
            g_eState == STATE_SECOND_ROUND ||
            g_eState == STATE_THIRD_ROUND)
        {
            rendering();
        }

        if ( g_eState == STATE_THIRD_ROUND )
        {
            //nemadc_wait_vsync();
            if (nemadc_get_vsync() == AM_HAL_STATUS_IN_USE)
            {
                // am_util_stdio_printf("DC in use\n");
                g_eState = STATE_DURING_WAIT_VSYNC;
            }
            else
            {
                g_eState = STATE_DURING_WAIT_VSYNC_COMPLETE;
            }
        }

        if (g_eState == STATE_DURING_WAIT_VSYNC_COMPLETE)
        {
            if (g_sDispCfg[g_eDispType].eInterface == DISP_IF_DSI)
            {
                end_time = nema_get_time();
                if (ui32PrintTimer % PRINT_LOG_INTERVAL == 0)
                {
                    am_util_stdio_printf("nema_wait_vsync used %f s\n", end_time - start_time);
                }
                dsi_send_frame_single_end();
            }
            else
            {
                nemadc_send_frame_single_end();
            }
            g_eState = STATE_THIRD_ROUND;
        }

        if (g_eState == STATE_FIRST_ROUND ||
            g_eState == STATE_SECOND_ROUND ||
            g_eState == STATE_THIRD_ROUND)
        {
            start_time = nema_get_time();
            nema_cl_submit(&sCLCircles);
            end_time = nema_get_time();
            if (ui32PrintTimer % PRINT_LOG_INTERVAL == 0)
            {
                am_util_stdio_printf("nema_cl_submit used %f s\n", end_time - start_time);
            }

            if (g_eState == STATE_FIRST_ROUND)
            {
                g_eState = STATE_DURING_WAIT_CL;
            }
        }

        if ( g_eState == STATE_SECOND_ROUND || g_eState == STATE_THIRD_ROUND)
        {
            nemadc_set_layer(0, &sLayer[i32LastFB]);

            if (g_sDispCfg[g_eDispType].eInterface == DISP_IF_DSI)
            {
                dsi_send_frame_single_start_none_block_te();
                start_time = nema_get_time();
            }
            else
            {
                nemadc_send_frame_single();
            }
#ifdef ENABLE_TE
            if (nemadc_get_te() == AM_HAL_STATUS_IN_USE)
            {
                // am_util_stdio_printf("waiting for TE\n");
                g_eState = STATE_DURING_WAIT_TE;
                start_time = nema_get_time();
            }
            else
            {
                g_eState = STATE_DURING_WAIT_TE_COMPLETE;
            }
#else
            g_eState = STATE_DURING_WAIT_TE_COMPLETE;
#endif

            nema_calculate_fps();
        }

        if (g_eState == STATE_DURING_WAIT_TE_COMPLETE)
        {
            end_time = nema_get_time();
            if (ui32PrintTimer % PRINT_LOG_INTERVAL == 0)
            {
                am_util_stdio_printf("nema_wait_te used %f s\n", end_time - start_time);
            }
            // incase direct go into STATE_DURING_WAIT_TE_COMPLETE
            dsi_send_frame_single_start_none_block_vsync(NEMADC_OUTP_OFF);

            g_eState = STATE_DURING_WAIT_CL;

            start_time = nema_get_time();
        }

        if (g_eState == STATE_DURING_WAIT_CL_COMPLETE)
        {
            end_time = nema_get_time();
            if (ui32PrintTimer % PRINT_LOG_INTERVAL == 0)
            {
                am_util_stdio_printf("nema_cl_wait used %f s\n", end_time - start_time);
            }

            nema_cl_rewind(&sCLCircles);

            swap_fb();

            g_eState = STATE_THIRD_ROUND;
        }

        state_transformation();

#ifndef BAREMETAL
        taskYIELD();
 #endif
    }
}

#ifndef BAREMETAL
void
task_rending(void *pvParameters)
{
    bool run_second_round = false;

    while(1)
    {
        if (run_second_round == true)
        {
            xSemaphoreTake(g_sSemFlushComplete, portMAX_DELAY);
        }
        rendering();
        run_second_round = true;
        xSemaphoreGive(g_sSemRenderComplete);
    }
}

void
task_flush(void *pvParameters)
{
    bool run_second_round = false;
    bool run_third_round = false;

    while(1)
    {
        xSemaphoreTake(g_sSemRenderComplete, portMAX_DELAY);

        ui32PrintTimer++;

        if (run_third_round == true)
        {
            // nemadc_wait_vsync();
            while (nemadc_get_vsync() == AM_HAL_STATUS_IN_USE)
            {
                taskYIELD();
            }
            dsi_send_frame_single_end();
        }

        start_time = nema_get_time();
        nema_cl_submit(&sCLCircles);
        end_time = nema_get_time();

        if (ui32PrintTimer % PRINT_LOG_INTERVAL == 0)
        {
            am_util_stdio_printf("nema_cl_submit used %f s\n", end_time - start_time);
        }

        if (run_second_round == true)
        {
            start_time = nema_get_time();
            nemadc_set_layer(0, &sLayer[i32LastFB]);
            dsi_send_frame_single_start_none_block_te();
#ifdef ENABLE_TE
            // nemadc_wait_te();

            while (nemadc_get_te() == AM_HAL_STATUS_IN_USE)
            {
                taskYIELD();
            }
#endif
            dsi_send_frame_single_start_none_block_vsync(NEMADC_OUTP_OFF);

            end_time = nema_get_time();

            if (ui32PrintTimer % PRINT_LOG_INTERVAL == 0)
            {
                am_util_stdio_printf("send frame used %f s\n", end_time - start_time);
            }

            nema_calculate_fps();

            run_third_round = true;
        }


        start_time = nema_get_time();
        nema_cl_wait(&sCLCircles);
        end_time = nema_get_time();

        if (ui32PrintTimer % PRINT_LOG_INTERVAL == 0)
        {
            am_util_stdio_printf("nema_cl_wait used %f s\n", end_time - start_time);
        }

        //while (nema_reg_read(0x0fcU) != 0)
        //{
            //nema_cl_wait(&sCLCircles);
        //}

        nema_cl_rewind(&sCLCircles);
        swap_fb();
        run_second_round = true;
        xSemaphoreGive(g_sSemFlushComplete);
    }
}
#endif

int32_t
am_balls_bench(void)
{
    int i32Ret;
    uint32_t ui32MipiCfg = MIPICFG_8RGB888_OPT0; //!< default config
    //
    // Initialize NemaGFX
    //
    i32Ret = nema_init();
    if (i32Ret != 0)
    {
        return i32Ret;
    }
    //
    // Initialize Nema|dc
    //
    i32Ret = nemadc_init();
    if (i32Ret != 0)
    {
        return i32Ret;
    }

    if ((g_sDispCfg[g_eDispType].eInterface == DISP_IF_DSI) || (g_sDispCfg[g_eDispType].bUseDPHYPLL == true))
    {
        uint8_t ui8LanesNum = g_sDsiCfg.ui8NumLanes;
        uint8_t ui8DbiWidth = g_sDsiCfg.eDbiWidth;
        uint32_t ui32FreqTrim = g_sDsiCfg.eDsiFreq;
        pixel_format_t eFormat = FMT_RGB888;
        if (am_hal_dsi_para_config(ui8LanesNum, ui8DbiWidth, ui32FreqTrim, false) != 0)
        {
            return -3;
        }
        switch (eFormat)
        {
            case FMT_RGB888:
                if (ui8DbiWidth == 16)
                {
                    ui32MipiCfg = MIPICFG_16RGB888_OPT0;
                }
                if (ui8DbiWidth == 8)
                {
                    ui32MipiCfg = MIPICFG_8RGB888_OPT0;
                }
                break;

            case FMT_RGB565:
                if (ui8DbiWidth == 16)
                {
                    ui32MipiCfg = MIPICFG_16RGB565_OPT0;
                }
                if (ui8DbiWidth == 8)
                {
                    ui32MipiCfg = MIPICFG_8RGB565_OPT0;
                }
                break;

            default:
                //
                // invalid color component index
                //
                return -3;
        }
    }

    uint16_t ui16PanelResX = g_sDispCfg[g_eDispType].ui32PanelResX; //!< panel's max resolution
    uint16_t ui16PanelResY = g_sDispCfg[g_eDispType].ui32PanelResY; //!< panel's max resolution

    uint16_t ui16MinX, ui16MinY;
    //
    //Set the display region to center
    //
    if (RESX > ui16PanelResX)
    {
        ui16MinX = 0;   //!< set the minimum value to 0
    }
    else
    {
        ui16MinX = (ui16PanelResX - RESX) >> 1;
        ui16MinX = (ui16MinX >> 1) << 1;
    }

    if (RESY > ui16PanelResY)
    {
        ui16MinY = 0;   //!< set the minimum value to 0
    }
    else
    {
        ui16MinY = (ui16PanelResY - RESY) >> 1;
        ui16MinY = (ui16MinY >> 1) << 1;
    }
    //
    // Initialize the display
    //
    switch (g_sDispCfg[g_eDispType].eInterface)
    {
        case DISP_IF_SPI4:
            am_devices_nemadc_rm67162_init(MIPICFG_SPI4, MIPICFG_1RGB565_OPT0, RESX, RESY, ui16MinX, ui16MinY);
            break;
        case DISP_IF_DSPI:
            am_devices_nemadc_rm67162_init(MIPICFG_DSPI | MIPICFG_SPI4, MIPICFG_2RGB565_OPT0, RESX, RESY, ui16MinX, ui16MinY);
            break;
        case DISP_IF_QSPI:
            am_devices_nemadc_rm67162_init(MIPICFG_QSPI | MIPICFG_SPI4, MIPICFG_4RGB565_OPT0, RESX, RESY, ui16MinX, ui16MinY);
            break;
        case DISP_IF_DSI:
            am_devices_dsi_rm67162_init(ui32MipiCfg, RESX, RESY, ui16MinX, ui16MinY);
            break;
        default:
            ; //NOP
    }

//    nema_event_init(1, 0, 0, RESX, RESY);

    fb_reload_rgb565_2();

#ifdef BAREMETAL
    test_blit_balls();

    return 0;
#else

    g_sSemRenderComplete = xSemaphoreCreateBinary();
    configASSERT(g_sSemRenderComplete);

    g_sSemFlushComplete = xSemaphoreCreateBinary();
    configASSERT(g_sSemFlushComplete);

    balls_parameter_init();

    sCLCircles = nema_cl_create();

    xTaskCreate(task_rending, "task_rending", 512, 0, 3, &g_sTaskRending);
    xTaskCreate(task_flush, "task_flush", 512, 0, 3, &g_sTaskFlush);

    while (1)
    {
        vTaskDelay(1000);
    }
#endif

}

