//*****************************************************************************
//
//  am_hal_cachectrl.c
//! @file
//!
//! @brief Functions for interfacing with the CACHE controller.
//!
//! @addtogroup cachectrl4 Cache Control (CACHE)
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

#include <stdint.h>
#include <stdbool.h>
#include "am_mcu_apollo.h"

//*****************************************************************************
//
//  Default settings for the cache.
//
//*****************************************************************************
const am_hal_cachectrl_config_t am_hal_cachectrl_defaults =
{
    .bLRU                       = 0,
    .eDescript                  = AM_HAL_CACHECTRL_DESCR_1WAY_128B_4096E,
    .eMode                      = AM_HAL_CACHECTRL_CONFIG_MODE_INSTR_DATA,
};

//*****************************************************************************
//
//  Configure the cache with given and recommended settings, but do not enable.
//
//*****************************************************************************
uint32_t
am_hal_cachectrl_config(const am_hal_cachectrl_config_t *psConfig)
{
    //
    // In the case where cache is currently enabled, we need to gracefully
    // bow out of that configuration before reconfiguring.  The best way to
    // accomplish that is to shut down the ID bits, leaving the cache enabled.
    // Once the instr and data caches have been disabled, we can safely set
    // any new configuration, including disabling the controller.
    //
    AM_CRITICAL_BEGIN
    CPU->CACHECFG &=
        ~(CPU_CACHECFG_DENABLE_Msk  |
          CPU_CACHECFG_IENABLE_Msk);
    AM_CRITICAL_END

    CPU->CACHECFG =
        _VAL2FLD(CPU_CACHECFG_ENABLE, 0)                              |
        _VAL2FLD(CPU_CACHECFG_CLKGATE, 1)                             |
        _VAL2FLD(CPU_CACHECFG_LS, 0)                                  |
        _VAL2FLD(CPU_CACHECFG_DATACLKGATE, 1)                         |
        _VAL2FLD(CPU_CACHECFG_ENABLEMONITOR, 0)                       |
        _VAL2FLD(CPU_CACHECFG_LRU, psConfig->bLRU)                    |
        _VAL2FLD(CPU_CACHECFG_CONFIG, psConfig->eDescript)            |
        ((psConfig->eMode << CPU_CACHECFG_IENABLE_Pos) &
            (CPU_CACHECFG_DENABLE_Msk   |
             CPU_CACHECFG_IENABLE_Msk));

    return AM_HAL_STATUS_SUCCESS;

} // am_hal_cachectrl_config()

//*****************************************************************************
//
//  Enable the cache.
//
//*****************************************************************************
uint32_t
am_hal_cachectrl_enable(void)
{
    //
    // Enable the cache
    //
    CPU->CACHECFG |= _VAL2FLD(CPU_CACHECFG_ENABLE, 1);
    CPU->CACHECTRL |= _VAL2FLD(CPU_CACHECTRL_INVALIDATE, 1);

    return AM_HAL_STATUS_SUCCESS;
} // am_hal_cachectrl_enable()

//*****************************************************************************
//
//  Disable the cache.
//
//*****************************************************************************
uint32_t
am_hal_cachectrl_disable(void)
{
    //
    // Shut down as gracefully as possible.
    // Disable the I/D cache enable bits first to allow a little time
    // for any in-flight transactions to hand off to the line buffer.
    // Then clear the enable.
    //
    AM_CRITICAL_BEGIN
    CPU->CACHECFG &= ~(_VAL2FLD(CPU_CACHECFG_IENABLE, 1) |
                             _VAL2FLD(CPU_CACHECFG_DENABLE, 1));
    CPU->CACHECFG &= ~_VAL2FLD(CPU_CACHECFG_ENABLE, 1);
    AM_CRITICAL_END

    return AM_HAL_STATUS_SUCCESS;
} // am_hal_cachectrl_disable()

