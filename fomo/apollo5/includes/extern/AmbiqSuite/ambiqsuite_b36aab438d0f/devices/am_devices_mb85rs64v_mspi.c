//*****************************************************************************
//
//! @file am_devices_mb85rs64v_mspi.c
//!
//! @brief Fujitsu 64K SPI FRAM driver.
//!
//! @addtogroup mb85rs64v_mspi MB85RS64V MSPI FRAM Driver
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
#include "am_util_stdio.h"
#include "am_devices_mb85rs64v_mspi.h"
#include "am_bsp.h"
#include "am_util_delay.h"

#define MSPI_TRANSFER_TIMEOUT   1000

//*****************************************************************************
//
// Global variables.
//
//*****************************************************************************
const am_hal_mspi_dev_config_t      MSPI_FRAM_Serial_CE0_MSPIConfig =
{
    .eSpiMode             = AM_HAL_MSPI_SPI_MODE_0,
    .eClockFreq           = AM_HAL_MSPI_CLK_24MHZ,
    .ui8TurnAround        = 3,
    .eAddrCfg             = AM_HAL_MSPI_ADDR_3_BYTE,
    .eInstrCfg            = AM_HAL_MSPI_INSTR_1_BYTE,
    .eDeviceConfig        = AM_HAL_MSPI_FLASH_SERIAL_CE0,
    .bSendInstr           = false,
    .bSendAddr            = true,
    .bTurnaround          = false,
    .ui8ReadInstr         = AM_DEVICES_MB85RS64V_READ,
    .ui8WriteInstr        = AM_DEVICES_MB85RS64V_WRITE,
#if defined(AM_PART_APOLLO3P)
    // #### INTERNAL BEGIN ####
    // TODO - Fixme.  What is the proper value? (ui8WriteLatency = 0?)
    // TODO - Fixme.  What is the proper value? (bEnWriteLatency = false?)
    // TODO - Fixme.  What is the proper value? (.ui16DMATimeLimit = 0?)
    // #### INTERNAL END ####
    .ui8WriteLatency      = 0,
    .bEnWriteLatency      = false,
    .bEmulateDDR          = false,
    .ui16DMATimeLimit     = 0,
    .eDMABoundary         = AM_HAL_MSPI_BOUNDARY_NONE,
#endif
    .ui32TCBSize          = 0,
    .pTCB                 = 0,
    .scramblingStartAddr  = 0,
    .scramblingEndAddr    = 0,
};

typedef struct
{
    uint32_t                    ui32Module;
    void                        *pMspiHandle;
    bool                        bOccupied;
} am_devices_mspi_fram_t;

am_devices_mspi_fram_t gAmFram[AM_DEVICES_MSPI_FRAM_MAX_DEVICE_NUM];

static void pfnMSPI_FRAM_Callback(void *pCallbackCtxt, uint32_t status)
{
    // Set the DMA complete flag.
    *(volatile bool *)pCallbackCtxt = true;
}

