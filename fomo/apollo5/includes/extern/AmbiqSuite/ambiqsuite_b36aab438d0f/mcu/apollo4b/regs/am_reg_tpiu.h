//*****************************************************************************
//
//! @file am_reg_tpiu.h
//!
//! @brief Register macros for the ARM TPIU module
//
//*****************************************************************************

//*****************************************************************************
//
// ${copyright}
//
// This is part of revision ${version} of the AmbiqSuite Development Package.
//
//*****************************************************************************
#ifndef AM_REG_TPIU_H
#define AM_REG_TPIU_H

//*****************************************************************************
//
// Instance finder. (1 instance(s) available)
//
//*****************************************************************************
#define AM_REG_TPIU_NUM_MODULES                      1
#define AM_REG_TPIUn(n) \
    (REG_TPIU_BASEADDR + 0x00000000 * n)

#endif // AM_REG_TPIU_H

