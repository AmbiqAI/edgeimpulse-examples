//*****************************************************************************
//
//! @file cachectrl_test_cases.c
//!
//! @brief CACHECTRL test cases.
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
#include "am_widget_cachectrl.h"
#include "am_util.h"

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
    // Print the banner.
    //
    am_util_stdio_terminal_clear();
    am_util_stdio_printf("Apollo4 CACHECTRL Test Cases\n\n");
}

void
globalTearDown(void)
{
}

//*****************************************************************************
//  Helper functions
//*****************************************************************************

unsigned int
prime_number( unsigned int n )
{
    unsigned int count = 1, prime;

    for ( unsigned int i = 3; i < n ; i++ )
    {
        prime = 1;
        for ( unsigned int j = 2; j * j <= i; j++ )
        {
            if ( (i % j) == 0 )
            {
                prime = 0;
                break;
            }
        }
        if ( prime )
        {
            count++;
        }
    }

    return count;
}

//*****************************************************************************
//
//  Test Case
//  CACHECTRL control
//
//*****************************************************************************
void
cachectrl_test_config(void)
{
    uint32_t ux;
    bool bTestPass = true;

    am_util_stdio_printf("Cache config test takes a few seconds, patience please...\n");

    //
    // Start with a simple write bit test. Check every non-reserved bit in the
    // register except the enable bit at bit0.
    // CACHECFG register:
    // RSVD[19:12], RSVD[23:21], RSVD[31:25], ENABLE[0:0]
    //
    for ( ux = 0; ux < (1 << 24); ux += 2 )
    {
        if ( 0 == (ux % 500000) )
        {
          am_util_stdio_printf(".");
        }
        
        if ( ux & ((0x1 << 0) | (0xFF << 12) | (0x7 << 21) | ((uint32_t)0x7F << 25)) )
        {
            continue;
        }
        
        //am_util_stdio_printf("CPU->CACHECFG = %8.8X\n",ux | 1);
        
        CPU->CACHECFG = ux | 1;

        if ( CPU->CACHECFG != (ux | 1) )
        {
            am_util_stdio_printf("Fail: CACHECFG value 0x%08x (ux=%d)\n", ux, ux);
            bTestPass = false;
        }
        
        CPU->CACHECTRL_b.INVALIDATE = 1;
        
        prime_number(100);
    }

    am_util_stdio_printf("\n");

    //
    // Now that the config test is done, turn on cacheing and montoring.
    //
    am_hal_cachectrl_config(&am_hal_cachectrl_defaults);
    am_hal_cachectrl_enable();
    am_hal_cachectrl_control(AM_HAL_CACHECTRL_CONTROL_MONITOR_ENABLE, 0);

    TEST_ASSERT_TRUE(bTestPass);

    return;

} // cachectrl_test_config()

//*****************************************************************************
//
//  Test Case
//  CACHECTRL status
//
//*****************************************************************************
bool
cachectrl_test_status(void)
{
    bool bTestPass = true;
    uint32_t ui32RetVal;
    am_hal_cachectrl_status_t sStatus;

    ui32RetVal = am_hal_cachectrl_status_get(&sStatus);
    if ( ui32RetVal )
    {
        am_util_stdio_printf("Fail: am_hal_cachectrl_status_get() returned %d).\n",
                             ui32RetVal);
        bTestPass = false;
    }

    //
    // At this point, nothing should be in sleep mode.
    //
    if ( !sStatus.bCacheReady )
    {
        am_util_stdio_printf("Fail: Cache is not ready.\n");
        bTestPass = false;
    }

    TEST_ASSERT_TRUE(bTestPass);

    return bTestPass;

} // cachectrl_test_status()

//*****************************************************************************
//
//  Test Case
//  CACHECTRL control
//
//*****************************************************************************
#define CHK_MON_DISABLE     0   // Replicate Jira CORVETTE-951

