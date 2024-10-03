//*****************************************************************************
//
//! @file mspi_test_common.h
//!
//! @brief IOM test cases common definitions.
//
//*****************************************************************************

//*****************************************************************************
//
// ${copyright}
//
// This is part of revision ${version} of the AmbiqSuite Development Package.
//
//*****************************************************************************
#ifndef MSPI_TEST_COMMON_H
#define MSPI_TEST_COMMON_H

#include "unity.h"
#include "am_mcu_apollo.h"
#if (defined(MICRON_N25Q256A) || defined(MACRONIX_MX25U12835F) || defined(ADESTO_ATXP032) || defined(CYPRESS_S25FS064S) ||\
     defined(APS6404L) || defined(MB85RS64V))
#include "am_widget_mspi.h"
extern am_widget_mspi_devices_config_t MSPI_Flash_Config;
#endif
#include "am_util.h"

#define MSPI_TEST_READ_QUEUE
#if defined (AM_DEVICES_MSPI_FLASH_PAGE_SIZE)
#define MSPI_TEST_PAGE_INCR     (AM_DEVICES_MSPI_FLASH_PAGE_SIZE*17)
#endif
#if defined (AM_DEVICES_MSPI_PSRAM_PAGE_SIZE)
#define MSPI_PSRAM_TEST_PAGE_INCR     (AM_DEVICES_MSPI_PSRAM_PAGE_SIZE*7)
#endif
#define MSPI_SECTOR_INCR        (33)
#define MSPI_INT_TIMEOUT        (100)

#define DEFAULT_TIMEOUT         10000

#define START_SPEED_INDEX       (APOLLO3_GE_B0?0:1)
#define END_SPEED_INDEX         4

extern uint32_t        DMATCBBuffer[4096];
extern uint8_t         TestBuffer[2048];
extern uint8_t         DummyBuffer[1024];

typedef struct
{
  const am_hal_mspi_clock_e     speed;
  const char                    *string;
} mspi_speed_t;

extern mspi_speed_t mspi_test_speeds[];

extern void          *pWidget;
extern char          errStr[128];

#endif // MSPI_TEST_COMMON_H
