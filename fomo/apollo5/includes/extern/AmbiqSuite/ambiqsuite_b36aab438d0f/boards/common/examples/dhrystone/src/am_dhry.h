//*****************************************************************************
//
//! @file am_dhry.h
//!
//! @brief Support functions for dhrystone.
//
//*****************************************************************************

//*****************************************************************************
//
// ${copyright}
//
// This is part of revision ${version} of the AmbiqSuite Development Package.
//
//*****************************************************************************
#ifndef AM_DHRY_H
#define AM_DHRY_H

#ifdef __cplusplus
extern "C"
{
#endif

//*****************************************************************************
//
// Includes
//
//*****************************************************************************
#include "am_mcu_apollo.h"
#include "am_util.h"
#include "am_dhry.h"
#include <stdarg.h>

//*****************************************************************************
//
// Configuration
//
//*****************************************************************************
#define AM_PRINT_RESULTS    1


//*****************************************************************************
//
// Macros
//
//*****************************************************************************
#define CLOCK   1
#define DRUNS   500000

#define REG
#define FILE    int
#define stdout  0

//
// Redirect some function naming
//
#define strcpy                  am_util_string_strcpy
#define strcmp                  am_util_string_strcmp
//#define fprintf                 am_util_stdio_fprintf
#define fprintf                 am_fprintf
#define clock                   am_clock

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

//
// am_dhry.c
//
extern void am_systick_isr(void);
//extern int  am_util_stdio_fprintf(FILE* fp, const char *pcFmt, ...);
//extern int  am_sprintf(const char *pcFmt, ...);
extern int am_fprintf(FILE* fp, const char *pcFmt,...);
extern uint32_t am_clock(void);
void am_dhrystone_init(void);
void am_dhrystone_fini(void);

#ifdef __cplusplus
}
#endif

#endif // AM_DHRY_H



