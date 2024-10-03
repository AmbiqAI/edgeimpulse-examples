//*****************************************************************************
//
//! @file am_hal_pwrctrl.c
//!
//! @brief Functions for enabling and disabling power domains.
//!
//! @addtogroup pwrctrl4 PWRCTRL - Power Control
//! @ingroup apollo5a_hal
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
// #### INTERNAL BEGIN ####
//#define TEMPCO_DBG 1
#if TEMPCO_DBG
extern uint32_t am_util_stdio_printf(const char *pui8Fmt, ...);
#endif
// #### INTERNAL END ####

//*****************************************************************************
//
// Local defines
//
//*****************************************************************************

//
//! Maximum number of checks to memory power status before declaring error
// (5 x 1usec = 5usec).
//
#define AM_HAL_PWRCTRL_MAX_WAIT_US          5
#define AM_HAL_PWRCTRL_MAX_BOOTROM_COUNT    10000

#define AM_HAL_PWRCTRL_MEMPWRSTATUS_MASK    ( PWRCTRL_MEMPWRSTATUS_PWRSTDTCM_Msk       |    \
                                              PWRCTRL_MEMPWRSTATUS_PWRSTNVM0_Msk       |    \
                                              PWRCTRL_MEMPWRSTATUS_PWRSTCACHE_Msk      |    \
                                              PWRCTRL_MEMPWRSTATUS_PWRSTROM_Msk )

#define TRIM_WR_ENABLE                         \
        AM_REGVAL(0x400401fc) = 0x2EF1543E;    \
        AM_REGVAL(0x400400f8) = 0xDF328C18;    \
        AM_REGVAL(0x40040144) = 0x8299B572;    \
        AM_REGVAL(0x40040278) = 0xA07DB9C8;

#define TRIM_WR_DISABLE                        \
        AM_REGVAL(0x400401fc) = 0;             \
        AM_REGVAL(0x400400f8) = 0;             \
        AM_REGVAL(0x40040144) = 0;             \
        AM_REGVAL(0x40040278) = 0;

//
// Power trims
//
#define ADJUST_POWER_SETTING           true
#define RESTORE_POWER_SETTING          false
#define CORELDOTEMPCOTRIM_VAL          2

#define MEMLDOACTIVETRIM_DELTA_0        5
#define TVRGFVREFTRIM_DELTA_0           24

#define MEMLDOACTIVETRIM_DELTA_1        23

#define TVRGFVREFTRIM_VAL_0             127
#define TVRGFTEMPCOTRIM_VAL_0           31

#define VDDCACTLOWTONTRIM_VAL           21
#define VDDFACTLOWTONTRIM_VAL           31

//
//! Trim revision 6 is required for the following pwrctrl adjustments:
//!  - Sourcing MCUH from the VDDC_LV rail
//!  - Reliable application of TempCo
//!  - Applying Crypto boosts
// #### INTERNAL BEGIN ####
//  TRIMREV_VDDC_LV see FALCSW-286
//  TRIMREV_TEMPCO see FALCSW-253
//  TRIMREV_BOOST_ADJUST see FALCSW-348.
// #### INTERNAL END ####
//
#define TRIMREV_PWRCTRL         6

#define AM_HAL_PWRCTRL_GPU_VOLTADJ_WAIT  1
#define AM_HAL_PWRCTRL_GPU_PWRADJ_WAIT   6
#define AM_HAL_PWRCTRL_GPU_PWRON_WAIT    1

#define CORE_SLEEP_DURATION            50  //in us

static uint32_t nvic_en[16];
static uint32_t origBasePri, orgPriMask;
static bool g_bSTIntEnabled = false;

//*****************************************************************************
//
//! @name Define max values of some useful fields
//! @{
//
// ****************************************************************************
#define MAX_ACTTRIMVDDF         _FLD2VAL(MCUCTRL_SIMOBUCK12_ACTTRIMVDDF, 0xFFFFFFFF)    // Buck VDDF
#define MAX_MEMLDOACTIVETRIM    _FLD2VAL(MCUCTRL_LDOREG2_MEMLDOACTIVETRIM, 0xFFFFFFFF)  // LDO VDDF
#define MAX_LPTRIMVDDF          _FLD2VAL(MCUCTRL_SIMOBUCK12_LPTRIMVDDF, 0xFFFFFFFF)     // VDDF LP
#define MAX_MEMLPLDOTRIM        _FLD2VAL(MCUCTRL_LDOREG2_MEMLPLDOTRIM, 0xFFFFFFFF)      // MEM LP LDO
#define MAX_BUCKVDDCTRIM        _FLD2VAL(MCUCTRL_VREFGEN2_TVRGVREFTRIM, 0xFFFFFFFF)     // Buck VDDC
//! @}

// ****************************************************************************
//
// Global variables.
//
// ****************************************************************************

//*****************************************************************************
//
//! @name Global State Variables for the VDDF and VDDC boosting
//! @{
//
// ****************************************************************************
am_hal_pwrctrl_mcu_mode_e       g_eCurMcuPwrMode    = AM_HAL_PWRCTRL_MCU_MODE_LOW_POWER;
am_hal_pwrctrl_gpu_mode_e       g_eCurGpuPwrMode    = AM_HAL_PWRCTRL_GPU_MODE_LOW_POWER;
am_hal_pwrctrl_gpu_mode_e       g_ePreGpuPwrMode    = AM_HAL_PWRCTRL_GPU_MODE_LOW_POWER;
am_hal_pwrctrl_rom_select_e     g_eCurROMPwrMode    = AM_HAL_PWRCTRL_ROM_ALWAYS_ON;

uint32_t g_ui32TrimVer                      = 0xFFFFFFFF;
uint32_t g_ui32origSimobuckVDDStrim         = 0xFFFFFFFF;
uint32_t g_ui32VDDCBoostReqCnt              = 0;
uint32_t g_ui32VDDFBoostReqLevel1Cnt        = 0;
uint32_t g_ui32VDDFBoostReqLevel2Cnt        = 0;
//! @}

//*****************************************************************************
//
//! @name Save factory trim values.
//! @{
//
// ****************************************************************************
static bool     g_bOrigTrimsStored          = false;
static uint32_t g_orig_ACTTRIMVDDF          = 0;
static uint32_t g_orig_MEMLDOACTIVETRIM     = 0;
static uint32_t g_orig_LPTRIMVDDF           = 0;
static uint32_t g_orig_MEMLPLDOTRIM         = 0;
static uint32_t g_orig_TVRGVREFTRIM         = 0;
static uint32_t g_orig_MEMLDOTRIM           = 0xFFFFFFFF;
static uint32_t g_orig_CORELDOTEMPCOTRIM    = 0xFFFFFFFF;
static uint32_t g_orig_TVRGFVREFTRIM        = 0xFFFFFFFF;
static uint32_t g_orig_D2ASPARE             = 0xFFFFFFFF;
static uint32_t g_orig_VDDCACTLOWTONTRIM    = 0xFFFFFFFF;
static uint32_t g_orig_VDDFACTLOWTONTRIM    = 0xFFFFFFFF;
static uint32_t g_orig_TVRGFTEMPCOTRIM      = 0xFFFFFFFF;


//! @}

#if AM_HAL_TEMPCO_LP
//*****************************************************************************
//
//! @name Saved handle and slot number for TempCo.
//! @{
//
// ****************************************************************************
static void    *g_TempcoADCHandle;
static uint32_t g_ui32TempcoADCslot;
static uint16_t g_ui16TempcoTEMP_code;
static bool     g_bTempcoValid           = false;
static float    g_pfTempMeasured;
//
//! @}
//
#endif // AM_HAL_TEMPCO_LP

//*****************************************************************************
//
// This table will be populated with all available INFO1 values and
// will be used for easy lookup after OTP is powered down.
//
//*****************************************************************************
am_hal_pwrctrl_info1_regs_t g_sINFO1regs;

//*****************************************************************************
//
//! @name DEVPWREN and DEVPWRSTATUS Mask Macros
//! @{
//! The below DEVPWREN and DEVPWRSTATUS masks are used to check if a peripheral
//!    has been disabled properly
//!
//! The original check of ((PWRCTRL->DEVPWRSTATUS & ui32PeriphStatus) == 0)
//!     will fail when more than one enable in the same domain is set and the
//!     user tries disable only one.
// ****************************************************************************

#define PWRCTRL_HCPB_DEVPWREN_MASK       ( \
    _VAL2FLD(PWRCTRL_DEVPWREN_PWRENIOM0, PWRCTRL_DEVPWREN_PWRENIOM0_EN) | \
    _VAL2FLD(PWRCTRL_DEVPWREN_PWRENIOM1, PWRCTRL_DEVPWREN_PWRENIOM1_EN) | \
    _VAL2FLD(PWRCTRL_DEVPWREN_PWRENIOM2, PWRCTRL_DEVPWREN_PWRENIOM2_EN) | \
    _VAL2FLD(PWRCTRL_DEVPWREN_PWRENIOM3, PWRCTRL_DEVPWREN_PWRENIOM3_EN))

#define PWRCTRL_HCPC_DEVPWREN_MASK       ( \
    _VAL2FLD(PWRCTRL_DEVPWREN_PWRENIOM4, PWRCTRL_DEVPWREN_PWRENIOM4_EN) | \
    _VAL2FLD(PWRCTRL_DEVPWREN_PWRENIOM5, PWRCTRL_DEVPWREN_PWRENIOM5_EN) | \
    _VAL2FLD(PWRCTRL_DEVPWREN_PWRENIOM6, PWRCTRL_DEVPWREN_PWRENIOM6_EN) | \
    _VAL2FLD(PWRCTRL_DEVPWREN_PWRENIOM7, PWRCTRL_DEVPWREN_PWRENIOM7_EN))

#define PWRCTRL_HCPA_DEVPWREN_MASK       ( \
    _VAL2FLD(PWRCTRL_DEVPWREN_PWRENUART0, PWRCTRL_DEVPWREN_PWRENUART0_EN) | \
    _VAL2FLD(PWRCTRL_DEVPWREN_PWRENUART1, PWRCTRL_DEVPWREN_PWRENUART1_EN) | \
    _VAL2FLD(PWRCTRL_DEVPWREN_PWRENUART2, PWRCTRL_DEVPWREN_PWRENUART2_EN) | \
    _VAL2FLD(PWRCTRL_DEVPWREN_PWRENUART3, PWRCTRL_DEVPWREN_PWRENUART3_EN))

#define PWRCTRL_MSPI_DEVPWREN_MASK       ( \
    _VAL2FLD(PWRCTRL_DEVPWREN_PWRENMSPI0, PWRCTRL_DEVPWREN_PWRENMSPI0_EN) | \
    _VAL2FLD(PWRCTRL_DEVPWREN_PWRENMSPI1, PWRCTRL_DEVPWREN_PWRENMSPI1_EN) | \
    _VAL2FLD(PWRCTRL_DEVPWREN_PWRENMSPI2, PWRCTRL_DEVPWREN_PWRENMSPI2_EN))

#define PWRCTRL_AUD_DEVPWREN_MASK       ( \
    _VAL2FLD(PWRCTRL_AUDSSPWREN_PWRENAUDREC, PWRCTRL_AUDSSPWREN_PWRENAUDREC_EN) | \
    _VAL2FLD(PWRCTRL_AUDSSPWREN_PWRENAUDPB, PWRCTRL_AUDSSPWREN_PWRENAUDPB_EN)   | \
    _VAL2FLD(PWRCTRL_AUDSSPWREN_PWRENPDM0, PWRCTRL_AUDSSPWREN_PWRENPDM0_EN)     | \
    _VAL2FLD(PWRCTRL_AUDSSPWREN_PWRENI2S0, PWRCTRL_AUDSSPWREN_PWRENI2S0_EN)     | \
    _VAL2FLD(PWRCTRL_AUDSSPWREN_PWRENI2S1, PWRCTRL_AUDSSPWREN_PWRENI2S1_EN))

#define PWRCTRL_HCPB_DEVPWRSTATUS_MASK      ( \
    PWRCTRL_DEVPWRSTATUS_PWRSTIOM0_Msk | \
    PWRCTRL_DEVPWRSTATUS_PWRSTIOM1_Msk | \
    PWRCTRL_DEVPWRSTATUS_PWRSTIOM2_Msk | \
    PWRCTRL_DEVPWRSTATUS_PWRSTIOM3_Msk)

#define PWRCTRL_HCPC_DEVPWRSTATUS_MASK      ( \
    PWRCTRL_DEVPWRSTATUS_PWRSTIOM4_Msk | \
    PWRCTRL_DEVPWRSTATUS_PWRSTIOM5_Msk | \
    PWRCTRL_DEVPWRSTATUS_PWRSTIOM6_Msk | \
    PWRCTRL_DEVPWRSTATUS_PWRSTIOM7_Msk)

#define PWRCTRL_HCPA_DEVPWRSTATUS_MASK          ( \
    PWRCTRL_DEVPWRSTATUS_PWRSTUART0_Msk | \
    PWRCTRL_DEVPWRSTATUS_PWRSTUART1_Msk | \
    PWRCTRL_DEVPWRSTATUS_PWRSTUART2_Msk | \
    PWRCTRL_DEVPWRSTATUS_PWRSTUART3_Msk)

#define PWRCTRL_MSPI_DEVPWRSTATUS_MASK          ( \
    PWRCTRL_DEVPWRSTATUS_PWRSTMSPI0_Msk | \
    PWRCTRL_DEVPWRSTATUS_PWRSTMSPI1_Msk | \
    PWRCTRL_DEVPWRSTATUS_PWRSTMSPI2_Msk | \
    PWRCTRL_DEVPWRSTATUS_PWRSTMSPI3_Msk)

#define PWRCTRL_AUD_DEVPWRSTATUS_MASK      ( \
    PWRCTRL_AUDSSPWRSTATUS_PWRSTAUDREC_Msk | \
    PWRCTRL_AUDSSPWRSTATUS_PWRSTAUDPB_Msk  | \
    PWRCTRL_AUDSSPWRSTATUS_PWRSTPDM0_Msk   | \
    PWRCTRL_AUDSSPWRSTATUS_PWRSTI2S0_Msk   | \
    PWRCTRL_AUDSSPWRSTATUS_PWRSTI2S1_Msk)
//! @}

// **********************************************
//! Define the peripheral control structure.
// **********************************************
struct am_pwr_s
{
    uint32_t    ui32PwrEnRegAddr;
    uint32_t    ui32PeriphEnable;
    uint32_t    ui32PwrStatReqAddr;
    uint32_t    ui32PeriphStatus;
};

//
//! Peripheral control data structure
//
#ifndef AM_HAL_PWRCTRL_RAM_TABLE
const struct am_pwr_s am_hal_pwrctrl_peripheral_control[AM_HAL_PWRCTRL_PERIPH_MAX] =
{
    {
        AM_REGADDR(PWRCTRL, DEVPWREN),
        _VAL2FLD(PWRCTRL_DEVPWREN_PWRENIOS, PWRCTRL_DEVPWREN_PWRENIOS_EN),
        AM_REGADDR(PWRCTRL, DEVPWRSTATUS),
        PWRCTRL_DEVPWRSTATUS_PWRSTIOS_Msk
    },
    {
        AM_REGADDR(PWRCTRL, DEVPWREN),
        _VAL2FLD(PWRCTRL_DEVPWREN_PWRENIOM0, PWRCTRL_DEVPWREN_PWRENIOM0_EN),
        AM_REGADDR(PWRCTRL, DEVPWRSTATUS),
        PWRCTRL_HCPB_DEVPWRSTATUS_MASK
    },
    {
        AM_REGADDR(PWRCTRL, DEVPWREN),
        _VAL2FLD(PWRCTRL_DEVPWREN_PWRENIOM1, PWRCTRL_DEVPWREN_PWRENIOM1_EN),
        AM_REGADDR(PWRCTRL, DEVPWRSTATUS),
        PWRCTRL_HCPB_DEVPWRSTATUS_MASK
    },
    {
        AM_REGADDR(PWRCTRL, DEVPWREN),
        _VAL2FLD(PWRCTRL_DEVPWREN_PWRENIOM2, PWRCTRL_DEVPWREN_PWRENIOM2_EN),
        AM_REGADDR(PWRCTRL, DEVPWRSTATUS),
        PWRCTRL_HCPB_DEVPWRSTATUS_MASK
    },
    {
        AM_REGADDR(PWRCTRL, DEVPWREN),
        _VAL2FLD(PWRCTRL_DEVPWREN_PWRENIOM3, PWRCTRL_DEVPWREN_PWRENIOM3_EN),
        AM_REGADDR(PWRCTRL, DEVPWRSTATUS),
        PWRCTRL_HCPB_DEVPWRSTATUS_MASK
    },
    {
        AM_REGADDR(PWRCTRL, DEVPWREN),
        _VAL2FLD(PWRCTRL_DEVPWREN_PWRENIOM4, PWRCTRL_DEVPWREN_PWRENIOM4_EN),
        AM_REGADDR(PWRCTRL, DEVPWRSTATUS),
        PWRCTRL_HCPC_DEVPWRSTATUS_MASK
    },
    {
        AM_REGADDR(PWRCTRL, DEVPWREN),
        _VAL2FLD(PWRCTRL_DEVPWREN_PWRENIOM5, PWRCTRL_DEVPWREN_PWRENIOM5_EN),
        AM_REGADDR(PWRCTRL, DEVPWRSTATUS),
        PWRCTRL_HCPC_DEVPWRSTATUS_MASK
    },
    {
        AM_REGADDR(PWRCTRL, DEVPWREN),
        _VAL2FLD(PWRCTRL_DEVPWREN_PWRENIOM6, PWRCTRL_DEVPWREN_PWRENIOM6_EN),
        AM_REGADDR(PWRCTRL, DEVPWRSTATUS),
        PWRCTRL_HCPC_DEVPWRSTATUS_MASK
    },
    {
        AM_REGADDR(PWRCTRL, DEVPWREN),
        _VAL2FLD(PWRCTRL_DEVPWREN_PWRENIOM7, PWRCTRL_DEVPWREN_PWRENIOM7_EN),
        AM_REGADDR(PWRCTRL, DEVPWRSTATUS),
        PWRCTRL_HCPC_DEVPWRSTATUS_MASK
    },
    {
        AM_REGADDR(PWRCTRL, DEVPWREN),
        _VAL2FLD(PWRCTRL_DEVPWREN_PWRENUART0, PWRCTRL_DEVPWREN_PWRENUART0_EN),
        AM_REGADDR(PWRCTRL, DEVPWRSTATUS),
        PWRCTRL_HCPA_DEVPWRSTATUS_MASK
    },
    {
        AM_REGADDR(PWRCTRL, DEVPWREN),
        _VAL2FLD(PWRCTRL_DEVPWREN_PWRENUART1, PWRCTRL_DEVPWREN_PWRENUART1_EN),
        AM_REGADDR(PWRCTRL, DEVPWRSTATUS),
        PWRCTRL_HCPA_DEVPWRSTATUS_MASK
    },
    {
        AM_REGADDR(PWRCTRL, DEVPWREN),
        _VAL2FLD(PWRCTRL_DEVPWREN_PWRENUART2, PWRCTRL_DEVPWREN_PWRENUART2_EN),
        AM_REGADDR(PWRCTRL, DEVPWRSTATUS),
        PWRCTRL_HCPA_DEVPWRSTATUS_MASK
    },
    {
        AM_REGADDR(PWRCTRL, DEVPWREN),
        _VAL2FLD(PWRCTRL_DEVPWREN_PWRENUART3, PWRCTRL_DEVPWREN_PWRENUART3_EN),
        AM_REGADDR(PWRCTRL, DEVPWRSTATUS),
        PWRCTRL_HCPA_DEVPWRSTATUS_MASK
    },
    {
        AM_REGADDR(PWRCTRL, DEVPWREN),
        _VAL2FLD(PWRCTRL_DEVPWREN_PWRENADC, PWRCTRL_DEVPWREN_PWRENADC_EN),
        AM_REGADDR(PWRCTRL, DEVPWRSTATUS),
        PWRCTRL_DEVPWRSTATUS_PWRSTADC_Msk
    },
    {
        AM_REGADDR(PWRCTRL, DEVPWREN),
        _VAL2FLD(PWRCTRL_DEVPWREN_PWRENMSPI0, PWRCTRL_DEVPWREN_PWRENMSPI0_EN),
        AM_REGADDR(PWRCTRL, DEVPWRSTATUS),
        PWRCTRL_MSPI_DEVPWRSTATUS_MASK
    },
    {
        AM_REGADDR(PWRCTRL, DEVPWREN),
        _VAL2FLD(PWRCTRL_DEVPWREN_PWRENMSPI1, PWRCTRL_DEVPWREN_PWRENMSPI1_EN),
        AM_REGADDR(PWRCTRL, DEVPWRSTATUS),
        PWRCTRL_MSPI_DEVPWRSTATUS_MASK
    },
    {
        AM_REGADDR(PWRCTRL, DEVPWREN),
        _VAL2FLD(PWRCTRL_DEVPWREN_PWRENMSPI2, PWRCTRL_DEVPWREN_PWRENMSPI2_EN),
        AM_REGADDR(PWRCTRL, DEVPWRSTATUS),
        PWRCTRL_MSPI_DEVPWRSTATUS_MASK
    },
    {
        AM_REGADDR(PWRCTRL, DEVPWREN),
        _VAL2FLD(PWRCTRL_DEVPWREN_PWRENMSPI3, PWRCTRL_DEVPWREN_PWRENMSPI3_EN),
        AM_REGADDR(PWRCTRL, DEVPWRSTATUS),
        PWRCTRL_MSPI_DEVPWRSTATUS_MASK
    },
    {
        AM_REGADDR(PWRCTRL, DEVPWREN),
        _VAL2FLD(PWRCTRL_DEVPWREN_PWRENGFX, PWRCTRL_DEVPWREN_PWRENGFX_EN),
        AM_REGADDR(PWRCTRL, DEVPWRSTATUS),
        PWRCTRL_DEVPWRSTATUS_PWRSTGFX_Msk
    },
    {
        AM_REGADDR(PWRCTRL, DEVPWREN),
        _VAL2FLD(PWRCTRL_DEVPWREN_PWRENDISP, PWRCTRL_DEVPWREN_PWRENDISP_EN),
        AM_REGADDR(PWRCTRL, DEVPWRSTATUS),
        PWRCTRL_DEVPWRSTATUS_PWRSTDISP_Msk
    },
    {
        AM_REGADDR(PWRCTRL, DEVPWREN),
        _VAL2FLD(PWRCTRL_DEVPWREN_PWRENDISPPHY, PWRCTRL_DEVPWREN_PWRENDISPPHY_EN),
        AM_REGADDR(PWRCTRL, DEVPWRSTATUS),
        PWRCTRL_DEVPWRSTATUS_PWRSTDISPPHY_Msk
    },
    {
        AM_REGADDR(PWRCTRL, DEVPWREN),
        _VAL2FLD(PWRCTRL_DEVPWREN_PWRENCRYPTO, PWRCTRL_DEVPWREN_PWRENCRYPTO_EN),
        AM_REGADDR(PWRCTRL, DEVPWRSTATUS),
        PWRCTRL_DEVPWRSTATUS_PWRSTCRYPTO_Msk
    },
    {
        AM_REGADDR(PWRCTRL, DEVPWREN),
        _VAL2FLD(PWRCTRL_DEVPWREN_PWRENSDIO0, PWRCTRL_DEVPWREN_PWRENSDIO0_EN),
        AM_REGADDR(PWRCTRL, DEVPWRSTATUS),
        PWRCTRL_DEVPWRSTATUS_PWRSTSDIO0_Msk
    },
    {
        AM_REGADDR(PWRCTRL, DEVPWREN),
        _VAL2FLD(PWRCTRL_DEVPWREN_PWRENSDIO1, PWRCTRL_DEVPWREN_PWRENSDIO1_EN),
        AM_REGADDR(PWRCTRL, DEVPWRSTATUS),
        PWRCTRL_DEVPWRSTATUS_PWRSTSDIO1_Msk
    },
    {
        AM_REGADDR(PWRCTRL, DEVPWREN),
        _VAL2FLD(PWRCTRL_DEVPWREN_PWRENUSB, PWRCTRL_DEVPWREN_PWRENUSB_EN),
        AM_REGADDR(PWRCTRL, DEVPWRSTATUS),
        PWRCTRL_DEVPWRSTATUS_PWRSTUSB_Msk
    },
    {
        AM_REGADDR(PWRCTRL, DEVPWREN),
        _VAL2FLD(PWRCTRL_DEVPWREN_PWRENUSBPHY, PWRCTRL_DEVPWREN_PWRENUSBPHY_EN),
        AM_REGADDR(PWRCTRL, DEVPWRSTATUS),
        PWRCTRL_DEVPWRSTATUS_PWRSTUSBPHY_Msk
    },
    {
        AM_REGADDR(PWRCTRL, DEVPWREN),
        _VAL2FLD(PWRCTRL_DEVPWREN_PWRENDBG, PWRCTRL_DEVPWREN_PWRENDBG_EN),
        AM_REGADDR(PWRCTRL, DEVPWRSTATUS),
        PWRCTRL_DEVPWRSTATUS_PWRSTDBG_Msk
    },
    {
        AM_REGADDR(PWRCTRL, DEVPWREN),
        _VAL2FLD(PWRCTRL_DEVPWREN_PWRENOTP, PWRCTRL_DEVPWREN_PWRENOTP_EN),
        AM_REGADDR(PWRCTRL, MEMPWRSTATUS),
        PWRCTRL_MEMPWRSTATUS_PWRSTOTP_Msk
    },
    {
        AM_REGADDR(PWRCTRL, AUDSSPWREN),
        _VAL2FLD(PWRCTRL_AUDSSPWREN_PWRENAUDREC, PWRCTRL_AUDSSPWREN_PWRENAUDREC_EN),
        AM_REGADDR(PWRCTRL, AUDSSPWRSTATUS),
        PWRCTRL_AUD_DEVPWRSTATUS_MASK
    },
    {
        AM_REGADDR(PWRCTRL, AUDSSPWREN),
        _VAL2FLD(PWRCTRL_AUDSSPWREN_PWRENAUDPB, PWRCTRL_AUDSSPWREN_PWRENAUDPB_EN),
        AM_REGADDR(PWRCTRL, AUDSSPWRSTATUS),
        PWRCTRL_AUD_DEVPWRSTATUS_MASK
    },
    {
        AM_REGADDR(PWRCTRL, AUDSSPWREN),
        _VAL2FLD(PWRCTRL_AUDSSPWREN_PWRENPDM0, PWRCTRL_AUDSSPWREN_PWRENPDM0_EN),
        AM_REGADDR(PWRCTRL, AUDSSPWRSTATUS),
        PWRCTRL_AUD_DEVPWRSTATUS_MASK
    },
    {
        AM_REGADDR(PWRCTRL, AUDSSPWREN),
        _VAL2FLD(PWRCTRL_AUDSSPWREN_PWRENI2S0, PWRCTRL_AUDSSPWREN_PWRENI2S0_EN),
        AM_REGADDR(PWRCTRL, AUDSSPWRSTATUS),
        PWRCTRL_AUD_DEVPWRSTATUS_MASK
    },
    {
        AM_REGADDR(PWRCTRL, AUDSSPWREN),
        _VAL2FLD(PWRCTRL_AUDSSPWREN_PWRENI2S1, PWRCTRL_AUDSSPWREN_PWRENI2S1_EN),
        AM_REGADDR(PWRCTRL, AUDSSPWRSTATUS),
        PWRCTRL_AUD_DEVPWRSTATUS_MASK
    },
    {
        AM_REGADDR(PWRCTRL, AUDSSPWREN),
        _VAL2FLD(PWRCTRL_AUDSSPWREN_PWRENAUDADC, PWRCTRL_AUDSSPWREN_PWRENAUDADC_EN),
        AM_REGADDR(PWRCTRL, AUDSSPWRSTATUS),
        PWRCTRL_AUDSSPWRSTATUS_PWRSTAUDADC_Msk
    },
};

