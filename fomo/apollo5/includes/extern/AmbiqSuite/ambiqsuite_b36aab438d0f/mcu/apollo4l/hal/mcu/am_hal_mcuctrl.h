//*****************************************************************************
//
//! @file am_hal_mcuctrl.h
//!
//! @brief Functions for interfacing with the MCUCTRL.
//!
//! @addtogroup mcuctrl4_4l MCUCTRL - MCU Control
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
#ifndef AM_HAL_MCUCTRL_H
#define AM_HAL_MCUCTRL_H

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C"
{
#endif
//

//****************************************************************************
//! @brief but number representing a peripheral that is using the HS XTAL
//! @details this is used by each peripheral to register the use of the HS XTAL
//! so the HFXTAL will not be disabled when a peripheral are using it
//****************************************************************************
typedef enum
{
    AM_HAL_HFXTAL_BLE_CONTROLLER_EN = 0,
    AM_HAL_HFXTAL_USB_PHI_EN        = 1,
    AM_HAL_HFXTAL_ADC_EN            = 2,
    AM_HAL_HFXTAL_AUADC_EN          = 3,
    AM_HAL_HCXTAL_DBGCTRL_EN        = 4,
    AM_HAL_HCXTAL_CLKGEN_MISC_EN    = 5,
    AM_HAL_HCXTAL_CLKGEN_CLKOUT_EN  = 6,
    AM_HAL_HCXTAL_PDM_BASE_EN       = 7,
    AM_HAL_HCXTAL_II2S_BASE_EN      = AM_HAL_HCXTAL_PDM_BASE_EN + AM_REG_PDM_NUM_MODULES,
    AM_HAL_HCXTAL_IOM_BASE_EN       = AM_HAL_HCXTAL_II2S_BASE_EN + AM_REG_I2S_NUM_MODULES,
    //
    //! this is used when setting a bit and no argument was passed (legacy)
    //
    AM_HAL_HCXTAL_DEFAULT_EN        = AM_HAL_HCXTAL_IOM_BASE_EN + AM_REG_IOM_NUM_MODULES,
    AM_HAL_HCXTAL_END_EN,
    AM_HAL_HCXTAL_X32               = 0x7FFFFFFF,

}
am_hal_mcuctrl_hfxtal_users_e;

//****************************************************************************
//
//! @brief this struct is used to pass data into am_hal_mcuctrl_control()
//
//****************************************************************************
typedef struct
{
    //
    //! bit that identifies which peripheral is requesting modification of the
    //! HF XTAL clock
    //
    uint32_t                       ui32_arg_hfxtal_user_mask ;
    //
    //! this is set if the hfxtal bit is being used
    //
    bool                           b_arg_hfxtal_in_use;
    //
    //! this is for legacy calls that would pass an argument with some enums
    //
    bool                           b_arg_apply_ext_source;
    //
    //! force the register modification
    //
    bool                           b_arg_force_update;
    //
    //! enable XTALHF GPIO output on clockout pin.
    //
    bool                           b_arg_enable_HfXtalClockout;

}
am_hal_mcuctrl_control_arg_t;

extern const am_hal_mcuctrl_control_arg_t g_amHalMcuctrlArgDefault;
extern const am_hal_mcuctrl_control_arg_t g_amHalMcuctrlArgBLEDefault;

#define MCUCTRL_CHIPPN_PARTNUM_PN_M             0xFF000000
#define MCUCTRL_CHIPPN_PARTNUM_PN_S             24

//**********************************************************
//
//! MCUCTRL XTALHSCAP defaults for Cooper
//! Refer to App Note Apollo4 Blue 32MHz Crystal Calibration
//
//**********************************************************
#define XTALHSCAP2TRIM_DEFAULT  44
#define XTALHSCAPTRIM_DEFAULT   4

#define APOLLO4_A                                                      \
  ((MCUCTRL->CHIPPN & MCUCTRL_CHIPPN_REVMAJ_Msk) ==                  \
      _VAL2FLD(MCUCTRL_CHIPPN_REVMAJ, MCUCTRL_CHIPPN_REVMAJ_A))

#define APOLLO4_A0                                                     \
  ((MCUCTRL->CHIPPN  &                                                \
    (MCUCTRL_CHIPPN_REVMAJ_Msk | MCUCTRL_CHIPPN_REVMIN_Msk)) ==      \
      (_VAL2FLD(MCUCTRL_CHIPPN_REVMAJ, MCUCTRL_CHIPPN_REVMAJ_A) |    \
        _VAL2FLD(MCUCTRL_CHIPPN_REVMIN, MCUCTRL_CHIPPN_REVMIN_0)))

#define APOLLO4_B0                                                     \
  ((MCUCTRL->CHIPPN  &                                                \
    (MCUCTRL_CHIPPN_REVMAJ_Msk | MCUCTRL_CHIPPN_REVMIN_Msk)) ==      \
      (_VAL2FLD(MCUCTRL_CHIPPN_REVMAJ, MCUCTRL_CHIPPN_REVMAJ_B) |    \
        _VAL2FLD(MCUCTRL_CHIPPN_REVMIN, MCUCTRL_CHIPPN_REVMIN_0)))

#define APOLLO4_B1                                                     \
  ((MCUCTRL->CHIPPN  &                                                \
    (MCUCTRL_CHIPPN_REVMAJ_Msk | MCUCTRL_CHIPPN_REVMIN_Msk)) ==      \
      (_VAL2FLD(MCUCTRL_CHIPPN_REVMAJ, MCUCTRL_CHIPPN_REVMAJ_B) |    \
        _VAL2FLD(MCUCTRL_CHIPPN_REVMIN, MCUCTRL_CHIPPN_REVMIN_1)))

//
// Determine if >= a given revision level.
//
#define APOLLO4_GE_B0                                                       \
        ((MCUCTRL->CHIPPN  &                                               \
           (MCUCTRL_CHIPPN_REVMAJ_Msk | MCUCTRL_CHIPPN_REVMIN_Msk)) >=    \
           (_VAL2FLD(MCUCTRL_CHIPPN_REVMAJ, MCUCTRL_CHIPPN_REVMAJ_B) |    \
            _VAL2FLD(MCUCTRL_CHIPPN_REVMIN, MCUCTRL_CHIPPN_REVMIN_0)))

#define APOLLO4_GE_B1                                                       \
        ((MCUCTRL->CHIPPN  &                                               \
           (MCUCTRL_CHIPPN_REVMAJ_Msk | MCUCTRL_CHIPPN_REVMIN_Msk)) >=    \
           (_VAL2FLD(MCUCTRL_CHIPPN_REVMAJ, MCUCTRL_CHIPPN_REVMAJ_B) |    \
            _VAL2FLD(MCUCTRL_CHIPPN_REVMIN, MCUCTRL_CHIPPN_REVMIN_1)))

#define APOLLO4_GE_C0                                                       \
        ((MCUCTRL->CHIPPN  &                                               \
           (MCUCTRL_CHIPPN_REVMAJ_Msk | MCUCTRL_CHIPPN_REVMIN_Msk)) >=    \
           (_VAL2FLD(MCUCTRL_CHIPPN_REVMAJ, MCUCTRL_CHIPPN_REVMAJ_C) |    \
            _VAL2FLD(MCUCTRL_CHIPPN_REVMIN, MCUCTRL_CHIPPN_REVMIN_0)))

#define APOLLO4_GT_A0                                                       \
        ((MCUCTRL->CHIPPN  &                                               \
           (MCUCTRL_CHIPPN_REVMAJ_Msk | MCUCTRL_CHIPPN_REVMIN_Msk)) >    \
           (_VAL2FLD(MCUCTRL_CHIPPN_REVMAJ, MCUCTRL_CHIPPN_REVMAJ_A) |    \
            _VAL2FLD(MCUCTRL_CHIPPN_REVMIN, MCUCTRL_CHIPPN_REVMIN_0)))

#define APOLLO4_GT_C0                                                       \
        ((MCUCTRL->CHIPPN  &                                               \
           (MCUCTRL_CHIPPN_REVMAJ_Msk | MCUCTRL_CHIPPN_REVMIN_Msk)) >     \
           (_VAL2FLD(MCUCTRL_CHIPPN_REVMAJ, MCUCTRL_CHIPPN_REVMAJ_C) |    \
            _VAL2FLD(MCUCTRL_CHIPPN_REVMIN, MCUCTRL_CHIPPN_REVMIN_0)))

//*****************************************************************************
//
// MCUCTRL specific definitions.
//
//*****************************************************************************
// Define the size of fields derived from the PARTNUM register
#define AM_HAL_MCUCTRL_CHIPPN_NV_SIZE_N     (_FLD2VAL(MCUCTRL_CHIPPN_MRAMSIZE, 0xFFFFFFFF) + 1)
#define AM_HAL_MCUCTRL_CHIPPN_SRAM_SIZE_N   (_FLD2VAL(MCUCTRL_CHIPPN_SRAMSIZE, 0xFFFFFFFF) + 1)
// #### INTERNAL BEGIN ####
#if 0
#define AM_HAL_MCUCTRL_CHIPPN_TCM_SIZE_N    AM_HAL_MCUCTRL_CHIPPN_SRAM_SIZE_N
#endif
// #### INTERNAL END ####

// Define the size of fields derived from the SKU register
#define AM_HAL_MCUCTRL_SKU_SSRAM_SIZE_N     (_FLD2VAL(MCUCTRL_SKU_SKUSRAMSIZE, 0xFFFFFFFF) + 1)
#define AM_HAL_MCUCTRL_SKU_MRAM_SIZE_N      (_FLD2VAL(MCUCTRL_SKU_SKUMRAMSIZE, 0xFFFFFFFF) + 1)

#define AM_HAL_MCUCTRL_CRYSTAL_IBST_DURATION    500

//*****************************************************************************
//
// MCUCTRL enumerations
//
//*****************************************************************************
//**************************************
//! MCUCTRL control operations
//**************************************
typedef enum
{
    AM_HAL_MCUCTRL_CONTROL_EXTCLK32K_ENABLE,
    AM_HAL_MCUCTRL_CONTROL_EXTCLK32K_DISABLE,
    AM_HAL_MCUCTRL_CONTROL_EXTCLK32M_KICK_START,
    AM_HAL_MCUCTRL_CONTROL_EXTCLK32M_NORMAL,
    AM_HAL_MCUCTRL_CONTROL_EXTCLK32M_DISABLE,
    AM_HAL_MCUCTRL_CONTROL_EXTCLK32M_CLOCKOUT,
}
am_hal_mcuctrl_control_e;

//**************************************
//
//! MCUCTRL info get
//
//**************************************
typedef enum
{
    AM_HAL_MCUCTRL_INFO_FEATURES_AVAIL,
    AM_HAL_MCUCTRL_INFO_DEVICEID
}
am_hal_mcuctrl_infoget_e;

//*****************************************************************************
//
//! MCUCTRL SKU/Feature Enums
//
//*****************************************************************************
typedef enum
{
    AM_HAL_MCUCTRL_DTCM_384K
} am_hal_mcuctrl_dtcm_e;

//
// #### INTERNAL BEGIN ####
// Per TBIRD-3, only 2 SRAM SKUs are valid for revC and TBird:
// #### INTERNAL END ####
//! SKU SSRAM size: 0=1MB,   1=2MB, 2=1MB,   3=2MB
//! SKU MRAM  size: 0=0.5MB, 1=1MB, 2=1.5MB, 3=2MB,
//
typedef enum
{
    AM_HAL_MCUCTRL_SSRAM_1M_PLUS_EXT0,  // 1024KB SSRAM + 384KB Ext
    AM_HAL_MCUCTRL_SSRAM_2M_PLUS_EXT1,  // 2048KB SSRAM + 384KB Ext
    AM_HAL_MCUCTRL_SSRAM_1M_PLUS_EXT,   // 1024KB SSRAM + 384KB Ext
    AM_HAL_MCUCTRL_SSRAM_2M_PLUS_EXT,   // 2048KB SSRAM + 384KB Ext
} am_hal_mcuctrl_ssram_e;
#define AM_HAL_MCUCTRL_SSRAM_MAX    AM_HAL_MCUCTRL_SSRAM_2M_PLUS_EXT

//
//! MRAM Size Setting
//
typedef enum
{
    AM_HAL_MCUCTRL_MRAM_0P5M,
    AM_HAL_MCUCTRL_MRAM_1M,
    AM_HAL_MCUCTRL_MRAM_1P5M,
    AM_HAL_MCUCTRL_MRAM_2M,
} am_hal_mcuctrl_mram_e;

#define AM_HAL_MCUCTRL_MRAM_MAX     AM_HAL_MCUCTRL_MRAM_2M

//
//! MRAM GP/LL Setting
//
typedef enum
{
    AM_HAL_MCUCTRL_CM4F_ONLY,
    AM_HAL_MCUCTRL_CM4F_GPDSP,
    AM_HAL_MCUCTRL_CM4F_GPDSP_LLDSP
} am_hal_mcuctrl_dsp_e;

//*****************************************************************************
//
// MCUCTRL data structures
//
//*****************************************************************************
//**************************************
//
//! MCUCTRL device structure
//
//**************************************
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
    //! SKU (Apollo4l).
    //
    uint32_t ui32SKU;

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
    //! Note: Total onboard SRAM is computed as ui32DTCMSize + ui32SSRAMSize.
    //
    uint32_t ui32DTCMSize;

    //
    //! SSRAM Size.
    //
    uint32_t ui32SSRAMSize;

// #### INTERNAL BEGIN ####
#if 0
// Note: We considered adding a total RAM for Apollo4p, but decided against it
//       since it would be incompatible with the Apollo4b SDK in the R4.0.0
//       release. Plus the total can be computed from the TCM + SSRAM values.
    //
    //! Total RAM Size (TCM + SRAM + Extended)
    //
    uint32_t ui32TotalRAMSize;
#endif
// #### INTERNAL END ####
    //
    //! MRAM Size.
    //
    uint32_t ui32MRAMSize;

    //
    // JEDEC chip info
    //
    uint32_t ui32JedecPN;
    uint32_t ui32JedecJEPID;
    uint32_t ui32JedecCHIPREV;
    uint32_t ui32JedecCID;
} am_hal_mcuctrl_device_t;

