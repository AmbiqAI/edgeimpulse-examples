//*****************************************************************************
//
//! @file cm_pro_grmn.h
//
//*****************************************************************************

//*****************************************************************************
//
// ${copyright}
//
// This is part of revision ${version} of the AmbiqSuite Development Package.
//
//*****************************************************************************

#ifndef CORE_MARK_PRO_H
#define CORE_MARK_PRO_H

//*****************************************************************************
//
// Required built-ins.
//
//*****************************************************************************
#include <stdint.h>

#if HAS_STDIO
#include <stdio.h>
#endif

#include <time.h>

//*****************************************************************************
//
// Standard AmbiqSuite includes.
//
//*****************************************************************************
#include "am_mcu_apollo.h"
#include "am_bsp.h"
#include "am_util.h"

#include "am_time.h"
#include "ambiq_core_config.h"

#include "mspi_device_init.h"

//*****************************************************************************
//
// Macro definitions
//
//*****************************************************************************
#ifdef ENABLE_PMU
  #define pmu_enable am_util_pmu_enable
  #define pmu_get_profiling( x, y ) am_util_pmu_get_profiling( x, y )
  #define pmu_disable am_util_pmu_disable
#else
  #define pmu_enable()
  #define pmu_get_profiling( x, y )
  #define pmu_disable()
#endif

//*****************************************************************************
//
// PSRAM size and address definition.
//
//*****************************************************************************

#if defined(BAREMETAL) && defined(SYSTEM_VIEW)
#error SYSTEM_VIEW cannot be used in baremetal!
#endif

#define MSPI_PSRAM_MODULE              0

#if (MSPI_PSRAM_MODULE == 0)
#define MSPI_XIP_BASE_ADDRESS       (MSPI0_APERTURE_START_ADDR)
#elif (MSPI_PSRAM_MODULE == 1)
#define MSPI_XIP_BASE_ADDRESS       (MSPI1_APERTURE_START_ADDR)
#elif (MSPI_PSRAM_MODULE == 2)
#define MSPI_XIP_BASE_ADDRESS       (MSPI2_APERTURE_START_ADDR)
#endif // #if (MSPI_PSRAM_MODULE == 0)

#define MSPI_PSRAM_SIZE (0x800000)

void al_start_time(void);
void al_stop_time(void);

//*****************************************************************************
//
// ISR priority definition.
//
//*****************************************************************************
#define PSRAM_ISR_PRIORITY              (4 + 2)

#endif // CORE_MARK_PRO_H
