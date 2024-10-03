//*****************************************************************************
//
//! @file am_hal_burst.c
//!
//! @brief Functions for Controlling Burst Mode Operation.
//!
//! @addtogroup burstmode3 Burst - Burst Mode Functionality
//! @ingroup apollo3_hal
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
// Disable wait for WFE
//#define NO_WFE
// #### INTERNAL END ####
//
//! Globals.
//
bool    g_am_hal_burst_mode_available = false;

// ****************************************************************************
//
//  am_hal_burst_mode_initialize()
//  Burst mode initialization function
//
// ****************************************************************************
uint32_t
am_hal_burst_mode_initialize(am_hal_burst_avail_e *peBurstAvail)
{
    uint32_t    ui32Status;
    //
    // Check if the Burst Mode feature is available based on the SKU.
    //
    if ( 0 == MCUCTRL->SKU_b.ALLOWBURST )
    {
        //
        // Burst mode is not available.
        //
        g_am_hal_burst_mode_available = false;
        *peBurstAvail = AM_HAL_BURST_NOTAVAIL;
        return AM_HAL_STATUS_INVALID_OPERATION;
    }

// #### INTERNAL BEGIN ####
#if !APOLLO3_FPGA
// #### INTERNAL END ####
    //
    // Enable the Burst Feature Event (DEVPWREVENTEN).
    //
    PWRCTRL->DEVPWREVENTEN_b.BURSTEVEN = 1;

    //
    // Enable the Burst Functionality (FEATUREENABLE).
    //
    MCUCTRL->FEATUREENABLE_b.BURSTREQ = 1;

    ui32Status = am_hal_flash_delay_status_check(10000,
                            (uint32_t)&MCUCTRL->FEATUREENABLE,
                            MCUCTRL_FEATUREENABLE_BURSTACK_Msk,
                            MCUCTRL_FEATUREENABLE_BURSTACK_Msk,
                            true);

    if ( ui32Status != AM_HAL_STATUS_SUCCESS )
    {
        g_am_hal_burst_mode_available = false;
        *peBurstAvail = AM_HAL_BURST_NOTAVAIL;
        return ui32Status;
    }

    if ( 0 == MCUCTRL->FEATUREENABLE_b.BURSTAVAIL )
    {
        //
        // Burst mode is not available.
        //
        g_am_hal_burst_mode_available = false;
        *peBurstAvail = AM_HAL_BURST_NOTAVAIL;
        return AM_HAL_STATUS_INVALID_OPERATION;
    }
// #### INTERNAL BEGIN ####
#else
    //
    // Enable the Burst Functionality (FEATUREENABLE).
    //
    MCUCTRL->FEATUREENABLE_b.BURSTREQ = 1;

    am_hal_flash_delay(FLASH_CYCLES_US(10));
#endif // !APOLLO3_FPGA
// #### INTERNAL END ####

    //
    // Check the ACK for the Burst Functionality.
    //
    if ( MCUCTRL->FEATUREENABLE_b.BURSTACK == 0 )
    {
        //
        // If NACK, return status.
        //
        g_am_hal_burst_mode_available = false;
        *peBurstAvail = AM_HAL_BURST_NOTAVAIL;
        return AM_HAL_STATUS_INVALID_OPERATION;
    }

// #### INTERNAL BEGIN ####
#if 0
    //
    // Set up BURSTCLKON/OFF (CLK_GEN_MISC).
    //
    AM_BFW(CLKGEN, MISC, FRCBURSTOFF, AM_REG_CLKGEN_MISC_FRCBURSTOFF_BURSTCLKON);
#endif
// #### INTERNAL END ####
    //
    // Return Availability
    //
    g_am_hal_burst_mode_available = true;
    *peBurstAvail = AM_HAL_BURST_AVAIL;
    return AM_HAL_STATUS_SUCCESS;
}

