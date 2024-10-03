//*****************************************************************************
//
//! @file daxi_test_common.c
//!
//! @brief DAXI test cases common source code.
//
//*****************************************************************************

//*****************************************************************************
//
// ${copyright}
//
// This is part of revision ${version} of the AmbiqSuite Development Package.
//
//*****************************************************************************
#include "daxi_test_common.h"

//*****************************************************************************
//
// Macro definitions
//
//*****************************************************************************

//*****************************************************************************
//
// Global variables
//
//*****************************************************************************
static am_hal_daxi_config_numbuf_e g_eNumBuf[] =
#ifndef KNEE_OF_CURVE
{
    AM_HAL_DAXI_CONFIG_NUMBUF_32,
    AM_HAL_DAXI_CONFIG_NUMBUF_1,
    AM_HAL_DAXI_CONFIG_NUMBUF_4,
    AM_HAL_DAXI_CONFIG_NUMBUF_8,
    AM_HAL_DAXI_CONFIG_NUMBUF_16,
    AM_HAL_DAXI_CONFIG_NUMBUF_31
};
#else
{
    AM_HAL_DAXI_CONFIG_NUMBUF_32
};
#endif
static am_hal_daxi_config_aging_e g_eAging[] =
#ifndef KNEE_OF_CURVE
{
    AM_HAL_DAXI_CONFIG_AGING_2048,
    AM_HAL_DAXI_CONFIG_AGING_1,
    AM_HAL_DAXI_CONFIG_AGING_64,
    AM_HAL_DAXI_CONFIG_AGING_1024,
    AM_HAL_DAXI_CONFIG_AGING_4096,
    AM_HAL_DAXI_CONFIG_AGING_65536
};
#else
{
    AM_HAL_DAXI_CONFIG_AGING_256,
    AM_HAL_DAXI_CONFIG_AGING_512,
    AM_HAL_DAXI_CONFIG_AGING_1024,
    AM_HAL_DAXI_CONFIG_AGING_2048,
    AM_HAL_DAXI_CONFIG_AGING_4096
};
#endif
static daxi_func_comb_t g_sDaxiFuncComb[] = 
#ifndef KNEE_OF_CURVE
{
    {false, false, AM_HAL_DAXI_CONFIG_NUMFREEBUF_2},
    {true, true, AM_HAL_DAXI_CONFIG_NUMFREEBUF_3},
    {false, true, AM_HAL_DAXI_CONFIG_NUMFREEBUF_3}
};
#else
{
    {true, false, AM_HAL_DAXI_CONFIG_NUMFREEBUF_2},
    {false, false, AM_HAL_DAXI_CONFIG_NUMFREEBUF_2},
    {false, false, AM_HAL_DAXI_CONFIG_NUMFREEBUF_3},
    {false, true, AM_HAL_DAXI_CONFIG_NUMFREEBUF_2},
    {false, true, AM_HAL_DAXI_CONFIG_NUMFREEBUF_3}
};
#endif
am_hal_daxi_config_t g_sDaxiCfg[TEST_NUM];

static am_hal_cachectrl_descr_e g_eCacheDescr[] = 
{
    AM_HAL_CACHECTRL_DESCR_1WAY_128B_512E,
    AM_HAL_CACHECTRL_DESCR_2WAY_128B_512E,
    AM_HAL_CACHECTRL_DESCR_1WAY_128B_1024E,
    AM_HAL_CACHECTRL_DESCR_1WAY_128B_2048E,
    AM_HAL_CACHECTRL_DESCR_2WAY_128B_2048E,
    AM_HAL_CACHECTRL_DESCR_1WAY_128B_4096E
};
static am_hal_cachectrl_config_mode_e g_eCacheMode[] =
{
    AM_HAL_CACHECTRL_CONFIG_MODE_DISABLE,
    AM_HAL_CACHECTRL_CONFIG_MODE_INSTR,
    AM_HAL_CACHECTRL_CONFIG_MODE_DATA,
    AM_HAL_CACHECTRL_CONFIG_MODE_INSTR_DATA
};
static bool g_bCacheLRU[] =
{
    false,
    true
};
cache_enable_config_t g_sCacheCfg[CACHE_TEST_NUM];

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
globalTearDown(void)
{
}

//*****************************************************************************
//
// DAXI test configurations init.
//
//*****************************************************************************
void
daxi_test_cfg_init(void)
{
    uint32_t i, j, k, l = 0;
    for (i = 0; i < sizeof(g_sDaxiFuncComb) / sizeof(daxi_func_comb_t); i++)
    {
        if (g_sDaxiFuncComb[i].bPassThrough == true)
        {
            g_sDaxiCfg[l].bDaxiPassThrough = true;
            g_sDaxiCfg[l].bAgingSEnabled = true;
            g_sDaxiCfg[l].eAgingCounter = AM_HAL_DAXI_CONFIG_AGING_1;
            g_sDaxiCfg[l].eNumBuf = AM_HAL_DAXI_CONFIG_NUMBUF_1;
            g_sDaxiCfg[l].eNumFreeBuf = AM_HAL_DAXI_CONFIG_NUMFREEBUF_3;
            l++;
        }
        else
        {
            for (j = 0; j < sizeof(g_eAging) / sizeof(am_hal_daxi_config_aging_e); j++)
            {
                for (k = 0; k < sizeof(g_eNumBuf) / sizeof(am_hal_daxi_config_numbuf_e); k++)
                {
                    g_sDaxiCfg[l].bDaxiPassThrough = false;
                    g_sDaxiCfg[l].bAgingSEnabled = g_sDaxiFuncComb[i].bAgingSEnable;
                    g_sDaxiCfg[l].eAgingCounter = g_eAging[j];
                    g_sDaxiCfg[l].eNumBuf = g_eNumBuf[k];
                    g_sDaxiCfg[l].eNumFreeBuf = g_sDaxiFuncComb[i].eNumFreeBuff;
                    l++;
                }
            }
        }
    }
}

//*****************************************************************************
//
// MRAM cache test configurations init.
//
//*****************************************************************************
void
cache_test_cfg_init(void)
{
    uint32_t i, j, k, l = 0;
    g_sCacheCfg[l++].bCacheEnable = false;
    for (i = 0; i < sizeof(g_bCacheLRU) / sizeof(bool); i++)
    {
        for (j = 0; j < sizeof(g_eCacheMode) / sizeof(am_hal_cachectrl_config_mode_e); j++)
        {
            for (k = 0; k < sizeof(g_eCacheDescr) / sizeof(am_hal_cachectrl_descr_e); k++)
            {
                g_sCacheCfg[l].bCacheEnable = true;
                g_sCacheCfg[l].sCacheCtrl.bLRU = g_bCacheLRU[i];
                g_sCacheCfg[l].sCacheCtrl.eMode = g_eCacheMode[j];
                g_sCacheCfg[l].sCacheCtrl.eDescript = g_eCacheDescr[k];
                l++;
            }
        }
    }
}

