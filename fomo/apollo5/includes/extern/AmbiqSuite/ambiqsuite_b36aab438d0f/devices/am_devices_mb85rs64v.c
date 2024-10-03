//*****************************************************************************
//
//! @file am_devices_mb85rs64v.c
//!
//! @brief Fujitsu 64K SPI FRAM driver.
//!
//! @addtogroup mb85rs64v MB85RS64V SPI FRAM driver
//! @ingroup devices
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
#include "am_devices_mb85rs64v.h"
#include "am_bsp.h"

//*****************************************************************************
//
// Global variables.
//
//*****************************************************************************
typedef struct
{
    uint32_t                    ui32Module;
    uint32_t                    ui32CS;
    void                        *pIomHandle;
    bool                        bOccupied;
} am_devices_iom_mb85rs64v_t;

am_devices_iom_mb85rs64v_t gAmMb85rs64v[AM_DEVICES_MB85RS64V_MAX_DEVICE_NUM];

am_hal_iom_config_t     g_sIomMb85rs64vCfg =
{
    .eInterfaceMode       = AM_HAL_IOM_SPI_MODE,
    .ui32ClockFreq        = AM_HAL_IOM_1MHZ,
    .eSpiMode             = AM_HAL_IOM_SPI_MODE_0,
    .ui32NBTxnBufLength   = 0,
    .pNBTxnBuf = NULL,
};

//*****************************************************************************
//
//! @brief Generic Command Write function.
//!
//! @param pHandle
//! @param ui32InstrLen
//! @param ui64Instr
//! @param pData
//! @param ui32NumBytes
//!
//! @return
//
//*****************************************************************************
static uint32_t
am_device_command_write(void *pHandle, uint32_t ui32InstrLen, uint64_t ui64Instr,
                        uint32_t *pData, uint32_t ui32NumBytes)
{
    am_hal_iom_transfer_t Transaction;
    am_devices_iom_mb85rs64v_t *pIom = (am_devices_iom_mb85rs64v_t *)pHandle;

    //
    // Create the transaction.
    //
    Transaction.ui32InstrLen    = ui32InstrLen;
#if (defined(AM_PART_APOLLO4B) || defined(AM_PART_APOLLO4P) || defined(AM_PART_APOLLO4L) || defined(AM_PART_BRONCO) || defined(AM_PART_APOLLO5_API))
    Transaction.ui64Instr       = ui64Instr;
#else
    Transaction.ui32Instr       = (uint32_t)ui64Instr;
#endif
    Transaction.eDirection      = AM_HAL_IOM_TX;
    Transaction.ui32NumBytes    = ui32NumBytes;
    Transaction.pui32TxBuffer   = pData;
    Transaction.uPeerInfo.ui32SpiChipSelect = pIom->ui32CS;
    Transaction.bContinue       = false;
    Transaction.ui8RepeatCount  = 0;
    Transaction.ui32PauseCondition = 0;
    Transaction.ui32StatusSetClr = 0;

    //
    // Execute the transction over IOM.
    //
    if (am_hal_iom_blocking_transfer(pIom->pIomHandle, &Transaction))
    {
        return AM_DEVICES_MB85RS64V_STATUS_ERROR;
    }
    return AM_DEVICES_MB85RS64V_STATUS_SUCCESS;
}

