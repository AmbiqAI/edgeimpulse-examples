//*****************************************************************************
//
//! @file am_devices_mspi_psram_aps6408l.c
//!
//! @brief Micron Serial SPI PSRAM driver.
//!
//! @addtogroup mspi_psram_aps64048l APS6408L MSPI PSRAM Driver
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
#include "am_devices_mspi_psram_aps6408l.h"
#include "am_util_stdio.h"
#include "am_bsp.h"
#include "am_util_delay.h"

//*****************************************************************************
//
// Global variables.
//
//*****************************************************************************
#define APS6408L_TIMEOUT             1000000
// #### INTERNAL BEGIN ####
// TODO-FIXME - What is clock base?  24MHz or 48MHz?
// #### INTERNAL END ####
#define APS6408L_MR_WRLAT       1
#define APS6408L_MR_RDLAT       3
#define APS6408L_OP_WRLAT       3
#define APS6408L_OP_RDLAT       3
    // Variable Latency; LC=3/LCx2=6; 50ohm drive strength
//#define APS6408L_MR0_VAL        0x01
    // Fix Latency; LC=3/LCx2=6; 50ohm drive strength
#define APS6408L_MR0_VAL        0x21
    // WLC=3; Fast Refresh; Refresh full array
#define APS6408L_MR4_VAL        0x00

#define AM_DEVICES_MSPI_PSRAM_APS6408L_DEVICE_ID  0x00009300

am_hal_mspi_dev_config_t        g_psMSPISettings;

void                            *g_pMSPIHandle;

volatile bool                   g_MSPIDMAComplete;

am_hal_mspi_pio_transfer_t      g_PIOTransaction;
uint32_t                        g_PIOBuffer[32];

volatile uint32_t               g_MSPIInterruptStatus;

am_hal_mspi_dev_config_t  APS6408ModeRegisterConfig =
{
  .ui8Turnaround        = APS6408L_MR_RDLAT,
  .eAddrCfg             = AM_HAL_MSPI_ADDR_4_BYTE,
  .eInstrCfg            = AM_HAL_MSPI_INSTR_1_BYTE,
  .ui8ReadInstr         = AM_DEVICES_MSPI_PSRAM_APS6408L_MODE_REG_READ,
  .ui8WriteInstr        = AM_DEVICES_MSPI_PSRAM_APS6408L_MODE_REG_WRITE,
  .eDeviceConfig        = AM_HAL_MSPI_FLASH_OCTAL_CE0,
  .ui8WriteLatency      = APS6408L_MR_WRLAT,
  .eSpiMode             = AM_HAL_MSPI_SPI_MODE_0,
// #### INTERNAL BEGIN ####
// TODO - FIXME - eClockFreq = 24MHz or 48MHz?
// #### INTERNAL END ####
  .eClockFreq           = AM_HAL_MSPI_CLK_48MHZ,
  .bEnWriteLatency      = true,
  .bSendAddr            = true,
  .bSendInstr           = true,
  .bSeparateIO          = false,
  .bTurnaround          = true,
  .bEmulateDDR          = true,
// #### INTERNAL BEGIN ####
// TODO - FIXME - Is this correct? (ui16DMATimeLimit = 40?)
// TODO - FIXME - Is this correct? (eDMABoundary = AM_HAL_MSPI_BOUNDARY_BREAK128?)
// #### INTERNAL END ####
  .ui16DMATimeLimit     = 40,
  .eDMABoundary         = AM_HAL_MSPI_BOUNDARY_BREAK128,
  .ui32TCBSize          = 0,
  .pTCB                 = NULL,
  .scramblingStartAddr  = 0,
  .scramblingEndAddr    = 0,
};

am_hal_mspi_dev_config_t  APS6408OperationConfig =
{
  .ui8Turnaround        = APS6408L_OP_RDLAT,
  .eAddrCfg             = AM_HAL_MSPI_ADDR_4_BYTE,
  .eInstrCfg            = AM_HAL_MSPI_INSTR_1_BYTE,
  .ui8ReadInstr         = AM_DEVICES_MSPI_PSRAM_APS6408L_SYNC_READ_LB,
  .ui8WriteInstr        = AM_DEVICES_MSPI_PSRAM_APS6408L_SYNC_WRITE_LB,
  .eDeviceConfig        = AM_HAL_MSPI_FLASH_OCTAL_CE0,
  .ui8WriteLatency      = APS6408L_OP_WRLAT,
  .eSpiMode             = AM_HAL_MSPI_SPI_MODE_0,
// #### INTERNAL BEGIN ####
// TODO - FIXME - eClockFreq = 24MHz or 48MHz?
// #### INTERNAL END ####
  .eClockFreq           = AM_HAL_MSPI_CLK_48MHZ,
  .bSendAddr            = true,
  .bSendInstr           = true,
  .bSeparateIO          = false,
  .bTurnaround          = true,
  .bEmulateDDR          = true,
// #### INTERNAL BEGIN ####
// TODO - FIXME - Is this correct? (ui16DMATimeLimit = 40?)
// TODO - FIXME - Is this correct? (eDMABoundary = AM_HAL_MSPI_BOUNDARY_BREAK128?)
// TODO - FIXME - Need to configure TCB for CQ. (ui32TCBSize = 0?)
// #### INTERNAL END ####
  .ui16DMATimeLimit     = 40,
  .eDMABoundary         = AM_HAL_MSPI_BOUNDARY_BREAK128,
  .ui32TCBSize          = 0,
  .pTCB                 = NULL,
  .scramblingStartAddr  = 0,
  .scramblingEndAddr    = 0,
};

