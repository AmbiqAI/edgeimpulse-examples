//*****************************************************************************
//
//! @file am_hal_reset.c
//!
//! @brief Hardware abstraction layer for the Reset Generator module.
//!
//! @addtogroup rstgen3p Reset - Reset Generator (RSTGEN)
//! @ingroup apollo3p_hal
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

#include "am_mcu_apollo.h"

uint32_t gAmHalResetStatus = 0;

//*****************************************************************************
//
//  am_hal_reset_enable()
//  Enable and configure the Reset controller.
//
//*****************************************************************************
uint32_t
am_hal_reset_configure(am_hal_reset_configure_e eConfigure)
{
    uint32_t ui32Val;
    bool     bEnable;

    switch ( eConfigure )
    {
        case AM_HAL_RESET_BROWNOUT_HIGH_ENABLE:
            bEnable = true;
            ui32Val = RSTGEN_CFG_BODHREN_Msk;
            break;

        case AM_HAL_RESET_WDT_RESET_ENABLE:
            bEnable = true;
            ui32Val = RSTGEN_CFG_WDREN_Msk;
            break;

        case AM_HAL_RESET_BROWNOUT_HIGH_DISABLE:
            bEnable = false;
            ui32Val = RSTGEN_CFG_BODHREN_Msk;
            break;

        case AM_HAL_RESET_WDT_RESET_DISABLE:
            bEnable = false;
            ui32Val = RSTGEN_CFG_WDREN_Msk;
            break;

        default:
            return AM_HAL_STATUS_INVALID_ARG;
    }

    AM_CRITICAL_BEGIN
    if ( bEnable )
    {
        RSTGEN->CFG |= ui32Val;
    }
    else
    {
        RSTGEN->CFG &= ~ui32Val;
    }
    AM_CRITICAL_END

    //
    // Return success status.
    //
    return AM_HAL_STATUS_SUCCESS;

} // am_hal_reset_configure()


//*****************************************************************************
//
//  am_hal_reset_control()
//  Perform various reset functions including assertion of software resets.
//
//*****************************************************************************
uint32_t
am_hal_reset_control(am_hal_reset_control_e eControl, void *pArgs)
{
    switch ( eControl )
    {
        case AM_HAL_RESET_CONTROL_SWPOR:
            //
            // Perform a Power On Reset level reset.
            // Write the POR key to the software POR register.
            //
            RSTGEN->SWPOR =
                   _VAL2FLD(RSTGEN_SWPOR_SWPORKEY, RSTGEN_SWPOR_SWPORKEY_KEYVALUE);
            break;

        case AM_HAL_RESET_CONTROL_SWPOI:
            //
            // Perform a Power On Initialization level reset.
            // Write the POI key to the software POI register.
            //
            RSTGEN->SWPOI =
                _VAL2FLD(RSTGEN_SWPOI_SWPOIKEY, RSTGEN_SWPOI_SWPOIKEY_KEYVALUE);
            break;

        case AM_HAL_RESET_CONTROL_STATUSCLEAR:
            //
            // Clear ALL of the reset status register bits.
            //
// #### INTERNAL BEGIN ####
//          The SBL now controls the actual STAT register, so CLRSTAT is no
//          longer useful.
//          AM_REG(RSTGEN, CLRSTAT) = AM_REG_RSTGEN_CLRSTAT_CLRSTAT(1);
// #### INTERNAL END ####
            RSTGEN->STAT = 0;
            break;

        case AM_HAL_RESET_CONTROL_TPIU_RESET:
            //
            // Reset the TPIU.
            //
            RSTGEN->TPIURST = _VAL2FLD(RSTGEN_TPIURST_TPIURST, 1);
            break;

        default:
            return AM_HAL_STATUS_INVALID_ARG;
    }

    //
    // Return success status.
    //
    return AM_HAL_STATUS_SUCCESS;

} // am_hal_reset_control()


