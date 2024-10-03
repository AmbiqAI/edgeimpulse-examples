//*****************************************************************************
//
//! @file am_reg.h
//!
//! @brief Apollo510L register macros
//!
//! @addtogroup reg_macros_5 Apollo510L Register Macros
//! @ingroup apollo510L_hal
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
#ifndef AM_REG_H
#define AM_REG_H

//*****************************************************************************
//
//! @brief SYSPLL
//! Instance finder. (1 instance(s) available)
//
//*****************************************************************************
#define AM_REG_SYSPLL_NUM_MODULES                       1

//*****************************************************************************
//
//! @brief RTC
//! Instance finder. (1 instance(s) available)
//
//*****************************************************************************
#define AM_REG_RTC_NUM_MODULES                       1
#define AM_REG_RTCn(n) \
    (RTC_BASE + 0x00000000 * n)

//*****************************************************************************
//
//! @brief CM4WDT
//! Instance finder. (1 instance(s) available)
//
//*****************************************************************************
#define AM_REG_CM4WDT_NUM_MODULES                    1
#define AM_REG_CM4WDTn(n) \
    (CM4WDT_BASE + 0x00000000 * n)

//*****************************************************************************
//
//! @brief CM4IPC
//! Instance finder. (1 instance(s) available)
//
//*****************************************************************************
#define AM_REG_CM4IPC_NUM_MODULES                    1
#define AM_REG_CM4IPCn(n) \
    (CM4IPC_BASE + 0x00000000 * n)

//*****************************************************************************
//
//! @brief CM4MCUCTRL
//! Instance finder. (1 instance(s) available)
//
//*****************************************************************************
#define AM_REG_CM4MCUCTRL_NUM_MODULES                1
#define AM_REG_CM4MCUCTRLn(n) \
    (CM4MCUCTRL_BASE + 0x00000000 * n)

//*****************************************************************************
//
//! @brief CM4RADIO
//! Instance finder. (1 instance(s) available)
//
//*****************************************************************************
#define AM_REG_CM4RADIO_NUM_MODULES                  1
#define AM_REG_CM4RADIOn(n) \
    (CM4RADIO_BASE + 0x00000000 * n)

//*****************************************************************************
//
//! @brief CM4STIMER
//! Instance finder. (1 instance(s) available)
//
//*****************************************************************************
#define AM_REG_CM4STIMER_NUM_MODULES                 1
#define AM_REG_CM4STIMERn(n) \
    (CM4STIMER_BASE + 0x00000000 * n)

//*****************************************************************************
//
//! @brief CM4TIMER
//! Instance finder. (1 instance(s) available)
//
//*****************************************************************************
#define AM_REG_CM4TIMER_NUM_MODULES                  1
#define AM_REG_CM4TIMERn(n) \
    (CM4TIMER_BASE + 0x00000020 * n)

#endif // AM_REG_H

//*****************************************************************************
//
// End Doxygen group.
//! @}
//
//*****************************************************************************
