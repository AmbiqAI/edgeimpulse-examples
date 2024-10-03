//*****************************************************************************
//
//! @file am_widget_uart.c
//!
//! @brief This widget allows test cases to exercise the Apollo2 UART HAL
//! using loopback between UART #0 and #1.
//!
//!
//!
//! @endverbatim
//
//*****************************************************************************

//*****************************************************************************
//
// ${copyright}
//
// This is part of revision ${version} of the AmbiqSuite Development Package.
//
//*****************************************************************************

#include <stdlib.h>
#include "am_mcu_apollo.h"
#include "am_octal_widget_mspi.h"
#include "am_devices_atxp032.h"
#include "am_bsp.h"
#include "am_util.h"

//*****************************************************************************
//
// Macro definitions
//
//*****************************************************************************

//*****************************************************************************
//
// Global data
//
//*****************************************************************************
am_widget_mspi_config_t g_MSPITestConfig;               // Test configuration.

uint8_t                 g_SectorTXBuffer[64*1024];      // 64K SRAM TX buffer
uint8_t                 g_SectorRXBuffer[64*1024];      // 64K SRAM RX buffer.
uint8_t                 g_SectorRXBuffer2[1024];      // 64K SRAM RX buffer.

uint8_t       ui8Byte;
uint32_t      ui32Word, ui32CmpWord, ui32MiscompareCount;

//*****************************************************************************
//
// Memory allocation.
//
//*****************************************************************************

//*****************************************************************************
//
// Interrupt handler for MSPI
//
//*****************************************************************************

//*****************************************************************************
//
// Widget Setup Function.
//
//*****************************************************************************
uint32_t am_widget_mspi_octal_setup(am_widget_mspi_config_t *pMSPITestConfig, void **ppWidget, char *pErrStr)
{
  g_MSPITestConfig = *pMSPITestConfig;
  uint32_t      ui32Status;

  //
  // Initialize the widget state configuration.
  //
  *ppWidget = &g_MSPITestConfig;

  // Set up the MSPI configuraton for the N25Q256A part.
  ui32Status = am_devices_atxp032_init(&pMSPITestConfig->MSPIConfig, pMSPITestConfig->ui32DMACtrlBufferSize, pMSPITestConfig->pDMACtrlBuffer);

  g_MSPIInterruptStatus = 0;

  return ui32Status;
}

//*****************************************************************************
//
// Widget Cleanup Function.
//
//*****************************************************************************
uint32_t am_widget_mspi_octal_cleanup(void *pWidget, char *pErrStr)
{
  // Clean up the MSPI configuraton for the N25Q256A part.
  return am_devices_atxp032_deinit(&g_MSPITestConfig.MSPIConfig);
}

//*****************************************************************************
//
// Widget Test Execution Functions
//
//*****************************************************************************

// Just check that we can read the Device ID correctly.
bool am_widget_mspi_octal_test_get_eflash_id(void *pWidget, char *pErrStr)
{
  uint32_t      id = 0;
  uint32_t      status;

  status = am_devices_atxp032_id(&id);

  if (AM_DEVICES_ATXP032_STATUS_SUCCESS != status)
  {
    return false;
  }

  if ((id & 0x00FFFFFF) == 0x0019BA20)
  {
    return true;
  }
  else
  {
    return false;
  }

}