//**************************************
//
//! MCUCTRL status structure
//
//**************************************
typedef struct
{
    bool        bDebuggerLockout;   // DEBUGGER
    bool        bADCcalibrated;     // ADCCAL
    bool        bBattLoadEnabled;   // ADCBATTLOAD
    uint8_t     bSecBootOnWarmRst;  // BOOTLOADER
    uint8_t     bSecBootOnColdRst;  // BOOTLOADER
} am_hal_mcuctrl_status_t;

//**************************************
//
//! MCUCTRL features available structure
//
//**************************************
typedef struct
{
    am_hal_mcuctrl_dtcm_e       eDTCMSize;
    am_hal_mcuctrl_ssram_e      eSharedSRAMSize;
    am_hal_mcuctrl_mram_e       eMRAMSize;
    bool                        bTurboSpot;
    bool                        bDisplayCtrl;
    bool                        bGPU;
    bool                        bUSB;
    bool                        bSecBootFeature;
} am_hal_mcuctrl_feature_t;

//**********************************************************
//
//! MCUCTRL XTALHSCAP Globals for Cooper Device
//! Refer to App Note Apollo4 Blue 32MHz Crystal Calibration
//
//**********************************************************
extern uint32_t g_ui32xtalhscap2trim;
extern uint32_t g_ui32xtalhscaptrim;