//*****************************************************************************
//
// Mask off all interrupts
//
//*****************************************************************************
static inline uint32_t
am_critical_mask_all_begin(void)
{
    uint32_t basePrioGrouping = ((uint32_t)((SCB->AIRCR & SCB_AIRCR_PRIGROUP_Msk) >> SCB_AIRCR_PRIGROUP_Pos));

    orgPriMask = am_hal_interrupt_master_disable();
    origBasePri = __get_BASEPRI();
    __set_BASEPRI((basePrioGrouping >= (8 - __NVIC_PRIO_BITS))?(1 << (basePrioGrouping + 1)):(1 << (8 - __NVIC_PRIO_BITS)));
    for (uint32_t i = 0; i < 16; i++)
    {
        nvic_en[i] = NVIC->ISER[i];
        NVIC->ICER[i] = nvic_en[i];
    }
    //
    // Disable SysTick Int if it was enabled.
    //
    if (SysTick->CTRL & SysTick_CTRL_TICKINT_Msk)
    {
        g_bSTIntEnabled = true;
        SysTick->CTRL &= ~SysTick_CTRL_TICKINT_Msk;
    }

    return AM_HAL_STATUS_SUCCESS;
} // am_critical_mask_all_begin()

//*****************************************************************************
//
//  Restore the interrupts
//
//*****************************************************************************
static inline void
am_critical_mask_all_end(void)
{
    uint32_t ui32STVal = 0;
    bool bSetSTPend = false;

    for (uint32_t i = 0; i < 16; i++ )
    {
        NVIC->ISER[i] = nvic_en[i];
    }
    __set_BASEPRI(origBasePri);

    if (g_bSTIntEnabled)
    {
        //
        // Address potential race condition where Systick interrupt could have
        // been fired - while we disabled it
        //
        // Current snapshot for Systick
        //
        ui32STVal = SysTick->VAL;
        //
        // Need to read COUNTFLAG before writing to CTRL, as it would get reset
        //
        bSetSTPend = SysTick->CTRL & SysTick_CTRL_COUNTFLAG_Msk;
        //
        // Enable SysTick Int
        //
        SysTick->CTRL |= SysTick_CTRL_TICKINT_Msk;
        //
        // Read Systick again to see if it wrapped
        //
        if (SysTick->VAL > ui32STVal)
        {
              bSetSTPend = true;
        }
        //
        // Set PENDSTSET
        //
        if (bSetSTPend)
        {
            SCB->ICSR |= SCB_ICSR_PENDSTSET_Msk;
        }
    }

    am_hal_interrupt_master_set(orgPriMask);
} // am_critical_mask_all_end()

//*****************************************************************************
//
//! @brief  Return the pwr_ctrl entry for a given ePeripheral
//!
//! @param  pwr_ctrl address where the power entry is copied
//! @param  ePeripheral the peripheral to copy
//!
//! @return Returns AM_HAL_STATUS_SUCCESS on success
//
//*****************************************************************************
static inline uint32_t
am_get_pwrctrl(struct am_pwr_s *pwr_ctrl, uint32_t ePeripheral)
{
    if ( pwr_ctrl == NULL || ePeripheral >= AM_HAL_PWRCTRL_PERIPH_MAX )
    {
        return AM_HAL_STATUS_INVALID_ARG;
    }

    *pwr_ctrl = am_hal_pwrctrl_peripheral_control[ePeripheral];

    return AM_HAL_STATUS_SUCCESS;
} // am_get_pwrctrl()
#else
//*****************************************************************************
//
//! @brief  Return the pwr_ctrl entry for a given ePeripheral
//!
//! @param  pwr_ctrl address where the power entry is generated
//! @param  ePeripheral the peripheral for which to generate:
//!
//! @return Returns AM_HAL_STATUS_SUCCESS on success
//
//*****************************************************************************
static uint32_t
am_get_pwrctrl(struct am_pwr_s *pwr_ctrl, uint32_t ePeripheral)
{
    int shift_pos;

    if (pwr_ctrl == NULL || ePeripheral >= AM_HAL_PWRCTRL_PERIPH_MAX)
    {
        return AM_HAL_STATUS_INVALID_ARG;
    }

    if (ePeripheral < AM_HAL_PWRCTRL_PERIPH_AUDREC)
    {
        pwr_ctrl->ui32PwrEnRegAddr = AM_REGADDR(PWRCTRL, DEVPWREN);
        pwr_ctrl->ui32PwrStatReqAddr = AM_REGADDR(PWRCTRL, DEVPWRSTATUS);
        pwr_ctrl->ui32PeriphEnable = 1 << ePeripheral;
        pwr_ctrl->ui32PeriphStatus = 1 << ePeripheral;
    }
    else
    {
        shift_pos = (ePeripheral - AM_HAL_PWRCTRL_PERIPH_AUDREC);
        if (ePeripheral > AM_HAL_PWRCTRL_PERIPH_I2S1)
        {
            shift_pos += 2;
        }

        pwr_ctrl->ui32PwrEnRegAddr =  AM_REGADDR(PWRCTRL, AUDSSPWREN);
        pwr_ctrl->ui32PwrStatReqAddr = AM_REGADDR(PWRCTRL, AUDSSPWRSTATUS);
        pwr_ctrl->ui32PeriphEnable = 1 << shift_pos;
        pwr_ctrl->ui32PeriphStatus = 1 << shift_pos;
    }

    return AM_HAL_STATUS_SUCCESS;
} // am_get_pwrctrl()
#endif // AM_HAL_PWRCTRL_RAM_TABLE

//*****************************************************************************
//
// Default configurations definitions
//
//*****************************************************************************
// #### INTERNAL BEGIN ####
#warning "TODO - FIXME: What are the proper default memory configurations?"
// #### INTERNAL END ####
const am_hal_pwrctrl_mcu_memory_config_t    g_DefaultMcuMemCfg =
{
    .eROMMode           = AM_HAL_PWRCTRL_ROM_AUTO,
    .bEnableCache       = true,
    .bRetainCache       = true,
    .eDTCMCfg           = AM_HAL_PWRCTRL_ITCM256K_DTCM512K,
    .eRetainDTCM        = AM_HAL_PWRCTRL_ITCM256K_DTCM512K,
    .bEnableNVM         = true,
};

const am_hal_pwrctrl_sram_memcfg_t          g_DefaultSRAMCfg =
{
    .eSRAMCfg           = AM_HAL_PWRCTRL_SRAM_3M,
    .eActiveWithMCU     = AM_HAL_PWRCTRL_SRAM_NONE,
    .eActiveWithGFX     = AM_HAL_PWRCTRL_SRAM_NONE,
    .eActiveWithDISP    = AM_HAL_PWRCTRL_SRAM_NONE,
    .eSRAMRetain        = AM_HAL_PWRCTRL_SRAM_3M
};

//*****************************************************************************
//
// Function to determine the chip's TRIM version.
//
// return Status code.
//
// pui32TrimVer: The uint32_t that will receive the trim version number.
//               If no valid trim version found, *pui32TrimVer returns as 0.
//
//
//*****************************************************************************
static uint32_t
TrimVersionGet(uint32_t *pui32TrimVer)
{
    uint32_t ui32Ret;

    //
    // Get the TRIM version and set the global variable.
    // This only needs to be done and verified once.
    //
    if ( g_ui32TrimVer == 0xFFFFFFFF )
    {
        ui32Ret = am_hal_info1_read(AM_HAL_INFO_INFOSPACE_CURRENT_INFO1,
                                    AM_REG_OTP_INFO1_TRIM_REV_O / 4,
                                    1, &g_ui32TrimVer);

        if ( (ui32Ret != 0) || (g_ui32TrimVer == 0xFFFFFFFF) )
        {
            //
            // Invalid trim value. Set the global to indicate version 0.
            //
            g_ui32TrimVer = 0;
        }
    }

    if ( pui32TrimVer )
    {
        *pui32TrimVer = g_ui32TrimVer;
        return AM_HAL_STATUS_SUCCESS;
    }
    else
    {
        return AM_HAL_STATUS_INVALID_ARG;
    }

} // TrimVersionGet()

// ****************************************************************************
//
//  am_hal_pwrctrl_mcu_mode_status()
//
// ****************************************************************************
uint32_t
am_hal_pwrctrl_mcu_mode_status(am_hal_pwrctrl_mcu_mode_e *peCurrentPowerMode)
{
#ifndef AM_HAL_DISABLE_API_VALIDATION
    if ( peCurrentPowerMode == 0 )
    {
        return AM_HAL_STATUS_INVALID_ARG;
    }
#endif // AM_HAL_DISABLE_API_VALIDATION

    *peCurrentPowerMode = g_eCurMcuPwrMode;
    return AM_HAL_STATUS_SUCCESS;

} // am_hal_pwrctrl_mcu_mode_status()

// ****************************************************************************
//
//  Adjust or restore VDDC simobuck.
//  vddc_simobuck_adjust(bool bAdjust)
//  bAdjust  - true: boost or reduce VDDC simobuck
//           - false: restore VDDC simobuck
//
// ****************************************************************************
static void
vddc_simobuck_adjust(bool bAdjust)
{
    if (g_orig_D2ASPARE != 0xFFFFFFFF) // check if original trim was saved
    {
        if (bAdjust) // adjust trim
        {
            //
            // Boost simobuck VDDC by setting bit 8 in the D2ASPARE register.
            //
            MCUCTRL->D2ASPARE |= (1UL << 8);
        }
        else // restore
        {
            MCUCTRL->D2ASPARE &= ~(1UL << 8);
        }
    }
} // vddc_simobuck_adjust()

// ****************************************************************************
//
//  Adjust or restore VDDC ldo.
//  vddc_ldo_adjust(bool bAdjust, int32_t i32Delta)
//  bAdjust  - true: boost or reduce VDDC ldo
//           - false: restore VDDC ldo
//  ui32CoreLdoTempCoTrim - Absolute trim value, if bAdjust is true.
//
// ****************************************************************************
static void
vddc_ldo_adjust(bool bAdjust, uint32_t ui32CoreLdoTempCoTrim)
{
    if (g_orig_CORELDOTEMPCOTRIM != 0xFFFFFFFF) // check if original trim was saved
    {
        if (bAdjust) //adjust trim
        {
            if (ui32CoreLdoTempCoTrim > (MCUCTRL_LDOREG1_CORELDOTEMPCOTRIM_Msk >> MCUCTRL_LDOREG1_CORELDOTEMPCOTRIM_Pos))
            {
                ui32CoreLdoTempCoTrim = (MCUCTRL_LDOREG1_CORELDOTEMPCOTRIM_Msk >> MCUCTRL_LDOREG1_CORELDOTEMPCOTRIM_Pos); // ceiling
            }
            MCUCTRL->LDOREG1_b.CORELDOTEMPCOTRIM = ui32CoreLdoTempCoTrim;
        }
        else // restore
        {
            MCUCTRL->LDOREG1_b.CORELDOTEMPCOTRIM = g_orig_CORELDOTEMPCOTRIM;
        }
    }
} // vddc_ldo_adjust()

// ****************************************************************************
//
//  Adjust or restore VDDF ldo.
//  vddf_ldo_adjust(bool bAdjust, int32_t i32Delta)
//  bAdjust  - true: boost or reduce VDDF ldo
//           - false: restore VDDF ldo
//  i32Delta - Delta for register adjustment, if bAdjust is true.
//
// ****************************************************************************
static void
vddf_ldo_adjust(bool bAdjust, int32_t i32Delta)
{
    uint32_t ui32MemLdoTrim = 0;
    if (g_orig_MEMLDOTRIM != 0xFFFFFFFF) // check if original trim was saved
    {
        if (bAdjust) //adjust trim
        {
            if (i32Delta < 0)
            {
                if (g_orig_MEMLDOTRIM < -i32Delta)
                {
                    ui32MemLdoTrim = 0; // floor
                }
                else
                {
                    ui32MemLdoTrim = g_orig_MEMLDOTRIM + i32Delta;
                }
            }
            else
            {
                ui32MemLdoTrim = g_orig_MEMLDOTRIM + i32Delta;
                if (ui32MemLdoTrim > (MCUCTRL_LDOREG2_MEMLDOACTIVETRIM_Msk >> MCUCTRL_LDOREG2_MEMLDOACTIVETRIM_Pos))
                {
                    ui32MemLdoTrim = (MCUCTRL_LDOREG2_MEMLDOACTIVETRIM_Msk >> MCUCTRL_LDOREG2_MEMLDOACTIVETRIM_Pos); // ceiling
                }
            }
            MCUCTRL->LDOREG2_b.MEMLDOACTIVETRIM = ui32MemLdoTrim;
        }
        else // restore
        {
            MCUCTRL->LDOREG2_b.MEMLDOACTIVETRIM = g_orig_MEMLDOTRIM;
        }
    }
} // vddf_ldo_adjust()

// ****************************************************************************
//
//  Get the original TVRGFVREFTRIM.
//  vddf_simobuck_trim_get(uint32_t * ui32TVRGFVREFTRIM)
// #### INTERNAL BEGIN ####
// May move this to SBL later.
// #### INTERNAL END ####
//
// ****************************************************************************
static void
vddf_simobuck_trim_get(uint32_t * pui32TVRGFVREFTRIM)
{
    AM_CRITICAL_BEGIN

    TRIM_WR_ENABLE

    *pui32TVRGFVREFTRIM = ( (AM_REGVAL(0x4004004C) & MCUCTRL_VREFGEN4_TVRGFVREFTRIM_Msk) >> MCUCTRL_VREFGEN4_TVRGFVREFTRIM_Pos );

    TRIM_WR_DISABLE

    AM_CRITICAL_END
} // vddf_simobuck_trim_get()

// ****************************************************************************
//
//  Set the register TVRGFVREFTRIM.
//  vddf_simobuck_trim_set(uint32_t ui32TVRGFVREFTRIM)
// #### INTERNAL BEGIN ####
// May move this to SBL later.
// #### INTERNAL END ####
//
// ****************************************************************************
static void
vddf_simobuck_trim_set(uint32_t ui32TVRGFVREFTRIM)
{
    uint32_t ui32VREFGEN4 = 0;
    AM_CRITICAL_BEGIN

    TRIM_WR_ENABLE

    ui32VREFGEN4 = AM_REGVAL(0x4004004C);
    ui32VREFGEN4 &= ~MCUCTRL_VREFGEN4_TVRGFVREFTRIM_Msk;
    ui32VREFGEN4 |= (ui32TVRGFVREFTRIM << MCUCTRL_VREFGEN4_TVRGFVREFTRIM_Pos);
    AM_REGVAL(0x4004004C) = ui32VREFGEN4;

    TRIM_WR_DISABLE

    AM_CRITICAL_END
} // vddf_simobuck_trim_set()

static void
vddf_simobuck_trim_get_tvrgf_tempco(uint32_t * ui32TVRGFTEMPCOTRIM)
{
    AM_CRITICAL_BEGIN

    TRIM_WR_ENABLE

    *ui32TVRGFTEMPCOTRIM = ( (AM_REGVAL(0x4004004C) & MCUCTRL_VREFGEN4_TVRGFTEMPCOTRIM_Msk) >> MCUCTRL_VREFGEN4_TVRGFTEMPCOTRIM_Pos );

    TRIM_WR_DISABLE

    AM_CRITICAL_END
} // vddf_simobuck_trim_get_tvrgf_tempco()

static void
vddf_simobuck_trim_set_tvrgf_tempco(uint32_t ui32TVRGFTEMPCOTRIM)
{
    uint32_t ui32VREFGEN4 = 0;
    AM_CRITICAL_BEGIN

    TRIM_WR_ENABLE

    ui32VREFGEN4 = AM_REGVAL(0x4004004C);
    ui32VREFGEN4 &= ~MCUCTRL_VREFGEN4_TVRGFTEMPCOTRIM_Msk;
    ui32VREFGEN4 |= (ui32TVRGFTEMPCOTRIM << MCUCTRL_VREFGEN4_TVRGFTEMPCOTRIM_Pos);
    AM_REGVAL(0x4004004C) = ui32VREFGEN4;

    TRIM_WR_DISABLE

    AM_CRITICAL_END
} // vddf_simobuck_trim_set_tvrgf_tempco()

// ****************************************************************************
//
//  Adjust or restore VDDF simobuck.
//  vddf_simobuck_adjust(bool bAdjust, int32_t i32Delta)
//  bAdjust  - true: boost or reduce VDDF simobuck
//           - false: restore VDDF simobuck
//  i32Delta - Delta for register adjustment, if bAdjust is true.
//
// ****************************************************************************
static void
vddf_simobuck_adjust(bool bAdjust, int32_t i32Delta)
{
    uint32_t ui32VddfBuckTrim = 0;
    if (g_orig_TVRGFVREFTRIM != 0xFFFFFFFF) // check if original trim was saved
    {
        if (bAdjust) //adjust trim
        {
            if (i32Delta < 0)
            {
                if (g_orig_TVRGFVREFTRIM < -i32Delta)
                {
                    ui32VddfBuckTrim = 0; // floor
                }
                else
                {
                    ui32VddfBuckTrim = g_orig_TVRGFVREFTRIM + i32Delta;
                }
            }
            else
            {
                ui32VddfBuckTrim = g_orig_TVRGFVREFTRIM + i32Delta;
                if (ui32VddfBuckTrim > (MCUCTRL_VREFGEN4_TVRGFVREFTRIM_Msk >> MCUCTRL_VREFGEN4_TVRGFVREFTRIM_Pos))
                {
                    ui32VddfBuckTrim = (MCUCTRL_VREFGEN4_TVRGFVREFTRIM_Msk >> MCUCTRL_VREFGEN4_TVRGFVREFTRIM_Pos); // ceiling
                }
            }
            vddf_simobuck_trim_set(ui32VddfBuckTrim);
        }
        else // restore
        {
            vddf_simobuck_trim_set(g_orig_TVRGFVREFTRIM);
        }
    }
} // vddf_simobuck_adjust()

//*****************************************************************************
//
// Initialize the timer for waking-up the core.
//
//*****************************************************************************
static inline void
core_wakeup_timer_init(uint32_t ui32Delayus)
{
    //
    // Disable the timer.
    //
    TIMERn(CORE_WAKEUP_TIMER_NUM)->CTRL0_b.TMR0EN = 0;

    //
    // Apply the settings.
    //
    TIMERn(CORE_WAKEUP_TIMER_NUM)->CTRL0 = _VAL2FLD(TIMER_CTRL0_TMR0CLK,     AM_HAL_TIMER_CLOCK_HFRC_DIV16)  |
                                           _VAL2FLD(TIMER_CTRL0_TMR0FN,      AM_HAL_TIMER_FN_EDGE)           |
                                           _VAL2FLD(TIMER_CTRL0_TMR0POL1,    false)                          |
                                           _VAL2FLD(TIMER_CTRL0_TMR0POL0,    false)                          |
                                           _VAL2FLD(TIMER_CTRL0_TMR0TMODE,   AM_HAL_TIMER_TRIGGER_DIS)       |
                                           _VAL2FLD(TIMER_CTRL0_TMR0LMT,     0)                              |
                                           _VAL2FLD(TIMER_CTRL0_TMR0EN, 0);
    TIMERn(CORE_WAKEUP_TIMER_NUM)->MODE0 = _VAL2FLD(TIMER_MODE0_TMR0TRIGSEL, AM_HAL_TIMER_TRIGGER_TMR0_OUT1);
    TIMERn(CORE_WAKEUP_TIMER_NUM)->TMR0CMP0 = ui32Delayus * (192 / 16) / 2;
    TIMERn(CORE_WAKEUP_TIMER_NUM)->TMR0CMP1 = 0xFFFFFFFF;

    //
    // Clear the timer to make sure it has the appropriate starting value.
    //
    TIMERn(CORE_WAKEUP_TIMER_NUM)->CTRL0_b.TMR0CLR = 1;

    //am_hal_timer_clear(CORE_WAKEUP_TIMER_NUM);
    //
    // Disable the timer.
    //
    TIMERn(CORE_WAKEUP_TIMER_NUM)->CTRL0_b.TMR0EN = 0;

    //
    // Clear the timer.
    //
    TIMERn(CORE_WAKEUP_TIMER_NUM)->CTRL0_b.TMR0CLR = 1;
    TIMERn(CORE_WAKEUP_TIMER_NUM)->CTRL0_b.TMR0CLR = 0;

    //
    // Clear the timer Interrupt
    //
    TIMER->INTCLR = AM_HAL_TIMER_MASK(CORE_WAKEUP_TIMER_NUM, AM_HAL_TIMER_COMPARE0);
    //
    // Enable the timer Interrupt.
    //
    TIMER->INTEN |= AM_HAL_TIMER_MASK(CORE_WAKEUP_TIMER_NUM, AM_HAL_TIMER_COMPARE0);
    //
    // Enable the timer interrupt in the NVIC.
    //
    // This interrupt needs to be set as the highest priority (0)
    //
    NVIC->IPR[TIMER0_IRQn + CORE_WAKEUP_TIMER_NUM ] = (uint8_t)((0 << (8U - __NVIC_PRIO_BITS)) & (uint32_t)0xFFUL);

    //
    // Toggle the clear bit (required by the hardware), and then enable the timer.
    //
    TIMER->GLOBEN |= 1 <<  CORE_WAKEUP_TIMER_NUM;
    TIMERn(CORE_WAKEUP_TIMER_NUM)->CTRL0_b.TMR0CLR = 1;
    TIMERn(CORE_WAKEUP_TIMER_NUM)->CTRL0_b.TMR0CLR = 0;

    //
    // Set the timer interrupt to wakeup the core
    //
    NVIC->ISER[(TIMER0_IRQn + CORE_WAKEUP_TIMER_NUM) / 32] = (1 << ((TIMER0_IRQn + CORE_WAKEUP_TIMER_NUM) % 32));
} // core_wakeup_timer_init()

//*****************************************************************************
//
//! Shutdown core wakeup timer
//
//*****************************************************************************
static inline void
core_wakeup_timer_deinit(void)
{
    register uint32_t ui32IntStat;

    //
    //
    // Disable the timer
    TIMERn(CORE_WAKEUP_TIMER_NUM)->CTRL0_b.TMR0EN = 0;
    //
    // Clear the timer Interrupt
    //
    TIMER->INTCLR = AM_HAL_TIMER_MASK(CORE_WAKEUP_TIMER_NUM, AM_HAL_TIMER_COMPARE0);
    //
    // Before clearing the NVIC pending, avoid a race condition by
    // making sure the interrupt clear has propogated by reading
    // the INTSTAT register.
    //
    ui32IntStat = TIMER->INTSTAT ;
    ui32IntStat = ui32IntStat & TIMER->INTEN;
    //
    // Clear pending NVIC interrupt for the timer-specific IRQ.
    //
    NVIC->ICPR[(((uint32_t)TIMER0_IRQn + CORE_WAKEUP_TIMER_NUM) >> 5UL)] = (uint32_t)(1UL << (((uint32_t)TIMER0_IRQn + CORE_WAKEUP_TIMER_NUM) & 0x1FUL));
    //
    // There is also a pending on the timer common IRQ. But it should
    // only be cleared if the workaround timer is the only interrupt.
    //
    if ( !(ui32IntStat &
                ~AM_HAL_TIMER_MASK(CORE_WAKEUP_TIMER_NUM, AM_HAL_TIMER_COMPARE0)) )
    {
        NVIC->ICPR[(((uint32_t)TIMER_IRQn) >> 5UL)] = (uint32_t)(1UL << (((uint32_t)TIMER_IRQn) & 0x1FUL));
        //
        // One more race to consider.
        // If a different timer interrupt occurred while clearing the
        // common IRQ, set the timer common IRQ back to pending.
        //
        ui32IntStat = TIMER->INTSTAT ;
        ui32IntStat = ui32IntStat & TIMER->INTEN;
        if ( ui32IntStat &
                ~AM_HAL_TIMER_MASK(CORE_WAKEUP_TIMER_NUM, AM_HAL_TIMER_COMPARE0) )
        {
            NVIC->ISPR[(((uint32_t)TIMER_IRQn) >> 5UL)] = (uint32_t)(1UL << (((uint32_t)TIMER_IRQn) & 0x1FUL));
        }
    }
} // core_wakeup_timer_deinit()

