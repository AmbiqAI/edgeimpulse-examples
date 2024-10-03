//*****************************************************************************
//
//! @file am_hal_uart.c
//!
//! @brief Hardware abstraction for the UART
//!
//! @addtogroup uart UART Functionality
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

//*****************************************************************************
//
//! UART magic number for handle verification.
//
//*****************************************************************************
#define AM_HAL_MAGIC_UART               0xEA9E06

#define AM_HAL_UART_CHK_HANDLE(h)                                             \
    ((h) &&                                                                   \
     ((am_hal_handle_prefix_t *)(h))->s.bInit &&                              \
     (((am_hal_handle_prefix_t *)(h))->s.magic == AM_HAL_MAGIC_UART))

//*****************************************************************************
//
//! UART FIFO Size
//
//*****************************************************************************
#define AM_HAL_UART_FIFO_MAX 32

//*****************************************************************************
//
//! Delay constant
//
//*****************************************************************************
// #### INTERNAL BEGIN ####
// TODO: Fix the calculation for this delay time.
// 8/20/21: Activated after reviewed by CV.
// 7/27/21: The following #if 0 computation implemented but not activated
//          until it can be reviewed. After review, be sure to activate for
//          both Apollo4b & 4p HALs.
#if 1
//
// At 115200: 1 bit time = 1 / 115200 = 8.68us
// The integer number of us is therefore: 1000000 / 115200 = 8
//
// #### INTERNAL END ####
#define ONE_BYTE_DELAY(pState)                                              \
    if (1)                                                                  \
    {                                                                       \
        uint32_t delayus;                                                   \
        delayus = 1000000 / pState->ui32BaudRate;   /* Delay for 1 bit  */  \
        delayus += 1;                               /* Round up         */  \
        delayus *= 10;                              /* 8 bits,start/stop*/  \
        am_hal_delay_us(delayus);                                           \
    }
// #### INTERNAL BEGIN ####
#else
#define ONE_BYTE_DELAY(pState)      am_hal_delay_us(10);
#endif
// #### INTERNAL END ####

#define RETURN_ON_ERROR(x)                                                    \
    if ((x) != AM_HAL_STATUS_SUCCESS)                                         \
    {                                                                         \
        return (x);                                                           \
    };
//*****************************************************************************
//
//! Structure for handling UART register state information for power up/down
//
//*****************************************************************************
typedef struct
{
    bool bValid;
    uint32_t regILPR;
    uint32_t regIBRD;
    uint32_t regFBRD;
    uint32_t regLCRH;
    uint32_t regCR;
    uint32_t regIFLS;
    uint32_t regIER;
    uint32_t regDMACFG;
}
am_hal_uart_register_state_t;

//*****************************************************************************
//
//! Structure for handling UART HAL state information.
//
//*****************************************************************************
typedef struct
{
    //
    //! For internal verification purposes
    //
    am_hal_handle_prefix_t prefix;

    //
    //! Register state for power-up/power-down
    //
    am_hal_uart_register_state_t sRegState;

    //
    //! UART module number.
    //
    uint32_t ui32Module;

    //
    //! Most recently configured baud rate. (Do we really need this?)
    //
    uint32_t ui32BaudRate;

    //
    //! State information about nonblocking transfers.
    //
    bool bCurrentlyReading;
    am_hal_uart_transfer_t sActiveRead;
    volatile uint32_t ui32BytesRead;

    bool bCurrentlyWriting;
    am_hal_uart_transfer_t sActiveWrite;
    volatile uint32_t ui32BytesWritten;

    //
    //! Queued write/read implementation
    //
    bool bEnableTxQueue;
    am_hal_queue_t sTxQueue;

    bool bEnableRxQueue;
    am_hal_queue_t sRxQueue;

    //
    //! Cleared when any transmit transaction is started. Set after TX complete
    //! interrupt is received.
    //
    volatile bool bLastTxComplete;

    //
    // Stores the callback and context.
    //
    am_hal_uart_callback_t   pfnCallback;
    void                    *pCallbackCtxt;

    //
    //! DMA transaction in progress.
    //
    bool                bDMABusy;

    //
    //! DMA transaction Tranfer Control Buffer.
    //
    uint32_t            ui32BufferPtr;
    uint32_t            ui32BufferSizeBytes;
}
am_hal_uart_state_t;

// Create one state structure for each physical UART.
am_hal_uart_state_t g_am_hal_uart_states[AM_REG_UART_NUM_MODULES];

//*****************************************************************************
//
// Static function prototypes.
//
//*****************************************************************************
static uint32_t config_baudrate(uint32_t ui32Module,
                                uint32_t ui32DesiredBaudrate,
                                uint32_t *pui32ActualBaud);

static uint32_t blocking_write(void *pHandle,
                               const am_hal_uart_transfer_t *psTransfer);

static uint32_t blocking_read(void *pHandle,
                              const am_hal_uart_transfer_t *psTransfer);

static uint32_t nonblocking_write(void *pHandle,
                                  const am_hal_uart_transfer_t *psTransfer);

static uint32_t nonblocking_read(void *pHandle,
                                 const am_hal_uart_transfer_t *psTransfer);

static void nonblocking_write_sm(void *pHandle);
static void nonblocking_read_sm(void *pHandle);
static uint32_t tx_queue_update(void *pHandle);
static uint32_t rx_queue_update(void *pHandle);
//*****************************************************************************
//
// Initialize the UART
//
//*****************************************************************************
uint32_t
am_hal_uart_initialize(uint32_t ui32Module, void **ppHandle)
{
#ifndef AM_HAL_DISABLE_API_VALIDATION
    //
    // Check that the request module is in range.
    //
    if (ui32Module >= AM_REG_UART_NUM_MODULES )
    {
        return AM_HAL_STATUS_OUT_OF_RANGE;
    }

    //
    // Check for valid arguements.
    //
    if (!ppHandle)
    {
        return AM_HAL_STATUS_INVALID_ARG;
    }

    //
    // Make sure we're not re-activating an active handle.
    //
    if (AM_HAL_UART_CHK_HANDLE(*ppHandle))
    {
        return AM_HAL_STATUS_INVALID_OPERATION;
    }
#endif
    //
    // Initialize the handle.
    //
    g_am_hal_uart_states[ui32Module].prefix.s.bInit = true;
    g_am_hal_uart_states[ui32Module].prefix.s.magic = AM_HAL_MAGIC_UART;
    g_am_hal_uart_states[ui32Module].ui32Module = ui32Module;
    g_am_hal_uart_states[ui32Module].sRegState.bValid = false;
    g_am_hal_uart_states[ui32Module].ui32BaudRate = 0;
    g_am_hal_uart_states[ui32Module].bCurrentlyReading = 0;
    g_am_hal_uart_states[ui32Module].bCurrentlyWriting = 0;
    g_am_hal_uart_states[ui32Module].bEnableTxQueue = 0;
    g_am_hal_uart_states[ui32Module].bEnableRxQueue = 0;
    g_am_hal_uart_states[ui32Module].ui32BytesWritten = 0;
    g_am_hal_uart_states[ui32Module].ui32BytesRead = 0;
    g_am_hal_uart_states[ui32Module].bLastTxComplete = true;

    //
    // Return the handle.
    //
    *ppHandle = (void *)&g_am_hal_uart_states[ui32Module];

    //
    // Return the status.
    //
    return AM_HAL_STATUS_SUCCESS;
}

//*****************************************************************************
//
// Deinitialize the UART interface.
//
//*****************************************************************************
uint32_t
am_hal_uart_deinitialize(void *pHandle)
{
    am_hal_uart_state_t *pState = (am_hal_uart_state_t *)pHandle;

#ifndef AM_HAL_DISABLE_API_VALIDATION
    //
    // Check the handle.
    //
    if (!AM_HAL_UART_CHK_HANDLE(pHandle))
    {
        return AM_HAL_STATUS_INVALID_HANDLE;
    }
#endif
    //
    // Reset the handle.
    //
    pState->prefix.s.bInit = false;
    pState->prefix.s.magic = 0;
    pState->ui32Module = 0;
    pState->sRegState.bValid = false;

    //
    // Return the status.
    //
    return AM_HAL_STATUS_SUCCESS;
}

