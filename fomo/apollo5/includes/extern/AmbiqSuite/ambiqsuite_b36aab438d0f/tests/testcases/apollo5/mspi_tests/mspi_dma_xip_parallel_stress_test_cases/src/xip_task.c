//*****************************************************************************
//
//! @file xip_task.c
//!
//! @brief Task to handle XIP operations.
//!
//! AM_DEBUG_PRINTF
//! If enabled, debug messages will be sent over ITM.
//!
//*****************************************************************************

//*****************************************************************************
//
// ${copyright}
//
// This is part of revision ${version} of the AmbiqSuite Development Package.
//
//*****************************************************************************

//*****************************************************************************
//
// Global includes for this project.
//
//*****************************************************************************
#include "mspi_dma_xip_parallel_stress_test_cases.h"

#if (XIP_TASK_ENABLE == 1)

#define MAX_XIP_TASK_DELAY    (100 * 2)

#if defined(__ICCARM__)
#error "not supported compiler"
#elif defined(__GNUC__)
  /* defined on linker script file */
    extern uint32_t _sxipcode;
    extern uint32_t _exipcode;
    extern uint32_t __xip_text_load_addr;
#endif

//*****************************************************************************
//
// XIP task handle.
//
//*****************************************************************************
TaskHandle_t XipTaskHandle;

//*****************************************************************************
//
// Global Variables
//
//*****************************************************************************
#if defined(XIP_IN_FLASH)
extern void *g_pFlashHandle;
extern mspi_flash_device_func_t mspi_flash_device_func;
#endif

//*****************************************************************************
//
// Task function.
//
//*****************************************************************************
extern void loops_all_mid_10k_sp_main(int argc, char *argv[]);

//*****************************************************************************
//
// Short Description.
//
//*****************************************************************************
#if defined(MSPI_DMA_XIP_PARALLEL_TEST)
#include "am_widget_mspi.h"
#include "mspi_test_common.h"

#define MSPI_DMA_BUFFER_LEN               500*1024

void pfnMSPI_DMA_Callback(void *pCallbackCtxt, uint32_t status)
{
  // Set the DMA complete flag.
  *(volatile bool *)pCallbackCtxt = true;
}

typedef struct
{
  uint32_t              ui32InstanceStart;
  uint32_t              ui32InstanceEnd;
  uint32_t              mspi_instances[4];

  uint32_t              ui32SpeedStart;
  uint32_t              ui32SpeedEnd;

  bool                  bDQSEnable;

  uint32_t              ui32RandomDelayMax;

  am_hal_mspi_device_e  eDeviceCfg;
  am_widget_mspi_device_model_e eDeviceModel;

  am_widget_mspi_test_t sDMATestConfig;
  am_widget_mspi_test_t sXIPTestConfig;
}mspi_test_control_t;

static mspi_test_control_t g_Test_controls =
{
  .ui32InstanceStart = 1,
  .ui32InstanceEnd   = 2,
  .mspi_instances    = {0, 3},

  .ui32SpeedStart    = 0,
  .ui32SpeedEnd      = 1,

  .ui32RandomDelayMax = 5000, //unit us, generally half of the time spent of a DMA transfer

  .eDeviceCfg        = AM_HAL_MSPI_FLASH_HEX_DDR_CE0,
  .eDeviceModel      = AM_WIDGET_MSPI_DDR_PSRAM_APS25616N,
};

const MSPIBaseAddr_t g_MSPIAddresses[AM_REG_MSPI_NUM_MODULES] =
{
  {MSPI0_APERTURE_START_ADDR, MSPI0_APERTURE_START_ADDR},
  {MSPI1_APERTURE_START_ADDR, MSPI1_APERTURE_START_ADDR},
  {MSPI2_APERTURE_START_ADDR, MSPI2_APERTURE_START_ADDR},
  {MSPI3_APERTURE_START_ADDR, MSPI3_APERTURE_START_ADDR},
};

uint8_t ui8XIPTXBuffer[AM_DEVICES_MSPI_PSRAM_PAGE_SIZE];
uint8_t ui8XIPRXBuffer[AM_DEVICES_MSPI_PSRAM_PAGE_SIZE];

AM_SHARED_RW uint8_t        ui8DMATXBuffer[MSPI_DMA_BUFFER_LEN];      // 500K SRAM TX buffer
AM_SHARED_RW uint8_t        ui8DMARXBuffer[MSPI_DMA_BUFFER_LEN];      // 500K SRAM RX buffer.

extern void *g_pPsramXipCodeHandle;
extern mspi_psram_device_func_t mspi_psram_xip_device_func;
char          errStr[128];

//*****************************************************************************
//
// octal copy implement
//
//*****************************************************************************

#if (defined (__ARMCC_VERSION)) && (__ARMCC_VERSION <  6000000)
__asm static void
octal_copy( uint32_t srcAddr, uint32_t destAddr, uint32_t numOctal)
{
  push    {r3-r10}              // Save r3-r10 - used by this function
__octal_copy_loop
  ldmia r0!, {r3-r10}           // Load 8 words to registers
  stmia r1!, {r3-r10}           // Store 8 words from registers
  subs    r2, r2, #1
  bne     __octal_copy_loop
  pop     {r3-r10}              // Restore registers
  bx      lr
}
#elif (defined (__ARMCC_VERSION)) && (__ARMCC_VERSION >= 6000000)
__attribute__((naked))
static void
octal_copy( uint32_t srcAddr, uint32_t destAddr, uint32_t numOctal)
{
__asm
    (
     "    push    {r3-r10}\n\t"              // Save r3-r10 - used by this function
     "__octal_copy_loop:\n\t"
     "    ldmia r0!, {r3-r10}\n\t"           // Load 8 words to registers
     "    stmia r1!, {r3-r10}\n\t"           // Store 8 words from registers
     "    subs    r2, r2, #1\n\t"
     "    bne     __octal_copy_loop\n\t"
     "    pop     {r3-r10}\n\t"              // Restore registers
     "    bx      lr\n\t"
    );
}
#elif defined(__GNUC_STDC_INLINE__)
__attribute__((naked))
static void
octal_copy( uint32_t srcAddr, uint32_t destAddr, uint32_t numOctal)
{
__asm
    (
     "    push    {r3-r10}\n\t"              // Save r3-r10 - used by this function
     "__octal_copy_loop:\n\t"
     "    ldmia r0!, {r3-r10}\n\t"           // Load 8 words to registers
     "    stmia r1!, {r3-r10}\n\t"           // Store 8 words from registers
     "    subs    r2, r2, #1\n\t"
     "    bne     __octal_copy_loop\n\t"
     "    pop     {r3-r10}\n\t"              // Restore registers
     "    bx      lr\n\t"
    );
}
#elif defined(__IAR_SYSTEMS_ICC__)
__stackless static void
octal_copy( uint32_t srcAddr, uint32_t destAddr, uint32_t numOctal)
{
__asm volatile (
                "    push    {r3-r10}\n"              // Save r3-r10 - used by this function
                "__octal_copy_loop:\n"
                "    ldmia r0!, {r3-r10}\n"           // Load 8 words to registers
                "    stmia r1!, {r3-r10}\n"           // Store 8 words from registers
                "    subs    r2, r2, #1\n"
                "    bne     __octal_copy_loop\n"
                "    pop     {r3-r10}\n"              // Restore registers
                "    bx      lr\n"
               );
}
#else
#error "Compiler Not supported"
#endif


