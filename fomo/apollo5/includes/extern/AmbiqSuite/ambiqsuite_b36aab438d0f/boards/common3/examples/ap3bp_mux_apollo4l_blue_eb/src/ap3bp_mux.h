//*****************************************************************************
//
//! @file am_hal_rtc.h
//!
//! @brief Functions for interfacing and accessing the Real-Time Clock (RTC).
//!
//! @addtogroup rtc3 RTC - Real-Time Clock
//! @ingroup apollo3_hal
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
#ifndef AP3BP_MUX_H
#define AP3BP_MUX_H

#ifdef __cplusplus
extern "C"
{
#endif

#define MUX_VERSION                     "MUX_AP4L_BLUE_V 0.3"

typedef enum
{
    MUX_MODE_DIP_SWITCH,
    MUX_MODE_IOS_LINK ,
    MUX_MODE_MAX,
} mux_mode_e;

typedef struct
{
    mux_mode_e eCurrentMode;
    uint32_t Value[MUX_MODE_MAX];
} mux_info_t;


extern uint32_t mux_value_get(void);

extern void mux_mode_set(mux_mode_e eMode );
extern mux_mode_e mux_mode_get(void);
extern uint32_t mux_mode_value_get(mux_mode_e eMode);
extern void mux_mode_value_set(mux_mode_e eMode, uint32_t value);

#ifdef __cplusplus
}
#endif

#endif // AP3BP_MUX_H
