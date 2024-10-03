//*****************************************************************************
//
//! @file ap3bp_mux.c
//!
//! @brief
//!
//! Purpose: .
//!
//! Printing takes place over the ITM at 1M Baud.
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

#include "am_mcu_apollo.h"
#include "am_bsp.h"
#include "am_util.h"
#include "ap3bp_mux.h"

mux_info_t g_mux_info;

void mux_mode_set(mux_mode_e eMode )
{
    g_mux_info.eCurrentMode = eMode;
}

uint32_t mux_value_get(void)
{
  return g_mux_info.Value[g_mux_info.eCurrentMode] ;
}

mux_mode_e mux_mode_get(void)
{
  return g_mux_info.eCurrentMode;
}

uint32_t mux_mode_value_get(mux_mode_e eMode)
{
    return g_mux_info.Value[eMode];
}

void mux_mode_value_set(mux_mode_e eMode, uint32_t value)
{
    g_mux_info.Value[eMode] = value;
}


