//*************************************************************************
//
//! @file am_devices_tpV1p1.h
//!
//! @brief ZTW523 I2C driver.
//!
//! @addtogroup tpV1p1 ZTW523 - I2C Driver
//! @ingroup devices
//! @{
//
//**************************************************************************

//*****************************************************************************
//
// ${copyright}
//
// This is part of revision ${version} of the AmbiqSuite Development Package.
//
//*****************************************************************************

#ifndef AM_DEVICES_TPV1P1_H
#define AM_DEVICES_TPV1P1_H

#ifdef __cplusplus
extern "C"
{
#endif

#define __DRV_ZTW523_H__

#define ZTW523_INT_PIN                  (10)
#define GPIO_TP_2V8_EN                  (5) //103B_1.5
#define GPIO_TP_RESET                   (7) //103B_1.7

#define TOUCH_POINT_MODE                0
#define MAX_SUPPORTED_FINGER_NUM        1
#define TPD_RES_139_46_MAX_X            453
#define TPD_RES_139_46_MAX_Y            453
#define TPD_RES_119_42_MAX_X            389
#define TPD_RES_119_42_MAX_Y            389
#define I2C_BUFFER_SIZE                 2   /*2bytes*/
#define TC_SECTOR_SZ                    8
#define PAGE_SIZE                       64

#define TP_POWERON_DELAY                10
#define CHIP_ON_DELAY                   10
#define CHIP_OFF_DELAY                  70
#define FIRMWARE_ON_DELAY               50
#define ENABLE                          1
#define DISABLE                         0

#define CHIP_ID_REG                     0xCC00
#define CHIP_ID_VALUE                   0xE628
#define ZTW523_SLAVE_ADDR               0x20
/*
different technique in 1.39 and 1.19
1.39:Lens GFF
1.19:DEO oncell
*/

#define HW_ID_BOE_LENS_139_46           0x0103
#define HW_ID_EDO_LENS_139_46           0x0203
#define HW_ID_EDO_EDO_119_42            0x2

/* chip code */
#define ZTW523_CHIP_CODE                0xE628

//! @name register map
//! @{
#define ZINITIX_SWRESET_CMD             0x0000
#define ZINITIX_WAKEUP_CMD              0x0001
#define ZINITIX_IDLE_CMD                0x0004
#define ZINITIX_SLEEP_CMD               0x0005
#define ZINITIX_CLEAR_INT_STATUS_CMD    0x0003
#define ZINITIX_CALIBRATE_CMD           0x0006
#define ZINITIX_SAVE_STATUS_CMD         0x0007
#define ZINITIX_SAVE_CALIBRATION_CMD    0x0008
#define ZINITIX_RECALL_FACTORY_CMD      0x000f
#define ZINITIX_VENDOR_CMD              0xC000
#define ZINITIX_INTN_CLEAR_CMD          0xC004
#define ZINITIX_NVM_INIT_CMD            0xC002
#define ZINITIX_PROGRAM_START_CMD       0xC001
#define ZINITIX_NVM_VPP                 0xC003
#define ZINITIX_NVM_WP                  0xC104
#define ZINITIX_POSTURE_REG             0x0126

#define ZINITIX_INIT_RETRY_CNT          3
#define TOUCH_CHECK_SHORT_MODE          14
#define TOUCH_SEC_MODE                  48
#define TOUCH_REF_MODE                  10
#define TOUCH_NORMAL_MODE               5
#define TOUCH_DELTA_MODE                3
#define TOUCH_DND_MODE                  6
#define TOUCH_PDND_MODE                 11
#define NORMAL_SHORT_VALUE              1000

#define ZINITIX_SENSITIVITY                     0x0020
#define ZINITIX_I2C_CHECKSUM_WCNT               0x016a
#define ZINITIX_I2C_CHECKSUM_RESULT             0x016c
#define ZINITIX_DEBUG_REG                       0x0115
#define ZINITIX_TOUCH_MODE                      0x0010
#define ZINITIX_CHIP_REVISION                   0x0011
#define ZINITIX_FIRMWARE_VERSION                0x0012
#define ZINITIX_MINOR_FW_VERSION                0x0121
#define ZINITIX_DATA_VERSION_REG                0x0013
#define ZINITIX_HW_ID                           0x0014
#define ZINITIX_SUPPORTED_FINGER_NUM            0x0015
#define ZINITIX_EEPROM_INFO                     0x0018
#define ZINITIX_INITIAL_TOUCH_MODE              0x0019
#define ZINITIX_TOTAL_NUMBER_OF_X               0x0060
#define ZINITIX_TOTAL_NUMBER_OF_Y               0x0061
#define ZINITIX_DELAY_RAW_FOR_HOST              0x007f
#define ZINITIX_BUTTON_SUPPORTED_NUM            0x00B0
#define ZINITIX_BUTTON_SENSITIVITY              0x00B2
#define ZINITIX_X_RESOLUTION                    0x00C0
#define ZINITIX_Y_RESOLUTION                    0x00C1
#define ZINITIX_POINT_STATUS_REG                0x0080
#define ZINITIX_ICON_STATUS_REG                 0x00AA
#define ZINITIX_AFE_FREQUENCY                   0x0100
#define ZINITIX_DND_N_COUNT                     0x0122
#define ZINITIX_DND_U_COUNT                     0x0135
#define ZINITIX_RAWDATA_REG                     0x0200
#define ZINITIX_EEPROM_INFO_REG                 0x0018
#define ZINITIX_INT_ENABLE_FLAG                 0x00f0
#define ZINITIX_PERIODICAL_INTERRUPT_INTERVAL   0x00f1
#define ZINITIX_CHECKSUM_RESULT                 0x012c
#define ZINITIX_INIT_FLASH                      0x01d0
#define ZINITIX_WRITE_FLASH                     0x01d1
#define ZINITIX_READ_FLASH                      0x01d2
#define ZINITIX_VENDOR_REG                      0xC000
#define ZINITIX_NVM_REG                         0xC002
#define ZINITIX_VENDOR_ID                       0x001C
#define ZINITIX_VENDOR_ID_VALUE                 0x5A49

#define BIT_PT_CNT_CHANGE               0
#define BIT_DOWN                        1
#define BIT_MOVE                        2
#define BIT_UP                          3
#define BIT_PALM                        4
#define BIT_PALM_REJECT                 5
#define BIT_WAKEUP                      6
#define RESERVED_1                      7
#define BIT_WEIGHT_CHANGE               8
#define BIT_PT_NO_CHANGE                9
#define BIT_REJECT                      10
#define BIT_PT_EXIST                    11
#define RESERVED_2                      12
#define BIT_MUST_ZERO                   13
#define BIT_DEBUG                       14
#define BIT_ICON_EVENT                  15

#define SUB_BIT_EXIST                   0
#define SUB_BIT_DOWN                    1
#define SUB_BIT_MOVE                    2
#define SUB_BIT_UP                      3
#define SUB_BIT_UPDATE                  4
#define SUB_BIT_WAIT                    5
//! @}

//! @name Test Mode (Monitoring Raw Data)
//! @{

#define SEC_DND_N_COUNT                 10
#define SEC_DND_U_COUNT                 2
#define SEC_DND_FREQUENCY               99

#define SEC_PDND_N_COUNT_139_46         27
#define SEC_PDND_U_COUNT_139_46         3
#define SEC_PDND_FREQUENCY_139_46       37

#define SEC_PDND_N_COUNT_119_42         41
#define SEC_PDND_U_COUNT_119_42         9
#define SEC_PDND_FREQUENCY_119_42       37
//! @}

//! @name preriod raw data interval
//! @{
#define RAWDATA_DELAY_FOR_HOST      100

#define zinitix_bit_set(val, n)     ((val) &= ~(1 << (n)), (val) |= (1 << (n)))
#define zinitix_bit_clr(val, n)     ((val) &= ~(1 << (n)))
#define zinitix_bit_test(val, n)    ((val) & (1 << (n)))
#define zinitix_swap_v(a, b, t)     ((t) = (a), (a) = (b), (b) = (t))
#define zinitix_swap_16(s)          (((s & 0x00FF) << 8) | ((s & 0xFF00) >> 8))
#define zinitix_abs(a, b)           ((a >= b) ? (a - b) : (b - a))
#define zinitix_max(a, b)           (a > b ? a : b)
//! @}

//! @note do not need to modify the alignment.
struct _ts_zinitix_coord
{
    uint16_t    x;
    uint16_t    y;
    uint8_t    width;
    uint8_t    sub_status;
};

struct _ts_zinitix_point_info
{
    uint16_t    status;
    #if TOUCH_POINT_MODE
    uint16_t event_flag;
    #else
    uint8_t    finger_cnt;
    uint8_t    time_stamp;
    #endif
    struct _ts_zinitix_coord    coord[MAX_SUPPORTED_FINGER_NUM];
};

struct ztw_touch_drivers
{
    //! struct touch_drivers driver;
    struct _ts_zinitix_point_info touch_info;
};
typedef struct ztw_touch_drivers *ztw_touch_drv_t;

#ifndef offset_of
    #define offset_of(T, x) ((size_t) &((T *)0)->x)
#endif
#ifndef container_of
    #define container_of(p, T, x) ((T *)((uint8_t *)(p) - offset_of(T, x)))
#endif

#define RT_I2C_WR                0x0000
#define RT_I2C_RD               (1u << 0)
#define RT_I2C_ADDR_10BIT       (1u << 2)  //!< this is a ten bit chip address
#define RT_I2C_NO_START         (1u << 4)
#define RT_I2C_IGNORE_NACK      (1u << 5)
#define RT_I2C_NO_READ_ACK      (1u << 6)  //!< when I2C reading, we do not ACK

#define IIC_RETRY_NUM             2

#define RT_EOK                          0               //!< There is no error
#define RT_ERROR                        1               //!< A generic error happens
#define RT_ETIMEOUT                     2               //!< Timed out
#define RT_EFULL                        3               //!< The resource is full
#define RT_EEMPTY                       4               //!< The resource is empty
#define RT_ENOMEM                       5               //!< No memory
#define RT_ENOSYS                       6               //!< No system
#define RT_EBUSY                        7               //!< Busy
#define RT_EIO                          8               //!< IO error
#define RT_EINTR                        9               //!< Interrupted system call
#define RT_EINVAL                       10              //!< Invalid argument

typedef int                             rt_bool_t;      //!> boolean type
typedef long                            rt_base_t;      //!> Nbit CPU related date type
typedef unsigned long                   rt_ubase_t;     //!> Nbit unsigned CPU related data type

typedef rt_base_t                       rt_err_t;       //!< Type for error number
//typedef rt_uint32_t                     rt_time_t;    //!< Type for time stamp
//typedef rt_uint32_t                     rt_tick_t;    //!< Type for tick count
typedef rt_base_t                       rt_flag_t;      //!< Type for flags
typedef rt_ubase_t                      rt_size_t;      //!< Type for size number
typedef rt_ubase_t                      rt_dev_t;       //!< Type for device
typedef rt_base_t                       rt_off_t;       //!< Type for offset

typedef signed   char                   rt_int8_t;      //!<  8bit integer type
typedef signed   short                  rt_int16_t;     //!< 16bit integer type
typedef signed   int                    rt_int32_t;     //!< 32bit integer type
typedef unsigned char                   rt_uint8_t;     //!<  8bit unsigned integer type
typedef unsigned short                  rt_uint16_t;    //!< 16bit unsigned integer type
typedef unsigned int                    rt_uint32_t;    //!< 32bit unsigned integer type

struct rt_i2c_msg
{
    uint16_t addr;
    uint16_t flags;
    uint16_t len;
    uint8_t  *buf;
};

struct touch_message
{
    rt_uint16_t x;
    rt_uint16_t y;
    rt_uint8_t event;
};

//*****************************************************************************
//
// Global type definitions.
//
//*****************************************************************************
typedef enum
{
    AM_DEVICES_TPV1P1_STATUS_SUCCESS,
    AM_DEVICES_TPV1P1_STATUS_ERROR
} am_devices_TPV1P1_status_t;

//*****************************************************************************
//
// External function definitions.
//
//*****************************************************************************
//*****************************************************************************
//
//! @brief
//! @param ui32Module
//! @param psIOMSettings
//! @param ppIomHandle
//! @return
//
//*****************************************************************************
extern uint32_t am_devices_tpV1p1_init(uint32_t ui32Module,
                                       am_hal_iom_config_t *psIOMSettings,
                                       void **ppIomHandle);

//*****************************************************************************
//
//! @brief
//! @param pui8RxBuffer
//! @param RxNumBytes
//
//*****************************************************************************
extern void am_devices_tpV1p1_data_Read(uint8_t *pui8RxBuffer,
                                        uint32_t RxNumBytes);

//*****************************************************************************
//
//! @brief
//! @param x
//! @param y
//! @param touch_released
//! @return
//
//*****************************************************************************
extern rt_err_t ztw_get_point(uint16_t *x,
                              uint16_t *y,
                              bool *touch_released);

#ifdef __cplusplus
}
#endif

#endif // AM_DEVICES_TPV1P1_H

//*****************************************************************************
//
// End Doxygen group.
//! @}
//
//*****************************************************************************

