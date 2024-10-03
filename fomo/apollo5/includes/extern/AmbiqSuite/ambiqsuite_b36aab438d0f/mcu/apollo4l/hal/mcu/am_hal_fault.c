//*****************************************************************************
//
//! @file am_hal_fault.c
//!
//! @brief Functions for interfacing with the fault control.
//!
//! @addtogroup fault_4l Fault - CPU Fault Control
//! @ingroup apollo4l_hal
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

#include <stdint.h>
#include <stdbool.h>
#include "am_mcu_apollo.h"

// ****************************************************************************
//
//  am_hal_fault_capture_enable()
//  This function is used to enable fault capture on the CPU block.
//
// ****************************************************************************
uint32_t
am_hal_fault_capture_enable(void)
{
    //
    // Enable the Fault Capture registers.
    //
    CPU->FAULTCAPTUREEN_b.FAULTCAPTUREEN = CPU_FAULTCAPTUREEN_FAULTCAPTUREEN_EN;

    //
    // Return success status.
    //
    return AM_HAL_STATUS_SUCCESS;
} // am_hal_fault_capture_enable()

// ****************************************************************************
//
//  am_hal_fault_capture_disable()
//  This function is used to disable fault capture on the CPU block.
//
// ****************************************************************************
uint32_t
am_hal_fault_capture_disable(void)
{
    //
    // Enable the Fault Capture registers.
    //
    CPU->FAULTCAPTUREEN_b.FAULTCAPTUREEN = CPU_FAULTCAPTUREEN_FAULTCAPTUREEN_DIS;

    //
    // Return success status.
    //
    return AM_HAL_STATUS_SUCCESS;
} // am_hal_fault_capture_disable()

// ****************************************************************************
//
// am_hal_fault_status_get()
// This function returns  current fault status as obtained from the CPU block
// in Apollo4.
//
// ****************************************************************************
uint32_t
am_hal_fault_status_get(am_hal_fault_status_t *pFaultStatus)
{
    uint32_t  ui32FaultStat;

    if ( pFaultStatus == NULL )
    {
        return AM_HAL_STATUS_INVALID_ARG;
    }

    //
    // Read the Fault Status Register.
    //
    ui32FaultStat = CPU->FAULTSTATUS;
    pFaultStatus->bICODE = (bool)(ui32FaultStat & CPU_FAULTSTATUS_ICODEFAULT_Msk);
    pFaultStatus->bDCODE = (bool)(ui32FaultStat & CPU_FAULTSTATUS_DCODEFAULT_Msk);
    pFaultStatus->bSYS   = (bool)(ui32FaultStat & CPU_FAULTSTATUS_SYSFAULT_Msk);

    //
    // Read the DCODE fault capture address register.
    //
    pFaultStatus->ui32DCODE = CPU->DCODEFAULTADDR;

    //
    // Read the ICODE fault capture address register.
    //
    pFaultStatus->ui32ICODE |= CPU->ICODEFAULTADDR;

    //
    // Read the ICODE fault capture address register.
    //
    pFaultStatus->ui32SYS |= CPU->SYSFAULTADDR;

    //
    // Return success status.
    //
    return AM_HAL_STATUS_SUCCESS;
} // am_hal_fault_status_get()

//*****************************************************************************
//
// End Doxygen group.
//! @}
//
//*****************************************************************************
