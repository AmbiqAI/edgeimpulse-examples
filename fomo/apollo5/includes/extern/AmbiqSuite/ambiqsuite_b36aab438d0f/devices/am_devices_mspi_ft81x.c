//*****************************************************************************
//
//! @file am_devices_mspi_ft81x.c
//!
//! @brief MSPI FT81X display driver.
//!
//! @addtogroup mspi_ft81x FT81X MSPI Display Driver
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

#include <string.h>
#include "am_mcu_apollo.h"
#include "am_devices_mspi_ft81x.h"
#include "am_bsp.h"
#include "am_util_delay.h"
#include "am_util.h"

//*****************************************************************************
//
// Defines
//
//*****************************************************************************
#define AM_DEVICES_MSPI_TIMEOUT         1000000
#define BYTE_NUM_PER_WRITE              AM_HAL_MSPI_MAX_TRANS_SIZE
//#define BYTE_NUM_PER_WRITE              128

//  Display Formats
#define ARGB1555        0
#define ARGB4           6
#define RGB565          7

//*****************************************************************************
//
// Global variables.
//
//*****************************************************************************
//
//!
//
am_hal_mspi_dev_config_t  MSPI_FT81X_SerialCE0MSPIConfig =
{
  .ui8TurnAround        = 8,
  .eAddrCfg             = AM_HAL_MSPI_ADDR_3_BYTE,
  .eInstrCfg            = AM_HAL_MSPI_INSTR_1_BYTE,
  .ui8ReadInstr         = 0,
  .ui8WriteInstr        = 0,
  .eDeviceConfig        = AM_HAL_MSPI_FLASH_SERIAL_CE0,
  .ui8WriteLatency      = 0,
  .eSpiMode             = AM_HAL_MSPI_SPI_MODE_0,
  .eClockFreq           = AM_HAL_MSPI_CLK_16MHZ,
  .bEnWriteLatency      = false,
  .bSendAddr            = true,
  .bSendInstr           = false,
  .bTurnaround          = true,
  .bEmulateDDR          = false,
  .ui16DMATimeLimit     = 0,
  .eDMABoundary         = AM_HAL_MSPI_BOUNDARY_NONE,
  .ui32TCBSize          = 0,
  .pTCB                 = NULL,
  .scramblingStartAddr  = 0,
  .scramblingEndAddr    = 0,
};

//
//!
//
am_hal_mspi_dev_config_t  MSPI_FT81X_TestQuadCE0MSPIConfig =
{
  .ui8TurnAround        = 2,
  .eAddrCfg             = AM_HAL_MSPI_ADDR_3_BYTE,
  .eInstrCfg            = AM_HAL_MSPI_INSTR_1_BYTE,
  .ui8ReadInstr         = 0,
  .ui8WriteInstr        = 0,
  .eDeviceConfig        = AM_HAL_MSPI_FLASH_QUAD_CE0,
  .ui8WriteLatency      = 0,
  .eSpiMode             = AM_HAL_MSPI_SPI_MODE_0,
  .eClockFreq           = AM_HAL_MSPI_CLK_16MHZ,
  .bEnWriteLatency      = false,
  .bSendAddr            = true,
  .bSendInstr           = false,
  .bTurnaround          = true,
  .bEmulateDDR          = false,
  .ui16DMATimeLimit     = 0,
  .eDMABoundary         = AM_HAL_MSPI_BOUNDARY_NONE,
  .ui32TCBSize          = 0,
  .pTCB                 = NULL,
  .scramblingStartAddr  = 0,
  .scramblingEndAddr    = 0,
};

//
//!
//
typedef struct
{
    uint32_t                    ui32Module;
    void                        *pMspiHandle;
    bool                        bOccupied;
} am_devices_mspi_ft81x_t;

//
//!
//
am_devices_mspi_ft81x_t gAmFt81x[AM_DEVICES_MSPI_FT81X_MAX_DEVICE_NUM];

//*****************************************************************************
//
//! @brief
//!
//! @param pCallbackCtxt
//! @param status
//
//*****************************************************************************
void pfnMSPI_FT81X_Callback(void *pCallbackCtxt, uint32_t status)
{
    // Set the DMA complete flag.
    *(volatile bool *)pCallbackCtxt = true;
}

//*****************************************************************************
//
// Generic Command Write function.
//
//*****************************************************************************
//*****************************************************************************
//
//! @brief Generic Command Write function.
//!
//! @param pHandle
//! @param ui32Addr
//! @param pData
//! @param ui32NumBytes
//!
//! @return
//
//*****************************************************************************
uint32_t
am_devices_ft81x_host_command_write(void *pHandle,
                                    uint32_t ui32Addr,
                                    uint8_t *pData,
                                    uint32_t ui32NumBytes)
{
  am_hal_mspi_pio_transfer_t  Transaction;
  am_devices_mspi_ft81x_t *pDisplay = (am_devices_mspi_ft81x_t *)pHandle;

  // Create the individual write transaction.
  Transaction.ui32NumBytes            = ui32NumBytes;
  Transaction.bScrambling             = false;
  Transaction.bDCX                    = false;
  Transaction.eDirection              = AM_HAL_MSPI_TX;
  Transaction.bSendAddr               = true;
  Transaction.ui32DeviceAddr          = ui32Addr;
  Transaction.bSendInstr              = false;
  Transaction.ui16DeviceInstr         = 0;
  Transaction.bTurnaround             = false;
  Transaction.bEnWRLatency            = false;
  Transaction.bQuadCmd                = false;
  Transaction.bContinue               = false;
  Transaction.pui32Buffer             = (uint32_t *)pData;

  //
  // Execute the transction over MSPI.
  //
  if (am_hal_mspi_blocking_transfer(pDisplay->pMspiHandle, &Transaction, AM_DEVICES_MSPI_TIMEOUT))
  {
    return AM_DEVICES_FT81X_STATUS_ERROR;
  }

  return AM_DEVICES_FT81X_STATUS_SUCCESS;
}

