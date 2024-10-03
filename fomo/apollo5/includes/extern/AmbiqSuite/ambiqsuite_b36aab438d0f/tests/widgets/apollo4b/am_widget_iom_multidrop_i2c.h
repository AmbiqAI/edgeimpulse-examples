//*****************************************************************************
//
//! @file am_widget_iom_multidrop_i2c.h
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
#ifndef AM_WIDGET_IOM_MULTIDROP_I2C_H
#define AM_WIDGET_IOM_MULTIDROP_I2C_H

#include "am_devices_tsl2540.h"
#include "am_devices_tma525.h"

#define IOM_WIDGET_DEVICE_MODE        AM_WIDGET_IOM_I2C_DEVICE
#define IOM_WIDGET_DEVICE_ID          AM_DEVICES_TSL2540_ID
#define IOM_WIDGET_DEVICE_SUCCESS     AM_DEVICES_TSL2540_STATUS_SUCCESS
#define am_widget_iom_devices_t       am_hal_iom_config_t

#define AM_WIDGET_IOM_ERRMASK_NAK_DEVADDR       0x1
#define AM_WIDGET_IOM_ERRMASK_NAK_OFFSETSTART   0x2
#define AM_WIDGET_IOM_ERRMASK_NAK_OFFSETMID     0x4
#define AM_WIDGET_IOM_ERRMASK_NAK_CONT          0x8
#define AM_WIDGET_IOM_ERRMASK_DMA_START         0x10
#define AM_WIDGET_IOM_ERRMASK_DMA_MID           0x20

//*****************************************************************************
//
// Loop Device - dummy driver
//
//*****************************************************************************
#define AM_DEVICES_LOOP_MAX_DEVICE_NUM    1
typedef enum
{
    AM_DEVICES_LOOP_STATUS_SUCCESS,
    AM_DEVICES_LOOP_STATUS_ERROR
} am_devices_loop_status_t;

typedef struct
{
    uint32_t                    ui32Module;
    uint32_t                    ui32CS;
    void                        *pIomHandle;
    bool                        bOccupied;
} am_devices_iom_loop_t;

typedef struct
{
    uint32_t ui32ClockFreq;
    uint32_t *pNBTxnBuf;
    uint32_t ui32NBTxnBufLength;
} am_devices_loop_config_t;

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
    AM_WIDGET_DEVICE_TSL2540,   // I2C ALS Device
    AM_WIDGET_DEVICE_TMA525,    // I2C Touch Device
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

//*****************************************************************************
//
// Loop Device - dummy driver
//
//*****************************************************************************
extern uint32_t am_devices_loop_init(uint32_t ui32Module, am_widget_iom_devices_t *pDevConfig,
                        void **ppHandle, void **ppIomHandle);
extern uint32_t am_devices_loop_term(void *pHandle);


extern volatile uint32_t g_IOMInterruptStatus;
extern uint32_t am_widget_iom_test_setup(am_widget_iom_config_t *pTestCfg,
                             void **ppWidget, char *pErrStr);
extern uint32_t am_widget_iom_test_cleanup(void *pWidget, char *pErrStr);
extern uint32_t am_widget_iom_test_read_id(void *pWidget, char *pErrStr);
extern uint32_t am_widget_iom_test_status_get(void *pWidget, char *pErrStr);
extern uint32_t am_widget_iom_test_nonblocking_read(void *pWidget, void *pCfg, char *pErrStr);

extern void            *g_IomDevHdl;
extern void            *g_pIOMHandle;

#endif // AM_WIDGET_IOM_H
