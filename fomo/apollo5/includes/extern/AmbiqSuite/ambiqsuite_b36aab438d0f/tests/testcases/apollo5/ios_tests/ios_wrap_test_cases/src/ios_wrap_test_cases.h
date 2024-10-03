//*****************************************************************************
//
//! @file ios_wrap_test_cases.h
//!
//! @brief Example used for demonstrating the host burst write feature.
//!
//
//*****************************************************************************

//*****************************************************************************
//
// ${copyright}
//
// This is part of revision ${version} of the AmbiqSuite Development Package.
//
//*****************************************************************************
#include "am_mcu_apollo.h"
#include <string.h>

#define USE_SPI             1   // 0 = I2C, 1 = SPI
#define I2C_ADDR            0x10

#define TEST_IOS_XCMP_INT   1

#define AM_HAL_IOS_INT_ERR  (AM_HAL_IOS_INT_FOVFL | AM_HAL_IOS_INT_FUNDFL | AM_HAL_IOS_INT_FRDERR)
#define AM_HAL_IOS_XCMP_INT (AM_HAL_IOS_INT_XCMPWR | AM_HAL_IOS_INT_XCMPWF | AM_HAL_IOS_INT_XCMPRR | AM_HAL_IOS_INT_XCMPRF)

#define IOS_SENT_REQ_PIN        8
#define IOS_REQ_ACK_PIN         9

#define INBUFFER_EMPTY              (0xEE)
#define TEST_XOR_BYTE               0xFF // 0 Will not change, 0xFF will invert
#define ROBUFFER_INIT               (0x55)

#define IOM_IOS_CHECKSUM                1

#define IOS_WRAPAROUND_MAX_LRAM_SIZE    0xF0

//#define IOS_WRAP_START_ADDRESS          0x0           // have 2nd transaction halt issue.
#define IOS_WRAP_START_ADDRESS          0x08
//#define IOS_WRAP_START_ADDRESS          0x38
//#define IOS_WRAP_START_ADDRESS          0x78

#define IOS_DBG_MSG                     0

#if (IOS_WRAP_START_ADDRESS == 0x00)
#define IOS_REGACC_THRESHOLD_01		AM_HAL_IOS_ACCESS_INT_08	// 0x8 at wrap mode
#define IOS_REGACC_THRESHOLD_02		AM_HAL_IOS_ACCESS_INT_2F	// 0x7F at wrap mode
#define IOS_WRAP_HALF_ADDRESS           0x7F
#elif (IOS_WRAP_START_ADDRESS == 0x08)
#define IOS_REGACC_THRESHOLD_01		AM_HAL_IOS_ACCESS_INT_0F	// 0xF at wrap mode
#define IOS_REGACC_THRESHOLD_02		AM_HAL_IOS_ACCESS_INT_2F	// 0x7F at wrap mode
#define IOS_WRAP_HALF_ADDRESS           0x7F
#elif (IOS_WRAP_START_ADDRESS == 0x38)
#define IOS_REGACC_THRESHOLD_01		AM_HAL_IOS_ACCESS_INT_1F	// 0x3F at wrap mode
#define IOS_REGACC_THRESHOLD_02		AM_HAL_IOS_ACCESS_INT_37	// 0x9F at wrap mode
#define IOS_WRAP_HALF_ADDRESS           0x9F
#elif (IOS_WRAP_START_ADDRESS == 0x78)
#define IOS_REGACC_THRESHOLD_01		AM_HAL_IOS_ACCESS_INT_33	// 0x8F at wrap mode
#define IOS_REGACC_THRESHOLD_02		AM_HAL_IOS_ACCESS_INT_3F	// 0xBF at wrap mode
#define IOS_WRAP_HALF_ADDRESS           0xBF                            // 
#endif

typedef enum
{
    IOS_STATE_START,
    IOS_STATE_STREAM,
    IOS_STATE_IDLE
}eIosState;

#define AM_IOM_TX_UNIT_SIZE     0x800         // 2K bytes
//#define AM_IOS_LRAM_SIZE_MAX    0x1000          // 4K bytes

// Apollo5 LRAM is defined as device memory which should be access aligned
// so we re-define here to replace that in lib
void ios_memcpy(void* dst, const void* src, size_t size);
inline void ios_memcpy(void* dst, const void* src, size_t size)
{
    for(uint16_t cpy_index = 0; cpy_index < (size); cpy_index++)
    {
        *((uint8_t*)dst + cpy_index) = *((uint8_t*)src + cpy_index);
    }
}

void ios_memset(void * dst, uint8_t val, size_t size);
inline void ios_memset(void * dst, uint8_t val, size_t size)
{
    for(uint16_t cpy_index = 0; cpy_index < (size); cpy_index++)
    {
        *((uint8_t*)dst + cpy_index) = val;
    }
}

