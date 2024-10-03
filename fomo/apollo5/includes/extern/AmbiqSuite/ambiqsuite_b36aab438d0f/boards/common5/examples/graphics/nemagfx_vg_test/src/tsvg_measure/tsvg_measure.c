//*****************************************************************************
//
//! @file tsvg_benchmark.c
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
#include "nemagfx_buffer_customize.h"

#ifdef RUN_TSVG_MEASURE

#include "tiger.tsvg.h"


#define RESX 464
#define RESY 464

#define FRAME_BUFFER_FORMAT NEMA_RGB24
#define DC_FORMAT     NEMADC_RGB24


#define DEBUG_PIN (79)
#define DEBUG_PIN_GPU (80)

#define TIGER_X 400
#define TIGER_Y 400

#define ROTATE_ANGLE (0.f)

#define TEXTURE_IN_SSRAM
//#define  TEXTURE_IN_PSRAM


// Initializes NemaDC
static int init_dc_layer(nema_buffer_t *fb_bo, int img_w, int img_h)
{

    // Format        | Pixclock | RESX | FP | SYNC | BP | RESY | FP | SYNC | BP
    // 800x600, 60Hz | 40.000   | 800  | 40 | 128  | 88 | 600  | 1  | 4    | 23
    // nemadc_timing(800, 40, 128, 88, 600, 1, 4, 23);
    // -----------------------------------------------------

    nemadc_layer_t layer = {0};
    layer.format         = DC_FORMAT;
    layer.sizex = layer.resx = img_w;
    layer.sizey = layer.resy = img_h;
    layer.stride             = -1;
    layer.blendmode          = NEMADC_BL_SRC;
    layer.baseaddr_phys      = fb_bo->base_phys;
    layer.baseaddr_virt      = fb_bo->base_virt;
    nemadc_set_layer(0, &layer);

    return 0;
}

#ifdef DEBUG_PIN_GPU
void gpu_interrupt_cb(int commit_id)
{
    //NEMA_STATUS != 0 if GPU is busy.
    if(nema_reg_read(NEMA_STATUS) == 0)
    {
        am_hal_gpio_output_clear(DEBUG_PIN_GPU);
    }
}

uintptr_t svg_cl_start = 0;
uintptr_t svg_cl_end = 0;
uintptr_t clear_cl_start = 0;
uintptr_t clear_cl_end = 0;

void buffer_flush_cb(nema_buffer_t* buf)
{
    uintptr_t buf_addr = buf->base_phys;
    bool gpu_start = false;

    if(((buf_addr > svg_cl_start) && (buf_addr < svg_cl_end)) || \
     ((buf_addr > clear_cl_start) && (buf_addr < clear_cl_end)))
    {
        gpu_start = false;
    }
    else
    {
        gpu_start = true;
    }

    if(gpu_start)
    {
        am_hal_gpio_output_set(DEBUG_PIN_GPU);
    }

}
#endif