static bool mspi_check_buffer(uint32_t ui32NumBytes, uint8_t *pRxBuffer, uint8_t *pTxBuffer)
{
  for (uint32_t i = 0; i < ui32NumBytes; i++)
  {
    if (pRxBuffer[i] != pTxBuffer[i])
    {
      am_util_stdio_printf("Comparison failed at Index %2d with Received Value %2.2X while expected value was %2.2X\n", i, pRxBuffer[i], pTxBuffer[i]);
      return false;
    }
  }
  return true;
}

bool
mspi_xipmm(void *pTestCfg, char *pErrStr)
{
  uint32_t ix;
  uint32_t *pu32Ptr1, *pu32Ptr2;
  uint32_t u32Val1, u32Val2;
  uint16_t *pu16Ptr1, *pu16Ptr2;
  uint16_t u16Val1, u16Val2;
  uint8_t *pu8Ptr1, *pu8Ptr2;
  uint8_t u8Val1, u8Val2;

  uint32_t      ui32Status;

  //
  // Create a pointer to the MSPI Widget configuration.
  //
  //am_widget_mspi_config_t        *pWidgetConfig = (am_widget_mspi_config_t *)pWidget;

  //
  // Create a pointer to the MSPI widget state configuration.
  //
  am_widget_mspi_test_t          *pTestConfig = (am_widget_mspi_test_t *)pTestCfg;

  ui32Status = mspi_psram_xip_device_func.mspi_xip_enable(g_pPsramXipCodeHandle);
  if (AM_WIDGET_MSPI_SUCCESS != ui32Status)
  {
    am_util_stdio_printf("Failed to enable XIP mode in the MSPI!\n");
    return false;
  }

  switch (pTestConfig->eXIPMMAccess)
  {
  case MSPI_XIPMM_MEMCPY_ACCESS:
    {
      if(pTestConfig->pTxBuffer == NULL || pTestConfig->pRxBuffer == NULL)
      {
        return false;
      }

      uint8_t *pu8TxPtr = (uint8_t *)(pTestConfig->pTxBuffer);
      for(ix = 0; ix < pTestConfig->NumBytes; ix++)
      {
        *pu8TxPtr++ = ix;
      }

      pu32Ptr1 = (uint32_t *)(g_MSPIAddresses[MSPI_PSRAM_XIPCODE_MODULE].XIPMMBase + pTestConfig->ByteOffset);

	    memset(pu32Ptr1, 0, pTestConfig->NumBytes);
      memcpy(pu32Ptr1, pTestConfig->pTxBuffer, pTestConfig->NumBytes);
      memcpy(pTestConfig->pRxBuffer, pu32Ptr1, pTestConfig->NumBytes);

      if(!mspi_check_buffer(pTestConfig->NumBytes, pTestConfig->pRxBuffer, pTestConfig->pTxBuffer))
      {
        return false;
      }
    }
    break;

  case MSPI_XIPMM_WORD_ACCESS:
    {
      // Word Read, Word Write
      pu32Ptr1 = (uint32_t *)(g_MSPIAddresses[MSPI_PSRAM_XIPCODE_MODULE].XIPMMBase + pTestConfig->ByteOffset);
      pu32Ptr2 = (uint32_t *)(g_MSPIAddresses[MSPI_PSRAM_XIPCODE_MODULE].XIPMMBase + pTestConfig->ByteOffset + pTestConfig->NumBytes/2);

      // Initialize a pattern
      for (ix = 0; ix < pTestConfig->NumBytes/8; ix++)
      {
        *pu32Ptr1++ = ix;
        *pu32Ptr2++ = ix ^ 0xFFFFFFFF;
      }

      pu32Ptr1 = (uint32_t *)(g_MSPIAddresses[MSPI_PSRAM_XIPCODE_MODULE].XIPMMBase + pTestConfig->ByteOffset);
      pu32Ptr2 = (uint32_t *)(g_MSPIAddresses[MSPI_PSRAM_XIPCODE_MODULE].XIPMMBase + pTestConfig->ByteOffset + pTestConfig->NumBytes/2);

      // Verify the pattern
      for (ix = 0; ix < pTestConfig->NumBytes/8; ix++)
      {
        if ( (*pu32Ptr1++ != ix) || (*pu32Ptr2++ != (ix ^ 0xFFFFFFFF)) )
        {
          return false;
        }
      }
    }
    break;

  case MSPI_XIPMM_SHORT_ACCESS:
    {
      // Half word Write, Half word Read
      pu16Ptr1 = (uint16_t *)(g_MSPIAddresses[MSPI_PSRAM_XIPCODE_MODULE].XIPMMBase + pTestConfig->ByteOffset);
      pu16Ptr2 = (uint16_t *)(g_MSPIAddresses[MSPI_PSRAM_XIPCODE_MODULE].XIPMMBase + pTestConfig->ByteOffset + pTestConfig->NumBytes/2);

      // Initialize a pattern
      for (ix = 0; ix < pTestConfig->NumBytes/4; ix++)
      {
        *pu16Ptr1++ = (uint16_t)(ix & 0xFFFF);
        *pu16Ptr2++ = (uint16_t)((ix & 0xFFFF) ^ 0xFFFF);
      }

      pu16Ptr1 = (uint16_t *)(g_MSPIAddresses[MSPI_PSRAM_XIPCODE_MODULE].XIPMMBase + pTestConfig->ByteOffset);
      pu16Ptr2 = (uint16_t *)(g_MSPIAddresses[MSPI_PSRAM_XIPCODE_MODULE].XIPMMBase + pTestConfig->ByteOffset + pTestConfig->NumBytes/2);

      // Verify the pattern
      for (ix = 0; ix < pTestConfig->NumBytes/4; ix++)
      {
        if ( (*pu16Ptr1++ != (uint16_t)(ix & 0xFFFF)) || (*pu16Ptr2++ != (uint16_t)((ix & 0xFFFF) ^ 0xFFFF)) )
        {
          return false;
        }
      }
      // Half word Write, word Read
      pu16Ptr1 = (uint16_t *)(g_MSPIAddresses[MSPI_PSRAM_XIPCODE_MODULE].XIPMMBase + pTestConfig->ByteOffset);
      pu16Ptr2 = (uint16_t *)(g_MSPIAddresses[MSPI_PSRAM_XIPCODE_MODULE].XIPMMBase + pTestConfig->ByteOffset + pTestConfig->NumBytes/2);

      // Initialize a pattern
      for (ix = 0; ix < pTestConfig->NumBytes/8; ix++)
      {
        u32Val1 = ix;
        u32Val2 = ix ^ 0xFFFFFFFF;
        *pu16Ptr1++ = (uint16_t)(u32Val1 & 0xFFFF);
        *pu16Ptr1++ = (uint16_t)((u32Val1 >> 16) & 0xFFFF);
        *pu16Ptr2++ = (uint16_t)(u32Val2 & 0xFFFF);
        *pu16Ptr2++ = (uint16_t)((u32Val2 >> 16) & 0xFFFF);
      }
      pu32Ptr1 = (uint32_t *)(g_MSPIAddresses[MSPI_PSRAM_XIPCODE_MODULE].XIPMMBase + pTestConfig->ByteOffset);
      pu32Ptr2 = (uint32_t *)(g_MSPIAddresses[MSPI_PSRAM_XIPCODE_MODULE].XIPMMBase + pTestConfig->ByteOffset + pTestConfig->NumBytes/2);

      // Verify the pattern
      for (ix = 0; ix < pTestConfig->NumBytes/8; ix++)
      {
        if ( (*pu32Ptr1++ != ix) || (*pu32Ptr2++ != (ix ^ 0xFFFFFFFF)) )
        {
          return false;
        }
      }
      // word Write, Half word Read
      pu32Ptr1 = (uint32_t *)(g_MSPIAddresses[MSPI_PSRAM_XIPCODE_MODULE].XIPMMBase + pTestConfig->ByteOffset);
      pu32Ptr2 = (uint32_t *)(g_MSPIAddresses[MSPI_PSRAM_XIPCODE_MODULE].XIPMMBase + pTestConfig->ByteOffset + pTestConfig->NumBytes/2);

      // Initialize a pattern
      for (ix = 0; ix < pTestConfig->NumBytes/8; ix++)
      {
        *pu32Ptr1++ = ix;
        *pu32Ptr2++ = ix ^ 0xFFFFFFFF;
      }
      pu16Ptr1 = (uint16_t *)(g_MSPIAddresses[MSPI_PSRAM_XIPCODE_MODULE].XIPMMBase + pTestConfig->ByteOffset);
      pu16Ptr2 = (uint16_t *)(g_MSPIAddresses[MSPI_PSRAM_XIPCODE_MODULE].XIPMMBase + pTestConfig->ByteOffset + pTestConfig->NumBytes/2);

      // Verify the pattern
      for (ix = 0; ix < pTestConfig->NumBytes/8; ix++)
      {
        u32Val1 = *pu16Ptr1++;
        u32Val1 |= (*pu16Ptr1++ << 16);
        u32Val2 = *pu16Ptr2++;
        u32Val2 |= (*pu16Ptr2++ << 16);
        if ((u32Val1 != ix) || (u32Val2 != (ix ^ 0xFFFFFFFF)))
        {
          return false;
        }
      }

    }
    break;

  case MSPI_XIPMM_BYTE_ACCESS:
    {
      // Byte Write, Byte Read
      pu8Ptr1 = (uint8_t *)(g_MSPIAddresses[MSPI_PSRAM_XIPCODE_MODULE].XIPMMBase + pTestConfig->ByteOffset);
      pu8Ptr2 = (uint8_t *)(g_MSPIAddresses[MSPI_PSRAM_XIPCODE_MODULE].XIPMMBase + pTestConfig->ByteOffset + pTestConfig->NumBytes/2);

      // Initialize a pattern
      for (ix = 0; ix < pTestConfig->NumBytes/2; ix++)
      {
        *pu8Ptr1++ = (uint8_t)(ix & 0xFF);
        *pu8Ptr2++ = (uint8_t)((ix & 0xFF) ^ 0xFF);
      }

      pu8Ptr1 = (uint8_t *)(g_MSPIAddresses[MSPI_PSRAM_XIPCODE_MODULE].XIPMMBase + pTestConfig->ByteOffset);
      pu8Ptr2 = (uint8_t *)(g_MSPIAddresses[MSPI_PSRAM_XIPCODE_MODULE].XIPMMBase + pTestConfig->ByteOffset + pTestConfig->NumBytes/2);

      // Verify the pattern
      for (ix = 0; ix < pTestConfig->NumBytes/2; ix++)
      {
        if ( (*pu8Ptr1++ != (uint8_t)(ix & 0xFF)) || (*pu8Ptr2++ != (uint8_t)((ix & 0xFF) ^ 0xFF)) )
        {
          return false;
        }
      }
      // Byte Write, Half word Read
      pu16Ptr1 = (uint16_t *)(g_MSPIAddresses[MSPI_PSRAM_XIPCODE_MODULE].XIPMMBase + pTestConfig->ByteOffset);
      pu16Ptr2 = (uint16_t *)(g_MSPIAddresses[MSPI_PSRAM_XIPCODE_MODULE].XIPMMBase + pTestConfig->ByteOffset + pTestConfig->NumBytes/2);

      // Verify the pattern
      for (ix = 0; ix < pTestConfig->NumBytes/4; ix++)
      {
        u16Val1 = ((((2*ix + 1) & 0xFF) << 8) | ((2*ix) & 0xFF));
        u16Val2 = (((((2*ix + 1) & 0xFF) ^ 0xFF) << 8) | (((2*ix) & 0xFF) ^ 0xFF));
        if ( (*pu16Ptr1++ != u16Val1) || (*pu16Ptr2++ != u16Val2))
        {
          return false;
        }
      }
      // Byte Write, word Read
      pu32Ptr1 = (uint32_t *)(g_MSPIAddresses[MSPI_PSRAM_XIPCODE_MODULE].XIPMMBase + pTestConfig->ByteOffset);
      pu32Ptr2 = (uint32_t *)(g_MSPIAddresses[MSPI_PSRAM_XIPCODE_MODULE].XIPMMBase + pTestConfig->ByteOffset + pTestConfig->NumBytes/2);

      // Verify the pattern
      for (ix = 0; ix < pTestConfig->NumBytes/8; ix++)
      {
        u32Val1 = ((((4*ix + 3) & 0xFF) << 24) |(((4*ix + 2) & 0xFF) << 16) |(((4*ix + 1) & 0xFF) << 8) | ((4*ix) & 0xFF));
        u32Val2 = (((((4*ix + 3) & 0xFF) ^ 0xFF) << 24) |((((4*ix + 2) & 0xFF) ^ 0xFF) << 16) |((((4*ix + 1) & 0xFF) ^ 0xFF) << 8) | (((4*ix) & 0xFF) ^ 0xFF));
        if ( (*pu32Ptr1++ != u32Val1) || (*pu32Ptr2++ != u32Val2))
        {
          return false;
        }
      }
      // word Write, Byte Read
      pu32Ptr1 = (uint32_t *)(g_MSPIAddresses[MSPI_PSRAM_XIPCODE_MODULE].XIPMMBase + pTestConfig->ByteOffset);
      pu32Ptr2 = (uint32_t *)(g_MSPIAddresses[MSPI_PSRAM_XIPCODE_MODULE].XIPMMBase + pTestConfig->ByteOffset + pTestConfig->NumBytes/2);

      // Initialize a pattern
      for (ix = 0; ix < pTestConfig->NumBytes/8; ix++)
      {
        *pu32Ptr1++ = ix;
        *pu32Ptr2++ = ix ^ 0xFFFFFFFF;
      }
      pu8Ptr1 = (uint8_t *)(g_MSPIAddresses[MSPI_PSRAM_XIPCODE_MODULE].XIPMMBase + pTestConfig->ByteOffset);
      pu8Ptr2 = (uint8_t *)(g_MSPIAddresses[MSPI_PSRAM_XIPCODE_MODULE].XIPMMBase + pTestConfig->ByteOffset + pTestConfig->NumBytes/2);

      // Verify the pattern
      for (ix = 0; ix < pTestConfig->NumBytes/8; ix++)
      {
        u32Val1 = *pu8Ptr1++;
        u32Val1 |= (*pu8Ptr1++ << 8);
        u32Val1 |= (*pu8Ptr1++ << 16);
        u32Val1 |= (*pu8Ptr1++ << 24);
        u32Val2 = *pu8Ptr2++;
        u32Val2 |= (*pu8Ptr2++ << 8);
        u32Val2 |= (*pu8Ptr2++ << 16);
        u32Val2 |= (*pu8Ptr2++ << 24);
        if ((u32Val1 != ix) || (u32Val2 != (ix ^ 0xFFFFFFFF)))
        {
          return false;
        }
      }
      // Half word Write, Byte Read
      pu16Ptr1 = (uint16_t *)(g_MSPIAddresses[MSPI_PSRAM_XIPCODE_MODULE].XIPMMBase + pTestConfig->ByteOffset);
      pu16Ptr2 = (uint16_t *)(g_MSPIAddresses[MSPI_PSRAM_XIPCODE_MODULE].XIPMMBase + pTestConfig->ByteOffset + pTestConfig->NumBytes/2);

      // Initialize a pattern
      for (ix = 0; ix < pTestConfig->NumBytes/4; ix++)
      {
        *pu16Ptr1++ = (uint16_t)(ix & 0xFFFF);
        *pu16Ptr2++ = (uint16_t)((ix & 0xFFFF) ^ 0xFFFF);
      }
      pu8Ptr1 = (uint8_t *)(g_MSPIAddresses[MSPI_PSRAM_XIPCODE_MODULE].XIPMMBase + pTestConfig->ByteOffset);
      pu8Ptr2 = (uint8_t *)(g_MSPIAddresses[MSPI_PSRAM_XIPCODE_MODULE].XIPMMBase + pTestConfig->ByteOffset + pTestConfig->NumBytes/2);

      // Verify the pattern
      for (ix = 0; ix < pTestConfig->NumBytes/4; ix++)
      {
        u16Val1 = *pu8Ptr1++;
        u16Val1 |= (*pu8Ptr1++ << 8);
        u16Val2 = *pu8Ptr2++;
        u16Val2 |= (*pu8Ptr2++ << 8);
        if ( (u16Val1 != (uint16_t)(ix & 0xFFFF)) || (u16Val2 != (uint16_t)((ix & 0xFFFF) ^ 0xFFFF)) )
        {
          return false;
        }
      }
    }
    break;
  case MSPI_XIPMM_SHORT_ACCESS_UNALIGNED:
    {
      // Write aligned, read unaligned
      // Write 4 pages
      pu16Ptr1 = (uint16_t *)(g_MSPIAddresses[MSPI_PSRAM_XIPCODE_MODULE].XIPMMBase + pTestConfig->ByteOffset);
      pu16Ptr2 = (uint16_t *)(g_MSPIAddresses[MSPI_PSRAM_XIPCODE_MODULE].XIPMMBase + pTestConfig->ByteOffset + 2*pTestConfig->NumBytes);
      // Initialize a pattern
      for (ix = 0; ix < pTestConfig->NumBytes/2; ix++)
      {
        u32Val1 = ix;
        *pu16Ptr1++ = (uint16_t)(u32Val1 & 0xFFFF);
        *pu16Ptr1++ = (uint16_t)((u32Val1 >> 16) & 0xFFFF);
        u32Val2 = ix ^ 0xFFFFFFFF;
        *pu16Ptr2++ = (uint16_t)(u32Val2 & 0xFFFF);
        *pu16Ptr2++ = (uint16_t)((u32Val2 >> 16) & 0xFFFF);
      }

      pu16Ptr1 = (uint16_t *)(g_MSPIAddresses[MSPI_PSRAM_XIPCODE_MODULE].XIPMMBase + pTestConfig->ByteOffset + 1);
      pu16Ptr2 = (uint16_t *)(g_MSPIAddresses[MSPI_PSRAM_XIPCODE_MODULE].XIPMMBase + pTestConfig->ByteOffset + 1 + 2*pTestConfig->NumBytes);

      u8Val1 = 0;
      u8Val2 = 0xFF;
      // Verify the pattern
      for (ix = 0; ix < (pTestConfig->NumBytes/2); ix++)
      {
        u32Val1 = (*pu16Ptr1++) << 8 | u8Val1;
        u16Val1 = *pu16Ptr1++;
        u32Val1 |= (u16Val1 & 0xFF) << 24;
        u8Val1 = u16Val1 >> 8;
        u32Val2 = (*pu16Ptr2++) << 8 | u8Val2;
        u16Val2 = *pu16Ptr2++;
        u32Val2 |= (u16Val2 & 0xFF) << 24;
        u8Val2 = u16Val2 >> 8;

        if ((u32Val1 != ix) || (u32Val2 != (ix ^ 0xFFFFFFFF)))
        {
          return false;
        }
      }
      // Write unaligned, read aligned
      // Write 4 pages
      *((uint32_t *)(g_MSPIAddresses[MSPI_PSRAM_XIPCODE_MODULE].XIPMMBase + pTestConfig->ByteOffset)) = 0;
      *((uint32_t *)(g_MSPIAddresses[MSPI_PSRAM_XIPCODE_MODULE].XIPMMBase + pTestConfig->ByteOffset + 4)) = 0;
      pu16Ptr1 = (uint16_t *)(g_MSPIAddresses[MSPI_PSRAM_XIPCODE_MODULE].XIPMMBase + pTestConfig->ByteOffset + 1);
      u16Val1 = 0xAABB;
      am_util_debug_printf("Initial Value 0x%08x at address 0x%08x\n", *((uint32_t *)(g_MSPIAddresses[MSPI_PSRAM_XIPCODE_MODULE].XIPMMBase + pTestConfig->ByteOffset)), g_MSPIAddresses[MSPI_PSRAM_XIPCODE_MODULE].XIPMMBase + pTestConfig->ByteOffset);
      am_util_debug_printf("Initial Value 0x%08x at address 0x%08x\n", *((uint32_t *)(g_MSPIAddresses[MSPI_PSRAM_XIPCODE_MODULE].XIPMMBase + pTestConfig->ByteOffset + 4)), g_MSPIAddresses[MSPI_PSRAM_XIPCODE_MODULE].XIPMMBase + pTestConfig->ByteOffset + 4);
      am_util_debug_printf("Writing Half Word 0x%04x to address 0x%08x\n", u16Val1, pu16Ptr1);
      *pu16Ptr1 = u16Val1;
      am_util_debug_printf("Final Value 0x%08x at address 0x%08x\n", *((uint32_t *)(g_MSPIAddresses[MSPI_PSRAM_XIPCODE_MODULE].XIPMMBase + pTestConfig->ByteOffset)), g_MSPIAddresses[MSPI_PSRAM_XIPCODE_MODULE].XIPMMBase + pTestConfig->ByteOffset);
      am_util_debug_printf("Final Value 0x%08x at address 0x%08x\n", *((uint32_t *)(g_MSPIAddresses[MSPI_PSRAM_XIPCODE_MODULE].XIPMMBase + pTestConfig->ByteOffset + 4)), g_MSPIAddresses[MSPI_PSRAM_XIPCODE_MODULE].XIPMMBase + pTestConfig->ByteOffset + 4);

      *((uint32_t *)(g_MSPIAddresses[MSPI_PSRAM_XIPCODE_MODULE].XIPMMBase + pTestConfig->ByteOffset)) = 0;
      *((uint32_t *)(g_MSPIAddresses[MSPI_PSRAM_XIPCODE_MODULE].XIPMMBase + pTestConfig->ByteOffset + 4)) = 0;
      pu32Ptr1 = (uint32_t *)(g_MSPIAddresses[MSPI_PSRAM_XIPCODE_MODULE].XIPMMBase + pTestConfig->ByteOffset + 1);
      u32Val1 = 0xAABBCCDD;
      am_util_debug_printf("Initial Value 0x%08x at address 0x%08x\n", *((uint32_t *)(g_MSPIAddresses[MSPI_PSRAM_XIPCODE_MODULE].XIPMMBase + pTestConfig->ByteOffset)), g_MSPIAddresses[MSPI_PSRAM_XIPCODE_MODULE].XIPMMBase + pTestConfig->ByteOffset);
      am_util_debug_printf("Initial Value 0x%08x at address 0x%08x\n", *((uint32_t *)(g_MSPIAddresses[MSPI_PSRAM_XIPCODE_MODULE].XIPMMBase + pTestConfig->ByteOffset + 4)), g_MSPIAddresses[MSPI_PSRAM_XIPCODE_MODULE].XIPMMBase + pTestConfig->ByteOffset + 4);
      am_util_debug_printf("Writing Word 0x%08x to address 0x%08x\n", u32Val1, pu32Ptr1);
      *pu32Ptr1 = u32Val1;
      am_util_debug_printf("Final Value 0x%08x at address 0x%08x\n", *((uint32_t *)(g_MSPIAddresses[MSPI_PSRAM_XIPCODE_MODULE].XIPMMBase + pTestConfig->ByteOffset)), g_MSPIAddresses[MSPI_PSRAM_XIPCODE_MODULE].XIPMMBase + pTestConfig->ByteOffset);
      am_util_debug_printf("Final Value 0x%08x at address 0x%08x\n", *((uint32_t *)(g_MSPIAddresses[MSPI_PSRAM_XIPCODE_MODULE].XIPMMBase + pTestConfig->ByteOffset + 4)), g_MSPIAddresses[MSPI_PSRAM_XIPCODE_MODULE].XIPMMBase + pTestConfig->ByteOffset + 4);

      pu16Ptr1 = (uint16_t *)(g_MSPIAddresses[MSPI_PSRAM_XIPCODE_MODULE].XIPMMBase + pTestConfig->ByteOffset + 1);
      pu16Ptr2 = (uint16_t *)(g_MSPIAddresses[MSPI_PSRAM_XIPCODE_MODULE].XIPMMBase + pTestConfig->ByteOffset + 1 + 2*pTestConfig->NumBytes);
      // Initialize a pattern
      for (ix = 0; ix < pTestConfig->NumBytes/2; ix++)
      {
        u32Val1 = (ix >> 8) | ((ix + 1) << 24);
        *pu16Ptr1++ = (uint16_t)(u32Val1 & 0xFFFF);
        *pu16Ptr1++ = (uint16_t)((u32Val1 >> 16) & 0xFFFF);
        u32Val2 = (ix ^ 0xFFFFFFFF) >> 8 | (((ix + 1) ^ 0xFFFFFFFF) << 24);
        *pu16Ptr2++ = (uint16_t)(u32Val2 & 0xFFFF);
        *pu16Ptr2++ = (uint16_t)((u32Val2 >> 16) & 0xFFFF);
      }
      *((uint8_t *)(g_MSPIAddresses[MSPI_PSRAM_XIPCODE_MODULE].XIPMMBase + pTestConfig->ByteOffset)) = 0;
      *((uint8_t *)(g_MSPIAddresses[MSPI_PSRAM_XIPCODE_MODULE].XIPMMBase + pTestConfig->ByteOffset + 2*pTestConfig->NumBytes)) = 0xFF;

      pu16Ptr1 = (uint16_t *)(g_MSPIAddresses[MSPI_PSRAM_XIPCODE_MODULE].XIPMMBase + pTestConfig->ByteOffset);
      pu16Ptr2 = (uint16_t *)(g_MSPIAddresses[MSPI_PSRAM_XIPCODE_MODULE].XIPMMBase + pTestConfig->ByteOffset + 2*pTestConfig->NumBytes);

      // Verify the pattern
      for (ix = 0; ix < (pTestConfig->NumBytes/2); ix++)
      {
        u32Val1 = *pu16Ptr1++;
        u32Val1 |= (*pu16Ptr1++ << 16);
        u32Val2 = *pu16Ptr2++;
        u32Val2 |= (*pu16Ptr2++ << 16);

        if ((u32Val1 != ix) || (u32Val2 != (ix ^ 0xFFFFFFFF)))
        {
          return false;
        }
      }
    }
    break;
  case MSPI_XIPMM_WORD_ACCESS_UNALIGNED:
    {
      // Write aligned, read unaligned
      // Write 4 pages
      pu32Ptr1 = (uint32_t *)(g_MSPIAddresses[MSPI_PSRAM_XIPCODE_MODULE].XIPMMBase + pTestConfig->ByteOffset);
      pu32Ptr2 = (uint32_t *)(g_MSPIAddresses[MSPI_PSRAM_XIPCODE_MODULE].XIPMMBase + pTestConfig->ByteOffset + 2*pTestConfig->NumBytes);
      // Initialize a pattern
      for (ix = 0; ix < pTestConfig->NumBytes/2; ix++)
      {
        u32Val1 = ix;
        *pu32Ptr1++ = u32Val1;
        u32Val2 = ix ^ 0xFFFFFFFF;
        *pu32Ptr2++ = u32Val2;
      }

      // Read at offset 1
      pu32Ptr1 = (uint32_t *)(g_MSPIAddresses[MSPI_PSRAM_XIPCODE_MODULE].XIPMMBase + pTestConfig->ByteOffset + 1);
      pu32Ptr2 = (uint32_t *)(g_MSPIAddresses[MSPI_PSRAM_XIPCODE_MODULE].XIPMMBase + pTestConfig->ByteOffset + 1 + 2*pTestConfig->NumBytes);


      u32Val1 = 0;
      u32Val2 = 0xFF;
      // Verify the pattern
      for (ix = 0; ix < (pTestConfig->NumBytes/2); ix++)
      {
        uint32_t tempVal1, tempVal2;
        tempVal1 = *pu32Ptr1++;
        u32Val1 |= tempVal1 << 8;
        tempVal2 = *pu32Ptr2++;
        u32Val2 |= tempVal2 << 8;

        if ((u32Val1 != ix) || (u32Val2 != (ix ^ 0xFFFFFFFF)))
        {
          return false;
        }
        u32Val1 = tempVal1 >> 24;
        u32Val2 = tempVal2 >> 24;
      }
      // Read at offset 2
      pu32Ptr1 = (uint32_t *)(g_MSPIAddresses[MSPI_PSRAM_XIPCODE_MODULE].XIPMMBase + pTestConfig->ByteOffset + 2);
      pu32Ptr2 = (uint32_t *)(g_MSPIAddresses[MSPI_PSRAM_XIPCODE_MODULE].XIPMMBase + pTestConfig->ByteOffset + 2 + 2*pTestConfig->NumBytes);

      u32Val1 = 0;
      u32Val2 = 0xFFFF;
      // Verify the pattern
      for (ix = 0; ix < (pTestConfig->NumBytes/2); ix++)
      {
        uint32_t tempVal1, tempVal2;
        tempVal1 = *pu32Ptr1++;
        u32Val1 |= tempVal1 << 16;
        tempVal2 = *pu32Ptr2++;
        u32Val2 |= tempVal2 << 16;

        if ((u32Val1 != ix) || (u32Val2 != (ix ^ 0xFFFFFFFF)))
        {
          return false;
        }
        u32Val1 = tempVal1 >> 16;
        u32Val2 = tempVal2 >> 16;
      }
      // Read at offset 3
      pu32Ptr1 = (uint32_t *)(g_MSPIAddresses[MSPI_PSRAM_XIPCODE_MODULE].XIPMMBase + pTestConfig->ByteOffset + 3);
      pu32Ptr2 = (uint32_t *)(g_MSPIAddresses[MSPI_PSRAM_XIPCODE_MODULE].XIPMMBase + pTestConfig->ByteOffset + 3 + 2*pTestConfig->NumBytes);

      u32Val1 = 0;
      u32Val2 = 0xFFFFFF;
      // Verify the pattern
      for (ix = 0; ix < (pTestConfig->NumBytes/2); ix++)
      {
        uint32_t tempVal1, tempVal2;
        tempVal1 = *pu32Ptr1++;
        u32Val1 |= tempVal1 << 24;
        tempVal2 = *pu32Ptr2++;
        u32Val2 |= tempVal2 << 24;

        if ((u32Val1 != ix) || (u32Val2 != (ix ^ 0xFFFFFFFF)))
        {
          return false;
        }
        u32Val1 = tempVal1 >> 8;
        u32Val2 = tempVal2 >> 8;
      }
      // Write unaligned, read aligned
      // Write 4 pages
      // Write at offset 1
      pu32Ptr1 = (uint32_t *)(g_MSPIAddresses[MSPI_PSRAM_XIPCODE_MODULE].XIPMMBase + pTestConfig->ByteOffset + 1);
      pu32Ptr2 = (uint32_t *)(g_MSPIAddresses[MSPI_PSRAM_XIPCODE_MODULE].XIPMMBase + pTestConfig->ByteOffset + 1 + 2*pTestConfig->NumBytes);
      // Initialize a pattern
      for (ix = 0; ix < pTestConfig->NumBytes/2; ix++)
      {
        u32Val1 = (ix >> 8) | ((ix + 1) << 24);
        *pu32Ptr1++ = u32Val1;
        u32Val2 = (ix ^ 0xFFFFFFFF) >> 8 | (((ix + 1) ^ 0xFFFFFFFF) << 24);
        *pu32Ptr2++ = u32Val2;
      }
      *((uint8_t *)(g_MSPIAddresses[MSPI_PSRAM_XIPCODE_MODULE].XIPMMBase + pTestConfig->ByteOffset)) = 0;
      *((uint8_t *)(g_MSPIAddresses[MSPI_PSRAM_XIPCODE_MODULE].XIPMMBase + pTestConfig->ByteOffset + 2*pTestConfig->NumBytes)) = 0xFF;

      pu32Ptr1 = (uint32_t *)(g_MSPIAddresses[MSPI_PSRAM_XIPCODE_MODULE].XIPMMBase + pTestConfig->ByteOffset);
      pu32Ptr2 = (uint32_t *)(g_MSPIAddresses[MSPI_PSRAM_XIPCODE_MODULE].XIPMMBase + pTestConfig->ByteOffset + 2*pTestConfig->NumBytes);

      // Verify the pattern
      for (ix = 0; ix < (pTestConfig->NumBytes/2); ix++)
      {
        u32Val1 = *pu32Ptr1++;
        u32Val2 = *pu32Ptr2++;

        if ((u32Val1 != ix) || (u32Val2 != (ix ^ 0xFFFFFFFF)))
        {
          return false;
        }
      }
      // Write at offset 2
      pu32Ptr1 = (uint32_t *)(g_MSPIAddresses[MSPI_PSRAM_XIPCODE_MODULE].XIPMMBase + pTestConfig->ByteOffset + 2);
      pu32Ptr2 = (uint32_t *)(g_MSPIAddresses[MSPI_PSRAM_XIPCODE_MODULE].XIPMMBase + pTestConfig->ByteOffset + 2 + 2*pTestConfig->NumBytes);
      // Initialize a pattern
      for (ix = 0; ix < pTestConfig->NumBytes/2; ix++)
      {
        u32Val1 = (ix >> 16) | ((ix + 1) << 16);
        *pu32Ptr1++ = u32Val1;
        u32Val2 = (ix ^ 0xFFFFFFFF) >> 16 | (((ix + 1) ^ 0xFFFFFFFF) << 16);
        *pu32Ptr2++ = u32Val2;
      }
      *((uint16_t *)(g_MSPIAddresses[MSPI_PSRAM_XIPCODE_MODULE].XIPMMBase + pTestConfig->ByteOffset)) = 0;
      *((uint16_t *)(g_MSPIAddresses[MSPI_PSRAM_XIPCODE_MODULE].XIPMMBase + pTestConfig->ByteOffset + 2*pTestConfig->NumBytes)) = 0xFFFF;

      pu32Ptr1 = (uint32_t *)(g_MSPIAddresses[MSPI_PSRAM_XIPCODE_MODULE].XIPMMBase + pTestConfig->ByteOffset);
      pu32Ptr2 = (uint32_t *)(g_MSPIAddresses[MSPI_PSRAM_XIPCODE_MODULE].XIPMMBase + pTestConfig->ByteOffset + 2*pTestConfig->NumBytes);

      // Verify the pattern
      for (ix = 0; ix < (pTestConfig->NumBytes/2); ix++)
      {
        u32Val1 = *pu32Ptr1++;
        u32Val2 = *pu32Ptr2++;

        if ((u32Val1 != ix) || (u32Val2 != (ix ^ 0xFFFFFFFF)))
        {
          return false;
        }
      }
      // Write at offset 3
      pu32Ptr1 = (uint32_t *)(g_MSPIAddresses[MSPI_PSRAM_XIPCODE_MODULE].XIPMMBase + pTestConfig->ByteOffset + 3);
      pu32Ptr2 = (uint32_t *)(g_MSPIAddresses[MSPI_PSRAM_XIPCODE_MODULE].XIPMMBase + pTestConfig->ByteOffset + 3 + 2*pTestConfig->NumBytes);
      // Initialize a pattern
      for (ix = 0; ix < pTestConfig->NumBytes/2; ix++)
      {
        u32Val1 = (ix >> 24) | ((ix + 1) << 8);
        *pu32Ptr1++ = u32Val1;
        u32Val2 = (ix ^ 0xFFFFFFFF) >> 24 | (((ix + 1) ^ 0xFFFFFFFF) << 8);
        *pu32Ptr2++ = u32Val2;
      }
      *((uint16_t *)(g_MSPIAddresses[MSPI_PSRAM_XIPCODE_MODULE].XIPMMBase + pTestConfig->ByteOffset)) = 0;
      *((uint16_t *)(g_MSPIAddresses[MSPI_PSRAM_XIPCODE_MODULE].XIPMMBase + pTestConfig->ByteOffset + 2*pTestConfig->NumBytes)) = 0xFFFF;
      *((uint8_t *)(g_MSPIAddresses[MSPI_PSRAM_XIPCODE_MODULE].XIPMMBase + pTestConfig->ByteOffset + 2)) = 0;
      *((uint8_t *)(g_MSPIAddresses[MSPI_PSRAM_XIPCODE_MODULE].XIPMMBase + pTestConfig->ByteOffset + 2 + 2*pTestConfig->NumBytes)) = 0xFF;

      pu32Ptr1 = (uint32_t *)(g_MSPIAddresses[MSPI_PSRAM_XIPCODE_MODULE].XIPMMBase + pTestConfig->ByteOffset);
      pu32Ptr2 = (uint32_t *)(g_MSPIAddresses[MSPI_PSRAM_XIPCODE_MODULE].XIPMMBase + pTestConfig->ByteOffset + 2*pTestConfig->NumBytes);

      // Verify the pattern
      for (ix = 0; ix < (pTestConfig->NumBytes/2); ix++)
      {
        u32Val1 = *pu32Ptr1++;
        u32Val2 = *pu32Ptr2++;

        if ((u32Val1 != ix) || (u32Val2 != (ix ^ 0xFFFFFFFF)))
        {
          return false;
        }
      }
    }
    break;
  case MSPI_XIPMM_OWORD_ACCESS:
    {

      uint32_t pattern[16];
      uint32_t pattern2[16];
      uint32_t addr = g_MSPIAddresses[MSPI_PSRAM_XIPCODE_MODULE].XIPMMBase;

      // Initialize a pattern
      for (ix = 0; ix < 8; ix++)
      {
        pattern[ix] = ix;
        pattern[ix+8] = ix ^ 0xFFFFFFFF;
      }

      // Octal Word Write
      pu32Ptr1 = (uint32_t *)(addr + pTestConfig->ByteOffset);
      octal_copy((uint32_t)pattern, addr + pTestConfig->ByteOffset, 2);

      octal_copy(addr + pTestConfig->ByteOffset, (uint32_t)pattern2, 2);
      pu32Ptr1 = (uint32_t *)(pattern2);
      pu32Ptr2 = (uint32_t *)(pattern2 + 8);

      // Verify the pattern
      for (ix = 0; ix < 8; ix++)
      {
        if ( (*pu32Ptr1++ != ix) || (*pu32Ptr2++ != (ix ^ 0xFFFFFFFF)) )
        {
          return false;
        }
      }
    }
    break;
    default:break;
  }

  ui32Status = mspi_psram_xip_device_func.mspi_xip_disable(g_pPsramXipCodeHandle);
  if (AM_WIDGET_MSPI_SUCCESS != ui32Status)
  {
    am_util_stdio_printf("Failed to disable XIP mode.  Status = %d\n", ui32Status);
    return false;
  }

  return true;
}


