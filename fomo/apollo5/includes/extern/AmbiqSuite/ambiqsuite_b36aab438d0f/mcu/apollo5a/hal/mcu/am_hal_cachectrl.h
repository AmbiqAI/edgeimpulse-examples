// ****************************************************************************
//
//! @file am_hal_cachectrl.h
//!
//! @brief Functions for interfacing with the CACHE controller.
//!
//! @addtogroup cachectrl4 CACHE - Cache Control
//! @ingroup apollo5a_hal
//! @{
//
// ****************************************************************************

// ****************************************************************************
//
// ${copyright}
//
// This is part of revision ${version} of the AmbiqSuite Development Package.
//
// ****************************************************************************
#ifndef AM_HAL_CACHECTRL_H
#define AM_HAL_CACHECTRL_H

#ifdef __cplusplus
extern "C"
{
#endif

//! Cache invalidate range.
typedef struct
{
    uint32_t    ui32StartAddr;      // Starting address of block to invalidate.
    uint32_t    ui32Size;           // Size of block to invalidate in bytes.
} am_hal_cachectrl_range_t;

// ****************************************************************************
//
// Function prototypes
//
// ****************************************************************************
// ****************************************************************************
//
//! @brief Enable the Instruction cache for operation.
//!
//! @return Status.
//
// ****************************************************************************
extern uint32_t am_hal_cachectrl_icache_enable(void);

// ****************************************************************************
//
//! @brief Disable the Instruction cache.
//!
//! Use this function to disable icache.  Other configuration settings are not
//! not required.
//!
//! @return Status.
//
// ****************************************************************************
extern uint32_t am_hal_cachectrl_icache_disable(void);

// ****************************************************************************
//
//! @brief Enable the Data cache for operation.
//!
//! @param bClean - TRUE: Clean the dcache in addition to invalidating it.
//!                 FALSE: Simply invalidate the cache.
//!
//! @return Status.
//
// ****************************************************************************
extern uint32_t am_hal_cachectrl_dcache_enable(bool bClean);

// ****************************************************************************
//
//! @brief Disable the Data cache.
//!
//! Use this function to disable dcache.  Other configuration settings are not
//! not required.
//!
//! @return Status.
//
// ****************************************************************************
extern uint32_t am_hal_cachectrl_dcache_disable(void);

// ****************************************************************************
//
//! @brief Invalidate the Insruction cache.
//!
//! @param pRange - pointer to an address range to invalidate.  If NULL, then
//!                 invalidate the entire cache.
//!
//! Use this function to invalidate the icache.
//!
//! @return Status.
//
// ****************************************************************************
extern uint32_t am_hal_cachectrl_icache_invalidate(am_hal_cachectrl_range_t *pRange);

// ****************************************************************************
//
//! @brief Invalidate the Data cache.
//!
//! @param pRange - pointer to an address range to invalidate.  If NULL, then
//!                 invalidate the entire cache.
//! @param bClean - TRUE: Clean the dcache in addition to invalidating it.
//!                 FALSE: Simply invalidate the cache.
//!
//! Use this function to invalidate the dcache.
//!
//! @return Status.
//
// ****************************************************************************
extern uint32_t am_hal_cachectrl_dcache_invalidate(am_hal_cachectrl_range_t *pRange,
                                                   bool bClean);

// ****************************************************************************
//
//! @brief Clean the Data cache.
//!
//! @param pRange - pointer to an address range to clean. If NULL, then
//!                 clean the entire cache.
//!
//! Use this function to clean the dcache.
//!
//! @return Status.
//
// ****************************************************************************
extern uint32_t am_hal_cachectrl_dcache_clean(am_hal_cachectrl_range_t *pRange);

#ifdef __cplusplus
}
#endif

#endif // AM_HAL_CACHECTRL_H

//*****************************************************************************
//
// End Doxygen group.
//! @}
//
//*****************************************************************************
