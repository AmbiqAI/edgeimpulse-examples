//*****************************************************************************
//
//! @file daxi_test_common.h
//!
//! @brief DAXI test cases common definitions.
//
//*****************************************************************************

//*****************************************************************************
//
// ${copyright}
//
// This is part of revision ${version} of the AmbiqSuite Development Package.
//
//*****************************************************************************
#ifndef DAXI_TEST_COMMON_H
#define DAXI_TEST_COMMON_H

#include "unity.h"
#include "am_mcu_apollo.h"
#include "am_util.h"
#include "am_bsp.h"
#include "am_util_delay.h"
#include "am_util_stdio.h"
#include "string.h"
#ifndef KNEE_OF_CURVE
    #define TEST_NUM 73
#else
    #define TEST_NUM 21
#endif
#define CACHE_TEST_NUM 49
typedef struct 
{
    bool bPassThrough;
    bool bAgingSEnable;
    am_hal_daxi_config_numfreebuf_e eNumFreeBuff;
} daxi_func_comb_t;
extern am_hal_daxi_config_t g_sDaxiCfg[TEST_NUM];
extern void daxi_test_cfg_init(void);

typedef struct 
{
    bool bCacheEnable;
    am_hal_cachectrl_config_t sCacheCtrl;
} cache_enable_config_t;
extern cache_enable_config_t g_sCacheCfg[CACHE_TEST_NUM];
extern void cache_test_cfg_init(void);
#endif // DAXI_TEST_COMMON_H