//*****************************************************************************
//
//  am_hal_reset_status_get()
//  Return status of the reset generator.
//  Application MUST call this API at least once before going to deepsleep
//  Otherwise this API will not provide correct reset status
//
//*****************************************************************************
uint32_t
am_hal_reset_status_get(am_hal_reset_status_t *psStatus)
{
    // Need to read the status only the very first time

    if ( psStatus == NULL )
    {
        return AM_HAL_STATUS_INVALID_ARG;
    }

    //
    // Retrieve the reset generator status bits
    //
    if (!gAmHalResetStatus)
    {
        gAmHalResetStatus = RSTGEN->STAT;
    }
    psStatus->eStatus      = (am_hal_reset_status_e)gAmHalResetStatus;
    psStatus->bEXTStat     = _FLD2VAL(RSTGEN_STAT_EXRSTAT, gAmHalResetStatus);
    psStatus->bPORStat     = _FLD2VAL(RSTGEN_STAT_PORSTAT, gAmHalResetStatus);
    psStatus->bBODStat     = _FLD2VAL(RSTGEN_STAT_BORSTAT, gAmHalResetStatus);
    psStatus->bSWPORStat   = _FLD2VAL(RSTGEN_STAT_SWRSTAT, gAmHalResetStatus);
    psStatus->bSWPOIStat   = _FLD2VAL(RSTGEN_STAT_POIRSTAT, gAmHalResetStatus);
    psStatus->bDBGRStat    = _FLD2VAL(RSTGEN_STAT_DBGRSTAT, gAmHalResetStatus);
    psStatus->bWDTStat     = _FLD2VAL(RSTGEN_STAT_WDRSTAT, gAmHalResetStatus);
    psStatus->bBOUnregStat = _FLD2VAL(RSTGEN_STAT_BOUSTAT, gAmHalResetStatus);
    psStatus->bBOCOREStat  = _FLD2VAL(RSTGEN_STAT_BOCSTAT, gAmHalResetStatus);
    psStatus->bBOMEMStat   = _FLD2VAL(RSTGEN_STAT_BOFSTAT, gAmHalResetStatus);
    psStatus->bBOBLEStat   = _FLD2VAL(RSTGEN_STAT_BOBSTAT, gAmHalResetStatus);

    //
    // Return status.
    // If the Reset Status is 0 - this implies application did not capture the snapshot
    // before deepsleep, and hence the result is invalid
    //
    return (gAmHalResetStatus ? AM_HAL_STATUS_SUCCESS : AM_HAL_STATUS_FAIL);

} // am_hal_reset_status_get()

//*****************************************************************************
//
// @brief Enable selected RSTGEN Interrupts.
//
// @note Use this function to enable the reset generator interrupts.
//
// @param ui32IntMask - One or more of the following bits, any of which can
//                      be ORed together.
//                      AM_HAL_RESET_INTERRUPT_BODH
//
// @return status      - generic or interface specific status.
//
//*****************************************************************************
uint32_t
am_hal_reset_interrupt_enable(uint32_t ui32IntMask)
{
    AM_CRITICAL_BEGIN
    RSTGEN->INTEN |= ui32IntMask;
    AM_CRITICAL_END

    //
    // Return success status.
    //
    return AM_HAL_STATUS_SUCCESS;

} // am_hal_reset_interrupt_enable()

//*****************************************************************************
//
//  am_hal_reset_interrupt_disable()
//  Disable selected RSTGEN Interrupts.
//
//*****************************************************************************
uint32_t
am_hal_reset_interrupt_disable(uint32_t ui32IntMask)
{
    AM_CRITICAL_BEGIN
    RSTGEN->INTEN &= ~ui32IntMask;
    AM_CRITICAL_END

    //
    // Return success status.
    //
    return AM_HAL_STATUS_SUCCESS;

} // am_hal_reset_interrupt_disable()

//*****************************************************************************
//
//  am_hal_reset_interrupt_clear()
//  Reset generator interrupt clear
//
//*****************************************************************************
uint32_t
am_hal_reset_interrupt_clear(uint32_t ui32IntMask)
{
    RSTGEN->INTEN = ui32IntMask;

    //
    // Return success status.
    //
    return AM_HAL_STATUS_SUCCESS;

} // am_hal_reset_interrupt_clear()

//*****************************************************************************
//
//  am_hal_reset_interrupt_status_get()
//  Get interrupt status of reset generator.
//
//*****************************************************************************
uint32_t
am_hal_reset_interrupt_status_get(bool bEnabledOnly,
                                  uint32_t *pui32IntStatus)
{
    if ( pui32IntStatus == NULL )
    {
        return AM_HAL_STATUS_INVALID_ARG;
    }

    //
    // Retrieve the reset generator status bits
    //
    *pui32IntStatus = RSTGEN->INTSTAT;

    //
    // Return success status.
    //
    return AM_HAL_STATUS_SUCCESS;

} // am_hal_reset_interrupt_status_get()


//*****************************************************************************
//
// End Doxygen group.
//! @}
//
//*****************************************************************************