int tsvg_measure(void)
{
    // Initialize NemaGFX
    int ret = nema_init();
    if (ret)
    {
        return ret;
    }

    // Initialize Nema|dc
    // -----------------------------------------------------
    ret = display_setup(RESX, RESY);
    if (ret)
    {
        return ret;
    }

    nema_vg_init(RESX, RESY);

    unsigned char *tsvg_bin;
#ifdef  TEXTURE_IN_SSRAM
    //Create buffer for font in SSRAM and load them from MRAM to ssram
    nema_buffer_t tsvg_buffer = nema_buffer_create_pool(NEMA_MEM_POOL_ASSETS, tiger_tsvg_length);
    tsvg_bin = tsvg_buffer.base_virt;
#elif defined(TEXTURE_IN_PSRAM)
    tsvg_bin = (unsigned char *)MSPI_XIP_BASE_ADDRESS;
#else
    error "please specify the image data location!"
#endif

    nema_memcpy(tsvg_bin, tiger_tsvg, tiger_tsvg_length);

    uint32_t svg_width, svg_height;
    float svg_w, svg_h;

    nema_vg_get_tsvg_resolution(tsvg_bin, &svg_width, &svg_height);

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

    float scalex = TIGER_X / svg_w;
    float scaley = TIGER_Y / svg_h;

    am_util_stdio_printf("rasterizing svg %f x %f\n", svg_w, svg_h);
    am_util_stdio_printf("scalex: %f\nscaley: %f\n", scalex, scaley);

    // Allocate framebuffer
    nema_buffer_t fb_bo = nema_buffer_create_pool(NEMA_MEM_POOL_FB, RESX * RESY *  nema_format_size(FRAME_BUFFER_FORMAT));
    if (fb_bo.base_virt == NULL)
    {
        am_util_stdio_printf("Could not alloc image buffer.\n");
    }

    // Initialize NemaDC
    // -----------------------------------------------------
    ret = init_dc_layer(&fb_bo, RESX, RESY);
    if (ret != 0)
    {
        return ret;
    }

    // Create cl_clear Command List that will clear the
    // framebuffer on each frame
    nema_cmdlist_t cl_clear = nema_cl_create();

    nema_cl_bind(&cl_clear);
    nema_cl_rewind(&cl_clear);

    nema_set_clip(0, 0, RESX, RESY);
    // Bind Framebuffer
    nema_bind_dst_tex(fb_bo.base_phys, RESX, RESY, FRAME_BUFFER_FORMAT, -1);

    nema_clear(0xffffffff);

    nema_cl_submit(&cl_clear);
    nema_cl_wait(&cl_clear);

    nema_matrix3x3_t matrix;
    nema_mat3x3_load_identity(matrix);
    nema_mat3x3_translate(matrix, -svg_w * 0.5f, -svg_h * 0.5f);
    nema_mat3x3_rotate(matrix, ROTATE_ANGLE);
    nema_mat3x3_scale(matrix, scalex, scaley);
    nema_mat3x3_translate(matrix, RESX / 2, RESY / 2);

    nema_vg_set_global_matrix(matrix);

    // Draw the SVG
    // -----------------------------------------------------
    nema_cmdlist_t cl_svg = nema_cl_create_sized(16 * 1024);

#ifdef DEBUG_PIN
    am_hal_gpio_pinconfig(DEBUG_PIN, am_hal_gpio_pincfg_output);
#endif

#ifdef DEBUG_PIN_GPU
    am_hal_gpio_pinconfig(DEBUG_PIN_GPU, am_hal_gpio_pincfg_output);

    // Set up GPU complete call back
    nemagfx_set_interrupt_callback(gpu_interrupt_cb);

    svg_cl_start = cl_svg.bo.base_phys;
    svg_cl_end = cl_svg.bo.base_phys + cl_svg.bo.size;
    clear_cl_start = cl_clear.bo.base_phys;
    clear_cl_end = cl_clear.bo.base_phys + cl_clear.bo.size;

    graphic_heap_set_flush_cb(buffer_flush_cb);
#endif

    //nema_vg_set_quality(NEMA_VG_QUALITY_BETTER);
    //nema_vg_set_quality(NEMA_VG_QUALITY_FASTER);
    //nema_vg_set_quality(NEMA_VG_QUALITY_NON_AA);

#ifdef USE_PMU_PROFILING
    am_util_pmu_config_t pmu_config;
    am_util_pmu_profiling_t pmu_profiling;

    pmu_config.ui32Counters = VALID_PMU_COUNTERS;               // Enable all valid event counters

    pmu_config.ui32EventType[0] = ARM_PMU_CPU_CYCLES;
    pmu_config.ui32EventType[1] = ARM_PMU_CHAIN;                // Chain an odd-numbered counter with a preceding even-numbered counter to form a 32-bit counter.

    pmu_config.ui32EventType[2] = ARM_PMU_MEM_ACCESS;
    pmu_config.ui32EventType[3] = ARM_PMU_CHAIN;                // Chain an odd-numbered counter with a preceding even-numbered counter to form a 32-bit counter.

    pmu_config.ui32EventType[4] = ARM_PMU_L1D_CACHE_MISS_RD;
    pmu_config.ui32EventType[5] = ARM_PMU_CHAIN;                // Chain an odd-numbered counter with a preceding even-numbered counter to form a 32-bit counter.

    pmu_config.ui32EventType[6] = ARM_PMU_L1D_CACHE_RD;
    pmu_config.ui32EventType[7] = ARM_PMU_CHAIN;                // Chain an odd-numbered counter with a preceding even-numbered counter to form a 32-bit counter.

    //
    // By chaining counters in pairs,the counter range can be increased by halving the number of counters.
    //
    am_util_pmu_init(&pmu_config);
#endif

    uint32_t nema_error_code;

    while (1)
    {
        // Clear the framebuffer
        nema_cl_submit(&cl_clear);
        nema_cl_wait(&cl_clear);

#ifdef USE_PMU_PROFILING
        //
        // Enable performance monitoring unit
        //
        am_util_pmu_enable();
#endif

#ifdef DEBUG_PIN
        am_hal_gpio_output_set(DEBUG_PIN);
#endif

        // Draw the SVG
        nema_cl_bind_circular(&cl_svg);
        nema_cl_rewind(&cl_svg);

        nema_vg_draw_tsvg(tsvg_bin);

        nema_cl_submit(&cl_svg);
        nema_cl_wait(&cl_svg);

#ifdef DEBUG_PIN
        am_hal_gpio_output_clear(DEBUG_PIN);
#endif

        //
        // Get error code.
        //
        nema_error_code = nema_get_error();
        if ( nema_error_code != NEMA_ERR_NO_ERROR )
        {
            am_util_stdio_printf("nemagfx error: %8X\n", nema_error_code);
        }

#ifdef USE_PMU_PROFILING
        //
        // Stop Events Counters & Collect Profilings.
        //
        am_util_pmu_get_profiling(&pmu_config, &pmu_profiling);
        //
        // Disable performance monitoring unit
        //
        am_util_pmu_disable();
        am_util_pmu_deinit();
#endif

        // Start DC
        display_refresh_start();
        // Wait DC complete interrupt.
        nemadc_wait_vsync();
        // Do follow-up operations required by hardware.
        display_refresh_end();

    }

    // Destroy allocated assets
    // -----------------------------------------------------
    nema_cl_destroy(&cl_svg);
    nema_cl_destroy(&cl_clear);
    nema_vg_deinit();
    nema_buffer_destroy(&fb_bo);

    // -----------------------------------------------------

    return 0;
}

#endif