//*****************************************************************************
//
//
//
//*****************************************************************************
uint32_t
am_devices_ft81x_command_write(void *pHandle,
                               uint32_t ui32Addr,
                               uint8_t *pData,
                               uint32_t ui32NumBytes)
{
  am_hal_mspi_pio_transfer_t  Transaction;
  am_devices_mspi_ft81x_t *pDisplay = (am_devices_mspi_ft81x_t *)pHandle;

  while (ui32NumBytes)
  {
    // Create the individual write transaction.
    Transaction.ui32NumBytes            = (ui32NumBytes > BYTE_NUM_PER_WRITE) ? BYTE_NUM_PER_WRITE : ui32NumBytes;
    Transaction.bScrambling             = false;
    Transaction.bDCX                    = false;
    Transaction.eDirection              = AM_HAL_MSPI_TX;
    Transaction.bSendAddr               = true;
    Transaction.ui32DeviceAddr          = 0x00800000 | ui32Addr;
    Transaction.bSendInstr              = false;
    Transaction.ui16DeviceInstr         = 0;
    Transaction.bTurnaround             = false;
    Transaction.bEnWRLatency            = false;
    Transaction.bQuadCmd                = false;
    Transaction.bContinue               = false;
    Transaction.pui32Buffer             = (uint32_t *)pData;

    //
    // Execute the transction over MSPI.
    //
    if (am_hal_mspi_blocking_transfer(pDisplay->pMspiHandle, &Transaction, AM_DEVICES_MSPI_TIMEOUT))
    {
      return AM_DEVICES_FT81X_STATUS_ERROR;
    }

    ui32NumBytes -= Transaction.ui32NumBytes;
    ui32Addr += Transaction.ui32NumBytes;
    pData += Transaction.ui32NumBytes;
  }
  return AM_DEVICES_FT81X_STATUS_SUCCESS;
}

//*****************************************************************************
//
//
//
//*****************************************************************************
uint32_t
am_devices_ft81x_command_read(void *pHandle,
                              uint32_t ui32Addr,
                              uint8_t *pData,
                              uint32_t ui32NumBytes)
{
  am_hal_mspi_pio_transfer_t  Transaction;
  am_devices_mspi_ft81x_t *pDisplay = (am_devices_mspi_ft81x_t *)pHandle;

  // Create the individual write transaction.
  Transaction.ui32NumBytes            = ui32NumBytes;
  Transaction.bScrambling             = false;
  Transaction.bDCX                    = false;
  Transaction.eDirection              = AM_HAL_MSPI_RX;
  Transaction.bSendAddr               = true;
  Transaction.ui32DeviceAddr          = ui32Addr;
  Transaction.bSendInstr              = false;
  Transaction.ui16DeviceInstr         = 0;
  Transaction.bTurnaround             = true;
  Transaction.bEnWRLatency            = false;
  Transaction.bQuadCmd                = false;
  Transaction.bContinue               = false;
  Transaction.pui32Buffer             = (uint32_t *)pData;

  //
  // Execute the transction over IOM.
  //
  if (am_hal_mspi_blocking_transfer(pDisplay->pMspiHandle, &Transaction, AM_DEVICES_MSPI_TIMEOUT))
  {
    return AM_DEVICES_FT81X_STATUS_ERROR;
  }
  return AM_DEVICES_FT81X_STATUS_SUCCESS;
}

//*****************************************************************************
//
//  Programs the given range of flash addresses.
//
//*****************************************************************************
uint32_t
am_devices_ft81x_nonblocking_write(void *pHandle,
                                   uint32_t ui32WriteAddress,
                                   uint8_t *pui8TxBuffer,
                                   uint32_t ui32NumBytes,
                                   bool bWaitForCompletion)
{
  am_hal_mspi_dma_transfer_t    Transaction;
  volatile bool                 bDMAComplete;
  am_devices_mspi_ft81x_t *pDisplay = (am_devices_mspi_ft81x_t *)pHandle;

  if (bWaitForCompletion)
  {

    bDMAComplete = false;
    while (ui32NumBytes)
    {
      //
      // Create the transaction.
      //
      Transaction.ui8Priority               = 1;
      Transaction.eDirection                = AM_HAL_MSPI_TX;
      Transaction.ui32TransferCount         = (ui32NumBytes > BYTE_NUM_PER_WRITE) ? BYTE_NUM_PER_WRITE : ui32NumBytes;
      Transaction.ui32DeviceAddress         = 0x00800000 | ui32WriteAddress;
      Transaction.ui32SRAMAddress           = (uint32_t)pui8TxBuffer;
      Transaction.ui32PauseCondition        = 0;
      Transaction.ui32StatusSetClr          = 0;

      //
      // Execute the transction over MSPI.
      //
      if (am_hal_mspi_nonblocking_transfer(pDisplay->pMspiHandle,
                                           &Transaction,
                                           AM_HAL_MSPI_TRANS_DMA,
                                           (ui32NumBytes > BYTE_NUM_PER_WRITE) ? NULL : pfnMSPI_FT81X_Callback,
                                           (ui32NumBytes > BYTE_NUM_PER_WRITE) ? NULL : (void *)&bDMAComplete))
      {
        return AM_DEVICES_FT81X_STATUS_ERROR;
      }

      ui32NumBytes -= Transaction.ui32TransferCount;
      ui32WriteAddress += Transaction.ui32TransferCount;
      pui8TxBuffer += Transaction.ui32TransferCount;
    }
  }

  // Wait for DMA Complete or Timeout
  for (uint32_t i = 0; i < AM_DEVICES_MSPI_TIMEOUT; i++)
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
    return AM_DEVICES_FT81X_STATUS_ERROR;
  }

  return AM_DEVICES_FT81X_STATUS_SUCCESS;
}