//*****************************************************************************
//
//  MSPI Interrupt Service Routine.
//
//*****************************************************************************
void
am_mspi1_isr(void)
{
  uint32_t      ui32Status;

  am_hal_mspi_interrupt_status_get(g_pMSPIHandle, &ui32Status, false);

  am_hal_mspi_interrupt_clear(g_pMSPIHandle, ui32Status);

  am_hal_mspi_interrupt_service(g_pMSPIHandle, ui32Status);

  g_MSPIInterruptStatus &= ~ui32Status;
}

void pfnMSPI_PSRAM_Callback(void *pCallbackCtxt, uint32_t status)
{
  // Set the DMA complete flag.
  g_MSPIDMAComplete = true;
}

//*****************************************************************************
//
// Reset the PSRAM
//
//*****************************************************************************
uint32_t
am_devices_mspi_psram_aps6408l_reset(void)
{
  am_hal_mspi_pio_transfer_t    Transaction;
  uint32_t                      ui32Status;

  // Need to make sure this is at least 150usec after POR.
  am_util_delay_us(150);

  // Create the individual write transaction.
  Transaction.ui32NumBytes      = 0;
  Transaction.bScrambling       = false;
// #### INTERNAL BEGIN ####
// TODO-FIXME - Is this correct??? (Transaction.bDCX = false?)
// TODO-FIXME - Is this correct??? (Transaction.bEnWRLatency = true?)
// #### INTERNAL END ####
  Transaction.bDCX              = false;
  Transaction.eDirection        = AM_HAL_MSPI_TX;
  Transaction.bSendAddr         = false;
  Transaction.ui32DeviceAddr    = 0;
  Transaction.bSendInstr        = true;
  Transaction.ui16DeviceInstr   = AM_DEVICES_MSPI_PSRAM_APS6408L_GLOBAL_RESET;
  Transaction.bTurnaround       = false;
  Transaction.bEnWRLatency      = true;
  Transaction.bQuadCmd          = false;
  Transaction.bContinue         = false;
  Transaction.pui32Buffer       = 0;

  // Execute the transction over MSPI.
  ui32Status =  am_hal_mspi_blocking_transfer(g_pMSPIHandle,
                                              &Transaction,
                                              APS6408L_TIMEOUT);

  // Need to wait 2usec after issuing the command for reset to complete
  am_util_delay_us(2);

  return (AM_HAL_STATUS_SUCCESS == ui32Status) ? AM_DEVICES_MSPI_PSRAM_STATUS_SUCCESS : AM_DEVICES_MSPI_PSRAM_STATUS_ERROR;

}

//*****************************************************************************
//
//  Reads the ID of the external psram and returns the value.
//
//*****************************************************************************
uint32_t
am_devices_mspi_psram_aps6408l_id(void)
{
  am_hal_mspi_pio_transfer_t    Transaction;
  uint32_t                      ui32Status;
  uint32_t      ui32DeviceID = 0;

  // Create the individual read transaction.
  Transaction.ui32NumBytes      = 2;
  Transaction.bScrambling       = false;
// #### INTERNAL BEGIN ####
// TODO-FIXME - Is this correct??? (Transaction.bDCX = false?)
// TODO-FIXME - Is this correct??? (Transaction.bEnWRLatency = true?)
// #### INTERNAL END ####
  Transaction.bDCX              = false;
  Transaction.eDirection        = AM_HAL_MSPI_RX;
  Transaction.bSendAddr         = false;
  Transaction.ui32DeviceAddr    = 2;                    // Reads MR2 & MR3
  Transaction.bSendInstr        = true;
  Transaction.ui16DeviceInstr   = AM_DEVICES_MSPI_PSRAM_APS6408L_MODE_REG_READ;
  Transaction.bTurnaround       = false;
  Transaction.bEnWRLatency      = true;
  Transaction.bQuadCmd          = false;
  Transaction.bContinue         = false;
  Transaction.pui32Buffer       = &ui32DeviceID;

  // Execute the transction over MSPI.
  ui32Status =  am_hal_mspi_blocking_transfer(g_pMSPIHandle,
                                              &Transaction,
                                              APS6408L_TIMEOUT);

  am_util_stdio_printf("PSRAM ID is 0x%x\n", ui32DeviceID);
  if ((AM_DEVICES_MSPI_PSRAM_APS6408L_DEVICE_ID == ui32DeviceID) &&
      (AM_HAL_STATUS_SUCCESS == ui32Status))
  {
    return AM_DEVICES_MSPI_PSRAM_STATUS_SUCCESS;
  }
  else
  {
    return AM_DEVICES_MSPI_PSRAM_STATUS_ERROR;
  }
}