// Write/Read test.
bool am_widget_mspi_octal_test_write_read(void *pWidget, void *pTestCfg, char *pErrStr)
{
  uint32_t      ui32Status;

  //
  // Create a pointer to the MSPI widget state configuration.
  //
  am_widget_mspi_test_t        *pTestConfig = (am_widget_mspi_test_t *)pTestCfg;

  // Generate data into the Sector Buffer
  for (uint32_t i = 0; i < pTestConfig->NumBytes; i++)
  {
    g_SectorTXBuffer[i] = (i & 0xFF);
  }


  // Erase the target sector.
  am_util_stdio_printf("Erasing Sector %8X\n",pTestConfig->SectorAddress);
  ui32Status = am_devices_atxp032_sector_erase(pTestConfig->SectorAddress);
  if (AM_DEVICES_ATXP032_STATUS_SUCCESS != ui32Status)
  {
    am_util_stdio_printf("Failed to erase the target sector.  Status = %d\n",ui32Status);
    return false;
  }


  // Write the TX buffer into the target sector.
  am_util_stdio_printf("Writing %d Bytes to Sector %8X\n",pTestConfig->NumBytes,
                       pTestConfig->SectorAddress+pTestConfig->SectorOffset);
  ui32Status = am_devices_atxp032_write(&g_SectorTXBuffer[pTestConfig->ByteOffset],
                                         pTestConfig->SectorAddress+pTestConfig->SectorOffset,
                                         pTestConfig->NumBytes);
  if (AM_DEVICES_ATXP032_STATUS_SUCCESS != ui32Status)
  {
    am_util_stdio_printf("Failed to write to target sector.  Status = %d\n",ui32Status);
    return false;
  }

  // Read the data back into the RX buffer.
  am_util_stdio_printf("Read %d Bytes from Sector %8X\n",pTestConfig->NumBytes,
                       pTestConfig->SectorAddress+pTestConfig->SectorOffset);
  ui32Status = am_devices_atxp032_read(&g_SectorRXBuffer[pTestConfig->ByteOffset],
                           pTestConfig->SectorAddress+pTestConfig->SectorOffset,
                           pTestConfig->NumBytes, true);
  if (AM_DEVICES_ATXP032_STATUS_SUCCESS != ui32Status)
  {
    am_util_stdio_printf("Failed to read from target sector.  Status = %d\n",ui32Status);
    return false;
  }


  // Compare the buffers
  am_util_stdio_printf("Comparing the TX and RX Buffers\n");
  for (uint32_t i = 0; i < pTestConfig->NumBytes; i++)
  {
    if (g_SectorRXBuffer[i] != g_SectorTXBuffer[i])
    {
      return false;
    }
  }

  return true;
}