//*****************************************************************************
//
// Programs the given range of flash addresses.
//
//*****************************************************************************
uint32_t
am_devices_ft81x_nonblocking_write_adv(void *pHandle,
                                       uint32_t ui32WriteAddress,
                                       uint8_t *pui8TxBuffer,
                                       uint32_t ui32NumBytes,
                                       uint32_t ui32PauseCondition,
                                       uint32_t ui32StatusSetClr,
                                       am_hal_mspi_callback_t pfnCallback,
                                       void *pCallbackCtxt)
{
  am_hal_mspi_dma_transfer_t Transaction;
  am_devices_mspi_ft81x_t *pDisplay = (am_devices_mspi_ft81x_t *)pHandle;

  //
  // Create the transaction.
  //
  Transaction.ui8Priority               = 1;
  Transaction.eDirection                = AM_HAL_MSPI_TX;
  Transaction.ui32TransferCount         = ui32NumBytes;
  Transaction.ui32DeviceAddress         = ui32WriteAddress;
  Transaction.ui32SRAMAddress           = (uint32_t)pui8TxBuffer;
  Transaction.ui32PauseCondition        = ui32PauseCondition;
  Transaction.ui32StatusSetClr          = ui32StatusSetClr;

  //    am_hal_gpio_state_write(AM_BSP_GPIO_IOM0_DCX, AM_HAL_GPIO_OUTPUT_CLEAR);
  //
  // Execute the transction over IOM.
  //
  if (am_hal_mspi_nonblocking_transfer(pDisplay->pMspiHandle,
                                       &Transaction,
                                       AM_HAL_MSPI_TRANS_DMA,
                                       pfnCallback,
                                       pCallbackCtxt))
  {
    return AM_DEVICES_FT81X_STATUS_ERROR;
  }

  return AM_DEVICES_FT81X_STATUS_SUCCESS;
}

//*****************************************************************************
//
// Reads the contents of the fram into a buffer.
//
//*****************************************************************************
uint32_t
am_devices_ft81x_nonblocking_read(void *pHandle,
                                  uint32_t ui32ReadAddress,
                                  uint8_t *pui8RxBuffer,
                                  uint32_t ui32NumBytes,
                                  bool bWaitForCompletion)
{
  am_hal_mspi_dma_transfer_t Transaction;
  volatile bool                 bDMAComplete;
  am_devices_mspi_ft81x_t *pDisplay = (am_devices_mspi_ft81x_t *)pHandle;

  if (bWaitForCompletion)
  {

    bDMAComplete = false;
    while (ui32NumBytes)
    {
      //
      // Create the transaction.
      //
      Transaction.ui8Priority               = 1;
      Transaction.eDirection                = AM_HAL_MSPI_RX;
      Transaction.ui32TransferCount         = (ui32NumBytes > BYTE_NUM_PER_WRITE) ? BYTE_NUM_PER_WRITE : ui32NumBytes;
      Transaction.ui32DeviceAddress         = ui32ReadAddress;
      Transaction.ui32SRAMAddress           = (uint32_t)pui8RxBuffer;
      Transaction.ui32PauseCondition        = 0;
      Transaction.ui32StatusSetClr          = 0;

      //
      // Execute the transction over MSPI.
      //
      if (am_hal_mspi_nonblocking_transfer(pDisplay->pMspiHandle,
                                           &Transaction,
                                           AM_HAL_MSPI_TRANS_DMA,
                                           (ui32NumBytes > BYTE_NUM_PER_WRITE) ? NULL : pfnMSPI_FT81X_Callback,
                                           (ui32NumBytes > BYTE_NUM_PER_WRITE) ? NULL : (void *)&bDMAComplete))
      {
        return AM_DEVICES_FT81X_STATUS_ERROR;
      }

      ui32NumBytes -= Transaction.ui32TransferCount;
      ui32ReadAddress += Transaction.ui32TransferCount;
      pui8RxBuffer += Transaction.ui32TransferCount;
    }
  }

  // Wait for DMA Complete or Timeout
  for (uint32_t i = 0; i < AM_DEVICES_MSPI_TIMEOUT; i++)
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
    return AM_DEVICES_FT81X_STATUS_ERROR;
  }
  return AM_DEVICES_FT81X_STATUS_SUCCESS;
}

