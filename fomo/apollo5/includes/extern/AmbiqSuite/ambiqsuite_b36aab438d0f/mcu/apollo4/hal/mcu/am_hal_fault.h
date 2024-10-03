//*****************************************************************************
//
//  am_hal_fault.h
//! @file
//!
//! @brief Functions for enable/disable fault handling and for reading the
//!        fault status from CPU block in Apollo4.
//!
//! @addtogroup cpu4 Fault Control (CPU)
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
#ifndef AM_HAL_FAULT_H
#define AM_HAL_FAULT_H

#ifdef __cplusplus
extern "C"
{
#endif

//*****************************************************************************
//
// CPU Fault Status structure
//
//*****************************************************************************
typedef struct
{
    //
    //! ICODE bus fault occurred.
    //
    bool bICODE;

    //
    //! ICODE bus fault address.
    //
    uint32_t ui32ICODE;

    //
    //! DCODE bus fault occurred.
    //
    bool bDCODE;

    //
    //! DCODE bus fault address.
    //
    uint32_t ui32DCODE;

    //
    //! SYS bus fault occurred.
    //
    bool bSYS;

    //
    //! SYS bus fault address.
    //
    uint32_t ui32SYS;
}
am_hal_fault_status_t;

// ****************************************************************************
//
//! @brief Enable Fault Capture.
//!
//!  This function is used to enable fault capture on the CPU block.
//!
//! @param None
//!
//! @return status      - generic or interface specific status.
//
// ****************************************************************************
extern uint32_t am_hal_fault_capture_enable(void);

// ****************************************************************************
//
//! @brief Disable Fault Capture.
//!
//!  This function is used to disable fault capture on the CPU block.
//!
//! @param None
//!
//! @return status      - generic or interface specific status.
//
// ****************************************************************************
extern uint32_t am_hal_fault_capture_disable(void);

// ****************************************************************************
//
//! @brief Get fault information from the CPU.
//!
//! This function returns  current fault status as obtained from the CPU block
//! in Apollo4.
//!
//! @param pFaultStatus - A pointer to a structure to receive the fault data.
//!
//! @return status      - generic or interface specific status.
//
// ****************************************************************************
extern uint32_t am_hal_fault_status_get(am_hal_fault_status_t *pFaultStatus);

#ifdef __cplusplus
}
#endif

#endif // AM_HAL_FAULT_H

//*****************************************************************************
//
// End Doxygen group.
//! @}
//
//*****************************************************************************