//*****************************************************************************
//
//! @brief Generic Command Read function.
//!
//! @param pHandle
//! @param ui32InstrLen
//! @param ui64Instr
//! @param pData
//! @param ui32NumBytes
//!
//! @return
//
//*****************************************************************************
static uint32_t
am_device_command_read(void *pHandle, uint32_t ui32InstrLen, uint64_t ui64Instr,
                       uint32_t *pData, uint32_t ui32NumBytes)
{
    am_hal_iom_transfer_t  Transaction;
    am_devices_iom_mb85rs64v_t *pIom = (am_devices_iom_mb85rs64v_t *)pHandle;

    //
    // Create the transaction.
    //
    Transaction.ui32InstrLen      = ui32InstrLen;
#if (defined(AM_PART_APOLLO4B) || defined(AM_PART_APOLLO4P) || defined(AM_PART_APOLLO4L) || defined(AM_PART_BRONCO) || defined(AM_PART_APOLLO5_API))
    Transaction.ui64Instr         = ui64Instr;
#else
    Transaction.ui32Instr         = (uint32_t)ui64Instr;
#endif
    Transaction.eDirection        = AM_HAL_IOM_RX;
    Transaction.ui32NumBytes      = ui32NumBytes;
    Transaction.pui32RxBuffer     = pData;
    Transaction.uPeerInfo.ui32SpiChipSelect = pIom->ui32CS;
    Transaction.bContinue         = false;
    Transaction.ui8RepeatCount    = 0;
    Transaction.ui32PauseCondition = 0;
    Transaction.ui32StatusSetClr = 0;

    //
    // Execute the transction over IOM.
    //
    if (am_hal_iom_blocking_transfer(pIom->pIomHandle, &Transaction))
    {
        return AM_DEVICES_MB85RS64V_STATUS_ERROR;
    }
    return AM_DEVICES_MB85RS64V_STATUS_SUCCESS;
}

//*****************************************************************************
//
//  Initialize the mb85rs64v driver.
//
//*****************************************************************************
uint32_t
am_devices_mb85rs64v_init(uint32_t ui32Module, am_devices_mb85rs64v_config_t *pDevConfig, void **ppHandle, void **ppIomHandle)
{
    void *pIomHandle;
    am_hal_iom_config_t     g_stIOMMB85RS64VSettings;
    uint32_t      ui32Index = 0;

    // Allocate a vacant device handle
    for ( ui32Index = 0; ui32Index < AM_DEVICES_MB85RS64V_MAX_DEVICE_NUM; ui32Index++ )
    {
        if ( gAmMb85rs64v[ui32Index].bOccupied == false )
        {
            break;
        }
    }
    if ( ui32Index == AM_DEVICES_MB85RS64V_MAX_DEVICE_NUM )
    {
        return AM_DEVICES_MB85RS64V_STATUS_ERROR;
    }

    if ( (ui32Module > AM_REG_IOM_NUM_MODULES)  || (pDevConfig == NULL) )
    {
        return AM_DEVICES_MB85RS64V_STATUS_ERROR;
    }

    //
    // Configure the IOM pins.
    //
    am_bsp_iom_pins_enable(ui32Module, AM_HAL_IOM_SPI_MODE);

    //
    // Enable fault detection.
    //
#if !defined(AM_PART_APOLLO5_API)
#if defined(AM_PART_APOLLO4_API)
    am_hal_fault_capture_enable();
#elif AM_PART_APOLLO3_API
    am_hal_mcuctrl_control(AM_HAL_MCUCTRL_CONTROL_FAULT_CAPTURE_ENABLE, 0);
#else
    am_hal_mcuctrl_fault_capture_enable();
#endif
#endif

    g_stIOMMB85RS64VSettings = g_sIomMb85rs64vCfg;
    g_stIOMMB85RS64VSettings.ui32NBTxnBufLength = pDevConfig->ui32NBTxnBufLength;
    g_stIOMMB85RS64VSettings.pNBTxnBuf = pDevConfig->pNBTxnBuf;
    g_stIOMMB85RS64VSettings.ui32ClockFreq = pDevConfig->ui32ClockFreq;

    //
    // Initialize the IOM instance.
    // Enable power to the IOM instance.
    // Configure the IOM for Serial operation during initialization.
    // Enable the IOM.
    //
    if (am_hal_iom_initialize(ui32Module, &pIomHandle) ||
        am_hal_iom_power_ctrl(pIomHandle, AM_HAL_SYSCTRL_WAKE, false) ||
        am_hal_iom_configure(pIomHandle, &g_stIOMMB85RS64VSettings) ||
        am_hal_iom_enable(pIomHandle))
    {
        return AM_DEVICES_MB85RS64V_STATUS_ERROR;
    }
    else
    {
        gAmMb85rs64v[ui32Index].bOccupied = true;
        gAmMb85rs64v[ui32Index].ui32Module = ui32Module;
#if !defined(AM_PART_APOLLO4) && !defined(AM_PART_APOLLO4B) && !defined(AM_PART_APOLLO4P) && !defined(AM_PART_APOLLO4L) && !defined(AM_PART_BRONCO) && !defined(AM_PART_APOLLO5_API)
        gAmMb85rs64v[ui32Index].ui32CS = AM_BSP_IOM0_CS3_CHNL;
#endif
        *ppIomHandle = gAmMb85rs64v[ui32Index].pIomHandle = pIomHandle;
        *ppHandle = (void *)&gAmMb85rs64v[ui32Index];
        //
        // Return the status.
        //
        return AM_DEVICES_MB85RS64V_STATUS_SUCCESS;
    }
}