// ****************************************************************************
//
//  am_hal_burst_mode_enable()
//  Burst mode enable function
//
// ****************************************************************************
uint32_t
am_hal_burst_mode_enable(am_hal_burst_mode_e *peBurstStatus)
{
    uint32_t    ui32Status;

    //
    // Check if Burst Mode is allowed and return status if it is not.
    //
    if (!g_am_hal_burst_mode_available)
    {
        *peBurstStatus = AM_HAL_NORMAL_MODE;
        return AM_HAL_STATUS_INVALID_OPERATION;
    }

// #### INTERNAL BEGIN ####
#if !APOLLO3_FPGA
// #### INTERNAL END ####
    //
    // Request Burst Mode Enable (FREQCTRL)
    //
    CLKGEN->FREQCTRL_b.BURSTREQ = CLKGEN_FREQCTRL_BURSTREQ_EN;

//    while (0 == AM_BFR(CLKGEN, FREQCTRL, BURSTACK));
    ui32Status = am_hal_flash_delay_status_check(10000,
                    (uint32_t)&CLKGEN->FREQCTRL,
                    CLKGEN_FREQCTRL_BURSTSTATUS_Msk,
                    CLKGEN_FREQCTRL_BURSTSTATUS_Msk,
                    true);

    if ( ui32Status != AM_HAL_STATUS_SUCCESS )
    {
        *peBurstStatus = AM_HAL_NORMAL_MODE;
        return ui32Status;
    }
// #### INTERNAL BEGIN ####
#else
    //
    // Request Burst Mode Enable (FREQCTRL)
    //
    AM_BFW(CLKGEN, FREQCTRL, BURSTREQ, AM_REG_CLKGEN_FREQCTRL_BURSTREQ_EN);

    am_hal_flash_delay(FLASH_CYCLES_US(10));
#endif // !APOLLO3_FPGA
// #### INTERNAL END ####

    //
    // Check that the Burst Request was ACK'd.
    //
    if ( 0 == CLKGEN->FREQCTRL_b.BURSTACK )
    {
        *peBurstStatus = AM_HAL_NORMAL_MODE;
        return AM_HAL_STATUS_FAIL;
    }

    //
    // Check the Burst Mode Status (FREQCTRL)
    //
    if ( CLKGEN->FREQCTRL_b.BURSTSTATUS > 0)
    {
        *peBurstStatus =  AM_HAL_BURST_MODE;
    }
    else
    {
        *peBurstStatus =  AM_HAL_NORMAL_MODE;
    }

    return AM_HAL_STATUS_SUCCESS;
}

// ****************************************************************************
//
//  am_hal_burst_mode_disable()
//  Burst mode disable function
//
// ****************************************************************************
uint32_t
am_hal_burst_mode_disable(am_hal_burst_mode_e *peBurstStatus)
{
    uint32_t    ui32Status;

    //
    // Request Burst Mode Enable (FREQCTRL)
    //
    //
    // Safely disable burst mode.
    //
    AM_CRITICAL_BEGIN
    am_hal_flash_store_ui32((uint32_t*)&CLKGEN->FREQCTRL, CLKGEN_FREQCTRL_BURSTREQ_DIS);
    AM_CRITICAL_END

    //
    // Disable the Burst Feature Event (DEVPWREVENTEN).
    //
    PWRCTRL->DEVPWREVENTEN_b.BURSTEVEN = 0;

    ui32Status = am_hal_flash_delay_status_check(10000,
                            (uint32_t)&CLKGEN->FREQCTRL,
                            CLKGEN_FREQCTRL_BURSTSTATUS_Msk,
                            0,
                            true);

    if ( ui32Status != AM_HAL_STATUS_SUCCESS )
    {
        *peBurstStatus = AM_HAL_NORMAL_MODE;
        return ui32Status;
    }

    //
    // Check the Burst Mode Status (FREQCTRL)
    //
    //
    // Check the Burst Mode Status (FREQCTRL)
    //
    if ( CLKGEN->FREQCTRL_b.BURSTSTATUS > 0 )
    {
        *peBurstStatus = AM_HAL_BURST_MODE;
    }
    else
    {
        *peBurstStatus = AM_HAL_NORMAL_MODE;
    }


    return AM_HAL_STATUS_SUCCESS;
}

//*****************************************************************************
//
// am_hal_burst_mode_status() - Return current burst mode state.
//
// Implemented as a macro, this function returns the current burst mode state.
//     AM_HAL_BURST_MODE
//     AM_HAL_NORMAL_MODE
//
//*****************************************************************************

//*****************************************************************************
//
// End Doxygen group.
//! @}
//
//*****************************************************************************
