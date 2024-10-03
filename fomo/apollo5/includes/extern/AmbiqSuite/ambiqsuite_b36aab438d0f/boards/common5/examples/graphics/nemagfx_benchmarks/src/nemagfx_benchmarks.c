//*****************************************************************************
//
//! @file nemagfx_benchmarks.c
//!
//! @brief NemaGFX example.
//!
//! this example demonstrate the Nema GPU and CPU performance use Nema GPU's
//! basic characteristics, we should care about the FPS after each individual
//! test.
//! It uses the stimer to get the accurate time past.
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

//*****************************************************************************
//
// This application has a large number of common include files. For
// convenience, we'll collect them all together in a single header and include
// that everywhere.
//
//*****************************************************************************
#include "nemagfx_benchmarks.h"
#include "bench.h"
#include "utils.h"

#ifdef GPU_PERFORMANCE_MEASURE
    #undef DEFAULT_EXEC_MODE
    #define DEFAULT_EXEC_MODE GPU_BOUND
#else
    #ifndef DEFAULT_EXEC_MODE
        #define DEFAULT_EXEC_MODE CPU_GPU
    #endif
#endif

TLS_VAR nema_cmdlist_t *g_psCLCur, g_sCL0, g_sCL1, g_sContextCL;
ExecutionMode_e eExecMode =         DEFAULT_EXEC_MODE;

typedef enum
{
    GPU_BURST_LENGTH_16 = 4,
    GPU_BURST_LENGTH_32 = 5,
    GPU_BURST_LENGTH_64 = 6,
    GPU_BURST_LENGTH_128 = 7,
} nemagfx_burst_length_t;
nemagfx_burst_length_t tex_burst_length = GPU_BURST_LENGTH_16;
nemagfx_burst_length_t fb_burst_length = GPU_BURST_LENGTH_16;

uint32_t nema_burst_reg_value = 0x00000000;

//*****************************************************************************
//
//! @brief define empty function to eliminate compiling warning
//!
//! @return Zero.
//
//*****************************************************************************
static int
i32RenderFrame(void)
{
  return 0;
}

