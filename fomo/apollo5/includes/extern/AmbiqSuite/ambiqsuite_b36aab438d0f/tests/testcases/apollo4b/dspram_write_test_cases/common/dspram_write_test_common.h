//*****************************************************************************
//
//! @file dspram_write_test_common.h
//!
//! @brief dapram write test cases common definitions.
//
//*****************************************************************************

//*****************************************************************************
//
// ${copyright}
//
// This is part of revision ${version} of the AmbiqSuite Development Package.
//
//*****************************************************************************
#ifndef DSPRAM_WRITE_TEST_COMMON_H
#define DSPRAM_WRITE_TEST_COMMON_H

#include "unity.h"
#include "am_mcu_apollo.h"
#if (defined(MICRON_N25Q256A) || defined(MACRONIX_MX25U12835F) || defined(ADESTO_ATXP032) || defined(CYPRESS_S25FS064S) ||\
     defined(APS6404L) || defined(APS12808L) || defined(MB85RS64V))
#include "am_widget_mspi.h"
extern am_widget_mspi_devices_config_t MSPI_Flash_Config;
#endif
#include "am_util.h"

#define MSPI_TEST_NUM_INSTANCES         0
#define DSPRAM_WRITE_TEST_COUNT         (0x100)

#define DSPRAM_TEST_ADDR        0x10170000
#define PSRAM_TEST_ADDR         0x14040000
#define SSRAM_TEST_ADDR         0x100E0000
#define PSRAM_DUMMY_READ_ADDR   0x14000000
#define SSRAM_DUMMY_READ_ADDR   0x10060000

#define DSPRAM_START_ADDR       0x10160000
#define DSPRAM_END_ADDR         0x101D8000
#define SSRAM_START_ADDR        0x10060000
#define SSRAM_END_ADDR          0x10160000
#define PSRAM_START_ADDR        0x14000000
#define PSRAM_END_ADDR          0x18000000

#define DSPRAM0_TEST_ADDR0       0x10162A40   
#define DSPRAM0_TEST_ADDR1       0x10164A40 
#define DSPRAM1_TEST_ADDR0       0x101C1B00            
#define DSPRAM1_TEST_ADDR1       0x101C2B00  

extern uint8_t          WriteBuffer[6];
extern void             *pWidget;
extern char             errStr[128];

extern uint32_t         *g_pDSPRAMAddr[4];
extern uint32_t         g_uiDSPRAMData[4];

extern am_widget_mspi_config_t widgetConfig;
 

#endif // DSPRAM_WRITE_TEST_COMMON_H