//*****************************************************************************
//
//  De-Initialize the mb85rs64v driver.
//
//*****************************************************************************
uint32_t
am_devices_mb85rs64v_term(void *pHandle)
{
    am_devices_iom_mb85rs64v_t *pIom = (am_devices_iom_mb85rs64v_t *)pHandle;

    if ( pIom->ui32Module > AM_REG_IOM_NUM_MODULES )
    {
        return AM_DEVICES_MB85RS64V_STATUS_ERROR;
    }

// #### INTERNAL BEGIN ####
    // TODO - no bsp function to disable pins
// #### INTERNAL END ####
    //
    // Disable the IOM.
    //
    am_hal_iom_disable(pIom->pIomHandle);

    //
    // Disable power to and uninitialize the IOM instance.
    //
    am_hal_iom_power_ctrl(pIom->pIomHandle, AM_HAL_SYSCTRL_DEEPSLEEP, false);

    am_hal_iom_uninitialize(pIom->pIomHandle);

    // Free this device handle
    pIom->bOccupied = false;

    //
    // Return the status.
    //
    return AM_DEVICES_MB85RS64V_STATUS_SUCCESS;
}

//*****************************************************************************
//
//  Reads the ID of the external flash and returns the value.
//
//*****************************************************************************
uint32_t
am_devices_mb85rs64v_read_id(void *pHandle, uint32_t *pDeviceID)
{
    //
    // Send the command sequence to read the Device ID.
    //
    if (am_device_command_read(pHandle, 1, AM_DEVICES_MB85RS64V_READ_DEVICE_ID, pDeviceID, 4))
    {
        return AM_DEVICES_MB85RS64V_STATUS_ERROR;
    }

    //
    // Return the status.
    //
    return AM_DEVICES_MB85RS64V_STATUS_SUCCESS;
}

//*****************************************************************************
//
// Reads the current status of the external flash
//
//*****************************************************************************
uint32_t
am_devices_mb85rs64v_status_get(void *pHandle, uint32_t *pStatus)
{
    //
    // Send the command sequence to read the device status.
    //
    if (am_device_command_read(pHandle, 1, AM_DEVICES_MB85RS64V_READ_STATUS, pStatus, 1))
    {
        return AM_DEVICES_MB85RS64V_STATUS_ERROR;
    }

    //
    // Return the status.
    //
    return AM_DEVICES_MB85RS64V_STATUS_SUCCESS;
}

//*****************************************************************************
//
// Sends a specific command to the device (blocking).
//
//*****************************************************************************
uint32_t
am_devices_mb85rs64v_command_send(void *pHandle, uint32_t ui32Cmd)
{
    uint32_t Dummy;

    if ( ui32Cmd == AM_DEVICES_MB85RS64V_CMD_WREN )
    {
        //
        // Send the command to enable writing.
        //
        if (am_device_command_write(pHandle, 1, AM_DEVICES_MB85RS64V_WRITE_ENABLE, &Dummy, 0))
        {
            return AM_DEVICES_MB85RS64V_STATUS_ERROR;
        }
    }
    else if ( ui32Cmd == AM_DEVICES_MB85RS64V_CMD_WRDI )
    {
        //
        // Send the command to enable writing.
        //
        if (am_device_command_write(pHandle, 1, AM_DEVICES_MB85RS64V_WRITE_DISABLE, &Dummy, 0))
        {
            return AM_DEVICES_MB85RS64V_STATUS_ERROR;
        }
    }
    else
    {
        return AM_DEVICES_MB85RS64V_STATUS_ERROR;
    }

    //
    // Return the status.
    //
    return AM_DEVICES_MB85RS64V_STATUS_SUCCESS;
}

