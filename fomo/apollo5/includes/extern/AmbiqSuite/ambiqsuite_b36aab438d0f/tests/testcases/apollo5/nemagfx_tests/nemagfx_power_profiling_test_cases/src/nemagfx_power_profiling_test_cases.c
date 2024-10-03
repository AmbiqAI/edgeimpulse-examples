//*****************************************************************************
//
//! @file nemagfx_power_profiling_test_cases.c
//!
//! @brief Test to measure power used by the SoC over various NemaGFX and
//! NemaDC operations
//!
//! The test currently measures the difference in energy required for
//! GFX and DC operations when the framebuffer or textures are located
//! in on-chip (SSRAM) or off-chip (MSPI) memories. All of the cases avoid
//! computationally expensive operations like color format converson, fragment
//! shaders, compression/decompression, and blending.
//!
//! Usage:
//!     - Requires Apollo5 EB with a display peripheral card and a Joulescope
//!     - Attach the Joulescope between SYS_VDD and VDD_MCU
//!     - Attach the IN0 of the Joulescope to GP14 of the EB. Don't forget to
//!         run a ground between the EB and the Joulescope I/O header; the
//!         Joulescope I/O ground and the front panel binding posts are
//!         isolated from each other.
//!     - Make sure the EB is in IOX mode 1
//!     - Set up the Joulescope software to log V,I,P,and E at the highest
//!         sample rate possible
//!     - Enable SWO terminal or SWO Viewer
//!     - Load the firmware in the debugger but do not start it yet
//!     - Start the Joulescope then immediately after start the application
//!     - Stop the capture when the Unity result string appears
//!     - Use cursors to manually measure the E (Joules) during the high
//!         periods of IO0. IO0 goes high for each test case listed on
//!         the SWO output, so match your recorded results accordingly
//!     - This test sets MSPI0 to 96MHz DDR ("192MHz") hex mode. If you want
//!         to downgrade the MSPI to octal, uncomment
//!         "NEMAGFX_COMMON_FORCE_OCTAL_PSRAM" in config-template.ini. If
//!         you want to downgrade to 48MHz DDR ("96MHz") comment out
//!         "NEMAGFX_COMMON_USE_192M_PSRAM" in config-template.ini. You will
//!         need to first "make realclean" before "make" after making that
//!         change.
//!
//! Notes:
//!     - The first case, idle, is to set a quiescent energy baseline with the
//!         peripherals enabled and clocks set to their final (HP) rates but
//!         with no peripheral activity.
//!     - The GPU does not have an operation that performs a read without a
//!         corresponding write. Therefore, the BLIT cases measure the time
//!         to read from one memory and write to another. The FILL cases
//!         perform writes without reads and can be used to null out the
//!         write energy and latency from BLITs.
//!     - BLITs are purely memory copy from source to destination. They use
//!         source blend mode so that the destination is only written and no
//!         read-modify-write occurs as would happen with other blend modes
//!     - FILLs are purely memory writes of a constant color generated
//!         internally by the GPU. As with BLIT there is no read-modify-write
//!
//*****************************************************************************

//*****************************************************************************
//
// ${copyright}
//
// This is part of revision ${version} of the AmbiqSuite Development Package.
//
//*****************************************************************************
#include "nemagfx_test_common.h"
#include "nemagfx_power_profiling_test_cases.h"

typedef enum
{
    GPU_MSPI_BURST_SIZE_16 = 4,
    GPU_MSPI_BURST_SIZE_32 = 5,
    GPU_MSPI_BURST_SIZE_64 = 6,
    GPU_MSPI_BURST_SIZE_128 = 7,
} nemagfx_mspi_burst_size_t;

//*****************************************************************************
//
// Globals
//
//*****************************************************************************
bool g_bDispDone;
float g_fDispStopTime;

//*****************************************************************************
//
// Buffer location
//
//*****************************************************************************

// stride is row-centric
img_obj_t buffer_A_PSRAM = {{0}, TEX_RESX, TEX_RESY, TEX_RESX * 4, 0, NEMA_RGBA8888, 0};
img_obj_t buffer_B_SSRAM = {{0}, TEX_RESX, TEX_RESY, TEX_RESX * 4, 0, NEMA_RGBA8888, 0};
img_obj_t buffer_C_SSRAM = {{0}, TEX_RESX, TEX_RESY, TEX_RESX * 4, 0, NEMA_RGBA8888, 0};
img_obj_t buffer_D_PSRAM = {{0}, TEX_RESX, TEX_RESY, TEX_RESX * 4, 0, NEMA_RGBA8888, 0};
img_obj_t framebuffer_SSRAM = {{0}, FB_RESX, FB_RESY, FB_RESX * 4, 0, NEMA_RGBA8888, 0};
img_obj_t framebuffer_PSRAM = {{0}, FB_RESX, FB_RESY, FB_RESX * 4, 0, NEMA_RGBA8888, 0};