//*****************************************************************************
//
// Change the power state of the UART module.
//
//*****************************************************************************
uint32_t
am_hal_uart_power_control(void *pHandle, uint32_t ePowerState,
                          bool bRetainState)
{
    am_hal_uart_state_t *pState = (am_hal_uart_state_t *) pHandle;
    uint32_t ui32Module = pState->ui32Module;

    am_hal_pwrctrl_periph_e eUARTPowerModule = ((am_hal_pwrctrl_periph_e)
                                                (AM_HAL_PWRCTRL_PERIPH_UART0 +
                                                 ui32Module));

#ifndef AM_HAL_DISABLE_API_VALIDATION
    //
    // Check to make sure this is a valid handle.
    //
    if (!AM_HAL_UART_CHK_HANDLE(pHandle))
    {
        return AM_HAL_STATUS_INVALID_HANDLE;
    }
#endif
    //
    // Decode the requested power state and update UART operation accordingly.
    //
    switch (ePowerState)
    {
        //
        // Turn on the UART.
        //
        case AM_HAL_SYSCTRL_WAKE:
            //
            // Make sure we don't try to restore an invalid state.
            //
            if (bRetainState && !pState->sRegState.bValid)
            {
                return AM_HAL_STATUS_INVALID_OPERATION;
            }

            //
            // Enable power control.
            //
            am_hal_pwrctrl_periph_enable(eUARTPowerModule);

            if (bRetainState)
            {
                //
                // Restore UART registers
                //
                // AM_CRITICAL_BEGIN

                UARTn(ui32Module)->ILPR = pState->sRegState.regILPR;
                UARTn(ui32Module)->IBRD = pState->sRegState.regIBRD;
                UARTn(ui32Module)->FBRD = pState->sRegState.regFBRD;
                UARTn(ui32Module)->LCRH = pState->sRegState.regLCRH;
                UARTn(ui32Module)->CR   = pState->sRegState.regCR;
                UARTn(ui32Module)->IFLS = pState->sRegState.regIFLS;
                UARTn(ui32Module)->IER  = pState->sRegState.regIER;
                UARTn(ui32Module)->DCR  = pState->sRegState.regDMACFG;
                pState->sRegState.bValid = false;

                // AM_CRITICAL_END
            }
            break;

        //
        // Turn off the UART.
        //
        case AM_HAL_SYSCTRL_NORMALSLEEP:
        case AM_HAL_SYSCTRL_DEEPSLEEP:
            if (bRetainState)
            {
                // AM_CRITICAL_BEGIN

                pState->sRegState.regILPR = UARTn(ui32Module)->ILPR;
                pState->sRegState.regIBRD = UARTn(ui32Module)->IBRD;
                pState->sRegState.regFBRD = UARTn(ui32Module)->FBRD;
                pState->sRegState.regLCRH = UARTn(ui32Module)->LCRH;
                pState->sRegState.regCR   = UARTn(ui32Module)->CR;
                pState->sRegState.regIFLS = UARTn(ui32Module)->IFLS;
                pState->sRegState.regIER  = UARTn(ui32Module)->IER;
                pState->sRegState.regDMACFG = UARTn(ui32Module)->DCR;
                pState->sRegState.bValid = true;

                // AM_CRITICAL_END
            }

            //
            // Clear all interrupts before sleeping as having a pending UART
            // interrupt burns power.
            //
            am_hal_uart_interrupt_clear(pState, 0xFFFFFFFF);

            //
            // If the user is going to sleep, certain bits of the CR register
            // need to be 0 to be low power and have the UART shut off.
            // Since the user either wishes to retain state which takes place
            // above or the user does not wish to retain state, it is acceptable
            // to set the entire CR register to 0.
            //
            UARTn(ui32Module)->CR = 0;

            //
            // Disable power control.
            //
            am_hal_pwrctrl_periph_disable(eUARTPowerModule);
            break;

        default:
            return AM_HAL_STATUS_INVALID_ARG;
    }

    //
    // Return the status.
    //
    return AM_HAL_STATUS_SUCCESS;
} // am_hal_uart_power_control()

//*****************************************************************************
//
//! @brief UART DMA configuration function
//!
//! @param pHandle    - handle for the module instance.
//! @param pDMAConfig - pointer to the configuration structure.
//!
//! This function configures the UART DMA for operation.
//!
//! @return status    - generic or interface specific status.
//
//*****************************************************************************
void
am_hal_uart_dma_configure(void *pHandle, am_hal_uart_transfer_t *pTransferCfg)
{
    am_hal_uart_state_t* pUARTState = (am_hal_uart_state_t *)pHandle;
    uint32_t ui32Module = pUARTState->ui32Module;

    //
    // Save the buffers.
    //
    if ( pTransferCfg->eDirection == AM_HAL_UART_TX )
    {
        pUARTState->ui32BufferPtr = (uint32_t)pTransferCfg->pui32TxBuffer;
    }
    else
    {
        pUARTState->ui32BufferPtr = (uint32_t)pTransferCfg->pui32RxBuffer;
    }
    pUARTState->ui32BufferSizeBytes = pTransferCfg->ui32NumBytes;
    pUARTState->pfnCallback = pTransferCfg->pfnCallback;
    pUARTState->pCallbackCtxt = pTransferCfg->pvContext;

    //
    // Clear the interrupts
    //
    UARTn(ui32Module)->IEC |= UART0_IEC_DMACPIC_Pos | UART0_IEC_DMAEIC_Pos;

    //
    // Configure the DMA static priority.
    //
    UARTn(ui32Module)->DCR_b.DMAPRI = pTransferCfg->ui8Priority;

    //
    // Set the DMA transfer count.
    //
    UARTn(ui32Module)->COUNT_b.TOTCOUNT = pTransferCfg->ui32NumBytes;

    //
    // Set the DMA target address.
    //
    UARTn(ui32Module)->TARGADDR = pUARTState->ui32BufferPtr;
}

//*****************************************************************************
//
// UART DMA nonblocking transfer function
//
//*****************************************************************************
void
am_hal_uart_dma_transfer_start(void *pHandle,   am_hal_uart_transfer_t *pTransferCfg)
{
    am_hal_uart_state_t *pState = (am_hal_uart_state_t *) pHandle;
    uint32_t ui32Module = pState->ui32Module;

    //
    // Enable the DMA.
    //
    if (pTransferCfg->eDirection == AM_HAL_UART_TX)
    {
        UARTn(ui32Module)->DCR_b.TXDMAE = 1;
    }
    else
    {
        UARTn(ui32Module)->DCR_b.RXDMAE = 1;
    }
}

//*****************************************************************************
//
// am_hal_uart_dma_transfer_complete
//
//*****************************************************************************
void
am_hal_uart_dma_transfer_complete(void *pHandle)
{
    am_hal_uart_state_t *pState = (am_hal_uart_state_t *) pHandle;
    uint32_t ui32Module = pState->ui32Module;
    //
    // Once completed, software must first write the DCR register to 0,
    // prior to making any update
    //
    UARTn(ui32Module)->DCR = 0x0;
    //
    // Clear dma status.
    //
    UARTn(ui32Module)->RSR_b.DMACPL = 0x0;
    UARTn(ui32Module)->RSR_b.DMAERR = 0x0;
}