//*****************************************************************************
//
// Programs the given range of flash addresses.
//
//*****************************************************************************
uint32_t
am_devices_mb85rs64v_blocking_write(void *pHandle, uint8_t *pui8TxBuffer,
                                    uint32_t ui32WriteAddress,
                                    uint32_t ui32NumBytes)
{
    am_hal_iom_transfer_t       Transaction;
    uint32_t                    Dummy;
    am_devices_iom_mb85rs64v_t *pIom = (am_devices_iom_mb85rs64v_t *)pHandle;

    //
    // Send the command to enable writing.
    //
    if (am_device_command_write(pHandle, 1, AM_DEVICES_MB85RS64V_WRITE_ENABLE, &Dummy, 0))
    {
        return AM_DEVICES_MB85RS64V_STATUS_ERROR;
    }

    //
    // Set up the IOM transaction to write to the device.
    //
    Transaction.eDirection      = AM_HAL_IOM_TX;
    Transaction.ui32InstrLen    = 3;
#if (defined(AM_PART_APOLLO4B) || defined(AM_PART_APOLLO4P) || defined(AM_PART_APOLLO4L) || defined(AM_PART_BRONCO) || defined(AM_PART_APOLLO5_API))
    Transaction.ui64Instr       = (AM_DEVICES_MB85RS64V_WRITE << 16) |
                                  (uint16_t)(ui32WriteAddress & 0x0000FFFF);
#else
    Transaction.ui32Instr       = (AM_DEVICES_MB85RS64V_WRITE << 16) |
                                  (uint16_t)(ui32WriteAddress & 0x0000FFFF);
#endif
    Transaction.ui32NumBytes    = ui32NumBytes;
    Transaction.pui32TxBuffer   = (uint32_t *)pui8TxBuffer;
    Transaction.uPeerInfo.ui32SpiChipSelect    = pIom->ui32CS;
    Transaction.bContinue       = false;
    Transaction.ui8RepeatCount  = 0;
    Transaction.ui32PauseCondition = 0;
    Transaction.ui32StatusSetClr = 0;

    //
    // Start the transaction.
    //
    if (am_hal_iom_blocking_transfer(pIom->pIomHandle, &Transaction))
    {
        return AM_DEVICES_MB85RS64V_STATUS_ERROR;
    }

    //
    // Send the command to disable writing.
    //
    if (am_device_command_write(pHandle, 1, AM_DEVICES_MB85RS64V_WRITE_DISABLE, &Dummy, 0))
    {
        return AM_DEVICES_MB85RS64V_STATUS_ERROR;
    }

    //
    // Return the status.
    //
    return AM_DEVICES_MB85RS64V_STATUS_SUCCESS;
}

