//*****************************************************************************
//
//! @file am_devices_dc_jdi_sharp.h
//!
//! @brief Generic sharp JDI(LS014B7DD01) driver.
//!
//! @addtogroup dc_jdi DC JDI Driver
//! @ingroup devices
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

#ifndef AM_DEVICES_DC_JDI_SHARP_H
#define AM_DEVICES_DC_JDI_SHARP_H

#include "stdint.h"
#include "nema_dc.h"
#include "nema_dc_hal.h"
#include "nema_dc_mipi.h"
#include "nema_dc_regs.h"
#include "nema_dc_intern.h"
#include "nema_sys_defs.h"
#include "am_devices_display_types.h"
#include "am_hal_timer.h"
#ifdef __cplusplus
extern "C"
{
#endif

#define SHARP_VA_VCOM_TIMER         7
#define SHARP_VA_VCOM_FREQUENCY     60

typedef struct
{
   uint32_t ui32TimerNum;
   uint32_t ui32Frequency;
   am_hal_timer_clock_e eTimerClock;
}am_devices_dc_jdi_timer_config_t;

//*****************************************************************************
//
//! External function definitions.
//
//*****************************************************************************

//*****************************************************************************
//
//! @brief Initialize sharp model type LS014B7DD01
//!
//! @param pTimerConfig     - jdi timer configuration structure.
//! @param pDCConfig        - jdi panel configuration structure.
//!
//! @note It should be called before NemaDC is initialized and configured.
//!
//! @return status.
//
//*****************************************************************************
extern uint32_t am_devices_dc_jdi_sharp_init(am_devices_dc_jdi_timer_config_t *pTimerConfig, nemadc_initial_config_t *pDCConfig);

//*****************************************************************************
//
//! @brief start generate PWM signal for JDI timer.
//!
//! This function generate two PWM signal one is inphase and another is opposite
//! phase to VCOM.
//!
//! @return None.
//
//*****************************************************************************
extern void am_devices_dc_jdi_sharp_timer_start(void);

//*****************************************************************************
//
//! @brief stop generate PWM signal for JDI timer.
//!
//! This function stop the two PWM signal.
//!
//! @return None.
//
//*****************************************************************************
extern void am_devices_dc_jdi_sharp_timer_stop(void);

#ifdef __cplusplus
}
#endif

#endif // AM_DEVICES_DC_JDI_SHARP_H

//*****************************************************************************
//
// End Doxygen group.
//! @}
//
//*****************************************************************************

