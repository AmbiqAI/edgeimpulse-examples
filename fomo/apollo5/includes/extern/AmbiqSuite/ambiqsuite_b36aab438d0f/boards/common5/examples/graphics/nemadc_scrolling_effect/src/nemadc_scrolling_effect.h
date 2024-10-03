//*****************************************************************************
//
//! @file nemadc_scrolling_effect.h
//!
//! @brief The nemadc_scrolling_effect includes headers for the example.
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

#ifndef NEMADC_SCROLLING_EFFECT_H
#define NEMADC_SCROLLING_EFFECT_H

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

#include "nema_hal.h"
#include "nema_dc.h"
#include "am_util_delay.h"
#include "am_util_stdio.h"
#include "tsi_malloc.h"
#include "nema_utils.h"

#include "am_devices_dc_xspi_raydium.h"
#include "am_devices_dc_dsi_raydium.h"

#ifndef BAREMETAL
//*****************************************************************************
//
// FreeRTOS include files.
//
//*****************************************************************************
#include "FreeRTOS.h"
#include "task.h"
#include "portmacro.h"
#include "portable.h"
#include "semphr.h"
#include "event_groups.h"

//*****************************************************************************
//
// Task include files.
//
//*****************************************************************************
#include "gui_task.h"
#endif // BAREMETAL

#endif // NEMADC_SCROLLING_EFFECT_H
