//*****************************************************************************
//
//! @file emmc_multiple_instances_test_cases.c
//!
//! @brief Run emmc multiple test cases.
//!
//! This test includes 2 emmc test tasks. We delete all of tasks except setup tast
//! and idle task after they run to the end of task. 

//
//*****************************************************************************

//*****************************************************************************
//
// ${copyright}
//
// This is part of revision ${version} of the AmbiqSuite Development Package.
//
//*****************************************************************************

#include "unity.h"
#include "am_mcu_apollo.h"
#include "am_bsp.h"
#include "am_util.h"
#include "sdio_test_common.h"
#include "am_widget_sdio.h"
#include "emmc0_task.h"
#include "emmc_multiple_instances_test_cases.h"

static am_widget_emmc_devices_config_t eMMC_setting[DUT_NUM] =
{
  {
    .eCardPwrCtrlPolicy = AM_HAL_CARD_PWR_CTRL_NONE,
    .pCardPwrCtrlFunc = NULL,
    .bAsyncWriteIsDone = false,
    .bAsyncReadIsDone = false,
  },
  {
    .eCardPwrCtrlPolicy = AM_HAL_CARD_PWR_CTRL_NONE,
    .pCardPwrCtrlFunc = NULL,
    .bAsyncWriteIsDone = false,
    .bAsyncReadIsDone = false,
  },
};

am_hal_card_t eMMCs[DUT_NUM];
am_widget_timing_scan_hashtable_t eMMC_timing_table[DUT_NUM][SCAN_NUM];

//*****************************************************************************
//
// Optional setup/tear-down functions.
//
// These will be called before and after each test function listed below.
//
//*****************************************************************************
void
setUp(void)
{
  for (uint32_t index = 0; index < DUT_NUM; index++)
  {
    DUTs_setting[index].eCardType = AM_HAL_CARD_TYPE_EMMC;
    DUTs_setting[index].device_config = &eMMC_setting[index];

    g_DUTs[index].pDevHandle = &eMMCs[index];
    am_widget_setupTimingScanTable(eMMC_timing_table[index]);
  }
}

void
tearDown(void)
{
}

//*****************************************************************************
//
// Enable printing to the console.
//
//*****************************************************************************
void
enable_print_interface(void)
{
    //
    // Initialize a debug printing interface.
    //
    am_bsp_debug_printf_enable();
}

//*****************************************************************************
//
// Disable printing to the console.
//
//*****************************************************************************
void
disable_print_interface(void)
{
    //
    // Initialize a debug printing interface.
    //
    am_bsp_debug_printf_disable();
    am_hal_itm_disable();
}


//We need to set up a small heap to be used by Clib, delete this may cause unexpected error.
__attribute__ ((section(".heap")))
uint32_t clib_heap[100];


//*****************************************************************************
//
// Main Function
//
//*****************************************************************************
void
sdio_get_card_info_test(void)
{
  uint32_t eMMC_info_index = 0;
  for (uint32_t index = 0; index < DUT_NUM; index++)
  {
    //
    // Get the CID, CSD and EXT CSD Info of THGBMNG5D1LBAIK eMMC
    //
    am_util_stdio_printf("SDIO #%d: Get Card Info Test\n", index);
    TEST_ASSERT_TRUE(am_widget_sdio_setup(&g_DUTs[DUTs_setting[index].eHost], &DUTs_setting[index]));

    eMMC_info_index = am_widget_get_emmc_identifier(g_DUTs[DUTs_setting[index].eHost].pDevHandle);

    uint32_t emmc_psn;
    emmc_psn = am_hal_card_get_cid_field(g_DUTs[DUTs_setting[index].eHost].pDevHandle, 16, 32);

    if(eMMC_info_index < EMMC_NUM)
    {
      am_util_stdio_printf("%s, SN: 0x%x\n", emmcs_info[eMMC_info_index].manufacturer, emmc_psn);
    }
    else
    {
      am_util_stdio_printf("SDIO #%d:unidentified eMMC device SN: 0x%x\n", index, emmc_psn);
    }
    TEST_ASSERT_TRUE(eMMC_info_index < EMMC_NUM);

    uint32_t emmc_csize;
    emmc_csize = am_hal_card_get_csd_field(g_DUTs[DUTs_setting[index].eHost].pDevHandle,  62, 12);
    am_util_stdio_printf("SDIO #%d:Product CSD Size : 0x%x\n", index, emmc_csize);
    TEST_ASSERT_EQUAL_HEX32(emmcs_info[eMMC_info_index].emmc_csize, emmc_csize);

    uint32_t max_enh_size_mult;
    max_enh_size_mult = am_hal_card_get_ext_csd_field(g_DUTs[DUTs_setting[index].eHost].pDevHandle, 157, 3);
    am_util_stdio_printf("SDIO #%d:Product EXT CSD Max Enh Size Multi : 0x%x\n", index, max_enh_size_mult);
    TEST_ASSERT_EQUAL_HEX32(emmcs_info[eMMC_info_index].max_enh_size_mult, max_enh_size_mult);

    DUTs_setting[index].ui32SectorCount = am_hal_card_get_ext_csd_field(g_DUTs[DUTs_setting[index].eHost].pDevHandle, 212, 4);
    am_util_stdio_printf("SDIO #%d:Product EXT CSD Sector Count : 0x%x\n", index, DUTs_setting[index].ui32SectorCount);
    TEST_ASSERT_EQUAL_HEX32(emmcs_info[eMMC_info_index].sector_count, DUTs_setting[index].ui32SectorCount);

    TEST_ASSERT_TRUE(am_widget_sdio_cleanup(&g_DUTs[DUTs_setting[index].eHost]));
  }
}

int
emmc_multiple_instances_test_cases(void)
{ 
    //
    // Initialize the printf interface for ITM output
    //
    am_bsp_debug_printf_enable();

    //
    // Enable printing to the console.
    //
#ifdef AM_DEBUG_PRINTF
    enable_print_interface();
#endif

    //
    // Initialize timer
    //
    am_hal_timer_config_t       TimerConfig;
    am_hal_timer_default_config_set(&TimerConfig);
    TimerConfig.eInputClock = AM_HAL_TIMER_CLOCK_HFRC_DIV16;
    TimerConfig.eFunction = AM_HAL_TIMER_FN_EDGE;

    am_hal_timer_config(0, &TimerConfig);
    am_hal_timer_start(0);

    //
    // Initialize plotting interface.
    //
    am_util_stdio_printf("emmc_multiple_instances_test_cases\n");

    run_tasks();

    //
    // We shouldn't ever get here.
    //
    while (1)
    {
    }

}

