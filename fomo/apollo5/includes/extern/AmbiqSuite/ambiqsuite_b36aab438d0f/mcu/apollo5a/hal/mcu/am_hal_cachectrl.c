//*****************************************************************************
//
//! @file am_hal_cachectrl.c
//!
//! @brief Functions for interfacing with the CACHE controller.
//!
//! @addtogroup cachectrl4 CACHE - Cache Control
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

//Enable the Instruction cache for operation.
uint32_t
am_hal_cachectrl_icache_enable(void)
{
    //
    // Check if cache was powered on, if not, return error.
    //
    if (PWRCTRL->MEMPWRSTATUS_b.PWRSTCACHE != 1)
    {
        return AM_HAL_STATUS_FAIL;
    }
    //
    // Call CMSIS function to enable I-Cache.
    //
    SCB_EnableICache();

    return AM_HAL_STATUS_SUCCESS;
}

//Disable the Instruction cache.
uint32_t
am_hal_cachectrl_icache_disable(void)
{
    //
    // Check if cache was powered on, if not, return error.
    //
    if (PWRCTRL->MEMPWRSTATUS_b.PWRSTCACHE != 1)
    {
        return AM_HAL_STATUS_FAIL;
    }
    //
    // Call CMSIS function to disable the I-Cache.
    //
    SCB_DisableICache();
    return AM_HAL_STATUS_SUCCESS;
}

//Enable the Data cache for operation.
uint32_t
am_hal_cachectrl_dcache_enable(bool bClean)
{
    //
    // Check if cache was powered on, if not, return error.
    //
    if (PWRCTRL->MEMPWRSTATUS_b.PWRSTCACHE != 1)
    {
        return AM_HAL_STATUS_FAIL;
    }
 // #### INTERNAL BEGIN ####
    //
    // SW Workaround for CAY-1179, RAXI Upsizer Pre-fetch issue.
    // Only 1 outstanding transaction is allowed in RevA. To be fixed in RevB.
    //
#warning "Verify that CAY-1179 is fixed in RevB."
// #### INTERNAL END ####
    //
    //  Set the CM55 Pre-Fetch Control for RevA.
    //
    if (APOLLO5_A0)
    {
        MEMSYSCTL->PFCR = _VAL2FLD(MEMSYSCTL_PFCR_MAX_OS, 1)    |
                          _VAL2FLD(MEMSYSCTL_PFCR_MAX_LA, 6)    |
                          _VAL2FLD(MEMSYSCTL_PFCR_MIN_LA, 2)    |
                          _VAL2FLD(MEMSYSCTL_PFCR_ENABLE, 1);
    }

    //
    // Call CMSIS function to enable the D-Cache.
    //
    SCB_EnableDCache();

    //
    // Check whether to clean the D-Cache or not.
    //
    if (bClean)
    {
        SCB_CleanDCache();
    }

    return AM_HAL_STATUS_SUCCESS;
}

//Disable the Data cache.
uint32_t
am_hal_cachectrl_dcache_disable(void)
{
    //
    // Check if cache was powered on, if not, return error.
    //
    if (PWRCTRL->MEMPWRSTATUS_b.PWRSTCACHE != 1)
    {
        return AM_HAL_STATUS_FAIL;
    }
    //
    // Call CMSIS function to disable the D-Cache.
    //
    SCB_DisableDCache();
    return AM_HAL_STATUS_SUCCESS;
}

//Invalidate the Insruction cache.
uint32_t
am_hal_cachectrl_icache_invalidate(am_hal_cachectrl_range_t *pRange)
{
    //
    // If I-Cache was not enabled, only do DSB and ISB operations.
    //
    if ((SCB->CCR & (uint32_t)SCB_CCR_IC_Msk) == 0)
    {
        __DSB();
        __ISB();

        return AM_HAL_STATUS_SUCCESS;
    }
    //
    // If no address range was provided, invalidate the entire icache.
    //
    if (NULL == pRange)
    {
        SCB_InvalidateICache();
    }
    else
    {
        SCB_InvalidateICache_by_Addr((void *)pRange->ui32StartAddr, pRange->ui32Size);
    }

    return AM_HAL_STATUS_SUCCESS;
}

//Invalidate the Data cache.
uint32_t
am_hal_cachectrl_dcache_invalidate(am_hal_cachectrl_range_t *pRange,
                                   bool bClean)
{
    //
    // If D-Cache was not enabled, only do DSB and ISB operations.
    //
    if ((SCB->CCR & (uint32_t)SCB_CCR_DC_Msk) == 0)
    {
        __DSB();
        __ISB();

        return AM_HAL_STATUS_SUCCESS;
    }
    //
    // If the address range is NULL then invalidate the entire D-Cache.
    //
    if (NULL == pRange)
    {
        if (bClean)
        {
            SCB_CleanInvalidateDCache();
        }
        else
        {
            SCB_InvalidateDCache();
        }
    }
    else
    {
        if (bClean)
        {
            SCB_CleanInvalidateDCache_by_Addr((void *)pRange->ui32StartAddr, pRange->ui32Size);
        }
        else
        {
            SCB_InvalidateDCache_by_Addr((void *)pRange->ui32StartAddr, pRange->ui32Size);
        }
    }

    return AM_HAL_STATUS_SUCCESS;
}

//Clean the Data cache.
uint32_t
am_hal_cachectrl_dcache_clean(am_hal_cachectrl_range_t *pRange)
{
    //
    // If D-Cache was not enabled, only do DSB and ISB operations.
    //
    if ((SCB->CCR & (uint32_t)SCB_CCR_DC_Msk) == 0)
    {
        __DSB();
        __ISB();

        return AM_HAL_STATUS_SUCCESS;
    }
    //
    // If the address range is NULL then clean the entire D-Cache.
    //
    if (NULL == pRange)
    {
        SCB_CleanDCache();
    }
    else
    {
        SCB_CleanDCache_by_Addr((void *)pRange->ui32StartAddr, pRange->ui32Size);
    }

    return AM_HAL_STATUS_SUCCESS;
}

//*****************************************************************************
//
// End Doxygen group.
//! @}
//
//*****************************************************************************
