//*****************************************************************************
//
//  am_hal_system.h
//! @file
//!
//! @brief Apollo4 system-wide definitions.
//!
//
//*****************************************************************************

//*****************************************************************************
//
// ${copyright}
//
// This is part of revision ${version} of the AmbiqSuite Development Package.
//
//*****************************************************************************

#ifndef AM_HAL_SYSTEM_H
#define AM_HAL_SYSTEM_H

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdint.h>
#include <stdbool.h>


//*****************************************************************************
//
// Some helpful macros defining addresses and sizes.
//
//*****************************************************************************
#define AM_HAL_SYSTEM_NVRAM_SBL_OFFSET          0x00000000  // 0x0000C000
#define AM_HAL_SYSTEM_NVRAM_ADDR                (0x00000000 + AM_HAL_SYSTEM_NVRAM_SBL_OFFSET)
#define AM_HAL_SYSTEM_NVRAM_SIZE                ((2 * 1024 * 1024) - AM_HAL_SYSTEM_NVRAM_SBL_OFFSET)
#define AM_HAL_SYSTEM_NVRAM_NWDS                (AM_HAL_SYSTEM_NVRAM_SIZE / 4)

#define AM_HAL_SYSTEM_MCU_DTCM_ADDR             0x10000000
#define AM_HAL_SYSTEM_MCU_DTCM_SIZE             (128 * 1024)
#define AM_HAL_SYSTEM_MCU_DTCM_NWDS             (AM_HAL_SYSTEM_MCU_DTCM_SIZE / 4)

#define AM_HAL_SYSTEM_SSRAM_ADDR                (0x10000000 + (384 * 1024))
#define AM_HAL_SYSTEM_SSRAM_SIZE                (1024 * 1024)
#define AM_HAL_SYSTEM_SSRAM_NWDS                (AM_HAL_SYSTEM_SSRAM_SIZE / 4)


#ifdef __cplusplus
}
#endif

#endif // AM_HAL_SYSTEM_H

//*****************************************************************************
//
// End Doxygen group.
//! @}
//
//*****************************************************************************
