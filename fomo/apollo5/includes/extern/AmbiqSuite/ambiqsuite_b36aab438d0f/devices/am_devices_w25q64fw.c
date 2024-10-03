//*****************************************************************************
//
//! @file am_devices_w25q64fw.c
//!
//! @brief Windbond SPI Flash driver
//!
//! @addtogroup w25q64fw W25Q64FW SPI Flash Driver
//! @ingroup devices
//! @{
//
//**************************************************************************

//*****************************************************************************
//
// ${copyright}
//
// This is part of revision ${version} of the AmbiqSuite Development Package.
//
//*****************************************************************************

#include "am_mcu_apollo.h"
#include "am_devices_w25q64fw.h"
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
} am_devices_iom_w25q64fw_t;

am_devices_iom_w25q64fw_t gAmW25q64fw[AM_DEVICES_W25Q64FW_MAX_DEVICE_NUM];

am_hal_iom_config_t     g_sIomW25q64fwCfg =
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
//! @param pHandle
//! @param ui32InstrLen
//! @param ui32Instr
//! @param pData
//! @param ui32NumBytes
//! @return
//
//*****************************************************************************
static uint32_t
am_device_command_write(void *pHandle, uint32_t ui32InstrLen, uint32_t ui32Instr,
                        uint32_t *pData, uint32_t ui32NumBytes)
{
    am_hal_iom_transfer_t Transaction;
    am_devices_iom_w25q64fw_t *pIom = (am_devices_iom_w25q64fw_t *)pHandle;

    //
    // Create the transaction.
    //
    Transaction.ui32InstrLen    = ui32InstrLen;
    Transaction.ui32Instr       = ui32Instr;
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
        return AM_DEVICES_W25Q64FW_STATUS_ERROR;
    }
    return AM_DEVICES_W25Q64FW_STATUS_SUCCESS;
}

//*****************************************************************************
//
//! @brief
//! @param pHandle
//! @param ui32InstrLen
//! @param ui32Instr
//! @param pData
//! @param ui32NumBytes
//! @return
//
//*****************************************************************************
static uint32_t
am_device_command_read(void *pHandle, uint32_t ui32InstrLen, uint32_t ui32Instr,
                       uint32_t *pData, uint32_t ui32NumBytes)
{
    am_hal_iom_transfer_t Transaction;
    am_devices_iom_w25q64fw_t *pIom = (am_devices_iom_w25q64fw_t *)pHandle;

    //
    // Create the transaction.
    //
    Transaction.ui32InstrLen    = ui32InstrLen;
    Transaction.ui32Instr       = ui32Instr;
    Transaction.eDirection      = AM_HAL_IOM_RX;
    Transaction.ui32NumBytes    = ui32NumBytes;
    Transaction.pui32RxBuffer   = pData;
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
        return AM_DEVICES_W25Q64FW_STATUS_ERROR;
    }
    return AM_DEVICES_W25Q64FW_STATUS_SUCCESS;
}

