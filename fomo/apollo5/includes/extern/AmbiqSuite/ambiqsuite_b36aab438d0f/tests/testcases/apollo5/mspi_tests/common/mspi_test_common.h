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
     defined(APS6404L) || defined(APS12808L) || defined(APS25616N) || defined(APS25616BA) || defined(W958D6NW) || defined(MB85RS64V) ||\
     defined(AMBT52) || defined(DOSILICON_DS35X1GA) || defined(TOSHIBA_TC58CYG0) || defined(ISSI_IS25WX064) || defined(MACRONIX_MX25UM51245G))
#include "am_widget_mspi.h"
extern am_widget_mspi_devices_config_t MSPI_Flash_Config;

#define TIMER_NUM 0
#ifdef COLLECT_BANDWIDTH
#if defined(APOLLO5_FPGA)
#define TIME_DIVIDER       (16/(APOLLO5_FPGA*1.0))    //Chose HFRC_DIV16 as timer clk source when HFRC=8M
#else
#define TIME_DIVIDER       (16/96.0)                  //Chose HFRC_DIV16 as timer clk source when HFRC=96M
#endif

#define WAKE_INTERVAL_IN_MS     1
#define XT_PERIOD               32768
#define WAKE_INTERVAL           (XT_PERIOD * WAKE_INTERVAL_IN_MS / TIME_DIVIDER)

extern volatile uint32_t g_ui32TimerCount;
#endif
typedef struct
{
  const am_widget_mspi_access_e    eAccess;
  const char                       *string;
}mspi_access_str_t;

typedef struct
{
  const am_widget_mspi_device_model_e    eDevice;
  const char                             *string;
}mspi_device_str_t;

typedef struct
{
  const am_hal_mspi_device_e    eDeviceMode;
  const char                    *string;
}mspi_device_mode_str_t;

extern mspi_access_str_t mspi_test_access[];
extern mspi_device_str_t mspi_test_devices[];
extern mspi_device_mode_str_t mspi_test_device_modes[];

#endif
#include "am_util.h"

#define MSPI_TEST_READ_QUEUE
#if defined (AM_DEVICES_MSPI_FLASH_PAGE_SIZE)
#define MSPI_TEST_PAGE_INCR     (AM_DEVICES_MSPI_FLASH_SECTOR_SIZE)
#endif
#if defined (AM_DEVICES_MSPI_PSRAM_PAGE_SIZE)
#define MSPI_PSRAM_TEST_PAGE_INCR     (AM_DEVICES_MSPI_PSRAM_PAGE_SIZE*7)
#endif
#define MSPI_SECTOR_INCR        (AM_DEVICES_MSPI_FLASH_MAX_SECTORS / 10)
#define MSPI_INT_TIMEOUT        (100)

#define DEFAULT_TIMEOUT         10000

#if defined (APOLLO5_FPGA)
#define START_SPEED_INDEX       0
#else
#define START_SPEED_INDEX       3
#endif

#if defined(APOLLO5_FPGA)
#define END_SPEED_INDEX         20
#else
#define END_SPEED_INDEX         14
#endif

#define START_WINDOW_SIZE       30
#define END_WINDOW_SIZE         33

#define WINDOW_INCR             3

extern uint32_t        DMATCBBuffer[4096];
extern uint8_t         TestBuffer[2048];
extern uint8_t         DummyBuffer[1024];

typedef struct
{
  const am_hal_mspi_clock_e     speed;
  const char                    *string;
} mspi_speed_t;

extern mspi_speed_t mspi_test_speeds[];

extern void          *pWidget[AM_REG_MSPI_NUM_MODULES];
extern char          errStr[128];


#endif // MSPI_TEST_COMMON_H