//*****************************************************************************
//
// Initialize the mspi_psram driver.
//
//*****************************************************************************
uint32_t
am_devices_mspi_psram_aps6408l_init(uint32_t ui32Module,
                                    am_hal_mspi_dev_config_t *psMSPISettings,
                                    void **pHandle)
{
  uint32_t                      ui32Status;
  uint32_t                      ui32MR0Val = APS6408L_MR0_VAL;
  uint32_t                      ui32MR4Val = APS6408L_MR4_VAL;
  am_hal_mspi_pio_transfer_t    Transaction;

  //
  // Enable fault detection.
  //
  am_hal_mcuctrl_control(AM_HAL_MCUCTRL_CONTROL_FAULT_CAPTURE_ENABLE, 0);

  // Initialize the MSPI interface
  if (AM_HAL_STATUS_SUCCESS != am_hal_mspi_initialize(ui32Module, &g_pMSPIHandle))
  {
    am_util_stdio_printf("Error - Failed to initialize MSPI.\n");
    return AM_DEVICES_MSPI_PSRAM_STATUS_ERROR;
  }

  if (AM_HAL_STATUS_SUCCESS != am_hal_mspi_power_control(g_pMSPIHandle, AM_HAL_SYSCTRL_WAKE, false))
  {
    am_util_stdio_printf("Error - Failed to power on MSPI.\n");
    return AM_DEVICES_MSPI_PSRAM_STATUS_ERROR;
  }

  if (AM_HAL_STATUS_SUCCESS != am_hal_mspi_device_configure(g_pMSPIHandle, &APS6408ModeRegisterConfig))
  {
    am_util_stdio_printf("Error - Failed to configure MSPI.\n");
    return AM_DEVICES_MSPI_PSRAM_STATUS_ERROR;
  }
  if (AM_HAL_STATUS_SUCCESS != am_hal_mspi_enable(g_pMSPIHandle))
  {
    am_util_stdio_printf("Error - Failed to enable MSPI.\n");
    return AM_DEVICES_MSPI_PSRAM_STATUS_ERROR;
  }
  am_bsp_mspi_pins_enable(ui32Module, APS6408ModeRegisterConfig.eDeviceConfig);

  if (AM_HAL_STATUS_SUCCESS != am_devices_mspi_psram_aps6408l_reset())
  {
    return AM_DEVICES_MSPI_PSRAM_STATUS_ERROR;
  }
  //
  // Set the Read Latency in MR0.
  //
  Transaction.ui32NumBytes      = 0;
  Transaction.bScrambling       = false;
// #### INTERNAL BEGIN ####
// TODO-FIXME - Is this correct??? (Transaction.bDCX = false?)
// TODO-FIXME - Is this correct??? (Transaction.bEnWRLatency = true?)
// #### INTERNAL END ####
  Transaction.bDCX              = false;
  Transaction.eDirection        = AM_HAL_MSPI_TX;
  Transaction.bSendAddr         = true;
  Transaction.ui32DeviceAddr    = 0;    // MR0
  Transaction.bSendInstr        = true;
  Transaction.ui16DeviceInstr   = AM_DEVICES_MSPI_PSRAM_APS6408L_MODE_REG_WRITE;
  Transaction.bTurnaround       = false;
  Transaction.bEnWRLatency      = true;
  Transaction.bQuadCmd          = false;
  Transaction.bContinue         = false;
  Transaction.pui32Buffer       = &ui32MR0Val;

  // Execute the transction over MSPI.
  ui32Status =  am_hal_mspi_blocking_transfer(g_pMSPIHandle,
                                              &Transaction,
                                              APS6408L_TIMEOUT);

  //
  // Set the Write Latency in MR4.
  //
  Transaction.ui32NumBytes      = 0;
  Transaction.bScrambling       = false;
// #### INTERNAL BEGIN ####
// TODO-FIXME - Is this correct??? (Transaction.bDCX = false?)
// TODO-FIXME - Is this correct??? (Transaction.bEnWRLatency = true?)
// #### INTERNAL END ####
  Transaction.bDCX              = false;
  Transaction.eDirection        = AM_HAL_MSPI_TX;
  Transaction.bSendAddr         = true;
  Transaction.ui32DeviceAddr    = 4;    // MR4
  Transaction.bSendInstr        = true;
  Transaction.ui16DeviceInstr   = AM_DEVICES_MSPI_PSRAM_APS6408L_MODE_REG_WRITE;
  Transaction.bTurnaround       = false;
  Transaction.bEnWRLatency      = true;
  Transaction.bQuadCmd          = false;
  Transaction.bContinue         = false;
  Transaction.pui32Buffer       = &ui32MR4Val;

  // Execute the transction over MSPI.
  ui32Status =  am_hal_mspi_blocking_transfer(g_pMSPIHandle,
                                              &Transaction,
                                              APS6408L_TIMEOUT);

  if (AM_HAL_STATUS_SUCCESS != am_hal_mspi_device_configure(g_pMSPIHandle, &APS6408OperationConfig))
  {
    am_util_stdio_printf("Error - Failed to configure MSPI.\n");
    return AM_DEVICES_MSPI_PSRAM_STATUS_ERROR;
  }
  if (AM_HAL_STATUS_SUCCESS != am_hal_mspi_enable(g_pMSPIHandle))
  {
    am_util_stdio_printf("Error - Failed to enable MSPI.\n");
    return AM_DEVICES_MSPI_PSRAM_STATUS_ERROR;
  }
  am_bsp_mspi_pins_enable(ui32Module, APS6408OperationConfig.eDeviceConfig);

  //
  // Store the MSPI settings for the MSPI_PSRAM.
  //
  g_psMSPISettings = APS6408OperationConfig;

  //
  // Enable MSPI interrupts.
  //
  ui32Status = am_hal_mspi_interrupt_clear(g_pMSPIHandle, AM_HAL_MSPI_INT_CQUPD | AM_HAL_MSPI_INT_ERR );
  if (AM_HAL_STATUS_SUCCESS != ui32Status)
  {
    return AM_DEVICES_MSPI_PSRAM_STATUS_ERROR;
  }

  ui32Status = am_hal_mspi_interrupt_enable(g_pMSPIHandle, AM_HAL_MSPI_INT_CQUPD | AM_HAL_MSPI_INT_ERR );
  if (AM_HAL_STATUS_SUCCESS != ui32Status)
  {
    return AM_DEVICES_MSPI_PSRAM_STATUS_ERROR;
  }
#ifdef AM_IRQ_PRIORITY_DEFAULT
  NVIC_SetPriority(MSPI0_IRQn, AM_IRQ_PRIORITY_DEFAULT);
#endif // AM_IRQ_PRIORITY_DEFAULT
// #### INTERNAL BEGIN ####
// TODO - FIXME - need to make the IRQ module-specific.
// #### INTERNAL END ####
  NVIC_EnableIRQ(MSPI0_IRQn);
  am_hal_interrupt_master_enable();

  //
  // Return the handle.
  //
  *pHandle = g_pMSPIHandle;

  //
  // Return the status.
  //
  return AM_DEVICES_MSPI_PSRAM_STATUS_SUCCESS;
}

