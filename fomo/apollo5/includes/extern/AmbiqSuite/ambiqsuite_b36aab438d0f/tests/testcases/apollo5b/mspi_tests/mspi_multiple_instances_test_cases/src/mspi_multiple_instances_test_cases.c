//*****************************************************************************
//
//! @file mspi_multiple_instances_test_cases.c
//!
//! @brief Run mspi multiple test cases.
//!
//! This test includes 2 mspi test tasks. We delete all of tasks except setup tast
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
#include "mspi_multiple_instances_test_cases.h"

am_hal_mpu_region_config_t sMPUCfg[] =
{
    {
        .ui32RegionNumber = 6,
        .ui32BaseAddress = (uint32_t)PSRAMDMATCBBuffer,
        .eShareable = NON_SHARE,
        .eAccessPermission = RW_NONPRIV,
        .bExecuteNever = true,
        .ui32LimitAddress = (uint32_t)PSRAMDMATCBBuffer + sizeof(PSRAMDMATCBBuffer) - 1,
        .ui32AttrIndex = 0,
        .bEnable = true,
    },
    {
        .ui32RegionNumber = 7,
        .ui32BaseAddress = (uint32_t)FlashDMATCBBuffer,
        .eShareable = NON_SHARE,
        .eAccessPermission = RW_NONPRIV,
        .bExecuteNever = true,
        .ui32LimitAddress = (uint32_t)FlashDMATCBBuffer + sizeof(FlashDMATCBBuffer) - 1,
        .ui32AttrIndex = 0,
        .bEnable = true,
    },
};

am_hal_mpu_attr_t sMPUAttr =
{
    .ui8AttrIndex = 0,
    .bNormalMem = true,
    .sOuterAttr = {
                    .bNonTransient = false,
                    .bWriteBack = true,
                    .bReadAllocate = false,
                    .bWriteAllocate = false
                  },
    .sInnerAttr = {
                    .bNonTransient = false,
                    .bWriteBack = true,
                    .bReadAllocate = false,
                    .bWriteAllocate = false
                  },
    .eDeviceAttr = 0,
} ;
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
  am_util_stdio_printf("\n=====================================================================================================\n");
}

void
tearDown(void)
{
}

//*****************************************************************************
//
// Optional Global setup/tear-down.
//
// globalSetUp() will get called before the test group starts, and
// globalTearDown() will get called after it ends. These won't be necessary for
// every test.
//
//*****************************************************************************
void
globalSetUp(void)
{

  //
  //  Enable the I-Cache and D-Cache.
  //
  am_hal_cachectrl_icache_enable();
  am_hal_cachectrl_dcache_enable(true);

  //
  // Set up the attributes.
  //
  am_hal_mpu_attr_configure(&sMPUAttr, 1);
  //
  // Clear the MPU regions.
  //
  am_hal_mpu_region_clear();
  //
  // Set up the regions.
  //
  am_hal_mpu_region_configure((am_hal_mpu_region_config_t *)&sMPUCfg, 2);
  //
  // Invalidate and clear DCACHE, this is required by CM55 TRF.
  //
  am_hal_cachectrl_dcache_invalidate(NULL, true);

  //
  // MPU enable
  //
  am_hal_mpu_enable(true, true);

  //
  // Print the banner.
  //
  am_bsp_itm_printf_enable();
  am_util_stdio_terminal_clear();
  am_util_stdio_printf("Apollo5B MSPI Test Cases\n\n");
}

void
globalTearDown(void)
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
// Prepare test data pattern.
//
//*****************************************************************************
bool mspi_prepare_data_pattern(uint32_t pattern_index, uint8_t* buff, uint32_t len)
{
    uint32_t *pui32TxPtr = (uint32_t*)buff;
    uint8_t  *pui8TxPtr  = (uint8_t*)buff;

    switch ( pattern_index )
    {
        case 0:
            // 0x5555AAAA
            for (uint32_t i = 0; i < len / 4; i++)
            {
               pui32TxPtr[i] = (0x5555AAAA);
            }
            break;
        case 1:
            // 0xFFFF0000
            for (uint32_t i = 0; i < len / 4; i++)
            {
               pui32TxPtr[i] = (0xFFFF0000);
            }
            break;
        case 2:
            // walking
            for (uint32_t i = 0; i < len; i++)
            {
               pui8TxPtr[i] = 0x01 << (i % 8);
            }
            break;
        case 3:
            // incremental from 1
            for (uint32_t i = 0; i < len; i++)
            {
               pui8TxPtr[i] = ((i + 1) & 0xFF);
            }
            break;
        case 4:
            // decremental from 0xff
            for ( uint32_t i = 0; i < len; i++ )
            {
                // decrement starting from 0xff
                pui8TxPtr[i] = (0xff - i) & 0xFF;
            }
            break;
        default:
            // incremental from 1
            for (uint32_t i = 0; i < len; i++)
            {
               pui8TxPtr[i] = ((i ) & 0xFF);
            }
            break;
    }
    return true;
}


//*****************************************************************************
//
// Check data match
//
//*****************************************************************************

bool mspi_check_data_match(uint8_t *pui8RdBuf, uint8_t *pui8WrBuf, uint32_t ui32Len)
{
    uint32_t i;
    uint32_t err_cnt =0;
    bool pass = true;
    for (i = 0; i < ui32Len; i++)
    {
        if (pui8RdBuf[i] != pui8WrBuf[i])
        {
            am_util_stdio_printf("RdBuf[%d]=%x,WrBuf[%d]=%x\n", i, pui8RdBuf[i], i, pui8WrBuf[i]);
            pass = false;
            err_cnt ++;
            if(err_cnt > MSPI_DATA_ERROR_CHECK_NUM)
            {
                am_util_stdio_printf("\n******************************************************************************\n");
                am_util_stdio_printf("Mismatched data count is larger than %d. Stopped printing data buffer, len=%d\n", MSPI_DATA_ERROR_CHECK_NUM, ui32Len);
                am_util_stdio_printf("********************************************************************************\n");
                return pass;
            }
        }
    }

    if (pass)
    {
        am_util_debug_printf("data matched\n");
    }
    else
    {
        am_util_stdio_printf("check mspi data fail: data not matched, len=%d\n", ui32Len);
    }

    return pass;
}

//*****************************************************************************
//
// Main Function
//
//*****************************************************************************

int
mspi_multiple_instances_test_cases(void)
{
    //
    // Initialize the printf interface for ITM output
    //
//     am_bsp_debug_printf_enable();

//     //
//     // Enable printing to the console.
//     //
// #ifdef AM_DEBUG_PRINTF
//     enable_print_interface();
// #endif

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
    am_util_stdio_printf("\n mspi_multiple_instances_test_cases \n");

    run_tasks();

    //
    // We shouldn't ever get here.
    //
    while (1)
    {
    }

}