//*****************************************************************************
//
// Programs the given range of flash addresses.
//
//*****************************************************************************
uint32_t
am_devices_mb85rs64v_nonblocking_write_adv(void *pHandle, uint8_t *pui8TxBuffer,
                                           uint32_t ui32WriteAddress,
                                           uint32_t ui32NumBytes,
                                           uint32_t ui32PauseCondition,
                                           uint32_t ui32StatusSetClr,
                                           am_hal_iom_callback_t pfnCallback,
                                           void *pCallbackCtxt)
{
    am_hal_iom_transfer_t      Transaction;
    am_devices_iom_mb85rs64v_t *pIom = (am_devices_iom_mb85rs64v_t *)pHandle;

    Transaction.ui8Priority     = 1;        // High priority for now.
    Transaction.uPeerInfo.ui32SpiChipSelect = pIom->ui32CS;
    Transaction.bContinue       = false;
    Transaction.ui8RepeatCount  = 0;
    Transaction.ui32PauseCondition = ui32PauseCondition;
    Transaction.ui32StatusSetClr = 0;

    //
    // Set up the IOM transaction to send the WREN device command.
    //
    Transaction.eDirection      = AM_HAL_IOM_TX;
    Transaction.ui32InstrLen    = 1;        // Sending 1 offset byte
#if (defined(AM_PART_APOLLO4B) || defined(AM_PART_APOLLO4P) || defined(AM_PART_APOLLO4L) || defined(AM_PART_BRONCO) || defined(AM_PART_APOLLO5_API))
    Transaction.ui64Instr       = AM_DEVICES_MB85RS64V_WRITE_ENABLE;
#else
    Transaction.ui32Instr       = AM_DEVICES_MB85RS64V_WRITE_ENABLE;
#endif
    Transaction.ui32NumBytes    = 0;        // WREN CMD is sent as the offset
    Transaction.pui32TxBuffer   = (uint32_t *)pui8TxBuffer; // Not used for this CMD

    //
    // Start the transaction (no callback).
    //
    if (am_hal_iom_nonblocking_transfer(pIom->pIomHandle, &Transaction, 0, 0))
    {
        return AM_DEVICES_MB85RS64V_STATUS_ERROR;
    }

    //
    // Set up the IOM transaction to write to the device.
    //
    Transaction.eDirection      = AM_HAL_IOM_TX;
    Transaction.ui32InstrLen    = 3;
#if (defined(AM_PART_APOLLO4B) || defined(AM_PART_APOLLO4P) || defined(AM_PART_APOLLO4L) || defined(AM_PART_BRONCO) || defined(AM_PART_APOLLO5_API))
    Transaction.ui64Instr       = (AM_DEVICES_MB85RS64V_WRITE << 16) |
                                  (uint16_t)(ui32WriteAddress & 0x0000FFFF);
#else
    Transaction.ui32Instr       = (AM_DEVICES_MB85RS64V_WRITE << 16) |
                                  (uint16_t)(ui32WriteAddress & 0x0000FFFF);
#endif
    Transaction.ui32NumBytes    = ui32NumBytes;
    Transaction.pui32TxBuffer   = (uint32_t *)pui8TxBuffer;

    //
    // Start the transaction (no callback).
    //
    if (am_hal_iom_nonblocking_transfer(pIom->pIomHandle, &Transaction, 0, 0))
    {
        return AM_DEVICES_MB85RS64V_STATUS_ERROR;
    }

    //
    // Set up the IOM transaction to send the WRDI (write disable) device command.
    //
    Transaction.eDirection      = AM_HAL_IOM_TX;
    Transaction.ui32InstrLen    = 1;        // Sending 1 offset byte
#if (defined(AM_PART_APOLLO4B) || defined(AM_PART_APOLLO4P) || defined(AM_PART_APOLLO4L) || defined(AM_PART_BRONCO) || defined(AM_PART_APOLLO5_API))
    Transaction.ui64Instr       = AM_DEVICES_MB85RS64V_WRITE_DISABLE;
#else
    Transaction.ui32Instr       = AM_DEVICES_MB85RS64V_WRITE_DISABLE;
#endif
    Transaction.ui32NumBytes    = 0;        // WRDI CMD is sent as the offset
    Transaction.pui32TxBuffer   = (uint32_t *)pui8TxBuffer; // Not used for this CMD
    // Now set the post processing condition
    Transaction.ui32StatusSetClr = ui32StatusSetClr;

    //
    // Start the transaction, sending the callback.
    //
    if (am_hal_iom_nonblocking_transfer(pIom->pIomHandle, &Transaction, pfnCallback, pCallbackCtxt))
    {
        return AM_DEVICES_MB85RS64V_STATUS_ERROR;
    }

    //
    // Return the status.
    //
    return AM_DEVICES_MB85RS64V_STATUS_SUCCESS;
}

