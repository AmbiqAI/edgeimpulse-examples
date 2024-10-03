//*****************************************************************************
//
//! @file ios_burst.h
//!
//! @brief Header for ios_burst example
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

#define IOS_TEST_ITERATION      5

#define IOS_MODE_HOST_WRITE     1
#define IOS_MODE_HOST_READ      2

#define TEST_IOS_XCMP_INT   1

#define AM_HAL_IOS_INT_ERR  (AM_HAL_IOS_INT_FOVFL | AM_HAL_IOS_INT_FUNDFL | AM_HAL_IOS_INT_FRDERR)
#define AM_HAL_IOS_XCMP_INT (AM_HAL_IOS_INT_XCMPWR | AM_HAL_IOS_INT_XCMPWF | AM_HAL_IOS_INT_XCMPRR | AM_HAL_IOS_INT_XCMPRF)

#define INBUFFER_EMPTY              (0xEE)
#define XOR_BYTE                    0// used to initilize the test data, 0 Will not change, 0xFF will invert
#define ROBUFFER_INIT               (0x55)

//#define SENSOR0_DATA_SIZE           200
#define SENSOR0_DATA_SIZE           896

//#define SENSOR0_FREQ                1 // 1 time a second
#define SENSOR0_FREQ                120 // 12 times a second

#define IOM_IOS_CHECKSUM                1


//#define IOS_WRAP_START_ADDRESS          0x0
#define IOS_WRAP_START_ADDRESS          0x08
//#define IOS_WRAP_START_ADDRESS          0x38
//#define IOS_WRAP_START_ADDRESS          0x78

#define IOS_DBG_MSG                     0

#if (IOS_WRAP_START_ADDRESS == 0x00)
#define IOS_REGACC_THRESHOLD_01     AM_HAL_IOS_ACCESS_INT_08    // 0x8 at wrap mode
#define IOS_REGACC_THRESHOLD_02     AM_HAL_IOS_ACCESS_INT_2F    // 0x7F at wrap mode
#define IOS_WRAP_HALF_ADDRESS           0x7F
#elif (IOS_WRAP_START_ADDRESS == 0x08)
#define IOS_REGACC_THRESHOLD_01     AM_HAL_IOS_ACCESS_INT_0F    // 0xF at wrap mode
#define IOS_REGACC_THRESHOLD_02     AM_HAL_IOS_ACCESS_INT_2F    // 0x7F at wrap mode
#define IOS_WRAP_HALF_ADDRESS           0x7F
#elif (IOS_WRAP_START_ADDRESS == 0x38)
#define IOS_REGACC_THRESHOLD_01     AM_HAL_IOS_ACCESS_INT_1F    // 0x3F at wrap mode
#define IOS_REGACC_THRESHOLD_02     AM_HAL_IOS_ACCESS_INT_37    // 0x9F at wrap mode
#define IOS_WRAP_HALF_ADDRESS           0x9F
#elif (IOS_WRAP_START_ADDRESS == 0x78)
#define IOS_REGACC_THRESHOLD_01     AM_HAL_IOS_ACCESS_INT_33    // 0x8F at wrap mode
#define IOS_REGACC_THRESHOLD_02     AM_HAL_IOS_ACCESS_INT_3F    // 0xBF at wrap mode
#define IOS_WRAP_HALF_ADDRESS           0xBF
#endif

#define TEST_IOS_MODULE                 0

#define HANDSHAKE_PIN                   4