//*****************************************************************************
//
//! @brief initialize FB,render different shapes patterns,calculate GPU performance.
//!
//! @param i32TestNo - Selects graphics shapes.
//!
//! This function initialize FB,GPU's various of operations,printing performance.
//!
//! @return None.
//
//*****************************************************************************
void
run_bench(int32_t i32TestNo)
{
    int32_t i32Result = 0;
    suite_init();

    switch (i32TestNo)
    {
        case 1:
            bench_start(i32TestNo);
            i32Result = bench_fill_tri(0);
            bench_stop(i32TestNo, i32Result);
            break;

        case 2:
            bench_start(i32TestNo);
            i32Result = bench_fill_tri(1);
            bench_stop(i32TestNo, i32Result);
            break;

        case 3:
            bench_start(i32TestNo);
            i32Result = bench_fill_rect(0);
            bench_stop(i32TestNo, i32Result);
            break;

        case 4:
            bench_start(i32TestNo);
            i32Result = bench_fill_rect(1);
            bench_stop(i32TestNo, i32Result);
            break;

        case 5:
            bench_start(i32TestNo);
            i32Result = bench_fill_quad(0);
            bench_stop(i32TestNo, i32Result);
            break;

        case 6:
            bench_start(i32TestNo);
            i32Result = bench_fill_quad(1);
            bench_stop(i32TestNo, i32Result);
            break;

        case 7:
            bench_start(i32TestNo);
            i32Result = bench_draw_string(NEMA_BL_SRC);
            bench_stop(i32TestNo, i32Result);
            break;

        case 8:
            bench_start(i32TestNo);
            i32Result = bench_draw_line(0);
            bench_stop(i32TestNo, i32Result);
            break;

        case 9:
            bench_start(i32TestNo);
            i32Result = bench_draw_line(1);
            bench_stop(i32TestNo, i32Result);
            break;

        case 10:
            bench_start(i32TestNo);
            i32Result = bench_draw_rect(0);
            bench_stop(i32TestNo, i32Result);
            break;

        case 11:
            bench_start(i32TestNo);
            i32Result = bench_draw_rect(1);
            bench_stop(i32TestNo, i32Result);
            break;

        case 12:
            bench_start(i32TestNo);
            i32Result = bench_blit(NEMA_BL_SRC, NEMA_ROT_000_CCW);
            bench_stop(i32TestNo, i32Result);
            break;

        case 13:
            bench_start(i32TestNo);
            i32Result = bench_blit(NEMA_BL_SRC | NEMA_BLOP_MODULATE_RGB, NEMA_ROT_000_CCW);
            bench_stop(i32TestNo, i32Result);
            break;

        case 14:
            bench_start(i32TestNo);
            i32Result = bench_blit(NEMA_BL_SIMPLE, NEMA_ROT_000_CCW);
            bench_stop(i32TestNo, i32Result);
            break;

        case 15:
            bench_start(i32TestNo);
            i32Result = bench_blit(NEMA_BL_SIMPLE | NEMA_BLOP_MODULATE_RGB, NEMA_ROT_000_CCW);
            bench_stop(i32TestNo, i32Result);
            break;

        case 16:
            bench_start(i32TestNo);
            i32Result = bench_blit(NEMA_BL_SRC, NEMA_ROT_090_CW);
            bench_stop(i32TestNo, i32Result);
            break;

        case 17:
            bench_start(i32TestNo);
            i32Result = bench_blit(NEMA_BL_SRC, NEMA_ROT_180_CW);
            bench_stop(i32TestNo, i32Result);
            break;

        case 18:
            bench_start(i32TestNo);
            i32Result = bench_blit(NEMA_BL_SRC, NEMA_ROT_270_CW);
            bench_stop(i32TestNo, i32Result);
            break;

        case 19:
            bench_start(i32TestNo);
            i32Result = bench_blit(NEMA_BL_SRC, NEMA_MIR_VERT);
            bench_stop(i32TestNo, i32Result);
            break;

        case 20:
            bench_start(i32TestNo);
            i32Result = bench_blit(NEMA_BL_SRC, NEMA_MIR_HOR);
            bench_stop(i32TestNo, i32Result);
            break;

        case 21:
            bench_start(i32TestNo);
            i32Result = bench_blit(NEMA_BL_SRC | NEMA_BLOP_SRC_CKEY, NEMA_ROT_000_CCW);
            bench_stop(i32TestNo, i32Result);
            break;

        case 22:
            bench_start(i32TestNo);
            i32Result = bench_blit(NEMA_BL_SRC | NEMA_BLOP_DST_CKEY, NEMA_ROT_000_CCW);
            bench_stop(i32TestNo, i32Result);
            break;

        case 23:
            bench_start(i32TestNo);
            i32Result = bench_stretch_blit(NEMA_BL_SRC, 1.5, NEMA_FILTER_PS);
            bench_stop(i32TestNo, i32Result);
            break;

        case 24:
            bench_start(i32TestNo);
            i32Result = bench_stretch_blit(NEMA_BL_SIMPLE, 1.5, NEMA_FILTER_PS);
            bench_stop(i32TestNo, i32Result);
            break;

        case 25:
            bench_start(i32TestNo);
            i32Result = bench_stretch_blit(NEMA_BL_SRC, 1.5, NEMA_FILTER_BL);
            bench_stop(i32TestNo, i32Result);
            break;

        case 26:
            bench_start(i32TestNo);
            i32Result = bench_stretch_blit(NEMA_BL_SIMPLE, 1.5, NEMA_FILTER_BL);
            bench_stop(i32TestNo, i32Result);
            break;

        case 27:
            bench_start(i32TestNo);
            i32Result = bench_stretch_blit_rotate(NEMA_BL_SRC, 0.75, NEMA_FILTER_PS);
            bench_stop(i32TestNo, i32Result);
            break;

        case 28:
            bench_start(i32TestNo);
            i32Result = bench_stretch_blit_rotate(NEMA_BL_SRC, 0.75, NEMA_FILTER_BL);
            bench_stop(i32TestNo, i32Result);
            break;

        case 29:
            bench_start(i32TestNo);
            i32Result = bench_textured_tri(NEMA_BL_SRC, NEMA_FILTER_PS);
            bench_stop(i32TestNo, i32Result);
            break;

        case 30:
            bench_start(i32TestNo);
            i32Result = bench_textured_tri(NEMA_BL_SIMPLE, NEMA_FILTER_PS);
            bench_stop(i32TestNo, i32Result);
            break;

        case 31:
            bench_start(i32TestNo);
            i32Result = bench_textured_tri(NEMA_BL_SRC, NEMA_FILTER_BL);
            bench_stop(i32TestNo, i32Result);
            break;

        case 32:
            bench_start(i32TestNo);
            i32Result = bench_textured_tri(NEMA_BL_SIMPLE, NEMA_FILTER_BL);
            bench_stop(i32TestNo, i32Result);
            break;

        case 33:
            bench_start(i32TestNo);
            i32Result = bench_textured_quad(NEMA_BL_SRC, NEMA_FILTER_PS);
            bench_stop(i32TestNo, i32Result);
            break;

        case 34:
            bench_start(i32TestNo);
            i32Result = bench_textured_quad(NEMA_BL_SIMPLE, NEMA_FILTER_PS);
            bench_stop(i32TestNo, i32Result);
            break;

        case 35:
            bench_start(i32TestNo);
            i32Result = bench_textured_quad(NEMA_BL_SRC, NEMA_FILTER_BL);
            bench_stop(i32TestNo, i32Result);
            break;

        case 36:
            bench_start(i32TestNo);
            i32Result = bench_textured_quad(NEMA_BL_SIMPLE, NEMA_FILTER_BL);
            bench_stop(i32TestNo, i32Result);
            break;
        default:
            return;
    }
    suite_terminate();
}