//*****************************************************************************
//
//! @brief Validate an UART transaction.
//!
//! @param pUARTState     - pointer to the UART internal state.
//! @param psTransaction  - pointer to UART transaction.
//!
//! This function validates.
//!
//! @return HAL status of the operation.
//
//*****************************************************************************
uint32_t
am_hal_uart_validate_transaction(am_hal_uart_transfer_t *psTransaction)
{
    uint32_t ui32Bytes = psTransaction->ui32NumBytes;
    uint32_t ui32Dir = psTransaction->eDirection;

    if ((ui32Bytes && (ui32Dir != AM_HAL_UART_TX) && (psTransaction->pui32RxBuffer == NULL)) ||
         (ui32Bytes && (ui32Dir != AM_HAL_UART_RX) && (psTransaction->pui32TxBuffer == NULL)))
    {
        return AM_HAL_STATUS_INVALID_ARG;
    }
    return AM_HAL_STATUS_SUCCESS;
}

//*****************************************************************************
//
// UART non-blocking transfer function
//
//*****************************************************************************
uint32_t
am_hal_uart_dma_transfer(void *pHandle,
                         am_hal_uart_transfer_t *psTransaction)
{
    am_hal_uart_state_t *pUARTState = (am_hal_uart_state_t*)pHandle;
    uint32_t ui32Status = AM_HAL_STATUS_SUCCESS;

#ifndef AM_HAL_DISABLE_API_VALIDATION
    if ( !AM_HAL_UART_CHK_HANDLE(pHandle) )
    {
        return AM_HAL_STATUS_INVALID_HANDLE;
    }

    if ( !psTransaction )
    {
        return AM_HAL_STATUS_INVALID_ARG;
    }

    //
    // Validate parameters
    //
    ui32Status = am_hal_uart_validate_transaction(psTransaction);

    if (ui32Status != AM_HAL_STATUS_SUCCESS)
    {
        return ui32Status;
    }
#endif // AM_HAL_DISABLE_API_VALIDATION

    if ( pUARTState->bDMABusy )
    {
        return AM_HAL_STATUS_IN_USE;
    }

    am_hal_uart_dma_configure(pHandle, psTransaction);

    am_hal_uart_dma_transfer_start(pHandle, psTransaction);

    pUARTState->bDMABusy = true;

    return AM_HAL_STATUS_SUCCESS;
}

//*****************************************************************************
//
// am_hal_uart_dma_error
//
//*****************************************************************************
uint32_t
am_hal_uart_dma_error(void *pHandle)
{
    am_hal_uart_state_t *pState = (am_hal_uart_state_t *) pHandle;
    uint32_t ui32Module = pState->ui32Module;
    //
    // If an error condition did occur during a DMA operation, the DMA must first be disabled
    //
    UARTn(ui32Module)->DCR = 0x0;
    //
    // DMA status bits cleared.
    //
    UARTn(ui32Module)->RSR_b.DMAERR = 0x0;

    return AM_HAL_STATUS_SUCCESS;
}

//*****************************************************************************
//
// Error handling.
//
//*****************************************************************************
uint32_t
internal_uart_get_int_err(uint32_t ui32Module, uint32_t ui32IntStatus)
{
    //
    // Map the INTSTAT bits for transaction status
    //
    uint32_t ui32Status = AM_HAL_STATUS_SUCCESS;

    //
    // Let's accumulate the errors
    //
    ui32IntStatus |= UARTn(ui32Module)->IES;

    if (ui32IntStatus & AM_HAL_UART_INT_RX_TMOUT)
    {
        ui32Status = AM_HAL_UART_STATUS_RX_TIMEOUT;
    }
    else if (ui32IntStatus & AM_HAL_UART_INT_FRAME_ERR)
    {
        ui32Status = AM_HAL_UART_STATUS_FRAME_ERROR;
    }
    else if (ui32IntStatus & AM_HAL_UART_INT_PARITY_ERR)
    {
        ui32Status = AM_HAL_UART_STATUS_PARITY_ERROR;
    }
    else if (ui32IntStatus & AM_HAL_UART_INT_BREAK_ERR)
    {
        ui32Status = AM_HAL_UART_STATUS_BREAK_ERROR;
    }
    else if (ui32IntStatus & AM_HAL_UART_INT_OVER_RUN)
    {
        ui32Status = AM_HAL_UART_STATUS_OVERRUN_ERROR;
    }
    else if (ui32IntStatus & AM_HAL_UART_INT_DMAERIS)
    {
        ui32Status = AM_HAL_UART_STATUS_DMA_ERROR;
    }

    return ui32Status;
} // internal_uart_get_int_err()

//*****************************************************************************
//
// Allows the UART HAL to use extra space to store TX and RX data.
//
//*****************************************************************************
uint32_t
am_hal_uart_buffer_configure(void *pHandle, uint8_t *pui8TxBuffer,
                             uint32_t ui32TxBufferSize,
                             uint8_t *pui8RxBuffer, uint32_t ui32RxBufferSize)
{
    am_hal_uart_state_t *pState = (am_hal_uart_state_t *) pHandle;
    uint32_t ui32ErrorStatus = 0;

#ifndef AM_HAL_DISABLE_API_VALIDATION
    //
    // Check to make sure this is a valid handle.
    //
    if (!AM_HAL_UART_CHK_HANDLE(pHandle))
    {
        return AM_HAL_STATUS_INVALID_HANDLE;
    }
#endif
    //
    // Check to see if we have a TX buffer.
    //
    if (pui8TxBuffer && ui32TxBufferSize)
    {
        //
        // If so, initialzie the transmit queue, and enable the TX FIFO
        // interrupt.
        //
        pState->bEnableTxQueue = true;
        am_hal_queue_init(&pState->sTxQueue, pui8TxBuffer, 1, ui32TxBufferSize);
        RETURN_ON_ERROR(ui32ErrorStatus);
    }
    else
    {
        //
        // If not, make sure the TX FIFO interrupt is disabled.
        //
        pState->bEnableTxQueue = false;
        RETURN_ON_ERROR(ui32ErrorStatus);
    }

    //
    // Check to see if we have an RX buffer.
    //
    if (pui8RxBuffer && ui32RxBufferSize)
    {
        //
        // If so, initialize the receive queue and the associated interupts.
        //
        pState->bEnableRxQueue = true;
        am_hal_queue_init(&pState->sRxQueue, pui8RxBuffer, 1, ui32RxBufferSize);
        RETURN_ON_ERROR(ui32ErrorStatus);
    }
    else
    {
        pState->bEnableRxQueue = false;
        RETURN_ON_ERROR(ui32ErrorStatus);
    }

    return AM_HAL_STATUS_SUCCESS;
} // buffer_configure()