// ****************************************************************************
//
//  mcu_hp_lp_switch_sequence_fixed_trimmed()
//  MCU HP/LP switching sequence for trimver < MIN_PCM_TRIM_VER.
//
// ****************************************************************************
static uint32_t
mcu_hp_lp_switch_sequence_fixed_trimmed(am_hal_pwrctrl_mcu_mode_e ePowerMode)
{
    uint32_t ui32Status;

    AM_CRITICAL_BEGIN

    //
    // Disable cache
    //
    bool bDCacheEnabled = false;
    bool bICacheEnabled = false;
    if (SCB->CCR & SCB_CCR_IC_Msk)
    {
        am_hal_cachectrl_icache_disable();
        bICacheEnabled = true;
    }
    if (SCB->CCR & SCB_CCR_DC_Msk)
    {
        am_hal_cachectrl_dcache_disable();
        bDCacheEnabled = true;
    }

    uint32_t ui32MemLdoTrim = 0;

    g_eCurMcuPwrMode = ePowerMode;

    //
    // Boost memldo and coreldo for HP mode
    //
    if (ePowerMode == AM_HAL_PWRCTRL_MCU_MODE_HIGH_PERFORMANCE)
    {
        //
        // Override VDDCPU to VDDC
        //
        MCUCTRL->PWRSW0 = _VAL2FLD(MCUCTRL_PWRSW0_PWRSWVDDRCPUOVERRIDE, 0)  |              // PWRSWVDDRCPUOVERRIDE (Bit 31)
                          _VAL2FLD(MCUCTRL_PWRSW0_PWRSWVDDRCPUSTATSEL, 0)   |              // PWRSWVDDRCPUSTATSEL (Bit 30)
                          _VAL2FLD(MCUCTRL_PWRSW0_PWRSWVDDRCPUPGN, 1)       |              // PWRSWVDDRCPUPGN (Bit 29)
                          _VAL2FLD(MCUCTRL_PWRSW0_PWRSWVDDRCPUDYNSEL, 1)    |              // PWRSWVDDRCPUDYNSEL (Bit 27)
                          _VAL2FLD(MCUCTRL_PWRSW0_PWRSWVDDMLOVERRIDE, 0)    |              // PWRSWVDDMLOVERRIDE (Bit 26)
                          _VAL2FLD(MCUCTRL_PWRSW0_PWRSWVDDMLSTATSEL, 1)     |              // PWRSWVDDMLSTATSEL (Bit 25)
                          _VAL2FLD(MCUCTRL_PWRSW0_PWRSWVDDMLDYNSEL, 1)      |              // PWRSWVDDMLDYNSEL (Bit 24)
                          _VAL2FLD(MCUCTRL_PWRSW0_PWRSWVDDMCPUOVERRIDE, 0)  |              // PWRSWVDDMCPUOVERRIDE (Bit 17)
                          _VAL2FLD(MCUCTRL_PWRSW0_PWRSWVDDMCPUSTATSEL, 1)   |              // PWRSWVDDMCPUSTATSEL (Bit 16)
                          _VAL2FLD(MCUCTRL_PWRSW0_PWRSWVDDMCPUDYNSEL, 1)    |              // PWRSWVDDMCPUDYNSEL (Bit 15)
                          _VAL2FLD(MCUCTRL_PWRSW0_PWRSWVDDCAORPGN, 1)       |              // PWRSWVDDCAORPGN (Bit 7)
                          _VAL2FLD(MCUCTRL_PWRSW0_PWRSWVDDCAOROVERRIDE, 1)  |              // PWRSWVDDCAOROVERRIDE (Bit 6)
                          _VAL2FLD(MCUCTRL_PWRSW0_PWRSWVDDCAORDYNSEL, 1)    |              // PWRSWVDDCAORDYNSEL (Bit 5 - 4)
                          _VAL2FLD(MCUCTRL_PWRSW0_PWRSWVDDCPUOVERRIDE, 1)   |              // PWRSWVDDCPUOVERRIDE (Bit 3)
                          _VAL2FLD(MCUCTRL_PWRSW0_PWRSWVDDCPUPGN, 1)        |              // PWRSWVDDCPUPGN (Bit 2)
                          _VAL2FLD(MCUCTRL_PWRSW0_PWRSWVDDCPUDYNSEL, 1);                   // PWRSWVDDCPUDYNSEL (Bit 1 - 0)
        //
        // Boost memldo active trim by 6 codes
        //
        g_orig_MEMLDOTRIM = MCUCTRL->LDOREG2_b.MEMLDOACTIVETRIM;
        ui32MemLdoTrim = g_orig_MEMLDOTRIM + 6;
        if (ui32MemLdoTrim > (MCUCTRL_LDOREG2_MEMLDOACTIVETRIM_Msk >> MCUCTRL_LDOREG2_MEMLDOACTIVETRIM_Pos))
        {
            ui32MemLdoTrim = (MCUCTRL_LDOREG2_MEMLDOACTIVETRIM_Msk >> MCUCTRL_LDOREG2_MEMLDOACTIVETRIM_Pos);
        }
        MCUCTRL->LDOREG2_b.MEMLDOACTIVETRIM = ui32MemLdoTrim;
        //
        // Change coreldo tempco to 0
        //
        MCUCTRL->LDOREG1_b.CORELDOTEMPCOTRIM = 0;

        // #### INTERNAL BEGIN ####
        // Added 15us delay per validation suggestion.
        // #### INTERNAL END ####
        //
        // Wait for VDDC & VDDF stable
        //
        am_hal_delay_us(15);
    }

    //
    // Set the MCU power mode.
    //
    PWRCTRL->MCUPERFREQ_b.MCUPERFREQ = ePowerMode;

    //
    // Wait for the ACK
    //
    ui32Status = AM_HAL_STATUS_TIMEOUT;
    for ( uint32_t i = 0; i < 5; i++ )
    {
        if ( PWRCTRL->MCUPERFREQ_b.MCUPERFACK > 0 )
        {
            ui32Status = AM_HAL_STATUS_SUCCESS;
            break;
        }
        am_hal_delay_us(1);
    }

    //
    // Restore VDDC and VDDF settings for LP mode
    //
    if (ePowerMode == AM_HAL_PWRCTRL_MCU_MODE_LOW_POWER)
    {
        //
        // Cancel VDDCPU to VDDC override
        //
        MCUCTRL->PWRSW0 = _VAL2FLD(MCUCTRL_PWRSW0_PWRSWVDDRCPUOVERRIDE, 0)  |              // PWRSWVDDRCPUOVERRIDE (Bit 31)
                          _VAL2FLD(MCUCTRL_PWRSW0_PWRSWVDDRCPUSTATSEL, 0)   |              // PWRSWVDDRCPUSTATSEL (Bit 30)
                          _VAL2FLD(MCUCTRL_PWRSW0_PWRSWVDDRCPUPGN, 1)       |              // PWRSWVDDRCPUPGN (Bit 29)
                          _VAL2FLD(MCUCTRL_PWRSW0_PWRSWVDDRCPUDYNSEL, 1)    |              // PWRSWVDDRCPUDYNSEL (Bit 27)
                          _VAL2FLD(MCUCTRL_PWRSW0_PWRSWVDDMLOVERRIDE, 0)    |              // PWRSWVDDMLOVERRIDE (Bit 26)
                          _VAL2FLD(MCUCTRL_PWRSW0_PWRSWVDDMLSTATSEL, 1)     |              // PWRSWVDDMLSTATSEL (Bit 25)
                          _VAL2FLD(MCUCTRL_PWRSW0_PWRSWVDDMLDYNSEL, 1)      |              // PWRSWVDDMLDYNSEL (Bit 24)
                          _VAL2FLD(MCUCTRL_PWRSW0_PWRSWVDDMCPUOVERRIDE, 0)  |              // PWRSWVDDMCPUOVERRIDE (Bit 17)
                          _VAL2FLD(MCUCTRL_PWRSW0_PWRSWVDDMCPUSTATSEL, 1)   |              // PWRSWVDDMCPUSTATSEL (Bit 16)
                          _VAL2FLD(MCUCTRL_PWRSW0_PWRSWVDDMCPUDYNSEL, 1)    |              // PWRSWVDDMCPUDYNSEL (Bit 15)
                          _VAL2FLD(MCUCTRL_PWRSW0_PWRSWVDDCAORPGN, 1)       |              // PWRSWVDDCAORPGN (Bit 7)
                          _VAL2FLD(MCUCTRL_PWRSW0_PWRSWVDDCAOROVERRIDE, 1)  |              // PWRSWVDDCAOROVERRIDE (Bit 6)
                          _VAL2FLD(MCUCTRL_PWRSW0_PWRSWVDDCAORDYNSEL, 1)    |              // PWRSWVDDCAORDYNSEL (Bit 5 - 4)
                          _VAL2FLD(MCUCTRL_PWRSW0_PWRSWVDDCPUOVERRIDE, 0)   |              // PWRSWVDDCPUOVERRIDE (Bit 3)
                          _VAL2FLD(MCUCTRL_PWRSW0_PWRSWVDDCPUPGN, 1)        |              // PWRSWVDDCPUPGN (Bit 2)
                          _VAL2FLD(MCUCTRL_PWRSW0_PWRSWVDDCPUDYNSEL, 2);                   // PWRSWVDDCPUDYNSEL (Bit 1 - 0)
        //
        // Change coreldo tempco to 8
        //
        MCUCTRL->LDOREG1_b.CORELDOTEMPCOTRIM = 8;
        //
        // Restore memldo active trim
        //
        if (g_orig_MEMLDOTRIM)
        {
            MCUCTRL->LDOREG2_b.MEMLDOACTIVETRIM = g_orig_MEMLDOTRIM;
        }
    }

    //
    // Enable cache
    //
    if (bICacheEnabled)
    {
        am_hal_cachectrl_icache_enable();
    }
    if (bDCacheEnabled)
    {
        am_hal_cachectrl_dcache_enable(true);
    }

    AM_CRITICAL_END

    return ui32Status;
} // mcu_hp_lp_switch_sequence_fixed_trimmed()

// ****************************************************************************
//
// Set ITCM/DTCM/Cache trims
// #### INTERNAL BEGIN ####
// ITCM/DTCM/ICACHE/DCACHE EMA trims from Validation for HP mode.
// MCU_CTRL:CPUICACHETRIM:CPUICACHETAGEMA (change from 3 to 2)
// MCU_CTRL:CPUDCACHETRIM:CPUDCACHEDATAEMA (change from 3 to 2)
// MCU_CTRL:CPUDCACHETRIM:CPUDCACHETAGEMA (change from 4 to 3)
// MCU_CTRL:SRAMTRIM:DTCMEMA (change from 3 to 2)
// MCU_CTRL:SRAMTRIM:ITCMEMA (change from 3 to 2)
// #### INTERNAL END ####
//
// ****************************************************************************
static inline void
tcm_cache_trim_set(void)
{
    register uint32_t ui32RegVal;
    TRIM_WR_ENABLE
    //
    // DTCMEMA = 2, ITCMEMA = 2.
    //
    ui32RegVal = AM_REGVAL(0x40040334);
    ui32RegVal &= ~(7UL << 16);
    ui32RegVal &= ~(7UL << 0);
    ui32RegVal |= (2UL << 16);
    ui32RegVal |= (2UL << 0);
    AM_REGVAL(0x40040334) = ui32RegVal;

    //
    // CPUICACHETAGEMA = 2.
    //
    ui32RegVal = AM_REGVAL(0x40040418);
    ui32RegVal &= ~(7UL << 2);
    ui32RegVal |= (2UL << 2);
    AM_REGVAL(0x40040418) = ui32RegVal;
    //
    // CPUDCACHETAGEMA = 3, CPUDCACHEDATAEMA = 2.
    //
    ui32RegVal = AM_REGVAL(0x40040424);
    ui32RegVal &= ~(7UL << 18);
    ui32RegVal &= ~(7UL << 2);
    ui32RegVal |= (2UL << 18);
    ui32RegVal |= (3UL << 2);
    AM_REGVAL(0x40040424) = ui32RegVal;
    TRIM_WR_DISABLE
} // tcm_cache_trim_set()

// ****************************************************************************
//
//  mcu_hp_lp_switch_sequence_pcm_trimmed()
//  MCU HP/LP switching sequence for trimver >= MIN_PCM_TRIM_VER.
//
// ****************************************************************************
static uint32_t
mcu_hp_lp_switch_sequence_pcm_trimmed(am_hal_pwrctrl_mcu_mode_e ePowerMode)
{
    register uint32_t ui32Status;
    uint32_t          ui32MRAMPWRCTRLVal;
    uint32_t ui32OrigVal;

    g_eCurMcuPwrMode = ePowerMode;

    //
    // Have the core sleep CORE_SLEEP_DURATION(300)
    // us during switching VDDCPU
    //
    bool bGFXEnabled = false;

    ui32Status = am_critical_mask_all_begin();
    if (ui32Status != AM_HAL_STATUS_SUCCESS)
    {
        return ui32Status;
    }

    //
    // Check if GPU is already enabled.  If no, turn on GFX as work-around for register retention issue.
    //
    am_hal_pwrctrl_periph_enabled(AM_HAL_PWRCTRL_PERIPH_GFX, &bGFXEnabled);
    if (!bGFXEnabled)
    {
        am_hal_pwrctrl_periph_enable(AM_HAL_PWRCTRL_PERIPH_GFX);
    }
    //
    // Init the wakeup timer
    //
    core_wakeup_timer_init(CORE_SLEEP_DURATION);

    //
    // Save the current value of the MRAMPWRCTRL settings.
    //
    ui32MRAMPWRCTRLVal = MCUCTRL->MRAMPWRCTRL;

    //
    // Disable MRAM LP Mode.
    //
    MCUCTRL->MRAMPWRCTRL_b.MRAMLPREN = 0;
    MCUCTRL->MRAMPWRCTRL_b.MRAMSLPEN = 0;
    MCUCTRL->MRAMPWRCTRL_b.MRAMPWRCTRL = 1;

    am_hal_itm_handle_deepsleep_enter();

    ui32OrigVal = MCUCTRL->PWRSW0;
    //
    //  Switch VDDCPU
    //
    if (ePowerMode == AM_HAL_PWRCTRL_MCU_MODE_HIGH_PERFORMANCE)
    {
        //
        // Boost VDDF
        //
        am_hal_pwrctrl_vddf_boost(AM_HAL_PWRCTRL_VDDF_BOOST_LEVEL1);

        //
        // Boost VDDC buck and ldo
        //
        am_hal_pwrctrl_vddc_boost();

        // #### INTERNAL BEGIN ####
        // This sequence is from Validation Team.
        // #### INTERNAL END ####
        //
        // Switch VDDCPU to VDDF
        //
        SCB->SCR |= _VAL2FLD(SCB_SCR_SLEEPDEEP, 1);
        am_hal_delay_us(1);

        //
        // Start timer
        //
        TIMERn(CORE_WAKEUP_TIMER_NUM)->CTRL0_b.TMR0EN = 1;

        //
        // Switch VDDCPU to VDDF
        // MCUCTRL->PWRSW0 = 0x2b0180ff;
        //
        MCUCTRL->PWRSW0 = _VAL2FLD(MCUCTRL_PWRSW0_PWRSWVDDRCPUOVERRIDE, 0)  |              // PWRSWVDDRCPUOVERRIDE (Bit 31)
                          _VAL2FLD(MCUCTRL_PWRSW0_PWRSWVDDRCPUSTATSEL, 0)   |              // PWRSWVDDRCPUSTATSEL (Bit 30)
                          _VAL2FLD(MCUCTRL_PWRSW0_PWRSWVDDRCPUPGN, 1)       |              // PWRSWVDDRCPUPGN (Bit 29)
                          _VAL2FLD(MCUCTRL_PWRSW0_PWRSWVDDRCPUDYNSEL, 1)    |              // PWRSWVDDRCPUDYNSEL (Bit 27)
                          _VAL2FLD(MCUCTRL_PWRSW0_PWRSWVDDMLOVERRIDE, 0)    |              // PWRSWVDDMLOVERRIDE (Bit 26)
                          _VAL2FLD(MCUCTRL_PWRSW0_PWRSWVDDMLSTATSEL, 1)     |              // PWRSWVDDMLSTATSEL (Bit 25)
                          _VAL2FLD(MCUCTRL_PWRSW0_PWRSWVDDMLDYNSEL, 1)      |              // PWRSWVDDMLDYNSEL (Bit 24)
                          _VAL2FLD(MCUCTRL_PWRSW0_PWRSWVDDMCPUOVERRIDE, 0)  |              // PWRSWVDDMCPUOVERRIDE (Bit 17)
                          _VAL2FLD(MCUCTRL_PWRSW0_PWRSWVDDMCPUSTATSEL, 1)   |              // PWRSWVDDMCPUSTATSEL (Bit 16)
                          _VAL2FLD(MCUCTRL_PWRSW0_PWRSWVDDMCPUDYNSEL, 1)    |              // PWRSWVDDMCPUDYNSEL (Bit 15)
                          _VAL2FLD(MCUCTRL_PWRSW0_PWRSWVDDCAORPGN, 1)       |              // PWRSWVDDCAORPGN (Bit 7)
                          _VAL2FLD(MCUCTRL_PWRSW0_PWRSWVDDCAOROVERRIDE, 1)  |              // PWRSWVDDCAOROVERRIDE (Bit 6)
                          _VAL2FLD(MCUCTRL_PWRSW0_PWRSWVDDCAORDYNSEL, 3)    |              // PWRSWVDDCAORDYNSEL (Bit 5 - 4)
                          _VAL2FLD(MCUCTRL_PWRSW0_PWRSWVDDCPUOVERRIDE, 1)   |              // PWRSWVDDCPUOVERRIDE (Bit 3)
                          _VAL2FLD(MCUCTRL_PWRSW0_PWRSWVDDCPUPGN, 1)        |              // PWRSWVDDCPUPGN (Bit 2)
                          _VAL2FLD(MCUCTRL_PWRSW0_PWRSWVDDCPUDYNSEL, 3);                   // PWRSWVDDCPUDYNSEL (Bit 1 - 0)
        am_hal_sysctrl_sysbus_write_flush();
        __WFI();
        __ISB();
        //
        // Set the MCU power mode.
        //
        PWRCTRL->MCUPERFREQ_b.MCUPERFREQ = ePowerMode;

        //
        // Wait for the ACK
        //
        ui32Status = AM_HAL_STATUS_TIMEOUT;
        for ( uint32_t i = 0; i < 5; i++ )
        {
            if ( PWRCTRL->MCUPERFREQ_b.MCUPERFACK > 0 )
            {
                ui32Status = AM_HAL_STATUS_SUCCESS;
                break;
            }
            am_hal_delay_us(1);
        }
    }
    else
    {
        //
        // Set the MCU power mode.
        //
        PWRCTRL->MCUPERFREQ_b.MCUPERFREQ = ePowerMode;
         //
        // Wait for the ACK
        //
        ui32Status = AM_HAL_STATUS_TIMEOUT;
        for ( uint32_t i = 0; i < 5; i++ )
        {
            if ( PWRCTRL->MCUPERFREQ_b.MCUPERFACK > 0 )
            {
                ui32Status = AM_HAL_STATUS_SUCCESS;
                break;
            }
            am_hal_delay_us(1);
        }

        am_hal_delay_us(20);
        SCB->SCR |= _VAL2FLD(SCB_SCR_SLEEPDEEP, 1);
        am_hal_delay_us(1);

        //
        // Start timer
        //
        TIMERn(CORE_WAKEUP_TIMER_NUM)->CTRL0_b.TMR0EN = 1;

        MCUCTRL->PWRSW0 = _VAL2FLD(MCUCTRL_PWRSW0_PWRSWVDDRCPUOVERRIDE, 0)  |              // PWRSWVDDRCPUOVERRIDE (Bit 31)
                          _VAL2FLD(MCUCTRL_PWRSW0_PWRSWVDDRCPUSTATSEL, 0)   |              // PWRSWVDDRCPUSTATSEL (Bit 30)
                          _VAL2FLD(MCUCTRL_PWRSW0_PWRSWVDDRCPUPGN, 1)       |              // PWRSWVDDRCPUPGN (Bit 29)
                          _VAL2FLD(MCUCTRL_PWRSW0_PWRSWVDDRCPUDYNSEL, 1)    |              // PWRSWVDDRCPUDYNSEL (Bit 27)
                          _VAL2FLD(MCUCTRL_PWRSW0_PWRSWVDDMLOVERRIDE, 0)    |              // PWRSWVDDMLOVERRIDE (Bit 26)
                          _VAL2FLD(MCUCTRL_PWRSW0_PWRSWVDDMLSTATSEL, 1)     |              // PWRSWVDDMLSTATSEL (Bit 25)
                          _VAL2FLD(MCUCTRL_PWRSW0_PWRSWVDDMLDYNSEL, 1)      |              // PWRSWVDDMLDYNSEL (Bit 24)
                          _VAL2FLD(MCUCTRL_PWRSW0_PWRSWVDDMCPUOVERRIDE, 0)  |              // PWRSWVDDMCPUOVERRIDE (Bit 17)
                          _VAL2FLD(MCUCTRL_PWRSW0_PWRSWVDDMCPUSTATSEL, 1)   |              // PWRSWVDDMCPUSTATSEL (Bit 16)
                          _VAL2FLD(MCUCTRL_PWRSW0_PWRSWVDDMCPUDYNSEL, 1)    |              // PWRSWVDDMCPUDYNSEL (Bit 15)
                          _VAL2FLD(MCUCTRL_PWRSW0_PWRSWVDDCAORPGN, 1)       |              // PWRSWVDDCAORPGN (Bit 7)
                          _VAL2FLD(MCUCTRL_PWRSW0_PWRSWVDDCAOROVERRIDE, 0)  |              // PWRSWVDDCAOROVERRIDE (Bit 6)
                          _VAL2FLD(MCUCTRL_PWRSW0_PWRSWVDDCAORDYNSEL, 2)    |              // PWRSWVDDCAORDYNSEL (Bit 5 - 4)
                          _VAL2FLD(MCUCTRL_PWRSW0_PWRSWVDDCPUOVERRIDE, 0)   |              // PWRSWVDDCPUOVERRIDE (Bit 3)
                          _VAL2FLD(MCUCTRL_PWRSW0_PWRSWVDDCPUPGN, 1)        |              // PWRSWVDDCPUPGN (Bit 2)
                          _VAL2FLD(MCUCTRL_PWRSW0_PWRSWVDDCPUDYNSEL, 2);                   // PWRSWVDDCPUDYNSEL (Bit 1 - 0)
        am_hal_sysctrl_sysbus_write_flush();
        __WFI();
        __ISB();

        //
        // Restore VDDF
        //
        am_hal_pwrctrl_vddf_restore(AM_HAL_PWRCTRL_VDDF_BOOST_LEVEL1);
        //
        // Restore VDDC buck and ldo
        //
        am_hal_pwrctrl_vddc_restore();
    }
    //
    // Revert overrides
    //
    MCUCTRL->PWRSW0 = ui32OrigVal;

    //
    // Restore the current value of the MRAMPWRCTRL settings.
    //
    MCUCTRL->MRAMPWRCTRL = ui32MRAMPWRCTRLVal;

    am_hal_itm_handle_deepsleep_exit();
    //
    // shut down timer
    //
    core_wakeup_timer_deinit();

    //
    // If GFX was turned on in this function, disable it here.
    // If bGFXEnabled == false, we assume that GFX was force enabled before mode switching in this function.
    //
    if (!bGFXEnabled)
    {
        am_hal_pwrctrl_periph_disable(AM_HAL_PWRCTRL_PERIPH_GFX);
    }

    am_critical_mask_all_end();

    return ui32Status;
} // mcu_hp_lp_switch_sequence_pcm_trimmed()

