//*****************************************************************************
//
//! @file am_devices_ambt53_pwrdown.h
//!
//! @brief Functions and variables related to the power down task.
//!
//*****************************************************************************

//*****************************************************************************
//
// ${copyright}
//
// This is part of revision ${version} of the AmbiqSuite Development Package.
//
//*****************************************************************************

#ifndef POWER_DOWN_H
#define POWER_DOWN_H

#include "am_devices_ambt53.h"

//*****************************************************************************
//
// Force activities
//
//*****************************************************************************
typedef enum
{
    IPC_READ       = 0x0001,
    IPC_WRITE      = 0x0002,
} force_req;

//*****************************************************************************
//
// Task handle.
//
//*****************************************************************************
extern bool bIsPowerDown;
extern bool bIsIpcMsgBlocked;
extern bool bEnablePowerDownMode;
//*****************************************************************************
//
// External function definitions.
//
//*****************************************************************************
extern void am_devices_ambt53_pwrdown_setup(bool power_down_enable);
extern void am_devices_ambt53_wakeup_from_pwrdown(void);
extern void am_devices_force_ambt53_active_req(force_req ipc_event);
extern void am_devices_force_ambt53_active_release(force_req ipc_event);
extern void am_devices_ambt53_register_wakeup_cb(void (*pfn_callback)());
extern bool am_devices_check_ambt53_active_state(void);
#endif // POWER_DOWN_H
