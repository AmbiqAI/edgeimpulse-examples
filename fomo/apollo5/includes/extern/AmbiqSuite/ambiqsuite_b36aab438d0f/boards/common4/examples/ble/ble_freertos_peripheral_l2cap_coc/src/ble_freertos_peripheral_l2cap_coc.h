//*****************************************************************************
//
//! @file freertos_peripheral_l2cap_coc.h
//!
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

#ifndef FREERTOS_PERIPHERAL_L2CAP_COC_H
#define FREERTOS_PERIPHERAL_L2CAP_COC_H

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
//#include "rtos.h"

//*****************************************************************************
//
// Task include files.
//
//*****************************************************************************
#include "radio_task.h"

#endif // FREERTOS_PERIPHERAL_L2CAP_COC_H
