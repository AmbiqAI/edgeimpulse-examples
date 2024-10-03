//*****************************************************************************
//
//! @file nemagfx_benchmarks.c
//!
//! @brief NemaGFX Benchmarks Example.
//!
//! @addtogroup graphics_examples Graphics Examples
//!
//! @defgroup nemagfx_benchmarks NemaGFX Benchmarks Example
//! @ingroup graphics_examples
//! @{
//!
//! Purpose: This example demonstrate the Nema GPU and CPU performance use Nema GPU's
//! basic characteristics, we should care about the FPS after each individual
//! test.
//! need a timer to get the accurate time past.
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

#ifndef DEFAULT_EXEC_MODE
#define DEFAULT_EXEC_MODE CPU_GPU
#endif
#ifdef AM_PART_APOLLO4B
//
// AXI Scratch buffer
// Need to allocate 20 Words even though we only need 16, to ensure we have 16 Byte alignment
//
AM_SHARED_RW uint32_t               axiScratchBuf[20];
#endif
TLS_VAR AM_SHARED_RW nema_cmdlist_t *g_psCLCur, g_sCL0, g_sCL1, g_sContextCL;
ExecutionMode_e eExecMode =         DEFAULT_EXEC_MODE;

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
// #### INTERNAL BEGIN ####
void
vddc_vddf_trim(void)
{
    am_util_delay_ms(3000);
    //!< !!!!!!!!! // AI Key
    //!< !!!!!!!!! // AI Key
    //!< !!!!!!!!! // AI Key
    //!< !!!!!!!!! // AI Key
    //!< mcuctrlpriv.ldoreg1.coreldoactivetrim 0x40040080 bit9~0
    uint32_t ui32Val0 = AM_REGVAL(0x40020080);
    AM_REGVAL(0x40020080) = ui32Val0 + 22; //!< 24; //  please increase trim value with a proper value, to increase VDDC to 750mV.
    am_util_delay_ms(100);
    //!< mcuctrlpriv.ldoreg2.memldoactivetrim 0x40040088 bit5~0
    ui32Val0 = AM_REGVAL(0x40020088);
    AM_REGVAL(0x40020088) = ui32Val0 + 26; //!< 28; // please increase trim value with a proper value, to increase VDDF to 950mV.
    am_util_delay_ms(100);
}
// #### INTERNAL END ####

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

    AM_REGVAL(0x40090090) = 0x00000002;

    switch (i32TestNo)
    {
        case 0:
            bench_start(i32TestNo);
            i32Result = bench_fill_tri(0);
            bench_stop(i32TestNo, i32Result);
            break;

        case 1:
            bench_start(i32TestNo);
            i32Result = bench_fill_tri(1);
            bench_stop(i32TestNo, i32Result);
            break;

        case 2:
            bench_start(i32TestNo);
            i32Result = bench_fill_rect(0);
            bench_stop(i32TestNo, i32Result);
            break;

        case 3:
            bench_start(i32TestNo);
            i32Result = bench_fill_rect(1);
            bench_stop(i32TestNo, i32Result);
            break;

        case 4:
            bench_start(i32TestNo);
            i32Result = bench_fill_quad(0);
            bench_stop(i32TestNo, i32Result);
            break;

        case 5:
            bench_start(i32TestNo);
            i32Result = bench_fill_quad(1);
            bench_stop(i32TestNo, i32Result);
            break;

        case 6:
            bench_start(i32TestNo);
            i32Result = bench_draw_stroked_arc_aa(NEMA_BL_SIMPLE);
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
    while (1)
    {
        for (int32_t i32Test = 0; i32Test <= TEST_MAX; ++i32Test)
        {
            run_bench(i32Test);
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
    // Set up scratch AXI buf (needs 64B - aligned to 16 Bytes)
    //
#ifdef AM_PART_APOLLO4B
    am_hal_daxi_control(AM_HAL_DAXI_CONTROL_AXIMEM, (uint8_t *)((uint32_t)(axiScratchBuf + 3) & ~0xF));
#endif

    //
    // External power on
    //
    am_bsp_external_pwr_on();
    am_util_delay_ms(100);
    am_bsp_low_power_init();
// #### INTERNAL BEGIN ####
    //
    // !!!! if DSI or MRAM textures are used, elevate VDDF !!!
    //
    //vddc_vddf_trim();
// #### INTERNAL END ####

    //
    // Initialize the printf interface for ITM/SWO output.
    //
    am_bsp_itm_printf_enable();
    //
    // Clear the terminal and print the banner.
    //
    am_util_stdio_terminal_clear();

#ifdef BURST_MODE
    //
    // Initialize for High Performance Mode
    //
    if (am_hal_pwrctrl_mcu_mode_select(AM_HAL_PWRCTRL_MCU_MODE_HIGH_PERFORMANCE) == AM_HAL_STATUS_SUCCESS)
    {
        am_util_stdio_printf("\nOperating in High Performance Mode\n");
    }
    else
    {
        am_util_stdio_printf("\nFailed to Initialize for High Performance Mode operation\n");
    }
#else
    am_util_stdio_printf("\nOperating in Normal Mode\n");
#endif

#ifdef AM_DEBUG_PRINTF
    am_bsp_debug_printf_enable();
#endif

    //
    // Initialize display
    //
    am_devices_display_init(FB_RESX,
                            FB_RESY,
                            COLOR_FORMAT_RGB888,
                            false);

    am_hal_pwrctrl_periph_enable(AM_HAL_PWRCTRL_PERIPH_GFX);
    //
    // Global interrupt enable
    //
    am_hal_interrupt_master_enable();

    //
    // Initialize NemaGFX
    //
    nema_init();

    am_hal_timer_config_t sTimerConfig;
    uint32_t ui32Status;
    ui32Status = am_hal_timer_default_config_set(&sTimerConfig);
    if (AM_HAL_STATUS_SUCCESS != ui32Status)
    {
        am_util_stdio_printf("Failed to initialize a timer configuration structure with default values!\n");
    }
    sTimerConfig.eInputClock = AM_HAL_TIMER_CLOCK_HFRC_DIV16;
    sTimerConfig.eFunction = AM_HAL_TIMER_FN_UPCOUNT;
    ui32Status = am_hal_timer_config(0, &sTimerConfig);
    if (AM_HAL_STATUS_SUCCESS != ui32Status)
    {
        am_util_stdio_printf("Failed to configure a timer!\n");
    }
    ui32Status = am_hal_timer_start(0);
    if (AM_HAL_STATUS_SUCCESS != ui32Status)
    {
        am_util_stdio_printf("Failed to start a timer!\n");
    }

    benchmarks();
    //
    // We shouldn't ever get here.
    //
    while (1)
    {
    }
}

//*****************************************************************************
//
// End Doxygen group.
//! @}
//
//*****************************************************************************