void
cachectrl_test_control(void)
{
    uint32_t ui32RetVal, ui32MonRegMask, ux;
    bool bTestPass = true;
    am_hal_cachectrl_status_t sStatus;

    ui32MonRegMask = 0;
    ui32MonRegMask |= CPU->DMON0 ? 0x1 : 0;
    ui32MonRegMask |= CPU->DMON1 ? 0x2 : 0;
    ui32MonRegMask |= CPU->DMON2 ? 0x4 : 0;
    ui32MonRegMask |= CPU->DMON3 ? 0x8 : 0;

    if ( ui32MonRegMask )
    {
#if CHK_MON_DISABLE
        //
        // Disable cache monitoring to prevent the monitor counters from further counting.
        //
        am_hal_cachectrl_control(AM_HAL_CACHECTRL_CONTROL_MONITOR_DISABLE, 0);
#endif

        //
        // Call the function to reset the counters
        //
        ui32RetVal = am_hal_cachectrl_control(AM_HAL_CACHECTRL_CONTROL_STATISTICS_RESET, 0);
        if ( ui32RetVal )
        {
            am_util_stdio_printf("Fail: am_hal_cachectrl_control(AM_HAL_CACHECTRL_CONTROL_STATISTICS_RESET) returned %d).\n",
                                 ui32RetVal);
            bTestPass = false;
        }

        //
        // Halt and check the monitor counters
        //
#if !CHK_MON_DISABLE
        //
        // Disable cache monitoring to prevent the monitor counters from further counting.
        //
        am_hal_cachectrl_control(AM_HAL_CACHECTRL_CONTROL_MONITOR_DISABLE, 0);
#endif

        ux = 0;
        while ( ux < 4 )
        {
            if ( ui32MonRegMask & (0x1 << ux) )
            {
                uint32_t ui32Addr = (uint32_t)&CPU->DMON0 + (ux * 4);

                //
                // Immediately after the statistics reset, we disabled monitor
                // which should snapshot the monitor counters at a low number.
                //
                if ( AM_REGVAL(ui32Addr) > 5 )
                {
                    am_util_stdio_printf("Fail: DMON%d did not reset (0x%08x = 0x%08x).\n", ux, ui32Addr, AM_REGVAL(ui32Addr));
                    bTestPass = false;
                }
            }
            else
            {
                am_util_stdio_printf("Notice: DMON%d was already zero.\n", ux);
            }
            ux++;
        }
    }
    else
    {
        am_util_stdio_printf("Notice: DMON0-3 were all already zero.\n");
    }


    //
    // Check cache invalidate
    //
    ui32RetVal = am_hal_cachectrl_control(AM_HAL_CACHECTRL_CONTROL_MRAM_CACHE_INVALIDATE, 0);
    if ( ui32RetVal )
    {
        am_util_stdio_printf("Fail: am_hal_cachectrl_control(AM_HAL_CACHECTRL_CONTROL_MRAM_CACHE_INVALIDATE) returned %d).\n",
                             ui32RetVal);
        bTestPass = false;
    }

    ui32RetVal = am_hal_cachectrl_status_get(&sStatus);
    if ( ui32RetVal )
    {
        am_util_stdio_printf("Fail: am_hal_cachectrl_status_get() returned %d).\n",
                             ui32RetVal);
        bTestPass = false;
    }

    TEST_ASSERT_TRUE(bTestPass);

    return;

} // cachectrl_test_control()

static int
memcmp_local(void *pA, void *pB, uint32_t nsize)
{
    uint8_t *p8A = (uint8_t*)pA;
    uint8_t *p8B = (uint8_t*)pB;
    for ( uint32_t ux = 0; ux < nsize; ux++ )
    {
        if (p8A[ux] != p8B[ux])
        {
            return 1;
        }
    }
    return 0;
}

void
cachectrl_test_daxiconfig(void)
{
    uint32_t ui32RetVal;
    am_hal_daxi_config_t curSettings;
    bool bTestPass = true;

    ui32RetVal = am_hal_daxi_config(&am_hal_daxi_defaults);
    if ( ui32RetVal )
    {
        am_util_stdio_printf("Fail: am_hal_daxi_config() returned %d).\n",
                             ui32RetVal);
        bTestPass = false;
    }
    ui32RetVal = am_hal_daxi_config_get(&curSettings);
    if ( ui32RetVal )
    {
        am_util_stdio_printf("Fail: am_hal_daxi_config_get() returned %d).\n",
                             ui32RetVal);
        bTestPass = false;
    }
    
    if (memcmp_local((void*)&curSettings, (void*)&am_hal_daxi_defaults, sizeof(am_hal_daxi_config_t)))
    {
        am_util_stdio_printf("Fail: DAXI settings do not match with config\n");
        bTestPass = false;
    }

    TEST_ASSERT_TRUE(bTestPass);

    return;

} // cachectrl_test_daxiconfig()
