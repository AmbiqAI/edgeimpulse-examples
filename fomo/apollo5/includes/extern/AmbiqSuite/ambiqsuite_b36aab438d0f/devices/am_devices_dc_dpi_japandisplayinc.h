//*****************************************************************************
//
//! @file am_devices_dc_dpi_japandisplayinc.h
//!
//! @brief Generic Japan Display Inc RGB/DPI interface(LPM027M700A) driver.
//!
//! @addtogroup dc_dpi DC DPI Driver
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

#ifndef AM_DEVICES_DC_DPI_JANPANDISPLAYINC_H
#define AM_DEVICES_DC_DPI_JANPANDISPLAYINC_H

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

#define JANPAN_DISPLAY_INC_COMIN_TIMER             7
#define JANPAN_DISPLAY_INC_COMIN_FREQUENCY         1

typedef struct
{
   uint32_t ui32TimerNum;
   uint32_t ui32Frequency;
   am_hal_timer_clock_e eTimerClock;
}am_devices_dc_dpi_timer_config_t;

//*****************************************************************************
//
//! External function definitions.
//
//*****************************************************************************

//*****************************************************************************
//
//! @brief Initialize the Japan Display Inc. model LPM027M700A panel.
//!
//! @param pTimerConfig     - JDI timer configuration structure.
//! @param pDCConfig        - JDI panel configuration structure.
//!
//! @note It should be called before NemaDC is initialized and configured.
//!
//! @return status.
//
//*****************************************************************************
extern uint32_t am_devices_dc_dpi_japandisplayinc_init(am_devices_dc_dpi_timer_config_t *pTimerConfig, nemadc_initial_config_t *pDCConfig);

//*****************************************************************************
//
//! @brief start generate PWM signal.
//!
//!
//! @return None.
//
//*****************************************************************************
extern void am_devices_dc_dpi_japandisplayinc_timer_start(void);

//*****************************************************************************
//
//! @brief stop generate PWM signal.
//!
//! @return None.
//
//*****************************************************************************
extern void am_devices_dc_dpi_japandisplayinc_timer_stop(void);

#ifdef __cplusplus
}
#endif

#endif // AM_DEVICES_DC_DPI_JANPANDISPLAYINC_H

//*****************************************************************************
//
// End Doxygen group.
//! @}
//
//*****************************************************************************

