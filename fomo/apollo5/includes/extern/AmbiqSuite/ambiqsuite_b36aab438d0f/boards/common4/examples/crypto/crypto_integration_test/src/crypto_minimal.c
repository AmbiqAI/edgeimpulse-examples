//*****************************************************************************
//
//! @file hello_world.c
//!
//! @brief A simple "Hello World" example.
//!
//! This example prints a banner message over SWO at 1M baud.
//! To see the output of this program, use J-Link SWO Viewer (or
//! similar viewer appl) and configure SWOClock for 1000.
//! The example sleeps after it is done printing.
//
//*****************************************************************************

//*****************************************************************************
//
// ${copyright}
//
// This is part of revision ${version} of the AmbiqSuite Development Package.
//
//*****************************************************************************

#include "am_mcu_apollo.h"
#include "am_bsp.h"
#include "am_util.h"

/* cerberus pal */
#include "cc_pal_types.h"
#include "cc_pal_types_plat.h"
#include "cc_pal_mem.h"
#include "cc_pal_perf.h"
#include "cc_regs.h"
#include "cc_otp_defs.h"
#include "cc_lib.h"
#include "cc_rnd_common.h"
#include "mbedtls_cc_mng.h"

/* tests pal and hal */
#include "board_configs.h"
#include "test_pal_mem.h"
#include "test_pal_mem_s.h"
#include "test_pal_thread.h"
#include "test_pal_log.h"
#include "test_proj_otp.h"

/* mbedtls */
#include "mbedtls/memory_buffer_alloc.h"
#include "mbedtls/entropy.h"
#include "mbedtls/ctr_drbg.h"

/* internal files */
#include "run_integration_test.h"
#include "run_integration_test_api.h"
#include "run_integration_helper.h"
#include "run_integration_pal_log.h"
#include "run_integration_pal_otp.h"
#include "run_integration_pal_reg.h"
#include "run_integration_profiler.h"
#include "run_integration_flash.h"
#include "run_integration_otp.h"

//*****************************************************************************
//
// Main function
//
//*****************************************************************************
int
main(void)
{
    am_bsp_debug_printf_enable();
    am_util_stdio_terminal_clear();
    am_util_stdio_printf("Crypto integration test.\n");

    am_bsp_low_power_init();

    //
    // Set the default cache configuration
    //
#if ENABLE_CACHE
    am_hal_cachectrl_config(&am_hal_cachectrl_defaults);
    am_hal_cachectrl_enable();
#endif

    //
    // Enable the timer interrupt in the NVIC.
    //
    am_hal_interrupt_master_enable();

    runIt_all();

    while (1);
}