//*****************************************************************************
//
//! @brief DeInitialize the mspi_psram driver.
//!
//! @param psMSPISettings - MSPI device structure describing the target spi psram.
//!
//! @return status.
//
//*****************************************************************************
uint32_t
am_devices_mspi_psram_deinit(am_hal_mspi_dev_config_t *psMSPISettings)
{
  uint32_t      ui32Status;

  //
  // Disable the master interrupt on NVIC
  //
  am_hal_interrupt_master_disable();

  //
  // Disable and clear the interrupts to start with.
  //
  ui32Status = am_hal_mspi_interrupt_disable(g_pMSPIHandle, 0xFFFFFFFF);
  if (AM_HAL_STATUS_SUCCESS != ui32Status)
  {
    return AM_DEVICES_MSPI_PSRAM_STATUS_ERROR;
  }
  ui32Status = am_hal_mspi_interrupt_clear(g_pMSPIHandle, 0xFFFFFFFF);
  if (AM_HAL_STATUS_SUCCESS != ui32Status)
  {
    return AM_DEVICES_MSPI_PSRAM_STATUS_ERROR;
  }

  //
  // Disable the MSPI interrupts.
  //
// #### INTERNAL BEGIN ####
// TODO - FIXME - need to make the IRQ module-specific.
// #### INTERNAL END ####
  NVIC_DisableIRQ(MSPI0_IRQn);

  //
  // Disable MSPI instance.
  //
  ui32Status = am_hal_mspi_disable(g_pMSPIHandle);
  if (AM_HAL_STATUS_SUCCESS != ui32Status)
  {
    return AM_DEVICES_MSPI_PSRAM_STATUS_ERROR;
  }
  //
  // Disable power to the MSPI instance.
  //
  if (AM_HAL_STATUS_SUCCESS != am_hal_mspi_power_control(g_pMSPIHandle, AM_HAL_SYSCTRL_DEEPSLEEP, false))
  {
    am_util_stdio_printf("Error - Failed to power on MSPI.\n");
    return AM_DEVICES_MSPI_PSRAM_STATUS_ERROR;
  }
  //
  // Deinitialize the MPSI instance.
  //
  ui32Status = am_hal_mspi_deinitialize(g_pMSPIHandle);
  if (AM_HAL_STATUS_SUCCESS != ui32Status)
  {
    return AM_DEVICES_MSPI_PSRAM_STATUS_ERROR;
  }

  //
  // Return the status.
  //
  return AM_DEVICES_MSPI_PSRAM_STATUS_SUCCESS;
}