//*****************************************************************************
//
// Set Baud Rate based on the UART clock frequency.
//
//*****************************************************************************
#define BAUDCLK     (16) // Number of UART clocks needed per bit.
static uint32_t
config_baudrate(uint32_t ui32Module, uint32_t ui32DesiredBaudrate, uint32_t *pui32ActualBaud)
{
    uint64_t ui64FractionDivisorLong;
    uint64_t ui64IntermediateLong;
    uint32_t ui32IntegerDivisor;
    uint32_t ui32FractionDivisor;
    uint32_t ui32BaudClk;
    uint32_t ui32UartClkFreq;

    //
    // Check that the baudrate is in range.
    //
    switch ( UARTn(ui32Module)->CR_b.CLKSEL )
    {
        case UART0_CR_CLKSEL_48MHZ:
            ui32UartClkFreq = 48000000;
            break;

        case UART0_CR_CLKSEL_24MHZ:
            ui32UartClkFreq = 24000000;
            break;

        case UART0_CR_CLKSEL_12MHZ:
            ui32UartClkFreq = 12000000;
            break;

        case UART0_CR_CLKSEL_6MHZ:
            ui32UartClkFreq = 6000000;
            break;

        case UART0_CR_CLKSEL_3MHZ:
            ui32UartClkFreq = 3000000;
            break;

        default:
            *pui32ActualBaud = 0;
            return AM_HAL_UART_STATUS_CLOCK_NOT_CONFIGURED;
    }

    //
    // Calculate register values.
    //
    ui32BaudClk = BAUDCLK * ui32DesiredBaudrate;
    ui32IntegerDivisor = (uint32_t)(ui32UartClkFreq / ui32BaudClk);
    ui64IntermediateLong = (ui32UartClkFreq * 64) / ui32BaudClk; // Q58.6
    ui64FractionDivisorLong = ui64IntermediateLong - (ui32IntegerDivisor * 64); // Q58.6
    ui32FractionDivisor = (uint32_t)ui64FractionDivisorLong;

    //
    // Check the result.
    //
    if (ui32IntegerDivisor == 0)
    {
        *pui32ActualBaud = 0;
        return AM_HAL_UART_STATUS_BAUDRATE_NOT_POSSIBLE;
    }

    //
    // Write the UART regs.
    //
    UARTn(ui32Module)->IBRD = ui32IntegerDivisor;
    UARTn(ui32Module)->FBRD = ui32FractionDivisor;

    //
    // Return the actual baud rate.
    //
    *pui32ActualBaud = (ui32UartClkFreq / ((BAUDCLK * ui32IntegerDivisor) + ui32FractionDivisor / 4));
    return AM_HAL_STATUS_SUCCESS;
} // config_baudrate()

//*****************************************************************************
//
// Save a transaction to the UART state.
//
//*****************************************************************************
uint32_t
write_transaction_save(am_hal_uart_state_t *psState,
                       const am_hal_uart_transfer_t *psTransfer)
{
    uint32_t ui32Error = AM_HAL_STATUS_SUCCESS;

    AM_CRITICAL_BEGIN

    if (psState->bCurrentlyWriting == false)
    {
        psState->bCurrentlyWriting = true;
        psState->sActiveWrite.eType = psTransfer->eType;
        psState->sActiveWrite.pui8Data = psTransfer->pui8Data;
        psState->sActiveWrite.ui32NumBytes = psTransfer->ui32NumBytes;
        psState->sActiveWrite.pui32BytesTransferred = psTransfer->pui32BytesTransferred;
        psState->sActiveWrite.ui32TimeoutMs = psTransfer->ui32TimeoutMs;
        psState->sActiveWrite.pfnCallback = psTransfer->pfnCallback;
        psState->sActiveWrite.pvContext = psTransfer->pvContext;
        psState->sActiveWrite.ui32ErrorStatus = psTransfer->ui32ErrorStatus;
        psState->ui32BytesWritten = 0;
        psState->bLastTxComplete = false;
    }
    else
    {
        ui32Error = AM_HAL_UART_STATUS_TX_CHANNEL_BUSY;
    }

    AM_CRITICAL_END

    return ui32Error;
}

//*****************************************************************************
//
// Save a transaction to the UART state.
//
//*****************************************************************************
uint32_t
read_transaction_save(am_hal_uart_state_t *psState,
                      const am_hal_uart_transfer_t *psTransfer)
{
    uint32_t ui32Error = AM_HAL_STATUS_SUCCESS;

    AM_CRITICAL_BEGIN

    if (psState->bCurrentlyReading == false)
    {
        psState->bCurrentlyReading = true;
        psState->sActiveRead.eType = psTransfer->eType;
        psState->sActiveRead.pui8Data = psTransfer->pui8Data;
        psState->sActiveRead.ui32NumBytes = psTransfer->ui32NumBytes;
        psState->sActiveRead.pui32BytesTransferred = psTransfer->pui32BytesTransferred;
        psState->sActiveRead.ui32TimeoutMs = psTransfer->ui32TimeoutMs;
        psState->sActiveRead.pfnCallback = psTransfer->pfnCallback;
        psState->sActiveRead.pvContext = psTransfer->pvContext;
        psState->sActiveRead.ui32ErrorStatus = psTransfer->ui32ErrorStatus;
        psState->ui32BytesRead = 0;
    }
    else
    {
        ui32Error = AM_HAL_UART_STATUS_RX_CHANNEL_BUSY;
    }

    AM_CRITICAL_END

    return ui32Error;
}

//*****************************************************************************
//
// Used to configure basic UART settings.
//
//*****************************************************************************
uint32_t
am_hal_uart_configure(void *pHandle, const am_hal_uart_config_t *psConfig)
{
    am_hal_uart_state_t *pState = (am_hal_uart_state_t *) pHandle;
    uint32_t ui32Module = pState->ui32Module;

    uint32_t ui32ErrorStatus;

#ifndef AM_HAL_DISABLE_API_VALIDATION
    //
    // Check to make sure this is a valid handle.
    //
    if (!AM_HAL_UART_CHK_HANDLE(pHandle))
    {
        return AM_HAL_STATUS_INVALID_HANDLE;
    }
#endif
    //
    // Reset the CR register to a known value.
    //
    UARTn(ui32Module)->CR = 0;

    //
    // Start by enabling the clocks, which needs to happen in a critical
    // section.
    //
    // AM_CRITICAL_BEGIN
    UARTn(ui32Module)->CR_b.CLKEN = 1;
    if ( psConfig->ui32BaudRate > 1500000 )
    {
        UARTn(ui32Module)->CR_b.CLKSEL = UART0_CR_CLKSEL_48MHZ;
    }
    else
    {
        UARTn(ui32Module)->CR_b.CLKSEL = UART0_CR_CLKSEL_24MHZ;
    }
    // AM_CRITICAL_END

    //
    // Disable the UART.
    //
    // AM_CRITICAL_BEGIN
    UARTn(ui32Module)->CR_b.UARTEN = 0;
    UARTn(ui32Module)->CR_b.RXE = 0;
    UARTn(ui32Module)->CR_b.TXE = 0;
    // AM_CRITICAL_END

    //
    // Set the baud rate.
    //
    ui32ErrorStatus = config_baudrate(ui32Module, psConfig->ui32BaudRate,
                                      &(pState->ui32BaudRate));

    if (ui32ErrorStatus != AM_HAL_STATUS_SUCCESS)
    {
        return ui32ErrorStatus;
    }

    //
    // Set the flow control options
    //
    // AM_CRITICAL_BEGIN
    UARTn(ui32Module)->CR_b.RTSEN = 0;
    UARTn(ui32Module)->CR_b.CTSEN = 0;
    UARTn(ui32Module)->CR |= psConfig->eFlowControl;
    // AM_CRITICAL_END

    //
    // Calculate the parity options.
    //
    uint32_t ui32ParityEnable = 0;
    uint32_t ui32EvenParity = 0;

    switch (psConfig->eParity)
    {
        case AM_HAL_UART_PARITY_ODD:
            ui32ParityEnable = 1;
            ui32EvenParity = 0;
            break;

        case AM_HAL_UART_PARITY_EVEN:
            ui32ParityEnable = 1;
            ui32EvenParity = 1;
            break;

        case AM_HAL_UART_PARITY_NONE:
            ui32ParityEnable = 0;
            ui32EvenParity = 0;
            break;
    }

    //
    // Set the data format.
    //
    // AM_CRITICAL_BEGIN
    UARTn(ui32Module)->LCRH_b.BRK  = 0;
    UARTn(ui32Module)->LCRH_b.PEN  = ui32ParityEnable;
    UARTn(ui32Module)->LCRH_b.EPS  = ui32EvenParity;
    UARTn(ui32Module)->LCRH_b.STP2 = psConfig->eStopBits;
    UARTn(ui32Module)->LCRH_b.FEN  = 1;
    UARTn(ui32Module)->LCRH_b.WLEN = psConfig->eDataBits;
    UARTn(ui32Module)->LCRH_b.SPS  = 0;
    // AM_CRITICAL_END

    //
    // Set the FIFO levels.
    //
    // AM_CRITICAL_BEGIN
    UARTn(ui32Module)->IFLS_b.TXIFLSEL = psConfig->eTXFifoLevel;
    UARTn(ui32Module)->IFLS_b.RXIFLSEL = psConfig->eRXFifoLevel;
    // AM_CRITICAL_END

    //
    // Enable the UART, RX, and TX.
    //
    // AM_CRITICAL_BEGIN
    UARTn(ui32Module)->CR_b.UARTEN = 1;
    UARTn(ui32Module)->CR_b.RXE = 1;
    UARTn(ui32Module)->CR_b.TXE = 1;
    // AM_CRITICAL_END

    return AM_HAL_STATUS_SUCCESS;
}