//*****************************************************************************
//
//  Programs the given range of flash addresses.
//
//*****************************************************************************
uint32_t
am_devices_mb85rs64v_nonblocking_write(void *pHandle, uint8_t *pui8TxBuffer,
                                       uint32_t ui32WriteAddress,
                                       uint32_t ui32NumBytes,
                                       am_hal_iom_callback_t pfnCallback,
                                       void *pCallbackCtxt)
{
    am_hal_iom_transfer_t      Transaction;
    am_devices_iom_mb85rs64v_t *pIom = (am_devices_iom_mb85rs64v_t *)pHandle;

    Transaction.ui8Priority     = 1;        // High priority for now.
    Transaction.uPeerInfo.ui32SpiChipSelect = pIom->ui32CS;
    Transaction.bContinue       = false;
    Transaction.ui8RepeatCount  = 0;
    Transaction.ui32PauseCondition = 0;
    Transaction.ui32StatusSetClr = 0;

    //
    // Set up the IOM transaction to send the WREN device command.
    //
    Transaction.eDirection      = AM_HAL_IOM_TX;
    Transaction.ui32InstrLen    = 1;        // Sending 1 offset byte
#if (defined(AM_PART_APOLLO4B) || defined(AM_PART_APOLLO4P) || defined(AM_PART_APOLLO4L) || defined(AM_PART_BRONCO) || defined(AM_PART_APOLLO5_API))
    Transaction.ui64Instr       = AM_DEVICES_MB85RS64V_WRITE_ENABLE;
#else
    Transaction.ui32Instr       = AM_DEVICES_MB85RS64V_WRITE_ENABLE;
#endif
    Transaction.ui32NumBytes    = 0;        // WREN CMD is sent as the offset
    Transaction.pui32TxBuffer   = (uint32_t *)pui8TxBuffer; // Not used for this CMD

    //
    // Start the transaction (no callback).
    //
    if (am_hal_iom_nonblocking_transfer(pIom->pIomHandle, &Transaction, 0, 0))
    {
        return AM_DEVICES_MB85RS64V_STATUS_ERROR;
    }

    //
    // Set up the IOM transaction to write to the device.
    //
    Transaction.eDirection      = AM_HAL_IOM_TX;
    Transaction.ui32InstrLen    = 3;
#if (defined(AM_PART_APOLLO4B) || defined(AM_PART_APOLLO4P) || defined(AM_PART_APOLLO4L) || defined(AM_PART_BRONCO) || defined(AM_PART_APOLLO5_API))
    Transaction.ui64Instr       = (AM_DEVICES_MB85RS64V_WRITE << 16) |
                                  (uint16_t)(ui32WriteAddress & 0x0000FFFF);
#else
    Transaction.ui32Instr       = (AM_DEVICES_MB85RS64V_WRITE << 16) |
                                  (uint16_t)(ui32WriteAddress & 0x0000FFFF);
#endif
    Transaction.ui32NumBytes    = ui32NumBytes;
    Transaction.pui32TxBuffer   = (uint32_t *)pui8TxBuffer;

    //
    // Start the transaction (no callback).
    //
    if (am_hal_iom_nonblocking_transfer(pIom->pIomHandle, &Transaction, 0, 0))
    {
        return AM_DEVICES_MB85RS64V_STATUS_ERROR;
    }

    //
    // Set up the IOM transaction to send the WRDI (write disable) device command.
    //
    Transaction.eDirection      = AM_HAL_IOM_TX;
    Transaction.ui32InstrLen    = 1;        // Sending 1 offset byte
#if (defined(AM_PART_APOLLO4B) || defined(AM_PART_APOLLO4P) || defined(AM_PART_APOLLO4L) || defined(AM_PART_BRONCO) || defined(AM_PART_APOLLO5_API))
    Transaction.ui64Instr       = AM_DEVICES_MB85RS64V_WRITE_DISABLE;
#else
    Transaction.ui32Instr       = AM_DEVICES_MB85RS64V_WRITE_DISABLE;
#endif
    Transaction.ui32NumBytes    = 0;        // WRDI CMD is sent as the offset
    Transaction.pui32TxBuffer   = (uint32_t *)pui8TxBuffer; // Not used for this CMD

    //
    // Start the transaction, sending the callback.
    //
    if (am_hal_iom_nonblocking_transfer(pIom->pIomHandle, &Transaction, pfnCallback, pCallbackCtxt))
    {
        return AM_DEVICES_MB85RS64V_STATUS_ERROR;
    }

    //
    // Return the status.
    //
    return AM_DEVICES_MB85RS64V_STATUS_SUCCESS;
}