#if 0  // Shouldn't be required for Apollo3 Blue Plus as it is handled in HW.
// This function takes care of splitting the transaction as needed, if the transaction crosses
// PSRAM page boundary
static uint32_t psram_nonblocking_transfer(bool bHiPrio, bool bWrite,
                                           uint8_t *pui8Buffer,
                                           uint32_t ui32Address,
                                           uint32_t ui32NumBytes,
                                           uint32_t ui32PauseCondition,
                                           uint32_t ui32StatusSetClr,
                                           am_hal_mspi_callback_t pfnCallback,
                                           void *pCallbackCtxt)
{
  uint32_t ui32Status = AM_DEVICES_MSPI_PSRAM_STATUS_SUCCESS;
  am_hal_mspi_dma_transfer_t    Transaction;

  // Set the DMA priority
  Transaction.ui8Priority = 1;

  // Set the address configuration for the read
  Transaction.eAddrCfg = AM_HAL_MSPI_ADDR_3_BYTE;

  // Set the transfer direction to RX (Read)
  Transaction.eDirection = bWrite ? AM_HAL_MSPI_TX: AM_HAL_MSPI_RX;

  // Initialize the CQ stimulus.
  Transaction.ui32PauseCondition = ui32PauseCondition;
  // Initialize the post-processing
  Transaction.ui32StatusSetClr = 0;

  // Need to be aware of page size
  while (ui32NumBytes)
  {
    uint32_t maxSize = AM_DEVICES_MPSI_PSRAM_APS6408L_PAGE_SIZE - (ui32Address & (AM_DEVICES_MPSI_PSRAM_APS6408L_PAGE_SIZE - 1));
    uint32_t size = (ui32NumBytes > maxSize) ? maxSize : ui32NumBytes;
    bool bLast = (size == ui32NumBytes);

    // Set the transfer count in bytes.
    Transaction.ui32TransferCount = size;

    // Set the address to read data from.
    Transaction.ui32DeviceAddress = ui32Address;

    // Set the target SRAM buffer address.
    Transaction.ui32SRAMAddress = (uint32_t)pui8Buffer;

    if (bLast)
    {
      Transaction.ui32StatusSetClr = ui32StatusSetClr;
    }
    else
    {
      Transaction.ui32PauseCondition = 0;
    }

    if (bHiPrio)
    {
      ui32Status = am_hal_mspi_highprio_transfer(g_pMSPIHandle, &Transaction, AM_HAL_MSPI_TRANS_DMA,
                                                 bLast ? pfnCallback : NULL,
                                                 bLast ? pCallbackCtxt : NULL);
    }
    else
    {
      ui32Status = am_hal_mspi_nonblocking_transfer(g_pMSPIHandle, &Transaction, AM_HAL_MSPI_TRANS_DMA,
                                                    bLast ? pfnCallback : NULL,
                                                    bLast ? pCallbackCtxt : NULL);
    }
    if (AM_HAL_STATUS_SUCCESS != ui32Status)
    {
      break;
    }
    ui32Address += size;
    ui32NumBytes -= size;
    pui8Buffer += size;
  }
  return ui32Status;
}
#endif

//*****************************************************************************
//
//! @brief Reads the contents of the external psram into a buffer.
//!
//! @param pui8RxBuffer - Buffer to store the received data from the psram
//! @param ui32ReadAddress - Address of desired data in external psram
//! @param ui32NumBytes - Number of bytes to read from external psram
//! @param ui32PauseCondition
//! @param ui32StatusSetClr
//! @param pfnCallback
//! @param pCallbackCtxt
//!
//! This function reads the external psram at the provided address and stores
//! the received data into the provided buffer location. This function will
//! only store ui32NumBytes worth of data.
//
//! @return 32-bit status
//
//*****************************************************************************
uint32_t
am_devices_mspi_psram_read_adv(uint8_t *pui8RxBuffer,
                               uint32_t ui32ReadAddress,
                               uint32_t ui32NumBytes,
                               uint32_t ui32PauseCondition,
                               uint32_t ui32StatusSetClr,
                               am_hal_mspi_callback_t pfnCallback,
                               void *pCallbackCtxt)
{
  uint32_t                      ui32Status;

  ui32Status = psram_nonblocking_transfer(false, false,
                                          pui8RxBuffer,
                                          ui32ReadAddress,
                                          ui32NumBytes,
                                          ui32PauseCondition,
                                          ui32StatusSetClr,
                                          pfnCallback,
                                          pCallbackCtxt);

  // Check the transaction status.
  if (AM_HAL_STATUS_SUCCESS != ui32Status)
  {
    return AM_DEVICES_MSPI_PSRAM_STATUS_ERROR;
  }

  //
  // Return the status.
  //
  return AM_DEVICES_MSPI_PSRAM_STATUS_SUCCESS;
}

//*****************************************************************************
//
//! @brief Writes the contents of the external psram into a buffer.
//!
//! @param pui8TxBuffer - Buffer to store the transfer data to the psram
//! @param ui32WriteAddress - Address of desired data in external psram
//! @param ui32NumBytes - Number of bytes to read from external psram
//! @param ui32PauseCondition
//! @param ui32StatusSetClr
//! @param pfnCallback
//! @param pCallbackCtxt
//!
//! This function Writes the external psram at the provided address.
//! This function will only transfer ui32NumBytes worth of data.
//
//! @return 32-bit status
//
//*****************************************************************************