//*****************************************************************************
//
//  Detect display frame transfer completion
//
//*****************************************************************************
void disp_vsync_callback(void * pCallbackCtxt, uint32_t status)
{
    if(!g_bDispDone)
    {
        am_hal_gpio_state_write(GPIO_MEASURE, AM_HAL_GPIO_OUTPUT_CLEAR);
        g_fDispStopTime = nema_get_time();
        g_bDispDone = true;
    }
}
//*****************************************************************************
//
// Load objects.
//
//*****************************************************************************
void
load_objects(void)
{
    buffer_A_PSRAM.bo.base_phys = (uintptr_t)MSPI_XIP_BASE_ADDRESS;
    buffer_A_PSRAM.bo.base_virt = (void*)buffer_A_PSRAM.bo.base_phys;
    buffer_A_PSRAM.bo.size = buffer_A_PSRAM.w * buffer_A_PSRAM.h * 4;
    framebuffer_PSRAM.bo.base_phys = buffer_A_PSRAM.bo.base_phys + buffer_A_PSRAM.bo.size;
    framebuffer_PSRAM.bo.base_virt = (void*)framebuffer_PSRAM.bo.base_phys;
    framebuffer_PSRAM.bo.size = framebuffer_PSRAM.w * framebuffer_PSRAM.h * 4;
    buffer_D_PSRAM.bo.base_phys = framebuffer_PSRAM.bo.base_phys + framebuffer_PSRAM.bo.size;
    buffer_D_PSRAM.bo.base_virt = (void*)buffer_D_PSRAM.bo.base_phys;
    buffer_D_PSRAM.bo.size = buffer_D_PSRAM.w * buffer_D_PSRAM.h * 4;
    buffer_B_SSRAM.bo = nema_buffer_create_pool(NEMA_MEM_POOL_ASSETS, buffer_B_SSRAM.w * buffer_B_SSRAM.h * 4);
    buffer_C_SSRAM.bo = nema_buffer_create_pool(NEMA_MEM_POOL_ASSETS, buffer_C_SSRAM.w * buffer_C_SSRAM.h * 4);
    framebuffer_SSRAM.bo = nema_buffer_create_pool(NEMA_MEM_POOL_ASSETS, framebuffer_SSRAM.w * framebuffer_SSRAM.h * 4);
}

void
destroy_objects(void)
{
    nema_buffer_destroy(&buffer_B_SSRAM.bo);
    nema_buffer_destroy(&buffer_C_SSRAM.bo);
    nema_buffer_destroy(&framebuffer_SSRAM.bo);
}

//------------------------------------------------------------------------
//
//  Set the CPU and GPU performance mode
//
// mcu_mode: CPU performance mode. See am_hal_pwrctrl_mcu_mode_e for options
// gpu_mode: GPU performance mode. See am_hal_pwrctrl_gpu_mode_e for options
//
//------------------------------------------------------------------------
void
set_perf_mode(am_hal_pwrctrl_mcu_mode_e mcu_mode, am_hal_pwrctrl_gpu_mode_e gpu_mode)
{
    uint32_t ui32Status = am_hal_pwrctrl_mcu_mode_select(mcu_mode);
    if ( AM_HAL_STATUS_SUCCESS != ui32Status)
    {
        am_util_stdio_printf("CPU change power mode failed!\n");
        while(1){};
    }

    // disable GPU before power mode change
    am_hal_pwrctrl_periph_disable(AM_HAL_PWRCTRL_PERIPH_GFX);

    am_hal_pwrctrl_gpu_mode_e current_mode;
    am_hal_pwrctrl_gpu_mode_select(gpu_mode);
    am_hal_pwrctrl_gpu_mode_status(&current_mode);

    if(gpu_mode != current_mode)
    {
        am_util_stdio_printf("gpu switch power mode failed!\n");
        while(1){};
    }
    // reenable and reinit GPU after power mode change
    am_hal_pwrctrl_periph_enable(AM_HAL_PWRCTRL_PERIPH_GFX);
    nema_init();
}

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
    am_hal_cachectrl_icache_enable();
    am_hal_cachectrl_dcache_enable(true);
    set_perf_mode(AM_HAL_PWRCTRL_MCU_MODE_HIGH_PERFORMANCE, AM_HAL_PWRCTRL_GPU_MODE_HIGH_PERFORMANCE);
    dc_common_interface(FB_RESX, FB_RESY);

    am_hal_gpio_state_write(GPIO_MEASURE, AM_HAL_GPIO_OUTPUT_CLEAR);
    am_hal_gpio_pinconfig(GPIO_MEASURE, am_hal_gpio_pincfg_output);
    load_objects();
}

