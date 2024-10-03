//*****************************************************************************
//
//! @file mpu.h
//!
//! @brief TODO:INSERT DESCRIPTION
//
//*****************************************************************************

//*****************************************************************************
//
// ${copyright}
//
// This is part of revision ${version} of the AmbiqSuite Development Package.
//
//*****************************************************************************

#ifndef MPU_H
#define MPU_H

#include <stdint.h>

#ifdef __cplusplus
extern "C"
{
#endif
//*****************************************************************************
//
// Macro definitions for MPU Registers
//
//*****************************************************************************
#define MPU_TYPE                0xE000ED90 // MPU Type
#define MPU_CTRL                0xE000ED94 // MPU Control
#define MPU_RNR                 0xE000ED98 // MPU Region Number
#define MPU_RBAR                0xE000ED9C // MPU Region Base Address
#define MPU_RASR                0xE000EDA0 // MPU Region AttrSize

// MPU Alias registers.
#define MPU_RBAR_A1             0xE000EDA4
#define MPU_RASR_A1             0xE000EDA8
#define MPU_RBAR_A2             0xE000EDAC
#define MPU_RASR_A2             0xE000EDB0
#define MPU_RBAR_A3             0xE000EDB4
#define MPU_RASR_A3             0xE000EDB8

// Default value to use for TEX,S,C,B
#define MPU_DEFAULT_TEXSCB      0x00050000

// Simple macro for accessing registers
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
//! \brief Enum type for specifying memory access priveleges for an MPU region.
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
//! \brief Configuration structure for MPU regions.
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
extern uint32_t mpu_type_get(void);

extern void mpu_global_configure(bool bMPUEnable, bool bPrivelegedDefault,
                                 bool bFaultNMIProtect);

extern void mpu_region_configure(tMPURegion *psConfig, bool bEnableNow);

extern void mpu_region_enable(uint8_t ui8RegionNumber);

extern void mpu_region_disable(uint8_t ui8RegionNumber);

#ifdef __cplusplus
}
#endif

#endif // MPU_H