uint32_t
am_devices_mspi_psram_write_adv(uint8_t *pui8TxBuffer,
                                uint32_t ui32WriteAddress,
                                uint32_t ui32NumBytes,
                                uint32_t ui32PauseCondition,
                                uint32_t ui32StatusSetClr,
                                am_hal_mspi_callback_t pfnCallback,
                                void *pCallbackCtxt)
{
  uint32_t                      ui32Status;

  ui32Status = psram_nonblocking_transfer(false, true,
                                          pui8TxBuffer,
                                          ui32WriteAddress,
                                          ui32NumBytes,
                                          ui32PauseCondition,
                                          ui32StatusSetClr,
                                          pfnCallback,
                                          pCallbackCtxt);

  // Check the transaction status.
  if (AM_HAL_STATUS_SUCCESS != ui32Status)
  {
    return AM_DEVICES_MSPI_PSRAM_STATUS_ERROR;
  }

  //
  // Return the status.
  //
  return AM_DEVICES_MSPI_PSRAM_STATUS_SUCCESS;
}

//*****************************************************************************
//
//! @brief Reads the contents of the external psram into a buffer.
//!
//! @param pui8RxBuffer - Buffer to store the received data from the psram
//! @param ui32ReadAddress - Address of desired data in external psram
//! @param ui32NumBytes - Number of bytes to read from external psram
//! @param bWaitForCompletion - blocking when true
//!
//! This function reads the external psram at the provided address and stores
//! the received data into the provided buffer location. This function will
//! only store ui32NumBytes worth of data.
//
//! @return 32-bit status
//
//*****************************************************************************
uint32_t
am_devices_mspi_psram_read(uint8_t *pui8RxBuffer,
                           uint32_t ui32ReadAddress,
                           uint32_t ui32NumBytes,
                           bool bWaitForCompletion)
{
  uint32_t                      ui32Status;

  if (bWaitForCompletion)
  {
    // Start the transaction.
    g_MSPIDMAComplete = false;
    ui32Status = psram_nonblocking_transfer(false, false,
                                            pui8RxBuffer,
                                            ui32ReadAddress,
                                            ui32NumBytes,
                                            0,
                                            0,
                                            pfnMSPI_PSRAM_Callback,
                                            NULL);

    // Check the transaction status.
    if (AM_HAL_STATUS_SUCCESS != ui32Status)
    {
      return AM_DEVICES_MSPI_PSRAM_STATUS_ERROR;
    }

    // Wait for DMA Complete or Timeout
    for (uint32_t i = 0; i < APS6408L_TIMEOUT; i++)
    {
      if (g_MSPIDMAComplete)
      {
        break;
      }
      //
      // Call the BOOTROM cycle function to delay for about 1 microsecond.
      //
      am_hal_flash_delay( FLASH_CYCLES_US(1) );
    }

    // Check the status.
    if (!g_MSPIDMAComplete)
    {
      return AM_DEVICES_MSPI_PSRAM_STATUS_ERROR;
    }
  }
  else
  {
    // Check the transaction status.
    ui32Status = psram_nonblocking_transfer(false, false,
                                            pui8RxBuffer,
                                            ui32ReadAddress,
                                            ui32NumBytes,
                                            0,
                                            0,
                                            pfnMSPI_PSRAM_Callback,
                                            NULL);
    if (AM_HAL_STATUS_SUCCESS != ui32Status)
    {
      return AM_DEVICES_MSPI_PSRAM_STATUS_ERROR;
    }
  }

  //
  // Return the status.
  //
  return AM_DEVICES_MSPI_PSRAM_STATUS_SUCCESS;
}

//*****************************************************************************
//
//! @brief Reads the contents of the external psram into a buffer.
//!
//! @param pui8RxBuffer - Buffer to store the received data from the psram
//! @param ui32ReadAddress - Address of desired data in external psram
//! @param ui32NumBytes - Number of bytes to read from external psram
//! @param pfnCallback
//! @param pCallbackCtxt
//!
//! This function reads the external psram at the provided address and stores
//! the received data into the provided buffer location. This function will
//! only store ui32NumBytes worth of data.
//!
//! @return 32-bit status
//
//*****************************************************************************
uint32_t
am_devices_mspi_psram_read_hiprio(uint8_t *pui8RxBuffer,
                                  uint32_t ui32ReadAddress,
                                  uint32_t ui32NumBytes,
                                  am_hal_mspi_callback_t pfnCallback,
                                  void *pCallbackCtxt)
{
  uint32_t                      ui32Status;

  ui32Status = psram_nonblocking_transfer(true, false,
                                          pui8RxBuffer,
                                          ui32ReadAddress,
                                          ui32NumBytes,
                                          0,
                                          0,
                                          pfnCallback,
                                          pCallbackCtxt);

  // Check the transaction status.
  if (AM_HAL_STATUS_SUCCESS != ui32Status)
  {
    return AM_DEVICES_MSPI_PSRAM_STATUS_ERROR;
  }

  //
  // Return the status.
  //
  return AM_DEVICES_MSPI_PSRAM_STATUS_SUCCESS;
}