//*****************************************************************************
//
//! @brief loop call function run_bench()
//!
//! test run_bench() repeatedly.
//!
//! @return None.
//
//*****************************************************************************
void
benchmarks()
{
    uint32_t i32TestNo = 0;
    if (i32TestNo != 0)
    {
        run_bench(i32TestNo);
    }
    else
    {
        while(1)
        {
            //Set the seed of random number generator.
            srand(0xffffff00);

            //Get nema burst register value.
            nema_burst_reg_value = 0x0UL | (fb_burst_length << 4) | (tex_burst_length);
            am_util_stdio_printf("FB burst: %d \n", fb_burst_length);
            am_util_stdio_printf("TEX burst: %d \n", tex_burst_length);
            am_util_stdio_printf("Burst size register value: %08X \n", nema_burst_reg_value);

            //Run test bench.
            for (uint32_t i32Test = 1; i32Test <= TEST_MAX; ++i32Test)
            {
                run_bench(i32Test);
            }

            //Sweep different burst length.
            tex_burst_length++;
            if ( tex_burst_length > GPU_BURST_LENGTH_128 )
            {
                tex_burst_length = GPU_BURST_LENGTH_16;
                fb_burst_length ++;
                if ( fb_burst_length > GPU_BURST_LENGTH_128 )
                {
                    fb_burst_length = GPU_BURST_LENGTH_16;
                }
            }
        }
    }
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

#ifdef CPU_BURST_MODE
    //
    // Initialize for CPU High Performance Mode
    //
    if (am_hal_pwrctrl_mcu_mode_select(AM_HAL_PWRCTRL_MCU_MODE_HIGH_PERFORMANCE) == AM_HAL_STATUS_SUCCESS)
    {
        am_util_stdio_printf("\nCPU is operating in High Performance Mode\n");
    }
    else
    {
        am_util_stdio_printf("\nFailed to Initialize for CPU High Performance Mode operation\n");
    }
#else
    am_util_stdio_printf("\nCPU is operating in Normal Mode\n");
#endif

#ifdef AM_DEBUG_PRINTF
    am_bsp_debug_printf_enable();
#endif

#ifdef GPU_BURST_MODE
    //
    // Initialize for GPU High Performance Mode
    //
    if (am_hal_pwrctrl_gpu_mode_select(AM_HAL_PWRCTRL_GPU_MODE_HIGH_PERFORMANCE) == AM_HAL_STATUS_SUCCESS)
    {
        am_util_stdio_printf("\nGPU is operating in High Performance Mode\n");
    }
    else
    {
        am_util_stdio_printf("\nFailed to Initialize for GPU High Performance Mode operation\n");
    }
#else
    am_util_stdio_printf("\nGPU is operating in Normal Mode\n");
#endif
    //
    // Initialize display
    //
    am_devices_display_init(FB_RESX,
                            FB_RESY,
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
    // Run benchmark
    //
    benchmarks();

    //
    // We shouldn't ever get here.
    //
    while (1)
    {
    }

}

