//*****************************************************************************
//
//  am_hal_gpdma.c
//! @file
//!
//! @brief Functions for Memcopy DMA Engine
//!
//! @addtogroup gpdma4 Memcopy DMA
//! @ingroup apollo4hal
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

uint32_t am_hal_gpdma_initialize(uint32_t channelId, void **pHandle)
{
    return AM_HAL_STATUS_FAIL;
}
uint32_t am_hal_gpdma_config(void *pHandle, am_hal_gpdma_config_t *pConfig)
{
    return AM_HAL_STATUS_FAIL;
}
uint32_t am_hal_gpdma_transfer(void *pHandle,
                               am_hal_gpdma_transfer_t *pTransfer,
                               am_hal_transfer_callback_t pfnCallback,
                               void *pCallbackCtxt)
{
    return AM_HAL_STATUS_FAIL;
}
uint32_t am_hal_gpdma_get_status(void *pHandle, am_hal_gpdma_status_t *pStatus)
{
    return AM_HAL_STATUS_FAIL;
}
uint32_t am_hal_gpdma_uninitialize(void *pHandle)
{
    return AM_HAL_STATUS_FAIL;
}
uint32_t am_hal_gpdma_interrupt_enable(void *pHandle, uint32_t ui32IntMask)
{
    return AM_HAL_STATUS_FAIL;
}
uint32_t am_hal_gpdma_interrupt_disable(void *pHandle, uint32_t ui32IntMask)
{
    return AM_HAL_STATUS_FAIL;
}
uint32_t am_hal_gpdma_interrupt_clear(void *pHandle, uint32_t ui32IntMask)
{
    return AM_HAL_STATUS_FAIL;
}
uint32_t am_hal_gpdma_interrupt_status_get(void *pHandle, bool bEnabledOnly,
                                           uint32_t *pui32IntStatus)
{
    return AM_HAL_STATUS_FAIL;
}

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