bool insert_xipmm(am_widget_mspi_test_t *pXIPTestCfg)
{
    bool testPassed = false;
#if 1
      pXIPTestCfg->eXIPMMAccess = MSPI_XIPMM_MEMCPY_ACCESS;
      am_hal_gpio_output_set(195);
      testPassed = mspi_xipmm(pXIPTestCfg, errStr);
      am_hal_gpio_output_clear(195);
      TEST_ASSERT(testPassed);
      if (!testPassed)
      {
        am_util_stdio_printf("DDR HEX XIPMM Data test failed MEMCPY_ACCESS\n");
      }
#endif

#if 1
      pXIPTestCfg->eXIPMMAccess = MSPI_XIPMM_WORD_ACCESS;
      am_hal_gpio_output_set(195);
      testPassed = mspi_xipmm(pXIPTestCfg, errStr);
      am_hal_gpio_output_clear(195);
      TEST_ASSERT(testPassed);
      if (!testPassed)
      {
        am_util_stdio_printf("DDR HEX XIPMM Data test failed WORD_ACCESS\n");
      }
#endif
#if 1
      pXIPTestCfg->eXIPMMAccess = MSPI_XIPMM_SHORT_ACCESS;
      am_hal_gpio_output_set(195);
      testPassed = mspi_xipmm(pXIPTestCfg, errStr);
      am_hal_gpio_output_clear(195);
      TEST_ASSERT(testPassed);
      if (!testPassed)
      {
        am_util_stdio_printf("DDR HEX XIPMM Data test failed SHORT_ACCESS\n");
      }
#endif
#if 1
      pXIPTestCfg->eXIPMMAccess = MSPI_XIPMM_BYTE_ACCESS;
      am_hal_gpio_output_set(195);
      testPassed = mspi_xipmm(pXIPTestCfg, errStr);
      am_hal_gpio_output_clear(195);
      TEST_ASSERT(testPassed);
      if (!testPassed)
      {
        am_util_stdio_printf("DDR HEX XIPMM Data test failed BYTE_ACCESS\n");
      }
#endif
#if 1
      pXIPTestCfg->eXIPMMAccess = MSPI_XIPMM_SHORT_ACCESS_UNALIGNED;
      am_hal_gpio_output_set(195);
      testPassed = mspi_xipmm(pXIPTestCfg, errStr);
      am_hal_gpio_output_clear(195);
      TEST_ASSERT(testPassed);
      if (!testPassed)
      {
        am_util_stdio_printf("DDR HEX XIPMM Data test failed ACCESS_UNALIGNED\n");
      }
#endif
#if 1
      pXIPTestCfg->eXIPMMAccess = MSPI_XIPMM_OWORD_ACCESS;
      am_hal_gpio_output_set(195);
      testPassed = mspi_xipmm(pXIPTestCfg, errStr);
      am_hal_gpio_output_clear(195);
      TEST_ASSERT(testPassed);
      if (!testPassed)
      {
        am_util_stdio_printf("DDR HEX XIPMM Data test failed OWORD_ACCESS\n");
      }
#endif
      return testPassed;
}


