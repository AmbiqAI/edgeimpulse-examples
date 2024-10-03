//*****************************************************************************
//
//! @file am_util.h
//!
//! @brief Top Include for all of the utilities
//!
//! This file provides all the includes necessary to use the utilities.
//!
//
//! @defgroup utils utils
//! @{
//
//*****************************************************************************

//*****************************************************************************
//
// ${copyright}
//
// This is part of revision ${version} of the AmbiqSuite Development Package.
//
//*****************************************************************************
#ifndef AM_UTIL_H
#define AM_UTIL_H

//*****************************************************************************
//
// C99
//
//*****************************************************************************
#include <stdint.h>
#include <stdbool.h>

//*****************************************************************************
//
// Utilities
//
//*****************************************************************************
#include "am_util_debug.h"
#include "am_util_delay.h"
#include "am_util_id.h"
#include "am_util_stdio.h"
#include "am_util_string.h"
#include "am_util_time.h"
// #### INTERNAL BEGIN ####
#include "am_util_test.h"
#if defined(AM_PART_APOLLO) || defined(AM_PART_APOLLO2)
#include "am_util_aes.h"
#include "am_util_cmdline.h"
#include "am_util_draw_2d1bpp.h"
#include "am_util_fonts.h"
#include "am_util_math.h"
#include "am_util_plot.h"
#include "am_util_ring_buffer.h"
#include "am_util_stopwatch.h"
#include "am_util_stxetx.h"
#include "am_util_tap_detect.h"
#endif
#if defined(AM_PART_APOLLO4_API) || defined(AM_PART_APOLLO5_API)
#include "am_util_crypto.h"
#endif
#if defined(AM_PART_APOLLO3_API) || defined(AM_PART_APOLLO4_API) || defined(AM_PART_APOLLO5_API)
#include "am_util_swd.h"
#endif // AM_PART_APOLLO3_API || AM_PART_APOLLO4_API

// #### INTERNAL END ####

#if defined(AM_PART_APOLLO3_API)
#include "am_util_ble.h"
#include "am_util_regdump.h"
#elif defined(AM_PART_APOLLO4_API)
#include "am_util_ble_cooper.h"
#endif

#if defined(AM_PART_APOLLO5_API)
#include "am_util_pmu.h"
#endif

#endif  // AM_UTIL_H

//*****************************************************************************
//
// End Doxygen group.
//! @}
//
//*****************************************************************************

