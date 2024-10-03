//*****************************************************************************
//
//! @file mspi_dma_xip_parallel_stress_test_cases.h
//!
//! @brief Global includes for the mspi_dma_xip_parallel_stress_test_cases.
//
//*****************************************************************************

//*****************************************************************************
//
// ${copyright}
//
// This is part of revision ${version} of the AmbiqSuite Development Package.
//
//*****************************************************************************

#ifndef NEMAGFX_ENHANCED_STRESS_TEST_H
#define NEMAGFX_ENHANCED_STRESS_TEST_H

//*****************************************************************************
//
// Required built-ins.
//
//*****************************************************************************
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>

//*****************************************************************************
//
// Standard AmbiqSuite includes.
//
//*****************************************************************************
#include "am_mcu_apollo.h"
#include "am_bsp.h"
#include "am_util.h"
#include "mspi_dma_xip_parallel_stress_test_config.h"
#include "mspi_device_init.h"

//*****************************************************************************
//
// FreeRTOS include files.
//
//*****************************************************************************
#ifdef SYSTEM_VIEW
#include "SEGGER_SYSVIEW_FreeRTOS.h"
#endif
#include "FreeRTOS.h"
#include "task.h"
#include "portmacro.h"
#include "portable.h"
#include "semphr.h"
#include "event_groups.h"

#ifdef SYSTEM_VIEW
#include "SEGGER_SYSVIEW.h"
#endif

//*****************************************************************************
//
// Task include files.
//
//*****************************************************************************
#include "gui_task.h"
#include "render_task.h"
#include "display_task.h"
#include "xip_task.h"
#include "mspi_task.h"
#include "hp_lp_task.h"
#include "crypto_task.h"
#include "adc_task.h"
#include "iom_psram_task.h"
#include "emmc_task.h"
#include "uart_task.h"
#include "rtos.h"
#include "mspi_xipmm_task.h"
#include "mspi_scrambling_task.h"

#if defined(BAREMETAL) && defined(SYSTEM_VIEW)
#error SYSTEM_VIEW cannot be used in baremetal!
#endif

#define DEEPSLEEP_DEBUG
#define DEEPSLEEP_DEBUG_PIN      153
#define DCACHE_SIZE 64*1024

extern void vErrorHandler(void);
extern void *g_pPsramHandle;


#endif // NEMAGFX_ENHANCED_STRESS_TEST_H