bool
dma_xip_parallel(am_widget_mspi_test_t *pDMATestCfg, am_widget_mspi_test_t *pXIPTestCfg)
{
  uint32_t      ui32Status;
  volatile bool bDMAComplete = false;

//   ui32Status = mspi_psram_xip_device_func.mspi_xip_enable(g_pPsramXipCodeHandle);
//   if (AM_WIDGET_MSPI_SUCCESS != ui32Status)
//   {
//     am_util_stdio_printf("Failed to enable XIP mode in the MSPI!\n");
//     return false;
//   }

  // Generate data into the Sector Buffer
  for (uint32_t i = 0; i < pDMATestCfg->NumBytes; i++)
  {
    pDMATestCfg->pTxBuffer[pDMATestCfg->ByteOffset + i] = (rand() & 0xFE); //ensure D0 is always 0
  }

  // Write the TX buffer into the target sector.
  am_hal_gpio_output_set(138);
  am_util_stdio_printf("Writing %d Bytes to Sector %8X\n", pDMATestCfg->NumBytes,
                       pDMATestCfg->SectorAddress + pDMATestCfg->SectorOffset);
  ui32Status = mspi_psram_xip_device_func.mspi_write_adv(g_pPsramXipCodeHandle,
                                              &pDMATestCfg->pTxBuffer[pDMATestCfg->ByteOffset],
                                              pDMATestCfg->SectorAddress + pDMATestCfg->SectorOffset,
                                              pDMATestCfg->NumBytes, 0, 0,
                                              pfnMSPI_DMA_Callback,
                                              (void *)&bDMAComplete);
  if (AM_WIDGET_MSPI_SUCCESS != ui32Status)
  {
    am_util_stdio_printf("Failed to write to target sector.  Status = %d\n", ui32Status);
    return false;
  }

  // Wait for DMA Complete or Timeout
  for (uint32_t i = 0; i < AM_WIDGET_MSPI_TIMEOUT; i++)
  {
    if (bDMAComplete)
    {
      am_hal_gpio_output_clear(138);
      break;
    }
    am_hal_delay_us(rand()%g_Test_controls.ui32RandomDelayMax);
    insert_xipmm(pXIPTestCfg);
  }

  // Check the status.
  if (!bDMAComplete)
  {
    am_util_stdio_printf("Failed to write to target sector. DMA timed out!\n");
    return false;
  }

  // Read the data back into the RX buffer.
  am_hal_gpio_output_set(138);
  am_util_stdio_printf("Read %d Bytes from Sector %8X\n", pDMATestCfg->NumBytes,
                       pDMATestCfg->SectorAddress + pDMATestCfg->SectorOffset);
  bDMAComplete = false;
  ui32Status = mspi_psram_xip_device_func.mspi_read_adv(g_pPsramXipCodeHandle,
                                              &pDMATestCfg->pRxBuffer[pDMATestCfg->ByteOffset],
                                              pDMATestCfg->SectorAddress + pDMATestCfg->SectorOffset,
                                              pDMATestCfg->NumBytes, 0, 0,
                                              pfnMSPI_DMA_Callback,
                                              (void *)&bDMAComplete);
  if (AM_WIDGET_MSPI_SUCCESS != ui32Status)
  {
    am_util_stdio_printf("Failed to read from target sector.  Status = %d\n", ui32Status);
    return false;
  }

  // Wait for DMA Complete or Timeout
  for (uint32_t i = 0; i < AM_WIDGET_MSPI_TIMEOUT; i++)
  {
    if (bDMAComplete)
    {
      am_hal_gpio_output_clear(138);
      break;
    }
    am_hal_delay_us(rand()%g_Test_controls.ui32RandomDelayMax);
    insert_xipmm(pXIPTestCfg);
  }

  // Check the status.
  if (!bDMAComplete)
  {
    am_util_stdio_printf("Failed to read from target sector. DMA timed out!\n");
    return false;
  }

  // Compare the buffers

  am_util_stdio_printf("Comparing the TX and RX Buffers\n");
  

  ui32Status = memcmp(pDMATestCfg->pRxBuffer + pDMATestCfg->ByteOffset, pDMATestCfg->pTxBuffer + pDMATestCfg->ByteOffset, pDMATestCfg->NumBytes);
  if(ui32Status == 0)
  {
    return true;
  }
  else
  {
    return false;
  }
}

