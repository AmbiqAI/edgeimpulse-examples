//*****************************************************************************
//
//  am_reg_sysctrl.h
//! @file
//!
//! @brief Register macros for the ARM SYSCTRL module
//
//*****************************************************************************

//*****************************************************************************
//
// ${copyright}
//
// This is part of revision ${version} of the AmbiqSuite Development Package.
//
//*****************************************************************************
#ifndef AM_REG_SYSCTRL_H
#define AM_REG_SYSCTRL_H

//*****************************************************************************
//
// Instance finder. (1 instance(s) available)
//
//*****************************************************************************
#define AM_REG_SYSCTRL_NUM_MODULES                   1
#define AM_REG_SYSCTRLn(n) \
    (REG_SYSCTRL_BASEADDR + 0x00000000 * n)

#endif // AM_REG_SYSCTRL_H