//*****************************************************************************
//
//! @brief Programs the given range of psram addresses.
//!
//! @param pui8TxBuffer - Buffer to write the external psram data from
//! @param ui32WriteAddress - Address to write to in the external psram
//! @param ui32NumBytes - Number of bytes to write to the external psram
//! @param bWaitForCompletion - Blocking when true
//!
//! This function uses the data in the provided pui8TxBuffer and copies it to
//! the external psram at the address given by ui32WriteAddress. It will copy
//! exactly ui32NumBytes of data from the original pui8TxBuffer pointer. The
//! user is responsible for ensuring that they do not overflow the target psram
//! memory or underflow the pui8TxBuffer array
//!
//! @return 32-bit status
//
//*****************************************************************************
uint32_t
am_devices_mspi_psram_write(uint8_t *pui8TxBuffer,
                            uint32_t ui32WriteAddress,
                            uint32_t ui32NumBytes,
                            bool bWaitForCompletion)
{
  uint32_t                      ui32Status;

  if (bWaitForCompletion)
  {
    // Start the transaction.
    g_MSPIDMAComplete = false;
    ui32Status = psram_nonblocking_transfer(false, true,
                                            pui8TxBuffer,
                                            ui32WriteAddress,
                                            ui32NumBytes,
                                            0,
                                            0,
                                            pfnMSPI_PSRAM_Callback,
                                            NULL);

    // Check the transaction status.
    if (AM_HAL_STATUS_SUCCESS != ui32Status)
    {
      return AM_DEVICES_MSPI_PSRAM_STATUS_ERROR;
    }

    // Wait for DMA Complete or Timeout
    for (uint32_t i = 0; i < APS6408L_TIMEOUT; i++)
    {
      if (g_MSPIDMAComplete)
      {
        break;
      }
      //
      // Call the BOOTROM cycle function to delay for about 1 microsecond.
      //
      am_hal_flash_delay( FLASH_CYCLES_US(1) );
    }

    // Check the status.
    if (!g_MSPIDMAComplete)
    {
      return AM_DEVICES_MSPI_PSRAM_STATUS_ERROR;
    }
  }
  else
  {
    // Check the transaction status.
    ui32Status = psram_nonblocking_transfer(false, true,
                                            pui8TxBuffer,
                                            ui32WriteAddress,
                                            ui32NumBytes,
                                            0,
                                            0,
                                            NULL,
                                            NULL);
    if (AM_HAL_STATUS_SUCCESS != ui32Status)
    {
      return AM_DEVICES_MSPI_PSRAM_STATUS_ERROR;
    }
  }

  //
  // Return the status.
  //
  return AM_DEVICES_MSPI_PSRAM_STATUS_SUCCESS;
}

uint32_t
am_devices_mspi_psram_nonblocking_write(uint8_t *pui8TxBuffer,
                                        uint32_t ui32WriteAddress,
                                        uint32_t ui32NumBytes,
                                        am_hal_mspi_callback_t pfnCallback,
                                        void *pCallbackCtxt)
{
  uint32_t                      ui32Status;

  // Check the transaction status.
  ui32Status = psram_nonblocking_transfer(false, true,
                                          pui8TxBuffer,
                                          ui32WriteAddress,
                                          ui32NumBytes,
                                          0,
                                          0,
                                          pfnCallback,
                                          pCallbackCtxt);
  if (AM_HAL_STATUS_SUCCESS != ui32Status)
  {
    return AM_DEVICES_MSPI_PSRAM_STATUS_ERROR;
  }

  //
  // Return the status.
  //
  return AM_DEVICES_MSPI_PSRAM_STATUS_SUCCESS;
}

//*****************************************************************************
//
//! @brief
//! @param pui8RxBuffer
//! @param ui32ReadAddress
//! @param ui32NumBytes
//! @param pfnCallback
//! @param pCallbackCtxt
//! @return
//
//*****************************************************************************
uint32_t
am_devices_mspi_psram_nonblocking_read(uint8_t *pui8RxBuffer,
                                       uint32_t ui32ReadAddress,
                                       uint32_t ui32NumBytes,
                                       am_hal_mspi_callback_t pfnCallback,
                                       void *pCallbackCtxt)
{
  uint32_t                      ui32Status;

  // Check the transaction status.
  ui32Status = psram_nonblocking_transfer(false, false,
                                          pui8RxBuffer,
                                          ui32ReadAddress,
                                          ui32NumBytes,
                                          0,
                                          0,
                                          pfnCallback,
                                          pCallbackCtxt);
  if (AM_HAL_STATUS_SUCCESS != ui32Status)
  {
    return AM_DEVICES_MSPI_PSRAM_STATUS_ERROR;
  }

  //
  // Return the status.
  //
  return AM_DEVICES_MSPI_PSRAM_STATUS_SUCCESS;
}