//*****************************************************************************
//
//  Select the cache configuration type.
//
//*****************************************************************************
uint32_t
am_hal_cachectrl_control(am_hal_cachectrl_control_e eControl, void *pArgs)
{
//    uint32_t ui32Arg;
    uint32_t ui32SetMask = 0;

    switch ( eControl )
    {
        case AM_HAL_CACHECTRL_CONTROL_MRAM_CACHE_INVALIDATE:
            ui32SetMask = CPU_CACHECTRL_INVALIDATE_Msk;
            break;
        case AM_HAL_CACHECTRL_CONTROL_STATISTICS_RESET:
            if ( !_FLD2VAL(CPU_CACHECFG_ENABLEMONITOR, CPU->CACHECFG) )
            {
                //
                // The monitor must be enabled for the reset to have any affect.
                //
                return AM_HAL_STATUS_INVALID_OPERATION;
            }
            else
            {
                ui32SetMask = CPU_CACHECTRL_RESETSTAT_Msk;
            }
            break;
        case AM_HAL_CACHECTRL_CONTROL_MONITOR_ENABLE:
            ui32SetMask = 0;
            AM_CRITICAL_BEGIN
            CPU->CACHECFG |= CPU_CACHECFG_ENABLEMONITOR_Msk;
            AM_CRITICAL_END
            break;
        case AM_HAL_CACHECTRL_CONTROL_MONITOR_DISABLE:
            ui32SetMask = 0;
            AM_CRITICAL_BEGIN
            CPU->CACHECFG &= ~CPU_CACHECFG_ENABLEMONITOR_Msk;
            AM_CRITICAL_END
            break;
        case AM_HAL_CACHECTRL_CONTROL_NC_CFG:
        {
            if ( pArgs == NULL )
            {
                return AM_HAL_STATUS_INVALID_ARG;
            }

            am_hal_cachectrl_nc_cfg_t *pNcCfg;
            pNcCfg = (am_hal_cachectrl_nc_cfg_t *)pArgs;
#ifndef AM_HAL_DISABLE_API_VALIDATION
            // Make sure the addresses are valid
            if ((pNcCfg->ui32StartAddr & ~CPU_NCR0START_ADDR_Msk) ||
                (pNcCfg->ui32EndAddr & ~CPU_NCR0START_ADDR_Msk))
            {
                return AM_HAL_STATUS_INVALID_ARG;
            }
#endif // AM_HAL_DISABLE_API_VALIDATION
            if (pNcCfg->eNCRegion == AM_HAL_CACHECTRL_NCR0)
            {
                CPU->NCR0START = pNcCfg->ui32StartAddr;
                CPU->NCR0END   = pNcCfg->ui32EndAddr;
                CPU->CACHECFG_b.NC0ENABLE = pNcCfg->bEnable;
            }
            else if (pNcCfg->eNCRegion == AM_HAL_CACHECTRL_NCR1)
            {
                CPU->NCR1START = pNcCfg->ui32StartAddr;
                CPU->NCR1END   = pNcCfg->ui32EndAddr;
                CPU->CACHECFG_b.NC1ENABLE = pNcCfg->bEnable;
            }
#ifndef AM_HAL_DISABLE_API_VALIDATION
            else
            {
                return AM_HAL_STATUS_INVALID_ARG;
            }
#endif // AM_HAL_DISABLE_API_VALIDATION
            return AM_HAL_STATUS_SUCCESS;
        }
        default:
            return AM_HAL_STATUS_INVALID_ARG;
    }

    //
    // All fields in the CACHECTRL register are write-only or read-only.
    // A write to CACHECTRL acts as a mask-set.  That is, only the bits
    // written as '1' have an effect, any bits written as '0' are unaffected.
    //
    // Important note - setting of an enable and disable simultanously has
    // unpredicable results.
    //
    if ( ui32SetMask )
    {
        CPU->CACHECTRL = ui32SetMask;
    }

    return AM_HAL_STATUS_SUCCESS;

} // am_hal_cachectrl_control()

//*****************************************************************************
//
//  Cache controller status function
//
//*****************************************************************************
uint32_t
am_hal_cachectrl_status_get(am_hal_cachectrl_status_t *psStatus)
{
    if ( psStatus == NULL )
    {
        return AM_HAL_STATUS_INVALID_ARG;
    }

    //
    // Cache Ready Status
    //
    psStatus->bCacheReady = CPU->CACHECTRL_b.CACHEREADY;

    return AM_HAL_STATUS_SUCCESS;

} // am_hal_cachectrl_status_get()


//*****************************************************************************
//
// End Doxygen group.
//! @}
//
//*****************************************************************************