void
tearDown(void)
{
    destroy_objects();
}

//------------------------------------------------------------------------
//
//  Set GPU fb and texture burst sizes in the current command list
//
// fb_burst_size: one-hot framebuffer read burst size. See enums in nemagfx_mspi_burst_size_t
// tex_burst_size: one-hot texture read burst size. See enums in nemagfx_mspi_burst_size_t
//
//------------------------------------------------------------------------
void
set_nema_burst_size(nemagfx_mspi_burst_size_t fb_burst_size, nemagfx_mspi_burst_size_t tex_burst_size)
{
    uint32_t burst_size_reg_val = 0x0UL|((fb_burst_size)<<4)|(tex_burst_size);
    nema_cl_add_cmd(NEMA_BURST_SIZE, burst_size_reg_val);
}

//------------------------------------------------------------------------
//
//  Benchmark the performance of writes to a destination buffer
//
// buffer: destination buffer descriptor
// test_name: string describing the test case in the result output
//
//------------------------------------------------------------------------
void
clear_buffer_bench(img_obj_t* buffer, char* test_name)
{
    nema_cmdlist_t sCL = nema_cl_create();
    float start_time, total_time;

    nema_cl_bind(&sCL);
    //
    // GPU write solid color to PSRAM
    //
    nema_bind_dst_tex(buffer->bo.base_phys,
                        buffer->w,
                        buffer->h,
                        buffer->format,
                        buffer->stride);
    set_nema_burst_size(FB_BURST_SIZE, TEX_BURST_SIZE);
    nema_clear(0x55AA55AA); // arbitrary data
    start_time = nema_get_time();
    am_hal_gpio_state_write(GPIO_MEASURE, AM_HAL_GPIO_OUTPUT_SET);
    // measure start
    nema_cl_submit(&sCL);
    nema_cl_wait(&sCL);
    // measure end
    am_hal_gpio_state_write(GPIO_MEASURE, AM_HAL_GPIO_OUTPUT_CLEAR);
    total_time = nema_get_time() - start_time;

    am_util_stdio_printf("Fill %s time %f (s) %f MB/s\n", test_name, total_time, buffer->bo.size / total_time / 1048576);
    nema_cl_destroy(&sCL);
}

//------------------------------------------------------------------------
//
//  Benchmark the performance of transfers from one buffer to another
//
// buffer_src: source buffer descriptor
// buffer_dst: destination buffer descriptor
// test_name: string describing the test case in the result output
//
//------------------------------------------------------------------------
void
blit_buffers_bench(img_obj_t* buffer_src, img_obj_t* buffer_dst, char* test_name)
{
    nema_cmdlist_t sCL = nema_cl_create();
    float start_time, total_time;
    nema_cl_bind(&sCL);

    nema_buffer_flush(&(buffer_src->bo));
    nema_buffer_flush(&(buffer_dst->bo));
    nema_bind_dst_tex(buffer_dst->bo.base_phys,
                        buffer_dst->w,
                        buffer_dst->h,
                        buffer_dst->format,
                        buffer_dst->stride);
    nema_bind_src_tex(buffer_src->bo.base_phys,
                        buffer_src->w,
                        buffer_src->h,
                        buffer_src->format,
                        buffer_src->stride,
                        NEMA_FILTER_PS);
    set_nema_burst_size(FB_BURST_SIZE, TEX_BURST_SIZE);
    nema_set_clip(0, 0, TEX_RESX, TEX_RESY);
    nema_set_blend_blit(NEMA_BL_SRC);
    nema_blit(0, 0);

    start_time = nema_get_time();
    am_hal_gpio_state_write(GPIO_MEASURE, AM_HAL_GPIO_OUTPUT_SET);
    // measure start
    nema_cl_submit(&sCL);
    nema_cl_wait(&sCL);
    // measure end
    am_hal_gpio_state_write(GPIO_MEASURE, AM_HAL_GPIO_OUTPUT_CLEAR);
    total_time = nema_get_time() - start_time;

    am_util_stdio_printf("Blit %s time %f (s) %f MB/s\n", test_name, total_time, buffer_dst->bo.size / total_time / 1048576);

    nema_cl_destroy(&sCL);
}

//*****************************************************************************
//
//  Idle for 10ms to gather a power baseline
//
//*****************************************************************************
void
idle_bench()
{
    // Settling time
    am_util_delay_ms(10);
    am_hal_gpio_state_write(GPIO_MEASURE, AM_HAL_GPIO_OUTPUT_SET);
    // measure start
    am_util_delay_ms(10);
    // measure end
    am_hal_gpio_state_write(GPIO_MEASURE, AM_HAL_GPIO_OUTPUT_CLEAR);
    am_util_stdio_printf("Idle 10ms          time 0.010000 (s)\n");
}