static uint32_t
ft81x_init_display(void *pHandle)
{
  uint32_t      Command;

  //  Configure display registers - demonstration for WQVGA resolution */

#if 0
  //  REG_HCYCLE(548);
  Command = 0x00000224;
  if ( am_devices_ft81x_command_write(pHandle, ui32Module,AM_DEVICES_FX81X_REG_HCYCLE, (uint8_t *)&Command, 2) )
  {
    return AM_DEVICES_FT81X_STATUS_ERROR;
  }

  //  REG_HOFFSET(43);
  Command = 0x0000002B;
  if ( am_devices_ft81x_command_write(pHandle, AM_DEVICES_FT81X_REG_HOFFSET, (uint8_t *)&Command, 2) )
  {
    return AM_DEVICES_FT81X_STATUS_ERROR;
  }

  //  REG_HSYNC0(0);
  Command = 0x00000000;
  if ( am_devices_ft81x_command_write(pHandle, AM_DEVICES_FT81X_REG_HSYNC0, (uint8_t *)&Command, 2) )
  {
    return AM_DEVICES_FT81X_STATUS_ERROR;
  }

  //  REG_HSYNC1(41);
  Command = 0x00000029;
  if ( am_devices_ft81x_command_write(pHandle, AM_DEVICES_FT81X_REG_HSYNC1, (uint8_t *)&Command, 2) )
  {
    return AM_DEVICES_FT81X_STATUS_ERROR;
  }

  //  REG_VCYCLE(292);
  Command = 0x00000124;
  if ( am_devices_ft81x_command_write(pHandle, AM_DEVICES_FT81X_REG_VCYCLE, (uint8_t *)&Command, 2) )
  {
    return AM_DEVICES_FT81X_STATUS_ERROR;
  }

  //  REG_VOFFSET(12);
  Command = 0x0000000C;
  if ( am_devices_ft81x_command_write(pHandle, AM_DEVICES_FT81X_REG_VOFFSET, (uint8_t *)&Command, 2) )
  {
    return AM_DEVICES_FT81X_STATUS_ERROR;
  }

  //  REG_VSYNC0(0);
  Command = 0x00000000;
  if ( am_devices_ft81x_command_write(pHandle, AM_DEVICES_FT81X_REG_VSYNC0, (uint8_t *)&Command, 2) )
  {
    return AM_DEVICES_FT81X_STATUS_ERROR;
  }

  //  REG_VSYNC1(10);
  Command = 0x0000000A;
  if ( am_devices_ft81x_command_write(pHandle, AM_DEVICES_FT81X_REG_VSYNC1, (uint8_t *)&Command, 2) )
  {
    return AM_DEVICES_FT81X_STATUS_ERROR;
  }

  //  REG_SWIZZLE(0);
  Command = 0x00000000;
  if ( am_devices_ft81x_command_write(pHandle, AM_DEVICES_FT81X_REG_SWIZZLE, (uint8_t *)&Command, 1) )
  {
    return AM_DEVICES_FT81X_STATUS_ERROR;
  }

  //  REG_PCLK_POL(1);
  Command = 0x00000001;
  if ( am_devices_ft81x_command_write(pHandle, AM_DEVICES_FT81X_REG_PCLK_POL, (uint8_t *)&Command, 1) )
  {
    return AM_DEVICES_FT81X_STATUS_ERROR;
  }

  //  REG_CSPREAD(1);
  Command = 0x00000001;
  if ( am_devices_ft81x_command_write(pHandle, AM_DEVICES_FT81X_REG_CSPREAD, (uint8_t *)&Command, 1) )
  {
    return AM_DEVICES_FT81X_STATUS_ERROR;
  }

  //  REG_HSIZE(480);
  Command = 0x000001E0;
  if ( am_devices_ft81x_command_write(pHandle, AM_DEVICES_FT81X_REG_HSIZE, (uint8_t *)&Command, 2) )
  {
    return AM_DEVICES_FT81X_STATUS_ERROR;
  }

  //  REG_VSIZE(272);
  Command = 0x00000110;
  if ( am_devices_ft81x_command_write(pHandle, AM_DEVICES_FT81X_REG_VSIZE, (uint8_t *)&Command, 2) )
  {
    return AM_DEVICES_FT81X_STATUS_ERROR;
  }
#endif

#if 1
  // CLEAR_COLOR_RGB(255,255,255)
  Command = 0x02FFFFFF;
  if ( am_devices_ft81x_command_write(pHandle, AM_DEVICES_FT81X_RAM_DL_BASE + 0, (uint8_t *)&Command, 4) )
  {
    return AM_DEVICES_FT81X_STATUS_ERROR;
  }

 // CLEAR(1,0,0)
  Command = 0x26000004;
  if ( am_devices_ft81x_command_write(pHandle, AM_DEVICES_FT81X_RAM_DL_BASE + 4, (uint8_t *)&Command, 4) )
  {
    return AM_DEVICES_FT81X_STATUS_ERROR;
  }

  // DISPLAY command
  Command = 0x00000000;
  if ( am_devices_ft81x_command_write(pHandle, AM_DEVICES_FT81X_RAM_DL_BASE + 8, (uint8_t *)&Command, 4) )
  {
    return AM_DEVICES_FT81X_STATUS_ERROR;
  }

  //REG_DLSWAP(2)
  Command = 0x00000002;
  if ( am_devices_ft81x_command_write(pHandle, AM_DEVICES_FT81X_REG_DLSWAP, (uint8_t *)&Command, 4) )
  {
    return AM_DEVICES_FT81X_STATUS_ERROR;
  }

  // REG_GPIO_DIR |= 0x80;
  Command = 0;
  if ( am_devices_ft81x_command_read(pHandle, AM_DEVICES_FT81X_REG_GPIO_DIR, (uint8_t *)&Command, 1) )
  {
    return AM_DEVICES_FT81X_STATUS_ERROR;
  }
  Command |= 0x00000080;
  if ( am_devices_ft81x_command_write(pHandle, AM_DEVICES_FT81X_REG_GPIO_DIR, (uint8_t *)&Command, 1) )
  {
    return AM_DEVICES_FT81X_STATUS_ERROR;
  }

  //REG_GPIO |= 0x080;
  Command = 0;
  if ( am_devices_ft81x_command_read(pHandle, AM_DEVICES_FT81X_REG_GPIO, (uint8_t *)&Command, 1) )
  {
    return AM_DEVICES_FT81X_STATUS_ERROR;
  }
  Command |= 0x00000080;
  if ( am_devices_ft81x_command_write(pHandle, AM_DEVICES_FT81X_REG_GPIO, (uint8_t *)&Command, 1) )
  {
    return AM_DEVICES_FT81X_STATUS_ERROR;
  }

  // REG_PCLK(5);
  Command = 0x00000005;
  if ( am_devices_ft81x_command_write(pHandle, AM_DEVICES_FT81X_REG_PCLK, (uint8_t *)&Command, 1) )
  {
    return AM_DEVICES_FT81X_STATUS_ERROR;
  }
#endif

  return AM_DEVICES_FT81X_STATUS_SUCCESS;
}

