//*****************************************************************************
//
//! @file am_widget_iom.h
//!
//! @brief Test widget for testing IOM channels by data transfer using spifram.
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

#ifndef AM_WIDGET_IOM_H
#define AM_WIDGET_IOM_H

#define AM_WIDGET_IOM_ERRMASK_NAK_DEVADDR       0x1
#define AM_WIDGET_IOM_ERRMASK_NAK_OFFSETSTART   0x2
#define AM_WIDGET_IOM_ERRMASK_NAK_OFFSETMID     0x4
#define AM_WIDGET_IOM_ERRMASK_NAK_CONT          0x8
#define AM_WIDGET_IOM_ERRMASK_DMA_START         0x10
#define AM_WIDGET_IOM_ERRMASK_DMA_MID           0x20

typedef enum
{
    AM_WIDGET_SUCCESS,
    AM_WIDGET_ERROR
} am_widget_iom_status_t;

typedef enum
{
    AM_WIDGET_IOM_BLOCKING,
    AM_WIDGET_IOM_NONBLOCKING
} am_widget_iom_mode_t;

typedef enum
{
    AM_WIDGET_DEVICE_MB85RS64V,     // SPI FRAM
    AM_WIDGET_DEVICE_MB85RC256V,    // I2C FRAM
    AM_WIDGET_DEVICE_EM9304,        // EM9304 (for duplex testing)
    AM_WIDGET_DEVICE_PSRAM          // PSRAM
} am_widget_iom_device_t;

typedef struct
{
    uint32_t              ui32Module;
    uint32_t              ui32DeviceType;
    am_hal_iom_config_t   IOMConfig;
} am_widget_iom_config_t;

typedef struct
{
    uint32_t                      ui32BufferSize;
    uint32_t                      ui32ByteOffset;
    uint32_t                      ui32QueueSize; // Relevant only for am_widget_iom_test_queue_write_read
    am_hal_sysctrl_power_state_e  sleepMode; // Relevant only for nonblocking tests
} am_widget_iom_test_t;

extern volatile uint32_t g_IOMInterruptStatus;
extern uint32_t am_widget_iom_test_setup(am_widget_iom_config_t *pTestCfg,
                             void **ppWidget, char *pErrStr);
extern uint32_t am_widget_iom_test_cleanup(void *pWidget, char *pErrStr);
extern uint32_t am_widget_iom_test_read_id(void *pWidget, char *pErrStr);
extern uint32_t am_widget_iom_test_status_get(void *pWidget, char *pErrStr);
extern uint32_t am_widget_iom_test_blocking_write_read(void *pWidget, void *pTestCfg, char *pErrStr);
extern uint32_t am_widget_iom_test_nonblocking_write_read(void *pWidget, void *pTestCfg, char *pErrStr);
extern uint32_t am_widget_iom_test_queue_write_read(void *pWidget, void *pTestCfg, char *pErrStr);
extern uint32_t am_widget_iom_test_duplex(void *pWidget, void *pTestCfg, char *pErrStr);
extern uint32_t am_widget_iom_power_saverestore(void *pWidget, am_hal_sysctrl_power_state_e sleepMode);
extern uint32_t am_widget_iom_test_blocking_write_read_error(void *pWidget, void *pCfg, char *pErrStr, uint32_t errorMask);
extern uint32_t am_widget_iom_test_queue_write_read_error(void *pWidget, void *pCfg, char *pErrStr, uint32_t errorMask);

#endif // AM_WIDGET_IOM_H