// ****************************************************************************
//
//  am_hal_pwrctrl_mcu_mode_select()
//  Select the MCU power mode.
//
// ****************************************************************************
uint32_t
am_hal_pwrctrl_mcu_mode_select(am_hal_pwrctrl_mcu_mode_e ePowerMode)
{
    uint32_t ui32Status;

#ifndef AM_HAL_DISABLE_API_VALIDATION
    if ( (ePowerMode != AM_HAL_PWRCTRL_MCU_MODE_LOW_POWER)      &&
         (ePowerMode != AM_HAL_PWRCTRL_MCU_MODE_HIGH_PERFORMANCE) )
    {
        return AM_HAL_STATUS_INVALID_ARG;
    }

    //
    // We must be using SIMOBUCK in order to go to HP mode.
    //
    if ( (ePowerMode == AM_HAL_PWRCTRL_MCU_MODE_HIGH_PERFORMANCE)   &&
         (PWRCTRL->VRSTATUS_b.SIMOBUCKST != PWRCTRL_VRSTATUS_SIMOBUCKST_ACT) )
    {
        return AM_HAL_STATUS_INVALID_OPERATION;
    }
#endif // AM_HAL_DISABLE_API_VALIDATION

    //
    // If trimver <= 6, disallow mcu to enter HP250M
    //
    if ( ((g_ui32TrimVer == 0xFFFFFFFF) || (g_ui32TrimVer <= MIN_PCM_TRIM_VER)) &&
         (CLKGEN->MISC_b.CPUHPFREQSEL == CLKGEN_MISC_CPUHPFREQSEL_HP250MHz) )
    {
        return AM_HAL_STATUS_INVALID_OPERATION;
    }
    //
    // Check if the request mode is already selected.  If so, return SUCCESS.
    //
    if ( ePowerMode == g_eCurMcuPwrMode )
    {
        return AM_HAL_STATUS_SUCCESS;
    }

    //
    // Check if original MEMLDOTRIM and CORELDOTEMPCOTRIM were saved.
    // These original settings are saved in am_hal_pwrctrl_low_power_init.
    //
    if ( (g_orig_MEMLDOTRIM == 0xFFFFFFFF)        ||
         (g_orig_CORELDOTEMPCOTRIM == 0xFFFFFFFF) ||
         (g_orig_TVRGFVREFTRIM == 0xFFFFFFFF)     ||
         (g_orig_D2ASPARE == 0xFFFFFFFF)          ||
         (g_orig_VDDCACTLOWTONTRIM == 0xFFFFFFFF) ||
         (g_orig_VDDFACTLOWTONTRIM == 0xFFFFFFFF) ||
         (g_orig_TVRGFTEMPCOTRIM == 0xFFFFFFFF) )
    {
        return AM_HAL_STATUS_INVALID_ARG;
    }

    //
    // Check trimver
    //
    if ( (g_ui32TrimVer == 0xFFFFFFFF) || (g_ui32TrimVer < MIN_PCM_TRIM_VER) ) // If trimver < MIN_PCM_TRIM_VER, call the switching sequence for fixed trimmed parts.
    {
        ui32Status = mcu_hp_lp_switch_sequence_fixed_trimmed(ePowerMode);
    }
    else // trimver >= MIN_PCM_TRIM_VER, call the switching sequence for pcm trimmed parts
    {
        if (ePowerMode == AM_HAL_PWRCTRL_MCU_MODE_HIGH_PERFORMANCE)
        {
            if (CLKGEN->MISC_b.CPUHPFREQSEL == CLKGEN_MISC_CPUHPFREQSEL_HP250MHz)
            {
                //
                // HFRC2 ON request.
                //
                am_hal_clkgen_control(AM_HAL_CLKGEN_CONTROL_HFRC2_ON_REQ, false);
            }
            //
            // HP/LP switching sequence
            //
            ui32Status = mcu_hp_lp_switch_sequence_pcm_trimmed(ePowerMode);
        }
        else
        {
            //
            // HP/LP switching sequence
            //
            ui32Status = mcu_hp_lp_switch_sequence_pcm_trimmed(ePowerMode);
            if (CLKGEN->MISC_b.CPUHPFREQSEL == CLKGEN_MISC_CPUHPFREQSEL_HP250MHz)
            {
                //
                // HFRC2 OFF request.
                //
                am_hal_clkgen_control(AM_HAL_CLKGEN_CONTROL_HFRC2_OFF_REQ, false);
            }
        }
    }

    //
    // Check for timeout.
    //
    if ( ui32Status != AM_HAL_STATUS_SUCCESS )
    {
        //
        // Caution: Reaching this point means the device is in an unpredictable
        //          state and may not be able to recover.
        //
        return ui32Status;
    }

    //
    // Check the MCU power mode status and return SUCCESS/FAIL.
    //
    if ( PWRCTRL->MCUPERFREQ_b.MCUPERFSTATUS == ePowerMode )
    {
        return AM_HAL_STATUS_SUCCESS;
    }
    else
    {
        //
        // Caution: Reaching this point means the device is in an unpredictable
        //          state and may not be able to recover.
        //
        return AM_HAL_STATUS_FAIL;
    }

} // am_hal_pwrctrl_mcu_mode_select()

// ****************************************************************************
//
//  am_hal_pwrctrl_gpu_mode_status()
//
// ****************************************************************************
uint32_t
am_hal_pwrctrl_gpu_mode_status(am_hal_pwrctrl_gpu_mode_e *peCurrentPowerMode)
{
#ifndef AM_HAL_DISABLE_API_VALIDATION
    if ( peCurrentPowerMode == 0 )
    {
        return AM_HAL_STATUS_INVALID_ARG;
    }
#endif // AM_HAL_DISABLE_API_VALIDATION

    *peCurrentPowerMode = g_eCurGpuPwrMode;
    return AM_HAL_STATUS_SUCCESS;

} // am_hal_pwrctrl_mcu_mode_status()

// ****************************************************************************
//
//  gpu_hp_lp_switch_sequence_fixed_trimmed()
//  GPU HP/LP switching sequence for trimver < MIN_PCM_TRIM_VER.
//
// ****************************************************************************
static void
gpu_hp_lp_switch_sequence_fixed_trimmed(am_hal_pwrctrl_gpu_mode_e ePowerMode)
{
    AM_CRITICAL_BEGIN

    //
    // Adjust the voltage supplied to the GPU.
    //
    if (ePowerMode == AM_HAL_PWRCTRL_GPU_MODE_HIGH_PERFORMANCE)
    {
        PWRCTRL->GFXPWRSWSEL_b.GFXVDDSEL = PWRCTRL_GFXPWRSWSEL_GFXVDDSEL_VDDF;
    }
    else
    {
        PWRCTRL->GFXPWRSWSEL_b.GFXVDDSEL = PWRCTRL_GFXPWRSWSEL_GFXVDDSEL_VDDC;
    }

    //
    // Wait for voltage to settle.
    //
    am_hal_delay_us(AM_HAL_PWRCTRL_GPU_VOLTADJ_WAIT);

    //
    // Adjust the clock supplied to the GPU.
    // limit fixed trimmed parts to 192MHz.
    //
    PWRCTRL->GFXPERFREQ_b.GFXPERFREQ = PWRCTRL_GFXPERFREQ_GFXPERFREQ_HP1;
    g_eCurGpuPwrMode = ePowerMode;
    g_ePreGpuPwrMode = ePowerMode;

    //
    // Wait for clock to settle.
    //
    am_hal_delay_us(AM_HAL_PWRCTRL_GPU_PWRADJ_WAIT);

    AM_CRITICAL_END
} // gpu_hp_lp_switch_sequence_fixed_trimmed()

// ****************************************************************************
//
//  gpu_hp_lp_switch_sequence_pcm_trimmed()
//  GPU HP/LP switching sequence for trimver >= MIN_PCM_TRIM_VER.
//
// ****************************************************************************
static void
gpu_hp_lp_switch_sequence_pcm_trimmed(am_hal_pwrctrl_gpu_mode_e ePowerMode)
{

    AM_CRITICAL_BEGIN

    //
    // Power settings for HP mode
    //
    if (ePowerMode == AM_HAL_PWRCTRL_GPU_MODE_HIGH_PERFORMANCE)
    {
        //
        // Boost VDDF buck and ldo
        //
        am_hal_pwrctrl_vddf_boost(AM_HAL_PWRCTRL_VDDF_BOOST_LEVEL1);
    }

    //
    // Adjust the voltage supplied to the GPU.
    //
    if (ePowerMode == AM_HAL_PWRCTRL_GPU_MODE_HIGH_PERFORMANCE)
    {
        PWRCTRL->GFXPWRSWSEL_b.GFXVDDSEL = PWRCTRL_GFXPWRSWSEL_GFXVDDSEL_VDDF;
    }
    else
    {
        PWRCTRL->GFXPWRSWSEL_b.GFXVDDSEL = PWRCTRL_GFXPWRSWSEL_GFXVDDSEL_VDDC;
    }

    //
    // Wait for voltage to settle.
    //
    am_hal_delay_us(AM_HAL_PWRCTRL_GPU_VOLTADJ_WAIT);

    //
    // Adjust the clock supplied to the GPU.
    // if trimver == 6, limit to 192MHz.
    //
    if (g_ui32TrimVer == MIN_PCM_TRIM_VER)
    {
        PWRCTRL->GFXPERFREQ_b.GFXPERFREQ = PWRCTRL_GFXPERFREQ_GFXPERFREQ_HP1;
    }
    else
    {
        PWRCTRL->GFXPERFREQ_b.GFXPERFREQ = ePowerMode;
    }

    g_eCurGpuPwrMode = ePowerMode;
    g_ePreGpuPwrMode = ePowerMode;

    //
    // Wait for clock to settle.
    //
    am_hal_delay_us(AM_HAL_PWRCTRL_GPU_PWRADJ_WAIT);

    //
    // Restore power settings for LP mode
    //
    if (ePowerMode == AM_HAL_PWRCTRL_GPU_MODE_LOW_POWER)
    {
        //
        // Restore VDDF buck and ldo
        //
        am_hal_pwrctrl_vddf_restore(AM_HAL_PWRCTRL_VDDF_BOOST_LEVEL1);
    }

    AM_CRITICAL_END
} // gpu_hp_lp_switch_sequence_pcm_trimmed()

// ****************************************************************************
//
//  am_hal_pwrctrl_gpu_mode_select()
//  Select the GPU power mode.
//
// ****************************************************************************
uint32_t
am_hal_pwrctrl_gpu_mode_select(am_hal_pwrctrl_gpu_mode_e ePowerMode)
{
    bool     bGFXEnabled;

#ifndef AM_HAL_DISABLE_API_VALIDATION
    if ( (ePowerMode != AM_HAL_PWRCTRL_GPU_MODE_LOW_POWER)      &&
         (ePowerMode != AM_HAL_PWRCTRL_GPU_MODE_HIGH_PERFORMANCE) )
    {
        return AM_HAL_STATUS_INVALID_ARG;
    }
#endif // AM_HAL_DISABLE_API_VALIDATION

#if !defined(APOLLO5_FPGA)
    //
    // We must be using SIMOBUCK in order to go to HP mode.
    //
    if ( (ePowerMode == AM_HAL_PWRCTRL_GPU_MODE_HIGH_PERFORMANCE)   &&
         (PWRCTRL->VRSTATUS_b.SIMOBUCKST != PWRCTRL_VRSTATUS_SIMOBUCKST_ACT) )
    {
        return AM_HAL_STATUS_INVALID_OPERATION;
    }
#endif

    //
    // Check if GPU is already enabled.  If so, return an error code.
    //
    if ( AM_HAL_STATUS_SUCCESS != am_hal_pwrctrl_periph_enabled(AM_HAL_PWRCTRL_PERIPH_GFX, &bGFXEnabled) )
    {
        return AM_HAL_STATUS_FAIL;
    }
    else
    {
        if (bGFXEnabled)
        {
            return AM_HAL_STATUS_IN_USE;
        }
    }

    //
    // Check if the request mode is already selected.  If so, return SUCCESS.
    //
    if ( ePowerMode == g_eCurGpuPwrMode )
    {
        //
        // If GPU switched from HP to LP in API am_hal_pwrctrl_periph_disable,
        // it is allowed to change previous mode to LP.
        //
        if (g_ePreGpuPwrMode != g_eCurGpuPwrMode)
        {
            g_ePreGpuPwrMode = ePowerMode;
        }
        return AM_HAL_STATUS_SUCCESS;
    }

    //
    // Check trimver
    //
    if ( (g_ui32TrimVer == 0xFFFFFFFF) || (g_ui32TrimVer < MIN_PCM_TRIM_VER) ) // If trimver < MIN_PCM_TRIM_VER, call the switching sequence for fixed trimmed parts.
    {
        gpu_hp_lp_switch_sequence_fixed_trimmed(ePowerMode);
    }
    else // trimver >= MIN_PCM_TRIM_VER, call the switching sequence for pcm trimmed parts
    {
        if (ePowerMode == AM_HAL_PWRCTRL_GPU_MODE_HIGH_PERFORMANCE)
        {
            if (g_ui32TrimVer > MIN_PCM_TRIM_VER)
            {
                //
                // HFRC2 ON request.
                //
                am_hal_clkgen_control(AM_HAL_CLKGEN_CONTROL_HFRC2_ON_REQ, false);
            }
            //
            // gpu mode switching sequence
            //
            gpu_hp_lp_switch_sequence_pcm_trimmed(ePowerMode);
        }
        else
        {
            //
            // gpu mode switching sequence
            //
            gpu_hp_lp_switch_sequence_pcm_trimmed(ePowerMode);
            if (g_ui32TrimVer > MIN_PCM_TRIM_VER)
            {
                //
                // HFRC2 OFF request.
                //
                am_hal_clkgen_control(AM_HAL_CLKGEN_CONTROL_HFRC2_OFF_REQ, false);
            }
        }

    }

    return AM_HAL_STATUS_SUCCESS;

} // am_hal_pwrctrl_gpu_mode_select()

// ****************************************************************************
//
//  am_hal_pwrctrl_mcu_memory_config()
//  Configure the MCU memory.
//
// ****************************************************************************
uint32_t
am_hal_pwrctrl_mcu_memory_config(am_hal_pwrctrl_mcu_memory_config_t *psConfig)
{
    uint32_t      ui32Status;
    uint32_t      ui32PwrStatus = 0;

    //
    // Configure the ROM power mode.
    //
    g_eCurROMPwrMode = psConfig->eROMMode;

    switch ( psConfig->eROMMode)
    {
        case AM_HAL_PWRCTRL_ROM_AUTO:
            //
            // Initialize the ROM to off.  Let the HAL access function control the power.
            //
            PWRCTRL->MEMPWREN_b.PWRENROM = PWRCTRL_MEMPWREN_PWRENROM_DIS;
            ui32PwrStatus |= _VAL2FLD(PWRCTRL_MEMPWRSTATUS_PWRSTROM, 0);
            break;
        case AM_HAL_PWRCTRL_ROM_ALWAYS_ON:
            //
            // Initialize the ROM to be always on.
            //
            PWRCTRL->MEMPWREN_b.PWRENROM = PWRCTRL_MEMPWREN_PWRENROM_EN;
            ui32PwrStatus |= _VAL2FLD(PWRCTRL_MEMPWRSTATUS_PWRSTROM, 1);
            break;
    }

    //
    // Configure the MCU Cache.
    //
    if ( psConfig->bEnableCache )
    {
        PWRCTRL->MEMPWREN_b.PWRENCACHE = PWRCTRL_MEMPWREN_PWRENCACHE_EN;
        ui32PwrStatus |= _VAL2FLD(PWRCTRL_MEMPWRSTATUS_PWRSTCACHE, 1);
    }
    else
    {
        //
        // Check if DCache or ICache was enabled, if yes, return error.
        //
        if ((SCB->CCR & SCB_CCR_IC_Msk) || (SCB->CCR & SCB_CCR_DC_Msk))
        {
            return AM_HAL_STATUS_IN_USE;
        }
        PWRCTRL->MEMPWREN_b.PWRENCACHE = PWRCTRL_MEMPWREN_PWRENCACHE_DIS;
        ui32PwrStatus |= _VAL2FLD(PWRCTRL_MEMPWRSTATUS_PWRSTCACHE, 0);
    }
    //
    // Configure the MCU Tightly Coupled Memory.
    //
    PWRCTRL->MEMPWREN_b.PWRENDTCM = psConfig->eDTCMCfg;
    ui32PwrStatus |= _VAL2FLD(PWRCTRL_MEMPWRSTATUS_PWRSTDTCM, psConfig->eDTCMCfg);

    //
    // Configure the Non-Volatile Memory.
    //
    if (psConfig->bEnableNVM)
    {
        PWRCTRL->MEMPWREN_b.PWRENNVM = 1;
        ui32PwrStatus |= _VAL2FLD(PWRCTRL_MEMPWRSTATUS_PWRSTNVM0, 1);
    }
    else
    {
        PWRCTRL->MEMPWREN_b.PWRENNVM = 0;
        ui32PwrStatus |= _VAL2FLD(PWRCTRL_MEMPWRSTATUS_PWRSTNVM0, 0);
    }

    DIAG_SUPPRESS_VOLATILE_ORDER()
    //
    // Wait for Status
    //
    ui32Status = am_hal_delay_us_status_check(AM_HAL_PWRCTRL_MAX_WAIT_US,
                                              (uint32_t)&PWRCTRL->MEMPWRSTATUS,
                                              AM_HAL_PWRCTRL_MEMPWRSTATUS_MASK,
                                              ui32PwrStatus,
                                              true);

    //
    // Check for timeout.
    //
    if (AM_HAL_STATUS_SUCCESS != ui32Status)
    {
        return ui32Status;
    }

    //
    // Check the MCU power mode status and return SUCCESS/FAIL.
    //
// #### INTERNAL BEGIN ####
#warning "TODO - Fixme!!! - MEMPWREN has only NMV, but MEMPWRSTATUS has NMV0 and NVM1."
// #### INTERNAL END ####
    if ((PWRCTRL->MEMPWRSTATUS_b.PWRSTDTCM != PWRCTRL->MEMPWREN_b.PWRENDTCM)        ||
        (PWRCTRL->MEMPWRSTATUS_b.PWRSTNVM0 != PWRCTRL->MEMPWREN_b.PWRENNVM)         ||
        (PWRCTRL->MEMPWRSTATUS_b.PWRSTCACHE != PWRCTRL->MEMPWREN_b.PWRENCACHE)      ||
        (PWRCTRL->MEMPWRSTATUS_b.PWRSTROM != PWRCTRL->MEMPWREN_b.PWRENROM)          ||
        (PWRCTRL->MEMPWRSTATUS_b.PWRSTOTP != PWRCTRL->DEVPWREN_b.PWRENOTP))
    {
        return AM_HAL_STATUS_FAIL;
    }

    DIAG_DEFAULT_VOLATILE_ORDER()

    //
    // Configure Cache retention.
    //
    if (psConfig->bRetainCache)
    {
        PWRCTRL->MEMRETCFG_b.CACHEPWDSLP = PWRCTRL_MEMRETCFG_CACHEPWDSLP_DIS;
    }
    else
    {
        PWRCTRL->MEMRETCFG_b.CACHEPWDSLP = PWRCTRL_MEMRETCFG_CACHEPWDSLP_EN;
    }

    //
    // Configure the Non-Volatile Memory retention.
    //
    if (psConfig->bKeepNVMOnInDeepSleep)
    {
        PWRCTRL->MEMRETCFG_b.NVMPWDSLP = PWRCTRL_MEMRETCFG_NVMPWDSLP_DIS;
    }
    else
    {
        PWRCTRL->MEMRETCFG_b.NVMPWDSLP = PWRCTRL_MEMRETCFG_NVMPWDSLP_EN;
    }

    //
    // Configure the MCU Tightly Coupled Memory retention.
    //
    switch ( psConfig->eRetainDTCM )
    {
        case AM_HAL_PWRCTRL_ITCM_DTCM_NONE:
            PWRCTRL->MEMRETCFG_b.DTCMPWDSLP = PWRCTRL_MEMRETCFG_DTCMPWDSLP_ALL;
            break;
        case AM_HAL_PWRCTRL_ITCM32K_DTCM128K:
            PWRCTRL->MEMRETCFG_b.DTCMPWDSLP = PWRCTRL_MEMRETCFG_DTCMPWDSLP_ALLBUTGROUP0DTCM0;
            break;
        case AM_HAL_PWRCTRL_ITCM128K_DTCM256K:
            PWRCTRL->MEMRETCFG_b.DTCMPWDSLP = PWRCTRL_MEMRETCFG_DTCMPWDSLP_GROUP1;
            break;
        case AM_HAL_PWRCTRL_ITCM256K_DTCM512K:
            PWRCTRL->MEMRETCFG_b.DTCMPWDSLP = PWRCTRL_MEMRETCFG_DTCMPWDSLP_NONE;
            break;
    }

    return AM_HAL_STATUS_SUCCESS;

} // am_hal_pwrctrl_mcu_memory_config()

// ****************************************************************************
//
//  am_hal_pwrctrl_mcu_memory_config_get()
//  Get the MCU Memory configuration.
//
// ****************************************************************************
uint32_t
am_hal_pwrctrl_mcu_memory_config_get(am_hal_pwrctrl_mcu_memory_config_t *psConfig)
{
    //
    // Get the MCU Cache configuration.
    //
    if (PWRCTRL->MEMPWREN_b.PWRENCACHE == PWRCTRL_MEMPWREN_PWRENCACHE_EN)
    {
            psConfig->bEnableCache = true;
    }
    else
    {
            psConfig->bEnableCache = false;
    }

    //
    // Get the MCU Tightly Coupled Memory configuration.
    //
    psConfig->eDTCMCfg =
        (am_hal_pwrctrl_dtcm_select_e)PWRCTRL->MEMPWREN_b.PWRENDTCM;

    //
    // Get the Non-Volatile Memory configuration.
    //
    psConfig->bEnableNVM = PWRCTRL->MEMPWREN_b.PWRENNVM;

    //
    // Get the Cache retention configuration.
    //
    psConfig->bRetainCache =
        (PWRCTRL->MEMRETCFG_b.CACHEPWDSLP == PWRCTRL_MEMRETCFG_CACHEPWDSLP_DIS);

    //
    // Configure the Non-Volatile Memory retention.
    //
    psConfig->bKeepNVMOnInDeepSleep =
        (PWRCTRL->MEMRETCFG_b.NVMPWDSLP == PWRCTRL_MEMRETCFG_NVMPWDSLP_DIS);

    //
    // Configure the MCU Tightly Coupled Memory retention.
    //
    if (PWRCTRL->MEMRETCFG_b.DTCMPWDSLP == PWRCTRL_MEMRETCFG_DTCMPWDSLP_ALL)
    {
        psConfig->eRetainDTCM = AM_HAL_PWRCTRL_ITCM_DTCM_NONE;
    }
    else if (PWRCTRL->MEMRETCFG_b.DTCMPWDSLP == PWRCTRL_MEMRETCFG_DTCMPWDSLP_ALLBUTGROUP0DTCM0)
    {
        psConfig->eRetainDTCM = AM_HAL_PWRCTRL_ITCM32K_DTCM128K;
    }
    else if (PWRCTRL->MEMRETCFG_b.DTCMPWDSLP == PWRCTRL_MEMRETCFG_DTCMPWDSLP_GROUP1)
    {
        psConfig->eRetainDTCM = AM_HAL_PWRCTRL_ITCM128K_DTCM256K;
    }
    else if (PWRCTRL->MEMRETCFG_b.DTCMPWDSLP == PWRCTRL_MEMRETCFG_DTCMPWDSLP_NONE)
    {
        psConfig->eRetainDTCM = AM_HAL_PWRCTRL_ITCM256K_DTCM512K;
    }
    else
    {
        return AM_HAL_STATUS_OUT_OF_RANGE;
    }

    return AM_HAL_STATUS_SUCCESS;
} // am_hal_pwrctrl_mcu_memory_config_get()

// ****************************************************************************
//
//  am_hal_pwrctrl_rom_enable()
//  Enable the ROM power domain if AUTO mode is selected.
//
// ****************************************************************************
uint32_t
am_hal_pwrctrl_rom_enable(void)
{
    uint32_t    count;

    if ( AM_HAL_PWRCTRL_ROM_AUTO == g_eCurROMPwrMode )
    {
        PWRCTRL->MEMPWREN_b.PWRENROM = PWRCTRL_MEMPWREN_PWRENROM_EN;

        //
        // Wait for Status
        //
// #### INTERNAL BEGIN ####
        // TODO - We cannot call am_hal_delay_us_status_check now as it will also call the BootROM power enable/disable
        //        functions as they are no reentrant.
        //ui32Status = am_hal_delay_us_status_check(AM_HAL_PWRCTRL_MAX_WAIT_US,
        //                                          (uint32_t)&PWRCTRL->MEMPWRSTATUS,
        //                                          AM_HAL_PWRCTRL_MEMPWRSTATUS_MASK,
        // TODO - Need to calculate expected power status due to misalignment of MEMPWREN and MEMPWRSTATUS fields.
        //                                          PWRCTRL->MEMPWREN,
        //                                          true);
#warning "TODO - Fixme!!! Replace this with TCM based delay function when it is ready."
// #### INTERNAL END ####
        for (count = 0; count < AM_HAL_PWRCTRL_MAX_BOOTROM_COUNT; count++)
        {
            if (PWRCTRL->MEMPWRSTATUS_b.PWRSTROM == 1)
            {
                break;
            }
        }

        //
        // Check for timeout.
        //
        if (count == AM_HAL_PWRCTRL_MAX_BOOTROM_COUNT)
        {
            return AM_HAL_STATUS_TIMEOUT;
        }
    }

    return AM_HAL_STATUS_SUCCESS;
} // am_hal_pwrctrl_rom_enable()

// ****************************************************************************
//
//  am_hal_pwrctrl_rom_disable()
//  Enable the ROM power domain if AUTO mode is selected.
//
// ****************************************************************************
uint32_t
am_hal_pwrctrl_rom_disable(void)
{
    uint32_t    ui32Count;

    if ( AM_HAL_PWRCTRL_ROM_AUTO == g_eCurROMPwrMode )
    {
        PWRCTRL->MEMPWREN_b.PWRENROM = PWRCTRL_MEMPWREN_PWRENROM_DIS;

        //
        // Wait for Status
        //
// #### INTERNAL BEGIN ####
        // TODO - We cannot call am_hal_delay_us_status_check now as it will also call the BootROM power enable/disable
        //        functions as they are no reentrant.
        //ui32Status = am_hal_delay_us_status_check(AM_HAL_PWRCTRL_MAX_WAIT_US,
        //                                          (uint32_t)&PWRCTRL->MEMPWRSTATUS,
        //                                          AM_HAL_PWRCTRL_MEMPWRSTATUS_MASK,
        // TODO - Need to calculate expected power status due to misalignment of MEMPWREN and MEMPWRSTATUS fields.
        //                                          PWRCTRL->MEMPWREN,
        //                                          true);
#warning "TODO - Fixme!!! Replace this with TCM based delay function when it is ready."
// #### INTERNAL END ####
        for (ui32Count = 0; ui32Count < AM_HAL_PWRCTRL_MAX_BOOTROM_COUNT; ui32Count++)
        {
            if (PWRCTRL->MEMPWRSTATUS_b.PWRSTROM == 0)
            {
                break;
            }
        }

        //
        // Check for timeout.
        //
        if (ui32Count == AM_HAL_PWRCTRL_MAX_BOOTROM_COUNT)
        {
            return AM_HAL_STATUS_TIMEOUT;
        }
    }

    return AM_HAL_STATUS_SUCCESS;
} // am_hal_pwrctrl_rom_disable()