//*****************************************************************************
//
//! @brief
//! @param pHandle
//! @return
//
//*****************************************************************************
static uint32_t
ft81x_ftdi_red_dot(void *pHandle)
{
  uint32_t      Command;

  // CLEAR(1,1,1)
  Command = 0x26000007;
  if ( am_devices_ft81x_command_write(pHandle, AM_DEVICES_FT81X_RAM_DL_BASE + 0, (uint8_t *)&Command, 4) )
  {
    return AM_DEVICES_FT81X_STATUS_ERROR;
  }

  // BEGIN(BITMAP) command
  Command = 0x1F000001;
  if ( am_devices_ft81x_command_write(pHandle, AM_DEVICES_FT81X_RAM_DL_BASE + 4, (uint8_t *)&Command, 4) )
  {
    return AM_DEVICES_FT81X_STATUS_ERROR;
  }

  // VERTEX2II(220,110,31,'F') command
  Command = 0x80000000 | ((uint32_t)220 << 21) | ((uint32_t)110 << 12) | ((uint32_t)31 << 7) | 'F';
  if ( am_devices_ft81x_command_write(pHandle, AM_DEVICES_FT81X_RAM_DL_BASE + 8, (uint8_t *)&Command, 4) )
  {
    return AM_DEVICES_FT81X_STATUS_ERROR;
  }

  // VERTEX2II(244,110,31,'T') command
  Command = 0x80000000 | ((uint32_t)244 << 21) | ((uint32_t)110 << 12) | ((uint32_t)31 << 7) | 'T';
  if ( am_devices_ft81x_command_write(pHandle, AM_DEVICES_FT81X_RAM_DL_BASE + 12, (uint8_t *)&Command, 4) )
  {
    return AM_DEVICES_FT81X_STATUS_ERROR;
  }

  // VERTEX2II(270,110,31,'D') command
  Command = 0x80000000 | ((uint32_t)270 << 21) | ((uint32_t)110 << 12) | ((uint32_t)31 << 7) | 'D';
  if ( am_devices_ft81x_command_write(pHandle, AM_DEVICES_FT81X_RAM_DL_BASE + 16, (uint8_t *)&Command, 4) )
  {
    return AM_DEVICES_FT81X_STATUS_ERROR;
  }

  // VERTEX2II(299,110,31,'I') command
  Command = 0x80000000 | ((uint32_t)299 << 21) | ((uint32_t)110 << 12) | ((uint32_t)31 << 7) | 'I';
  if ( am_devices_ft81x_command_write(pHandle, AM_DEVICES_FT81X_RAM_DL_BASE + 20, (uint8_t *)&Command, 4) )
  {
    return AM_DEVICES_FT81X_STATUS_ERROR;
  }

  // END command
  Command = 0x21000000;
  if ( am_devices_ft81x_command_write(pHandle, AM_DEVICES_FT81X_RAM_DL_BASE + 24, (uint8_t *)&Command, 4) )
  {
    return AM_DEVICES_FT81X_STATUS_ERROR;
  }

  // COLOR_RGB(160,22,22)
  Command = 0x04000000 | ((uint32_t)160 << 16) | ((uint32_t)22 << 8) | ((uint32_t)22);
  if ( am_devices_ft81x_command_write(pHandle, AM_DEVICES_FT81X_RAM_DL_BASE + 28, (uint8_t *)&Command, 4) )
  {
    return AM_DEVICES_FT81X_STATUS_ERROR;
  }

  // POINT_SIZE(320)
  Command = 0x0D000000 | ((uint32_t)320);
  if ( am_devices_ft81x_command_write(pHandle, AM_DEVICES_FT81X_RAM_DL_BASE + 32, (uint8_t *)&Command, 4) )
  {
    return AM_DEVICES_FT81X_STATUS_ERROR;
  }

  // BEGIN(POINTS) command
  Command = 0x1F000002;
  if ( am_devices_ft81x_command_write(pHandle, AM_DEVICES_FT81X_RAM_DL_BASE + 36, (uint8_t *)&Command, 4) )
  {
    return AM_DEVICES_FT81X_STATUS_ERROR;
  }

  // VERTEX2II(192,133,0,0) command
  Command = 0x80000000 | ((uint32_t)192 << 21) | ((uint32_t)133 << 12) | ((uint32_t)0 << 7) | 0;
  if ( am_devices_ft81x_command_write(pHandle, AM_DEVICES_FT81X_RAM_DL_BASE + 40, (uint8_t *)&Command, 4) )
  {
    return AM_DEVICES_FT81X_STATUS_ERROR;
  }

  // END command
  Command = 0x21000000;
  if ( am_devices_ft81x_command_write(pHandle, AM_DEVICES_FT81X_RAM_DL_BASE + 44, (uint8_t *)&Command, 4) )
  {
    return AM_DEVICES_FT81X_STATUS_ERROR;
  }

  // DISPLAY command
  Command = 0x00000000;
  if ( am_devices_ft81x_command_write(pHandle, AM_DEVICES_FT81X_RAM_DL_BASE + 48, (uint8_t *)&Command, 4) )
  {
    return AM_DEVICES_FT81X_STATUS_ERROR;
  }

  //REG_DLSWAP(2)
  Command = 0x00000002;
  if ( am_devices_ft81x_command_write(pHandle, AM_DEVICES_FT81X_REG_DLSWAP, (uint8_t *)&Command, 4) )
  {
    return AM_DEVICES_FT81X_STATUS_ERROR;
  }

  return AM_DEVICES_FT81X_STATUS_SUCCESS;
}

