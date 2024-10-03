//*****************************************************************************
//
//  am_hal_mcuctrl.h
//! @file
//!
//! @brief Functions for accessing and configuring the MCUCTRL.
//!
//! @addtogroup mcuctrl1 MCU Control (MCUCTRL)
//! @ingroup apollo1hal
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
#ifndef AM_HAL_MCUCTRL_H
#define AM_HAL_MCUCTRL_H

//*****************************************************************************
//
// Apollo Number Decode.
//
//*****************************************************************************
extern const uint32_t am_hal_mcuctrl_flash_size[];
extern const uint32_t am_hal_mcuctrl_sram_size[];

//*****************************************************************************
//
// FLASH Bank Power defines.
//
//*****************************************************************************
#define AM_HAL_MCUCTRL_FLASH_POWER_DOWN_NONE    0x0
#define AM_HAL_MCUCTRL_FLASH_POWER_DOWN_0       0x1
#define AM_HAL_MCUCTRL_FLASH_POWER_DOWN_1       0x2
#define AM_HAL_MCUCTRL_FLASH_POWER_DOWN_ALL     0x3

//*****************************************************************************
//
// SRAM Bank Power defines.
//
//*****************************************************************************
#define AM_HAL_MCUCTRL_SRAM_POWER_DOWN_NONE    0x0
#define AM_HAL_MCUCTRL_SRAM_POWER_DOWN_0       0x1
#define AM_HAL_MCUCTRL_SRAM_POWER_DOWN_1       0x2
#define AM_HAL_MCUCTRL_SRAM_POWER_DOWN_2       0x4
#define AM_HAL_MCUCTRL_SRAM_POWER_DOWN_3       0x8
#define AM_HAL_MCUCTRL_SRAM_POWER_DOWN_4       0x10
#define AM_HAL_MCUCTRL_SRAM_POWER_DOWN_5       0x20
#define AM_HAL_MCUCTRL_SRAM_POWER_DOWN_6       0x40
#define AM_HAL_MCUCTRL_SRAM_POWER_DOWN_7       0x80
#define AM_HAL_MCUCTRL_SRAM_POWER_DOWN_ALL     0xFF


// #### INTERNAL BEGIN ####
//*****************************************************************************
//
// BOD and POR power down defines
//
//*****************************************************************************
#define AM_HAL_MCUCTRL_BODPOR_POWER_DOWN_NONE  0x0
#define AM_HAL_MCUCTRL_BODPOR_POWER_DOWN_BOD AM_REG_MCUCTRL_BODPORCTRL_PWDBOD_M
#define AM_HAL_MCUCTRL_BODPOR_POWER_DOWN_POR AM_REG_MCUCTRL_BODPORCTRL_PWDPDR_M
// #### INTERNAL END ####

//*****************************************************************************
//
//! MCUCTRL device structure
//
//*****************************************************************************
typedef struct
{
    //
    //! Device part number. (BCD format)
    //
    uint32_t ui32ChipPN;

    //
    //! Unique Chip ID 0.
    //
    uint32_t ui32ChipID0;

    //
    //! Unique Chip ID 1.
    //
    uint32_t ui32ChipID1;

    //
    //! Chip Revision.
    //
    uint32_t ui32ChipRev;

    //
    //! Vendor ID.
    //
    uint32_t ui32VendorID;

    //
    //! Qualified chip.
    //
    uint32_t ui32Qualified;

    //
    //! Flash Size.
    //
    uint32_t ui32FlashSize;

    //
    //! SRAM Size.
    //
    uint32_t ui32SRAMSize;

    //
    // JEDEC chip info
    //
    uint32_t ui32JedecPN;
    uint32_t ui32JedecJEPID;
    uint32_t ui32JedecCHIPREV;
    uint32_t ui32JedecCID;
}
am_hal_mcuctrl_device_t;

//*****************************************************************************
//
//! MCUCTRL fault structure
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
am_hal_mcuctrl_fault_t;

#ifdef __cplusplus
extern "C"
{
#endif

//*****************************************************************************
//
// External function definitions
//
//*****************************************************************************
extern void am_hal_mcuctrl_device_info_get(am_hal_mcuctrl_device_t *psDevice);
extern void am_hal_mcuctrl_fault_capture_enable(void);
extern void am_hal_mcuctrl_fault_capture_disable(void);
extern void am_hal_mcuctrl_fault_status(am_hal_mcuctrl_fault_t *psFault);
extern void am_hal_mcuctrl_flash_power_set(uint32_t ui32FlashPower);
extern void am_hal_mcuctrl_sram_power_set(uint32_t ui32SRAMPower,
                                         uint32_t ui32SRAMPowerDeepSleep);
extern void am_hal_mcuctrl_bandgap_enable(void);
extern void am_hal_mcuctrl_bandgap_disable(void);
extern void am_hal_mcuctrl_bucks_enable(void);
extern void am_hal_mcuctrl_bucks_disable(void);
// #### INTERNAL BEGIN ####
extern void am_hal_mcuctrl_pmu_enable(void);
extern void am_hal_mcuctrl_pmu_disable(void);
extern void am_hal_mcuctrl_bodpor_control(uint32_t ui32BODPOR);
// #### INTERNAL END ####

#ifdef __cplusplus
}
#endif

#endif // AM_HAL_MCUCTRL_H

//*****************************************************************************
//
// End Doxygen group.
//! @}
//
//*****************************************************************************