//*****************************************************************************
//
// Read from the UART RX FIFO.
//
//*****************************************************************************
uint32_t
am_hal_uart_fifo_read(void *pHandle, uint8_t *pui8Data, uint32_t ui32NumBytes,
                      uint32_t *pui32NumBytesRead)
{
    uint32_t i = 0;
    uint32_t ui32ReadData;
    uint32_t ui32ErrorStatus = AM_HAL_STATUS_SUCCESS;

    am_hal_uart_state_t *pState = (am_hal_uart_state_t *) pHandle;
    uint32_t ui32Module = pState->ui32Module;

    //
    // Start a loop where we attempt to read everything requested.
    //
    while (i < ui32NumBytes)
    {
        //
        // If the fifo is empty, return with the number of bytes we read.
        // Otherwise, read the data into the provided buffer.
        //
        if ( UARTn(ui32Module)->FR_b.RXFE )
        {
            break;
        }
        else
        {
            ui32ReadData = UARTn(ui32Module)->DR;

            //
            // If error bits are set, we need to alert the caller.
            //
            if (ui32ReadData & (_VAL2FLD(UART0_DR_OEDATA, UART0_DR_OEDATA_ERR) |
                                _VAL2FLD(UART0_DR_BEDATA, UART0_DR_BEDATA_ERR) |
                                _VAL2FLD(UART0_DR_PEDATA, UART0_DR_PEDATA_ERR) |
                                _VAL2FLD(UART0_DR_FEDATA, UART0_DR_FEDATA_ERR)) )
            {
                ui32ErrorStatus =  AM_HAL_UART_STATUS_BUS_ERROR;
                break;
            }
            else
            {
                pui8Data[i++] = ui32ReadData & 0xFF;
            }
        }
    }

    if (pui32NumBytesRead)
    {
        *pui32NumBytesRead = i;
    }

    return ui32ErrorStatus;
}

//*****************************************************************************
//
// Write to the UART TX FIFO
//
//*****************************************************************************
uint32_t
am_hal_uart_fifo_write(void *pHandle, uint8_t *pui8Data, uint32_t ui32NumBytes,
                       uint32_t *pui32NumBytesWritten)
{
    uint32_t i = 0;

    am_hal_uart_state_t *pState = (am_hal_uart_state_t *) pHandle;
    uint32_t ui32Module = pState->ui32Module;

    //
    // Start a loop where we attempt to write everything requested.
    //
    while (i < ui32NumBytes)
    {
        //
        // If the TX FIFO is full, break out of the loop. We've sent everything
        // we can.
        //
        if ( UARTn(ui32Module)->FR_b.TXFF )
        {
            break;
        }
        else
        {
            UARTn(ui32Module)->DR = pui8Data[i++];
        }
    }

    //
    // Let the caller know how much we sent.
    //
    if (pui32NumBytesWritten)
    {
        *pui32NumBytesWritten = i;
    }

    return AM_HAL_STATUS_SUCCESS;
}

//*****************************************************************************
//
// Empty the UART RX FIFO, and place the data into the RX queue.
//
//*****************************************************************************
static uint32_t
rx_queue_update(void *pHandle)
{
    am_hal_uart_state_t *pState = (am_hal_uart_state_t *) pHandle;

    uint8_t pui8Data[AM_HAL_UART_FIFO_MAX];
    uint32_t ui32BytesTransferred;
    uint32_t ui32ErrorStatus;

    AM_CRITICAL_BEGIN;

    //
    // Read as much of the FIFO as we can.
    //
    ui32ErrorStatus = am_hal_uart_fifo_read(pHandle, pui8Data, AM_HAL_UART_FIFO_MAX,
                                            &ui32BytesTransferred);
    //
    // If we were successful, go ahead and transfer the data along to the
    // buffer.
    //
    if (ui32ErrorStatus == AM_HAL_STATUS_SUCCESS)
    {
        if (!am_hal_queue_item_add(&pState->sRxQueue, pui8Data,
                                   ui32BytesTransferred))
        {
            ui32ErrorStatus = AM_HAL_UART_STATUS_RX_QUEUE_FULL;
        }
    }

    AM_CRITICAL_END;

    return ui32ErrorStatus;
} // rx_queue_update()

//*****************************************************************************
//
// Transfer as much data as possible from the TX queue to the TX FIFO.
//
//*****************************************************************************
static uint32_t
tx_queue_update(void *pHandle)
{
    am_hal_uart_state_t *pState = (am_hal_uart_state_t *) pHandle;
    uint32_t ui32Module = pState->ui32Module;

    uint8_t pui8Data;
    uint32_t ui32BytesTransferred;
    uint32_t ui32ErrorStatus = AM_HAL_STATUS_SUCCESS;

    AM_CRITICAL_BEGIN;

    //
    // Loop as long as the TX fifo isn't full yet.
    //
    while ( !UARTn(ui32Module)->FR_b.TXFF )
    {
        //
        // Attempt to grab an item from the queue, and add it to the fifo.
        //
        if (am_hal_queue_item_get(&pState->sTxQueue, &pui8Data, 1))
        {
            ui32ErrorStatus = am_hal_uart_fifo_write(pHandle, &pui8Data, 1,
                                                     &ui32BytesTransferred);

            if (ui32ErrorStatus != AM_HAL_STATUS_SUCCESS)
            {
                break;
            }
        }
        else
        {
            //
            // If we didn't get anything from the FIFO, we can just return.
            //
            break;
        }
    }

    AM_CRITICAL_END;

    return ui32ErrorStatus;
} // tx_queue_update()

//*****************************************************************************
//
// Run a UART transaction.
//
//*****************************************************************************
uint32_t
am_hal_uart_transfer(void *pHandle,
                     const am_hal_uart_transfer_t *psTransfer)
{
#ifndef AM_HAL_DISABLE_API_VALIDATION
    //
    // Check to make sure this is a valid handle.
    //
    if (!AM_HAL_UART_CHK_HANDLE(pHandle))
    {
        return AM_HAL_STATUS_INVALID_HANDLE;
    }
#endif
    switch (psTransfer->eType)
    {
        case AM_HAL_UART_BLOCKING_WRITE:
            return blocking_write(pHandle, psTransfer);

        case AM_HAL_UART_BLOCKING_READ:
            return blocking_read(pHandle, psTransfer);

        case AM_HAL_UART_NONBLOCKING_WRITE:
            return nonblocking_write(pHandle, psTransfer);

        case AM_HAL_UART_NONBLOCKING_READ:
            return nonblocking_read(pHandle, psTransfer);

        default:
            return AM_HAL_STATUS_FAIL;
    }
}

