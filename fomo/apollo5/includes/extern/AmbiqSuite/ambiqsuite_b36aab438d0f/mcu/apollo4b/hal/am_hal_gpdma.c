//*****************************************************************************
//
//! @file am_hal_gpdma.c
//!
//! @brief Functions for General Purpose Memcopy DMA Engine
//!
//! @addtogroup gpdma4_4b GPDMA - General Purpose Memcopy DMA
//! @ingroup apollo4b_hal
//! @{

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
// GPDMA initialization function
//
//*****************************************************************************
uint32_t am_hal_gpdma_initialize(uint32_t channelId, void **pHandle)
{
    return AM_HAL_STATUS_FAIL;
}

//*****************************************************************************
//
// GPDMA configuration function
//
//*****************************************************************************
uint32_t am_hal_gpdma_config(void *pHandle, am_hal_gpdma_config_t *pConfig)
{
    return AM_HAL_STATUS_FAIL;
}

//*****************************************************************************
//
// General Purpose DMA Transfer
//
//*****************************************************************************
uint32_t am_hal_gpdma_transfer(void *pHandle,
                               am_hal_gpdma_transfer_t *pTransfer,
                               am_hal_transfer_callback_t pfnCallback,
                               void *pCallbackCtxt)
{
    return AM_HAL_STATUS_FAIL;
}

//*****************************************************************************
//
// General Purpose DMA Status Get
//
//*****************************************************************************
uint32_t am_hal_gpdma_get_status(void *pHandle, am_hal_gpdma_status_t *pStatus)
{
    return AM_HAL_STATUS_FAIL;
}

//*****************************************************************************
//
// General Purpose DMA Uninitialize
//
//*****************************************************************************
uint32_t am_hal_gpdma_uninitialize(void *pHandle)
{
    return AM_HAL_STATUS_FAIL;
}

//*****************************************************************************
//
// General Purpose DMA Interrupt Enable
//
//*****************************************************************************
uint32_t am_hal_gpdma_interrupt_enable(void *pHandle, uint32_t ui32IntMask)
{
    return AM_HAL_STATUS_FAIL;
}

//*****************************************************************************
//
// General Purpose DMA Interrupt Disable
//
//*****************************************************************************
uint32_t am_hal_gpdma_interrupt_disable(void *pHandle, uint32_t ui32IntMask)
{
    return AM_HAL_STATUS_FAIL;
}

//*****************************************************************************
//
// General Purpose DMA Interrupt Clear
//
//*****************************************************************************
uint32_t am_hal_gpdma_interrupt_clear(void *pHandle, uint32_t ui32IntMask)
{
    return AM_HAL_STATUS_FAIL;
}

//*****************************************************************************
//
// General Purpose DMA Status Get
//
//*****************************************************************************
uint32_t am_hal_gpdma_interrupt_status_get(void *pHandle, bool bEnabledOnly,
                                           uint32_t *pui32IntStatus)
{
    return AM_HAL_STATUS_FAIL;
}

//*****************************************************************************
//
// General Purpose DMA Interrupt Service
//
//*****************************************************************************
uint32_t am_hal_gpdma_interrupt_service(void *pHandle, uint32_t ui32IntMask)
{
    return AM_HAL_STATUS_FAIL;
}

//*****************************************************************************
//
// End Doxygen group.
//! @}
//
//*****************************************************************************