// How many reads to split the Readback into
#define AM_WIDGET_MSPI_MAX_QUEUE_SIZE    16
// Write/Read test.
bool am_widget_mspi_octal_test_write_queue_read(void *pWidget, void *pTestCfg, char *pErrStr)
{
  uint32_t      ui32Status;

  //
  // Create a pointer to the MSPI widget state configuration.
  //
  am_widget_mspi_test_t          *pTestConfig = (am_widget_mspi_test_t *)pTestCfg;
  uint32_t                       readBlockSize;
  uint32_t                       numBytesToRead;


  // Generate data into the Sector Buffer
  for (uint32_t i = 0; i < pTestConfig->NumBytes; i++)
  {
    g_SectorTXBuffer[pTestConfig->ByteOffset + i] = (i & 0xFF);
  }


  // Erase the target sector.
  am_util_stdio_printf("Erasing Sector %8X\n",pTestConfig->SectorAddress);
  ui32Status = am_devices_atxp032_sector_erase(pTestConfig->SectorAddress);
  if (AM_DEVICES_ATXP032_STATUS_SUCCESS != ui32Status)
  {
    am_util_stdio_printf("Failed to erase the target sector.  Status = %d\n",ui32Status);
    return false;
  }

  // Write the TX buffer into the target sector.
  am_util_stdio_printf("Writing %d Bytes to Sector %8X\n",pTestConfig->NumBytes,
                       pTestConfig->SectorAddress+pTestConfig->SectorOffset);
  ui32Status = am_devices_atxp032_write(&g_SectorTXBuffer[pTestConfig->ByteOffset],
                                         pTestConfig->SectorAddress+pTestConfig->SectorOffset,
                                         pTestConfig->NumBytes);
  if (AM_DEVICES_ATXP032_STATUS_SUCCESS != ui32Status)
  {
    am_util_stdio_printf("Failed to write to target sector.  Status = %d\n",ui32Status);
    return false;
  }

  // Read the data back into the RX buffer.
  am_util_stdio_printf("Read %d Bytes as a single block from Sector %8X\n",pTestConfig->NumBytes,
                       pTestConfig->SectorAddress+pTestConfig->SectorOffset);
  ui32Status = am_devices_atxp032_read(&g_SectorRXBuffer[pTestConfig->ByteOffset],
                           pTestConfig->SectorAddress+pTestConfig->SectorOffset,
                           pTestConfig->NumBytes, true);
  if (AM_DEVICES_ATXP032_STATUS_SUCCESS != ui32Status)
  {
    am_util_stdio_printf("Failed to read from target sector.  Status = %d\n",ui32Status);
    return false;
  }

  // Compare the buffers
  am_util_stdio_printf("Comparing the TX and RX Buffers\n");
  for (uint32_t i = 0; i < pTestConfig->NumBytes; i++)
  {
    if (g_SectorRXBuffer[i] != g_SectorTXBuffer[i])
    {
      am_util_stdio_printf("Comparison failed at Index %2d with Received Value %2.2X while expected value was %2.2X\n",i,g_SectorRXBuffer[i],g_SectorTXBuffer[i]);
      return false;
    }
  }

  AM_UPDATE_TEST_PROGRESS();

  numBytesToRead = pTestConfig->NumBytes;
  for (uint32_t i = AM_WIDGET_MSPI_MAX_QUEUE_SIZE - 1; i > 0; i--)
  {
    readBlockSize = numBytesToRead / i;
    if (readBlockSize > 0)
    {
        break;
    }
  }
  if (readBlockSize == 0)
  {
    readBlockSize = numBytesToRead;
  }

  // Read the data back into the RX buffer.
  am_util_stdio_printf("Read %d Bytes using smaller blocks of %d bytes each and queued transactions from Sector %8X\n",
  pTestConfig->NumBytes,
  readBlockSize,
  pTestConfig->SectorAddress+pTestConfig->SectorOffset);

  while (numBytesToRead)
  {
      uint32_t num = (numBytesToRead > readBlockSize) ? readBlockSize : numBytesToRead;
      ui32Status = am_devices_atxp032_read(&g_SectorRXBuffer[pTestConfig->ByteOffset + pTestConfig->NumBytes - numBytesToRead],
                               pTestConfig->SectorAddress+pTestConfig->SectorOffset + pTestConfig->NumBytes - numBytesToRead,
                               num,
                               (numBytesToRead > readBlockSize) ? false : true);
      if (AM_DEVICES_ATXP032_STATUS_SUCCESS != ui32Status)
      {
        am_util_stdio_printf("Failed to read from target sector.  Status = %d\n",ui32Status);
        return false;
      }
      numBytesToRead -= num;
  }

  // Compare the buffers
  am_util_stdio_printf("Comparing the TX and RX Buffers\n");
  for (uint32_t i = 0; i < pTestConfig->NumBytes; i++)
  {
    if (g_SectorRXBuffer[i] != g_SectorTXBuffer[i])
    {
      am_util_stdio_printf("Comparison failed at Index %2d with Received Value %2.2X while expected value was %2.2X\n",i,g_SectorRXBuffer[i],g_SectorTXBuffer[i]);
      return false;
    }
  }

  return true;
}

