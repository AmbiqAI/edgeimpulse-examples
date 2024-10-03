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

#ifndef TPI_ACPR_SWOSCALER_Pos
//
// In the CMSIS 5.6.0 version of core_cm4.h, the SWOSCALER field was no longer
// defined, while the PRESCALER field was left intact even though previous CMSIS
// versions PRESCALER as deprecated.  On the off chance that future versions
// make a correction and remove PRESCALER, define SWOSCALER here (per 5.3.0).
//
#define TPI_ACPR_SWOSCALER_Pos              0U                                         /*!< TPI ACPR: SWOSCALER Position */
#define TPI_ACPR_SWOSCALER_Msk             (0xFFFFUL /*<< TPI_ACPR_SWOSCALER_Pos*/)    /*!< TPI ACPR: SWOSCALER Mask */
#endif

#endif // AM_REG_CM4_H
