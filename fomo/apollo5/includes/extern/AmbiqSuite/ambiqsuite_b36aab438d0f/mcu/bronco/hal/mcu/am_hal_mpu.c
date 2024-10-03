//*****************************************************************************
//
//! @file am_hal_mpu.c
//!
//! @brief Hardware abstraction for the MPU.
//!
//! @addtogroup mpu MPU - Memory Protection Unit
//! @ingroup bronco_hal
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
#include "am_hal_mpu.h"

//*****************************************************************************
//
// Returns the contents of the MPU_TYPE register
//
//*****************************************************************************
uint32_t
mpu_type_get(void)
{
    return REGVAL(MPU_TYPE);
}

//*****************************************************************************
//
// Sets the global configuration of the MPU
//
//*****************************************************************************
void
mpu_global_configure(bool bMPUEnable, bool bPrivelegedDefault,
                     bool bFaultNMIProtect)
{
    __DMB();
    REGVAL(MPU_CTRL) = ((bMPUEnable * 0x1) |
                        (bFaultNMIProtect * 0x2) |
                        (bPrivelegedDefault * 0x4));

    SCB->SHCSR |= SCB_SHCSR_MEMFAULTENA_Msk;
    __DSB();
    __ISB();
}

//*****************************************************************************
//
// Configures an MPU region.
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
// Enable an MPU region.
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
// Disable an MPU region.
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

//*****************************************************************************
//
// Get the MPU region number.
//
//*****************************************************************************
uint32_t
mpu_get_region_number(void)
{
    return REGVAL(MPU_RNR);
}

//*****************************************************************************
//
// End Doxygen group.
//! @}
//
//*****************************************************************************