// XIP Data bus read test.
bool am_widget_mspi_octal_test_xip_databus(void *pWidget, void *pTestCfg, char *pErrStr)
{

  //
  // Create a pointer to the MSPI widget state configuration.
  //
  am_widget_mspi_test_t        *pTestConfig = (am_widget_mspi_test_t *)pTestCfg;

  // Generate data into the Sector Buffer
  for (uint32_t i = 0; i < pTestConfig->NumBytes; i++)
  {
    g_SectorTXBuffer[i] = (i & 0xFF);
  }

  // Erase the target sector.
  am_util_stdio_printf("Erasing Sector %8X\n",pTestConfig->SectorAddress);
  am_devices_atxp032_sector_erase(pTestConfig->SectorAddress);

  // Write the TX buffer into the target sector.
  am_util_stdio_printf("Writing %d Bytes to Sector %8X\n",pTestConfig->NumBytes,pTestConfig->SectorAddress);
  am_devices_atxp032_write(g_SectorTXBuffer,pTestConfig->SectorAddress, pTestConfig->NumBytes);

  // Set up for XIP operation.
  am_util_stdio_printf("Putting the MSPI and External Flash into XIP mode\n");
  am_devices_atxp032_enable(true,false);

  if (pTestConfig->TurnOnCaching)
  {
    // Turn on flash caching so we don't see a fetch for every read.
  CACHECTRL->CACHECFG =
    _VAL2FLD(CACHECTRL_CACHECFG_CONFIG, CACHECTRL_CACHECFG_CONFIG_W2_128B_512E) |
    CACHECTRL_CACHECFG_ENABLE_Msk                                               |
    CACHECTRL_CACHECFG_ICACHE_ENABLE_Msk                                        |
    CACHECTRL_CACHECFG_DCACHE_ENABLE_Msk;
  }
  else
  {
    CACHECTRL->CACHECFG = 0;
  }

  // Read and check the data by comparing it to the base structure (byte-wise)
  for (uint32_t i = 0; i < pTestConfig->NumBytes; i++)
  {
    ui8Byte = *(uint8_t *)(MSPI_XIP_BASE_ADDRESS + pTestConfig->SectorAddress + i);
    if (g_SectorTXBuffer[i] != ui8Byte)
    {
      am_util_stdio_printf("Byte Aligned Data Bus compare with External Flash failed at Address %8X\n",
                           (MSPI_XIP_BASE_ADDRESS + pTestConfig->SectorAddress + i));
      return false;
    }
  }

  // Read and check the data by comparing it to the base structure (word-wise)
  for (uint32_t i = 0; i < pTestConfig->NumBytes; i += 4)
  {
    ui32Word = *(uint32_t *)(MSPI_XIP_BASE_ADDRESS + pTestConfig->SectorAddress + i);
    ui32CmpWord = *(uint32_t *)&g_SectorTXBuffer[i];
    if (ui32CmpWord != ui32Word)
    {
      am_util_stdio_printf("Word Aligned Data Bus compare with External Flash failed at Address %8X\n",
                           (MSPI_XIP_BASE_ADDRESS + pTestConfig->SectorAddress + i));
      return false;
    }
  }

  // Shutdown XIP operation.
  am_util_stdio_printf("Disabling the MSPI and External Flash from XIP mode\n");
  am_devices_atxp032_disable_xip();

  return true;
}

//*****************************************************************************
//
// Static function to be executed from external flash device
//
//*****************************************************************************
#if defined(__GNUC_STDC_INLINE__)
__attribute__((naked))
static void xip_test_function(void)
{
    __asm
    (
        "   nop\n"              // Just execute NOPs and return.
        "   nop\n"
        "   nop\n"
        "   nop\n"
        "   nop\n"
        "   nop\n"
        "   nop\n"
        "   nop\n"
        "   nop\n"
        "   nop\n"
        "   nop\n"
        "   nop\n"
        "   nop\n"
        "   nop\n"
        "   nop\n"
        "   nop\n"
        "   nop\n"
        "   nop\n"
        "   nop\n"
        "   nop\n"
        "   nop\n"
        "   nop\n"
        "   nop\n"
        "   nop\n"
        "   nop\n"
        "   nop\n"
        "   nop\n"
        "   nop\n"
        "   nop\n"
        "   nop\n"
        "   nop\n"
        "   nop\n"
        "   bx      lr\n"
    );
}

#elif defined(__ARMCC_VERSION)
__asm static void xip_test_function(void)
{
    nop                         // Just execute NOPs and return.
    nop
    nop
    nop
    nop
    nop
    nop
    nop
    nop
    nop
    nop
    nop
    nop
    nop
    nop
    nop
    nop
    nop
    nop
    nop
    nop
    nop
    nop
    nop
    nop
    nop
    nop
    nop
    nop
    nop
    nop
    nop
    bx      lr
}

#elif defined(__IAR_SYSTEMS_ICC__)
__stackless static void xip_test_function(void)
{
    __asm("    nop");           // Just execute NOPs and return.
    __asm("    nop");
    __asm("    nop");
    __asm("    nop");
    __asm("    nop");
    __asm("    nop");
    __asm("    nop");
    __asm("    nop");
    __asm("    nop");
    __asm("    nop");
    __asm("    nop");
    __asm("    nop");
    __asm("    nop");
    __asm("    nop");
    __asm("    nop");
    __asm("    nop");
    __asm("    nop");
    __asm("    nop");
    __asm("    nop");
    __asm("    nop");
    __asm("    nop");
    __asm("    nop");
    __asm("    nop");
    __asm("    nop");
    __asm("    nop");
    __asm("    nop");
    __asm("    nop");
    __asm("    nop");
    __asm("    nop");
    __asm("    nop");
    __asm("    nop");
    __asm("    nop");
    __asm("    bx      lr");
}
#endif

#define MSPI_XIP_FUNCTION_SIZE  72
typedef uint32_t (*mspi_xip_test_function_t)(uint32_t, uint32_t, uint32_t);