#if defined(AM_PART_APOLLO5B)
#if (TEST_IOS_MODULE == 0)
#define SLINT_GPIO                      1
#define lram_array                      am_hal_ios_pui8LRAM
#define IOS_ACC_IRQ                     IOSLAVEACC_IRQn
#define IOS_IRQ                         IOSLAVE_IRQn
#define ios_isr                         am_ioslave_ios_isr
#define ios_acc_isr                     am_ioslave_acc_isr
#define SLINT_FUN_SEL                   AM_HAL_PIN_4_GPIO //Apollo5B does not expose IOS0 SLINT pin
#define IOS_PWRCTRL                     AM_HAL_PWRCTRL_PERIPH_IOS0
#define IOS_WRAPAROUND_MAX_LRAM_SIZE    0xF0
#define IOS_FIFO_THRESHOLD              0x80
#elif (TEST_IOS_MODULE == 1)
#define SLINT_GPIO                      0
#define lram_array                      am_hal_iosfd0_pui8LRAM
#define IOS_ACC_IRQ                     IOSLAVEFDACC0_IRQn
#define IOS_IRQ                         IOSLAVEFD0_IRQn
#define ios_isr                         am_ioslave_fd0_isr
#define ios_acc_isr                     am_ioslave_fd0_acc_isr
#define SLINT_FUN_SEL                   AM_HAL_PIN_4_SLFDINT
#define IOS_PWRCTRL                     AM_HAL_PWRCTRL_PERIPH_IOSFD1
#define IOS_WRAPAROUND_MAX_LRAM_SIZE    0x38
#define IOS_FIFO_THRESHOLD              0x20
#elif (TEST_IOS_MODULE == 2)
#define SLINT_GPIO                      0
#define lram_array                      am_hal_iosfd1_pui8LRAM
#define IOS_ACC_IRQ                     IOSLAVEFDACC1_IRQn
#define IOS_IRQ                         IOSLAVEFD1_IRQn
#define ios_isr                         am_ioslave_fd1_isr
#define ios_acc_isr                     am_ioslave_fd1_acc_isr
#define SLINT_FUN_SEL                   AM_HAL_PIN_4_SLFD1INT
#define IOS_PWRCTRL                     AM_HAL_PWRCTRL_PERIPH_IOSFD2
#define IOS_WRAPAROUND_MAX_LRAM_SIZE    0x38
#define IOS_FIFO_THRESHOLD              0x20
#endif
//#define ENABLE_DMA                      1
#else
#define SLINT_GPIO                      0
#define lram_array                      am_hal_ios_pui8LRAM
#define IOS_ACC_IRQ                     IOSLAVEACC_IRQn
#define IOS_IRQ                         IOSLAVE_IRQn
#define ios_isr                         am_ioslave_ios_isr
#define ios_acc_isr                     am_ioslave_acc_isr
#define SLINT_FUN_SEL                   AM_HAL_PIN_4_SLINT
#define IOS_PWRCTRL                     AM_HAL_PWRCTRL_PERIPH_IOS
#define IOS_WRAPAROUND_MAX_LRAM_SIZE    0xF0
#define IOS_FIFO_THRESHOLD              0x80
#endif

typedef enum
{
    IOS_STATE_HOST_WRITE_START,
    IOS_STATE_HOST_WRITE_STREAM,
    IOS_STATE_HOST_READ_NODATA,
    IOS_STATE_HOST_READ_DATA,
    IOS_STATE_IDLE
} AM_IOS_STATE_E;

#define HANDSHAKE_CMD_STOP         0xFC
#define HANDSHAKE_CMD_WRITE_DATA   0xFD
#define HANDSHAKE_CMD_READ_DATA    0xFE
#define HANDSHAKE_CMD_ACK          0xFF

#define AM_IOSTEST_IOSTOHOST_DATAAVAIL_INTMASK  1

//#define AM_IOM_TX_UNIT_SIZE     0x800         // 2K bytes
#define AM_IOM_TX_UNIT_SIZE    0x1000          // 4K bytes

// Apollo5 LRAM is defined as device memory which should be access aligned
// so we re-define here to replace that in lib
void ios_memcpy(void* dst, const void* src, size_t size);
inline void ios_memcpy(void* dst, const void* src, size_t size)
{
    for ( uint16_t cpy_index = 0; cpy_index < (size); cpy_index++ )
    {
        *((uint8_t*)dst + cpy_index) = *((uint8_t*)src + cpy_index);
    }
}

void ios_memset(void * dst, uint8_t val, size_t size);
inline void ios_memset(void * dst, uint8_t val, size_t size)
{
    for ( uint16_t cpy_index = 0; cpy_index < (size); cpy_index++ )
    {
        *((uint8_t*)dst + cpy_index) = val;
    }
}

