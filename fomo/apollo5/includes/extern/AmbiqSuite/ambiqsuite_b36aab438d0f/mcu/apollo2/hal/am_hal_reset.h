//*****************************************************************************
//
//  am_hal_reset.h
//! @file
//!
//! @brief Hardware abstraction layer for the Reset Generator module.
//!
//! @addtogroup wdt2 Watchdog Timer (RSTGEN)
//! @ingroup apollo2hal
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
#ifndef AM_HAL_RSTGEN_H
#define AM_HAL_RSTGEN_H

//*****************************************************************************
//
//! @name Reset Generator Configuration
//! @brief These macros may be used to set the reset generator's configuration.
//! @{
//
//*****************************************************************************
#define AM_HAL_RESET_CFG_WDT_RESET_ENABLE       (AM_REG_RSTGEN_CFG_WDREN(1))
// Brown out high (2.1v) reset enable.
#define AM_HAL_RESET_CFG_BOD_HIGH_RESET_ENABLE  (AM_REG_RSTGEN_CFG_BODHREN(1))
//! @}

//*****************************************************************************
//
//! @name Reset Generator Status Bit Masks
//! @brief These macros may be used to determine which type(s) of resets have
//!        been seen.
//! @{
//
//*****************************************************************************
// Reset was initiated by a Watchdog Timer Reset.
#define AM_HAL_RESET_STAT_WDT (AM_REG_RSTGEN_STAT_WDRSTAT_M)

// Reset was a initiated by Debugger Reset.
#define AM_HAL_RESET_STAT_DEBUG (AM_REG_RSTGEN_STAT_DBGRSTAT_M)

// Reset was a initiated by Software POI Reset.
#define AM_HAL_RESET_STAT_POI (AM_REG_RSTGEN_STAT_POIRSTAT_M)

// Reset was a initiated by Software POR or AIRCR Reset.
#define AM_HAL_RESET_STAT_SOFTWARE (AM_REG_RSTGEN_STAT_SWRSTAT_M)

// Reset was initiated by a Brown-Out Reset.
#define AM_HAL_RESET_STAT_BOD (AM_REG_RSTGEN_STAT_BORSTAT_M)

// Reset was initiated by a Power Cycle
#define AM_HAL_RESET_STAT_POWER_CYCLE (AM_REG_RSTGEN_STAT_PORSTAT_M)

// Reset was initiated by an External Reset.
#define AM_HAL_RESET_STAT_EXTERNAL (AM_REG_RSTGEN_STAT_EXRSTAT_M)
//! @}

#ifdef __cplusplus
extern "C"
{
#endif
//*****************************************************************************
//
// External function definitions
//
//*****************************************************************************
extern void am_hal_reset_init(uint32_t ui32Config);
extern void am_hal_reset_por(void);
extern void am_hal_reset_poi(void);
extern uint32_t am_hal_reset_status_get(void);
extern void am_hal_reset_status_clear(void);

#ifdef __cplusplus
}
#endif

#endif // AM_HAL_RSTGEN_H

//*****************************************************************************
//
// End Doxygen group.
//! @}
//
//*****************************************************************************