typedef struct
{
    uint32_t binAddr;
    uint32_t funcOffset;
    uint32_t binSize;
    uint32_t param0;
    uint32_t param1;
    uint32_t param2;
    uint32_t result;
} mspi_xip_test_funcinfo_t;

// Patchable section of binary
extern uint32_t __Patchable[];

// Configurable option to load a position independent library instead
void get_test_function(mspi_xip_test_funcinfo_t *pFuncInfo)
{
    // some of the parameters are controllable through binary patching
    if (__Patchable[0])
    {
        pFuncInfo->binAddr = __Patchable[0];
        pFuncInfo->funcOffset = __Patchable[1];
        pFuncInfo->binSize = __Patchable[2];
        pFuncInfo->param0 = __Patchable[3];
        pFuncInfo->param1 = __Patchable[4];
        pFuncInfo->param2 = __Patchable[5];
        pFuncInfo->result = __Patchable[6];
    }
    else
    {
        pFuncInfo->binAddr = ((uint32_t)&xip_test_function) & 0xFFFFFFFE;
        pFuncInfo->funcOffset = 0;
        pFuncInfo->binSize = MSPI_XIP_FUNCTION_SIZE;
        pFuncInfo->param0 = 0xDEADBEEF;
        pFuncInfo->result = 0xDEADBEEF;
    }
}

// XIP Instruction bus execute test.
bool am_widget_mspi_octal_test_xip_instrbus(void *pWidget, void *pTestCfg, char *pErrStr)
{
  uint32_t      ui32Status;
  mspi_xip_test_funcinfo_t  funcInfo;
  uint32_t                  result;
  //
  // Create a pointer to the MSPI widget state configuration.
  //
  am_widget_mspi_test_t        *pTestConfig = (am_widget_mspi_test_t *)pTestCfg;

  get_test_function(&funcInfo);
  //
  // Cast a pointer to the begining of the sector as the test function to call.
  //
  mspi_xip_test_function_t test_function = (mspi_xip_test_function_t)((MSPI_XIP_BASE_ADDRESS + pTestConfig->SectorAddress + funcInfo.funcOffset) | 0x00000001);

  //
  // Erase the target sector.
  //
  am_util_stdio_printf("Erasing Sector %8X\n",pTestConfig->SectorAddress);
  ui32Status = am_devices_atxp032_sector_erase(pTestConfig->SectorAddress);
  if (AM_DEVICES_ATXP032_STATUS_SUCCESS != ui32Status)
  {
    am_util_stdio_printf("Failed to erase the target sector.  Status = %d\n",ui32Status);
    return false;
  }

  //
  // Write the TX buffer into the target sector.
  //
  am_util_stdio_printf("Writing %d Bytes to Sector %8X\n",funcInfo.binSize,pTestConfig->SectorAddress);
  ui32Status = am_devices_atxp032_write((uint8_t *)funcInfo.binAddr,pTestConfig->SectorAddress, funcInfo.binSize);
  if (AM_DEVICES_ATXP032_STATUS_SUCCESS != ui32Status)
  {
    am_util_stdio_printf("Failed to write to target sector.  Status = %d\n",ui32Status);
    return false;
  }

  //
  // Set up for XIP operation.
  //
  am_util_stdio_printf("Putting the MSPI and External Flash into XIP mode\n");
  ui32Status = am_devices_atxp032_enable(true,false);
  if (AM_DEVICES_ATXP032_STATUS_SUCCESS != ui32Status)
  {
    return false;
  }

  //
  // Execute a call to the test function in the sector.
  //
  result = test_function(funcInfo.param0, funcInfo.param1, funcInfo.param2);
  if (result != funcInfo.result)
  {
      am_util_stdio_printf("XIP function returned wrong result - Expected 0x%x, Actual 0x%x\n", funcInfo.result, result);
      am_devices_atxp032_disable_xip();
      return false;
  }

  // Shutdown XIP operation.
  am_util_stdio_printf("Disabling the MSPI and External Flash from XIP mode\n");
  ui32Status = am_devices_atxp032_disable_xip();
  if (AM_DEVICES_ATXP032_STATUS_SUCCESS != ui32Status)
  {
    return false;
  }

  return true;
}