//*****************************************************************************
//
// Blocking UART write.
//
//
//         (start blocking_write)
//        ___________|_____________
//       / Check for timeout?      \     YES
//   ->-/(psTransfer->ui32TimeoutMs)\_________________________________________
//  |   \ and check if we are in    /                                         |
//  |    \ writing state           /                                          |
//  |     \_______________________/                                           |
//  ^                |  No                                                    |
//  |     ___________V____________                                            |
//  |    / Is the software Queue  \     YES                                   |
//  |   / (pHandle->bEnableTxQueue)\______________________                    |
//  |   \  active?                 /                      |                   V
//  |    \________________________/     __________________V________________   |
//  |                |  No             | Write as much as possible to      |  |
//  ^    ____________V___________      | software queue(pHandle->sTxQueue).|  |
//  |   | Write up to 32 samples |     |___________________________________|  |
//  |   | to hardware queue.     |                        |                   |
//  |   |________________________|      __________________V_________________  |
//  |                |                 | Read from software queue and write | V
//  |                |                 | up to 32 samples to hardware queue.| |
//  |                |                 | ('tx_queue_update()' function).    | |
//  |                |                 |____________________________________| |
//  ^                |                                    |                   |
//  |             ___V____________________________________V____               |
//  |            / Keep updating the number of bytes that has  \              |
//  |           / been tranferred to the ActiveWrite structure  \             V
//  |           \ variable                                      /             |
//  |            \_____________________________________________/              |
//  |                |                                    |                   |
//  |             ___V____________________________________V____               |
//  |            / Number of bytes transfered till now is equal\     YES      |
//  |           / to psTransfer->ui32NumBytes?                  \_____        V
//  |           \                                               /     |       |
//  |            \_____________________________________________/      V       |
//  |________________|  No                                            |       |
//                                ____________________________________|_______|_
//                               | Return number of bytes transfered to queues  |
//                               | (psTransfer->pui32BytesTransferred).         |
//                               |______________________________________________|
//                                                      |
//                                             (end blocking_write)
//
// Note: When returning from blocking_write, the Tx data is still in UART queues
//   (software or hardware queues).
//*****************************************************************************
uint32_t
blocking_write(void *pHandle, const am_hal_uart_transfer_t *psTransfer)
{
    uint32_t ui32Status;
    uint32_t ui32TimeSpent = 0;
    am_hal_uart_state_t *psState = pHandle;

    //
    // Blocking write has all of the same requirements as non-blocking write,
    // but we can't guarantee that the interrupt handlers will be called, so we
    // have to call them in a loop instead.
    //
    ui32Status = nonblocking_write(pHandle, psTransfer);
    if (ui32Status != AM_HAL_STATUS_SUCCESS)
    {
        return ui32Status;
    }

    while (psState->bCurrentlyWriting)
    {
        nonblocking_write_sm(pHandle);
        am_hal_delay_us(1000);

        if ( psTransfer->ui32TimeoutMs != AM_HAL_UART_WAIT_FOREVER )
        {
            ++ui32TimeSpent;
            if ( ui32TimeSpent == psTransfer->ui32TimeoutMs )
            {
                psState->bCurrentlyWriting = false;
                return AM_HAL_STATUS_TIMEOUT;
            }
        }
    }

    return AM_HAL_STATUS_SUCCESS;
}

//*****************************************************************************
//
// Blocking UART read.
//
//*****************************************************************************
uint32_t
blocking_read(void *pHandle, const am_hal_uart_transfer_t *psTransfer)
{
    uint32_t ui32Status;
    uint32_t ui32TimeSpent = 0;
    am_hal_uart_state_t *psState = pHandle;

    //
    // Blocking read has all of the same requirements as non-blocking read,
    // but we can't guarantee that the interrupt handlers will be called, so we
    // have to call them in a loop instead.
    //
    ui32Status = nonblocking_read(pHandle, psTransfer);
    if (ui32Status != AM_HAL_STATUS_SUCCESS)
    {
        return ui32Status;
    }

    //
    // Blocking read continue until these two conditions are met:
    // 1) The user defined Timeout occurs in which case the HAL returns with a timeout error status
    // 2) All the recieved data in FIFO has been read
    //
    while (psState->bCurrentlyReading)
    {
        nonblocking_read_sm(pHandle);
        am_hal_delay_us(1000);

        if ( psTransfer->ui32TimeoutMs != AM_HAL_UART_WAIT_FOREVER )
        {
            ++ui32TimeSpent;
            if ( ui32TimeSpent == psTransfer->ui32TimeoutMs )
            {
                psState->bCurrentlyReading = false;
                return AM_HAL_STATUS_TIMEOUT;
            }
        }
    }

    return AM_HAL_STATUS_SUCCESS;
}

//*****************************************************************************
//
// Non-blocking UART write.
//
//         (start nonblocking_write)
//                      |
//           ________________________
//          / Is the software Queue  \     YES
//         / (pHandle->bEnableTxQueue)\______________________
//         \  active?                 /                      |
//          \________________________/     __________________V________________
//                      |  No             | Write as much as possible to      |
//          ____________V___________      | software queue(pHandle->sTxQueue).|
//         | Write up to 32 samples |     |___________________________________|
//         | to hardware queue.     |                        |
//         |________________________|      __________________V_________________
//                      |                 | Read from software queue and write |
//                      |                 | up to 32 samples to hardware queue |
//                      |                 | ('tx_queue_update()' function).    |
//                      |                 |____________________________________|
//                      |                                    |
//                   ___V____________________________________V____
//                  | Return number of bytes transfered to queues |
//                  | (psTransfer->pui32BytesTransferred).        |
//                  |_____________________________________________|
//                                          |
//                               (end nonblocking_write)
//
// Note: The transmit data is always written to the software queue first
// because old data may be already in the software queue.
//*****************************************************************************
uint32_t
nonblocking_write(void *pHandle, const am_hal_uart_transfer_t *psTransfer)
{
    uint32_t ui32ErrorStatus;
    uint32_t *pui32NumBytesWritten = psTransfer->pui32BytesTransferred;

    //
    // Initialize the variable tracking how many bytes we've written.
    //
    if (pui32NumBytesWritten)
    {
        *pui32NumBytesWritten = 0;
    }

    //
    // Save the transaction, and run the state machine to transfer the data.
    //
    ui32ErrorStatus = write_transaction_save(pHandle, psTransfer);
    if (ui32ErrorStatus != AM_HAL_STATUS_SUCCESS)
    {
        return ui32ErrorStatus;
    }
    else
    {
        nonblocking_write_sm(pHandle);
    }

    return AM_HAL_STATUS_SUCCESS;
}

//*****************************************************************************
//
// Non-blocking UART read.
//
//*****************************************************************************
uint32_t
nonblocking_read(void *pHandle, const am_hal_uart_transfer_t *psTransfer)
{
    uint32_t ui32ErrorStatus = AM_HAL_STATUS_SUCCESS;
    uint32_t *pui32NumBytesRead = psTransfer->pui32BytesTransferred;

    //
    // Start by setting the number of bytes read to 0.
    //
    if (pui32NumBytesRead)
    {
        *pui32NumBytesRead = 0;
    }

    //
    // Save the read transaction and run the state machine to transfer the data.
    //
    ui32ErrorStatus = read_transaction_save(pHandle, psTransfer);
    if (ui32ErrorStatus != AM_HAL_STATUS_SUCCESS)
    {
        return ui32ErrorStatus;
    }
    else
    {
        nonblocking_read_sm(pHandle);
    }

    return ui32ErrorStatus;
}

