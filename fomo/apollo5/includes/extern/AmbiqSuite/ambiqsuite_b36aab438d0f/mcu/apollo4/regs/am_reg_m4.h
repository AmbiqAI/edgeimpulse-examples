//*****************************************************************************
//
//  am_reg_m4.h
//! @file
//!
//! @brief A collection of a few CMSIS-style macros that are not automatically
//!        generated in their respective core files.
//
//*****************************************************************************

//*****************************************************************************
//
// ${copyright}
//
// This is part of revision ${version} of the AmbiqSuite Development Package.
//
//*****************************************************************************

#ifndef AM_REG_CM4_H
#define AM_REG_CM4_H

//*****************************************************************************
//
// am_reg_itm.h
// CMSIS-style defines.
//
//*****************************************************************************
#define ITM_LAR_KEYVAL      0xC5ACCE55

//*****************************************************************************
//
// am_reg_sysctrl.h
// CMSIS-style defines.
//
//*****************************************************************************
#define SCB_CPACR_CP11_Pos                  22
#define SCB_CPACR_CP11_Msk                  0x00C00000
#define SCB_CPACR_CP10_Pos                  20
#define SCB_CPACR_CP10_Msk                  0x00300000

//*****************************************************************************
//
// am_reg_tpiu.h
// CMSIS-style defines.
//
//*****************************************************************************
#define TPI_CSPSR_CWIDTH_1BIT      1
#define TPI_SPPR_TXMODE_UART       2
#define TPI_ITCTRL_Mode_NORMAL     0

#endif // AM_REG_CM4_H
