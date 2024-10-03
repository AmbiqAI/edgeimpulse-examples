//*****************************************************************************
//
//! @file am_hal_mpu.h
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

#ifndef AM_HAL_MPU_H
#define AM_HAL_MPU_H

#include <stdint.h>

#ifdef __cplusplus
extern "C"
{
#endif

//*****************************************************************************
//
//! @name Macro definitions for MPU Registers
//! @{
//
//*****************************************************************************
#define MPU_TYPE                0xE000ED90 // MPU Type
#define MPU_CTRL                0xE000ED94 // MPU Control
#define MPU_RNR                 0xE000ED98 // MPU Region Number
#define MPU_RBAR                0xE000ED9C // MPU Region Base Address
#define MPU_RASR                0xE000EDA0 // MPU Region AttrSize
//! @}

//*****************************************************************************
//
//! @name MPU Alias registers.
//! @{
//
//*****************************************************************************
#define MPU_RBAR_A1             0xE000EDA4
#define MPU_RASR_A1             0xE000EDA8
#define MPU_RBAR_A2             0xE000EDAC
#define MPU_RASR_A2             0xE000EDB0
#define MPU_RBAR_A3             0xE000EDB4
#define MPU_RASR_A3             0xE000EDB8
//! @}

//
//! Default value to use for TEX,S,C,B
//
#define MPU_DEFAULT_TEXSCB      0x00050000

//*****************************************************************************
//
//! @name Macro definitions for system control block registers
//! @}
//
//*****************************************************************************
#define AM_REG_SYSCTRL_MMFSR    0xE000ED28
#define AM_REG_SYSCTRL_MMFAR    0xE000ED34
//! @}

//
//! Simple macro for accessing registers
//
#define REGVAL(x)               (*(volatile uint32_t *)(x))

//*****************************************************************************
//
// External variable definitions
//
//*****************************************************************************

//*****************************************************************************
//
// Structure definitions.
//
//*****************************************************************************

//
//! @brief Enum type for specifying memory access priveleges for an MPU region.
//
typedef enum
{
    NO_ACCESS       = 0x0,
    PRIV_RW         = 0x1,
    PRIV_RW_PUB_RO  = 0x2,
    PRIV_RW_PUB_RW  = 0x3,
    PRIV_RO         = 0x5,
    PRIV_RO_PUB_RO  = 0x6
}
tAccessPermission;

//
//! @brief Configuration structure for MPU regions.
//
typedef struct
{
    uint8_t ui8RegionNumber;
    uint32_t ui32BaseAddress;
    uint8_t ui8Size;
    tAccessPermission eAccessPermission;
    bool bExecuteNever;
    uint16_t ui16SubRegionDisable;
}
tMPURegion;

//*****************************************************************************
//
// External function definitions
//
//*****************************************************************************

//*****************************************************************************
//
//! @brief Returns the contents of the MPU_TYPE register
//!
//! This function accesses the ARM MPU_TYPE register. It can be used to check
//! for the presence of an MPU, and to obtain basic information about the
//! implementation of the MPU.
//!
//! @return 32-bit unsigned integer representing the contents of MPU_TYPE
//
//*****************************************************************************
extern uint32_t mpu_type_get(void);

//*****************************************************************************
//
//! @brief Sets the global configuration of the MPU
//!
//! @param bMPUEnable - Enable the MPU
//! @param bPrivelegedDefault - Enable the default priveleged memory map
//! @param bFaultNMIProtect - Enable the MPU during fault handlers
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
//
//*****************************************************************************
extern void mpu_global_configure(bool bMPUEnable, bool bPrivelegedDefault,
                                 bool bFaultNMIProtect);

//*****************************************************************************
//
//! @brief Configures an MPU region.
//!
//! @param psConfig
//! @param bEnableNow
//!
//! This function performs the necessary configuration for the MPU region
//! described by the \e psConfig structure, and will also enable the region if
//! the \e bEnableNow option is true.
//
//*****************************************************************************
extern void mpu_region_configure(tMPURegion *psConfig, bool bEnableNow);

//*****************************************************************************
//
//! @brief Enable an MPU region.
//!
//! @param ui8RegionNumber
//!
//! Enable the MPU region referred to by \e ui8RegionNumber.
//!
//! @note This function should only be called after the desired region has
//! already been configured.
//
//*****************************************************************************
extern void mpu_region_enable(uint8_t ui8RegionNumber);

//*****************************************************************************
//
//! @brief Disable an MPU region.
//!
//! @param ui8RegionNumber
//!
//! Disable the MPU region referred to by \e ui8RegionNumber.
//
//*****************************************************************************
extern void mpu_region_disable(uint8_t ui8RegionNumber);

//*****************************************************************************
//
//! @brief Get the MPU region number.
//!
//! Get the MPU region number from MPU_RNR register.
//!
//! @return MPU_RNR register value.
//
//*****************************************************************************
extern uint32_t mpu_get_region_number(void);

#ifdef __cplusplus
}
#endif

#endif // AM_HAL_MPU_H

//*****************************************************************************
//
// End Doxygen group.
//! @}
//
//*****************************************************************************

