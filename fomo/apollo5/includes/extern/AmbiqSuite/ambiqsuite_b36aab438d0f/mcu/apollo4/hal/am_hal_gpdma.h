//*****************************************************************************
//
//  am_hal_gpdma.h
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

#ifndef AM_HAL_GPDMA_H
#define AM_HAL_GPDMA_H

typedef struct
{
    bool     bHighPriority;
    uint32_t burstSize;
} am_hal_gpdma_config_t;

typedef struct
{
    bool bSrcStatic; // address is static (non-incrementing), e.g. for FIFO
    bool bDstStatic; // address is static (non-incrementing), e.g. for FIFO
    uint32_t srcAddress;
    uint32_t dstAddress;
    uint32_t numBytes;
} am_hal_gpdma_transfer_t;

typedef enum
{
    AM_HAL_GPDMA_STATUS_TIP,
    AM_HAL_GPDMA_STATUS_ERR,
    AM_HAL_GPDMA_STATUS_IDLE,
} am_hal_gpdma_state_e;

typedef struct
{
    am_hal_gpdma_state_e  state;
    uint32_t               numPendingBytes;
} am_hal_gpdma_status_t;

#ifdef __cplusplus
extern "C"
{
#endif

//
// Transfer callback function prototype
//
typedef void (*am_hal_transfer_callback_t)(void *pCallbackCtxt, uint32_t transferStatus);

extern uint32_t am_hal_gpdma_initialize(uint32_t channelId, void **pHandle);
extern uint32_t am_hal_gpdma_config(void *pHandle, am_hal_gpdma_config_t *pConfig);
extern uint32_t am_hal_gpdma_transfer(void *pHandle,
                                      am_hal_gpdma_transfer_t *pTransfer,
                                      am_hal_transfer_callback_t pfnCallback,
                                      void *pCallbackCtxt);
extern uint32_t am_hal_gpdma_get_status(void *pHandle, am_hal_gpdma_status_t *pStatus);
extern uint32_t am_hal_gpdma_uninitialize(void *pHandle);
// TODO - Interrupt missing
extern uint32_t am_hal_gpdma_interrupt_enable(void *pHandle, uint32_t ui32IntMask);
extern uint32_t am_hal_gpdma_interrupt_disable(void *pHandle, uint32_t ui32IntMask);
extern uint32_t am_hal_gpdma_interrupt_clear(void *pHandle, uint32_t ui32IntMask);
extern uint32_t am_hal_gpdma_interrupt_status_get(void *pHandle, bool bEnabledOnly,
                                                  uint32_t *pui32IntStatus);
extern uint32_t am_hal_gpdma_interrupt_service(void *pHandle, uint32_t ui32IntMask);

#ifdef __cplusplus
}
#endif

#endif // AM_HAL_GPDMA_H

//*****************************************************************************
//
// End Doxygen group.
//! @}
//
//*****************************************************************************