//*****************************************************************************
//
//! @brief Programs the given range of psram addresses.
//!
//! @param pui8TxBuffer - Buffer to write the external psram data from
//! @param ui32WriteAddress - Address to write to in the external psram
//! @param ui32NumBytes - Number of bytes to write to the external psram
//! @param pfnCallback
//! @param pCallbackCtxt
//!
//! This function uses the data in the provided pui8TxBuffer and copies it to
//! the external psram at the address given by ui32WriteAddress. It will copy
//! exactly ui32NumBytes of data from the original pui8TxBuffer pointer. The
//! user is responsible for ensuring that they do not overflow the target psram
//! memory or underflow the pui8TxBuffer array
//!
//! @return 32-bit status
//
//*****************************************************************************
uint32_t
am_devices_mspi_psram_write_hiprio(uint8_t *pui8TxBuffer,
                                   uint32_t ui32WriteAddress,
                                   uint32_t ui32NumBytes,
                                   am_hal_mspi_callback_t pfnCallback,
                                   void *pCallbackCtxt)
{
  uint32_t                      ui32Status;

  // Check the transaction status.
  ui32Status = psram_nonblocking_transfer(true, true,
                                          pui8TxBuffer,
                                          ui32WriteAddress,
                                          ui32NumBytes,
                                          0,
                                          0,
                                          pfnCallback,
                                          pCallbackCtxt);

  // Check the transaction status.
  if (AM_HAL_STATUS_SUCCESS != ui32Status)
  {
    return AM_DEVICES_MSPI_PSRAM_STATUS_ERROR;
  }

  //
  // Return the status.
  //
  return AM_DEVICES_MSPI_PSRAM_STATUS_SUCCESS;
}

//*****************************************************************************
//
//! @brief Sets up the MSPI and external psram into XIP mode.
//!
//! This function sets the external psram device and the MSPI into XIP mode.
//!
//! @return 32-bit status
//
//*****************************************************************************
uint32_t
am_devices_mspi_psram_enable_xip(void)
{
  uint32_t ui32Status;

  //
  // Enable XIP on the MSPI.
  //
  ui32Status = am_hal_mspi_control(g_pMSPIHandle, AM_HAL_MSPI_REQ_XIP_EN, NULL);
  if (AM_HAL_STATUS_SUCCESS != ui32Status)
  {
    return AM_DEVICES_MSPI_PSRAM_STATUS_ERROR;
  }

#if !MSPI_USE_CQ
  // Disable the DMA interrupts.
  ui32Status = am_hal_mspi_interrupt_disable(g_pMSPIHandle,
                                             AM_HAL_MSPI_INT_DMAERR |
                                               AM_HAL_MSPI_INT_DMACMP );
  if (AM_HAL_STATUS_SUCCESS != ui32Status)
  {
    return AM_DEVICES_MSPI_PSRAM_STATUS_ERROR;
  }
#endif

  return AM_DEVICES_MSPI_PSRAM_STATUS_SUCCESS;
}

//*****************************************************************************
//
//! @brief Removes the MSPI and external psram from XIP mode.
//!
//! This function removes the external device and the MSPI from XIP mode.
//!
//! @return 32-bit status
//
//*****************************************************************************
uint32_t
am_devices_mspi_psram_disable_xip(void)
{
  uint32_t ui32Status;

  //
  // Disable XIP on the MSPI.
  //
  ui32Status = am_hal_mspi_control(g_pMSPIHandle, AM_HAL_MSPI_REQ_XIP_DIS, NULL);
  if (AM_HAL_STATUS_SUCCESS != ui32Status)
  {
    return AM_DEVICES_MSPI_PSRAM_STATUS_ERROR;
  }

  return AM_DEVICES_MSPI_PSRAM_STATUS_SUCCESS;
}

//*****************************************************************************
//
//! @brief Sets up the MSPI and external psram into scrambling mode.
//!
//! This function sets the external psram device and the MSPI into scrambling mode.
//
//! @return 32-bit status
//
//*****************************************************************************
uint32_t
am_devices_mspi_psram_enable_scrambling(void)
{
  uint32_t ui32Status;

  //
  // Enable scrambling on the MSPI.
  //
  ui32Status = am_hal_mspi_control(g_pMSPIHandle, AM_HAL_MSPI_REQ_SCRAMB_EN, NULL);
  if (AM_HAL_STATUS_SUCCESS != ui32Status)
  {
    return AM_DEVICES_MSPI_PSRAM_STATUS_ERROR;
  }
  return AM_DEVICES_MSPI_PSRAM_STATUS_SUCCESS;
}

//*****************************************************************************
//
//! @brief Removes the MSPI and external psram from scrambling mode.
//!
//! This function removes the external device and the MSPI from scrambling mode.
//!
//! @return 32-bit status
//
//*****************************************************************************
uint32_t
am_devices_mspi_psram_disable_scrambling(void)
{
  uint32_t ui32Status;

  //
  // Disable Scrambling on the MSPI.
  //
  ui32Status = am_hal_mspi_control(g_pMSPIHandle, AM_HAL_MSPI_REQ_SCRAMB_DIS, NULL);
  if (AM_HAL_STATUS_SUCCESS != ui32Status)
  {
    return AM_DEVICES_MSPI_PSRAM_STATUS_ERROR;
  }

  return AM_DEVICES_MSPI_PSRAM_STATUS_SUCCESS;
}

//*****************************************************************************
//
// End Doxygen group.
//! @}
//
//*****************************************************************************