//*****************************************************************************
//
//! @brief
//! @param pHandle
//! @param pBitMapImage
//! @param ui32Width
//! @param ui32Height
//! @param ui32NumBytes
//! @return
//
//*****************************************************************************
static uint32_t
ft81x_write_bitmap(void *pHandle,
                   uint8_t *pBitMapImage,
                   uint32_t ui32Width,
                   uint32_t ui32Height,
                   uint32_t ui32NumBytes)
{
  uint32_t      Command;
  uint8_t ui8TXBuffer[32*1024];
  uint8_t ui8RXBuffer[32*1024];

#if 0
  am_devices_mspi_ft81x_t *pDisplay = (am_devices_mspi_ft81x_t *)pHandle;

  //  Configure the interface for Serial SPI operation.
  Command = 0;
  if ( am_devices_ft81x_command_write(pHandle, AM_DEVICES_FT81X_REG_SPI_WIDTH, (uint8_t *)&Command, 1) )
  {
    return AM_DEVICES_FT81X_STATUS_ERROR;
  }

  //
  // Put the MSPI into the desired mode.
  //
  if ( am_hal_mspi_disable(pDisplay->pMspiHandle) ||
      am_hal_mspi_device_configure(pDisplay->pMspiHandle, &MSPI_FT81X_SerialCE0MSPIConfig) ||
        am_hal_mspi_enable(pDisplay->pMspiHandle) )
  {
    return AM_DEVICES_FT81X_STATUS_ERROR;
  }
#endif
#if 0
  if (AM_DEVICES_FT81X_STATUS_SUCCESS != am_devices_ft81x_nonblocking_write(pHandle,
                                                                              AM_DEVICES_FT81X_DISPLAY_RAM_BASE,
                                                                              pBitMapImage,
                                                                              ui32NumBytes,
                                                                              true))
  {
    return AM_DEVICES_FT81X_STATUS_ERROR;
  }
#else
  // Initial the TX Buffer.
  for (uint32_t i = 0; i < sizeof(ui8TXBuffer); i++)
  {
    ui8TXBuffer[i] = i & 0xFF;
//    ui8TXBuffer[i] = 0xA5;
  }

//  while (1)
//  {
  // Write the TX Buffer to the Display RAM
  if (AM_DEVICES_FT81X_STATUS_SUCCESS != am_devices_ft81x_nonblocking_write(pHandle,
                                                                            AM_DEVICES_FT81X_DISPLAY_RAM_BASE,
                                                                            ui8TXBuffer,
                                                                            sizeof(ui8TXBuffer),
                                                                            true))
  {
    return AM_DEVICES_FT81X_STATUS_ERROR;
  }
//  }
  // Read the Display RAM back into the RX Buffer
  if (AM_DEVICES_FT81X_STATUS_SUCCESS != am_devices_ft81x_nonblocking_read(pHandle,
                                                                           AM_DEVICES_FT81X_DISPLAY_RAM_BASE,
                                                                           ui8RXBuffer,
                                                                           sizeof(ui8RXBuffer),
                                                                           true))
  {
    return AM_DEVICES_FT81X_STATUS_ERROR;
  }
  // Compare the TX and RX buffer.
  for (uint32_t i = 0; i < sizeof(ui8TXBuffer); i++)
  {
    if (ui8TXBuffer[i] != ui8RXBuffer[i])
    {
      uint32_t breakpoint = 0;
    }
  }
#endif

#if 0
  //  Configure the interface for Quad SPI operation.
  Command = 2;
  if ( am_devices_ft81x_command_write(pHandle, AM_DEVICES_FT81X_REG_SPI_WIDTH, (uint8_t *)&Command, 1) )
  {
    return AM_DEVICES_FT81X_STATUS_ERROR;
  }

  //
  // Put the MSPI into the desired mode.
  //
  if ( am_hal_mspi_disable(pDisplay->pMspiHandle) ||
      am_hal_mspi_device_configure(pDisplay->pMspiHandle, &MSPI_FT81X_TestQuadCE0MSPIConfig) ||
        am_hal_mspi_enable(pDisplay->pMspiHandle) )
  {
    return AM_DEVICES_FT81X_STATUS_ERROR;
  }
#endif

  // CLEAR_COLOR_RGB(255,255,255)
  Command = 0x02FFFFFF;
  if ( am_devices_ft81x_command_write(pHandle, AM_DEVICES_FT81X_RAM_DL_BASE + 0, (uint8_t *)&Command, 4) )
  {
    return AM_DEVICES_FT81X_STATUS_ERROR;
  }

  // CLEAR(1,1,1)
  Command = 0x26000007;
  if ( am_devices_ft81x_command_write(pHandle, AM_DEVICES_FT81X_RAM_DL_BASE + 4, (uint8_t *)&Command, 4) )
  {
    return AM_DEVICES_FT81X_STATUS_ERROR;
  }

 // COLOR_RGB(255,255,255)
  Command = 0x04FFFFFF;
  if ( am_devices_ft81x_command_write(pHandle, AM_DEVICES_FT81X_RAM_DL_BASE + 8, (uint8_t *)&Command, 4) )
  {
    return AM_DEVICES_FT81X_STATUS_ERROR;
  }

  // BITMAP_SOURCE(RAM_G) command
  Command = 0x01000000;
  if ( am_devices_ft81x_command_write(pHandle, AM_DEVICES_FT81X_RAM_DL_BASE + 12, (uint8_t *)&Command, 4) )
  {
    return AM_DEVICES_FT81X_STATUS_ERROR;
  }

  // BITMAP_LAYOUT (RGB565, stride, height) command
  Command = 0x07380000 | ((uint32_t)RGB565 << 19) | (((ui32Width *2) & 0x3FF) << 9) | (ui32Height & 0x1FF);
  if ( am_devices_ft81x_command_write(pHandle, AM_DEVICES_FT81X_RAM_DL_BASE + 16, (uint8_t *)&Command, 4) )
  {
    return AM_DEVICES_FT81X_STATUS_ERROR;
  }

  // BITMAP_SIZE (width, height) command
  Command = 0x08000000 | ((ui32Width & 0x1FF) << 9) | (ui32Height & 0x1FF);
  if ( am_devices_ft81x_command_write(pHandle, AM_DEVICES_FT81X_RAM_DL_BASE + 20, (uint8_t *)&Command, 4) )
  {
    return AM_DEVICES_FT81X_STATUS_ERROR;
  }

  // BEGIN(BITMAP) command
  Command = 0x1F000001;
  if ( am_devices_ft81x_command_write(pHandle, AM_DEVICES_FT81X_RAM_DL_BASE + 24, (uint8_t *)&Command, 4) )
  {
    return AM_DEVICES_FT81X_STATUS_ERROR;
  }

  // VERTEX2II(20,100) command
  Command = 0x80000000 | ((uint32_t)20 << 21) | ((uint32_t)100 << 12);
  if ( am_devices_ft81x_command_write(pHandle, AM_DEVICES_FT81X_RAM_DL_BASE + 28, (uint8_t *)&Command, 4) )
  {
    return AM_DEVICES_FT81X_STATUS_ERROR;
  }

  // END command
  Command = 0x21000000;
  if ( am_devices_ft81x_command_write(pHandle, AM_DEVICES_FT81X_RAM_DL_BASE + 32, (uint8_t *)&Command, 4) )
  {
    return AM_DEVICES_FT81X_STATUS_ERROR;
  }

  // DISPLAY command
  Command = 0x00000000;
  if ( am_devices_ft81x_command_write(pHandle, AM_DEVICES_FT81X_RAM_DL_BASE + 36, (uint8_t *)&Command, 4) )
  {
    return AM_DEVICES_FT81X_STATUS_ERROR;
  }

  //REG_DLSWAP(2)
  Command = 0x00000002;
  if ( am_devices_ft81x_command_write(pHandle, AM_DEVICES_FT81X_REG_DLSWAP, (uint8_t *)&Command, 4) )
  {
    return AM_DEVICES_FT81X_STATUS_ERROR;
  }

  //REG_GPIO_DIR |= 0x80;
  Command = 0;
  if ( am_devices_ft81x_command_read(pHandle, AM_DEVICES_FT81X_REG_GPIO_DIR, (uint8_t *)&Command, 1) )
  {
    return AM_DEVICES_FT81X_STATUS_ERROR;
  }
  Command |= 0x80;
  if ( am_devices_ft81x_command_write(pHandle, AM_DEVICES_FT81X_REG_GPIO_DIR, (uint8_t *)&Command, 1) )
  {
    return AM_DEVICES_FT81X_STATUS_ERROR;
  }

  //REG_GPIO |= 0x080;
  Command = 0;
  if ( am_devices_ft81x_command_read(pHandle, AM_DEVICES_FT81X_REG_GPIO, (uint8_t *)&Command, 1) )
  {
    return AM_DEVICES_FT81X_STATUS_ERROR;
  }
  Command |= 0x80;
  if ( am_devices_ft81x_command_write(pHandle, AM_DEVICES_FT81X_REG_GPIO, (uint8_t *)&Command, 1) )
  {
    return AM_DEVICES_FT81X_STATUS_ERROR;
  }

  //REG_PCLK(5);//after this display is visible on the LCD
  Command = 0x00000005;
  if ( am_devices_ft81x_command_write(pHandle, AM_DEVICES_FT81X_REG_PCLK, (uint8_t *)&Command, 1) )
  {
    return AM_DEVICES_FT81X_STATUS_ERROR;
  }

  return AM_DEVICES_FT81X_STATUS_SUCCESS;
}