void
mspi_dma_xip_parallel_test(void)
{
    am_widget_mspi_config_t widgetConfig;
    uint32_t randomDelay;
    SCB_DisableDCache();

    // 138 used to indicate the start and end of a single DMA operation
    // 195 used to indicate the start and end of a batch of 1024byte XIPMM operation
    am_hal_gpio_output_clear(138);
    am_hal_gpio_output_clear(195);
    am_hal_gpio_pinconfig(138, am_hal_gpio_pincfg_output);
    am_hal_gpio_pinconfig(195, am_hal_gpio_pincfg_output);

    bool  testPassed = true;
    uint32_t speed = 0;
    uint32_t mspi = MSPI_PSRAM_XIPCODE_MODULE;

    g_Test_controls.sDMATestConfig.NumBytes = MSPI_DMA_BUFFER_LEN;
    g_Test_controls.sDMATestConfig.ByteOffset = 0;
    g_Test_controls.sDMATestConfig.SectorAddress = 0x00000000;
    g_Test_controls.sDMATestConfig.SectorOffset = 0;
    g_Test_controls.sDMATestConfig.UnscrambledSector = 0;
    g_Test_controls.sDMATestConfig.pTxBuffer = ui8DMATXBuffer;
    g_Test_controls.sDMATestConfig.pRxBuffer = ui8DMARXBuffer;

    g_Test_controls.sXIPTestConfig.NumBytes = AM_DEVICES_MSPI_PSRAM_PAGE_SIZE;
    g_Test_controls.sXIPTestConfig.pTxBuffer = ui8XIPTXBuffer;
    g_Test_controls.sXIPTestConfig.pRxBuffer = ui8XIPRXBuffer;
    g_Test_controls.sXIPTestConfig.ByteOffset = 2048*1024;

    testPassed = dma_xip_parallel(&g_Test_controls.sDMATestConfig, &g_Test_controls.sXIPTestConfig);

    srand(xTaskGetTickCount());
    randomDelay = rand() % MAX_XIP_TASK_DELAY;
    vTaskDelay(randomDelay);

    if (!testPassed)
    {
        am_util_stdio_printf("DMA XIP Parallel test failed at speed \n");
    }

    am_util_delay_ms(100);
}
#endif

