//*****************************************************************************
//
//  am_hal_mcu_sysctrl.h
//! @file
//!
//! @brief Functions for interfacing with the M4F system control registers
//!
//! @addtogroup sysctrl4 System Control (SYSCTRL)
//! @ingroup apollo4hal
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
#ifndef AM_HAL_MCU_SYSCTRL_H
#define AM_HAL_MCU_SYSCTRL_H

#ifdef __cplusplus
extern "C"
{
#endif

//*****************************************************************************
//
// Write flush - This function will hold the bus until all queued write
// operations have completed, thereby guaranteeing that all writes have
// been flushed.
//
//*****************************************************************************
#define am_hal_sysctrl_membarrier()                                     \
    if (1)                                                              \
    {                                                                   \
        __DMB();                                                        \
        am_hal_sysctrl_bus_write_flush();                               \
    }

//
// This Write will begin only after all previous load/store operations are done
//
#define am_hal_sysctrl_membarrier_write(addr, data)                     \
    if (1)                                                              \
    {                                                                   \
        am_hal_sysctrl_membarrier();                                    \
        *((uint32_t *)(addr)) = (data);                                 \
    }

//
// This Read will be performed before any subsequent load/store operations are done
//
#define am_hal_sysctrl_membarrier_read(addr)                            \
    if (1)                                                              \
    {                                                                   \
        AM_REGVAL((addr));                                              \
        am_hal_sysctrl_membarrier();                                    \
    }

//*****************************************************************************
//
// External function definitions
//
//*****************************************************************************
extern void am_hal_sysctrl_fpu_enable(void);
extern void am_hal_sysctrl_fpu_disable(void);
extern void am_hal_sysctrl_fpu_stacking_enable(bool bLazy);
extern void am_hal_sysctrl_fpu_stacking_disable(void);
extern void am_hal_sysctrl_aircr_reset(void);
#ifdef __cplusplus
}
#endif

#endif // AM_HAL_MCU_SYSCTRL_H

//*****************************************************************************
//
// End Doxygen group.
//! @}
//
//*****************************************************************************