//*****************************************************************************
//
// Initialize the FT81X driver.
//
//*****************************************************************************
uint32_t
am_devices_mspi_ft81x_init(uint32_t ui32Module,
                           am_devices_mspi_ft81x_config_t *psMspiSettings,
                           void **ppHandle,
                           void **ppMspiHandle)
{
  uint32_t      ui32Status;
  uint32_t      ui32RegID = 0;
  uint32_t      ui32DeviceID = 0;
  uint32_t      ui32Index = 0;
  am_hal_mspi_dev_config_t    mspiDevCfg;
  void                       *pMspiHandle;

  if ((ui32Module > AM_REG_MSPI_NUM_MODULES) || (psMspiSettings == NULL))
  {
    return AM_DEVICES_FT81X_STATUS_ERROR;
  }

  // Allocate a vacant device handle
  for ( ui32Index = 0; ui32Index < AM_DEVICES_MSPI_FT81X_MAX_DEVICE_NUM; ui32Index++ )
  {
      if ( gAmFt81x[ui32Index].bOccupied == false )
      {
          break;
      }
  }
  if ( ui32Index == AM_DEVICES_MSPI_FT81X_MAX_DEVICE_NUM )
  {
      return AM_DEVICES_FT81X_STATUS_ERROR;
  }

  //
  // Re-Configure the MSPI for the requested operation mode.
  //
  switch (psMspiSettings->eDeviceConfig)
  {
      case AM_HAL_MSPI_FLASH_SERIAL_CE0:
          mspiDevCfg = MSPI_FT81X_SerialCE0MSPIConfig;
          break;
      case AM_HAL_MSPI_FLASH_QUAD_CE0:
          mspiDevCfg = MSPI_FT81X_TestQuadCE0MSPIConfig;
          break;
      default:
          return AM_DEVICES_FT81X_STATUS_ERROR;
  }
  mspiDevCfg.eClockFreq = psMspiSettings->eClockFreq;
  //mspiDevCfg.eDeviceConfig = psMspiSettings->eDeviceConfig;
  mspiDevCfg.ui32TCBSize = psMspiSettings->ui32NBTxnBufLength;
  mspiDevCfg.pTCB = psMspiSettings->pNBTxnBuf;
  mspiDevCfg.scramblingStartAddr = psMspiSettings->ui32ScramblingStartAddr;
  mspiDevCfg.scramblingEndAddr = psMspiSettings->ui32ScramblingEndAddr;

  //
  // Initialize the MSPI instance.
  // Enable power to the MSPI instance.
  // Configure the MSPI for Serial operation during initialization.
  // Enable the MSPI.
  // HAL Success return is 0
  //
  if ( am_hal_mspi_initialize(ui32Module, &pMspiHandle) ||
      am_hal_mspi_power_control(pMspiHandle, AM_HAL_SYSCTRL_WAKE, false) ||
        am_hal_mspi_device_configure(pMspiHandle, &mspiDevCfg) ||
          am_hal_mspi_enable(pMspiHandle) )
  {
    return AM_DEVICES_FT81X_STATUS_ERROR;
  }
  else
  {
      gAmFt81x[ui32Index].pMspiHandle = pMspiHandle;
      gAmFt81x[ui32Index].ui32Module = ui32Module;
      *ppMspiHandle = pMspiHandle;
      *ppHandle = (void *)&gAmFt81x[ui32Index];
  }

  //
  // Configure the MSPI pins.
  //
  am_bsp_mspi_pins_enable(ui32Module, mspiDevCfg.eDeviceConfig);

  //
  // Enable MSPI interrupts.
  //

  ui32Status = am_hal_mspi_interrupt_clear(pMspiHandle, AM_HAL_MSPI_INT_CQUPD | AM_HAL_MSPI_INT_ERR );
  if (AM_HAL_STATUS_SUCCESS != ui32Status)
  {
    return AM_DEVICES_FT81X_STATUS_ERROR;
  }

  ui32Status = am_hal_mspi_interrupt_enable(pMspiHandle, AM_HAL_MSPI_INT_CQUPD | AM_HAL_MSPI_INT_ERR );
  if (AM_HAL_STATUS_SUCCESS != ui32Status)
  {
    return AM_DEVICES_FT81X_STATUS_ERROR;
  }

  //
  // Set the FT81X to use internal clock
  //
  if (AM_DEVICES_FT81X_STATUS_SUCCESS != am_devices_ft81x_host_command_write((void*)&gAmFt81x[ui32Index], 0x00480000, NULL, 0))
  {
    return AM_DEVICES_FT81X_STATUS_ERROR;
  }
  //
  // Put the display into ACTIVE mode.
  //
  if (AM_DEVICES_FT81X_STATUS_SUCCESS != am_devices_ft81x_host_command_write((void*)&gAmFt81x[ui32Index], 0x00000000, NULL, 0))
  {
    return AM_DEVICES_FT81X_STATUS_ERROR;
  }

#if 1
  //  Configure the interface for Serial SPI operation.
  uint32_t Data = 0;
  if ( am_devices_ft81x_command_write((void*)&gAmFt81x[ui32Index], AM_DEVICES_FT81X_REG_SPI_WIDTH, (uint8_t *)&Data, 1) )
  {
    return AM_DEVICES_FT81X_STATUS_ERROR;
  }
#else
  //  Configure the interface for Quad SPI operation.
  uint32_t Data = 2;
  if ( am_devices_ft81x_command_write((void*)&gAmFt81x[ui32Index], AM_DEVICES_FT81X_REG_SPI_WIDTH, (uint8_t *)&Data, 1) )
  {
    return AM_DEVICES_FT81X_STATUS_ERROR;
  }

  //
  // Put the MSPI into the desired mode.
  //
  if ( am_hal_mspi_disable(pMspiHandle) ||
      am_hal_mspi_device_configure(pMspiHandle, &mspiDevCfg) ||
        am_hal_mspi_enable(pMspiHandle) )
  {
    return AM_DEVICES_FT81X_STATUS_ERROR;
  }
#endif

  am_util_delay_ms(300);

  if (AM_DEVICES_FT81X_STATUS_SUCCESS != am_devices_ft81x_command_read((void*)&gAmFt81x[ui32Index], AM_DEVICES_FT81X_REG_ID, (uint8_t *)&ui32RegID, 1))
  {
    return AM_DEVICES_FT81X_STATUS_ERROR;
  }

  if (AM_DEVICES_FT81X_REG_ID_VAL != (ui32RegID & 0x000000FF))
  {
    return AM_DEVICES_FT81X_STATUS_ERROR;
  }

  am_util_delay_ms(100);

  // Read the Device ID
  if (AM_DEVICES_FT81X_STATUS_SUCCESS != am_devices_ft81x_command_read((void*)&gAmFt81x[ui32Index], AM_DEVICES_FT81X_DEVICE_ID_ADDR, (uint8_t *)&ui32DeviceID, 4))
  {
    return AM_DEVICES_FT81X_STATUS_ERROR;
  }

  // Compare the Device ID to the expected value.
  if (AM_DEVICES_FT81X_DEVICE_ID_VAL != ui32DeviceID)
  {
    return AM_DEVICES_FT81X_STATUS_ERROR;
  }

  //
  // Device specific TFT display initialization.
  //
  am_util_delay_ms(500);
  ui32Status = ft81x_init_display((void*)&gAmFt81x[ui32Index]);
  if (AM_DEVICES_FT81X_STATUS_SUCCESS != ui32Status)
  {
    return AM_DEVICES_FT81X_STATUS_ERROR;
  }

  //ui32Status = ft81x_ftdi_red_dot((void*)&gAmFt81x[ui32Index]);

  //am_util_delay_ms(5000);

  extern const unsigned char g_ambiq_logo_bmp[];
  ui32Status = ft81x_write_bitmap((void*)&gAmFt81x[ui32Index], (uint8_t *)g_ambiq_logo_bmp, 440, 81, 71280);

  if (AM_DEVICES_FT81X_STATUS_SUCCESS != ui32Status)
  {
    return AM_DEVICES_FT81X_STATUS_ERROR;
  }
  gAmFt81x[ui32Index].bOccupied = true;

  return AM_DEVICES_FT81X_STATUS_SUCCESS;
}

