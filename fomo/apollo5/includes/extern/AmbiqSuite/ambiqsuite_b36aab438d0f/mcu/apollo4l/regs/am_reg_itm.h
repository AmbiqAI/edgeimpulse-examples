//*****************************************************************************
//
//! @file am_reg_itm.h
//!
//! @brief Register macros for the ARM ITM module
//
//*****************************************************************************

//*****************************************************************************
//
// ${copyright}
//
// This is part of revision ${version} of the AmbiqSuite Development Package.
//
//*****************************************************************************
#ifndef AM_REG_ITM_H
#define AM_REG_ITM_H

//*****************************************************************************
//
// Instance finder. (1 instance(s) available)
//
//*****************************************************************************
#define AM_REG_ITM_NUM_MODULES                       1
#define AM_REG_ITMn(n) \
    (REG_ITM_BASEADDR + 0x00000000 * n)

#endif // AM_REG_ITM_H