//*****************************************************************************
//
//! @brief Generic Command Write function.
//!
//! @param pHandle      - Pointer to driver handle
//! @param ui32InstrLen
//! @param ui32Instr
//! @param pData
//! @param ui32NumBytes
//!
//! @return status from am_devices_mb85rs64v_status_t
//
//*****************************************************************************
static uint32_t
am_device_command_write(void *pHandle, uint32_t ui32InstrLen, uint32_t ui32Instr,
                        uint32_t *pData, uint32_t ui32NumBytes)
{
    am_hal_mspi_pio_transfer_t Transaction;
    am_devices_mspi_fram_t *pFram = (am_devices_mspi_fram_t *)pHandle;

    //
    // Create the transaction.
    //
    Transaction.ui32NumBytes      = ui32NumBytes;
    Transaction.bScrambling       = false;
    Transaction.eDirection        = AM_HAL_MSPI_TX;

    // Update the global configuration based on the transaction needs.
    switch (ui32InstrLen)
    {
        case 0:
            Transaction.bSendAddr       = false;
            break;
#if AM_CMSIS_REGS
        case 1:
            MSPIn(pFram->ui32Module)->CFG_b.ASIZE = AM_HAL_MSPI_ADDR_1_BYTE;
            Transaction.bSendAddr       = true;
            Transaction.ui32DeviceAddr  = ui32Instr;
            break;
        case 2:
            MSPIn(pFram->ui32Module)->CFG_b.ASIZE = AM_HAL_MSPI_ADDR_2_BYTE;
            Transaction.bSendAddr       = true;
            Transaction.ui32DeviceAddr  = ui32Instr;
            break;
        case 3:
            MSPIn(pFram->ui32Module)->CFG_b.ASIZE = AM_HAL_MSPI_ADDR_3_BYTE;
            Transaction.bSendAddr       = true;
            Transaction.ui32DeviceAddr  = ui32Instr;
            break;
#else // AM_CMSIS_REGS
        case 1:
            AM_BFWn(MSPI, pFram->ui32Module, CFG, ASIZE, AM_HAL_MSPI_ADDR_1_BYTE);
            Transaction.bSendAddr       = true;
            Transaction.ui32DeviceAddr  = ui32Instr;
            break;
        case 2:
            AM_BFWn(MSPI, pFram->ui32Module, CFG, ASIZE, AM_HAL_MSPI_ADDR_2_BYTE);
            Transaction.bSendAddr       = true;
            Transaction.ui32DeviceAddr  = ui32Instr;
            break;
        case 3:
            AM_BFWn(MSPI, pFram->ui32Module, CFG, ASIZE, AM_HAL_MSPI_ADDR_3_BYTE);
            Transaction.bSendAddr       = true;
            Transaction.ui32DeviceAddr  = ui32Instr;
            break;
#endif // AM_CMSIS_REGS
    }

    Transaction.bSendInstr        = false;
    Transaction.ui16DeviceInstr   = 0;
    Transaction.bTurnaround       = false;
#if 0 // A3DS-25 Deprecate MSPI CONT
    Transaction.bContinue         = false;
#endif // A3DS-25 Deprecate MSPI CONT
    Transaction.bQuadCmd          = false;
    Transaction.pui32Buffer       = pData;

    //
    // Execute the transction over IOM.
    //
    if (am_hal_mspi_blocking_transfer(pFram->pMspiHandle, &Transaction, MSPI_TRANSFER_TIMEOUT))
    {
        return AM_DEVICES_MB85RS64V_STATUS_ERROR;
    }

    return AM_DEVICES_MB85RS64V_STATUS_SUCCESS;
}

