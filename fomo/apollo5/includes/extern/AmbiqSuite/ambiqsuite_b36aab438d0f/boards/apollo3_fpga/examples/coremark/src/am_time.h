//*****************************************************************************
//
//! @file time.h
//!
//! @brief Systick support functions for coremark.
//
//*****************************************************************************

//*****************************************************************************
//
// ${copyright}
//
// This is part of revision ${version} of the AmbiqSuite Development Package.
//
//*****************************************************************************
#ifndef TIME_H
#define TIME_H

#ifdef __cplusplus
extern "C"
{
#endif
//*****************************************************************************
//
// External variable definitions
//
//*****************************************************************************
extern volatile uint32_t g_ui32SysTickWrappedTime;

//*****************************************************************************
//
// External function definitions
//
//*****************************************************************************
extern void am_systick_isr(void);

#ifdef __cplusplus
}
#endif

#endif // TIME_H