//*****************************************************************************
//
// Initialize the w25q64fw driver.
//
//*****************************************************************************
uint32_t
am_devices_w25q64fw_init(uint32_t ui32Module, am_devices_w25q64fw_config_t *pDevConfig, void **ppHandle, void **ppIomHandle)
{
    void *pIomHandle;
    am_hal_iom_config_t     stIOMW25Q64FWSettings;

    uint32_t g_CS[AM_REG_IOM_NUM_MODULES] =
#ifndef AM_PART_APOLLO4
#if defined(APOLLO3_EVB_CYGNUS) || defined(APOLLO3P_EVB_CYGNUS)
    {
        AM_BSP_FLASH2_CS_CHNL,
        AM_BSP_IOM1_CS_CHNL,
        0,
        AM_BSP_IOM3_CS_CHNL,
        AM_BSP_IOM4_CS_CHNL,
        0
    };
#else
    {
      AM_BSP_IOM0_CS_CHNL,
      AM_BSP_IOM1_CS_CHNL,
      AM_BSP_IOM2_CS_CHNL,
      AM_BSP_IOM3_CS_CHNL,
      AM_BSP_IOM4_CS_CHNL,
      AM_BSP_IOM5_CS_CHNL,
    };
#endif
#else
    {0, 0, 0, 0, 0, 0, 0, 0};
#endif

    uint32_t      ui32Index = 0;

    // Allocate a vacant device handle
    for ( ui32Index = 0; ui32Index < AM_DEVICES_W25Q64FW_MAX_DEVICE_NUM; ui32Index++ )
    {
        if ( gAmW25q64fw[ui32Index].bOccupied == false )
        {
            break;
        }
    }
    if ( ui32Index == AM_DEVICES_W25Q64FW_MAX_DEVICE_NUM )
    {
        return AM_DEVICES_W25Q64FW_STATUS_ERROR;
    }

    if ( (ui32Module > AM_REG_IOM_NUM_MODULES)  || (pDevConfig == NULL) )
    {
        return AM_DEVICES_W25Q64FW_STATUS_ERROR;
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

    stIOMW25Q64FWSettings = g_sIomW25q64fwCfg;
    stIOMW25Q64FWSettings.ui32NBTxnBufLength = pDevConfig->ui32NBTxnBufLength;
    stIOMW25Q64FWSettings.pNBTxnBuf = pDevConfig->pNBTxnBuf;
    stIOMW25Q64FWSettings.ui32ClockFreq = pDevConfig->ui32ClockFreq;

    //
    // Initialize the IOM instance.
    // Enable power to the IOM instance.
    // Configure the IOM for Serial operation during initialization.
    // Enable the IOM.
    // HAL Success return is 0
    //
    if (am_hal_iom_initialize(ui32Module, &pIomHandle) ||
        am_hal_iom_power_ctrl(pIomHandle, AM_HAL_SYSCTRL_WAKE, false) ||
        am_hal_iom_configure(pIomHandle, &stIOMW25Q64FWSettings) ||
        am_hal_iom_enable(pIomHandle))
    {
        return AM_DEVICES_W25Q64FW_STATUS_ERROR;
    }
    else
    {
        gAmW25q64fw[ui32Index].bOccupied = true;
        gAmW25q64fw[ui32Index].ui32CS = g_CS[ui32Module];
        gAmW25q64fw[ui32Index].ui32Module = ui32Module;
        *ppIomHandle = gAmW25q64fw[ui32Index].pIomHandle = pIomHandle;
        *ppHandle = (void *)&gAmW25q64fw[ui32Index];

        //
        // Return the status.
        //
        return AM_DEVICES_W25Q64FW_STATUS_SUCCESS;
    }
}

//*****************************************************************************
//
// De-Initialize the w25q64fw driver.
//
//*****************************************************************************
uint32_t
am_devices_w25q64fw_term(void *pHandle)
{
    am_devices_iom_w25q64fw_t *pIom = (am_devices_iom_w25q64fw_t *)pHandle;

    if ( pIom->ui32Module > AM_REG_IOM_NUM_MODULES )
    {
        return AM_DEVICES_W25Q64FW_STATUS_ERROR;
    }

    // Disable the pins
    am_bsp_iom_pins_disable(pIom->ui32Module, AM_HAL_IOM_SPI_MODE);

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
    return AM_DEVICES_W25Q64FW_STATUS_SUCCESS;
}

//*****************************************************************************
//
// Resets the device
//
//*****************************************************************************
uint32_t
am_devices_w25q64fw_reset(void *pHandle)
{
    uint32_t      ui32PIOBuffer[32] = {0};
    //
    // Send the command sequence to enable resetting the device.
    //
    if (am_device_command_write(pHandle, 1, AM_DEVICES_W25Q64FW_ENABLE_RESET, ui32PIOBuffer, 0))
    {
        return AM_DEVICES_W25Q64FW_STATUS_ERROR;
    }
    //
    // Send the command sequence to enable resetting the device.
    //
    if (am_device_command_write(pHandle, 1, AM_DEVICES_W25Q64FW_RESET_DEVICE, ui32PIOBuffer, 0))
    {
        return AM_DEVICES_W25Q64FW_STATUS_ERROR;
    }
    return AM_DEVICES_W25Q64FW_STATUS_SUCCESS;

}

//*****************************************************************************
//
// Reads the ID of the external FRAM and returns the value.
//
//*****************************************************************************
uint32_t
am_devices_w25q64fw_read_id(void *pHandle, uint32_t *pDeviceID)
{
    //
    // Send the command sequence to read the Device ID.
    //
    if (am_device_command_read(pHandle, 1, AM_DEVICES_W25Q64FW_READ_DEVICE_ID, pDeviceID, 4))
    {
        return AM_DEVICES_W25Q64FW_STATUS_ERROR;
    }
    return AM_DEVICES_W25Q64FW_STATUS_SUCCESS;

}

//*****************************************************************************
//
// Reads the current status of the external FRAM
//
//*****************************************************************************
uint32_t
am_devices_w25q64fw_status_get(void *pHandle, uint32_t *pStatus)
{
    //
    // Send the command sequence to read the device status.
    //
    if (am_device_command_read(pHandle, 1, AM_DEVICES_W25Q64FW_READ_STATUS, pStatus, 1))
    {
        return AM_DEVICES_W25Q64FW_STATUS_ERROR;
    }
    return AM_DEVICES_W25Q64FW_STATUS_SUCCESS;
}

//*****************************************************************************
//
// @brief Sends a specific command to the device (blocking).
//
//*****************************************************************************
uint32_t
am_devices_w25q64fw_command_send(void *pHandle, uint32_t ui32Cmd)
{
    uint32_t Dummy;

    if ( ui32Cmd == AM_DEVICES_W25Q64FW_CMD_WREN )
    {
        //
        // Send the command to enable writing.
        //
        if (am_device_command_write(pHandle, 1, AM_DEVICES_W25Q64FW_WRITE_ENABLE, &Dummy, 0))
        {
            return AM_DEVICES_W25Q64FW_STATUS_ERROR;
        }

    }
    else if ( ui32Cmd == AM_DEVICES_W25Q64FW_CMD_WRDI )
    {
        //
        // Send the command to enable writing.
        //
        if (am_device_command_write(pHandle, 1, AM_DEVICES_W25Q64FW_WRITE_DISABLE, &Dummy, 0))
        {
            return AM_DEVICES_W25Q64FW_STATUS_ERROR;
        }
    }
    else
    {
        return AM_DEVICES_W25Q64FW_STATUS_ERROR;
    }
    return AM_DEVICES_W25Q64FW_STATUS_SUCCESS;

}

//*****************************************************************************
//
// Erases the contents of a single sector of flash
//
//*****************************************************************************
uint32_t
am_devcies_w25q64fw_sector_erase(void *pHandle, uint32_t ui32SectorAddress)
{
    bool          bEraseComplete = false;
    uint32_t      ui32Status;
    uint32_t      ui32PIOBuffer[32] = {0};

    //
    // Send the command to remove protection from the sector.
    //
    ui32Status = am_device_command_write(pHandle, 1, AM_DEVICES_W25Q64FW_BLOCK_UNLOCK, &ui32SectorAddress, 3);
    if (AM_HAL_STATUS_SUCCESS != ui32Status)
    {
      return AM_DEVICES_W25Q64FW_STATUS_ERROR;
    }

    //
    // Send the command sequence to enable writing.
    //
    ui32Status = am_device_command_write(pHandle, 1, AM_DEVICES_W25Q64FW_WRITE_ENABLE, ui32PIOBuffer, 0);
    if (AM_HAL_STATUS_SUCCESS != ui32Status)
    {
        return AM_DEVICES_W25Q64FW_STATUS_ERROR;
    }

    //
    // Send the command sequence to do the sector erase.
    //
    ui32Status = am_device_command_write(pHandle, 1, AM_DEVICES_W25Q64FW_BLOCK_ERASE, &ui32SectorAddress, 3);
    if (AM_HAL_STATUS_SUCCESS != ui32Status)
    {
        return AM_DEVICES_W25Q64FW_STATUS_ERROR;
    }

    //
    // Wait for the Write In Progress to indicate the erase is complete.
    //
    for (uint32_t i = 0; i < AM_DEVICES_W25Q64FW_MAX_TIMEOUT; i++)
    {
        am_device_command_read(pHandle, 1, AM_DEVICES_W25Q64FW_READ_STATUS, ui32PIOBuffer, 2);
        bEraseComplete = (0 == (ui32PIOBuffer[0] & AM_DEVICES_W25Q64FW_WIP));

        if (bEraseComplete)
        {
            break;
        }
        am_hal_flash_delay( FLASH_CYCLES_US(10000) );
    }

    //
    // Check the status.
    //
    if (!bEraseComplete)
    {
        return AM_DEVICES_W25Q64FW_STATUS_ERROR;
    }

    //
    // Send the command sequence to disable writing.
    //
    ui32Status = am_device_command_write(pHandle, 1, AM_DEVICES_W25Q64FW_WRITE_DISABLE, ui32PIOBuffer, 0);
    if (AM_HAL_STATUS_SUCCESS != ui32Status)
    {
        return AM_DEVICES_W25Q64FW_STATUS_ERROR;
    }

    //
    // Return the status.
    //
    return AM_DEVICES_W25Q64FW_STATUS_SUCCESS;
}

//*****************************************************************************
//
// Programs the given range of FRAM addresses.
//
//*****************************************************************************
uint32_t
am_devices_w25q64fw_blocking_write(void *pHandle, uint8_t *pui8TxBuffer,
                                   uint32_t ui32WriteAddress,
                                   uint32_t ui32NumBytes)
{
    am_hal_iom_transfer_t       Transaction;
    uint32_t                    Dummy;
    am_devices_iom_w25q64fw_t  *pIom = (am_devices_iom_w25q64fw_t *)pHandle;
    bool                        bWriteComplete = false;
    uint32_t                    ui32BytesLeft = ui32NumBytes;
    uint32_t                    ui32PageAddress = ui32WriteAddress;
    uint32_t                    ui32BufferAddress = (uint32_t)pui8TxBuffer;
    uint32_t                    ui32PIOBuffer[32] = {0};

    while (ui32BytesLeft > 0)
    {
        //
        // Send the WRITE ENABLE command to enable writing.
        //
        if (am_device_command_write(pHandle, 1, AM_DEVICES_W25Q64FW_WRITE_ENABLE, &Dummy, 0))
        {
            return AM_DEVICES_W25Q64FW_STATUS_ERROR;
        }

        Transaction.ui8RepeatCount  = 0;
        Transaction.ui32PauseCondition = 0;
        Transaction.ui32StatusSetClr = 0;

        //
        // Write the command to the device.
        //
        Transaction.eDirection      = AM_HAL_IOM_TX;
        Transaction.ui32InstrLen    = 1;
        Transaction.ui32Instr       = AM_DEVICES_W25Q64FW_WRITE;
        Transaction.ui32NumBytes    = 0;
        Transaction.pui32TxBuffer   = (uint32_t *)pui8TxBuffer;
        Transaction.uPeerInfo.ui32SpiChipSelect = pIom->ui32CS;
        Transaction.bContinue       = true;

        //
        // Start the transaction.
        //
        if (am_hal_iom_blocking_transfer(pIom->pIomHandle, &Transaction))
        {
            return AM_DEVICES_W25Q64FW_STATUS_ERROR;
        }

        //
        // Write the offset (24 bits) to the device.
        //
        Transaction.eDirection      = AM_HAL_IOM_TX;
        if (ui32BytesLeft > AM_DEVICES_W25Q64FW_PAGE_SIZE)
        {
            // Set the transfer count in bytes.
            Transaction.ui32NumBytes = AM_DEVICES_W25Q64FW_PAGE_SIZE;
            ui32BytesLeft -= AM_DEVICES_W25Q64FW_PAGE_SIZE;
        }
        else
        {
            // Set the transfer count in bytes.
            Transaction.ui32NumBytes = ui32BytesLeft;
            ui32BytesLeft = 0;
        }
        // Set the address to write data to.
        Transaction.ui32InstrLen    = 3;
        Transaction.ui32Instr       = ui32PageAddress & 0x00FFFFFF;
        ui32PageAddress += AM_DEVICES_W25Q64FW_PAGE_SIZE;
        // Set the source SRAM buffer address.
        Transaction.pui32TxBuffer   = (uint32_t *)ui32BufferAddress;
        ui32BufferAddress += AM_DEVICES_W25Q64FW_PAGE_SIZE;
        Transaction.uPeerInfo.ui32SpiChipSelect = pIom->ui32CS;
        Transaction.bContinue       = false;

        //
        // Start the transaction.
        //
        if (am_hal_iom_blocking_transfer(pIom->pIomHandle, &Transaction))
        {
            return AM_DEVICES_W25Q64FW_STATUS_ERROR;
        }

        //
        // Wait for the Write In Progress to indicate the erase is complete.
        //
        for (uint32_t i = 0; i < AM_DEVICES_W25Q64FW_MAX_TIMEOUT; i++)
        {
            am_device_command_read(pHandle, 1, AM_DEVICES_W25Q64FW_READ_STATUS, ui32PIOBuffer, 2);
            bWriteComplete = (0 == (ui32PIOBuffer[0] & AM_DEVICES_W25Q64FW_WIP));

            if (bWriteComplete)
            {
                break;
            }
            am_hal_flash_delay( FLASH_CYCLES_US(10000) );
        }

        //
        // Check the status.
        //
        if (!bWriteComplete)
        {
            return AM_DEVICES_W25Q64FW_STATUS_ERROR;
        }

        // No Need to issue AM_DEVICES_W25Q64FW_WRITE_DISABLE, as WEL gets deasserted
        // following the WRITE transaction
    }

    //
    // Return the status.
    //
    return AM_DEVICES_W25Q64FW_STATUS_SUCCESS;
}

//*****************************************************************************
//
// Programs the given range of FRAM addresses.
//
//*****************************************************************************
uint32_t
am_devices_w25q64fw_nonblocking_write_adv(void *pHandle, uint8_t *pui8TxBuffer,
                                          uint32_t ui32WriteAddress,
                                          uint32_t ui32NumBytes,
                                          uint32_t ui32PauseCondition,
                                          uint32_t ui32StatusSetClr,
                                          am_hal_iom_callback_t pfnCallback,
                                          void *pCallbackCtxt)
{
    am_hal_iom_transfer_t Transaction;
    am_devices_iom_w25q64fw_t *pIom = (am_devices_iom_w25q64fw_t *)pHandle;

    bool                        bWriteComplete = false;
    uint32_t                    ui32BytesLeft = ui32NumBytes;
    uint32_t                    ui32PageAddress = ui32WriteAddress;
    uint32_t                    ui32BufferAddress = (uint32_t)pui8TxBuffer;
    uint32_t                    ui32PIOBuffer[32] = {0};

    while (ui32BytesLeft > 0)
    {
        //
        // Common transaction parameters
        //
        Transaction.ui8Priority     = 1;        // High priority for now.
        Transaction.uPeerInfo.ui32SpiChipSelect = pIom->ui32CS;
        Transaction.ui8RepeatCount  = 0;
        Transaction.ui32PauseCondition = ui32PauseCondition;

        //
        // Set up the IOM transaction to send the WREN device command.
        //
        Transaction.eDirection      = AM_HAL_IOM_TX;
        Transaction.ui32InstrLen    = 1;        // Sending 1 offset byte
        Transaction.ui32Instr       = AM_DEVICES_W25Q64FW_WRITE_ENABLE;
        Transaction.ui32NumBytes    = 0;        // WREN CMD is sent as the offset
        Transaction.pui32TxBuffer   = (uint32_t *)pui8TxBuffer; // Not used for this CMD
        Transaction.bContinue       = false;
        Transaction.ui32StatusSetClr = 0;

        //
        // Start the transaction (no callback).
        //
        if (am_hal_iom_nonblocking_transfer(pIom->pIomHandle, &Transaction, 0, 0))
        {
            return AM_DEVICES_W25Q64FW_STATUS_ERROR;
        }

        //
        // Set up the IOM transaction to write the FRAM command to the device.
        // This one needs to keep CE asserted (via continue).
        //
        Transaction.eDirection      = AM_HAL_IOM_TX;
        Transaction.ui32InstrLen    = 1;
        Transaction.ui32Instr       = AM_DEVICES_W25Q64FW_WRITE;
        Transaction.ui32NumBytes    = 0;
        Transaction.pui32TxBuffer   = (uint32_t *)pui8TxBuffer;
        Transaction.ui32PauseCondition = 0;
        Transaction.ui32StatusSetClr = 0;
        Transaction.bContinue       = true;

        //
        // Start the transaction (no callback).
        //
        if (am_hal_iom_nonblocking_transfer(pIom->pIomHandle, &Transaction, 0, 0))
        {
            return AM_DEVICES_W25Q64FW_STATUS_ERROR;
        }

        if (ui32NumBytes)
        {
            //
            // Write the offset (24 bits) to the device.
            //
            Transaction.eDirection      = AM_HAL_IOM_TX;
            if (ui32BytesLeft > AM_DEVICES_W25Q64FW_PAGE_SIZE)
            {
                // Set the transfer count in bytes.
                Transaction.ui32NumBytes = AM_DEVICES_W25Q64FW_PAGE_SIZE;
                ui32BytesLeft -= AM_DEVICES_W25Q64FW_PAGE_SIZE;
            }
            else
            {
                // Set the transfer count in bytes.
                Transaction.ui32NumBytes = ui32BytesLeft;
                ui32BytesLeft = 0;
            }
            // Set the address to write data to.
            Transaction.ui32InstrLen    = 3;
            Transaction.ui32Instr       = ui32PageAddress & 0x00FFFFFF;
            ui32PageAddress += AM_DEVICES_W25Q64FW_PAGE_SIZE;
            // Set the source SRAM buffer address.
            Transaction.pui32TxBuffer   = (uint32_t *)ui32BufferAddress;
            ui32BufferAddress += AM_DEVICES_W25Q64FW_PAGE_SIZE;
            Transaction.uPeerInfo.ui32SpiChipSelect = pIom->ui32CS;
            Transaction.bContinue       = false;

            //
            // Start the transaction
            //
            if (am_hal_iom_nonblocking_transfer(pIom->pIomHandle, &Transaction, pfnCallback, pCallbackCtxt))
            {
                return AM_DEVICES_W25Q64FW_STATUS_ERROR;
            }

            //
            // Wait for the Write In Progress to indicate the erase is complete.
            //
            for (uint32_t i = 0; i < AM_DEVICES_W25Q64FW_MAX_TIMEOUT; i++)
            {
                am_device_command_read(pHandle, 1, AM_DEVICES_W25Q64FW_READ_STATUS, ui32PIOBuffer, 2);
                bWriteComplete = (0 == (ui32PIOBuffer[0] & AM_DEVICES_W25Q64FW_WIP));

                if (bWriteComplete)
                {
                    break;
                }
                am_hal_flash_delay( FLASH_CYCLES_US(10000) );
            }

            //
            // Check the status.
            //
            if (!bWriteComplete)
            {
                return AM_DEVICES_W25Q64FW_STATUS_ERROR;
            }
        }
        // No Need to issue AM_DEVICES_W25Q64FW_WRITE_DISABLE, as WEL gets deasserted
        // following the WRITE transaction
    }

    //
    // Return the status.
    //
    return AM_DEVICES_W25Q64FW_STATUS_SUCCESS;
}

//*****************************************************************************
//
// Programs the given range of FRAM addresses.
//
//*****************************************************************************
uint32_t
am_devices_w25q64fw_nonblocking_write(void *pHandle, uint8_t *pui8TxBuffer,
                                      uint32_t ui32WriteAddress,
                                      uint32_t ui32NumBytes,
                                      am_hal_iom_callback_t pfnCallback,
                                      void *pCallbackCtxt)
{
#if 0
    am_hal_iom_transfer_t Transaction;

    //
    // Common transaction parameters
    //
    Transaction.ui8Priority     = 1;        // High priority for now.
    Transaction.uPeerInfo.ui32SpiChipSelect = g_W25Q64FWCS;
    Transaction.ui8RepeatCount  = 0;
    Transaction.ui32PauseCondition       = 0;
    Transaction.ui32StatusSetClr = 0;

#if 0
    //
    // Set up the IOM transaction to send the WREN device command.
    //
    Transaction.eDirection      = AM_HAL_IOM_TX;
    Transaction.ui32InstrLen    = 1;        // Sending 1 offset byte
    Transaction.ui32Instr       = AM_DEVICES_W25Q64FW_WRITE_ENABLE;
    Transaction.ui32NumBytes    = 0;        // WREN CMD is sent as the offset
    Transaction.pui32TxBuffer   = (uint32_t *)pui8TxBuffer; // Not used for this CMD
    Transaction.bContinue       = false;

    //
    // Start the transaction (no callback).
    //
    if (am_hal_iom_nonblocking_transfer(pHandle, &Transaction, 0, 0))
    {
        return AM_DEVICES_W25Q64FW_STATUS_ERROR;
    }
#endif

    // Do the transaction (no callback).
    if (am_devices_w25q64fw_nonblocking_write_adv(pui8TxBuffer,
                                                   ui32WriteAddress,
                                                   ui32NumBytes,
                                                   0, 0, 0, 0))
    {
        return AM_DEVICES_W25Q64FW_STATUS_ERROR;
    }

    //
    // Set up the IOM transaction to send the WRDI (write disable) device command.
    //
    Transaction.eDirection      = AM_HAL_IOM_TX;
    Transaction.ui32InstrLen    = 1;        // Sending 1 offset byte
    Transaction.ui32Instr       = AM_DEVICES_W25Q64FW_WRITE_DISABLE;
    Transaction.ui32NumBytes    = 0;        // WRDI CMD is sent as the offset
    Transaction.pui32TxBuffer   = (uint32_t *)pui8TxBuffer; // Not used for this CMD
    Transaction.bContinue       = false;

    //
    // Start the transaction, sending the callback.
    //
    if (am_hal_iom_nonblocking_transfer(pHandle, &Transaction, pfnCallback, pCallbackCtxt))
    {
        return AM_DEVICES_W25Q64FW_STATUS_ERROR;
    }

    //
    // Return the status.
    //
    return AM_DEVICES_W25Q64FW_STATUS_SUCCESS;
#else
    return (am_devices_w25q64fw_nonblocking_write_adv(pHandle, pui8TxBuffer,
                                                   ui32WriteAddress,
                                                   ui32NumBytes,
                                                   0, 0, pfnCallback, pCallbackCtxt));
#endif
}

//*****************************************************************************
//
// Reads the contents of the fram into a buffer.
//
//*****************************************************************************
uint32_t
am_devices_w25q64fw_blocking_read(void *pHandle, uint8_t *pui8RxBuffer,
                                  uint32_t ui32ReadAddress,
                                  uint32_t ui32NumBytes)
{
    am_hal_iom_transfer_t Transaction;
    uint32_t              ui32Dummy;
    am_devices_iom_w25q64fw_t *pIom = (am_devices_iom_w25q64fw_t *)pHandle;

    Transaction.ui8RepeatCount  = 0;
    Transaction.ui32PauseCondition = 0;
    Transaction.ui32StatusSetClr = 0;

    //
    // Set up the IOM transaction to write the FRAM command to the device.
    // This one needs to keep CE asserted (via continue).
    //
    Transaction.eDirection      = AM_HAL_IOM_TX;
    Transaction.ui32InstrLen    = 1;
    Transaction.ui32Instr       = AM_DEVICES_W25Q64FW_FAST_READ;
    Transaction.ui32NumBytes    = 0;
    Transaction.pui32TxBuffer   = (uint32_t *)&pui8RxBuffer;
    Transaction.uPeerInfo.ui32SpiChipSelect = pIom->ui32CS;
    Transaction.bContinue       = true;

    //
    // Start the transaction.
    //
    if (am_hal_iom_blocking_transfer(pIom->pIomHandle, &Transaction))
    {
        return AM_DEVICES_W25Q64FW_STATUS_ERROR;
    }

    //
    // Set up the IOM transaction to write the FRAM command to the device.
    // This one needs to keep CE asserted (via continue).
    //
    ui32Dummy                   = 0;
    Transaction.eDirection      = AM_HAL_IOM_TX;
    Transaction.ui32InstrLen    = 3;
    Transaction.ui32Instr       = ui32ReadAddress;
    Transaction.ui32NumBytes    = 1;
    Transaction.pui32TxBuffer   = (uint32_t *)&ui32Dummy;
    Transaction.uPeerInfo.ui32SpiChipSelect = pIom->ui32CS;
    Transaction.bContinue       = true;

    //
    // Start the transaction.
    //
    if (am_hal_iom_blocking_transfer(pIom->pIomHandle, &Transaction))
    {
        return AM_DEVICES_W25Q64FW_STATUS_ERROR;
    }

    //
    // Set up the IOM transaction.
    //
    Transaction.ui8Priority     = 1;        // High priority for now.
    Transaction.eDirection      = AM_HAL_IOM_RX;
    Transaction.ui32InstrLen    = 0;
    Transaction.ui32Instr       = 0;
    Transaction.ui32NumBytes    = ui32NumBytes;
    Transaction.pui32RxBuffer   = (uint32_t *)pui8RxBuffer;
    Transaction.uPeerInfo.ui32SpiChipSelect = pIom->ui32CS;
    Transaction.bContinue       = false;

    //
    // Start the transaction.
    //
    if (am_hal_iom_blocking_transfer(pIom->pIomHandle, &Transaction))
    {
        return AM_DEVICES_W25Q64FW_STATUS_ERROR;
    }

    //
    // Return the status.
    //
    return AM_DEVICES_W25Q64FW_STATUS_SUCCESS;
}

//*****************************************************************************
//
// Reads the contents of the fram into a buffer.
//
//*****************************************************************************
uint32_t
am_devices_w25q64fw_nonblocking_read(void *pHandle, uint8_t *pui8RxBuffer,
                                     uint32_t ui32ReadAddress,
                                     uint32_t ui32NumBytes,
                                     am_hal_iom_callback_t pfnCallback,
                                     void *pCallbackCtxt)
{
    am_hal_iom_transfer_t      Transaction;
    uint32_t                   ui32Dummy;
    am_devices_iom_w25q64fw_t *pIom = (am_devices_iom_w25q64fw_t *)pHandle;

    Transaction.ui8RepeatCount  = 0;
    Transaction.ui32PauseCondition = 0;
    Transaction.ui32StatusSetClr = 0;

    //
    // Set up the IOM transaction to write the FRAM command to the device.
    // This one needs to keep CE asserted (via continue).
    //
    Transaction.eDirection      = AM_HAL_IOM_TX;
    Transaction.ui32InstrLen    = 1;
    Transaction.ui32Instr       = AM_DEVICES_W25Q64FW_FAST_READ;
    Transaction.ui32NumBytes    = 0;
    Transaction.pui32TxBuffer   = (uint32_t *)&pui8RxBuffer;
    Transaction.uPeerInfo.ui32SpiChipSelect = pIom->ui32CS;
    Transaction.bContinue       = true;

    //
    // Start the transaction (no callback).
    //
    if (am_hal_iom_nonblocking_transfer(pIom->pIomHandle, &Transaction, 0, 0))
    {
        return AM_DEVICES_W25Q64FW_STATUS_ERROR;
    }

    //
    // Set up the IOM transaction to write the FRAM command to the device.
    // This one needs to keep CE asserted (via continue).
    //
    ui32Dummy                   = 0;
    Transaction.eDirection      = AM_HAL_IOM_TX;
    Transaction.ui32InstrLen    = 3;
    Transaction.ui32Instr       = ui32ReadAddress;
    Transaction.ui32NumBytes    = 1;
    Transaction.pui32TxBuffer   = (uint32_t *)&ui32Dummy;
    Transaction.uPeerInfo.ui32SpiChipSelect = pIom->ui32CS;
    Transaction.bContinue       = true;

    //
    // Start the transaction (no callback).
    //
    if (am_hal_iom_nonblocking_transfer(pIom->pIomHandle, &Transaction, 0, 0))
    {
        return AM_DEVICES_W25Q64FW_STATUS_ERROR;
    }

    //
    // Set up the IOM transaction.
    //
    Transaction.ui8Priority     = 1;        // High priority for now.
    Transaction.eDirection      = AM_HAL_IOM_RX;
    Transaction.ui32InstrLen    = 0;
    Transaction.ui32Instr       = 0;
    Transaction.ui32NumBytes    = ui32NumBytes;
    Transaction.pui32RxBuffer   = (uint32_t *)pui8RxBuffer;
    Transaction.uPeerInfo.ui32SpiChipSelect = pIom->ui32CS;
    Transaction.bContinue       = false;

    //
    // Start the transaction.
    //
    if (am_hal_iom_nonblocking_transfer(pIom->pIomHandle, &Transaction, pfnCallback, pCallbackCtxt))
    {
        return AM_DEVICES_W25Q64FW_STATUS_ERROR;
    }

    //
    // Return the status.
    //
    return AM_DEVICES_W25Q64FW_STATUS_SUCCESS;
}

//*****************************************************************************
//
// End Doxygen group.
//! @}
//
//*****************************************************************************