// ****************************************************************************
//
//  am_hal_pwrctrl_sram_config()
//  Configure the Shared RAM.
//
// ****************************************************************************
uint32_t
am_hal_pwrctrl_sram_config(am_hal_pwrctrl_sram_memcfg_t *psConfig)
{
    uint32_t      ui32Status;

    //
    // Configure the Shared RAM.
    //
    PWRCTRL->SSRAMPWREN_b.PWRENSSRAM = psConfig->eSRAMCfg;

    DIAG_SUPPRESS_VOLATILE_ORDER()

    //
    // Wait for Status
    //
    ui32Status = am_hal_delay_us_status_check(AM_HAL_PWRCTRL_MAX_WAIT_US,
                                              (uint32_t)&PWRCTRL->SSRAMPWRST,
                                              PWRCTRL_SSRAMPWRST_SSRAMPWRST_Msk,
                                              PWRCTRL->SSRAMPWREN,
                                              true);

    //
    // Check for error.
    //
    if (AM_HAL_STATUS_SUCCESS != ui32Status)
    {
        return ui32Status;
    }

    //
    // Check the Shared RAM power mode status.
    //
    if (PWRCTRL->SSRAMPWRST_b.SSRAMPWRST != PWRCTRL->SSRAMPWREN_b.PWRENSSRAM)
    {
        return AM_HAL_STATUS_FAIL;
    }
    DIAG_DEFAULT_VOLATILE_ORDER()

    //
    // Configure the Shared RAM domain active based on the states of the MCU,
    // graphics, and display.
    //
    PWRCTRL->SSRAMRETCFG_b.SSRAMACTMCU  = psConfig->eActiveWithMCU;
    PWRCTRL->SSRAMRETCFG_b.SSRAMACTGFX  = psConfig->eActiveWithGFX;
    PWRCTRL->SSRAMRETCFG_b.SSRAMACTDISP = psConfig->eActiveWithDISP;

    //
    // Configure the Shared RAM retention.
    //
    switch ( psConfig->eSRAMRetain )
    {
        case AM_HAL_PWRCTRL_SRAM_NONE:
            PWRCTRL->SSRAMRETCFG_b.SSRAMPWDSLP = PWRCTRL_SSRAMRETCFG_SSRAMPWDSLP_ALL;
            break;
        case AM_HAL_PWRCTRL_SRAM_1M:    // Retain lower 1M, pwr dwn upper 2M
            PWRCTRL->SSRAMRETCFG_b.SSRAMPWDSLP = PWRCTRL_SSRAMRETCFG_SSRAMPWDSLP_GROUP1 |
                                                 PWRCTRL_SSRAMRETCFG_SSRAMPWDSLP_GROUP2;
            break;
        case AM_HAL_PWRCTRL_SRAM_2M:    // Retain lower and middle 1M, pwr dwn upper 1M
            PWRCTRL->SSRAMRETCFG_b.SSRAMPWDSLP = PWRCTRL_SSRAMRETCFG_SSRAMPWDSLP_GROUP2;
            break;
        case AM_HAL_PWRCTRL_SRAM_3M:    // Retain all SSRAM, pwr dwn none
            PWRCTRL->SSRAMRETCFG_b.SSRAMPWDSLP = PWRCTRL_SSRAMRETCFG_SSRAMPWDSLP_NONE;
            break;
    }

    return AM_HAL_STATUS_SUCCESS;
} // am_hal_pwrctrl_sram_config()

// ****************************************************************************
//
//  am_hal_pwrctrl_sram_config_get()
//  Get the current Shared RAM configuration.
//
// ****************************************************************************
uint32_t
am_hal_pwrctrl_sram_config_get(am_hal_pwrctrl_sram_memcfg_t *psConfig)
{
    //
    // Get the Shared RAM configuration.
    //
    psConfig->eSRAMCfg = (am_hal_pwrctrl_sram_select_e)PWRCTRL->SSRAMPWREN_b.PWRENSSRAM;

    //
    // Get the SRAM active configurations based for each of MCU, graphics, and display.
    //
    psConfig->eActiveWithMCU  = (am_hal_pwrctrl_sram_select_e)PWRCTRL->SSRAMRETCFG_b.SSRAMACTMCU;
    psConfig->eActiveWithGFX  = (am_hal_pwrctrl_sram_select_e)PWRCTRL->SSRAMRETCFG_b.SSRAMACTGFX;
    psConfig->eActiveWithDISP = (am_hal_pwrctrl_sram_select_e)PWRCTRL->SSRAMRETCFG_b.SSRAMACTDISP;

    //uint32_t val = (PWRCTRL_SSRAMRETCFG_SSRAMPWDSLP_GROUP1 || PWRCTRL_SSRAMRETCFG_SSRAMPWDSLP_GROUP2);
    //
    // Get the SRAM retention configuration.
    //
    if (PWRCTRL->SSRAMRETCFG_b.SSRAMPWDSLP == PWRCTRL_SSRAMRETCFG_SSRAMPWDSLP_ALL)
    {
        psConfig->eSRAMRetain = AM_HAL_PWRCTRL_SRAM_NONE;
    }
    else if (PWRCTRL->SSRAMRETCFG_b.SSRAMPWDSLP == PWRCTRL_SSRAMRETCFG_SSRAMPWDSLP_NONE)
    {
        psConfig->eSRAMRetain = AM_HAL_PWRCTRL_SRAM_3M;
    }
    else if (PWRCTRL->SSRAMRETCFG_b.SSRAMPWDSLP == PWRCTRL_SSRAMRETCFG_SSRAMPWDSLP_GROUP2)
    {
        psConfig->eSRAMRetain = AM_HAL_PWRCTRL_SRAM_2M;
    }
    else if (PWRCTRL->SSRAMRETCFG_b.SSRAMPWDSLP == (PWRCTRL_SSRAMRETCFG_SSRAMPWDSLP_GROUP1 | PWRCTRL_SSRAMRETCFG_SSRAMPWDSLP_GROUP2))
    {
        psConfig->eSRAMRetain = AM_HAL_PWRCTRL_SRAM_1M;
    }
    else
    {
        return AM_HAL_STATUS_OUT_OF_RANGE;
    }

    return AM_HAL_STATUS_SUCCESS;
} // am_hal_pwrctrl_sram_config_get()

//******************************************************************************
//
// Function that waits for crypto peripheral to stabilize before or after
// powerup/powerdown.
//
//******************************************************************************
#define CRYPTO_WAIT_USEC        100
static uint32_t
crypto_quiesce(void)
{
    uint32_t ui32Status;

    //
    // Wait for crypto block idle.
    //
    ui32Status = am_hal_delay_us_status_change(CRYPTO_WAIT_USEC,
                                                (uint32_t)&CRYPTO->HOSTCCISIDLE,
                                                CRYPTO_HOSTCCISIDLE_HOSTCCISIDLE_Msk,
                                                CRYPTO_HOSTCCISIDLE_HOSTCCISIDLE_Msk);
    if (AM_HAL_STATUS_SUCCESS != ui32Status)
    {
        return ui32Status;
    }

    //
    // Wait for OTP idle.
    //
    ui32Status = am_hal_delay_us_status_change(CRYPTO_WAIT_USEC,
                                               (uint32_t)&CRYPTO->NVMISIDLE,
                                               CRYPTO_NVMISIDLE_NVMISIDLEREG_Msk,
                                               CRYPTO_NVMISIDLE_NVMISIDLEREG_Msk);
    if (AM_HAL_STATUS_SUCCESS != ui32Status)
    {
        return ui32Status;
    }

    //
    // Alert the CRYPTO block of imminent power down.
    //
    CRYPTO->HOSTPOWERDOWN_b.HOSTPOWERDOWN = 1;

    return AM_HAL_STATUS_SUCCESS;
} // crypto_quiesce()

// ****************************************************************************
//
//  am_hal_pwrctrl_periph_enable()
//  Enable power for a peripheral.
//
// ****************************************************************************
uint32_t
am_hal_pwrctrl_periph_enable(am_hal_pwrctrl_periph_e ePeripheral)
{
    uint32_t      ui32Status;
    struct am_pwr_s pwr_ctrl;

    ui32Status = am_get_pwrctrl(&pwr_ctrl, ePeripheral);

    if ( AM_HAL_STATUS_SUCCESS != ui32Status )
    {
        return ui32Status;
    }

    if ( AM_REGVAL(pwr_ctrl.ui32PwrEnRegAddr) & pwr_ctrl.ui32PeriphEnable )
    {
        //
        // We're already enabled, nothing to do.
        //
        return AM_HAL_STATUS_SUCCESS;
    }

    //
    // If INFOC (OTP or NVM0) was not powered on, it is not allowed to power on CRYPTO.
    //
    if (ePeripheral == AM_HAL_PWRCTRL_PERIPH_CRYPTO)
    {
        if (((MCUCTRL->SHADOWVALID_b.INFOCSELOTP == MCUCTRL_SHADOWVALID_INFOCSELOTP_VALID)   &&
             (PWRCTRL->MEMPWRSTATUS_b.PWRSTOTP == 0))                                        ||
            ((MCUCTRL->SHADOWVALID_b.INFOCSELOTP == MCUCTRL_SHADOWVALID_INFOCSELOTP_DEFAULT) &&
             (PWRCTRL->MEMPWRSTATUS_b.PWRSTNVM0 == 0)))
        {
            return AM_HAL_STATUS_FAIL;
        }
    }

    //
    // Switch GPU to the previous mode or desired mode
    //
    AM_CRITICAL_BEGIN
    if (ePeripheral == AM_HAL_PWRCTRL_PERIPH_GFX)
    {
        if (g_ePreGpuPwrMode == AM_HAL_PWRCTRL_GPU_MODE_HIGH_PERFORMANCE)
        {
            am_hal_pwrctrl_gpu_mode_select(AM_HAL_PWRCTRL_GPU_MODE_HIGH_PERFORMANCE);
        }
    }

    //
    // Enable power control for the given device.
    //
    AM_REGVAL(pwr_ctrl.ui32PwrEnRegAddr) |= pwr_ctrl.ui32PeriphEnable;
    AM_CRITICAL_END

    ui32Status = am_hal_delay_us_status_check(AM_HAL_PWRCTRL_MAX_WAIT_US,
                                              pwr_ctrl.ui32PwrStatReqAddr,
                                              pwr_ctrl.ui32PeriphStatus,
                                              pwr_ctrl.ui32PeriphStatus,
                                              true);

    //
    // Check for timeout.
    //
    if (AM_HAL_STATUS_SUCCESS != ui32Status)
    {
        return ui32Status;
    }

    //
    // Crypto peripheral needs more time to power up after the normal status bit
    // is set. We'll wait for the IDLE signal from the NVM as our signal that
    // crypto is ready.
    //
    if (ePeripheral == AM_HAL_PWRCTRL_PERIPH_CRYPTO)
    {
        ui32Status = am_hal_delay_us_status_change(CRYPTO_WAIT_USEC,
                                                   (uint32_t)&CRYPTO->NVMISIDLE,
                                                   CRYPTO_NVMISIDLE_NVMISIDLEREG_Msk,
                                                   CRYPTO_NVMISIDLE_NVMISIDLEREG_Msk);

        if (AM_HAL_STATUS_SUCCESS != ui32Status)
        {
            return ui32Status;
        }
    }

    if ( ePeripheral == AM_HAL_PWRCTRL_PERIPH_OTP)
    {
        // #### INTERNAL BEGIN ####
        // CAYNSWS-2001 2/26/24 It was found that after an OTP power up, additional
        // delay was required before a reliable read from OTP could occur.
        // Experimentation with Keil6 compiled HAL and using hello_world's Trim
        // Rev print showed that 45us was not enough delay, but that 50us worked.
        // For the time-being, at least for SDK6, we'll double that to 100us.
        // Note that the 100us delay was agreed to in a Teams chat between
        // Ron, Jayesh, and Eric Mao.
        // #### INTERNAL END ####
        //
        // Perform a short delay after OTP power up.
        //
        am_hal_delay_us(100);
    }

    //
    // Check the device status.
    //
    if ( (AM_REGVAL(pwr_ctrl.ui32PwrStatReqAddr) &
          pwr_ctrl.ui32PeriphStatus) != 0)
    {
        // #### INTERNAL BEGIN ####
        // This is a SW workaround for CAY-258 to solve a problem with the MSPI I/O clock control.
        // This is scheduled to be fixed in RevB.
        // #### INTERNAL END ####
        // #### INTERNAL BEGIN ####
        // Added 10us delay after clock enable to workaround CAYNSWS-896.
        // #### INTERNAL END ####
        //
        // For RevA silicon, the MSPI I/O Clock must be enabled manually after power-up.
        //
        switch(ePeripheral)
        {
            case AM_HAL_PWRCTRL_PERIPH_MSPI0:
                CLKGEN->MSPIIOCLKCTRL_b.MSPI0IOCLKEN = CLKGEN_MSPIIOCLKCTRL_MSPI0IOCLKEN_EN;
                am_hal_delay_us(10);
                break;

            case AM_HAL_PWRCTRL_PERIPH_MSPI1:
                CLKGEN->MSPIIOCLKCTRL_b.MSPI1IOCLKEN = CLKGEN_MSPIIOCLKCTRL_MSPI1IOCLKEN_EN;
                am_hal_delay_us(10);
                break;

            case AM_HAL_PWRCTRL_PERIPH_MSPI2:
                CLKGEN->MSPIIOCLKCTRL_b.MSPI2IOCLKEN = CLKGEN_MSPIIOCLKCTRL_MSPI2IOCLKEN_EN;
                am_hal_delay_us(10);
                break;

            case AM_HAL_PWRCTRL_PERIPH_MSPI3:
                CLKGEN->MSPIIOCLKCTRL_b.MSPI3IOCLKEN = CLKGEN_MSPIIOCLKCTRL_MSPI3IOCLKEN_EN;
                am_hal_delay_us(10);
                break;

            default:
                break;
        }
        return AM_HAL_STATUS_SUCCESS;
    }
    else
    {
        return AM_HAL_STATUS_FAIL;
    }

} // am_hal_pwrctrl_periph_enable()

// ****************************************************************************
//
//  am_hal_pwrctrl_periph_disable_msk_check()
//  Function checks the PWRCTRL->DEVPWREN
//
//  The original check of ((PWRCTRL->DEVPWRSTATUS & ui32PeriphStatus) == 0)
//      will fail when more than one enable in the same domain is set and the
//      user tries disable only one.
//
// ****************************************************************************
static uint32_t
pwrctrl_periph_disable_msk_check(am_hal_pwrctrl_periph_e ePeripheral)
{
    uint32_t      ui32Status;
    struct am_pwr_s pwr_ctrl;

    ui32Status = am_get_pwrctrl(&pwr_ctrl, ePeripheral);

    if ( AM_HAL_STATUS_SUCCESS != ui32Status )
    {
        return ui32Status;
    }

    switch (pwr_ctrl.ui32PeriphStatus)
    {
        case (PWRCTRL_HCPA_DEVPWRSTATUS_MASK):
            if (((AM_REGVAL(pwr_ctrl.ui32PwrEnRegAddr) & PWRCTRL_HCPA_DEVPWREN_MASK) != 0) &&
                ((AM_REGVAL(pwr_ctrl.ui32PwrEnRegAddr) & pwr_ctrl.ui32PeriphEnable) == 0))
            {
                ui32Status = AM_HAL_STATUS_SUCCESS;
            }
            break;

        case (PWRCTRL_HCPB_DEVPWRSTATUS_MASK):
            if (((AM_REGVAL(pwr_ctrl.ui32PwrEnRegAddr) & PWRCTRL_HCPB_DEVPWREN_MASK) != 0) &&
                ((AM_REGVAL(pwr_ctrl.ui32PwrEnRegAddr) & pwr_ctrl.ui32PeriphEnable) == 0))
            {
                ui32Status = AM_HAL_STATUS_SUCCESS;
            }
            break;

        case (PWRCTRL_HCPC_DEVPWRSTATUS_MASK):
            if (((AM_REGVAL(pwr_ctrl.ui32PwrEnRegAddr) & PWRCTRL_HCPC_DEVPWREN_MASK) != 0) &&
                ((AM_REGVAL(pwr_ctrl.ui32PwrEnRegAddr) & pwr_ctrl.ui32PeriphEnable) == 0))
            {
                ui32Status = AM_HAL_STATUS_SUCCESS;
            }
            break;

        case (PWRCTRL_MSPI_DEVPWRSTATUS_MASK):
            if (((AM_REGVAL(pwr_ctrl.ui32PwrEnRegAddr) & PWRCTRL_MSPI_DEVPWREN_MASK) != 0) &&
                ((AM_REGVAL(pwr_ctrl.ui32PwrEnRegAddr) & pwr_ctrl.ui32PeriphEnable) == 0))
            {
                ui32Status = AM_HAL_STATUS_SUCCESS;
            }
            break;

        case (PWRCTRL_AUD_DEVPWRSTATUS_MASK):
            if (((AM_REGVAL(pwr_ctrl.ui32PwrEnRegAddr) & PWRCTRL_AUD_DEVPWREN_MASK) != 0) &&
                ((AM_REGVAL(pwr_ctrl.ui32PwrEnRegAddr) & pwr_ctrl.ui32PeriphEnable) == 0))
            {
                ui32Status = AM_HAL_STATUS_SUCCESS;
            }
            break;

        default:
            break;
    }

    return ui32Status;
} // pwrctrl_periph_disable_msk_check()

// ****************************************************************************
//
//  am_hal_pwrctrl_periph_disable()
//  Disable power for a peripheral.
//
// ****************************************************************************
uint32_t
am_hal_pwrctrl_periph_disable(am_hal_pwrctrl_periph_e ePeripheral)
{
    uint32_t      ui32Status;
    struct am_pwr_s pwr_ctrl;

    ui32Status = am_get_pwrctrl(&pwr_ctrl, ePeripheral);

    if ( AM_HAL_STATUS_SUCCESS != ui32Status )
    {
        return ui32Status;
    }

    if ( !(AM_REGVAL(pwr_ctrl.ui32PwrEnRegAddr) & pwr_ctrl.ui32PeriphEnable) )
    {
        //
        // We're already disabled, nothing to do.
        //
        return AM_HAL_STATUS_SUCCESS;
    }

    //
    // The crypto block needs to be idle before it can be shut down. First,
    // we'll check to make sure crypto is actually on and accessible by checking the
    // peripheral ID bits (otherwise the next step would fault). If CRYPTO is
    // on, we poll to make sure NVM is inactive before setting the
    // HOSTPOWERDOWN bit to start the shutdown process.
    //
    if (ePeripheral == AM_HAL_PWRCTRL_PERIPH_CRYPTO)
    {
        if (CRYPTO->PERIPHERALID0 == 0xC0)
        {
            ui32Status = crypto_quiesce();

            if (AM_HAL_STATUS_SUCCESS != ui32Status)
            {
                return ui32Status;
            }

        }
    }

    AM_CRITICAL_BEGIN

    //
    // The peripheral is not AM_HAL_PWRCTRL_PERIPH_CRYPTO.
    // Disable power domain for the given device.
    //
    AM_REGVAL(pwr_ctrl.ui32PwrEnRegAddr) &= ~pwr_ctrl.ui32PeriphEnable;

    //
    // Switch GPU to LP when powering down GPU.
    //
    if (ePeripheral == AM_HAL_PWRCTRL_PERIPH_GFX)
    {
        if (g_eCurGpuPwrMode == AM_HAL_PWRCTRL_GPU_MODE_HIGH_PERFORMANCE)
        {
            am_hal_pwrctrl_gpu_mode_select(AM_HAL_PWRCTRL_GPU_MODE_LOW_POWER);
            g_ePreGpuPwrMode = AM_HAL_PWRCTRL_GPU_MODE_HIGH_PERFORMANCE;
        }
        else
        {
            g_ePreGpuPwrMode = AM_HAL_PWRCTRL_GPU_MODE_LOW_POWER;
        }
    }

    AM_CRITICAL_END

    //
    //  This check will fail when more than one enable in the same domain
    //      is set and the user tries to disable only one.
    //
    ui32Status = am_hal_delay_us_status_check(AM_HAL_PWRCTRL_MAX_WAIT_US,
                                              pwr_ctrl.ui32PwrStatReqAddr,
                                              pwr_ctrl.ui32PeriphStatus,
                                              pwr_ctrl.ui32PeriphStatus,
                                              false);

    //
    // Check for success.
    //
    if (AM_HAL_STATUS_SUCCESS == ui32Status)
    {
        // #### INTERNAL BEGIN ####
        // This is a SW workaround for CAY-258 to solve a problem with the MSPI I/O clock control.
        // This is scheduled to be fixed in RevB.
        // #### INTERNAL END ####
        //
        // For RevA silicon, the MSPI I/O Clock must be disable manually after power-down.
        //
        switch(ePeripheral)
        {
            case AM_HAL_PWRCTRL_PERIPH_MSPI0:
                CLKGEN->MSPIIOCLKCTRL_b.MSPI0IOCLKEN = CLKGEN_MSPIIOCLKCTRL_MSPI0IOCLKEN_DIS;
                break;

            case AM_HAL_PWRCTRL_PERIPH_MSPI1:
                CLKGEN->MSPIIOCLKCTRL_b.MSPI1IOCLKEN = CLKGEN_MSPIIOCLKCTRL_MSPI1IOCLKEN_DIS;
                break;

            case AM_HAL_PWRCTRL_PERIPH_MSPI2:
                CLKGEN->MSPIIOCLKCTRL_b.MSPI2IOCLKEN = CLKGEN_MSPIIOCLKCTRL_MSPI2IOCLKEN_DIS;
                break;

            case AM_HAL_PWRCTRL_PERIPH_MSPI3:
                CLKGEN->MSPIIOCLKCTRL_b.MSPI3IOCLKEN = CLKGEN_MSPIIOCLKCTRL_MSPI3IOCLKEN_DIS;
                break;

            default:
                break;
        }
        return ui32Status;
    }
    else
    {
        return pwrctrl_periph_disable_msk_check(ePeripheral);
    }

} // am_hal_pwrctrl_periph_disable()

// ****************************************************************************
//
//  am_hal_pwrctrl_periph_enabled()
//  Determine whether a peripheral is currently enabled.
//
// ****************************************************************************
uint32_t
am_hal_pwrctrl_periph_enabled(am_hal_pwrctrl_periph_e ePeripheral,
                              bool *bEnabled)
{
    uint32_t      ui32Status;
    struct am_pwr_s pwr_ctrl;

#ifndef AM_HAL_DISABLE_API_VALIDATION
    if ( bEnabled == NULL )
    {
        return AM_HAL_STATUS_INVALID_ARG;
    }
#endif // AM_HAL_DISABLE_API_VALIDATION

    //
    // Initialize bEnabled to false in case an error is encountered.
    //
    *bEnabled = false;

    ui32Status = am_get_pwrctrl(&pwr_ctrl, ePeripheral);

    if ( AM_HAL_STATUS_SUCCESS != ui32Status )
    {
        return ui32Status;
    }

    *bEnabled = ((AM_REGVAL(pwr_ctrl.ui32PwrStatReqAddr) &
                  pwr_ctrl.ui32PeriphStatus) != 0);

    return AM_HAL_STATUS_SUCCESS;

} // am_hal_pwrctrl_periph_enabled()

// ****************************************************************************
//
//  am_hal_pwrctrl_status_get()
//  Get the current powercontrol status registers.
//
// ****************************************************************************
uint32_t
am_hal_pwrctrl_status_get(am_hal_pwrctrl_status_t *psStatus)
{
    //
    // Device Power ON Status
    //
    psStatus->ui32Device = PWRCTRL->DEVPWRSTATUS;

    //
    // Audio Subsystem ON Status
    //
    psStatus->ui32AudioSS = PWRCTRL->AUDSSPWRSTATUS;

    //
    // MCU Memory Power ON Status
    //
    psStatus->ui32Memory = PWRCTRL->MEMPWRSTATUS;

    //
    // Power ON Status for MCU Core
    //
    psStatus->ui32System = PWRCTRL->SYSPWRSTATUS;

    //
    // Shared SRAM Power ON Status
    //
    psStatus->ui32SSRAM = PWRCTRL->SSRAMPWRST;

    //
    // Voltage Regulators status
    //
    psStatus->ui32VRStatus = PWRCTRL->VRSTATUS;

    //
    // Power Status Register for ADC Block
    //
    psStatus->ui32ADC = PWRCTRL->ADCSTATUS;

    //
    // Power Status Register for audio ADC Block
    //
    psStatus->ui32AudioADC = PWRCTRL->AUDADCSTATUS;

    return AM_HAL_STATUS_SUCCESS;
} // am_hal_pwrctrl_status_get()

