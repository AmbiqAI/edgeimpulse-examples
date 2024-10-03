//
//*****************************************************************************
//! @file mspi_multiple_instances_test_cases.h
//!
//! @brief Global includes for the mspi_multiple_instances_test_cases.
//
//*****************************************************************************

//*****************************************************************************
//
// ${copyright}
//
// This is part of revision ${version} of the AmbiqSuite Development Package.
//
//*****************************************************************************

#ifndef MSPI_MULTIPLE_INSTANCES_TEST_CASES_H
#define MSPI_MULTIPLE_INSTANCES_TEST_CASES_H

//*****************************************************************************
//
// Required built-ins.
//
//*****************************************************************************
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>

//*****************************************************************************
//
// Standard AmbiqSuite includes.
//
//*****************************************************************************
#include "am_mcu_apollo.h"
#include "am_bsp.h"
#include "am_util.h"
#include "unity.h"


//*****************************************************************************
//
// FreeRTOS include files.
//
//*****************************************************************************
#ifdef SYSTEM_VIEW
#include "SEGGER_SYSVIEW_FreeRTOS.h"
#endif
#include "FreeRTOS.h"
#include "task.h"
#include "portmacro.h"
#include "portable.h"
#include "semphr.h"
#include "event_groups.h"

#ifdef SYSTEM_VIEW
#include "SEGGER_SYSVIEW.h"
#endif

//*****************************************************************************
//
// Task include files.
//
//*****************************************************************************
#include "mspi_psram_task.h"
#include "mspi_flash_task.h"
#include "rtos.h"
#include "mspi_multiple_instances_test_cases.h"

#define TEST_NUM                    3
#define TEST_CONFIG_NUM             24
#define MSPI_DATA_ERROR_CHECK_NUM   10
#define MSPI_TEST_PATTERN_NUMBER    5       // 5 patterns

#define MSPI_PSRAM_MODULE               0
#define MSPI_FLASH_MODULE               1

#define MSPI_SEMAPHORE_TIMEOUT          0x100000//0xFFFFFFFE

#define MSPI_PSRAM_TASK_START_SPEED     1
#define MSPI_PSRAM_TASK_END_SPEED       3

#define MSPI_FLASH_TASK_START_SPEED     3
#define MSPI_FLASH_TASK_END_SPEED       4

#define MSPI_XIP_FUNCTION_SIZE  72

typedef uint32_t (*mspi_xip_test_function_t)(uint32_t, uint32_t, uint32_t);

typedef struct
{
    uint32_t binAddr;
    uint32_t funcOffset;
    uint32_t binSize;
    uint32_t param0;
    uint32_t param1;
    uint32_t param2;
    uint32_t result;
} mspi_xip_test_funcinfo_t;

typedef struct
{
  uint32_t                      NumBytes;               // Number of bytes to read/write.
  uint32_t                      ByteOffset;             // Byte offset in the RX/TX buffers.
  uint32_t                      SectorAddress;          // Sector address.
  uint32_t                      SectorOffset;           // Offset into Sector to write/read.
  bool                          TurnOnCaching;          // Turn on D/I caching.
  uint32_t                      UnscrambledSector;      // Address of the unscrambled sector.
  uint32_t                      ReadBlockSize;          // Number of bytes to read in each block.
} mspi_task_test_t;

typedef struct
{
  uint32_t      XIPBase;
  uint32_t      XIPMMBase;
} MSPIBaseAddr_t;

//*****************************************************************************
//
// Task priority definition.
//
//*****************************************************************************
#define TASK_PRIORITY_LOW               (1)
#define TASK_PRIORITY_MIDDLE            (TASK_PRIORITY_LOW + 1)
#define TASK_PRIORITY_HIGH              (TASK_PRIORITY_MIDDLE + 1)

extern void globalTearDown(void);
extern bool mspi_prepare_data_pattern(uint32_t pattern_index, uint8_t* buff, uint32_t len);
extern bool mspi_check_data_match(uint8_t *pui8RdBuf, uint8_t *pui8WrBuf, uint32_t ui32Len);
#endif // MSPI_MULTIPLE_INSTANCES_TEST_CASES_H