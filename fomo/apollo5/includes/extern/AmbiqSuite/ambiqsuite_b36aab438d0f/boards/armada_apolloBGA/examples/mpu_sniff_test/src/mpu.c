//*****************************************************************************
//
//! @file mpu.c
//!
//! @brief MPU sniff test.
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
#include "mpu.h"

//*****************************************************************************
//
// Macro definitions
//
//*****************************************************************************

//*****************************************************************************
//
// Global Variables
//
//*****************************************************************************

//*****************************************************************************
//
//! \brief Returns the contents of the MPU_TYPE register
//!
//! This function accesses the ARM MPU_TYPE register. It can be used to check
//! for the presence of an MPU, and to obtain basic information about the
//! implementation of the MPU.
//!
//! \return 32-bit unsigned integer representing the contents of MPU_TYPE
//
//*****************************************************************************
uint32_t
mpu_type_get(void)
{
    return REGVAL(MPU_TYPE);
}

//*****************************************************************************
//
//! \brief Sets the global configuration of the MPU
//!
//! \param bMPUEnable - Enable the MPU
//! \param bPrivelegedDefault - Enable the default priveleged memory map
//! \param bFaultNMIProtect - Enable the MPU during fault handlers
//!
//! This function is a wrapper for the MPU_CTRL register, which controls the
//! global configuration of the MPU. This function can enable or disable the
//! MPU overall with the \e bMPUEnable parameter, and also controls how fault
//! handlers, NMI service routines, and priveleged-mode execution is handled by
//! the MPU.
//!
//! Setting \e bPrivelegedDefault will enable the default memory map for
//! priveleged accesses. If the MPU is enabled with this value set, only
//! priveleged code can execute from the system address map
//!
//! Setting \e bFaultNMIProtect leaves the MPU active during the execution of
//! NMI and Hard Fault handlers. Clearing this value will disable the MPU
//! during these procedures.
//!
//! \return None.
//
//*****************************************************************************
void
mpu_global_configure(bool bMPUEnable, bool bPrivelegedDefault,
                     bool bFaultNMIProtect)
{
    REGVAL(MPU_CTRL) = ((bMPUEnable * 0x1) |
                        (bFaultNMIProtect * 0x2) |
                        (bPrivelegedDefault * 0x4));
}

//*****************************************************************************
//
//! \brief Configures an MPU region.
//!
//! \param psConfig
//! \param bEnableNow
//!
//! \description
//! This function performs the necessary configuration for the MPU region
//! described by the \e psConfig structure, and will also enable the region if
//! the \e bEnableNow option is true.
//!
//! \return None.
//
//*****************************************************************************
void
mpu_region_configure(tMPURegion *psConfig, bool bEnableNow)
{
    //
    // Set the new base address for the specified region.
    //
    REGVAL(MPU_RBAR) = ((psConfig->ui32BaseAddress & 0xFFFFFFE0) |
                        (psConfig->ui8RegionNumber) |
                        (0x00000010));

    //
    // Set the attributes for this region based on the input structure.
    //
    REGVAL(MPU_RASR) = ((psConfig->bExecuteNever * 0x10000000) |
                        (psConfig->eAccessPermission << 24) |
                        (psConfig->ui16SubRegionDisable << 8) |
                        (psConfig->ui8Size << 1) |
                        (bEnableNow) |
                        (MPU_DEFAULT_TEXSCB));

}

//*****************************************************************************
//
//! \brief Enable an MPU region.
//!
//! \param ui8RegionNumber
//!
//! Enable the MPU region referred to by \e ui8RegionNumber.
//!
//! \note This function should only be called after the desired region has
//! already been configured.
//!
//! \return None.
//
//*****************************************************************************
void
mpu_region_enable(uint8_t ui8RegionNumber)
{
    //
    // Set the region number in the MPU_RNR register, and set the enable bit.
    //
    REGVAL(MPU_RNR) = ui8RegionNumber;
    REGVAL(MPU_RASR) |= 0x1;
}

//*****************************************************************************
//
//! \brief Disable an MPU region.
//!
//! \param ui8RegionNumber
//!
//! Disable the MPU region referred to by \e ui8RegionNumber.
//!
//! \return None.
//
//*****************************************************************************
void
mpu_region_disable(uint8_t ui8RegionNumber)
{
    //
    // Set the region number in the MPU_RNR register, and clear the enable bit.
    //
    REGVAL(MPU_RNR) = ui8RegionNumber;
    REGVAL(MPU_RASR) &= ~(0x1);
}