// ****************************************************************************
//
//  Collect and store all INFO1 data, after which OTP can be turned off.
//
// ****************************************************************************
static uint32_t
pwrctrl_INFO1_populate(void)
{
    //
    // To optimizize power, we'll capture the INFO1 registers of interest here
    //  and power down OTP. The registers will then always be available.
    //

    uint32_t ui32Status;
    uint32_t info1buf[12];

//
// Helper macros for pwrctrl_INFO1_populate().
//  CHK_OFFSET_DELTA: Helper macro to assure continguousness of registers.
//  RD_INFO1: Macro to call am_hal_info1_read() and check return status.
//
#define CHK_OFFSET_DELTA(offh, offl, n)     STATIC_ASSERT((((offh - offl) / 4) + 1) != n)

#define RD_INFO1(infospace, wdoffset, numwds, pData)                    \
    ui32Status = am_hal_info1_read(infospace, wdoffset, numwds, pData); \
    if ( ui32Status != AM_HAL_STATUS_SUCCESS )                          \
    {                                                                   \
        return ui32Status;                                              \
    }

    if ( (MCUCTRL->SHADOWVALID_b.INFO1SELOTP != MCUCTRL_SHADOWVALID_INFO1SELOTP_VALID)  ||
         (PWRCTRL->MEMPWRSTATUS_b.PWRSTOTP   != 1) )
    {
// #### INTERNAL BEGIN ####
        //
        // It might seem that we should mark the data as invalid here. However,
        // it's possible that somehow, this function was called twice. In that
        // case, we just want to return a fail but leave the data intact.
        //
        // g_sINFO1regs.ui32INFO1GlobalValid = 0;
// #### INTERNAL END ####
        return AM_HAL_STATUS_INVALID_OPERATION;
    }

// #### INTERNAL BEGIN ####
#if 0 // SBLVer is Apollo5b
    CHK_OFFSET_DELTA(AM_REG_INFO1_SBL_VERSION_1_O, AM_REG_INFO1_SBL_VERSION_0_O, 2 );
    RD_INFO1(AM_HAL_INFO_INFOSPACE_MRAM_INFO1,    (AM_REG_INFO1_SBL_VERSION_0_O / 4), 2, &info1buf[0]);
    g_sINFO1regs.ui32SBLVer0                = info1buf[0];
    g_sINFO1regs.ui32SBLVer1                = info1buf[1];
#endif
// #### INTERNAL END ####
    RD_INFO1(AM_HAL_INFO_INFOSPACE_CURRENT_INFO1, (AM_REG_OTP_INFO1_MAINPTR_O / 4), 1, &info1buf[0]);
    g_sINFO1regs.ui32MAINPTR                = info1buf[0];
// #### INTERNAL BEGIN ####
#if 0 // SBLVer is Apollo5b
    RD_INFO1(AM_HAL_INFO_INFOSPACE_CURRENT_INFO1, (AM_REG_OTP_INFO1_SBLOTA_O / 4), 1, &info1buf[0]);
    g_sINFO1regs.ui32SBLOTA                 = info1buf[0];
#endif
// #### INTERNAL END ####

    CHK_OFFSET_DELTA( AM_REG_OTP_INFO1_SOCID7_O, AM_REG_OTP_INFO1_SOCID0_O, 8 );
    RD_INFO1(AM_HAL_INFO_INFOSPACE_OTP_INFO1,     (AM_REG_OTP_INFO1_SOCID0_O / 4), 8, &info1buf[0]);
    g_sINFO1regs.ui32SOCID0                 = info1buf[0];
    g_sINFO1regs.ui32SOCID1                 = info1buf[1];
    g_sINFO1regs.ui32SOCID2                 = info1buf[2];
    g_sINFO1regs.ui32SOCID3                 = info1buf[3];
    g_sINFO1regs.ui32SOCID4                 = info1buf[4];
    g_sINFO1regs.ui32SOCID5                 = info1buf[5];
    g_sINFO1regs.ui32SOCID6                 = info1buf[6];
    g_sINFO1regs.ui32SOCID7                 = info1buf[7];

    CHK_OFFSET_DELTA( AM_REG_OTP_INFO1_TEMP_CAL_ADC_OFFSET_O, AM_REG_OTP_INFO1_TEMP_CAL_ATE_O, 3);
    RD_INFO1(AM_HAL_INFO_INFOSPACE_CURRENT_INFO1, (AM_REG_OTP_INFO1_TEMP_CAL_ATE_O  / 4), 3, &info1buf[0]);
    g_sINFO1regs.ui32TEMP_CAL_ATE           = info1buf[0];
    g_sINFO1regs.ui32TEMP_CAL_MEASURED      = info1buf[1];
    g_sINFO1regs.ui32TEMP_CAL_ADC_OFFSET    = info1buf[2];

    CHK_OFFSET_DELTA(AM_REG_OTP_INFO1_ADC_OFFSET_ERR_O, AM_REG_OTP_INFO1_ADC_GAIN_ERR_O, 2 );
    RD_INFO1(AM_HAL_INFO_INFOSPACE_CURRENT_INFO1, (AM_REG_OTP_INFO1_ADC_GAIN_ERR_O  / 4), 2, &info1buf[0]);
    g_sINFO1regs.ui32ADC_GAIN_ERR           = info1buf[0];
    g_sINFO1regs.ui32ADC_OFFSET_ERR         = info1buf[1];

    CHK_OFFSET_DELTA(AM_REG_OTP_INFO1_AUDADC_B1_HG_INTERCEPT_O, AM_REG_OTP_INFO1_AUDADC_A0_LG_OFFSET_O, 12 );
    RD_INFO1(AM_HAL_INFO_INFOSPACE_CURRENT_INFO1, (AM_REG_OTP_INFO1_AUDADC_A0_LG_OFFSET_O  / 4), 12, &info1buf[0]);
    g_sINFO1regs.ui32AUDADC_A0_LG_OFFSET    = info1buf[0];
    g_sINFO1regs.ui32AUDADC_A0_HG_SLOPE     = info1buf[1];
    g_sINFO1regs.ui32AUDADC_A0_HG_INTERCEPT = info1buf[2];
    g_sINFO1regs.ui32AUDADC_A1_LG_OFFSET    = info1buf[3];
    g_sINFO1regs.ui32AUDADC_A1_HG_SLOPE     = info1buf[4];
    g_sINFO1regs.ui32AUDADC_A1_HG_INTERCEPT = info1buf[5];
    g_sINFO1regs.ui32AUDADC_B0_LG_OFFSET    = info1buf[6];
    g_sINFO1regs.ui32AUDADC_B0_HG_SLOPE     = info1buf[7];
    g_sINFO1regs.ui32AUDADC_B0_HG_INTERCEPT = info1buf[8];
    g_sINFO1regs.ui32AUDADC_B1_LG_OFFSET    = info1buf[9];
    g_sINFO1regs.ui32AUDADC_B1_HG_SLOPE     = info1buf[10];
    g_sINFO1regs.ui32AUDADC_B1_HG_INTERCEPT = info1buf[11];

    //
    // All done, mark the data as valid
    //
    g_sINFO1regs.ui32INFO1GlobalValid = INFO1GLOBALVALID;

    return ui32Status;

} // pwrctrl_INFO1_populate()

// ****************************************************************************
//
//  am_hal_pwrctrl_low_power_init()
//  Initialize the device for low power operation.
//
// ****************************************************************************
uint32_t
am_hal_pwrctrl_low_power_init(void)
{
    uint32_t ui32TrimVer = 0;
    am_hal_gpio_pincfg_t pad165LowPower =
    {
        .GP.cfg_b.uFuncSel         = 0,
        .GP.cfg_b.eGPInput         = AM_HAL_GPIO_PIN_INPUT_ENABLE,
        .GP.cfg_b.eGPRdZero        = AM_HAL_GPIO_PIN_RDZERO_READPIN,
        .GP.cfg_b.eIntDir          = AM_HAL_GPIO_PIN_INTDIR_NONE,
        .GP.cfg_b.eGPOutCfg        = AM_HAL_GPIO_PIN_OUTCFG_DISABLE,
        .GP.cfg_b.eDriveStrength   = AM_HAL_GPIO_PIN_DRIVESTRENGTH_0P1X,
        .GP.cfg_b.ePullup          = AM_HAL_GPIO_PIN_PULLUP_NONE,
        .GP.cfg_b.uNCE             = 0,
        .GP.cfg_b.eCEpol           = 0,
        .GP.cfg_b.uRsvd_0          = 0,
        .GP.cfg_b.ePowerSw         = 0,
        .GP.cfg_b.eForceInputEn    = 0,
        .GP.cfg_b.eForceOutputEn   = 0,
        .GP.cfg_b.uRsvd_1          = 0
    };

    //
    // Set the default EWIC/IWIC configuration.
    //
    MCUCTRL->WICCONTROL =
        _VAL2FLD(MCUCTRL_WICCONTROL_USEIWIC, 1) |
        _VAL2FLD(MCUCTRL_WICCONTROL_DEEPISWIC, 1);

    //  #### INTERNAL BEGIN ####
    //  This SW workaround is from CAYNSWS-576 and for the CAY-995 issue.
    //
    //  Background: There is a RTL bug in pwr_ctrl block in the logic which creates low power request for VDDC voltage regulator.
    //  It considers PD_MCUE_PG (MCUE power domain power gate signal) while generating this low power request, which is not correct.
    //  This low power request generation logic should ignore the state of MCUE power domain.
    //
    //  Workaround: We have a register bit to allow this logic to ignore MCUE power domain state. (REG_PWRCTRL_LEGACYVRLPOVR [21] = IGNOREMCUE)
    //  SW needs to write value 1 to this register bit. This bit needs to be written after the power-up. This will be a one time register write
    //  and it will hold this value since this register is a part of always on domain (pwr_ctrl).
    #warning "TODO - Add Errata number when available"
    #warning "TODO - Consider whether the LEGACYVRLPOVR register should be exposed to customers in the final SDK release."
    #warning "Verify that CAY-995 is fixed in RevB."
    //  To be fixed in RevB.
    //  #### INTERNAL END ####

    //
    // Remove leakage from GP165 (Apollo5a)
    //
    am_hal_gpio_pinconfig(165, pad165LowPower);

    //
    // SW Workaround for Errata ERRTBD.
    // Set the override to ignore MCUE when going into LP mode for RevA.
    //
    if (APOLLO5_A0)
    {
        PWRCTRL->LEGACYVRLPOVR_b.IGNOREMCUE = 1;
    }

    //
    // Turn HFRC2 off by default.
    //
    am_hal_clkgen_control(AM_HAL_CLKGEN_CONTROL_HFRC2_OFF_REQ, false);

    //
    // Set the default memory configuration.
    //
    am_hal_pwrctrl_mcu_memory_config((am_hal_pwrctrl_mcu_memory_config_t *)&g_DefaultMcuMemCfg);
    am_hal_pwrctrl_sram_config((am_hal_pwrctrl_sram_memcfg_t *)&g_DefaultSRAMCfg);

    // #### INTERNAL BEGIN ####
    // CAYNSWS-2671 6/24/24
    //  For Cayenne revB, Crypto must be powered down in order to power down OTP.
    //  For revA and 510L, it's not clear whether Crypto powerdown is required,
    //  but to keep everything in sync, power down Crypto nonetheless.
    // #### INTERNAL END ####
    //
    // For power optimization, power down OTP.
    // Since OTP is being powered down, first power down Crypto.
    //
    // If INFO1 is currently selected as OTP, collect and save INFO1 data
    // for use by HAL functions. This will eliminate the need to have OTP
    // powered. If INFO1 is MRAM, any INFO1 data needed by HAL functions
    // will simply be read directly via am_hal_info1_read().
    //
    pwrctrl_INFO1_populate();

    //
    // Power down Crypto.
    //
    am_hal_pwrctrl_periph_disable(AM_HAL_PWRCTRL_PERIPH_CRYPTO);

    //
    // Power down OTP.
    //
    am_hal_pwrctrl_periph_disable(AM_HAL_PWRCTRL_PERIPH_OTP);


    //
    // Now that we have the INFO1 data saved, power down the OTP
    //
    am_hal_pwrctrl_periph_disable(AM_HAL_PWRCTRL_PERIPH_OTP);

    //
    // Enable clock gate optimizations for Apollo4.
    //
    // #### INTERNAL BEGIN ####
    //  Bits Equivalent to CLKGEN->MISC_b.CLKGENMISCSPARES = 0x3F
    // #### INTERNAL END ####
    CLKGEN->MISC |=
        _VAL2FLD(CLKGEN_MISC_GFXCLKCLKGATEEN, 1)        |   // [19] GFX CLK
        _VAL2FLD(CLKGEN_MISC_GFXAXICLKCLKGATEEN, 1)     |   // [20] GFX AXI CLK
        _VAL2FLD(CLKGEN_MISC_APBDMACPUCLKCLKGATEEN, 1)  |   // [21] APB DMA CPU CLK
        _VAL2FLD(CLKGEN_MISC_ETMTRACECLKCLKGATEEN, 1)   |   // [22] ETM TRACE CLK
        _VAL2FLD(CLKGEN_MISC_HFRCFUNCCLKGATEEN, 1);         // [23] HFRC_FUNC_CLK

    CLKGEN->CLKCTRL = 0x70; // Disable all unneccesary clocks including display controller clock
    //
    //  Enable the I-Cache and D-Cache.
    //
    // #### INTERNAL BEGIN ####
    #warning Move out cache enabling from here to respective applications.
    // #### INTERNAL END ####
    am_hal_cachectrl_icache_enable();
    am_hal_cachectrl_dcache_enable(true);

    //
    // Additional required settings
    //
    // #### INTERNAL BEGIN ####
    // HSP20-429 PWRONCLKENDISP Needed for DC workaround, disables revB clock enable
    //           during reset, basically reverting to revA behavior.
    // #### INTERNAL END ####
    CLKGEN->MISC_b.PWRONCLKENDISP = 1;

    // #### INTERNAL BEGIN ####
    // FALCSW-176.
    // #### INTERNAL END ####
    // Increases the reference recovery time between scans in LPMODE1 from 5us to 10us.
    MCUCTRL->AUDADCPWRDLY_b.AUDADCPWR1 = 4;

    //
    // Store the factory values for various trims.
    //
    if ( g_bOrigTrimsStored == false )
    {
// #### INTERNAL BEGIN ####
#warning "Fixme!!! Need to update to reflect the changes in MCUCTRL register fields."
// #### INTERNAL END ####
        g_orig_ACTTRIMVDDF          = MCUCTRL->SIMOBUCK12_b.ACTTRIMVDDF;
        g_orig_MEMLDOACTIVETRIM     = MCUCTRL->LDOREG2_b.MEMLDOACTIVETRIM;
        g_orig_LPTRIMVDDF           = MCUCTRL->SIMOBUCK12_b.LPTRIMVDDF;
        g_orig_MEMLPLDOTRIM         = MCUCTRL->LDOREG2_b.MEMLPLDOTRIM;
        g_orig_TVRGVREFTRIM         = MCUCTRL->VREFGEN2_b.TVRGCVREFTRIM;
        g_bOrigTrimsStored          = true;
    }

    //
    // Store the original CORELDOTEMPCOTRIM
    //
    g_orig_CORELDOTEMPCOTRIM = MCUCTRL->LDOREG1_b.CORELDOTEMPCOTRIM;

    //
    // Store the original MEMLDOACTIVETRIM
    //
    g_orig_MEMLDOTRIM = MCUCTRL->LDOREG2_b.MEMLDOACTIVETRIM;

    //
    // Store the original D2ASPARE
    //
    g_orig_D2ASPARE = MCUCTRL->D2ASPARE;

    //
    // Store the original TVRGFVREFTRIM
    //
    vddf_simobuck_trim_get(&g_orig_TVRGFVREFTRIM);

    //
    // Store the original TVRGFTEMPCOTRIM
    //
    vddf_simobuck_trim_get_tvrgf_tempco(&g_orig_TVRGFTEMPCOTRIM);

    //
    // Store the original VDDCACTLOWTONTRIM
    //
    g_orig_VDDCACTLOWTONTRIM = MCUCTRL->SIMOBUCK2_b.VDDCACTLOWTONTRIM;

    //
    // Store the original VDDFACTLOWTONTRIM
    //
    g_orig_VDDFACTLOWTONTRIM = MCUCTRL->SIMOBUCK7_b.VDDFACTLOWTONTRIM;
    //
    // Set SSRAM/DTCM/NVM retain comes from VDDS
    //
    MCUCTRL->PWRSW0_b.PWRSWVDDRCPUSTATSEL = 0x1;
    MCUCTRL->PWRSW1_b.PWRSWVDDRMSTATSEL   = 0x1;
    MCUCTRL->PWRSW1_b.PWRSWVDDRLSTATSEL   = 0x1;
    //
    // Get trim version
    //
    TrimVersionGet(&ui32TrimVer);

    //
    //  Set ITCM DTCM CACHE trims
    //
    if (g_ui32TrimVer == 6)
    {
        tcm_cache_trim_set();
    }

    //
    // Selection of default HP mode clock for CPU.
    //
    if ( (g_ui32TrimVer == 0xFFFFFFFF) || (g_ui32TrimVer <= MIN_PCM_TRIM_VER) )
    {
        am_hal_clkgen_control(AM_HAL_CLKGEN_MISC_CPUHPFREQSEL_HP192M, NULL);
    }
    else
    {
        am_hal_clkgen_control(AM_HAL_CLKGEN_MISC_CPUHPFREQSEL_HP250M, NULL);
    }

    return AM_HAL_STATUS_SUCCESS;

} // am_hal_pwrctrl_low_power_init()

void
buck_ldo_override_init(void)
{
    //
    // Force SIMOBUCK into active mode. SIMOBUCKOVER must be set last.
    // This override, even though enabled - is not effective till we go to DeepSleep
    //
    MCUCTRL->VRCTRL_b.SIMOBUCKPDNB   = 1;
    MCUCTRL->VRCTRL_b.SIMOBUCKRSTB   = 1;
    MCUCTRL->VRCTRL_b.SIMOBUCKACTIVE = 1;
    MCUCTRL->VRCTRL_b.SIMOBUCKOVER   = 1;

#if AM_HAL_PWRCTL_SET_CORELDO_MEMLDO_IN_PARALLEL
    //
    // Force LDOs into active mode and to run in parallel with SIMO.
    //
    //
    // Core LDO. Set CORELDOOVER last
    //
    MCUCTRL->VRCTRL_b.CORELDOCOLDSTARTEN  = 0;
    MCUCTRL->VRCTRL |=
        MCUCTRL_VRCTRL_CORELDOACTIVE_Msk        |
        MCUCTRL_VRCTRL_CORELDOACTIVEEARLY_Msk   |
        MCUCTRL_VRCTRL_CORELDOPDNB_Msk;
    MCUCTRL->VRCTRL_b.CORELDOOVER         = 1;
    //
    // Mem LDO. Set MEMLDOOVER last
    //
    MCUCTRL->VRCTRL_b.MEMLDOCOLDSTARTEN   = 0;
    MCUCTRL->VRCTRL |=
        MCUCTRL_VRCTRL_MEMLDOACTIVE_Msk         |
        MCUCTRL_VRCTRL_MEMLDOACTIVEEARLY_Msk    |
        MCUCTRL_VRCTRL_MEMLDOPDNB_Msk;
    MCUCTRL->VRCTRL_b.MEMLDOOVER          = 1;
#endif // AM_HAL_PWRCTL_SET_CORELDO_MEMLDO_IN_PARALLEL
} // buck_ldo_override_init()

// #### INTERNAL BEGIN ####
#if 0 // Not needed with latest updates
#if AM_HAL_PWRCTL_SET_CORELDO_MEMLDO_IN_PARALLEL
// ****************************************************************************
//
//  coreldo_enable()
//  Function to set CORELDOTEMPCOTRIM and force CORELDO to be active
//
// ****************************************************************************
void
coreldo_enable(void)
{
    //
    // Enable coreldo
    // There are timing constraints involved in getting this done,
    // so optimize as much as possible.
    //
// #### INTERNAL2 BEGIN ####
    // 10/29/21 Ivan and Jamie: coldstarten is used to limit inrush current of
    // the LDO on initial power up. Technically it's not needed when used for
    // backup / running in parallel, but we should set the same as rev B.
// #### INTERNAL2 END ####
    MCUCTRL->VRCTRL_b.CORELDOCOLDSTARTEN  = 0;
    MCUCTRL->VRCTRL |=
        MCUCTRL_VRCTRL_CORELDOACTIVE_Msk        |
        MCUCTRL_VRCTRL_CORELDOACTIVEEARLY_Msk   |
        MCUCTRL_VRCTRL_CORELDOPDNB_Msk;
    MCUCTRL->VRCTRL_b.CORELDOOVER         = 1;
} // coreldo_enable()

// ****************************************************************************
//
//  coreldo_disable()
//  Function to turn off coreldo
//
// ****************************************************************************
void
coreldo_disable(void)
{
    //
    // Disable coreldo
    //
    MCUCTRL->VRCTRL_b.CORELDOOVER         = 0;
    MCUCTRL->VRCTRL &=
        ~(MCUCTRL_VRCTRL_CORELDOACTIVE_Msk      |
          MCUCTRL_VRCTRL_CORELDOACTIVEEARLY_Msk |
          MCUCTRL_VRCTRL_CORELDOPDNB_Msk);
} // coreldo_disable()

// ****************************************************************************
//
//  memldo_enable()
//  Function to force MEMLDO to be active
//
// ****************************************************************************
void
memldo_enable(void)
{
    //
    // Enable memldo
    // There are timing constraints involved in getting this done,
    // so optimize as much as possible.
    // However, MEMLDOOVER must be the last bit set.
    //
// #### INTERNAL2 BEGIN ####
    // 10/29/21 See "coldstarten" comment above.
// #### INTERNAL2 END ####
    MCUCTRL->VRCTRL_b.MEMLDOCOLDSTARTEN   = 0;
    MCUCTRL->VRCTRL |=
        MCUCTRL_VRCTRL_MEMLDOACTIVE_Msk         |
        MCUCTRL_VRCTRL_MEMLDOACTIVEEARLY_Msk    |
        MCUCTRL_VRCTRL_MEMLDOPDNB_Msk;
    MCUCTRL->VRCTRL_b.MEMLDOOVER          = 1;
} // memldo_enable()

// ****************************************************************************
//
//  memldo_disable()
//  Function to turn off memldo
//
// ****************************************************************************
void
memldo_disable(void)
{
    //
    // disable memldo
    //
    MCUCTRL->VRCTRL_b.MEMLDOOVER          = 0;
    MCUCTRL->VRCTRL &=
        ~(MCUCTRL_VRCTRL_MEMLDOACTIVE_Msk         |
          MCUCTRL_VRCTRL_MEMLDOACTIVEEARLY_Msk    |
          MCUCTRL_VRCTRL_MEMLDOPDNB_Msk);
} // memldo_disable()
#endif // AM_HAL_PWRCTL_SET_CORELDO_MEMLDO_IN_PARALLEL

void
simobuck_ldos_force_active(void)
{
    //
    // Force SIMOBUCK into active mode. SIMOBUCKOVER must be set last.
    //
    MCUCTRL->VRCTRL_b.SIMOBUCKPDNB   = 1;
    MCUCTRL->VRCTRL_b.SIMOBUCKRSTB   = 1;
    MCUCTRL->VRCTRL_b.SIMOBUCKACTIVE = 1;
    MCUCTRL->VRCTRL_b.SIMOBUCKOVER   = 1;

#if AM_HAL_PWRCTL_SET_CORELDO_MEMLDO_IN_PARALLEL
    //
    // Also force LDOs to active in deepsleep
    //
    MCUCTRL->VRCTRL_b.CORELDOPDNB    = 1;
    MCUCTRL->VRCTRL_b.CORELDOACTIVE  = 1;
    MCUCTRL->VRCTRL_b.CORELDOOVER    = 1;
    MCUCTRL->VRCTRL_b.MEMLDOPDNB     = 1;
    MCUCTRL->VRCTRL_b.MEMLDOACTIVE   = 1;
    MCUCTRL->VRCTRL_b.MEMLDOOVER     = 1;
#endif // AM_HAL_PWRCTL_SET_CORELDO_MEMLDO_IN_PARALLEL
} // simobuck_ldos_force_active()

void
simobuck_ldos_force_disable(void)
{
    MCUCTRL->VRCTRL_b.SIMOBUCKOVER   = 0;
    MCUCTRL->VRCTRL_b.SIMOBUCKACTIVE = 0;
    MCUCTRL->VRCTRL_b.SIMOBUCKRSTB   = 0;
    MCUCTRL->VRCTRL_b.SIMOBUCKPDNB   = 0;

#if AM_HAL_PWRCTL_SET_CORELDO_MEMLDO_IN_PARALLEL
    MCUCTRL->VRCTRL_b.CORELDOOVER    = 0;
    MCUCTRL->VRCTRL_b.MEMLDOOVER     = 0;
    MCUCTRL->VRCTRL_b.CORELDOACTIVE  = 0;
    MCUCTRL->VRCTRL_b.CORELDOPDNB    = 0;
    MCUCTRL->VRCTRL_b.MEMLDOACTIVE   = 0;
    MCUCTRL->VRCTRL_b.MEMLDOPDNB     = 0;
#endif // AM_HAL_PWRCTL_SET_CORELDO_MEMLDO_IN_PARALLEL
} // simobuck_ldos_force_disable()
#endif // if 0
// #### INTERNAL END ####

// Dynamically turn on and off the overrides for buck and LDO
// Override configs are already set once in buck_ldo_override_init
void
buck_ldo_update_override(bool bEnable)
{
    MCUCTRL->VRCTRL_b.SIMOBUCKOVER   = bEnable;
#if AM_HAL_PWRCTL_SET_CORELDO_MEMLDO_IN_PARALLEL
    MCUCTRL->VRCTRL_b.CORELDOOVER    = bEnable;
    MCUCTRL->VRCTRL_b.MEMLDOOVER     = bEnable;
#endif // AM_HAL_PWRCTL_SET_CORELDO_MEMLDO_IN_PARALLEL
} // buck_ldo_update_override()