//*****************************************************************************
//
// De-Initialize the FT81X driver.
//
//*****************************************************************************
uint32_t
am_devices_ft81x_term(void *pHandle)
{
    am_devices_mspi_ft81x_t *pDisplay = (am_devices_mspi_ft81x_t *)pHandle;

  if ( pDisplay->ui32Module > AM_REG_MSPI_NUM_MODULES )
  {
    return AM_DEVICES_FT81X_STATUS_ERROR;
  }

  //
  // Disable and clear the interrupts to start with.
  //
  if (AM_HAL_STATUS_SUCCESS != am_hal_mspi_interrupt_disable(pDisplay->pMspiHandle, 0xFFFFFFFF))
  {
    return AM_DEVICES_FT81X_STATUS_ERROR;
  }
  if (AM_HAL_STATUS_SUCCESS != am_hal_mspi_interrupt_clear(pDisplay->pMspiHandle, 0xFFFFFFFF))
  {
    return AM_DEVICES_FT81X_STATUS_ERROR;
  }

  //
  // Disable the IOM.
  //
  am_hal_mspi_disable(pDisplay->pMspiHandle);

  //
  // Disable power to and uninitialize the IOM instance.
  //
  am_hal_mspi_power_control(pDisplay->pMspiHandle, AM_HAL_SYSCTRL_DEEPSLEEP, false);

  am_hal_mspi_deinitialize(pDisplay->pMspiHandle);

  // Free this device handle
  pDisplay->bOccupied = false;

  //
  // Return the status.
  //
  return AM_DEVICES_FT81X_STATUS_SUCCESS;
}

//*****************************************************************************
//
// End Doxygen group.
//! @}
//
//*****************************************************************************