//*****************************************************************************
//
//  Reads the contents of the fram into a buffer.
// @return 32-bit status
//
//*****************************************************************************
uint32_t
am_devices_mb85rs64v_blocking_read(void *pHandle, uint8_t *pui8RxBuffer,
                                   uint32_t ui32ReadAddress,
                                   uint32_t ui32NumBytes)
{
    am_hal_iom_transfer_t          Transaction;
    am_devices_iom_mb85rs64v_t *pIom = (am_devices_iom_mb85rs64v_t *)pHandle;

    //
    // Set up the IOM transaction.
    //
    Transaction.eDirection      = AM_HAL_IOM_RX;
    Transaction.ui32InstrLen    = 3;
#if (defined(AM_PART_APOLLO4B) || defined(AM_PART_APOLLO4P) || defined(AM_PART_APOLLO4L) || defined(AM_PART_BRONCO) || defined(AM_PART_APOLLO5_API))
    Transaction.ui64Instr       = (AM_DEVICES_MB85RS64V_READ << 16) |
                                    (uint16_t)(ui32ReadAddress & 0x0000FFFF);
#else
    Transaction.ui32Instr       = (AM_DEVICES_MB85RS64V_READ << 16) |
                                    (uint16_t)(ui32ReadAddress & 0x0000FFFF);
#endif
    Transaction.ui32NumBytes    = ui32NumBytes;
    Transaction.pui32RxBuffer   = (uint32_t *)pui8RxBuffer;
    Transaction.uPeerInfo.ui32SpiChipSelect = pIom->ui32CS;
    Transaction.bContinue       = false;
    Transaction.ui8RepeatCount  = 0;
    Transaction.ui32PauseCondition = 0;
    Transaction.ui32StatusSetClr = 0;

    //
    // Start the transaction.
    //
    if (am_hal_iom_blocking_transfer(pIom->pIomHandle, &Transaction))
    {
        return AM_DEVICES_MB85RS64V_STATUS_ERROR;
    }

    //
    // Return the status.
    //
    return AM_DEVICES_MB85RS64V_STATUS_SUCCESS;
}

//*****************************************************************************
//
//  Reads the contents of the fram into a buffer.
// @return 32-bit status
//
//*****************************************************************************
uint32_t
am_devices_mb85rs64v_nonblocking_read(void *pHandle, uint8_t *pui8RxBuffer,
                                      uint32_t ui32ReadAddress,
                                      uint32_t ui32NumBytes,
                                      am_hal_iom_callback_t pfnCallback,
                                      void *pCallbackCtxt)
{
    am_hal_iom_transfer_t      Transaction;
    am_devices_iom_mb85rs64v_t *pIom = (am_devices_iom_mb85rs64v_t *)pHandle;

    //
    // Set up the IOM transaction.
    //
    Transaction.ui8Priority     = 1;        // High priority for now.
    Transaction.eDirection      = AM_HAL_IOM_RX;
    Transaction.ui32InstrLen    = 3;
#if (defined(AM_PART_APOLLO4B) || defined(AM_PART_APOLLO4P) || defined(AM_PART_APOLLO4L) || defined(AM_PART_BRONCO) || defined(AM_PART_APOLLO5_API))
    Transaction.ui64Instr       = (AM_DEVICES_MB85RS64V_READ << 16) |
                                  (uint16_t)(ui32ReadAddress & 0x0000FFFF);
#else
    Transaction.ui32Instr       = (AM_DEVICES_MB85RS64V_READ << 16) |
                                  (uint16_t)(ui32ReadAddress & 0x0000FFFF);
#endif
    Transaction.ui32NumBytes    = ui32NumBytes;
    Transaction.pui32RxBuffer   = (uint32_t *)pui8RxBuffer;
    Transaction.uPeerInfo.ui32SpiChipSelect = pIom->ui32CS;
    Transaction.bContinue       = false;
    Transaction.ui8RepeatCount  = 0;
    Transaction.ui32PauseCondition = 0;
    Transaction.ui32StatusSetClr = 0;

    //
    // Start the transaction.
    //
    if (am_hal_iom_nonblocking_transfer(pIom->pIomHandle, &Transaction, pfnCallback, pCallbackCtxt))
    {
        return AM_DEVICES_MB85RS64V_STATUS_ERROR;
    }

    //
    // Return the status.
    //
    return AM_DEVICES_MB85RS64V_STATUS_SUCCESS;
}

//*****************************************************************************
//
// End Doxygen group.
//! @}
//
//*****************************************************************************