// ****************************************************************************
//
//  am_hal_pwrctrl_control()
//  Additional miscellaneous power controls.
//
// ****************************************************************************
uint32_t
am_hal_pwrctrl_control(am_hal_pwrctrl_control_e eControl, void *pArgs)
{
    uint32_t ui32ReturnStatus = AM_HAL_STATUS_SUCCESS;

    switch ( eControl )
    {
        case AM_HAL_PWRCTRL_CONTROL_SIMOBUCK_INIT:
            {
                //
                // Set Set VDDC active low and high TON trim.
                //
                MCUCTRL->VRCTRL_b.SIMOBUCKPDNB   = 1;
                MCUCTRL->VRCTRL_b.SIMOBUCKRSTB   = 1;
                MCUCTRL->VRCTRL_b.SIMOBUCKACTIVE = 1;
                MCUCTRL->VRCTRL_b.SIMOBUCKOVER   = 1;

                //
                // Force LDOs into active mode and to run in parallel with SIMO.
                // Coreldo override
                //
                MCUCTRL->VRCTRL_b.CORELDOCOLDSTARTEN  = 0;
                MCUCTRL->VRCTRL |=
                    MCUCTRL_VRCTRL_CORELDOACTIVE_Msk        |
                    MCUCTRL_VRCTRL_CORELDOACTIVEEARLY_Msk   |
                    MCUCTRL_VRCTRL_CORELDOPDNB_Msk;
                MCUCTRL->VRCTRL_b.CORELDOOVER         = 1;

                //
                // Memldo override
                //
                MCUCTRL->VRCTRL_b.MEMLDOCOLDSTARTEN   = 0;
                MCUCTRL->VRCTRL |=
                    MCUCTRL_VRCTRL_MEMLDOACTIVE_Msk         |
                    MCUCTRL_VRCTRL_MEMLDOACTIVEEARLY_Msk    |
                    MCUCTRL_VRCTRL_MEMLDOPDNB_Msk;
                MCUCTRL->VRCTRL_b.MEMLDOOVER          = 1;

                MCUCTRL->SIMOBUCK15_b.TRIMLATCHOVER = 1;

                // #### INTERNAL BEGIN ####
                // *(uint32_t*)0x4004033c=0xf; from validation team
                // #### INTERNAL END ####
                //
                // Enable SIMOBUCK compensations
                //
                MCUCTRL->SIMOBUCK0 |=
                    MCUCTRL_SIMOBUCK0_VDDCRXCOMPEN_Msk |
                    MCUCTRL_SIMOBUCK0_VDDFRXCOMPEN_Msk |
                    MCUCTRL_SIMOBUCK0_VDDSRXCOMPEN_Msk |
                    MCUCTRL_SIMOBUCK0_VDDCLVRXCOMPEN_Msk;

                //
                // Enable the SIMOBUCK
                //
                PWRCTRL->VRCTRL_b.SIMOBUCKEN = 1;
            }
            break;

        case AM_HAL_PWRCTRL_CONTROL_CRYPTO_POWERDOWN:
            {
                uint32_t    ui32Status;
                bool        bEnabled;

                //
                // Check if CRYPTO block is powered on.
                //
                bEnabled = false;
                am_hal_pwrctrl_periph_enabled(AM_HAL_PWRCTRL_PERIPH_CRYPTO, &bEnabled);
                if ( bEnabled )
                {
                    //
                    // Power down the crypto block in case it was left on by SBR/SBL.
                    //
                    ui32Status = am_hal_pwrctrl_periph_disable(AM_HAL_PWRCTRL_PERIPH_CRYPTO);
                    if (AM_HAL_STATUS_SUCCESS != ui32Status)
                    {
                        return ui32Status;
                    }
                }
            }
            break;

        case AM_HAL_PWRCTRL_CONTROL_XTAL_PWDN_DEEPSLEEP:
            //
            // This optimization is optional. Enable it IFF the 32KHz crystal is
            // not required during deep sleep. If enabled it will save ~0.8uA.
            //
            MCUCTRL->XTALGENCTRL_b.XTALBIASTRIM   = 0x20;

// #### INTERNAL BEGIN ####
            // TODO FIXME: Is this intended to be a write to the field or the
            //      entire register? The workaround code from validation simply
            //      wrote the value 1 to the register.
            // Answer: Assuming the question is in reference to the following write
            //         to XTALCTRL, it is indeed writing the value 0x1 to the register.
            //         It's just more explicit than a plain, hard-coded value. - REH
// #### INTERNAL END ####
            MCUCTRL->XTALCTRL =
                _VAL2FLD(MCUCTRL_XTALCTRL_XTALICOMPTRIM,  0 )                                       |
                _VAL2FLD(MCUCTRL_XTALCTRL_XTALIBUFTRIM,   0 )                                       |
                _VAL2FLD(MCUCTRL_XTALCTRL_XTALCOMPPDNB,   MCUCTRL_XTALCTRL_XTALCOMPPDNB_PWRDNCOMP ) |
                _VAL2FLD(MCUCTRL_XTALCTRL_XTALPDNB,       MCUCTRL_XTALCTRL_XTALPDNB_PWRDNCORE )     |
                _VAL2FLD(MCUCTRL_XTALCTRL_XTALCOMPBYPASS, MCUCTRL_XTALCTRL_XTALCOMPBYPASS_USECOMP ) |
                _VAL2FLD(MCUCTRL_XTALCTRL_XTALCOREDISFB,  MCUCTRL_XTALCTRL_XTALCOREDISFB_EN )       |
                _VAL2FLD(MCUCTRL_XTALCTRL_XTALSWE,        MCUCTRL_XTALCTRL_XTALSWE_OVERRIDE_EN);
            break;

        case AM_HAL_PWRCTRL_CONTROL_DIS_PERIPHS_ALL:
            PWRCTRL->DEVPWREN =
// #### INTERNAL BEGIN ####
#if 0   // A2SD-2003 deprecate I3C enable/status registers/fields
                _VAL2FLD(PWRCTRL_DEVPWREN_PWRENI3C1,    PWRCTRL_DEVPWREN_PWRENI3C1_DIS)         |
                _VAL2FLD(PWRCTRL_DEVPWREN_PWRENI3C0,    PWRCTRL_DEVPWREN_PWRENI3C0_DIS)         |
#endif
// #### INTERNAL END ####
                _VAL2FLD(PWRCTRL_DEVPWREN_PWRENDBG,     PWRCTRL_DEVPWREN_PWRENDBG_DIS)          |
                _VAL2FLD(PWRCTRL_DEVPWREN_PWRENUSBPHY,  PWRCTRL_DEVPWREN_PWRENUSBPHY_DIS)       |
                _VAL2FLD(PWRCTRL_DEVPWREN_PWRENUSB,     PWRCTRL_DEVPWREN_PWRENUSB_DIS)          |
                _VAL2FLD(PWRCTRL_DEVPWREN_PWRENSDIO1,   PWRCTRL_DEVPWREN_PWRENSDIO1_DIS)        |
                _VAL2FLD(PWRCTRL_DEVPWREN_PWRENSDIO0,   PWRCTRL_DEVPWREN_PWRENSDIO0_DIS)        |
                _VAL2FLD(PWRCTRL_DEVPWREN_PWRENCRYPTO,  PWRCTRL_DEVPWREN_PWRENCRYPTO_DIS)       |
                _VAL2FLD(PWRCTRL_DEVPWREN_PWRENDISPPHY, PWRCTRL_DEVPWREN_PWRENDISPPHY_DIS)      |
                _VAL2FLD(PWRCTRL_DEVPWREN_PWRENDISP,    PWRCTRL_DEVPWREN_PWRENDISP_DIS)         |
                _VAL2FLD(PWRCTRL_DEVPWREN_PWRENGFX,     PWRCTRL_DEVPWREN_PWRENGFX_DIS)          |
                _VAL2FLD(PWRCTRL_DEVPWREN_PWRENMSPI2,   PWRCTRL_DEVPWREN_PWRENMSPI2_DIS)        |
                _VAL2FLD(PWRCTRL_DEVPWREN_PWRENMSPI1,   PWRCTRL_DEVPWREN_PWRENMSPI1_DIS)        |
                _VAL2FLD(PWRCTRL_DEVPWREN_PWRENMSPI0,   PWRCTRL_DEVPWREN_PWRENMSPI0_DIS)        |
                _VAL2FLD(PWRCTRL_DEVPWREN_PWRENADC,     PWRCTRL_DEVPWREN_PWRENADC_DIS)          |
                _VAL2FLD(PWRCTRL_DEVPWREN_PWRENUART3,   PWRCTRL_DEVPWREN_PWRENUART3_DIS)        |
                _VAL2FLD(PWRCTRL_DEVPWREN_PWRENUART2,   PWRCTRL_DEVPWREN_PWRENUART2_DIS)        |
                _VAL2FLD(PWRCTRL_DEVPWREN_PWRENUART1,   PWRCTRL_DEVPWREN_PWRENUART1_DIS)        |
                _VAL2FLD(PWRCTRL_DEVPWREN_PWRENUART0,   PWRCTRL_DEVPWREN_PWRENUART0_DIS)        |
                _VAL2FLD(PWRCTRL_DEVPWREN_PWRENIOM7,    PWRCTRL_DEVPWREN_PWRENIOM7_DIS)         |
                _VAL2FLD(PWRCTRL_DEVPWREN_PWRENIOM6,    PWRCTRL_DEVPWREN_PWRENIOM6_DIS)         |
                _VAL2FLD(PWRCTRL_DEVPWREN_PWRENIOM5,    PWRCTRL_DEVPWREN_PWRENIOM5_DIS)         |
                _VAL2FLD(PWRCTRL_DEVPWREN_PWRENIOM4,    PWRCTRL_DEVPWREN_PWRENIOM4_DIS)         |
                _VAL2FLD(PWRCTRL_DEVPWREN_PWRENIOM3,    PWRCTRL_DEVPWREN_PWRENIOM3_DIS)         |
                _VAL2FLD(PWRCTRL_DEVPWREN_PWRENIOM2,    PWRCTRL_DEVPWREN_PWRENIOM2_DIS)         |
                _VAL2FLD(PWRCTRL_DEVPWREN_PWRENIOM1,    PWRCTRL_DEVPWREN_PWRENIOM1_DIS)         |
                _VAL2FLD(PWRCTRL_DEVPWREN_PWRENIOM0,    PWRCTRL_DEVPWREN_PWRENIOM0_DIS)         |
                _VAL2FLD(PWRCTRL_DEVPWREN_PWRENIOS,     PWRCTRL_DEVPWREN_PWRENIOS_DIS);
// #### INTERNAL BEGIN ####
#if 0 // Probably not needed, no Coremark difference.
            //
            // Update only certain fields of this register.
            //
            MCUCTRL->AUDADCPWRCTRL &=
                ~(MCUCTRL_AUDADCPWRCTRL_AUDADCPWRCTRLSWE_Msk    |
                  MCUCTRL_AUDADCPWRCTRL_VDDAUDADCRESETN_Msk     |
                  MCUCTRL_AUDADCPWRCTRL_VDDAUDADCDIGISOLATE_Msk |
                  MCUCTRL_AUDADCPWRCTRL_VDDAUDADCSARISOLATE_Msk |
                  MCUCTRL_AUDADCPWRCTRL_AUDADCBPSEN_Msk         |
                  MCUCTRL_AUDADCPWRCTRL_AUDADCAPSEN_Msk);
            MCUCTRL->AUDADCPWRCTRL |=
                _VAL2FLD(MCUCTRL_AUDADCPWRCTRL_AUDADCPWRCTRLSWE,    MCUCTRL_AUDADCPWRCTRL_AUDADCPWRCTRLSWE_OVERRIDE_DIS)    |
                _VAL2FLD(MCUCTRL_AUDADCPWRCTRL_VDDAUDADCRESETN,     MCUCTRL_AUDADCPWRCTRL_VDDAUDADCRESETN_ASSERT)           |
                _VAL2FLD(MCUCTRL_AUDADCPWRCTRL_VDDAUDADCDIGISOLATE, MCUCTRL_AUDADCPWRCTRL_VDDAUDADCDIGISOLATE_DIS)          |
                _VAL2FLD(MCUCTRL_AUDADCPWRCTRL_VDDAUDADCSARISOLATE, MCUCTRL_AUDADCPWRCTRL_VDDAUDADCSARISOLATE_DIS)          |
                _VAL2FLD(MCUCTRL_AUDADCPWRCTRL_AUDADCBPSEN,         MCUCTRL_AUDADCPWRCTRL_AUDADCBPSEN_DIS)                  |
                _VAL2FLD(MCUCTRL_AUDADCPWRCTRL_AUDADCAPSEN,         MCUCTRL_AUDADCPWRCTRL_AUDADCAPSEN_DIS);

            //
            // Update only certain fields of this register.
            //
            MCUCTRL->ADCPWRCTRL &=
                ~(MCUCTRL_ADCPWRCTRL_VDDADCRESETN_Msk       |
                  MCUCTRL_ADCPWRCTRL_VDDADCDIGISOLATE_Msk   |
                  MCUCTRL_ADCPWRCTRL_VDDADCSARISOLATE_Msk   |
                  MCUCTRL_ADCPWRCTRL_ADCBPSEN_Msk);
            MCUCTRL->ADCPWRCTRL |=
                _VAL2FLD(MCUCTRL_ADCPWRCTRL_VDDADCRESETN, MCUCTRL_ADCPWRCTRL_VDDADCRESETN_ASSERT)       |
                _VAL2FLD(MCUCTRL_ADCPWRCTRL_VDDADCDIGISOLATE, MCUCTRL_ADCPWRCTRL_VDDADCDIGISOLATE_DIS)  |
                _VAL2FLD(MCUCTRL_ADCPWRCTRL_VDDADCSARISOLATE, MCUCTRL_ADCPWRCTRL_VDDADCSARISOLATE_DIS)  |
                _VAL2FLD(MCUCTRL_ADCPWRCTRL_ADCBPSEN, MCUCTRL_ADCPWRCTRL_ADCBPSEN_DIS);
#endif
#if 0 // Probably not needed, should be handled in the appl.
    //
    // Configure the Shared RAM domain active state settings.
    //
    PWRCTRL->SSRAMRETCFG_b.SSRAMACTDISP = AM_HAL_PWRCTRL_SRAM_NONE;
    PWRCTRL->SSRAMRETCFG_b.SSRAMACTGFX  = AM_HAL_PWRCTRL_SRAM_NONE;
    PWRCTRL->SSRAMRETCFG_b.SSRAMACTDSP  = AM_HAL_PWRCTRL_SRAM_NONE;
    PWRCTRL->SSRAMRETCFG_b.SSRAMACTMCU  = AM_HAL_PWRCTRL_SRAM_NONE;
    PWRCTRL->SSRAMRETCFG_b.SSRAMPWDSLP  = AM_HAL_PWRCTRL_SRAM_1M;
#endif
// #### INTERNAL END ####
            break;

// #### INTERNAL BEGIN ####
#if 0
        case IF_USED_THIS_WOULD_BE_A_NEW_CASE:
            PWRCTRL->MEMPWREN =
                _VAL2FLD(PWRCTRL_MEMPWREN_PWRENCACHEB2, PWRCTRL_AUDSSPWRSTATUS_PWRSTAUDADC_ON)  |
                _VAL2FLD(PWRCTRL_MEMPWREN_PWRENCACHEB0, PWRCTRL_MEMPWREN_PWRENCACHEB0_EN)       |
                _VAL2FLD(PWRCTRL_MEMPWREN_PWRENNVM0, PWRCTRL_MEMPWREN_PWRENNVM0_EN)             |
                _VAL2FLD(PWRCTRL_MEMPWREN_PWRENDTCM, PWRCTRL_MEMPWREN_PWRENDTCM_TCM384K);

            PWRCTRL->SSRAMPWREN =
                _VAL2FLD(PWRCTRL_SSRAMPWREN_PWRENSSRAM, PWRCTRL_SSRAMPWREN_PWRENSSRAM_ALL);

            PWRCTRL->AUDSSPWREN =
                _VAL2FLD(PWRCTRL_AUDSSPWREN_PWRENDSPA,   PWRCTRL_AUDSSPWREN_PWRENDSPA_DIS)      |
                _VAL2FLD(PWRCTRL_AUDSSPWREN_PWRENAUDADC, PWRCTRL_AUDSSPWREN_PWRENAUDADC_DIS)    |
                _VAL2FLD(PWRCTRL_AUDSSPWREN_PWRENI2S1,   PWRCTRL_AUDSSPWREN_PWRENI2S1_DIS)      |
                _VAL2FLD(PWRCTRL_AUDSSPWREN_PWRENI2S0,   PWRCTRL_AUDSSPWREN_PWRENI2S0_DIS)      |
                _VAL2FLD(PWRCTRL_AUDSSPWREN_PWRENPDM3,   PWRCTRL_AUDSSPWREN_PWRENPDM3_DIS)      |
                _VAL2FLD(PWRCTRL_AUDSSPWREN_PWRENPDM2,   PWRCTRL_AUDSSPWREN_PWRENPDM2_DIS)      |
                _VAL2FLD(PWRCTRL_AUDSSPWREN_PWRENPDM1,   PWRCTRL_AUDSSPWREN_PWRENPDM1_DIS)      |
                _VAL2FLD(PWRCTRL_AUDSSPWREN_PWRENPDM0,   PWRCTRL_AUDSSPWREN_PWRENPDM0_DIS)      |
                _VAL2FLD(PWRCTRL_AUDSSPWREN_PWRENAUDPB,  PWRCTRL_AUDSSPWREN_PWRENAUDPB_DIS)     |
                _VAL2FLD(PWRCTRL_AUDSSPWREN_PWRENAUDREC, PWRCTRL_AUDSSPWREN_PWRENAUDREC_DIS);
            break;
#endif
// #### INTERNAL END ####
#if AM_HAL_TEMPCO_LP
        case AM_HAL_PWRCTRL_CONTROL_TEMPCO_GETMEASTEMP:
            if ( pArgs )
            {
                *((float*)pArgs) = g_pfTempMeasured;
            }
#endif // AM_HAL_TEMPCO_LP

        default:
            ui32ReturnStatus = AM_HAL_STATUS_INVALID_ARG;
            break;
    }

    //
    // Return success status.
    //
    return ui32ReturnStatus;

} // am_hal_pwrctrl_control()

// ****************************************************************************
// Function to restore factory trims.
// ****************************************************************************
static void
restore_factory_trims(void)
{
    if ( g_bOrigTrimsStored )
    {
        //
        // Restore the original factory trim values
        //
// #### INTERNAL BEGIN ####
#warning "Fixe me!!! Need to update based on MCUCTRL register field changes."
// #### INTERNAL END ####
        MCUCTRL->SIMOBUCK12_b.ACTTRIMVDDF   = g_orig_ACTTRIMVDDF;
        MCUCTRL->LDOREG2_b.MEMLDOACTIVETRIM = g_orig_MEMLDOACTIVETRIM;
        MCUCTRL->SIMOBUCK12_b.LPTRIMVDDF    = g_orig_LPTRIMVDDF;
        MCUCTRL->LDOREG2_b.MEMLPLDOTRIM     = g_orig_MEMLPLDOTRIM;
        MCUCTRL->VREFGEN2_b.TVRGCVREFTRIM    = g_orig_TVRGVREFTRIM;
    }
} // restore_factory_trims()

//*****************************************************************************
//
// Restore original power settings
//
// This function restores default power trims, reverting relative changes that
// were done as part of am_hal_pwrctrl_low_power_init, SIMOBUCK init, and
// dynamic updates such as are made with Temperature Compensation (TempCo)
// and/or by enabling Crypto.
//
// Important:
// - This function must be called before transition to a new application, such
//   as the case of a secondary bootloader transistioning to an application.
// - If previously enabled, TempCo must be disabled before this function is
//   called.
//
//*****************************************************************************
uint32_t
am_hal_pwrctrl_settings_restore(void)
{
    uint32_t ui32Ret;

// #### INTERNAL BEGIN ####
    // Switch from HP to LP mode (if in HP mode)
    // Switch to LDO mode (if not already in LDO mode)
    // Delay 20us for rails to settle
    // Disable LDOs in parallel (if override was set to enabled this in simobuck mode)
    // Remove VDDS/VDDF short
    // Turn on Crypto if not already on.
// #### INTERNAL END ####
    //
    // Ensure - we're in LP mode
    //
    if (PWRCTRL->MCUPERFREQ_b.MCUPERFREQ != AM_HAL_PWRCTRL_MCU_MODE_LOW_POWER)
    {
        // Device needs to be in LP mode before restore is called
        return AM_HAL_STATUS_INVALID_OPERATION;
    }

    //
    // Need to revert the trim changes, and turn to LDO mode
    //
    AM_CRITICAL_BEGIN

    //
    // Switch to LDO mode (if not already in LDO mode)
    // Delay 20us for rails to settle
    //
    if (PWRCTRL->VRSTATUS_b.SIMOBUCKST == PWRCTRL_VRSTATUS_SIMOBUCKST_ACT)
    {
#if AM_HAL_PWRCTL_SHORT_VDDF_TO_VDDS
        if (g_ui32origSimobuckVDDStrim != 0xFFFFFFFF)
        {
            MCUCTRL->SIMOBUCK13_b.ACTTRIMVDDS = g_ui32origSimobuckVDDStrim;
        }

        //
        // Remove VDDS/VDDF short
        //
        MCUCTRL->PWRSW1_b.SHORTVDDFVDDSORVAL  = 0;
        MCUCTRL->PWRSW1_b.SHORTVDDFVDDSOREN   = 0;
#endif

// #### INTERNAL BEGIN ####
#if 0 // This is not needed, as LDO's should already be in parallel
#if AM_HAL_PWRCTL_SET_CORELDO_MEMLDO_IN_PARALLEL
#if AM_HAL_PWRCTRL_LDOS_FOR_VDDC
        // #### INTERNAL2 BEGIN ####
        // A2SD-2248 Activate LDOs in parallel when disabling SIMOBUCK.
        // #### INTERNAL2 END ####
        simobuck_ldos_force_active();  // Force LDOs in parallel with simobuck
#else
        coreldo_disable();
        memldo_disable();
#endif // AM_HAL_PWRCTRL_LDOS_FOR_VDDC
#endif
#endif // if 0
#warning "Fix me!!! - VRCTRL is not defined in PWRCTRL registers."
#warning "Fixme!!! - VRCTRL is NOW defined in PWRCTRL registers. Do we still want to enable SIMOBUCKEN??"
// #### INTERNAL END ####
        //PWRCTRL->VRCTRL_b.SIMOBUCKEN = 0;
        // Need to remove overrides
        buck_ldo_update_override(false);
    }

    //
    // Re-enable Crypto if not already on.
    // The enable function will check whether it's enabled or not.
    //
    ui32Ret = am_hal_pwrctrl_periph_enable(AM_HAL_PWRCTRL_PERIPH_CRYPTO);

    //
    // Restore original factory trims.
    // This will apply whether or not AM_HAL_TEMPCO_LP is activated.
    //
    restore_factory_trims();

    AM_CRITICAL_END

    return ui32Ret;

} // am_hal_pwrctrl_settings_restore()

// ****************************************************************************
//
//  Adjust or restore VDDC active high ton trim .
//  vddc_act_low_ton_adjust(bool bAdjust)
//  bAdjust  - true: change the VDDC active high ton trim
//           - false: restore the VDDC active high ton trim
//
// ****************************************************************************
static void
vddc_act_low_ton_adjust(bool bAdjust)
{
    if (g_orig_VDDCACTLOWTONTRIM != 0xFFFFFFFF) // check if original trim was saved
    {
        if (bAdjust) // adjust trim
        {
            //
            // change active high ton trim by setting the VDDCACTLOWTONTRIM register.
            //
            MCUCTRL->SIMOBUCK2_b.VDDCACTLOWTONTRIM = VDDCACTLOWTONTRIM_VAL;
        }
        else // restore
        {
            MCUCTRL->SIMOBUCK2_b.VDDCACTLOWTONTRIM = g_orig_VDDCACTLOWTONTRIM;
        }
    }
} // vddc_act_low_ton_adjust()

static void
vddf_act_low_ton_adjust(bool bAdjust)
{
    if (g_orig_VDDFACTLOWTONTRIM != 0xFFFFFFFF) // check if original trim was saved
    {
        if (bAdjust) // adjust trim
        {
            //
            // change active high ton trim by setting the VDDFACTLOWTONTRIM register.
            //
            MCUCTRL->SIMOBUCK7_b.VDDFACTLOWTONTRIM = VDDFACTLOWTONTRIM_VAL;
        }
        else // restore
        {
            MCUCTRL->SIMOBUCK7_b.VDDFACTLOWTONTRIM = g_orig_VDDFACTLOWTONTRIM;
        }
    }
} // vddf_act_low_ton_adjust()

// ****************************************************************************
//
//  am_hal_pwrctrl_vddc_boost()
// #### INTERNAL BEGIN ####
//  [SOR 5.1] Boost VDDC to 760mV in software for CPU HP (192MHz or 250MHz)
//  mode and/or 250MHz MSPI0/3 DDR data rate.
// #### INTERNAL END ####
//
// ****************************************************************************
void
am_hal_pwrctrl_vddc_boost(void)
{
    AM_CRITICAL_BEGIN

    if (g_ui32VDDCBoostReqCnt < 1)
    {
        // #### INTERNAL BEGIN ####
        //
        // [SOR 5.1.1] Boost the coreldo to 740mV (+100mV)
        //
        // #### INTERNAL END ####
        vddc_ldo_adjust(ADJUST_POWER_SETTING, CORELDOTEMPCOTRIM_VAL);

        // #### INTERNAL BEGIN ####
        //
        // [SOR 5.1.2] Boost simobuck VDDC to 760mV (+100mV) by setting d2aspare register bit 8
        //
        // #### INTERNAL END ####
        vddc_simobuck_adjust(ADJUST_POWER_SETTING);

        //
        // [SOR 5.1.3] Delay 15us.
        //
        am_hal_delay_us(15);

        //
        // [SOR 5.1.4] Disable simobuck active override and simobuck
        // Note that the simobuck must be disabled before changing the Ton trim values and then re-enabled after changing the Ton trim values.
        //
        MCUCTRL->VRCTRL_b.SIMOBUCKOVER = 0;
        PWRCTRL->VRCTRL_b.SIMOBUCKEN = 0;

        //
        // [SOR 5.1.5] Change vddc_act_low_ton_trim to 21.
        //
        vddc_act_low_ton_adjust(true);

        //
        // [SOR 5.1.6] Change vddf_act_low_ton_trim to 31.
        //
        vddf_act_low_ton_adjust(true);

        //
        // [SOR 5.1.7] Enable simobuck active override and simobuck.
        //
        MCUCTRL->VRCTRL_b.SIMOBUCKOVER = 1;
        PWRCTRL->VRCTRL_b.SIMOBUCKEN = 1;
    }

    //
    // VDDC boost request count + 1
    //
    g_ui32VDDCBoostReqCnt++;

    AM_CRITICAL_END

} // am_hal_pwrctrl_vddc_boost()