//*****************************************************************************
//
//! @brief Generic Command Read function.
//!
//! @param pHandle      - Pointer to driver handle
//! @param ui32InstrLen
//! @param ui32Instr
//! @param pData
//! @param ui32NumBytes
//!
//! @return status from am_devices_mb85rs64v_status_t
//
//*****************************************************************************
static uint32_t
am_device_command_read(void *pHandle, uint32_t ui32InstrLen, uint32_t ui32Instr,
                       uint32_t *pData, uint32_t ui32NumBytes)
{
    am_hal_mspi_pio_transfer_t Transaction;
    am_devices_mspi_fram_t *pFram = (am_devices_mspi_fram_t *)pHandle;

    //
    // Create the transaction.
    //
    Transaction.ui32NumBytes      = ui32NumBytes;
    Transaction.bScrambling       = false;
    Transaction.eDirection        = AM_HAL_MSPI_RX;

    // Update the global configuration based on the transaction needs.
    switch (ui32InstrLen)
    {
        case 0:
            Transaction.bSendAddr       = false;
            break;
#if AM_CMSIS_REGS
        case 1:
            MSPIn(pFram->ui32Module)->CFG_b.ASIZE = AM_HAL_MSPI_ADDR_1_BYTE;
            Transaction.bSendAddr       = true;
            Transaction.ui32DeviceAddr  = ui32Instr;
            break;
        case 2:
            MSPIn(pFram->ui32Module)->CFG_b.ASIZE = AM_HAL_MSPI_ADDR_2_BYTE;
            Transaction.bSendAddr       = true;
            Transaction.ui32DeviceAddr  = ui32Instr;
            break;
        case 3:
            MSPIn(pFram->ui32Module)->CFG_b.ASIZE = AM_HAL_MSPI_ADDR_3_BYTE;
            Transaction.bSendAddr       = true;
            Transaction.ui32DeviceAddr  = ui32Instr;
            break;
#else // AM_CMSIS_REGS
        case 1:
            AM_BFWn(MSPI, pFram->ui32Module, CFG, ASIZE, AM_HAL_MSPI_ADDR_1_BYTE);
            Transaction.bSendAddr       = true;
            Transaction.ui32DeviceAddr  = ui32Instr;
            break;
        case 2:
            AM_BFWn(MSPI, pFram->ui32Module, CFG, ASIZE, AM_HAL_MSPI_ADDR_2_BYTE);
            Transaction.bSendAddr       = true;
            Transaction.ui32DeviceAddr  = ui32Instr;
            break;
        case 3:
            AM_BFWn(MSPI, pFram->ui32Module, CFG, ASIZE, AM_HAL_MSPI_ADDR_3_BYTE);
            Transaction.bSendAddr       = true;
            Transaction.ui32DeviceAddr  = ui32Instr;
            break;
#endif // AM_CMSIS_REGS
    }

    Transaction.bSendInstr        = false;
    Transaction.ui16DeviceInstr   = 0;
    Transaction.bTurnaround       = false;
#if 0 // A3DS-25 Deprecate MSPI CONT
    Transaction.bContinue         = false;
#endif // A3DS-25
    Transaction.bQuadCmd          = false;
    Transaction.pui32Buffer       = pData;

    //
    // Execute the transction over IOM.
    //
    if (am_hal_mspi_blocking_transfer(pFram->pMspiHandle, &Transaction, MSPI_TRANSFER_TIMEOUT))
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
am_devices_mb85rs64v_mspi_init(uint32_t ui32Module, am_devices_mspi_fram_config_t *psMSPIConfig, void **ppHandle, void **ppMSPIHandle)
{
    am_hal_mspi_dev_config_t stConfig;
    void                     *pMspiHandle;
    uint32_t                 ui32Index = 0;

#if AM_APOLLO3_GPIO
    const am_hal_gpio_pincfg_t g_AM_HAL_PIN_OUTPUT =
    {
      .uFuncSel       = AM_HAL_PIN_0_GPIO,
      .eGPOutcfg      = AM_HAL_GPIO_PIN_OUTCFG_PUSHPULL,
    };
#else  // AM_APOLLO3_GPIO
#endif // AM_APOLLO3_GPIO

    if ((ui32Module > AM_REG_MSPI_NUM_MODULES) || (psMSPIConfig == NULL))
    {
        return AM_DEVICES_MB85RS64V_STATUS_ERROR;
    }

    // Allocate a vacant device handle
    for ( ui32Index = 0; ui32Index < AM_DEVICES_MSPI_FRAM_MAX_DEVICE_NUM; ui32Index++ )
    {
        if ( gAmFram[ui32Index].bOccupied == false )
        {
            break;
        }
    }
    if ( ui32Index == AM_DEVICES_MSPI_FRAM_MAX_DEVICE_NUM )
    {
        return AM_DEVICES_MB85RS64V_STATUS_ERROR;
    }

    stConfig = MSPI_FRAM_Serial_CE0_MSPIConfig;
    stConfig.eClockFreq = psMSPIConfig->eClockFreq;
    //stConfig.eDeviceConfig = psMSPIConfig->eDeviceConfig;
    stConfig.ui32TCBSize = psMSPIConfig->ui32NBTxnBufLength;
    stConfig.pTCB = psMSPIConfig->pNBTxnBuf;
    stConfig.scramblingStartAddr = psMSPIConfig->ui32ScramblingStartAddr;
    stConfig.scramblingEndAddr = psMSPIConfig->ui32ScramblingEndAddr;
    //
    // Configure the debug GPIOs.
    //
#if AM_APOLLO3_GPIO
    am_hal_gpio_pinconfig(0, g_AM_HAL_PIN_OUTPUT);
    am_hal_gpio_pinconfig(1, g_AM_HAL_PIN_OUTPUT);
    am_hal_gpio_pinconfig(2, g_AM_HAL_PIN_OUTPUT);
    am_hal_gpio_pinconfig(3, g_AM_HAL_PIN_OUTPUT);
    am_hal_gpio_pinconfig(4, g_AM_HAL_PIN_OUTPUT);
#else // AM_APOLLO3_GPIO
    am_hal_gpio_pin_config(0, AM_HAL_GPIO_OUTPUT);
    am_hal_gpio_pin_config(1, AM_HAL_GPIO_OUTPUT);
    am_hal_gpio_pin_config(2, AM_HAL_GPIO_OUTPUT);
    am_hal_gpio_pin_config(3, AM_HAL_GPIO_OUTPUT);
    am_hal_gpio_pin_config(4, AM_HAL_GPIO_OUTPUT);
#endif // AM_APOLLO3_GPIO

    //
    // Configure the IOM pins.
    //
    am_bsp_mspi_pins_enable(ui32Module, stConfig.eDeviceConfig);

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

    //
    // Initialize the MSPI instance.
    // Enable power to the MSPI instance.
    // Configure the MSPI for Serial operation during initialization.
    //
    if (am_hal_mspi_initialize(ui32Module, &pMspiHandle)                   ||
        am_hal_mspi_power_control(pMspiHandle, AM_HAL_SYSCTRL_WAKE, false) ||
        am_hal_mspi_device_configure(pMspiHandle, &stConfig)            ||
        am_hal_mspi_enable(pMspiHandle))
    {
        return AM_DEVICES_MB85RS64V_STATUS_ERROR;
    }
    else
    {
      //
      // Enable MSPI interrupts.
      //
      if (am_hal_mspi_interrupt_clear(pMspiHandle, AM_HAL_MSPI_INT_CQUPD | AM_HAL_MSPI_INT_ERR ) ||
          am_hal_mspi_interrupt_enable(pMspiHandle, AM_HAL_MSPI_INT_CQUPD | AM_HAL_MSPI_INT_ERR ))
      {
        return AM_DEVICES_MB85RS64V_STATUS_ERROR;
      }

        gAmFram[ui32Index].pMspiHandle = pMspiHandle;
        gAmFram[ui32Index].ui32Module = ui32Module;
        gAmFram[ui32Index].bOccupied = true;
        *ppMSPIHandle = pMspiHandle;
        *ppHandle = (void *)&gAmFram[ui32Index];

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
am_devices_mb85rs64v_mspi_deinit(void *pHandle)
{
    am_devices_mspi_fram_t *pFram = (am_devices_mspi_fram_t *)pHandle;

    if ( pFram->ui32Module > AM_REG_MSPI_NUM_MODULES )
    {
        return AM_DEVICES_MB85RS64V_STATUS_ERROR;
    }

    //
    // Deinit and disable power to the MSPI instance.
    //

    am_hal_mspi_disable(pFram->pMspiHandle);
    am_hal_mspi_power_control(pFram->pMspiHandle, AM_HAL_SYSCTRL_DEEPSLEEP, false);
    am_hal_mspi_deinitialize(pFram->pMspiHandle);

    // Free this device handle
    pFram->bOccupied = false;

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
am_devices_mb85rs64v_mspi_read_id(void *pHandle)
{
    uint32_t      ui32Status;
    uint32_t      ui32DeviceID;

    //
    // Send the command sequence to read the Device ID.
    //
    ui32Status = am_device_command_read(pHandle, 1, AM_DEVICES_MB85RS64V_READ_DEVICE_ID, &ui32DeviceID, 4);
    am_util_stdio_printf("FRAM ID is %8.8X\n", ui32DeviceID);
    if ( (ui32DeviceID  == AM_DEVICES_MB85RS64V_ID) &&
       (AM_HAL_STATUS_SUCCESS == ui32Status) )
    {
        return AM_DEVICES_MB85RS64V_STATUS_SUCCESS;
    }
    else
    {
        return AM_DEVICES_MB85RS64V_STATUS_ERROR;
    }
}

//*****************************************************************************
//
//  Reads the current status of the external flash
//
//*****************************************************************************
uint32_t
am_devices_mb85rs64v_mspi_status_get(void *pHandle, uint32_t *pStatus)
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
//  Sends a specific command to the device (blocking).
//
//*****************************************************************************
uint32_t
am_devices_mb85rs64v_mspi_command_send(void *pHandle, uint32_t ui32Cmd)
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
//  Programs the given range of flash addresses.
//
//*****************************************************************************
uint32_t
am_devices_mb85rs64v_mspi_blocking_write(void *pHandle, uint8_t *pui8TxBuffer,
                                         uint32_t ui32WriteAddress,
                                         uint32_t ui32NumBytes)
{
    am_hal_mspi_pio_transfer_t   Transaction;
    uint32_t                    Dummy;
    am_devices_mspi_fram_t *pFram = (am_devices_mspi_fram_t *)pHandle;

    //
    // Send the command to enable writing.
    //
    if (am_device_command_write(pHandle, 1, AM_DEVICES_MB85RS64V_WRITE_ENABLE, &Dummy, 0))
    {
        return AM_DEVICES_MB85RS64V_STATUS_ERROR;
    }

    //
    // Create the transaction.
    //
    Transaction.ui32NumBytes      = ui32NumBytes;
    Transaction.bScrambling       = false;
    Transaction.eDirection        = AM_HAL_MSPI_TX;
#if AM_CMSIS_REGS
    MSPIn(0)->CFG_b.ASIZE = AM_HAL_MSPI_ADDR_3_BYTE;
#else // AM_CMSIS_REGS
    AM_BFWn(MSPI, 0, CFG, ASIZE, AM_HAL_MSPI_ADDR_3_BYTE);
#endif // AM_CMSIS_REGS
    Transaction.bSendAddr         = true;
    Transaction.ui32DeviceAddr    = (AM_DEVICES_MB85RS64V_WRITE << 16) |
    (ui32WriteAddress & 0x0000FFFF);
    Transaction.bSendInstr        = false;
    Transaction.ui16DeviceInstr   = 0;
    Transaction.bTurnaround       = false;
#if 0 // A3DS-25 Deprecate MSPI CONT
    Transaction.bContinue         = false;
#endif // A3DS-25
    Transaction.bQuadCmd          = false;
    Transaction.pui32Buffer       = (uint32_t *)pui8TxBuffer;

    //
    // Execute the transction over IOM.
    //
    if (am_hal_mspi_blocking_transfer(pFram->pMspiHandle, &Transaction, MSPI_TRANSFER_TIMEOUT))
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
//  Programs the given range of flash addresses.
//
//*****************************************************************************
uint32_t
am_devices_mb85rs64v_mspi_write(void *pHandle, uint8_t *pui8TxBuffer,
                                uint32_t ui32WriteAddress,
                                uint32_t ui32NumBytes,
                                bool bWaitForCompletion)
{
    uint32_t      ui32Status;
    am_devices_mspi_fram_t *pFram = (am_devices_mspi_fram_t *)pHandle;

// ##### INTERNAL BEGIN #####
#if AM_APOLLO3_GPIO
    am_hal_gpio_state_write(0, AM_HAL_GPIO_OUTPUT_SET);
#else // AM_APOLLO3_GPIO
    am_hal_gpio_out_bit_set(0);
#endif // AM_APOLLO3_GPIO
// ##### INTERNAL END #####

    am_hal_mspi_dma_transfer_t  Transaction;

    //
    // Set up the MSPI transaction to write to the device.
    //
    Transaction.ui8Priority               = 1;        // High priority for now.
    Transaction.eDirection                = AM_HAL_MSPI_TX;
    Transaction.ui32TransferCount         = ui32NumBytes;
    Transaction.ui32DeviceAddress         = (AM_DEVICES_MB85RS64V_WRITE_ENABLE << 24) |
    (AM_DEVICES_MB85RS64V_WRITE << 16) |
      (ui32WriteAddress & 0x0000FFFF);
    Transaction.ui32SRAMAddress           = (uint32_t)pui8TxBuffer;
    Transaction.ui32PauseCondition        = 0;
    Transaction.ui32StatusSetClr          = 0;

    if (bWaitForCompletion)
    {
        // Start the transaction.
        volatile bool bDMAComplete = false;

        ui32Status = am_hal_mspi_nonblocking_transfer(pFram->pMspiHandle,
                                                      &Transaction,
                                                      AM_HAL_MSPI_TRANS_DMA, pfnMSPI_FRAM_Callback,
                                                      (void*)&bDMAComplete);

        //
        // Start the transaction (no callback).
        //
        if (AM_HAL_STATUS_SUCCESS != ui32Status)
        {
            return AM_DEVICES_MB85RS64V_STATUS_ERROR;
        }
        // Wait for DMA Complete or Timeout
        for (uint32_t i = 0; i < MSPI_TRANSFER_TIMEOUT; i++)
        {
            if (bDMAComplete)
            {
                break;
            }
            //
            // Call the BOOTROM cycle function to delay for about 1 microsecond.
            //
            am_hal_flash_delay( FLASH_CYCLES_US(1) );
        }

        // Check the status.
        if (!bDMAComplete)
        {
            return AM_DEVICES_MB85RS64V_STATUS_ERROR;
        }
    }
    else
    {
        ui32Status = am_hal_mspi_nonblocking_transfer(pFram->pMspiHandle,
                                                      &Transaction,
                                                      AM_HAL_MSPI_TRANS_DMA, NULL,
                                                      NULL);

        //
        // Start the transaction (no callback).
        //
        if (AM_HAL_STATUS_SUCCESS != ui32Status)
        {
            return AM_DEVICES_MB85RS64V_STATUS_ERROR;
        }
    }
    // ##### INTERNAL BEGIN #####
#if AM_APOLLO3_GPIO
    am_hal_gpio_state_write(0, AM_HAL_GPIO_OUTPUT_CLEAR);
#else // AM_APOLLO3_GPIO
    am_hal_gpio_out_bit_clear(0);
#endif // AM_APOLLO3_GPIO
    // ##### INTERNAL END #####
    //
    // Return the status.
    //
    return AM_DEVICES_MB85RS64V_STATUS_SUCCESS;
}

//*****************************************************************************
//
//  Reads the contents of the fram into a buffer.
//
//*****************************************************************************
uint32_t
am_devices_mb85rs64v_mspi_blocking_read(void *pHandle, uint8_t *pui8RxBuffer,
                                        uint32_t ui32ReadAddress,
                                        uint32_t ui32NumBytes)
{
    am_hal_mspi_pio_transfer_t Transaction;
    am_devices_mspi_fram_t *pFram = (am_devices_mspi_fram_t *)pHandle;

    //
    // Create the transaction.
    //
    Transaction.ui32NumBytes      = ui32NumBytes;
    Transaction.bScrambling       = false;
    Transaction.eDirection        = AM_HAL_MSPI_RX;
#if AM_CMSIS_REGS
    MSPIn(0)->CFG_b.ASIZE = AM_HAL_MSPI_ADDR_3_BYTE;
#else // AM_CMSIS_REGS
    AM_BFWn(MSPI, 0, CFG, ASIZE, AM_HAL_MSPI_ADDR_3_BYTE);
#endif // AM_CMSIS_REGS
    Transaction.bSendAddr       = true;
    Transaction.ui32DeviceAddr  = (AM_DEVICES_MB85RS64V_READ << 16) |
    (ui32ReadAddress & 0x0000FFFF);
    Transaction.bSendInstr        = false;
    Transaction.ui16DeviceInstr   = 0;
    Transaction.bTurnaround       = false;
#if 0 // A3DS-25 Deprecate MSPI CONT
    Transaction.bContinue         = false;
#endif // A3DS-25
    Transaction.bQuadCmd          = false;
    Transaction.pui32Buffer       = (uint32_t *)pui8RxBuffer;

    //
    // Execute the transction over IOM.
    //
    if (am_hal_mspi_blocking_transfer(pFram->pMspiHandle, &Transaction, MSPI_TRANSFER_TIMEOUT))
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
//
//*****************************************************************************
uint32_t
am_devices_mb85rs64v_mspi_read(void *pHandle, uint8_t *pui8RxBuffer,
                               uint32_t ui32ReadAddress,
                               uint32_t ui32NumBytes,
                               bool bWaitForCompletion)
{
    uint32_t      ui32Status;
    am_hal_mspi_dma_transfer_t      Transaction;
    am_devices_mspi_fram_t *pFram = (am_devices_mspi_fram_t *)pHandle;

// ##### INTERNAL BEGIN #####
#if AM_APOLLO3_GPIO
    am_hal_gpio_state_write(0, AM_HAL_GPIO_OUTPUT_SET);
#else // AM_APOLLO3_GPIO
    am_hal_gpio_out_bit_set(0);
#endif // AM_APOLLO3_GPIO
// ##### INTERNAL END #####

    //
    // Set up the IOM transaction to write to the device.
    //
    Transaction.ui8Priority               = 1;        // High priority for now.
    Transaction.eDirection                = AM_HAL_MSPI_RX;
    Transaction.ui32TransferCount         = ui32NumBytes;
    Transaction.ui32DeviceAddress         = (AM_DEVICES_MB85RS64V_WRITE_DISABLE << 24) |
                                            (AM_DEVICES_MB85RS64V_READ << 16) |
                                            (ui32ReadAddress & 0x0000FFFF);
    Transaction.ui32SRAMAddress           = (uint32_t)pui8RxBuffer;
    Transaction.ui32PauseCondition        = 0;
    Transaction.ui32StatusSetClr          = 0;

    if (bWaitForCompletion)
    {
        // Start the transaction.
        volatile bool bDMAComplete = false;

        //
        // Start the transaction (no callback).
        //
        ui32Status = am_hal_mspi_nonblocking_transfer(pFram->pMspiHandle, &Transaction,
                                             AM_HAL_MSPI_TRANS_DMA, pfnMSPI_FRAM_Callback, (void*)&bDMAComplete);
        if (AM_HAL_STATUS_SUCCESS != ui32Status)
        {
            return AM_DEVICES_MB85RS64V_STATUS_ERROR;
        }
        // Wait for DMA Complete or Timeout
        for (uint32_t i = 0; i < MSPI_TRANSFER_TIMEOUT; i++)
        {
            if (bDMAComplete)
            {
                break;
            }
            //
            // Call the BOOTROM cycle function to delay for about 1 microsecond.
            //
            am_hal_flash_delay( FLASH_CYCLES_US(1) );
        }

        // Check the status.
        if (!bDMAComplete)
        {
            return AM_DEVICES_MB85RS64V_STATUS_ERROR;
        }
    }
    else
    {
        ui32Status = am_hal_mspi_nonblocking_transfer(pFram->pMspiHandle,
                                                      &Transaction,
                                                      AM_HAL_MSPI_TRANS_DMA, NULL,
                                                      NULL);

        //
        // Start the transaction (no callback).
        //
        if (AM_HAL_STATUS_SUCCESS != ui32Status)
        {
            return AM_DEVICES_MB85RS64V_STATUS_ERROR;
        }
    }

// ##### INTERNAL BEGIN #####
#if AM_APOLLO3_GPIO
    am_hal_gpio_state_write(0, AM_HAL_GPIO_OUTPUT_CLEAR);
#else // AM_APOLLO3_GPIO
    am_hal_gpio_out_bit_clear(0);
#endif // AM_APOLLO3_GPIO
// ##### INTERNAL END #####

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

