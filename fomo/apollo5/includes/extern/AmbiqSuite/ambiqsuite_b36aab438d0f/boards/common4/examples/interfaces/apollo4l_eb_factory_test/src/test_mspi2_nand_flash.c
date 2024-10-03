//*****************************************************************************
//
//! @file test_mspi2_nand_flash.c
//
//*****************************************************************************

//*****************************************************************************
//
// ${copyright}
//
// This is part of revision ${version} of the AmbiqSuite Development Package.
//
//*****************************************************************************

#include "string.h"
#include "am_mcu_apollo.h"
#include "am_bsp.h"
#include "am_devices_mspi_ds35x1ga.h"
#include "am_util.h"
#include "factory_test_helpers.h"
//*****************************************************************************
//
// Macro definitions
//
//*****************************************************************************
#define FLASH_TEST_MODULE               2
#define NAND_TEST_FLASH_CFG_MODE        (1)    // 0:serial ; 1:quad
#define NAND_TEST_PAGE                  (0x100)


//*****************************************************************************
//
// Variables Definition
//
//*****************************************************************************
uint32_t ui32DMATCBBuffer2[2560];
void            *g_pFlashHandle;
void            *g_pMSPIFlashHandle;

const am_devices_mspi_ds35x1ga_config_t MSPI_Quad_Flash_Config =
{
    .eDeviceConfig = AM_HAL_MSPI_FLASH_QUAD_CE0_1_1_4,
    .eClockFreq = AM_HAL_MSPI_CLK_24MHZ,
    .pNBTxnBuf = ui32DMATCBBuffer2,
    .ui32NBTxnBufLength = (sizeof(ui32DMATCBBuffer2) / sizeof(uint32_t)),
    .ui32ScramblingStartAddr = 0,
    .ui32ScramblingEndAddr = 0,
};

uint8_t write_data[AM_DEVICES_MSPI_DS35X1GA_PAGE_DATA_SIZE];
uint8_t read_data[AM_DEVICES_MSPI_DS35X1GA_PAGE_DATA_SIZE];
uint8_t write_oob[AM_DEVICES_MSPI_DS35X1GA_PAGE_OOB_SIZE];
uint8_t read_oob[AM_DEVICES_MSPI_DS35X1GA_PAGE_OOB_SIZE];

//! MSPI interrupts.
static const IRQn_Type mspi_interrupts[] =
{
    MSPI0_IRQn,
    MSPI1_IRQn,
    MSPI2_IRQn,
};

//
// Take over the interrupt handler for whichever MSPI we're using.
//
#define flash_mspi_isr                                                         \
    am_mspi_isr1(FLASH_TEST_MODULE)
#define am_mspi_isr1(n)                                                        \
    am_mspi_isr(n)
#define am_mspi_isr(n)                                                         \
    am_mspi ## n ## _isr
//*****************************************************************************
//
// MSPI ISRs.
//
//*****************************************************************************
void flash_mspi_isr(void)
{
    uint32_t ui32Status;

    am_hal_mspi_interrupt_status_get(g_pMSPIFlashHandle, &ui32Status, false);
    am_hal_mspi_interrupt_clear(g_pMSPIFlashHandle, ui32Status);
    am_hal_mspi_interrupt_service(g_pMSPIFlashHandle, ui32Status);
}



int mspi2_quad_nand_flash_test(uint32_t mode)
{
  uint32_t ui32Status;
  uint8_t ui8EccStatus;

  //
  // fill the test page with some data
  //
  for (uint32_t i = 0; i < AM_DEVICES_MSPI_DS35X1GA_PAGE_DATA_SIZE; i++)
  {
    write_data[i] = i;
  }

  //
  // fill the oob area with zero
  //
  memset(write_oob, 0x0, sizeof(write_oob));

  ui32Status = am_devices_mspi_ds35x1ga_init(FLASH_TEST_MODULE, &MSPI_Quad_Flash_Config, &g_pFlashHandle, &g_pMSPIFlashHandle);

  if (AM_DEVICES_MSPI_DS35X1GA_STATUS_SUCCESS != ui32Status)
  {
    am_util_stdio_printf("Failed to configure the MSPI and Flash Device correctly!\n");
    goto _fail;
  }
  else
  {
    am_util_debug_printf("Configure the MSPI and Flash Device correctly!\n");
  }

  NVIC_EnableIRQ(mspi_interrupts[FLASH_TEST_MODULE]);
  am_hal_interrupt_master_enable();

  uint32_t ui32DeviceID;
  am_devices_mspi_ds35x1ga_id(g_pFlashHandle, &ui32DeviceID);
  am_util_debug_printf("NAND flash ID is 0x%x!\n", ui32DeviceID);

  ui32Status = am_devices_mspi_ds35x1ga_block_erase(g_pFlashHandle, NAND_TEST_PAGE >> 6);
  am_util_debug_printf("block %d erase status 0x%x!\n", NAND_TEST_PAGE >> 6, ui32Status);

  ui32Status = am_devices_mspi_ds35x1ga_write(g_pFlashHandle, NAND_TEST_PAGE, &write_data[0],
                                              AM_DEVICES_MSPI_DS35X1GA_PAGE_DATA_SIZE, &write_oob[0],
                                              AM_DEVICES_MSPI_DS35X1GA_PAGE_OOB_SIZE);
  am_util_debug_printf("page %d write status 0x%x!\n", NAND_TEST_PAGE, ui32Status);


  memset(read_data, 0x0, AM_DEVICES_MSPI_DS35X1GA_PAGE_DATA_SIZE);

  ui32Status = am_devices_mspi_ds35x1ga_read(g_pFlashHandle, NAND_TEST_PAGE, &read_data[0],  AM_DEVICES_MSPI_DS35X1GA_PAGE_DATA_SIZE, &read_oob[0], 16, &ui8EccStatus);

  am_util_debug_printf("page %d read status 0x%x!\n", NAND_TEST_PAGE, ui32Status);

  am_util_debug_printf("page %d read ECC status 0x%x!\n", NAND_TEST_PAGE, ui8EccStatus);

  //
    // check if block data match or not
    //
    if ( memory_compare( (const void *)write_data, (const void *)read_data, AM_DEVICES_MSPI_DS35X1GA_PAGE_DATA_SIZE ) != 0 )
    {
      am_util_stdio_printf("TX and RX buffers failed to compare!\n");
       goto _fail;
    }

    //
    // Clean up the MSPI before exit.
    //
    NVIC_DisableIRQ(mspi_interrupts[FLASH_TEST_MODULE]);

    ui32Status = am_devices_mspi_ds35x1ga_deinit(g_pFlashHandle);
    if (AM_DEVICES_MSPI_DS35X1GA_STATUS_SUCCESS != ui32Status)
    {
        am_util_stdio_printf("Failed to shutdown the MSPI and Flash Device!\n");
        goto _fail;
    }
    return 0;
_fail:
    return -1;
}