//*****************************************************************************
//
//  Benchmark a display refresh from the source framebuffer
//
// framebuffer: source framebuffer descriptor to use for the display refresh
// test_name: string describing the test case in the result output
//
//*****************************************************************************
void
dc_bench(img_obj_t *framebuffer, char *test_name)
{
    float start_time, total_time;
    //
    // Assign a fixed value to display type.
    //


    nemadc_set_vsync_interrupt_callback(disp_vsync_callback, NULL);

    //
    // send layer 0 to display via NemaDC
    //
    nemadc_layer_t layer0 = {0};
    layer0.resx          = framebuffer->w;
    layer0.resy          = framebuffer->h;
    layer0.buscfg        = 0;
    layer0.format        = framebuffer->format;
    layer0.blendmode     = NEMADC_BL_SRC;
    layer0.stride        = framebuffer->stride;
    layer0.startx        = 0;
    layer0.starty        = 0;
    layer0.sizex         = layer0.resx;
    layer0.sizey         = layer0.resy;
    layer0.alpha         = 0xff;
    layer0.flipx_en      = 0;
    layer0.flipy_en      = 0;
    layer0.baseaddr_virt = framebuffer->bo.base_virt;
    layer0.baseaddr_phys = framebuffer->bo.base_phys;

    nemadc_set_layer(0, &layer0); // This function includes layer enable.
    nemadc_transfer_frame_prepare(false);

    g_bDispDone = false;
    start_time = nema_get_time();
    am_hal_gpio_state_write(GPIO_MEASURE, AM_HAL_GPIO_OUTPUT_SET);
    // measure start

    nemadc_transfer_frame_launch();
    while(!g_bDispDone)
    {

    }
    // measure end - disp_vsync_callback clears the measure GPIO and logs the stop time
    total_time = g_fDispStopTime - start_time;

    am_util_stdio_printf("Disp %s time %f (s) %f MB/s\n", test_name, total_time, framebuffer->bo.size / total_time / 1048576);
}

//*****************************************************************************
//
//! @brief GPU and DC read/write PSRAM vs SSRAM performance and power measurement
//!
//! This test reports measurements to SWO and not pass/fail.
//!
//! @return bTestPass (always true)
//
//*****************************************************************************
bool
nemagfx_power_profile_test(void)
{
    bool bTestPass = true;

    am_util_stdio_printf("\n nemagfx power profiling test:\n\n");

    set_perf_mode(AM_HAL_PWRCTRL_MCU_MODE_HIGH_PERFORMANCE, AM_HAL_PWRCTRL_GPU_MODE_HIGH_PERFORMANCE);

    am_util_stdio_printf("\nMSPI0 config: ");
    if(g_sMspiPsramConfig.eClockFreq == AM_HAL_MSPI_CLK_192MHZ)
    {
        am_util_stdio_printf("192MHz DDR ");
    }
    else if (g_sMspiPsramConfig.eClockFreq == AM_HAL_MSPI_CLK_96MHZ)
    {
        am_util_stdio_printf("96MHz DDR ");
    }
    else
    {
        am_util_stdio_printf("unrecognized MSPI speed! ");
    }

    if(g_sMspiPsramConfig.eDeviceConfig == AM_HAL_MSPI_FLASH_HEX_DDR_CE0)
    {
        am_util_stdio_printf("Hex\n\n");
    }
    else if(g_sMspiPsramConfig.eDeviceConfig == AM_HAL_MSPI_FLASH_OCTAL_DDR_CE0)
    {
        am_util_stdio_printf("Octal\n\n");
    }
    else
    {
        am_util_stdio_printf("unrecognized MSPI width!\n\n");
    }

    idle_bench();
    clear_buffer_bench(&buffer_A_PSRAM, "PS           ");
    clear_buffer_bench(&buffer_B_SSRAM, "SS_B         ");
    clear_buffer_bench(&buffer_C_SSRAM, "SS_C         ");
    blit_buffers_bench(&buffer_A_PSRAM, &buffer_B_SSRAM, "PS->SS_B     ");
    blit_buffers_bench(&buffer_A_PSRAM, &buffer_C_SSRAM, "PS->SS_C     ");
    blit_buffers_bench(&buffer_B_SSRAM, &buffer_A_PSRAM, "SS_B->PS     ");
    blit_buffers_bench(&buffer_B_SSRAM, &buffer_C_SSRAM, "SS_B->SS_C   ");
    blit_buffers_bench(&buffer_A_PSRAM, &buffer_D_PSRAM, "PS_A->PS_D   ");
    dc_bench(&framebuffer_SSRAM, "SSRAM        ");
    dc_bench(&framebuffer_PSRAM, "PSRAM        ");
    am_util_stdio_printf("\n\n");
    return bTestPass;
} // nemagfx_power_profiling_test