// Scrambling test.
bool am_widget_mspi_octal_test_scrambling(void *pWidget, void *pTestCfg, char *pErrStr)
{
  uint32_t      ui32MiscompareCount;
  uint32_t      ui32Status;

  //
  // Create a pointer to the MSPI widget state configuration.
  //
  am_widget_mspi_test_t        *pTestConfig = (am_widget_mspi_test_t *)pTestCfg;
  am_widget_mspi_config_t        *pWidgetConfig = (am_widget_mspi_config_t *)pWidget;


  // Generate data into the Sector Buffer
  for (uint32_t i = 0; i < pTestConfig->NumBytes; i++)
  {
    g_SectorTXBuffer[i] = (i & 0xFF);
  }

  // Turn on scrambling.
  ui32Status = am_devices_atxp032_enable(false,true);
  if (AM_DEVICES_ATXP032_STATUS_SUCCESS != ui32Status)
  {
    am_util_stdio_printf("Failed to enable scrambling. Status = %d\n",ui32Status);
    return false;
  }

  // Erase the 1st scrambling region sector.
  am_util_stdio_printf("Erasing Sector %8X\n",pWidgetConfig->MSPIConfig.scramblingStartAddr);
  ui32Status = am_devices_atxp032_sector_erase(pWidgetConfig->MSPIConfig.scramblingStartAddr);
  if (AM_DEVICES_ATXP032_STATUS_SUCCESS != ui32Status)
  {
    am_util_stdio_printf("Failed to erase the target sector.  Status = %d\n",ui32Status);
    return false;
  }

  // Write the TX buffer into the target sector.
  am_util_stdio_printf("Writing %d Bytes to Sector %8X\n",pTestConfig->NumBytes,pWidgetConfig->MSPIConfig.scramblingStartAddr);
  ui32Status = am_devices_atxp032_write(g_SectorTXBuffer,pWidgetConfig->MSPIConfig.scramblingStartAddr, pTestConfig->NumBytes);
  if (AM_DEVICES_ATXP032_STATUS_SUCCESS != ui32Status)
  {
    am_util_stdio_printf("Failed to write to target sector.  Status = %d\n",ui32Status);
    return false;
  }

  // Read the data back into the RX buffer.
  am_util_stdio_printf("Read %d Bytes from Sector %8X\n",pTestConfig->NumBytes,pWidgetConfig->MSPIConfig.scramblingStartAddr);
  ui32Status = am_devices_atxp032_read(g_SectorRXBuffer,pWidgetConfig->MSPIConfig.scramblingStartAddr, pTestConfig->NumBytes, true);
  if (AM_DEVICES_ATXP032_STATUS_SUCCESS != ui32Status)
  {
    return false;
  }

  // Compare the buffers
  am_util_stdio_printf("Comparing the TX and RX Buffers\n");
  for (uint32_t i = 0; i < pTestConfig->NumBytes; i++)
  {
    if (g_SectorRXBuffer[i] != g_SectorTXBuffer[i])
    {
      am_util_stdio_printf("Comparison failed at Index %2d with Received Value %2.2X while expected value was %2.2X\n",i,g_SectorRXBuffer[i],g_SectorTXBuffer[i]);
      return false;
    }
  }

  AM_UPDATE_TEST_PROGRESS();

  // Erase the unscrambled test sector.
  am_util_stdio_printf("Erasing Sector %8X\n",pTestConfig->UnscrambledSector);
  ui32Status = am_devices_atxp032_sector_erase(pTestConfig->UnscrambledSector);
  if (AM_DEVICES_ATXP032_STATUS_SUCCESS != ui32Status)
  {
    am_util_stdio_printf("Failed to erase the target sector.  Status = %d\n",ui32Status);
    return false;
  }

  // Write the TX buffer into the target sector.
  am_util_stdio_printf("Writing %d Bytes to Sector %8X\n",pTestConfig->NumBytes,pTestConfig->UnscrambledSector);
  ui32Status = am_devices_atxp032_write(g_SectorTXBuffer,pTestConfig->UnscrambledSector, pTestConfig->NumBytes);
  if (AM_DEVICES_ATXP032_STATUS_SUCCESS != ui32Status)
  {
    am_util_stdio_printf("Failed to write to target sector.  Status = %d\n",ui32Status);
    return false;
  }

  // Read the data back into the RX buffer.
  am_util_stdio_printf("Read %d Bytes from Sector %8X\n",pTestConfig->NumBytes,pTestConfig->UnscrambledSector);
  ui32Status = am_devices_atxp032_read(g_SectorRXBuffer,pTestConfig->UnscrambledSector, pTestConfig->NumBytes, true);
  if (AM_DEVICES_ATXP032_STATUS_SUCCESS != ui32Status)
  {
    am_util_stdio_printf("Failed to read from target sector.  Status = %d\n",ui32Status);
    return false;
  }

  // Compare the buffers
  am_util_stdio_printf("Comparing the TX and RX Buffers\n");
  for (uint32_t i = 0; i < pTestConfig->NumBytes; i++)
  {
    if (g_SectorRXBuffer[i] != g_SectorTXBuffer[i])
    {
      am_util_stdio_printf("Comparison failed at Index %2d with Received Value %2.2X while expected value was %2.2X\n",i,g_SectorRXBuffer[i],g_SectorTXBuffer[i]);
      return false;
    }
  }

  AM_UPDATE_TEST_PROGRESS();

  // Turn off XIP.
  ui32Status = am_devices_atxp032_disable_xip();
  if (AM_DEVICES_ATXP032_STATUS_SUCCESS != ui32Status)
  {
    am_util_stdio_printf("Failed to disable XIP. Status = %d\n",ui32Status);
    return false;
  }

  // Read the scrambled data back into the RX buffer.
  am_util_stdio_printf("Read %d Bytes from Sector %8X\n",pTestConfig->NumBytes,pWidgetConfig->MSPIConfig.scramblingStartAddr);
  ui32Status = am_devices_atxp032_read(g_SectorRXBuffer,pWidgetConfig->MSPIConfig.scramblingStartAddr, pTestConfig->NumBytes, true);
  if (AM_DEVICES_ATXP032_STATUS_SUCCESS != ui32Status)
  {
    am_util_stdio_printf("Failed to read from target sector.  Status = %d\n",ui32Status);
    return false;
  }

  // Compare the buffers
  am_util_stdio_printf("Comparing the TX and RX Buffers\n");
  ui32MiscompareCount = 0;
  for (uint32_t i = 0; i < pTestConfig->NumBytes; i++)
  {
    if (g_SectorRXBuffer[i] != g_SectorTXBuffer[i])
    {
      ui32MiscompareCount++;
    }
  }
  if (ui32MiscompareCount <= (0.9 * pTestConfig->NumBytes))
  {
    am_util_stdio_printf("Scrambled data too closely compared to test data.  %d of %d bytes were the same!\n",ui32MiscompareCount,pTestConfig->NumBytes);
    return false;
  }

  AM_UPDATE_TEST_PROGRESS();

  // Read the unscrambled data back into the RX buffer.
  am_util_stdio_printf("Read %d Bytes from Sector %8X\n",pTestConfig->NumBytes,pTestConfig->UnscrambledSector);
  ui32Status = am_devices_atxp032_read(g_SectorRXBuffer,pTestConfig->UnscrambledSector, pTestConfig->NumBytes, true);
  if (AM_DEVICES_ATXP032_STATUS_SUCCESS != ui32Status)
  {
    am_util_stdio_printf("Failed to read from target sector.  Status = %d\n",ui32Status);
    return false;
  }

  // Compare the buffers
  am_util_stdio_printf("Comparing the TX and RX Buffers\n");
  for (uint32_t i = 0; i < pTestConfig->NumBytes; i++)
  {
    if (g_SectorRXBuffer[i] != g_SectorTXBuffer[i])
    {
      am_util_stdio_printf("Comparison failed at Index %2d with Received Value %2.2X while expected value was %2.2X\n",i,g_SectorRXBuffer[i],g_SectorTXBuffer[i]);
      return false;
    }
  }

  AM_UPDATE_TEST_PROGRESS();

  // Turn on scrambling.
  ui32Status = am_devices_atxp032_enable(false,true);
  if (AM_DEVICES_ATXP032_STATUS_SUCCESS != ui32Status)
  {
    return false;
  }

  // Erase the 1st scrambling region sector.
  am_util_stdio_printf("Erasing Sector %8X\n",pWidgetConfig->MSPIConfig.scramblingStartAddr);
  ui32Status = am_devices_atxp032_sector_erase(pWidgetConfig->MSPIConfig.scramblingStartAddr);
  if (AM_DEVICES_ATXP032_STATUS_SUCCESS != ui32Status)
  {
    am_util_stdio_printf("Failed to erase the target sector.  Status = %d\n",ui32Status);
    return false;
  }

  // Write the TX buffer into the target sector.
  am_util_stdio_printf("Writing %d Bytes to Sector %8X\n",pTestConfig->NumBytes,pWidgetConfig->MSPIConfig.scramblingStartAddr);
  ui32Status = am_devices_atxp032_write(g_SectorTXBuffer,pWidgetConfig->MSPIConfig.scramblingStartAddr, pTestConfig->NumBytes);
  if (AM_DEVICES_ATXP032_STATUS_SUCCESS != ui32Status)
  {
    am_util_stdio_printf("Failed to write to target sector.  Status = %d\n",ui32Status);
    return false;
  }

  AM_UPDATE_TEST_PROGRESS();

  // Erase the last scrambling region sector.
  am_util_stdio_printf("Erasing Sector %8X\n",pWidgetConfig->MSPIConfig.scramblingEndAddr);
  ui32Status = am_devices_atxp032_sector_erase(pWidgetConfig->MSPIConfig.scramblingEndAddr);
  if (AM_DEVICES_ATXP032_STATUS_SUCCESS != ui32Status)
  {
    am_util_stdio_printf("Failed to erase the target sector.  Status = %d\n",ui32Status);
    return false;
  }

  // Write the TX buffer into the target sector.
  am_util_stdio_printf("Writing %d Bytes to Sector %8X\n",pTestConfig->NumBytes,pWidgetConfig->MSPIConfig.scramblingEndAddr);
  ui32Status = am_devices_atxp032_write(g_SectorTXBuffer,pWidgetConfig->MSPIConfig.scramblingEndAddr, pTestConfig->NumBytes);
  if (AM_DEVICES_ATXP032_STATUS_SUCCESS != ui32Status)
  {
    am_util_stdio_printf("Failed to write to target sector.  Status = %d\n",ui32Status);
    return false;
  }

  // Turn off scrambling.
  ui32Status = am_devices_atxp032_disable_xip();
  if (AM_DEVICES_ATXP032_STATUS_SUCCESS != ui32Status)
  {
    return false;
  }


  AM_UPDATE_TEST_PROGRESS();

  // Read the data back into the RX buffer.
  am_util_stdio_printf("Read %d Bytes from Sector %8X\n",pTestConfig->NumBytes,pWidgetConfig->MSPIConfig.scramblingStartAddr);
  ui32Status = am_devices_atxp032_read(g_SectorRXBuffer,pWidgetConfig->MSPIConfig.scramblingStartAddr, pTestConfig->NumBytes, true);
  if (AM_DEVICES_ATXP032_STATUS_SUCCESS != ui32Status)
  {
    am_util_stdio_printf("Failed to read from target sector.  Status = %d\n",ui32Status);
    return false;
  }

  // Read the data back into the RX buffer.
  am_util_stdio_printf("Read %d Bytes from Sector %8X\n",pTestConfig->NumBytes,pWidgetConfig->MSPIConfig.scramblingEndAddr);
  ui32Status = am_devices_atxp032_read(g_SectorRXBuffer2,pWidgetConfig->MSPIConfig.scramblingEndAddr, pTestConfig->NumBytes, true);
  if (AM_DEVICES_ATXP032_STATUS_SUCCESS != ui32Status)
  {
    am_util_stdio_printf("Failed to read from target sector.  Status = %d\n",ui32Status);
    return false;
  }

  ui32MiscompareCount = 0;
  for (uint32_t i = 0; i < pTestConfig->NumBytes; i++)
  {
    if (g_SectorRXBuffer[i] != g_SectorTXBuffer[i])
    {
      ui32MiscompareCount++;
    }
  }
  if (ui32MiscompareCount <= (0.9 * pTestConfig->NumBytes))
  {
    am_util_stdio_printf("Scrambled data from two different sectors too closely compared.  %d of %d bytes were the same!\n",ui32MiscompareCount,pTestConfig->NumBytes);
    return false;
  }


  return true;
}