//*****************************************************************************
//
// Non-blocking UART write state machine.
//
// This function takes data from the user-supplied buffer and pushes it through
// the UART transmit fifo. If a TX write queue has been enabled, the data will
// go through the queue first. If the transfer is too big to complete all at
// once, this function will save its place in the UART state variable and
// return.
//
// This function will also clean up the UART module state and call the
// appropriate callback if the active transaction is completed.
//
//*****************************************************************************
void
nonblocking_write_sm(void *pHandle)
{
    am_hal_uart_state_t *pState = (am_hal_uart_state_t *) pHandle;

    //
    // Then, check to see if we currently have an active "write" transaction. If
    // we do, we need to pull data out of the user's buffer and either push them
    // into our internal buffer (if TX buffers were enabled) or push them to the
    // hardware fifo.
    //
    if (pState->bCurrentlyWriting)
    {
        //
        // Start a critical section to avoid problems associated with this
        // function being called from multiple places concurrently.
        //
        //
        // When the callback is called, the function must return immediately after
        // calling the callback. It can't return until the function exits the
        // critical section.
        // This flag tracks the callback usage
        //
        bool bReturnAfterCritical = false;

        AM_CRITICAL_BEGIN

        uint32_t i = pState->ui32BytesWritten;
        uint32_t ui32BytesLeft = pState->sActiveWrite.ui32NumBytes - i;

        uint8_t *pui8Src = &(pState->sActiveWrite.pui8Data[i]);

        uint32_t ui32WriteSize;

        if (pState->bEnableTxQueue)
        {
            //
            // If we are using an internal queue, we'll push as much data as we
            // can to the queue.
            //
            uint32_t ui32QueueSpace =
                am_hal_queue_space_left(&pState->sTxQueue);

            ui32WriteSize = (ui32BytesLeft < ui32QueueSpace ?
                             ui32BytesLeft : ui32QueueSpace);

            bool bSuccess = am_hal_queue_item_add(&pState->sTxQueue,
                                                  pui8Src,
                                                  ui32WriteSize);

            //
            // If something goes wrong with this queue write, even though we've
            // checked the size of the queue, then something isn't right, and we
            // should return with an error. This might indicate thread
            // mis-management.
            //
            if (bSuccess == false)
            {
                pState->bCurrentlyWriting = false;
                if (pState->sActiveWrite.pfnCallback)
                {
                    pState->sActiveWrite.pfnCallback(1, pState->sActiveWrite.pvContext);
                    // After the callback,
                    // this function needs to return once the critical section exits with
                    // no other variables modified.
                    // Set this flag to keep track of that (early return) state.
                    bReturnAfterCritical = true;
                }
            }
        }
        else // if ( !pState->bEnableTxQueue )
        {
            //
            // If we're not using a queue, we'll just write straight to the
            // FIFO. This function will attempt to write everything we have
            // left and return the actual number of bytes written to
            // ui32WriteSize.
            //
            am_hal_uart_fifo_write(pHandle, pui8Src, ui32BytesLeft,
                                   &ui32WriteSize);
        }

        if ( !bReturnAfterCritical)
        {
            //
            // Before returning, we also need to update the saved transaction,
            // closing it if we've consumed all the bytes in the user's
            // original buffer.
            // This variable is not updated after a callback has been called above.
            //
            pState->ui32BytesWritten += ui32WriteSize;
        } // else/if ( !pState->bEnableTxQueue )

        AM_CRITICAL_END

        if (bReturnAfterCritical)
        {
            //
            // the callback was executed in the critical section above,
            // and now this function needs to exit
            //
            return;
        }

    }

    //
    // If we're using a TX queue, send all of the data in the TX buffer to the
    // hardware fifo.
    //
    if (pState->bEnableTxQueue)
    {
        tx_queue_update(pHandle);
    }

    *pState->sActiveWrite.pui32BytesTransferred = pState->ui32BytesWritten;

    if ( (pState->ui32BytesWritten == pState->sActiveWrite.ui32NumBytes) &&
         pState->bCurrentlyWriting )
    {
        //
        // all data has been written and completion notification hasn't been sent yet
        //
        pState->bCurrentlyWriting = false;

        if (pState->sActiveWrite.pfnCallback)
        {
            pState->sActiveWrite.pfnCallback(0, pState->sActiveWrite.pvContext);
        }
    }
}

//*****************************************************************************
//
// Non-blocking UART read state machine.
//
// This function takes data from the UART fifo and writes it to the
// user-supplied buffer. If an RX queue has been enabled, it will transfer data
// from the UART FIFO to the RX queue, and then from the RX queue to the user's
// buffer.
//
// This function will also clean up the UART module state and call the
// appropriate callback if the active transaction is completed.
//
//*****************************************************************************
void
nonblocking_read_sm(void *pHandle)
{
    am_hal_uart_state_t *pState = (am_hal_uart_state_t *) pHandle;

    //
    // If we're using an RX queue, take all of the data from the RX FIFO and add
    // it to the internal queue.
    //
    if (pState->bEnableRxQueue)
    {
        rx_queue_update(pHandle);
    }

    //
    // If we have an active read, take data from the RX queue and transfer it to
    // the user-supplied destination buffer. If this completes a transaction,
    // this function will also call the user's supplied callback.
    //
    if (pState->bCurrentlyReading)
    {
        //
        // When the callback is called the function must return immediately after
        // calling the callback, but the callback is called in a critical section
        // this flag tracks the callback usage
        //

        bool bReturnAfterCritical = false;
        AM_CRITICAL_BEGIN

        uint32_t i = pState->ui32BytesRead;
        uint32_t ui32BytesLeft = pState->sActiveRead.ui32NumBytes - i;

        uint8_t *pui8Dest = &(pState->sActiveRead.pui8Data[i]);

        uint32_t ui32ReadSize = 0;

        if (pState->bEnableRxQueue)
        {
            //
            // If we're using an internal queue, read from there first.
            //
            uint32_t ui32QueueData =
                am_hal_queue_data_left(&pState->sRxQueue);

            ui32ReadSize = (ui32BytesLeft < ui32QueueData ?
                            ui32BytesLeft : ui32QueueData);

            bool bSuccess = am_hal_queue_item_get(&pState->sRxQueue,
                                             pui8Dest, ui32ReadSize);

            //
            // We checked the amount of data we had available before reading, so
            // the queue read really shouldn't fail. If it does anyway, we need
            // to raise an error and abort the read.
            //
            if (bSuccess == false)
            {
                pState->bCurrentlyReading = false;
                if (pState->sActiveRead.pfnCallback)
                {
                    pState->sActiveRead.pfnCallback(1, pState->sActiveRead.pvContext);
                    //
                    // This is in a critical section, can't return from function here
                    // first need to exit critical section, set this flag to keep track
                    // of the need to return early and do nothing else in this function
                    //
                    bReturnAfterCritical = true;
                }
            }
        }
        else
        {
            //
            // If we're not using an internal queue, we need to read directly
            // from the fifo instead. This should read as many bytes as possible
            // and update ui32ReadSize with the number of bytes we actually got.
            //
            am_hal_uart_fifo_read(pHandle, pui8Dest, ui32BytesLeft,
                                  &ui32ReadSize);

        }

        if ( !bReturnAfterCritical)
        {
            pState->ui32BytesRead += ui32ReadSize;
        }

        AM_CRITICAL_END

        if (bReturnAfterCritical)
        {
            return;
        }

        //
        // At this point we've moved all of the data we can. We need to update
        // the transaction state, and then check to see if this operation has
        // actually completed a transaction. If so, close out the transaction
        // and call the callback.
        //

        *pState->sActiveRead.pui32BytesTransferred = pState->ui32BytesRead;

        if (pState->ui32BytesRead == pState->sActiveRead.ui32NumBytes)
        {
            pState->bCurrentlyReading = false;

            if (pState->sActiveRead.pfnCallback)
            {
                pState->sActiveRead.pfnCallback(0, pState->sActiveRead.pvContext);
            }
        }
    }
}

