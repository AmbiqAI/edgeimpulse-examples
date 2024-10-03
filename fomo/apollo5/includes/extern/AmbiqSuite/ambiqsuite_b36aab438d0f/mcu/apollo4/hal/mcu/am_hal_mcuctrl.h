//*****************************************************************************
//
//! @file am_hal_mcuctrl.h
//!
//! @brief Functions for accessing and configuring the MCUCTRL.
//!
//! @addtogroup mcuctrl4 MCU Control (MCUCTRL)
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
#ifndef AM_HAL_MCUCTRL_H
#define AM_HAL_MCUCTRL_H

#ifdef __cplusplus
extern "C"
{
#endif

//*****************************************************************************
//
// Chip Revision IDentification.
//
//*****************************************************************************
static inline uint32_t
AM_HAL_MCUCTRL_REV_GET(void)
{
    uint32_t ui32ChipRev, ui32ChipID0, ui32ChipID1, ui32ChipId0Rev;

    ui32ChipRev = MCUCTRL->CHIPREV & 0xFF;

    if (ui32ChipRev == 0x11)
    {
        ui32ChipID0 = MCUCTRL->CHIPID0;
        ui32ChipID1 = MCUCTRL->CHIPID1;

        if (ui32ChipID1 == 0xFE00000A)
        {
            return 0x11;
        }
        else
        {
            ui32ChipId0Rev = ((ui32ChipID0 & 0xF0) >> 4);

            if (ui32ChipId0Rev != 0)
            {
                return (ui32ChipId0Rev | 0x10) + 1;
            }
            else
            {
                ui32ChipID0 &= 0xFFF80000;

                //
                // Look for date codes of 6/5/2020 or 6/6/2020.
                //
                if ( (ui32ChipID0 == ((3 << 28) | (6 << 24) | (5 << 19))) ||
                     (ui32ChipID0 == ((3 << 28) | (6 << 24) | (6 << 19))) )
                {
                    return 0x12;
                }
                else
                {
                    return 0x11;
                }
            }
        }
    }
    else
    {
        return ui32ChipRev;
    }
}

#define APOLLO4_A0 (AM_HAL_MCUCTRL_REV_GET() == 0x11)
#define APOLLO4_A1 (AM_HAL_MCUCTRL_REV_GET() == 0x12)

//*****************************************************************************
//
// MCUCTRL specific definitions.
//
//*****************************************************************************
#define AM_HAL_MCUCTRL_CHIPPN_NV_SIZE_N     ((MCUCTRL_CHIPPN_PARTNUM_MRAMSIZE_M >> MCUCTRL_CHIPPN_PARTNUM_MRAMSIZE_S) + 1)
#define AM_HAL_MCUCTRL_CHIPPN_DTCM_SIZE_N   ((MCUCTRL_CHIPPN_PARTNUM_SRAMSIZE_M  >> MCUCTRL_CHIPPN_PARTNUM_SRAMSIZE_S) + 1)

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
    AM_HAL_MCUCTRL_CONTROL_EXTCLK32M_DISABLE
} am_hal_mcuctrl_control_e;

//**************************************
//! MCUCTRL info get
//**************************************
typedef enum
{
    AM_HAL_MCUCTRL_INFO_FEATURES_AVAIL,
    AM_HAL_MCUCTRL_INFO_DEVICEID
} am_hal_mcuctrl_infoget_e;

//*****************************************************************************
//
// MCUCTRL SKU/Feature Enums
//
//*****************************************************************************
typedef enum
{
    AM_HAL_MCUCTRL_DTCM_384K
} am_hal_mcuctrl_dtcm_e;

//
// SSRAM size SKU: 0: 512K SSRAM, 1: 1MB SSRAM, 2: 1MB SSRAM + DSP Memories
// MRAM size SKU. 0:0.5MB, 1=1MB, 2=1.5MB, 3:2MB
//
typedef enum
{
    AM_HAL_MCUCTRL_SSRAM_512K,
    AM_HAL_MCUCTRL_SSRAM_1M,
    AM_HAL_MCUCTRL_SSRAM_1M_PLUS_DSP
} am_hal_mcuctrl_ssram_e;
#define AM_HAL_MCUCTRL_SSRAM_MAX    AM_HAL_MCUCTRL_SSRAM_1M_PLUS_DSP

typedef enum
{
    AM_HAL_MCUCTRL_MRAM_512K,
    AM_HAL_MCUCTRL_MRAM_1M,
    AM_HAL_MCUCTRL_MRAM_1P5M,
    AM_HAL_MCUCTRL_MRAM_2M,
} am_hal_mcuctrl_mram_e;
#define AM_HAL_MCUCTRL_MRAM_MAX     AM_HAL_MCUCTRL_MRAM_2M

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
//! MCUCTRL device structure
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
    //! SKU (Apollo3).
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
    //
    uint32_t ui32DTCMSize;

    //
    //! SSRAM Size.
    //
    uint32_t ui32SSRAMSize;

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
//! MCUCTRL status structure
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
//! MCUCTRL features available structure
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

// ****************************************************************************
//
//! @brief Apply various specific commands/controls on the MCUCTRL module.
//!
//! This function is used to apply various controls to MCUCTRL.
//!
//! @param eControl - One of the following:
//!     AM_HAL_MCUCTRL_CONTROL_EXTCLK32K_ENABLE
//!     AM_HAL_MCUCTRL_CONTROL_EXTCLK32K_DISABLE
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
