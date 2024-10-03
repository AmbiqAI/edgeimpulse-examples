//*****************************************************************************
//
//  am_hal_dsp_sysctrl.h
//! @file
//!
//! @brief Functions for interfacing with the DSP system control registers
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
#ifndef AM_HAL_DSP_SYSCTRL_H
#define AM_HAL_DSP_SYSCTRL_H

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
// TODO: Do we need to do additional operation to flush PIF to AXI Buffers?
#define _am_hal_dsp_flush_pif2axi()

#define am_hal_sysctrl_membarrier()                                     \
    if (1)                                                              \
    {                                                                   \
        XT_MEMW();                                                     \
        _am_hal_dsp_flush_pif2axi();                                   \
    }

// make sure all pending store/load are done
#define am_hal_sysctrl_bus_write_flush()                am_hal_sysctrl_membarrier()

// This Write will begin only after all previous load/store operations are done
#define am_hal_sysctrl_membarrier_write(addr, data32)                     \
    if (1)                                                              \
    {                                                                   \
        _am_hal_dsp_flush_pif2axi();                                   \
        XT_S32RI((data32), (addr), 0);                                    \
    }

// This Read will be performed before any subsequent load/store operations are done
#define am_hal_sysctrl_membarrier_read(addr)                            \
    if (1)                                                              \
    {                                                                   \
        volatile uint32_t ui32tmp;                                      \
        ui32tmp = XT_L32AI((addr), 0);                                  \
    }

//*****************************************************************************
//
// External function definitions
//
//*****************************************************************************
// Complete shut off
extern void am_hal_sysctrl_turnoff(void);

#ifdef __cplusplus
}
#endif

#endif // AM_HAL_DSP_SYSCTRL_H

//*****************************************************************************
//
// End Doxygen group.
//! @}
//
//*****************************************************************************