// ****************************************************************************
//
//  am_hal_pwrctrl_vddc_restore()
//  [SOR 5.2] The voltage boost trim settings and ton trim settings below
//  should be reverted when not using CPU HP mode or 250MHz MSPI0/3 DDR data rate.
//
//
// ****************************************************************************
void
am_hal_pwrctrl_vddc_restore(void)
{
    AM_CRITICAL_BEGIN

    //
    // VDDC boost request count - 1
    //
    if (g_ui32VDDCBoostReqCnt > 0)
    {
        g_ui32VDDCBoostReqCnt--;
    }

    if (g_ui32VDDCBoostReqCnt == 0)
    {
        //
        // [SOR 5.2.1] Disable simobuck active override and simobuck.
        //
        MCUCTRL->VRCTRL_b.SIMOBUCKOVER = 0;
        PWRCTRL->VRCTRL_b.SIMOBUCKEN = 0;
        //
        // [SOR 5.2.2] Change vddc_act_low_ton_trim back to its original value
        //
        vddc_act_low_ton_adjust(false);

        //
        // [SOR 5.2.3] Change vddf_act_low_ton_trim back to its original value
        //
        vddf_act_low_ton_adjust(false);

        //
        // [SOR 5.2.4] Set the simobuck VDDC voltage back to its original voltage level by clearing d2aspare register bit 8.
        //
        vddc_simobuck_adjust(RESTORE_POWER_SETTING);

        //
        // [SOR 5.2.5] Enable simobuck active override and simobuck.
        //
        MCUCTRL->VRCTRL_b.SIMOBUCKOVER = 1;
        PWRCTRL->VRCTRL_b.SIMOBUCKEN = 1;

        //
        // [SOR 5.2.6] Set the coreldo tempco trim back to its original value.
        //
        vddc_ldo_adjust(RESTORE_POWER_SETTING, 0);
    }

    AM_CRITICAL_END
} // am_hal_pwrctrl_vddc_restore()


static void
vddf_boost_level_set(am_hal_pwrctrl_vddf_boost_level_e eBoostLevel)
{
    static am_hal_pwrctrl_vddf_boost_level_e g_eCurVddfBoostState = AM_HAL_PWRCTRL_VDDF_BOOST_DEFAULT;

    switch(eBoostLevel)
    {
        case AM_HAL_PWRCTRL_VDDF_BOOST_DEFAULT :
            if (g_eCurVddfBoostState == AM_HAL_PWRCTRL_VDDF_BOOST_LEVEL1)
            {
                //
                // [SOR 6.2] Reversion sequence
                // [SOR 6.2.1] Set the simobuck VDDF vref trim and memldo active trim back to their original values.
                //
                vddf_simobuck_trim_set(g_orig_TVRGFVREFTRIM);
                MCUCTRL->LDOREG2_b.MEMLDOACTIVETRIM = g_orig_MEMLDOTRIM;
            }
            else if (g_eCurVddfBoostState == AM_HAL_PWRCTRL_VDDF_BOOST_LEVEL2)
            {
                //
                // [SOR 7.2] Reversion sequence
                // [SOR 7.2.1] Set simobuck VDDF vref and tempco trims and memldo active trim back to original values in the following order.
                //
                vddf_simobuck_trim_set(g_orig_TVRGFVREFTRIM);
                vddf_simobuck_trim_set_tvrgf_tempco(g_orig_TVRGFTEMPCOTRIM);
                MCUCTRL->LDOREG2_b.MEMLDOACTIVETRIM = g_orig_MEMLDOTRIM;
            }

            g_eCurVddfBoostState = AM_HAL_PWRCTRL_VDDF_BOOST_DEFAULT;

            break;
        case AM_HAL_PWRCTRL_VDDF_BOOST_LEVEL1 :
            if (g_eCurVddfBoostState == AM_HAL_PWRCTRL_VDDF_BOOST_DEFAULT)
            {
                // #### INTERNAL BEGIN ####
                //
                // [SOR 6] Boost VDDF to 830mV in software for any of the following conditions:
                //          192MHz MSPI0/3 DDR data rate, GPU HP mode, or CPU HP mode.
                //
                //
                // [SOR 6.1.1] Boost memldo to 805mV (+30mV)
                //
                // #### INTERNAL END ####
                vddf_ldo_adjust(ADJUST_POWER_SETTING, MEMLDOACTIVETRIM_DELTA_0);

                // #### INTERNAL BEGIN ####
                //
                // [SOR 6.1.2] Boost simobuck VDDF to 830mV (+35mV)
                //
                // #### INTERNAL END ####
                vddf_simobuck_adjust(ADJUST_POWER_SETTING, TVRGFVREFTRIM_DELTA_0);

                //
                // [SOR 6.1.3] Delay 15us.
                //
                am_hal_delay_us(15);

                g_eCurVddfBoostState = AM_HAL_PWRCTRL_VDDF_BOOST_LEVEL1;
            }
            else if (g_eCurVddfBoostState == AM_HAL_PWRCTRL_VDDF_BOOST_LEVEL2)
            {
                //
                // [SOR 7.3] Reversion sequence if any the following conditions exist:
                // 192MHz MSPI0/3 DDR data rate, GPU HP mode, or CPU HP mode.
                //

                //
                // Revert TVRGF TEMPCO setting
                //
                vddf_simobuck_trim_set_tvrgf_tempco(g_orig_TVRGFTEMPCOTRIM);

                // #### INTERNAL BEGIN ####
                //
                // [SOR 6.1.1] Boost memldo to 805mV (+30mV)
                //
                // #### INTERNAL END ####
                vddf_simobuck_adjust(ADJUST_POWER_SETTING, TVRGFVREFTRIM_DELTA_0);

                // #### INTERNAL BEGIN ####
                //
                // [SOR 6.1.2] Boost simobuck VDDF to 830mV (+35mV)
                //
                // #### INTERNAL END ####
                vddf_ldo_adjust(ADJUST_POWER_SETTING, MEMLDOACTIVETRIM_DELTA_0);
                //
                // [SOR 6.1.3] Delay 15us.
                //
                am_hal_delay_us(15);

                g_eCurVddfBoostState = AM_HAL_PWRCTRL_VDDF_BOOST_LEVEL1;
            }

            break;
        case AM_HAL_PWRCTRL_VDDF_BOOST_LEVEL2 :
            // #### INTERNAL BEGIN ####
            //
            // [SOR 7.1]  Code sequence
            //
            // [SOR 7.1.1] Boost the memldo to 920mV (+150mV)
            //
            // #### INTERNAL END ####
            vddf_ldo_adjust(ADJUST_POWER_SETTING, MEMLDOACTIVETRIM_DELTA_1);

            // #### INTERNAL BEGIN ####
            //
            // [SOR 7.1.2] Boost simobuck VDDF to 940mV (+145mV)
            //
            // #### INTERNAL END ####
            vddf_simobuck_trim_set(TVRGFVREFTRIM_VAL_0);
            vddf_simobuck_trim_set_tvrgf_tempco(TVRGFTEMPCOTRIM_VAL_0);

            //
            // [SOR 7.1.3] Delay 15us.
            //
            am_hal_delay_us(15);

            g_eCurVddfBoostState = AM_HAL_PWRCTRL_VDDF_BOOST_LEVEL2;

            break;
        default:
            break;
    }
} // vddf_boost_level_set()

// ****************************************************************************
//
//  am_hal_pwrctrl_vddf_boost()
//
// #### INTERNAL BEGIN ####
// [SOR 6] Boost VDDF to 830mV in software for any of the following conditions: 192MHz MSPI0/3 DDR data rate, GPU HP mode, or CPU HP mode.
//
// [SOR 7] Boost VDDF to 940mV in software for 250MHz MSPI0/3 DDR data rate.
// #### INTERNAL END ####
//
// ****************************************************************************
void
am_hal_pwrctrl_vddf_boost(am_hal_pwrctrl_vddf_boost_level_e eBoostLevel)
{
    AM_CRITICAL_BEGIN

    switch(eBoostLevel)
    {
        case AM_HAL_PWRCTRL_VDDF_BOOST_LEVEL1 :
            if ((g_ui32VDDFBoostReqLevel1Cnt < 1) && (g_ui32VDDFBoostReqLevel2Cnt < 1))
            {
                // #### INTERNAL BEGIN ####
                //
                // [SOR 6] Boost VDDF to 830mV in software for any of the following conditions:
                //          192MHz MSPI0/3 DDR data rate, GPU HP mode, or CPU HP mode.
                //
                // #### INTERNAL END ####
                vddf_boost_level_set(AM_HAL_PWRCTRL_VDDF_BOOST_LEVEL1);
            }
            //
            // VDDF Level1 boost request count +1
            //
            g_ui32VDDFBoostReqLevel1Cnt++;

            break;
        case AM_HAL_PWRCTRL_VDDF_BOOST_LEVEL2 :
            if (g_ui32VDDFBoostReqLevel2Cnt < 1)
            {
                // #### INTERNAL BEGIN ####
                //
                // [SOR 7] Boost VDDF to 940mV in software for 250MHz MSPI0/3 DDR data rate.
                //
                // #### INTERNAL END ####
                vddf_boost_level_set(AM_HAL_PWRCTRL_VDDF_BOOST_LEVEL2);
            }
            //
            // VDDF Level2 boost request count +1
            //
            g_ui32VDDFBoostReqLevel2Cnt++;
            break;
        default:
            break;
    }

    AM_CRITICAL_END
} // am_hal_pwrctrl_vddf_boost()

// ****************************************************************************
//
//  am_hal_pwrctrl_vddf_restore()
//
// ****************************************************************************
void
am_hal_pwrctrl_vddf_restore(am_hal_pwrctrl_vddf_boost_level_e eBoostLevel)
{
    AM_CRITICAL_BEGIN

    switch(eBoostLevel)
    {
        case AM_HAL_PWRCTRL_VDDF_BOOST_LEVEL1 :
            if (g_ui32VDDFBoostReqLevel1Cnt > 0)
            {
                g_ui32VDDFBoostReqLevel1Cnt--;
            }
            break;
        case AM_HAL_PWRCTRL_VDDF_BOOST_LEVEL2 :
            if (g_ui32VDDFBoostReqLevel2Cnt > 0)
            {
                g_ui32VDDFBoostReqLevel2Cnt--;
            }
            break;
        default:
            break;
    }
    if ((eBoostLevel == AM_HAL_PWRCTRL_VDDF_BOOST_LEVEL1) || (eBoostLevel == AM_HAL_PWRCTRL_VDDF_BOOST_LEVEL2))
    {
        if ((g_ui32VDDFBoostReqLevel1Cnt == 0) && (g_ui32VDDFBoostReqLevel2Cnt == 0))
        {
            vddf_boost_level_set(AM_HAL_PWRCTRL_VDDF_BOOST_DEFAULT);
        }

        if ( (eBoostLevel == AM_HAL_PWRCTRL_VDDF_BOOST_LEVEL2) &&
             (g_ui32VDDFBoostReqLevel2Cnt == 0)                &&
             (g_ui32VDDFBoostReqLevel1Cnt > 0) )
        {
            vddf_boost_level_set(AM_HAL_PWRCTRL_VDDF_BOOST_LEVEL1);
        }
    }

    AM_CRITICAL_END
} // am_hal_pwrctrl_vddf_restore()

#if AM_HAL_TEMPCO_LP
// ****************************************************************************
//
//  am_hal_pwrctrl_tempco_init()
//  ui32UpdateInterval - Time in seconds. 10 is recommended.
//
//  User must also call am_hal_adc_initialize().
//
// ****************************************************************************
uint32_t
am_hal_pwrctrl_tempco_init(void *pADCHandle,
                           uint32_t ui32ADCslot)
{
    uint32_t                    ui32Retval;
    am_hal_adc_slot_config_t    sSlotCfg;
    uint32_t ui32TrimVer;

    uint32_t ui32Temp[3];

    //
    // First make sure the temperature calibration is valid.
    //
    ui32Ret = am_hal_info1_read(AM_HAL_INFO_INFOSPACE_CURRENT_INFO1,
                                AM_REG_OTP_INFO1_TEMP_CAL_ATE_O / 4,
                                3, &ui32Temp[0]);
    if ( (ui32Retval != 0)           || (ui32Temp[0] == 0xFFFFFFFF) ||
         (ui32Temp[1] == 0xFFFFFFFF) || (ui32Temp[2] == 0xFFFFFFFF) )
    {
        //
        // Invalidate the application of TempCo.
        //
        g_bTempcoValid = false;
        return AM_HAL_STATUS_HW_ERR;
    }

    //
    // Make sure this device can reliably support TempCo
    //
    TrimVersionGet(&ui32TrimVer);
// #### INTERNAL BEGIN ####
#if 0 // Set to 1 to temporarily debug on a non-TempCo ready part
    ui32TrimVer = TRIMREV_PWRCTRL;
#endif
// #### INTERNAL END ####
    if ( ui32TrimVer < TRIMREV_PWRCTRL )
    {
        //
        // Invalidate that original trims have been saved, which
        // invalidates the application of TempCo.
        //
        //
        // Invalidate the application of TempCo.
        //
        g_bTempcoValid = false;
        return AM_HAL_STATUS_FAIL;
    }
    else
    {
        g_bTempcoValid = true;
    }

    //
    // Save the ADC handle and the slot number.
    //
    g_TempcoADCHandle   = pADCHandle;
    g_ui32TempcoADCslot = ui32ADCslot;

    //
    // At this point the ADC is expected to be initialized, powered, and configured.
    // Configure the temperature slot.
    //
    sSlotCfg.eMeasToAvg     = AM_HAL_ADC_SLOT_AVG_1;
    sSlotCfg.ui32TrkCyc     = 32;
    sSlotCfg.ePrecisionMode = AM_HAL_ADC_SLOT_12BIT;
    sSlotCfg.eChannel       = AM_HAL_ADC_SLOT_CHSEL_TEMP;
    sSlotCfg.bWindowCompare = false;
    sSlotCfg.bEnabled       = true;
    ui32Retval = am_hal_adc_configure_slot(g_TempcoADCHandle, g_ui32TempcoADCslot, &sSlotCfg);
    if ( ui32Retval != AM_HAL_STATUS_SUCCESS )
    {
       return ui32Retval;
    }

    return AM_HAL_STATUS_SUCCESS;

} // am_hal_pwrctrl_tempco_init()

//
// TempCo trims lookup tables
// The VDD table is arranged row-by-row with 3 values as:
//  0: Min temperature
//  1: Max temperature
//  2: Trim code adjust
//

//
//! Trim adjust table for VDDF
//
const static int8_t
g_VDDF_trimstbl[][3] =
{
    { -20, -11,   0},
    { -11,  -2,  -1},
    {  -2,   8,  -2},
    {   8,  17,  -3},
    {  17,  26,  -4},
    {  26,  35,  -5},
    {  35,  44,  -6},
    {  44,  53,  -7},
    {  53,  60,  -8},   // Last actual table entry
    { 127, 127,   0}    // End of table: Probably bogus temp, do no adjustment
};

//
//! Trim adjust table for VDDFLP
//
const static int8_t
g_VDDFLP_trimstbl[][3] =
{
    { -20, -11,   0},
    { -11,  -2,  -1},
    {  -2,   8,  -2},
    {   8,  17,  -3},
    {  17,  26,  -4},
    {  26,  35,  -5},
    {  35,  44,  -6},
    {  44,  53,  -7},
    {  53,  60,  -8},   // Last actual table entry
    { 127, 127,   0}    // End of table: Probably bogus temp, do no adjustment
};

//
//! Trim adjust table for memlpldo
//
const static int8_t
g_memlpldo_trimstbl[][3] =
{
    { -18, -14,   8},
    { -14, -10,   7},
    { -10,  -6,   6},
    {  -6,  -2,   5},
    {  -2,   2,   4},
    {   2,   6,   3},
    {   6,  10,   2},
    {  10,  14,   1},
    {  14,  18,   0},
    {  18,  22,  -1},
    {  22,  26,  -2},
    {  26,  42,  -3},
    {  42,  60,  -4},   // Last actual table entry
    { 127, 127,   0}    // End of table: Probably bogus temp, do no adjustment
};

//
//! Helper macro to round a float down.
//
#define FTOI_RNDDN(fval)        ( (fval < 0.00F) ? (int)fval - 1 : (int)fval )

// ****************************************************************************
// Function to lookup a trim given a temperature and a pointer
// to the appropriate lookup table.
// ****************************************************************************
static int8_t
lookup_trim(int8_t i8Temp, const int8_t pi8Tbl[][3])
{
    uint32_t ux;
    if ( i8Temp < pi8Tbl[0][0] )
    {
        //
        // Return the trim for the lowest temperature in the table
        //
        return pi8Tbl[0][2];
    }
    else
    {
        //
        // Lookup the trim
        //
        ux = 0;
        while ( pi8Tbl[ux][0] < 127 )
        {
            if ( i8Temp <= pi8Tbl[ux][1] )
            {
                return pi8Tbl[ux][2];
            }
            ux++;
        }

// #### INTERNAL BEGIN ####
        //return pi8Tbl[ux][2];
// #### INTERNAL END ####
        //
        // The temperature is very high, so snap to the default trims
        ///
        return 0;
    }

} // lookup_trim()

// ****************************************************************************
// Function to validate and apply trim changes.
// ****************************************************************************
static void
tempco_set_trims(int32_t i32VDDFtrim,
                 int32_t i32VDDFLPtrim,
                 int32_t i32MemlpLDOtrim)
{
    int32_t i32SimoVDDFact, i32Memldoact, i32SimoVDDFlp, i32Memlpldo;

    if ( !g_bOrigTrimsStored )
    {
        return;
    }

    i32SimoVDDFact = g_orig_ACTTRIMVDDF      + i32VDDFtrim;
    i32Memldoact   = g_orig_MEMLDOACTIVETRIM + i32VDDFtrim;
    i32SimoVDDFlp  = g_orig_LPTRIMVDDF       + i32VDDFLPtrim;
    i32Memlpldo    = g_orig_MEMLPLDOTRIM     + i32MemlpLDOtrim;

    //
    // If Crypto is currently off, adjust the VDDF active trim.
    //
    AM_CRITICAL_BEGIN

    if ( i32SimoVDDFact < 0 )
    {
        i32SimoVDDFact = 0;
    }
    else if ( i32SimoVDDFact > MAX_ACTTRIMVDDF )
    {
        i32SimoVDDFact = MAX_ACTTRIMVDDF;
    }

    if ( i32Memldoact < 0 )
    {
        i32Memldoact = 0;
    }
    else if ( i32Memldoact > MAX_MEMLDOACTIVETRIM )
    {
        i32Memldoact = MAX_MEMLDOACTIVETRIM;
    }

    if ( i32SimoVDDFlp < 0 )
    {
        i32SimoVDDFlp = 0;
    }
    else if ( i32SimoVDDFlp > MAX_LPTRIMVDDF )
    {
        i32SimoVDDFlp = MAX_LPTRIMVDDF;
    }

    if ( i32Memlpldo  < 0 )
    {
        i32Memlpldo  = 0;
    }
    else if ( i32Memlpldo > MAX_MEMLPLDOTRIM )
    {
        i32Memlpldo = MAX_MEMLPLDOTRIM;
    }

    //
    // Now set the new values
    //
    MCUCTRL->SIMOBUCK12_b.ACTTRIMVDDF   = i32SimoVDDFact;
    MCUCTRL->LDOREG2_b.MEMLDOACTIVETRIM = i32Memldoact;
    MCUCTRL->SIMOBUCK12_b.LPTRIMVDDF    = i32SimoVDDFlp;
    MCUCTRL->LDOREG2_b.MEMLPLDOTRIM     = i32Memlpldo;

    AM_CRITICAL_END
} // tempco_set_trims()

// ****************************************************************************
// Validate samples from temperature sensor, and average them.
// ****************************************************************************
static uint32_t
adc_get_temperature_volts(float *pfADCTempVolts,
                          uint32_t ui32NumSamples,
                          am_hal_adc_sample_t sSample[])
{
    uint32_t ux;
    float fSum;

    //
    // ui32NumSamples temperature samples have been obtained.
    // Make sure at least 2 of the samples are slightly different.
    //
    ux = 0;
    while ( ux < (ui32NumSamples - 1) )
    {
        if ( sSample[ux].ui32Sample != sSample[ux + 1].ui32Sample )
        {
            break;
        }
        ux++;
    }

    if ( ux == (ui32NumSamples - 1) )
    {
        //
        // This sample is not reliable, return with error.
        //
        return AM_HAL_STATUS_FAIL;
    }

    //
    // The measured temperature can be considered reliable.
    // Get an average of the temperature.
    //
    ux = 0;
    fSum = 0.0F;
    while ( ux < ui32NumSamples )
    {
        //
        // Convert and scale the temperature sample into its corresponding voltage.
        //
        g_ui16TempcoTEMP_code = AM_HAL_ADC_FIFO_SAMPLE(sSample[ux].ui32Sample);
        fSum += (float)g_ui16TempcoTEMP_code * AM_HAL_ADC_VREF / 4096.0f; // 12-bit sample
        ux++;
    }

    *pfADCTempVolts = fSum / ((float)ui32NumSamples);

    return AM_HAL_STATUS_SUCCESS;

} // adc_get_temperature_volts()

// ****************************************************************************
//
//  am_hal_pwrctrl_tempco_sample_handler()
//
//  This function to be called with a minimum periodicity as recommended
//  by Ambiq engineering.
//
// ****************************************************************************
uint32_t
am_hal_pwrctrl_tempco_sample_handler(uint32_t ui32NumSamples, am_hal_adc_sample_t sSamples[])
{
    uint32_t ui32Retval;
    float fVT[3];
    float fADCTempVolts, fADCTempDegreesC;
    int32_t i32VDDFtrim, i32VDDFLPtrim, i32MemlpLDOtrim;

    if ( (g_bTempcoValid == false) || (ui32NumSamples < AM_HAL_TEMPCO_NUMSAMPLES) )
    {
        return AM_HAL_STATUS_FAIL;
    }

    //
    // Query the ADC for the current temperature of the chip.
    //
    ui32Retval = adc_get_temperature_volts(&fADCTempVolts, ui32NumSamples, sSamples);
    if ( ui32Retval != AM_HAL_STATUS_SUCCESS )
    {
        //
        // This sample is not reliable.
        // Snap to the highest trim code settings from the tables and return.
// #### INTERNAL BEGIN ####
        // Note - these settings per Jamie email on 5/15/22.
// #### INTERNAL END ####
        //
        tempco_set_trims(g_VDDF_trimstbl[0][2],
                         g_VDDFLP_trimstbl[0][2],
                         g_memlpldo_trimstbl[0][2]);
// #### INTERNAL BEGIN ####
#if TEMPCO_DBG // Debug only
        am_util_stdio_printf("ERROR in adc_get_temperature_volts().\n");
#endif
// #### INTERNAL END ####
        return ui32Retval;
    }

    //
    // Now call the HAL routine to convert volts to degrees Celsius.
    //
    fVT[0] = fADCTempVolts;
    fVT[1] = 0.0f;
    fVT[2] = -123.456;
    ui32Retval = am_hal_adc_control(g_TempcoADCHandle, AM_HAL_ADC_REQ_TEMP_CELSIUS_GET, fVT);
    if ( ui32Retval == AM_HAL_STATUS_SUCCESS )
    {
        fADCTempDegreesC = fVT[1];  // Get the temperature
        g_pfTempMeasured = fADCTempDegreesC;
    }
    else
    {
        //
        // Error, restore default values.
        //
        tempco_set_trims(0, 0, 0);
        g_pfTempMeasured = 0.0F;
        return ui32Retval;
    }

    //
    // The temperature in degC is stored in fADCTempDegreesC
    //
    int8_t  i8Temp;
    int32_t i32Temp;
    i32Temp = FTOI_RNDDN(fADCTempDegreesC);

    //
    // Get the integer value of the temperature.
    // Allow for the temperature sensor specified at +-3C.
    //
    i8Temp = (int8_t)(i32Temp - 3);

    //
    // Look up the 3 trim adjustments.
    //
    i32VDDFtrim     = (int32_t)lookup_trim(i8Temp, g_VDDF_trimstbl);
    i32VDDFLPtrim   = (int32_t)lookup_trim(i8Temp, g_VDDFLP_trimstbl);
    i32MemlpLDOtrim = (int32_t)lookup_trim(i8Temp, g_memlpldo_trimstbl);

    //
    // Now, set the trims appropriately.
    //
// #### INTERNAL BEGIN ####
#if TEMPCO_DBG // Debug only
    am_util_stdio_printf("TEMP=%.2F C, AdjTemp=%4d, VDDFtrim=%4d, VDDFLPtrim=%4d, LDOtrim=%4d\n", fADCTempDegreesC, i8Temp, i32VDDFtrim, i32VDDFLPtrim, i32MemlpLDOtrim);
#endif
// #### INTERNAL END ####
    tempco_set_trims(i32VDDFtrim, i32VDDFLPtrim, i32MemlpLDOtrim);

    return AM_HAL_STATUS_SUCCESS;
} // am_hal_pwrctrl_tempco_sample_handler()

#endif // AM_HAL_TEMPCO_LP

//*****************************************************************************
//
// End Doxygen group.
//! @}
//
//*****************************************************************************