void
XipTask(void *pvParameters)
{
    int argc = 4;
    char* argv[4];
    argv[0] = "loops_all_mid_10k_sp";
    argv[1] = "-c1";
    argv[2] = "-w1";
    argv[3] = "-v1";

    uint32_t randomDelay;
    uint32_t ui32CodeSectionLength = 0;
    uint32_t ui32CodeSectionLoadAddr = 0;
    uint32_t ui32CodeRunAddr = 0;

#ifndef MSPI_DMA_XIP_PARALLEL_TEST 
    ui32CodeSectionLength = (uint32_t)&_exipcode - (uint32_t)&_sxipcode;
    ui32CodeSectionLoadAddr = (uint32_t)&__xip_text_load_addr;
    ui32CodeRunAddr = (uint32_t)&_sxipcode;
#endif
#if defined(XIP_IN_FLASH)
    if (mspi_flash_device_func.mspi_xip_enable == NULL)
    {
        am_util_stdio_printf(">> Flash Not supported XIP operations! <<\n");
        //Suspend and exit from current task.
        vTaskDelete(NULL);
    }
    uint32_t i;
    uint32_t ui32Status;
    uint32_t ui32SectorSize = (ui32CodeSectionLength + MSPI_FLASH_SECTOR_SIZE)/MSPI_FLASH_SECTOR_SIZE ;
    
    am_util_stdio_printf("Writing %d Bytes to flash\n", ui32CodeSectionLength);
    am_util_stdio_printf("Need to erased the sector size is %d.\n", ui32SectorSize);	
    //
    // Erase the target sector.
    //
    for(i = 0; i < ui32SectorSize; i++)
    {
        am_util_stdio_printf("Erasing Sector %d\n", i);
        ui32Status = mspi_flash_device_func.mspi_sector_erase(g_pFlashHandle, i * MSPI_FLASH_SECTOR_SIZE);
        if (MSPI_STATUS_SUCCESS != ui32Status)
        {
            am_util_stdio_printf("Failed to erase Flash Device sector!\n");

            //Suspend and exit from current task.
            vTaskDelete(NULL);
        }
    }

    //
    // Copy the code section from MRAM to FLASH.
    //
    ui32Status = mspi_flash_device_func.mspi_write(g_pFlashHandle, 
                                               (uint8_t*)ui32CodeSectionLoadAddr, 
                                               0, 
                                               ui32CodeSectionLength, 
                                               true);
    if (MSPI_STATUS_SUCCESS != ui32Status)
    {
        am_util_stdio_printf("Failed to write the Flash Device!\n");

        //Suspend and exit from current task.
        vTaskDelete(NULL);
    }
    am_util_stdio_printf("Success to write the Flash Device!\n");
    //
    // Enable XIP.
    //
    ui32Status = mspi_flash_device_func.mspi_xip_enable(g_pFlashHandle);
    if (MSPI_STATUS_SUCCESS != ui32Status)
    {
        am_util_stdio_printf("Failed to enable XIP mode in the MSPI!\n");

        //Suspend and exit from current task.
        vTaskDelete(NULL);
    }
    am_util_stdio_printf("XIP tasks in external flash..\r\n");
#elif defined(XIP_IN_PSRAM)
#if defined(MSPI_DMA_XIP_PARALLEL_TEST)
    am_util_stdio_printf("Running XIP and DMA mode in mspi\r\n");
#else
    //
    // Copy the code section from MRAM to PSRAM.
    //
    am_util_stdio_printf("Copy %d Bytes to psram, Code run address: 0x%x, Code loader address: 0x%x \n", ui32CodeSectionLength, ui32CodeRunAddr, ui32CodeSectionLoadAddr);
    memcpy((void *)ui32CodeRunAddr, (void *)ui32CodeSectionLoadAddr, ui32CodeSectionLength);
#endif
    am_hal_cachectrl_dcache_invalidate(NULL, true);
    am_util_stdio_printf("XIP tasks in external PSRAM..\r\n");
#else
    #error "Plaease select a memory to handle the XIP operations.!"
#endif
    while (1)
    {
#if defined(MSPI_DMA_XIP_PARALLEL_TEST)
        //
        // Run MSPI dma xip parallel test.
        //
        mspi_dma_xip_parallel_test();
#else
        am_util_stdio_printf("Running XIP loops_all_mid_10k_sp tasks...\r\n");
 
        //
        // Run the coremarkPro workload in XIP mode.
        //
        loops_all_mid_10k_sp_main(argc, argv);

        am_util_stdio_printf("Running XIP loops_all_mid_10k_sp tasks done...\r\n");

        srand(xTaskGetTickCount());
        randomDelay = rand() % MAX_XIP_TASK_DELAY;
        vTaskDelay(randomDelay);
#endif
    }
}

#endif  //XIP_TASK_ENABLE
