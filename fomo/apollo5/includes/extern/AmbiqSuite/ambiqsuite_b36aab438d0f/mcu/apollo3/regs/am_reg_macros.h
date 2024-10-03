//*****************************************************************************
//
//  am_reg_macros.h
//! @file
//!
//! @brief Helper macros for using hardware registers.
//
//*****************************************************************************

//*****************************************************************************
//
// ${copyright}
//
// This is part of revision ${version} of the AmbiqSuite Development Package.
//
//*****************************************************************************

#ifndef AM_REG_MACROS_H
#define AM_REG_MACROS_H

#ifdef __cplusplus
extern "C"
{
#endif

//*****************************************************************************
//
// For direct 32-bit access to a register or memory location, use AM_REGVAL:
//      AM_REGVAL(0x1234567) |= 0xDEADBEEF;
//
//*****************************************************************************
#define AM_REGVAL(x)               (*((volatile uint32_t *)(x)))
#define AM_REGVAL_FLOAT(x)         (*((volatile float *)(x)))

//*****************************************************************************
//
// AM_REGADDR()
// One thing CMSIS does not do well natively is to provide for static register
// address computation. The address-of operator (e.g. &periph->reg) is helpful,
// but does run into problems, such as when attempting to cast the resulting
// pointer to a uint32_t.  The standard C macro, offsetof() can help.
//
// Use AM_REGADDR() for single-module peripherals.
// Use AM_REGADDRn() for multi-module peripherals (e.g. IOM, UART).
//
//*****************************************************************************
#define AM_REGADDR(periph, reg) ( periph##_BASE + offsetof(periph##_Type, reg) )

#define AM_REGADDRn(periph, n, reg) ( periph##0_BASE                    +   \
                                      offsetof(periph##0_Type, reg)     +   \
                                      (n * (periph##1_BASE - periph##0_BASE)) )

// #### INTERNAL BEGIN ####
#if 0 // Not yet tested.
//
// AM_CMREGn()
// This macro reads and returns the value of the register of the indicated
// peripheral module number using CMSIS register specifiers.
//
#define AM_CMREGn(periph, n, reg)                               \
        (*((volatile uint32_t *)(                               \
            ( periph##0_BASE + offsetof(periph##0_Type,reg) +   \
              (n * (periph##1_BASE - periph##0_BASE)) ) )))
#endif
// #### INTERNAL END ####
//*****************************************************************************
//
// Critical section assembly macros
//
// These macros implement critical section protection using inline assembly
// for various compilers.  They are intended to be used in other register
// macros or directly in sections of code.
//
// Important usage note: These macros create a local scope and therefore MUST
// be used in pairs.
//
//*****************************************************************************
#define AM_CRITICAL_BEGIN                                                   \
    if ( 1 )                                                                \
    {                                                                       \
        volatile uint32_t ui32Primask_04172010;                             \
        ui32Primask_04172010 = am_hal_interrupt_master_disable();

#define AM_CRITICAL_END                                                     \
        am_hal_interrupt_master_set(ui32Primask_04172010);                  \
    }

#ifdef __cplusplus
}
#endif

#endif // AM_REG_MACROS_H