//*****************************************************************************
//
// Wait for all of the traffic in the TX pipeline to be sent.
//
//*****************************************************************************
uint32_t
am_hal_uart_tx_flush(void *pHandle)
{
    am_hal_uart_state_t *pState = (am_hal_uart_state_t *) pHandle;
    uint32_t ui32Module = pState->ui32Module;

    //
    // If our state variable says we're done, we're done.
    //
    if (pState->bLastTxComplete == true)
    {
        return AM_HAL_STATUS_SUCCESS;
    }

    //
    // If our state variable doesn't say we're done, we need to check to make
    // sure this program is actually capable of using the state variable.
    // Checking the interrupt enable bit isn't a perfect test, but it will give
    // us a reasonable guess about whether the bLastTxComplete flag might be
    // updated in the future.
    //
    if (UARTn(ui32Module)->IER & AM_HAL_UART_INT_TXCMP)
    {
        while(pState->bLastTxComplete == false)
        {
            ONE_BYTE_DELAY(pState);
        }
    }
    else
    {
        //
        // If we don't know the UART status by now, the best we can do is check
        // to see if the queue is empty, or it the busy bit is still set.
        //
        // If wehave a TX queue, we should wait for it to empty.
        //
        if (pState->bEnableTxQueue)
        {
            while (am_hal_queue_data_left(&(pState->sTxQueue)))
            {
                ONE_BYTE_DELAY(pState);
            }
        }

        //
        // Wait for the TX busy bit to go low.
        //
        while ( UARTn(ui32Module)->FR_b.BUSY )
        {
            ONE_BYTE_DELAY(pState);
        }
    }

    return AM_HAL_STATUS_SUCCESS;
}

//*****************************************************************************
//
// Read the UART flags.
//
//*****************************************************************************
uint32_t
am_hal_uart_flags_get(void *pHandle, uint32_t *pui32Flags)
{
    am_hal_uart_state_t *pState = (am_hal_uart_state_t *) pHandle;
    uint32_t ui32Module = pState->ui32Module;

#ifndef AM_HAL_DISABLE_API_VALIDATION
    //
    // Check to make sure this is a valid handle.
    //
    if ( !AM_HAL_UART_CHK_HANDLE(pHandle) )
    {
        return AM_HAL_STATUS_INVALID_HANDLE;
    }
#endif
    if ( pui32Flags )
    {
        //
        // Set the flags value, then return success.
        //
        *pui32Flags = UARTn(ui32Module)->FR;

        return AM_HAL_STATUS_SUCCESS;
    }
    else
    {
        return AM_HAL_STATUS_INVALID_ARG;
    }
}

//*****************************************************************************
//
// Enable interrupts.
//
//*****************************************************************************
uint32_t
am_hal_uart_interrupt_enable(void *pHandle, uint32_t ui32IntMask)
{
    am_hal_uart_state_t *pState = (am_hal_uart_state_t *) pHandle;
    uint32_t ui32Module = pState->ui32Module;

#ifndef AM_HAL_DISABLE_API_VALIDATION
    if (!AM_HAL_UART_CHK_HANDLE(pHandle))
    {
        return AM_HAL_STATUS_INVALID_HANDLE;
    }
#endif
    UARTn(ui32Module)->IER |= ui32IntMask;

    return AM_HAL_STATUS_SUCCESS;
}

//*****************************************************************************
//
// Return the set of enabled interrupts.
//
//*****************************************************************************
uint32_t
am_hal_uart_interrupt_enable_get(void *pHandle, uint32_t *pui32IntMask)
{
    am_hal_uart_state_t *pState = (am_hal_uart_state_t *) pHandle;
    uint32_t ui32Module = pState->ui32Module;

#ifndef AM_HAL_DISABLE_API_VALIDATION
    if (!AM_HAL_UART_CHK_HANDLE(pHandle))
    {
        return AM_HAL_STATUS_INVALID_HANDLE;
    }
#endif
    *pui32IntMask = UARTn(ui32Module)->IER;

    return AM_HAL_STATUS_SUCCESS;
} // am_hal_uart_interrupt_enable_get()

//*****************************************************************************
//
// Disable interrupts.
//
//*****************************************************************************
uint32_t
am_hal_uart_interrupt_disable(void *pHandle, uint32_t ui32IntMask)
{
    am_hal_uart_state_t *pState = (am_hal_uart_state_t *) pHandle;
    uint32_t ui32Module = pState->ui32Module;

#ifndef AM_HAL_DISABLE_API_VALIDATION
    if (!AM_HAL_UART_CHK_HANDLE(pHandle))
    {
        return AM_HAL_STATUS_INVALID_HANDLE;
    }
#endif
    UARTn(ui32Module)->IER &= ~ui32IntMask;

    return AM_HAL_STATUS_SUCCESS;
}

//*****************************************************************************
//
// Clear interrupt status.
//
//*****************************************************************************
uint32_t
am_hal_uart_interrupt_clear(void *pHandle, uint32_t ui32IntMask)
{
    am_hal_uart_state_t *pState = (am_hal_uart_state_t *) pHandle;
    uint32_t ui32Module = pState->ui32Module;

#ifndef AM_HAL_DISABLE_API_VALIDATION
    if (!AM_HAL_UART_CHK_HANDLE(pHandle))
    {
        return AM_HAL_STATUS_INVALID_HANDLE;
    }
#endif
    UARTn(ui32Module)->IEC = ui32IntMask;
    *(volatile uint32_t*)(&UARTn(ui32Module)->MIS);
    return AM_HAL_STATUS_SUCCESS;
}

//*****************************************************************************
//
// Read interrupt status.
//
//*****************************************************************************
uint32_t
am_hal_uart_interrupt_status_get(void *pHandle, uint32_t *pui32Status,
                                 bool bEnabledOnly)
{
    am_hal_uart_state_t *pState = (am_hal_uart_state_t *) pHandle;
    uint32_t ui32Module = pState->ui32Module;

#ifndef AM_HAL_DISABLE_API_VALIDATION
    if (!AM_HAL_UART_CHK_HANDLE(pHandle))
    {
        return AM_HAL_STATUS_INVALID_HANDLE;
    }
#endif
    //
    // If requested, only return the interrupts that are enabled.
    //
    *pui32Status = bEnabledOnly ? UARTn(ui32Module)->MIS : UARTn(ui32Module)->IES;

    return AM_HAL_STATUS_SUCCESS;
}

//*****************************************************************************
//
// Read interrupt status.
//
//*****************************************************************************
uint32_t
am_hal_uart_interrupt_service(void *pHandle, uint32_t ui32Status)
{
    am_hal_uart_state_t *pState = pHandle;

#ifndef AM_HAL_DISABLE_API_VALIDATION
    //
    // Check to make sure this is a valid handle.
    //
    if (!AM_HAL_UART_CHK_HANDLE(pHandle))
    {
        return AM_HAL_STATUS_INVALID_HANDLE;
    }
#endif
    uint32_t ui32Module = ((am_hal_uart_state_t*)pHandle)->ui32Module;
    if (pState->bDMABusy)
    {
        if (ui32Status & AM_HAL_UART_INT_DMAERIS)
        {
            //
            // Clear DMAERR bit.
            //
            am_hal_uart_dma_error(pHandle);
            pState->bDMABusy = false;
        }
        if (ui32Status & AM_HAL_UART_INT_DMACPRIS)
        {
            if ( pState->pfnCallback != NULL )
            {
                pState->pfnCallback(internal_uart_get_int_err(ui32Module, ui32Status), pState->pCallbackCtxt);
                pState->pfnCallback = NULL;
            }
            //
            // Complete DMA transaction
            //
            am_hal_uart_dma_transfer_complete(pHandle);
            pState->bDMABusy = false;
        }
    }

    if ( ui32Status & (AM_HAL_UART_INT_RX | AM_HAL_UART_INT_RX_TMOUT) )
    {
        nonblocking_read_sm(pHandle);
    }

    if ( ui32Status & (AM_HAL_UART_INT_TX) )
    {
        nonblocking_write_sm(pHandle);
    }

    if ( ui32Status & (AM_HAL_UART_INT_TXCMP) )
    {
        pState->bLastTxComplete = true;
    }

    return AM_HAL_STATUS_FAIL;
}

//*****************************************************************************
//
//! @}
// End Doxygen group.
//
//*****************************************************************************