/******************************************************************************
//! @brief get usage status of HF XTAL clock
//!
//! @return true   if one or more users (modules) are using the HF XTAL clock
//! @return false  if the XTAL clock is disabled and there are no users
 *****************************************************************************/
extern bool am_hal_mcuctrl_EXTCLK_active(void );

// ****************************************************************************
//
//! @brief Apply various specific commands/controls on the MCUCTRL module.
//!
//! This function is used to apply various controls to MCUCTRL.
//!
//! @param eControl - One of the following:
//!     AM_HAL_MCUCTRL_CONTROL_EXTCLK32K_ENABLE,
//!     AM_HAL_MCUCTRL_CONTROL_EXTCLK32K_DISABLE,
//!     AM_HAL_MCUCTRL_CONTROL_EXTCLK32M_KICK_START,
//!     AM_HAL_MCUCTRL_CONTROL_EXTCLK32M_NORMAL,
//!     AM_HAL_MCUCTRL_CONTROL_EXTCLK32M_DISABLE,
//!     AM_HAL_MCUCTRL_CONTROL_EXTCLK32M_CLOCKOUT,
//!
//! @param pArgs - Pointer to arguments for Control Switch Case, see note and example below
//!
//! @note pArgs: new use for SDK Rev 4.4:<br>
//! to use the HF XTAL clock this function now expects a pointer the following
//! struct variable<br>
//!      am_hal_mcuctrl_control_arg_t<br>
//! this is needed for the following eControl keywords
//!      AM_HAL_MCUCTRL_CONTROL_EXTCLK32M_KICK_START,
//!      AM_HAL_MCUCTRL_CONTROL_EXTCLK32M_NORMAL,
//!      AM_HAL_MCUCTRL_CONTROL_EXTCLK32M_DISABLE,
//!      AM_HAL_MCUCTRL_CONTROL_EXTCLK32M_CLOCKOUT,
//!
//! @code{.unparsed}
//!
//!   am_hal_mcuctrl_control_arg_t cvar = g_amHalMcuctrlArgDefault;<br>
//!
//!   //
//!   // the user must choose the appropriate bit from the enum am_hal_mcuctrl_hfxtal_users_e<br>
//!   // there are numerous examples that use this structure<br>
//!   //
//!   cvar.ui32_arg_hfxtal_user_mask = (1 \<\< AM_HAL_HFXTAL_AUADC_EN);<br>
//!
//!   //
//!   // populate the remaing fields as needed or use the default values<br>
//!   //
//!   retStat = am_hal_mcuctrl_control( AM_HAL_MCUCTRL_CONTROL_EXTCLK32M_NORMAL, (void *) &cvar );<br>
//!
//! @endcode
//!
//! @return status      - generic or interface specific status.
//
// ****************************************************************************
extern uint32_t am_hal_mcuctrl_control(am_hal_mcuctrl_control_e eControl,
                                       void *pArgs);

// ****************************************************************************
//
//! @brief MCUCTRL status function
//!
//! This function returns current status of the MCUCTRL as obtained from
//! various registers of the MCUCTRL block.
//!
//! @param psStatus - ptr to a status structure to receive the current statuses.
//!
//! @return status      - generic or interface specific status.
//
// ****************************************************************************
extern uint32_t am_hal_mcuctrl_status_get(am_hal_mcuctrl_status_t *psStatus);

// ****************************************************************************
//
//! @brief Get information of the given MCUCTRL item.
//!
//! This function returns a data structure of information regarding the given
//! MCUCTRL parameter.
//!
//! @param eInfoGet - One of the following:         Return structure type:
//!     AM_HAL_MCUCTRL_INFO_DEVICEID,               psDevice
//!     AM_HAL_MCUCTRL_INFO_FAULT_STATUS            psFault
//!
//! @param pInfo - A pointer to a structure to receive the return data,
//! the type of which is dependent on the eInfo parameter.
//!
//! @return status      - generic or interface specific status.
//
// ****************************************************************************
extern uint32_t am_hal_mcuctrl_info_get(am_hal_mcuctrl_infoget_e eInfoGet,
                                        void *pInfo);

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
